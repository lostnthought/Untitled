#ifndef AUDIO_IR_H
#define AUDIO_IR_H


#include "load/init.h"
#include "utilities/types.h"

void rotate_sh(float * OUT, float SH[SH_MAX], mat3 rot);
void fft_channels(ConvolverNode * node, int count, bool crossfading);
void load_random_scattering_sofa(const char * filename);
void ambisonic_matrix_construction(const char * filename);
void evaluate_sh(vec3 dir, float SH[SH_MAX]);
void load_hrtf_from_binary(const char * filename);


void update_player_audio_probe();
void init_hrtf_worker_thread(HRTFThreadData * td);
void delete_hrtf_worker_thread(HRTFThreadData * td);


#endif
