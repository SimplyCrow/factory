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
#include "conveyor.h"
#include "constants.h"

#define LOG_TITLE "Main"
#include "log.h"

const int   WINDOW_WIDTH        = 1600;
const int   WINDOW_HEIGHT       = 960;
const int   GAME_WIDTH          = 500;
const int   GAME_HEIGHT         = 300;
const int   FPS                 = 120;
const float METER_IN_PIXEL      = GAME_WIDTH / 50;
const float ITEM_SIZE           = 1;

int main(int argc, char **argv)
{
        UNUSED(argc);
        UNUSED(argv);
        UNUSED(ITEM_SIZE);

        if(!render_initialize_system("factory", WINDOW_WIDTH, WINDOW_HEIGHT, GAME_WIDTH, GAME_HEIGHT, FPS)) {
                printf("Could not initialize render system\n");
        }

        object_initialize_system();

        struct conveyor *conveyor = conveyor_create((struct vec2f){10, GAME_HEIGHT / 2.0}, (struct vec2f){GAME_WIDTH - 10, GAME_HEIGHT / 2.0}, 60);
        conveyor_append_item(conveyor);

        printf("GAME: Starting...\n");

        bool running = true;
        bool space_pulling = false;
        uint64_t fps_counter = 0;
        double   fps_timer = 0.00;
        SDL_Event event;
        while(running) {
                while(SDL_PollEvent(&event)) {
                        if(event.type == SDL_EVENT_QUIT) {
                                running = false;
                                break;
                        }
                        if(event.type == SDL_EVENT_KEY_DOWN) {
                                switch(event.key.key) {
                                        case SDLK_ESCAPE:
                                                running = false;
                                                break;
                                        case SDLK_SPACE:
                                                if(space_pulling)
                                                        break;
                                                conveyor_append_item(conveyor);
                                                space_pulling = true;
                                                break;
                                }
                                if(running == false)
                                        break;
                        }
                        if(event.type == SDL_EVENT_KEY_UP) {
                                switch(event.key.key) {
                                        case SDLK_SPACE:
                                                if(!space_pulling)
                                                        break;
                                                space_pulling = false;
                                                break;
                                }
                        }
                }

                render_clear_screen((struct color){0x20, 0x20, 0x20});

                object_call_on_all(OBJ_FUNCTION_UPDATE);
                object_call_on_all(OBJ_FUNCTION_RENDER);

                ++fps_counter;
                fps_timer += render_get_delta_time();
                if(fps_timer >= 3.00) {
                        LOGF("FPS: %lf", fps_counter / fps_timer);
                        fps_timer = 0.00;
                        fps_counter = 0;
                }

                render_present();
        }

        return 0;
}
