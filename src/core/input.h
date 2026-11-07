
/**
 * @file input.h
 * @brief Receives input events from SDL and processes them.
 */

#ifndef INPUT_H
#define INPUT_H


#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>


/**
 * @brief Receives input from SDL in a while loop, pushes th events onto queues to be consumed at the end of the rendering frame. Also processes keyboard events.
 * @param window The SDL_Window object
 * @param event_queue The queue for SDL_Event%s that are not keyboard events that must be consumed once every render frame. For Nuklear.
 * @param event_queue_count The queue count for SDL.
 * @param sdl_queue The queue for SDL_Event%s that are not keyboard events that must be consumed once every render frame.
 * @param physics_dt How much the cooldown for buttons must decrease (avoids double inputs).
 */
void
    process_input
    ( SDL_Window **window, SDL_Event * event_queue, int * event_queue_count, SDL_Event * sdl_queue, int * sdl_queue_count, float physics_dt);
    
/**
 * @brief Processes a deferred SDL_Event after the physics loop accumulator.
 * @param event The event to process.
 */
void process_sdl_event(SDL_Event event);
/**
 * @brief Processes UI events for persistent state if the LMB has not been released.
 */
void handle_held_events();



#endif
