#include "object.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdio.h>

#include "unused.h"

#define RESERVE_OBJECT_COUNT (128)
#define EMPTY_FREE_LIST (SIZE_MAX)

#define LOG(fmt, ...) printf("OBJECTS: "fmt"\n", __VA_ARGS__)
#define LOGF(fmt, ...) printf("OBJECTS: "fmt"\n", __VA_ARGS__)

const char *OBJECT_FUNCTIONS_STRING[] = {
        "OBJ_FUNCTION_START",
        "OBJ_FUNCTION_UPDATE",
        "OBJ_FUNCTION_RENDER",
        "OBJ_FUNCTION_FREE",
        "OBJ_FUNCTION_COUNT",
        [255] = "OBJ_FUNCTION_INVALID"
};

struct object {
        size_t payload_size;
        uint64_t register_id;
        void (*functions[OBJ_FUNCTION_COUNT])(void *self);
};

#define OBJECT_TO_PAYLOAD_PTR(object)   ((void*)((object) + 1))
#define PAYLOAD_TO_OBJECT_PTR(payload)  (((struct object *)(payload)) - 1)

struct object_array_entry {
        struct object   *obj;
        size_t  next_free_entry_index;
};

struct object_array {
        struct object_array_entry *data;
        size_t free_list_index;
        size_t count;
        size_t capacity;
};

static struct object_array OBJECTS = {0};

static inline void object_initialize_list(size_t reserve_capacity)
{
        if(reserve_capacity == 0) {
                OBJECTS.count    = 0;
                OBJECTS.capacity = 0;
                OBJECTS.data     = NULL;
                OBJECTS.free_list_index = EMPTY_FREE_LIST;
                return;
        }

        OBJECTS.count    = 0;
        OBJECTS.capacity = reserve_capacity;
        OBJECTS.data     = calloc(OBJECTS.capacity, sizeof(*OBJECTS.data));
        OBJECTS.free_list_index = EMPTY_FREE_LIST;
}

static bool object_register(struct object *obj)
{
        if(OBJECTS.data == NULL) {
                object_initialize_system();
        }

        if(OBJECTS.capacity >= OBJECTS.count) {
                if(OBJECTS.free_list_index != EMPTY_FREE_LIST) {
                        const size_t free_index = OBJECTS.free_list_index;
                        OBJECTS.free_list_index = OBJECTS.data[free_index].next_free_entry_index;
                        OBJECTS.data[free_index].obj = obj;
                        OBJECTS.data[free_index].obj->register_id = free_index;
                        LOGF("Registered object %zu from free list.", free_index);
                        return true;
                }

                size_t new_capacity = (OBJECTS.capacity * 2 > OBJECTS.capacity) ? OBJECTS.capacity * 2 : RESERVE_OBJECT_COUNT;
                OBJECTS.data = reallocarray(OBJECTS.data, new_capacity, sizeof(*OBJECTS.data));
        }

        const size_t index = OBJECTS.count;
        OBJECTS.data[index].obj = obj;
        OBJECTS.data[index].obj->register_id = index;
        ++OBJECTS.count;
        LOGF("Registered object %zu.", index);
        return true;
}

static bool object_unregister(size_t register_id)
{
        if(register_id >= OBJECTS.count || OBJECTS.data[register_id].obj == NULL)
                return false;

        // TODO: It could be that after unregister the last object in the list
        // that this object was preceded by a range of free elements in the free list.
        // These preceded objects could also be removed from the free list and the
        // object count could be set at the start of the range for faster allocation.

        if(register_id == OBJECTS.count - 1) {
                LOGF("Unregistered object %zu.", OBJECTS.count - 1);
                OBJECTS.data[OBJECTS.count - 1].obj = NULL;
                OBJECTS.data[OBJECTS.count - 1].next_free_entry_index = EMPTY_FREE_LIST;
                --OBJECTS.count;
                return true;
        }

        OBJECTS.data[register_id].obj = NULL;
        OBJECTS.data[register_id].next_free_entry_index = OBJECTS.free_list_index;
        OBJECTS.free_list_index = register_id;
        LOGF("Unregistered object %zu.", register_id);
        return true;
}

void object_initialize_system()
{
        LOGF("Reserving %d objects.", RESERVE_OBJECT_COUNT);
        object_initialize_list(RESERVE_OBJECT_COUNT);
}

void *object_allocate(size_t object_size)
{
        LOGF("Allocating object with size %zu.", object_size);
        struct object* obj = malloc(sizeof(struct object) + object_size);
        obj->payload_size = object_size;
        memset(obj->functions, 0, sizeof(obj->functions));
        object_register(obj);
        object_set_function(OBJECT_TO_PAYLOAD_PTR(obj), OBJ_FUNCTION_FREE, object_free);
        return OBJECT_TO_PAYLOAD_PTR(obj);
}

void *object_allocate_array(size_t object_count, size_t object_size)
{
        LOGF("Allocating %zu objects with size %zu.", object_count, object_size);
        const size_t size_of_object = sizeof(struct object) + object_size;
        uint8_t* objects    = malloc(object_count * size_of_object);
        void **object_array = malloc(object_count * sizeof(void*));
        for(size_t i = 0; i < object_count; ++i) {
                struct object *obj = (struct object *)(objects + size_of_object * i);
                obj->payload_size = object_size;
                memset(obj->functions, 0, sizeof(obj->functions));
                object_register(obj);
                if(i == 0) {
                        object_set_function(OBJECT_TO_PAYLOAD_PTR(obj), OBJ_FUNCTION_FREE, object_free);
                }

                object_array[i] = OBJECT_TO_PAYLOAD_PTR(obj);
        }

        return object_array;
}

void *object_allocate_and_start(size_t object_size, void (*start_func)(void *self))
{
        void *object = object_allocate(object_size);
        object_set_function(object, OBJ_FUNCTION_START, start_func);
        object_call_on_object(object, OBJ_FUNCTION_START);
        return object;
}

void object_free(void *object)
{
        if(object == NULL)
                return;

        struct object *object_header = PAYLOAD_TO_OBJECT_PTR(object);
        LOGF("Freeing object %zu with payload size %zu",
                        object_header->register_id, object_header->payload_size);
        object_unregister(object_header->register_id);
        free(object_header);
}

bool object_set_function(void *object, enum OBJECT_FUNCTIONS function_type, void (*func)(void *self))
{
        if(object == NULL || function_type >= OBJ_FUNCTION_COUNT)
                return false;

        struct object *object_header = PAYLOAD_TO_OBJECT_PTR(object);
        LOGF("Set function %s of object %zu.", OBJECT_FUNCTIONS_STRING[function_type], object_header->register_id);
        object_header->functions[function_type] = func;
        return true;
}

void object_call_on_object(void *object, enum OBJECT_FUNCTIONS function_type)
{
        if(object == NULL)
                return;

        struct object *object_header = PAYLOAD_TO_OBJECT_PTR(object);
        if(object_header->functions[function_type] == NULL)
                return;

        object_header->functions[function_type](object);
}

void object_call_on_all(enum OBJECT_FUNCTIONS function_type)
{
        for(size_t i = 0; i < OBJECTS.count; ++i) {
                if(OBJECTS.data[i].obj == NULL || OBJECTS.data[i].obj->functions[function_type] == NULL)
                        continue;
                OBJECTS.data[i].obj->functions[function_type](OBJECT_TO_PAYLOAD_PTR(OBJECTS.data[i].obj));
        }
}
