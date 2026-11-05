#ifndef ANIMATION_H
#define ANIMATION_H

#include "engine_types.h"



void sample_animation(const AnimationSampler *sampler, float t, float *out_value);

void play_animation(Animation *anim, float time, int entity_id);
float animation_length(Animation *anim);
Animation * find_animation(const char * name, int entity_id);
void update_animation_components(float dt);



#endif
