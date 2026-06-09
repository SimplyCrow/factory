#ifndef VEC_H
#define VEC_H

struct vec2f {
        float x;
        float y;
};

float vec2f_dot(struct vec2f a, struct vec2f b);
float vec2f_length(struct vec2f a);
float vec2f_distance(struct vec2f a, struct vec2f b);
struct vec2f vec2f_add(struct vec2f a, struct vec2f b);
struct vec2f vec2f_subtract(struct vec2f a, struct vec2f b);
struct vec2f vec2f_scale(struct vec2f a, float scale);
struct vec2f vec2f_normalize(struct vec2f a);
struct vec2f vec2f_direction(struct vec2f base, struct vec2f direction);
struct vec2f vec2f_world_to_pixel(struct vec2f vec);
struct vec2f vec2f_pixel_to_world(struct vec2f vec);

#endif // VEC_H
