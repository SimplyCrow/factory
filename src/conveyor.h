#ifndef CONVEYOR_H
#define CONVEYOR_H

#include <stdbool.h>

#include "vec.h"

struct conveyor;
struct cluster_item;
struct cluster;

struct conveyor {
        struct vec2f start;
        struct vec2f end;

        int items_per_minute;

        struct cluster *head;
        struct cluster *tail;
};

struct cluster_item {
        float distance;
        float base_offset;
        struct cluster_item *previous;
        struct cluster_item *next;
};

struct cluster {
        struct cluster_item *head;
        struct cluster_item *tail;
        struct cluster *next;
        struct cluster *previous;
        bool is_passiv;
};

struct conveyor     *conveyor_create(struct vec2f start, struct vec2f end, int items_per_minute);
struct cluster_item *conveyor_append_item(struct conveyor *conveyor);
float conveyor_cluster_distance_to_next(struct conveyor *conveyor, struct cluster *cluster);
struct cluster_item *cluster_item_create(float distance);
struct cluster      *cluster_create();
struct cluster_item *cluster_append_item(struct cluster *cluster, float distance);

#endif // CONVEYOR_H
