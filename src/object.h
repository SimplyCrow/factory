#ifndef OBJECT_H
#define OBJECT_H

#include <stddef.h>

enum OBJECT_FUNCTIONS {
        OBJ_FUNCTION_START = 0,
        OBJ_FUNCTION_UPDATE,
        OBJ_FUNCTION_RENDER,
        OBJ_FUNCTION_FREE,
        OBJ_FUNCTION_COUNT,
        OBJ_FUNCTION_INVALID = 255
};

void object_initialize_system();
void *object_allocate(size_t object_size);
void *object_allocate_array(size_t object_count, size_t object_size);
void *object_allocate_and_start(size_t object_size, void (*start_func)(void *self));
void object_free(void *object);
bool object_set_function(void *object, enum OBJECT_FUNCTIONS function_type, void (*func)(void *self));
void object_call_on_object(void *object, enum OBJECT_FUNCTIONS function_type);
void object_call_on_all(enum OBJECT_FUNCTIONS function_type);

#endif // OBJECT_H
