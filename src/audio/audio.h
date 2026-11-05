#ifndef AUDIO_H
#define AUDIO_H

#include "core/engine_types.h"


void update_audio_occlusion(Player * player);
int find_closest_visible_aportal(vec3 pos);
void recalculate_audio_portal_paths();
void init_new_audio_portal(vec3 pos);
void remove_audio_portal(int id);
AudioEmitter * init_emitter(const char * filepath, bool is_loop, bool is_occludable, float base_volume);
void load_emitter(AudioEmitter * emitter, const char* filepath, bool is_loop, bool is_occludable, float base_volume);
void play_emitter(AudioEmitter * emitter, vec3 pos);
void audio_update(Player * player);
void unload_emitter(int id);
void destroy_emitter(int id);
void populate_audio_probes();
void bake_audio_probes();
void bake_audio_probe_volume(ProbeVolume * pv);
void end_emitter(AudioEmitter * e);

void SDLAudioCallback(void* pUserData, Uint8* pStream, int frameCountInBytes);
int query_audio_probe(ProbeVolume * pv, vec3 pos, ivec3 * out_local);
void process_probe_chunk(int start, int count, GLuint aprobe_ssbo);
void init_band_kernels(float sample_rate,  float f1_hz,  float f2_hz,  float f3_hz,  float f4_hz, float f5_hz, float out_kernels[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE]);
void check_audio_pv();
void reset_audio_pvs();

#endif
