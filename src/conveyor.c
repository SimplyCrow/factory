#include "conveyor.h"

#include <stdio.h>

#include "vec.h"
#include "object.h"
#include "render.h"
#include "unused.h"
#include "constants.h"

#define LOG_TITLE "Conveyor"
#include "log.h"

static size_t conveyor_count_size(struct conveyor *conveyor)
{
        struct cluster *head = conveyor->head;
        struct cluster *tail = conveyor->tail;
        size_t count = 1;
        while(head != tail) {
                ++count;
                head = head->next;
        }
        return count;
}


static inline float cluster_item_absolute_distance(struct cluster *cluster, struct cluster_item *item)
{
        if(item == cluster->head)
                return item->distance;
        return cluster->head->distance - (item->distance - cluster->head->base_offset);
}

static size_t passive_count = 0;
static void conveyor_update(void *self)
{
        struct conveyor *conveyor = self;

        const float velocity = conveyor->items_per_minute * ITEM_SIZE / 60.0f;

        if(conveyor->head == NULL)
                return;
        for(struct cluster *cluster = conveyor->head; cluster != NULL; cluster = cluster->next) {
                if(conveyor_cluster_distance_to_next(conveyor, cluster) <= 0.00) {
                        if(cluster->is_passiv)
                                continue;
                        cluster->is_passiv = true;
                        ++passive_count;
                        if(cluster->head == cluster->tail)
                                continue;
                        const float offset_split   = cluster->head->next->distance;
                        const float distance_split = cluster_item_absolute_distance(cluster, cluster->head->next);
                        struct cluster *split_cluster = cluster_create();
                        split_cluster->head = cluster->head->next;
                        split_cluster->head->previous    = NULL;
                        split_cluster->head->base_offset = offset_split;
                        split_cluster->head->distance    = distance_split;
                        split_cluster->tail     = cluster->tail;

                        cluster->tail = cluster->head;
                        cluster->head->next     = NULL;
                        cluster->head->previous = NULL;
                        cluster->tail->next     = NULL;
                        cluster->tail->previous = NULL;

                        split_cluster->previous = cluster;
                        split_cluster->next     = cluster->next;

                        if(cluster->next)
                                cluster->next->previous = split_cluster;

                        cluster->next = split_cluster;

                        if(cluster == conveyor->tail)
                                conveyor->tail = split_cluster;

                        LOGF("Split passive cluster (cluster count: %zu)", conveyor_count_size(conveyor));
                        continue;
                }
                if(cluster->is_passiv)
                        --passive_count;
                cluster->is_passiv = false;
                cluster->head->distance += velocity * render_get_delta_time();
        }
        //LOGF("Passive: %zu", passive_count);
}

static void conveyor_render(void *self)
{
        struct conveyor *conveyor = self;

        render_draw_line(vec2f_world_to_pixel(conveyor->start), vec2f_world_to_pixel(conveyor->end), (struct color){0x20, 0xff, 0x00});

        if(conveyor->head == NULL)
                return;

        size_t cluster_idx = 0;
        for(struct cluster *cluster = conveyor->head; cluster != NULL; cluster = cluster->next) {
                if(cluster->head == NULL)
                        continue;

                size_t item_idx = 0;
                const struct vec2f conveyor_direction = vec2f_direction(conveyor->start, conveyor->end);
                const struct vec2f size = {ITEM_SIZE * 0.8, ITEM_SIZE * 0.8};
                for(struct cluster_item *item = cluster->head; item != NULL; item = item->next)
                {
                        //LOGF("C%zu I%zu", cluster_idx, item_idx);
                        UNUSED(cluster_idx); UNUSED(item_idx);
                        float item_distance = cluster_item_absolute_distance(cluster, item);
                        const struct vec2f item_position = vec2f_add(conveyor->start, vec2f_scale(conveyor_direction, item_distance));
                        render_draw_rectangle(vec2f_world_to_pixel(vec2f_subtract(item_position, (struct vec2f){size.x, 0.5f * size.y})), vec2f_world_to_pixel(size), (struct color){0xff, 0x20, 0x00});
                        ++item_idx;
                }
                ++cluster_idx;
        }
}

float conveyor_cluster_distance_to_next(struct conveyor *conveyor, struct cluster *cluster)
{
        if(cluster == conveyor->head) {
                const float conveyor_length = vec2f_length(vec2f_subtract(conveyor->start, conveyor->end));
                return conveyor_length - cluster_item_absolute_distance(cluster, cluster->head);
        }

        return cluster_item_absolute_distance(cluster->previous, cluster->previous->tail)
                - cluster_item_absolute_distance(cluster, cluster->head) - ITEM_SIZE;
}

struct cluster *conveyor_append_cluster(struct conveyor *conveyor)
{
        struct cluster *cluster = cluster_create();
        if(conveyor->head == NULL) {
                cluster->previous = NULL;
                cluster->next = NULL;
                conveyor->head = cluster;
                conveyor->tail = cluster;
                LOGF("Added cluster to conveyor. (cluster count: %zu)", conveyor_count_size(conveyor));
                return cluster;
        }

        conveyor->tail->next = cluster;
        cluster->previous = conveyor->tail;
        conveyor->tail = cluster;
        LOGF("Added cluster to conveyor. (cluster count: %zu)", conveyor_count_size(conveyor));
        return cluster;
}

struct cluster_item *conveyor_append_item(struct conveyor *conveyor)
{
        LOGF("Added item to conveyor. (cluster count: %zu)", conveyor_count_size(conveyor));
        float distance = 0.00f;
        if(conveyor->tail && conveyor->tail->head)
                distance = cluster_item_absolute_distance(conveyor->tail, conveyor->tail->head);
        if(conveyor->tail->is_passiv) {
                struct cluster *tail = conveyor_append_cluster(conveyor);
                LOGF("Encountered passive tail cluster. New tail constructed. (cluster count: %zu)", conveyor_count_size(conveyor));
                return cluster_append_item(tail, 0.00f);
        }
        return cluster_append_item(conveyor->tail, distance);
}

struct conveyor *conveyor_create(struct vec2f start, struct vec2f end, int items_per_minute)
{
        LOG("Created conveyor.");
        struct conveyor *conveyor = object_allocate(sizeof(struct conveyor));
        conveyor->start = start;
        conveyor->end   = end;
        conveyor->items_per_minute = items_per_minute;
        conveyor_append_cluster(conveyor);
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
        cluster->next = NULL;
        cluster->previous = NULL;
        cluster->is_passiv = false;
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
