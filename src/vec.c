#include "vec.h"

#include <math.h>

#include "constants.h"

float vec2f_dot(struct vec2f a, struct vec2f b)
{
        return a.x * b.x + a.y * b.y;
}

struct vec2f vec2f_add(struct vec2f a, struct vec2f b)
{
        return (struct vec2f){a.x + b.x, a.y + b.y};
}

struct vec2f vec2f_subtract(struct vec2f a, struct vec2f b)
{
        return (struct vec2f){a.x - b.x, a.y - b.y};
}

struct vec2f vec2f_scale(struct vec2f a, float scale)
{
        return (struct vec2f){a.x * scale, a.y * scale};
}

float vec2f_length(struct vec2f a)
{
        return sqrt(vec2f_dot(a, a));
}

float vec2f_distance(struct vec2f a, struct vec2f b)
{
        return vec2f_length(vec2f_subtract(b, a));
}

struct vec2f vec2f_normalize(struct vec2f a)
{
        return vec2f_scale(a, 1.0f / vec2f_length(a));
}

struct vec2f vec2f_direction(struct vec2f base, struct vec2f direction)
{
        return vec2f_normalize(vec2f_subtract(direction, base));
}

struct vec2f vec2f_world_to_pixel(struct vec2f vec)
{
        return vec2f_scale(vec, METER_IN_PIXEL);
}

struct vec2f vec2f_pixel_to_world(struct vec2f vec)
{
        return vec2f_scale(vec, 1.0 / METER_IN_PIXEL);
}
