#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "unused.h"
#include "object.h"
#include "vec.h"
#include "render.h"

static const int WINDOW_WIDTH  = 1600;
static const int WINDOW_HEIGHT = 960;

static const int GAME_WIDTH  = 500;
static const int GAME_HEIGHT = 300;
static const float UNIT_SIZE = GAME_WIDTH / 50;

static int FPS = 120;

struct conveyor {
        struct conveyor *previous;
        struct conveyor *next;

        struct vec2f start;
        struct vec2f end;

        float interpolation_per_unit;
        float units_per_second;
};

struct item {
        struct conveyor *conveyor;
        float interpolation;
};

void conveyor_set_speed(struct conveyor *conveyor, float units_per_second)
{
        conveyor->units_per_second = units_per_second;

        const float length = vec2f_distance(conveyor->start, conveyor->end);
        const float units = length / UNIT_SIZE;
        conveyor->interpolation_per_unit = 1.0f / units;
}

void conveyor_render(void *conveyor_ptr)
{
        struct conveyor *conveyor = conveyor_ptr;
        render_draw_line(conveyor->start, conveyor->end, (struct color){0x20, 0xff, 0x00});

        if(conveyor->next != NULL && conveyor->next->previous != NULL) {
                conveyor_render(conveyor->next);
        }
}

void item_render(void *item_ptr)
{
        struct item *item = item_ptr;
        struct vec2f conveyor_vector = vec2f_subtract(item->conveyor->end, item->conveyor->start);
        struct vec2f pos = vec2f_add(item->conveyor->start, vec2f_scale(conveyor_vector, item->interpolation));
        const  struct vec2f size = {GAME_WIDTH / 64.0f, GAME_WIDTH / 64.0f};
        render_draw_rectangle(vec2f_subtract(pos, vec2f_scale(size, 0.5f)), (struct vec2f){5, 5}, (struct color){0xff, 0x20, 0x00});
}

void item_update(void *item_ptr)
{
        struct item *item = item_ptr;
        struct conveyor *conveyor = item->conveyor;
        item->interpolation += conveyor->interpolation_per_unit * conveyor->units_per_second * render_get_delta_time();
        if(item->interpolation >= 1.0f) {
                if(conveyor->next == NULL) {
                        item->interpolation = 1.0f;
                } else {
                        item->interpolation = 0.0f;
                        item->conveyor = item->conveyor->next;
                }
        }
}

struct item *item_create(struct conveyor *conveyor, float interpolation)
{
        struct item *item = object_allocate(sizeof(struct item));
        *item = (struct item){
                .conveyor = conveyor,
                .interpolation = interpolation
        };
        object_set_function(item, OBJ_FUNCTION_UPDATE, item_update);
        object_set_function(item, OBJ_FUNCTION_RENDER, item_render);
        return item;
}

struct conveyor **generate_conveyor_circle(size_t points, float radius)
{
        struct conveyor **convs = malloc(sizeof(void*) * points);
        struct vec2f center = {GAME_WIDTH / 2.0f, GAME_HEIGHT / 2.0f};
        for(size_t i = 0; i < points; ++i) {
                convs[i] = object_allocate(sizeof(**convs));
                object_set_function(convs[i], OBJ_FUNCTION_RENDER, conveyor_render);
        }

        for(size_t i = 0; i < points; ++i) {

                if(i == 0) {
                        convs[i]->previous = NULL;
                } else {
                        convs[i]->previous = convs[i - 1];
                }

                size_t connect_to_point = i + 1;
                if(i == points - 1) {
                        convs[i]->next = convs[0];
                        connect_to_point = 0;
                } else {
                        convs[i]->next = convs[i + 1];
                }

                float delta = (2.0f / points);
                convs[i]->start = vec2f_add(center, vec2f_scale((struct vec2f){cosf(delta * i * M_PI), sinf(delta * i * M_PI)}, radius));
                convs[i]->end   = vec2f_add(center, vec2f_scale((struct vec2f){cosf(delta * connect_to_point * M_PI), sinf(delta * connect_to_point * M_PI)}, radius));
        }

        return convs;
}

int main(int argc, char **argv)
{
        UNUSED(argc);
        UNUSED(argv);

        if(!render_initialize_system("factory", WINDOW_WIDTH, WINDOW_HEIGHT, GAME_WIDTH, GAME_HEIGHT, FPS)) {
                printf("Could not initialize render system\n");
        }

        object_initialize_system();

        size_t convs_size = 12;
        struct conveyor **convs = generate_conveyor_circle(convs_size, GAME_HEIGHT * 0.4);

        const float max_speed = 160;
        const float speed_per_point = max_speed / convs_size / 2.0f;
        for(size_t i = 0; i < convs_size / 2; ++i) {
                conveyor_set_speed(convs[i], (i + 1) * speed_per_point);
                conveyor_set_speed(convs[convs_size - i - 1], (i + 1) * speed_per_point);
        }

        size_t slices = 1;
        for(size_t i = 0; i < convs_size; ++i) {
                for(size_t j = 0; j < slices; ++j) {
                        UNUSED(item_create(convs[i], 1.0f / (slices + 1) * j));
                }
        }

        printf("GAME: Starting...\n");

        bool running = true;
        SDL_Event event;
        while(running) {
                while(SDL_PollEvent(&event)) {
                        if(event.type == SDL_EVENT_QUIT) {
                                running = false;
                                break;
                        }
                }

                render_clear_screen((struct color){0x20, 0x20, 0x20});

                object_call_on_all(OBJ_FUNCTION_UPDATE);
                object_call_on_all(OBJ_FUNCTION_RENDER);

                render_present();
        }

        //SDL_DestroyTexture(screen_texture);
        //SDL_DestroyRenderer(renderer);
        //SDL_DestroyWindow(window);
        //SDL_Quit();
        return 0;
}
