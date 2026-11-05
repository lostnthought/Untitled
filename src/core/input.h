
#ifndef INPUT_H
#define INPUT_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>


void
    process_input
    ( SDL_Window **window, SDL_Event * event_queue, int * event_queue_count, SDL_Event * sdl_queue, int * sdl_queue_count, float physics_dt);
    
void process_sdl_event(SDL_Event event);
void handle_held_events();



#endif
