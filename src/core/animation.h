
/**
 * @file animation.h
 * @brief Skeletal animation and animation component playback and management.
 */
#ifndef ANIMATION_H
#define ANIMATION_H

#include "engine_types.h"

/**
 * @brief Samples a value for an animation sampler at a certain time value.
 * @param sample The animation sampler to sample.
 * @param t The time to sample at.
 * @param out_value Pointer to the float that recieves the value.
 * @pre out_value must not be NULL.
 */
void sample_animation(const AnimationSampler * sampler, float t, float *out_value);

/**
 * @brief Plays an animation at a certain time value.
 * @param anim The animation to play.
 * @param time The time to play the animation at.
 * @param entity_id The entity that plays the animation.
 */
void play_animation(Animation * anim, float time, int entity_id);

/**
 * @brief Finds the length of an animation.
 * @param anim The animation to use.
 */
float animation_length(Animation * anim);

/**
 * @brief Finds an animation via name.
 * @param name The name of the animation.
 * @param entity_id The entity to query.
 */
Animation * find_animation(const char * name, int entity_id);

/**
 * @brief Updates animation components (not skeletal) once a frame.
 * @param dt The time to play all components at.
 */
void update_animation_components(float dt);



#endif
