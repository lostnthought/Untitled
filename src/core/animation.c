
#include "core/engine_types.h"
#include "load/init.h"
#include "utilities/types.h"
#include <string.h>
#include "utilities/utilities.h"
#include "utilities/mathf.h"


float animation_length(Animation *anim) {
    if (!anim) return 0;
    float max_time = 0.0f;
    for (int i = 0; i < anim->sampler_count; i++) {
        AnimationSampler *s = &anim->samplers[i];
        if (s->count > 0) {
            float last_time = s->times[s->count - 1];
            if (last_time > max_time) max_time = last_time;
        }
    }
    return max_time;
}

void sample_animation(const AnimationSampler *sampler, float t, float *out_value) {
    if (sampler->count == 0) return;

    float duration = sampler->times[sampler->count - 1];
    if (duration <= 0.0f) {
        memcpy(out_value, sampler->values, sizeof(float) * sampler->elem_count);
        return;
    }
    t = fmodf(t, duration);

    int i = 0;
    while (i < sampler->count - 1 && t >= sampler->times[i + 1]) {
        i++;
    }

    if (i == sampler->count - 1) {
        memcpy(out_value, &sampler->values[i * sampler->elem_count],
               sizeof(float) * sampler->elem_count);
        return;
    }

    float t0 = sampler->times[i];
    float t1 = sampler->times[i + 1];
    float alpha = (t - t0) / (t1 - t0);

    const float *v0 = &sampler->values[i * sampler->elem_count];
    const float *v1 = &sampler->values[(i + 1) * sampler->elem_count];

    switch (sampler->interpolation) {
        case STEP:
            memcpy(out_value, v0, sizeof(float) * sampler->elem_count);
            break;

        case LINEAR:
            for (int j = 0; j < sampler->elem_count; j++) {
                out_value[j] = lerp(v0[j], v1[j], alpha);
            }
            break;

            // not handled
        case CUBICSPLINE:
            memcpy(out_value, v0, sizeof(float) * sampler->elem_count);
            break;
    }
}
void play_animation(Animation *anim, float time, int asset_instance) {
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < anim->channel_count; i++) {
        GLTFAnimationChannel *ch = &anim->channels[i];
        AnimationSampler *samp = &anim->samplers[ch->sampler];
        float tmp[4];
        sample_animation(samp, time, tmp);

        AssetInstance * AI = &ASSET_INSTANCES[asset_instance];
        
        MeshNode *node = &mns[AI->node_indices[ch->target_node]];
        switch (ch->path) {
            case TARGET_TRANSLATION:
                memcpy(&node->translation, tmp, sizeof(float)*3);
                break;
            case TARGET_ROTATION: memcpy(&node->rotation, tmp, sizeof(float)*4); break;
            case TARGET_SCALE: memcpy(&node->scale, tmp, sizeof(float)*3); break;
            case TARGET_WEIGHTS: break;
        }
    }
}


Animation * find_animation(const char * name, int asset_instance)
{
    AssetInstance * ai = &ASSET_INSTANCES[asset_instance];
    if (!ai) return NULL;
    Asset * as = CAST_VEC(GLOBALS.assets, Asset);
    Asset * a = &as[ai->asset_id];
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    for (int i = 0; i < ai->skin_count; i++)
    {
        
        GLTFSkin * skin = &skins[ai->skin_indices[i]];
        for (int j = 0; j < skin->animation_indices_count; j++)
        {
            if (strcmp(a->animations[skin->animation_indices[j]].name, name) == 0)
            {
                return &a->animations[skin->animation_indices[j]];
            }
        }

    }
    return NULL;
}


void update_animation_components(float dt)
{
    for (int i = 0; i < MAX_ANIMATION_COMPONENTS; i++)
    {
        AnimationComponent * ac = &ANIMATION_COMPONENTS[i];
        if (ac->free) continue;

        for (int j = 0; j < ac->track_count; j++)
        {


            AnimationTrack * t = &ac->tracks[j];
            if (!t->active) continue;
            t->current_time += dt;
            if (t->current_time >= t->duration)
            {
                if (t->loop)
                {
                    t->current_time = fmodf(t->current_time, t->duration);

                    if (t->completion_data == NULL) break;
                    switch(t->completion_type)
                    {
                        case ANIM_TARGET_FLOAT:
                            break;
                        case ANIM_TARGET_VEC3:
                            break;
                        case ANIM_TARGET_INT:
                            break;
                        case ANIM_TARGET_BOOL:
                            *((bool*)t->completion_data) = !(*(bool*)t->completion_data);
                            break;
                        default:
                            break;
                    }
                }
                else
                {
                    t->active = false;
                }
            }
            for (int k = 0; k < t->kf_count; k++)
            {
                AnimationKeyframe * kf2 = &t->kf[k];
                if (kf2->time < t->current_time) continue;
                
                AnimationKeyframe * kf = &t->kf[k > 0 ? k-1 : t->kf_count-1];
                float start_time = k > 0 ? kf->time : 0;
                float current_norm = t->current_time - start_time;
                float current_end = kf2->time - start_time;
                float current_time = current_norm / current_end;
                if (t->data == NULL) break;
                switch(t->type)
                {
                    case ANIM_TARGET_FLOAT:
                        *((float*)t->data) = lerp(kf->val.v, kf2->val.v, current_time);
                        break;
                    case ANIM_TARGET_VEC3:
                        *((vec3*)t->data) = vec3_lerp(kf->val.v3, kf2->val.v3, current_time);
                        break;
                    case ANIM_TARGET_INT:
                        *((int*)t->data) = (int)lerp(kf->val.i, kf2->val.i, current_time);
                        break;
                    case ANIM_TARGET_BOOL:
                        *((int*)t->data) = (int)lerp(kf->val.i, kf2->val.i, current_time);
                        break;
                    default:
                        break;
                }
                
                break;
            }
            

            
            
        }
        
        
    }


    
    
}
