
/**
 * @file audio.h
 * @brief Audio probe baking, portal creation, probe volumes, and emitter functions.
 */

#ifndef AUDIO_H
#define AUDIO_H

#include "core/engine_types.h"


/**
 * @brief Determines if a new IR needs to be signaled from the background thread.
 * @ingroup aprobe_group
 */
void update_player_audio_probe();

/**
 * @brief Raycasts from player to audio sources to determine occlusion and performs a star if necessary.
 * @param player The player
 * @ingroup audio_group
 */
void update_audio_occlusion(Player * player);

/**
 * @brief Finds the closest audio portal for a star to travel through.
 * @param pos The position to search from.
 * @returns Index of the closest portal.
 * @ingroup audio_group
 */
int find_closest_visible_aportal(vec3 pos);

/**
 * @brief Recalculates the adjacency map of portals between each other.
 * @ingroup audio_group
 */
void recalculate_audio_portal_paths();

/**
 * @brief Creates an audio portal by expanding the vector.
 * @param pos The position of the portal
 * @ingroup audio_group
 */
void init_new_audio_portal(vec3 pos);

/**
 * @brief Removes an audio portal from the vector.
 * @param id The id to remove.
 * @ingroup audio_group
 */
void remove_audio_portal(int id);

/**
 * @brief Initialized an audio emitter and sets up its audio object in miniaudio.
 * @param filepath The file to open.
 * @param is_loop If the sound is a loop.
 * @param is_occludable If the sound should be muffled or occluded through walls.
 * @param base_volume The overall volume of the sound.
 * @returns Pointer to the new emitter.
 * @ingroup audio_group
 */
AudioEmitter * init_emitter(const char * filepath, bool is_loop, bool is_occludable, float base_volume);

/**
 * @brief Loads an existing emitter into miniaudio.
 * @param emitter The emitter to load.
 * @param filepath The file to open.
 * @param is_loop If the sound is a loop.
 * @param is_occludable If the sound should be muffled or occluded through walls.
 * @param base_volume The overall volume of the sound.
 * @ingroup audio_group
 */
void load_emitter(AudioEmitter * emitter, const char* filepath, bool is_loop, bool is_occludable, float base_volume);

/**
 * @brief Starts playing an emitter.
 * @param emitter The emitter to play.
 * @param pos The position to play the emitter at.
 * @ingroup audio_group
 */
void play_emitter(AudioEmitter * emitter, vec3 pos);

/**
 * @brief Stops playing an emitter.
 * @param e The emitter to stop.
 * @ingroup audio_group
 */
void end_emitter(AudioEmitter * e);

/**
 * @brief Spatializes the audio from the player's position. Used after determining audio occlusion.
 * @param player The player to update audio for.
 * @ingroup audio_group
 */
void audio_update(Player * player);

/**
 * @brief Unloads an emitter from miniaudio.
 * @param id The emitter to unload.
 * @ingroup audio_group
 */
void unload_emitter(int id);

/**
 * @brief Unloads an emitter from miniaudio and frees the emitter as well.
 * @param id The emitter to destroy.
 * @ingroup audio_group
 */
void destroy_emitter(int id);


/**
 * @brief Bakes an audio probe volume.
 * @param pv The volume to bake.
 * @pre Must have an open APV file and a valid resolution for pv.
 * @ingroup aprobe_group
 */
void bake_audio_probe_volume(ProbeVolume * pv);

/**
 * @brief Passes frames to miniaudio in the audio thread.
 * @see init.c.
 * @ingroup aprobe_group
 */
void SDLAudioCallback(void* pUserData, Uint8* pStream, int frameCountInBytes);

/**
 * @brief Queries which audio probe is closest to given position.
 * @param pv The volume to check.
 * @param pos The position to check.
 * @param out_local Pointer to integer index of returned probe in xyz.
 * @returns Probe index.
 * @ingroup aprobe_group
 */
int query_audio_probe(ProbeVolume * pv, vec3 pos, ivec3 * out_local);



/**
 * @brief Initializes kernels used for IR sample filtering.
 * @param sample_rate The sample rate of audio.
 * @ingroup aprobe_group
 */
void init_band_kernels(float sample_rate, float f1_hz, float f2_hz, float f3_hz,  float f4_hz, float f5_hz, float out_kernels[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE]);


// TODO pass player as argument.
/**
 * @brief Finds the closest audio PVs to the player and loads the closest ones into open slots based on distance.
 * @ingroup aprobe_group
 */
void check_audio_pv();

/**
 * @brief Reset audio pvs, frees each slot in the global audio pv slots.
 * @ingroup aprobe_group
 */
void reset_audio_pvs();


#endif
