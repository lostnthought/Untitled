/**
 * @file audio_ir.h
 * @brief Audio IR processing, hrtf loading and parsing, and audio threading.
 * @see init.c for the audio thread convolution.
 */

#ifndef AUDIO_IR_H
#define AUDIO_IR_H

#include "core/engine_types.h"

/**
 * @brief Rotates the SH.
 * @param OUT The array of rotated SH.
 * @param SH The input SH.
 * @param rot The rotation matrix.
 * @author John Hable.
 */
void rotate_sh(float * OUT, float SH[SH_MAX], mat3 rot);

/**
 * @brief FFTs the newest audio chunk with the partitioned overlap-add buffer.
 * @param node The convolver node that is processing the audio.
 * @param crossfading If we are currently crossfading into a new IR this frame.
 */
void fft_channels(ConvolverNode * node, bool crossfading);

/**
 * @brief Evaluates 9 spherical harmonics based on direction.
 * @param dir The direction to evaluate.
 * @param SH The output SH array.
 */
void evaluate_sh(vec3 dir, float SH[SH_MAX]);

/**
 * @brief Loads a stored HRTF from engine binary.
 * @param filename The file to load.
 */
void load_hrtf_from_binary(const char * filename);

/**
 * @brief Determines if a new IR needs to be signaled from the background thread.
 */
void update_player_audio_probe();

/**
 * @brief Initializes the worker thread that convolves the audio probes with the HRTF.
 * @param td The worker thread data.
 */
void init_hrtf_worker_thread(HRTFThreadData * td);

/**
 * @brief Cleans up the worker thread that convolves the audio probes with the HRTF.
 * @param td The worker thread data.
 */
void delete_hrtf_worker_thread(HRTFThreadData * td);


#endif
