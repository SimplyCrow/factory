#include "conveyor.h"

#include <stdio.h>

#include "vec.h"
#include "object.h"
#include "render.h"
#include "unused.h"
#include "constants.h"

#define LOG_TITLE "Conveyor"
#include "log.h"

static void conveyor_update(void *self)
{
        struct conveyor *conveyor = self;
        struct cluster *cluster = conveyor->cluster;

        const float velocity = conveyor->items_per_minute * ITEM_SIZE / 60.0f;

        if(cluster == NULL || cluster->head == NULL)
                return;

        cluster->head->distance += velocity * render_get_delta_time();
}

static void conveyor_render(void *self)
{
        struct conveyor *conveyor = self;
        struct cluster *cluster = conveyor->cluster;

        render_draw_line(conveyor->start, conveyor->end, (struct color){0x20, 0xff, 0x00});

        if(cluster == NULL || cluster->head == NULL)
                return;

        const struct vec2f conveyor_direction = vec2f_direction(conveyor->start, conveyor->end);
        const struct vec2f size = {GAME_WIDTH / 50.0f, GAME_WIDTH / 50.0f};
        const float head_distance = cluster->head->distance;
        for(struct cluster_item *item = cluster->head; item != NULL; item = item->next)
        {
                float item_distance = head_distance;
                if(item != cluster->head)
                        item_distance -= item->distance;

                const struct vec2f item_position = vec2f_add(conveyor->start, vec2f_scale(conveyor_direction, item_distance * METER_IN_PIXEL));
                render_draw_rectangle(vec2f_subtract(item_position, vec2f_scale(size, 0.5f)), size, (struct color){0xff, 0x20, 0x00});
        }
}

struct cluster_item *conveyor_append_item(struct conveyor *conveyor)
{
        LOG("Added item to conveyor.");
        float distance = 0.00f;
        if(conveyor->cluster && conveyor->cluster->head)
                distance = conveyor->cluster->head->distance;
        return cluster_append_item(conveyor->cluster, distance);
}

struct conveyor *conveyor_create(struct vec2f start, struct vec2f end, int items_per_minute)
{
        LOG("Created conveyor.");
        struct conveyor *conveyor = object_allocate(sizeof(struct conveyor));
        conveyor->start = start;
        conveyor->end   = end;
        conveyor->cluster = cluster_create();
        conveyor->items_per_minute = items_per_minute;
        object_set_function(conveyor, OBJ_FUNCTION_UPDATE, conveyor_update);
        object_set_function(conveyor, OBJ_FUNCTION_RENDER, conveyor_render);
        return conveyor;
}

struct cluster_item *cluster_item_create(float distance)
{
        struct cluster_item *item = object_allocate(sizeof(struct cluster_item));
        item->distance = distance;
        item->next = NULL;
        item->previous = NULL;
        return item;
}

struct cluster *cluster_create()
{
        struct cluster *cluster = object_allocate(sizeof(struct cluster));
        cluster->head = NULL;
        cluster->tail = NULL;
        return cluster;
}

struct cluster_item *cluster_append_item(struct cluster *cluster, float distance)
{
        struct cluster_item *item = cluster_item_create(distance);
        if(cluster->head == NULL) {
                item->previous = NULL;
                item->next = NULL;
                cluster->head = item;
                cluster->tail = item;
                return item;
        }

        cluster->tail->next = item;
        item->previous = cluster->tail;
        cluster->tail = item;
        return item;
}
