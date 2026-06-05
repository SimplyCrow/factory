#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "unused.h"
#include "object.h"

static const int WINDOW_WIDTH  = 1600;
static const int WINDOW_HEIGHT = 960;

static const int GAME_WIDTH  = 500;
static const int GAME_HEIGHT = 300;
static const float UNIT_SIZE = GAME_WIDTH / 50;

static int FPS = 120;

static uint32_t *PIXEL_BUFFER = NULL;

struct vec2f {
        float x;
        float y;
};

struct color {
        uint8_t r;
        uint8_t g;
        uint8_t b;
};

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

void set_pixel(struct vec2f pos, struct color color)
{
        const int x = (int)roundf(pos.x);
        const int y = GAME_HEIGHT - (int)roundf(pos.y) - 1;
        if(x >= 0 && x < GAME_WIDTH && y >= 0 && y < GAME_HEIGHT) {
                PIXEL_BUFFER[(int)y * GAME_WIDTH + (int)x] = (color.r << 16) | (color.g << 8) | color.b;
        }
}

void clear_screen(struct color color)
{
        for(int y = 0; y < GAME_HEIGHT; ++y) {
                for(int x = 0; x < GAME_WIDTH; ++x) {
                        set_pixel((struct vec2f){x, y}, color);
                }
        }
}

void draw_rectangle(struct vec2f pos, struct vec2f size, struct color color)
{
        for(int i = 0; i < (int)size.x; ++i) {
                for(int j = 0; j < (int)size.y; ++j) {
                        set_pixel((struct vec2f){pos.x + j, pos.y + i}, color);
                }
        }
}

// ORIGINAL FROM: https://gist.github.com/bert/1085538#file-plot_line-c
void draw_line(struct vec2f a, struct vec2f b, struct color color)
{
        int x0 = a.x, y0 = a.y;
        int x1 = b.x, y1 = b.y;
        int dx = abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while(!(x0 == x1 && y0 == y1)) {
                set_pixel((struct vec2f){x0,y0}, color);
                e2 = 2 * err;
                if (e2 >= dy) {
                        err += dy;
                        x0 += sx;
                }
                if (e2 <= dx) {
                        err += dx;
                        y0 += sy;
                }
        }

        set_pixel((struct vec2f){x0,y0}, color);
}

uint64_t get_current_milliseconds()
{
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

static double DELTA_TIME = 0;
double get_delta_time()
{
        return DELTA_TIME;
}

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
        draw_line(conveyor->start, conveyor->end, (struct color){0x20, 0xff, 0x00});

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
        draw_rectangle(vec2f_subtract(pos, vec2f_scale(size, 0.5f)), (struct vec2f){5, 5}, (struct color){0xff, 0x20, 0x00});
}

void item_update(void *item_ptr)
{
        struct item *item = item_ptr;
        struct conveyor *conveyor = item->conveyor;
        item->interpolation += conveyor->interpolation_per_unit * conveyor->units_per_second * get_delta_time();
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

        if(!SDL_Init(SDL_INIT_VIDEO))
                return -1;

        SDL_Window *window   = NULL;
        SDL_Renderer *renderer = NULL;
        if(!SDL_CreateWindowAndRenderer("Factory", WINDOW_WIDTH, WINDOW_HEIGHT, 0, &window, &renderer)) {
                SDL_Quit();
                return -1;
        }

        PIXEL_BUFFER = malloc(sizeof(PIXEL_BUFFER[0]) * GAME_WIDTH * GAME_HEIGHT);
        if(PIXEL_BUFFER == NULL) {
                SDL_Quit();
                return -1;
        }

        SDL_Texture *screen_texture = SDL_CreateTexture(
                        renderer,
                        SDL_PIXELFORMAT_XRGB8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        GAME_WIDTH, GAME_HEIGHT
        );

        SDL_SetTextureScaleMode(screen_texture, SDL_SCALEMODE_NEAREST);

        SDL_SetRenderDrawColor(renderer, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(renderer);

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
        uint64_t last_milliseconds = get_current_milliseconds();
        SDL_Event event;
        while(running) {
                while(SDL_PollEvent(&event)) {
                        if(event.type == SDL_EVENT_QUIT) {
                                running = false;
                                break;
                        }
                }

                clear_screen((struct color){0x20, 0x20, 0x20});

                object_call_on_all(OBJ_FUNCTION_UPDATE);
                object_call_on_all(OBJ_FUNCTION_RENDER);

                SDL_UpdateTexture(screen_texture, NULL, PIXEL_BUFFER, GAME_WIDTH * sizeof(PIXEL_BUFFER[0]));

                SDL_RenderTexture(renderer, screen_texture, NULL, NULL);
                SDL_RenderPresent(renderer);

                uint64_t current_milliseconds = get_current_milliseconds();
                DELTA_TIME = (current_milliseconds - last_milliseconds) / 1000.0;
                last_milliseconds = current_milliseconds;

                float delay_time = 1000 / FPS - DELTA_TIME;
                if(delay_time > 0) {
                        SDL_Delay(delay_time);
                }
        }

        SDL_DestroyTexture(screen_texture);
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
}
