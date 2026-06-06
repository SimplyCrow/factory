#include "render.h"

#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <math.h>
#include <stdio.h>
#include <time.h>

#include <SDL3/SDL.h>

#include "vec.h"

#define LOG(fmt)        printf("Render: "fmt"\n")
#define LOGF(fmt, ...)  printf("Render: "fmt"\n", __VA_ARGS__)

static int WINDOW_WIDTH;
static int WINDOW_HEIGHT;

static int GAME_WIDTH;
static int GAME_HEIGHT;

static uint32_t *PIXEL_BUFFER = NULL;

static SDL_Window   *WINDOW;
static SDL_Renderer *RENDERER;
static SDL_Texture  *SCREEN_TEXTURE;

static int FPS;
static uint64_t LAST_MILLISECONDS;
static double DELTA_TIME;

static uint64_t get_current_milliseconds()
{
        struct timespec ts;
        clock_gettime(CLOCK_MONOTONIC, &ts);
        return (uint64_t)ts.tv_sec * 1000 + (uint64_t)ts.tv_nsec / 1000000;
}

double render_get_delta_time()
{
        return DELTA_TIME;
}

bool render_initialize_system(const char *title, int window_width, int window_height, int game_width, int game_height, int fps)
{
        LOGF("Initializing render system; title: \"%s\"", title);
        LOGF("WWIDTH: %d WHEIGHT: %d GWIDTH: %d GHEIGHT: %d", window_width, window_height, game_width, game_height);
        WINDOW_WIDTH  = window_width;
        WINDOW_HEIGHT = window_height;
        GAME_WIDTH    = game_width;
        GAME_HEIGHT   = game_height;
        FPS           = fps;

        if(!SDL_Init(SDL_INIT_VIDEO))
                return false;

        if(!SDL_CreateWindowAndRenderer(title, WINDOW_WIDTH, WINDOW_HEIGHT, 0, &WINDOW, &RENDERER)) {
                SDL_Quit();
                return false;
        }

        PIXEL_BUFFER = malloc(sizeof(PIXEL_BUFFER[0]) * GAME_WIDTH * GAME_HEIGHT);
        if(PIXEL_BUFFER == NULL) {
                SDL_Quit();
                return false;
        }

        SCREEN_TEXTURE = SDL_CreateTexture(
                        RENDERER,
                        SDL_PIXELFORMAT_XRGB8888,
                        SDL_TEXTUREACCESS_STREAMING,
                        GAME_WIDTH, GAME_HEIGHT
        );

        SDL_SetTextureScaleMode(SCREEN_TEXTURE, SDL_SCALEMODE_NEAREST);

        SDL_SetRenderDrawColor(RENDERER, 0x00, 0x00, 0x00, 0xff);
        SDL_RenderClear(RENDERER);

        render_clear_screen((struct color){0x00, 0x00, 0x00});

        LAST_MILLISECONDS = get_current_milliseconds();

        LOG("Initialized render system");
        return true;
}

void render_present()
{
        SDL_UpdateTexture(SCREEN_TEXTURE, NULL, PIXEL_BUFFER, GAME_WIDTH * sizeof(PIXEL_BUFFER[0]));
        SDL_RenderTexture(RENDERER, SCREEN_TEXTURE, NULL, NULL);
        SDL_RenderPresent(RENDERER);

        uint64_t current_milliseconds = get_current_milliseconds();
        DELTA_TIME = (current_milliseconds - LAST_MILLISECONDS) / 1000.0;
        LAST_MILLISECONDS = current_milliseconds;

        float delay_time = 1000 / FPS - DELTA_TIME;
        if(delay_time > 0) {
                SDL_Delay(delay_time);
        }
}

void render_set_pixel(struct vec2f pos, struct color color)
{
        const int x = (int)roundf(pos.x);
        const int y = GAME_HEIGHT - (int)roundf(pos.y) - 1;
        if(x >= 0 && x < GAME_WIDTH && y >= 0 && y < GAME_HEIGHT) {
                PIXEL_BUFFER[(int)y * GAME_WIDTH + (int)x] = (color.r << 16) | (color.g << 8) | color.b;
        }
}

void render_clear_screen(struct color color)
{
        for(int y = 0; y < GAME_HEIGHT; ++y) {
                for(int x = 0; x < GAME_WIDTH; ++x) {
                        render_set_pixel((struct vec2f){x, y}, color);
                }
        }
}

void render_draw_rectangle(struct vec2f pos, struct vec2f size, struct color color)
{
        for(int i = 0; i < (int)size.x; ++i) {
                for(int j = 0; j < (int)size.y; ++j) {
                        render_set_pixel((struct vec2f){pos.x + j, pos.y + i}, color);
                }
        }
}

// ORIGINAL FROM: https://gist.github.com/bert/1085538#file-plot_line-c
void render_draw_line(struct vec2f a, struct vec2f b, struct color color)
{
        int x0 = a.x, y0 = a.y;
        int x1 = b.x, y1 = b.y;
        int dx = abs (x1 - x0), sx = x0 < x1 ? 1 : -1;
        int dy = -abs (y1 - y0), sy = y0 < y1 ? 1 : -1;
        int err = dx + dy, e2;

        while(!(x0 == x1 && y0 == y1)) {
                render_set_pixel((struct vec2f){x0,y0}, color);
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

        render_set_pixel((struct vec2f){x0,y0}, color);
}
