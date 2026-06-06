#ifndef RENDER_H
#define RENDER_H

#include <stdint.h>

#include "vec.h"

struct color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
};

double render_get_delta_time();
bool render_initialize_system(const char *title, int window_width, int window_height, int game_width, int game_height, int fps);
void render_present();
void render_set_pixel(struct vec2f pos, struct color color);
void render_clear_screen(struct color color);
void render_draw_rectangle(struct vec2f pos, struct vec2f size, struct color color);
void render_draw_line(struct vec2f a, struct vec2f b, struct color color);

#endif // RENDER_H
