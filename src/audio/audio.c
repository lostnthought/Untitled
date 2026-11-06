#include "audio/audio.h"
#include "SDL2/SDL_mutex.h"
#include "SDL2/SDL_thread.h"
#include "core/camera.h"
#include "load/init.h"
#include "utilities/mathf.h"
#include "miniaudio/miniaudio.h"
#include "render/render.h"
#include <string.h>
#include "core/engine_types.h"
#include "core/acceleration.h"
#include "utilities/engine_utilities.h"
// #include "./libmysofa-1.3.5/src/hrtf/mysofa.h"
#include "fftw3.h"
#include <math.h>
#include "audio/audio_ir.h"
#include "utilities/utilities.h"
#include <stdatomic.h>
#include <stddef.h>
#include <stdio.h>


float BAND_IMPULSES[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE];



void recalculate_audio_portal_paths()
{

    AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    {
        
        AudioPortal * p = &portals[i];

        if (p->adjacent_portals.buffer) free(p->adjacent_portals.buffer);
        init_vector(&p->adjacent_portals, 0, sizeof(int));
        
    }

    for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    {

        AudioPortal * p0 = &portals[i];
        

        for (int j = i+1; j < GLOBALS.audio_portals.count; j++)
        {
            AudioPortal * p1 = &portals[j];

            vec3 diff = vec3_sub(p1->pos, p0->pos);
            float dist = vec3_length(diff);
            vec3 dir = vec3_scale(diff, 1.0f/dist);
            RayHit hit = raycast_scene(p0->pos, dir, INFINITY, false);
            if (hit.hit && hit.t < dist)
            {
                continue;
            }
            else
            {
                int * p0a = vector_expand(&p0->adjacent_portals, 1);
                int * p1a = vector_expand(&p1->adjacent_portals, 1);
                *p0a = j;
                *p1a = i;
            }
        }
    }
    for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    {
        AudioPortal * p = &portals[i];
        int * adj = CAST_VEC(p->adjacent_portals, int);
        for (int j = 0; j < p->adjacent_portals.count; j++)
        {
            printf("Portal %d Adjacent to %d\n", i, adj[j]);
            PRINT_VEC3N(p->pos);
            PRINT_VEC3N(portals[adj[j]].pos);
            
        }
    }
}

#define APORTAL_VISIBILITY_RADIUS 1.0f

int find_closest_visible_aportal(vec3 pos)
{

    AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    float closest_dist = INFINITY;
    int closest_index = -1;
    for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    {

        AudioPortal * p = &portals[i];

        vec3 diff = vec3_sub(p->pos, pos);
        float dist = vec3_length(diff);
        vec3 dir = vec3_scale(diff, 1.0f / dist); 
        
        RayHit hit = raycast_scene(pos, dir, INFINITY, false);

        if (hit.hit && hit.t < dist)
        {
            continue;
        }
        else
        {
            if (closest_dist > dist)
            {
                closest_dist = dist;
                closest_index = i;
            }
        }
    }

    return closest_index;

}


static void heap_swap(OpenNode *a, OpenNode *b) {
    OpenNode tmp = *a; *a = *b; *b = tmp;
}

static void heap_push(Vector * h) {
    int i = h->count - 1;
    OpenNode * buf = CAST_VEC(*h, OpenNode);
    
    while (i > 0) {
        int parent = (i - 1) / 2;
        if (buf[parent].f <= buf[i].f) break;
        heap_swap(&buf[parent], &buf[i]);
        i = parent;
    }
}

static OpenNode heap_pop(Vector *h) {

    OpenNode * buf = CAST_VEC(*h, OpenNode);
    OpenNode root = buf[0];
    buf[0] = buf[--h->count];
    int i = 0;
    while (1) {
        int left = 2*i + 1, right = 2*i + 2, smallest = i;
        if (left < h->count && buf[left].f < buf[smallest].f) smallest = left;
        if (right < h->count && buf[right].f < buf[smallest].f) smallest = right;
        if (smallest == i) break;
        heap_swap(&buf[i], &buf[smallest]);
        i = smallest;
    }
    return root;
}


void reconstruct_path_aportal(AStarWorkspace * WS, int start, int end)
{

    
    int current = end;
    while (current != start)
    {

        if (WS->path_count >= MAX_PORTALS) break;
        WS->path[WS->path_count++] = current;

        int prev = WS->stack[end].came_from;
        if (prev == -1)
        {
            printf("NO PARENT %d\n", current);
            break;
        }
        current = prev;
    }
    if (WS->path_count < MAX_PORTALS - 1) WS->path[WS->path_count++] = start;

    
    for (int i = 0; i < WS->path_count / 2; i++) {
        int tmp = WS->path[i];
        WS->path[i] = WS->path[WS->path_count-1-i];
        WS->path[WS->path_count-1-i] = tmp;
    }

    

}





int a_star_aportal(AStarWorkspace * WS, int start, int end)
{


    WS->current_run++;
    WS->path_count = 0;
    init_vector(&WS->min_heap, 0, sizeof(OpenNode));

    AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);

    AudioPortal * p0 = &portals[start];
    
    AudioPortal * p1 = &portals[end];
    
    OpenNode * start_node = vector_expand(&WS->min_heap, 1);
    start_node->i = start;
    float f = vec3_length(vec3_sub(p0->pos, p1->pos));
    start_node->f = f;
    WS->stack[start].g = 0.0f;
    WS->stack[start].f = f;
    WS->stack[start].came_from = -1;
    WS->stack[start].run_id = WS->current_run;
    heap_push(&WS->min_heap);
    

    while(WS->min_heap.count > 0)
    {

        OpenNode current = heap_pop(&WS->min_heap);
        if (current.i == end)
        {
            reconstruct_path_aportal(WS, start, end);
            return end;
        }

        AudioPortal * current_portal = &portals[current.i];
        if (!current_portal->adjacent_portals.buffer) continue;
        int * adj = CAST_VEC(current_portal->adjacent_portals, int);
        for (int i = 0; i < current_portal->adjacent_portals.count; i++)
        {

            int neighbor = adj[i];

            AudioPortal * neighbor_portal = &portals[neighbor];
            float tenative_g = current.g + vec3_length(vec3_sub(current_portal->pos, neighbor_portal->pos));
            float old_g = WS->stack[neighbor].g;
            bool did_search = (WS->stack[neighbor].run_id == WS->current_run);

            if (!did_search || tenative_g < old_g)
            {
                float f = tenative_g + vec3_length(vec3_sub(neighbor_portal->pos, p1->pos));

                OpenNode * neighbor_node = vector_expand(&WS->min_heap, 1);
                neighbor_node->i = neighbor;
                neighbor_node->g = tenative_g;
                neighbor_node->f = f;
                neighbor_node->parent = current.i;

                WS->stack[neighbor].came_from = current.i;
                WS->stack[neighbor].f = f;
                WS->stack[neighbor].g = tenative_g;
                WS->stack[neighbor].run_id = WS->current_run;
                heap_push(&WS->min_heap);
                
            }
        }
    }

    return -1;
}


float calculate_path_volume_inverse(float path_distance, float min_distance, float max_distance, float rolloff) {
    if (path_distance <= min_distance) {
        return 1.0f;
    }
    if (path_distance >= max_distance) {
        return 0.0f;
    }

    float gain = min_distance / (min_distance + rolloff * (path_distance - min_distance));

    float fade = 1.0f - ((path_distance - min_distance) / (max_distance - min_distance));
    return gain * fade;
}


float calculate_path_volume_equal_power(float path_distance, float min_distance, float max_distance) {
    if (path_distance <= min_distance) return 1.0f;
    if (path_distance >= max_distance) return 0.0f;

    float norm = (path_distance - min_distance) / (max_distance - min_distance);
    float linear_factor = 1.0f - norm;

    return linear_factor * linear_factor; 
}

float calculate_portal_path_volume(const float *path_nodes_dist, int node_count, float total_path_distance, float min_dist, float max_dist) {
    float distance_volume = calculate_path_volume_equal_power(total_path_distance, min_dist, max_dist);

    float portal_transmission = 1.0f;
    for (int i = 0; i < node_count - 1; i++) {
        float portal_openness = 1.0f;
        portal_transmission *= (0.2f + 0.8f * portal_openness); 
    }

    return distance_volume * portal_transmission;
}

bool pathfind_audio_occlusion(vec3 listener_pos, int closest_l, vec3 emitter_pos, vec3 * out_pos, float * out_volume, float * distance)
{


    Timer res_timer;
    start_timer(&res_timer);
    int closest_e = find_closest_visible_aportal(emitter_pos);

    
    if (closest_l == -1 || closest_e == -1) return false;

    

    int res = a_star_aportal(&AW, closest_l, closest_e);
    
    double res_time = end_timer(&res_timer);

    if (res == -1)
    {
        return false;
    }

    AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);


    start_timer(&res_timer);
    int final_path_pos = 0;
    int actual_path_start = 0;
    for (int i = 0; i < AW.path_count; i++)
    {

        AudioPortal * p = &portals[AW.path[i]];

        vec3 diff = vec3_sub(p->pos, listener_pos);
        float dist = vec3_length(diff);
        vec3 dir = vec3_scale(diff, 1.0f / dist); 
        
        RayHit hit = raycast_scene(listener_pos, dir, INFINITY, false);

        if (hit.hit && hit.t < dist)
        {
            break;
        }
        final_path_pos = AW.path[i];
        actual_path_start = i;
    }

    float dist = 0.0f;
    dist += vec3_length(vec3_sub(listener_pos, portals[final_path_pos].pos));
    dist += vec3_length(vec3_sub(emitter_pos, portals[closest_e].pos));

    for (int i = actual_path_start; i < AW.path_count-1; i++)
    {

        AudioPortal * p = &portals[AW.path[i]];
        dist += vec3_length(vec3_sub(p->pos, portals[AW.path[i+1]].pos));

    }

    if (final_path_pos < 0)
    {
        printf("Somehow the final path pos was less than zero which means that the original aportal thats visible is not visible.\n");
        return false;
    }

    *out_pos = portals[final_path_pos].pos;
    *out_volume = calculate_path_volume_equal_power(dist, 0.2f, 20.0f);
    *distance = dist;
    double res_time2 = end_timer(&res_timer);

    return true;

}



void init_new_audio_portal(vec3 pos)
{
    
    AudioPortal * ap = vector_expand(&GLOBALS.audio_portals, 1);
    ap->pos = pos;
    init_vector(&ap->adjacent_portals, 0, sizeof(int));
}

void remove_audio_portal(int id)
{

    AudioPortal * aps = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    AudioPortal * ap = &aps[id];

    if (ap->adjacent_portals.buffer) free(ap->adjacent_portals.buffer);
    REMOVE_FROM_VEC(&GLOBALS.audio_portals, id, AudioPortal);

}




void load_emitter(AudioEmitter * emitter, const char* filepath, bool is_loop, bool is_occludable, float base_volume)
{
    ma_sound_config sound_config = ma_sound_config_init();

    sound_config.pFilePath = filepath;    
    sound_config.channelsIn = 1;
    sound_config.channelsOut = 0;

    ma_result res = ma_sound_init_from_file(&AUDIO_ENGINE, filepath, 0, NULL, NULL, &emitter->sound_obj);

    if (res != MA_SUCCESS)
    {
        return;
    }
    emitter->is_loaded = true;
    emitter->is_loop = is_loop;
    emitter->is_occludable = is_occludable;
    emitter->base_volume = base_volume;
    strcpy(emitter->path, filepath);
    ma_uint32 channels = ma_engine_get_channels(&AUDIO_ENGINE);
    ma_uint32 sampleRate = ma_engine_get_sample_rate(&AUDIO_ENGINE);
    emitter->current_cutoff = 20000.0f;

    ma_result res2 = ma_node_attach_output_bus(&emitter->sound_obj, 0, &CONVOLVER_NODE, 0);
    if (res2 != MA_SUCCESS)
    {
        printf("COULDNT BIND\n");
        if (res2 == MA_INVALID_OPERATION)
        {
            printf("INVALID\n");
        }
    }

    ma_sound_set_spatialization_enabled(&emitter->sound_obj, MA_TRUE);
}


void update_lpf_sound(AudioEmitter * emitter, bool occluded)
{
    float cutoff = occluded ? 350.0f : 20000.0f;
    
    emitter->current_cutoff += (cutoff - emitter->current_cutoff) * 10.0f * dt;


    ma_lpf_config filterUpdate = ma_lpf_config_init(
        emitter->lpf_node.lpf.format,    
        emitter->lpf_node.lpf.channels,   
        emitter->lpf_node.lpf.sampleRate, 
        emitter->current_cutoff,        
        LPF_ORDER
    );
    
    ma_result result = ma_lpf_node_reinit(&filterUpdate, &emitter->lpf_node);
}

AudioEmitter * init_emitter(const char * filepath, bool is_loop, bool is_occludable, float base_volume)
{

    AudioEmitter * emitter = NULL;
    for (int i = 0; i < MAX_AUDIO_EMITTERS; i++)
    {
        if (AUDIO_EMITTERS[i].free)
        {
            emitter = &AUDIO_EMITTERS[i];
            break;
        }
    }
    load_emitter(emitter, filepath, is_loop, is_occludable, base_volume);
    emitter->free = false;
    return emitter;
}

void unload_emitter(int id)
{
    
    AudioEmitter * e = &AUDIO_EMITTERS[id];
    ma_sound_uninit(&e->sound_obj);
    e->is_loaded = false;


    
}

void destroy_emitter(int id)
{

    AudioEmitter * e = &AUDIO_EMITTERS[id];

    ma_sound_uninit(&e->sound_obj);
    memset(e, 0, sizeof(AudioEmitter));
    e->free = true;
    
}


void play_emitter(AudioEmitter * emitter, vec3 pos)
{
    
    if (!emitter->is_loaded) return;
    emitter->pos = pos;
    ma_sound_set_position(&emitter->sound_obj, pos.x, pos.y, pos.z);
    if (emitter->is_loop)
    {
        
        if (ma_sound_is_looping(&emitter->sound_obj))
        {
            return;
        }
        else
        {
            
            ma_sound_set_looping(&emitter->sound_obj, MA_TRUE);
            ma_sound_start(&emitter->sound_obj);
        }
    }
    else
    {
        ma_sound_seek_to_pcm_frame(&emitter->sound_obj, 0);
        ma_sound_start(&emitter->sound_obj);
    }
    

}
void end_emitter(AudioEmitter * e)
{
    ma_sound_seek_to_pcm_frame(&e->sound_obj, 0);
    ma_sound_stop(&e->sound_obj);
}
void audio_update(Player * player) {

    ma_engine_listener_set_position(&AUDIO_ENGINE, 0, player->skeleton.neck.pos.x, player->skeleton.neck.pos.y, player->skeleton.neck.pos.z);
    ma_engine_listener_set_direction(&AUDIO_ENGINE, 0, player->cam->forward.x, player->cam->forward.y, player->cam->forward.z);
    ma_engine_listener_set_world_up(&AUDIO_ENGINE, 0, player->cam->up.x, player->cam->up.y, player->cam->up.z);


    for (int i = 0; i < MAX_AUDIO_EMITTERS; i++) {
        AudioEmitter* emitter = &AUDIO_EMITTERS[i];
        if (emitter->free) continue;
        if (!ma_sound_is_playing(&emitter->sound_obj)) continue;

        ma_sound_set_position(&emitter->sound_obj, emitter->virtual_pos.x, emitter->virtual_pos.y, emitter->virtual_pos.z);


        ma_sound_set_min_distance(&emitter->sound_obj, 100000.0f);
        ma_sound_set_max_distance(&emitter->sound_obj, 100000.0f);
        ma_sound_set_volume(&emitter->sound_obj, emitter->current_volume);
    }
}


void SDLAudioCallback(void* pUserData, Uint8* pStream, int len) {

    ma_engine* pEngine = (ma_engine*)pUserData;

    ma_uint64 frames_to_read = len / (sizeof(float) * 2);
    memset(pStream, 0, len);
    
    ma_engine_read_pcm_frames(pEngine, pStream, frames_to_read, NULL);
}



vec3 pos_to_pv_local(OBB *obb, vec3 pos)
{
    
    vec3 local_pos = vec3_sub(pos, obb->center);
    vec3 unrotated = mat3_mul_vec3(mat3_transpose(obb->rot), local_pos);
    vec3 norm = vec3_div(unrotated, vec3_scale(obb->extents, 2.0f));
    return vec3_add(norm, (vec3){0.5f, 0.5f, 0.5f});
}

int query_audio_probe(ProbeVolume * pv, vec3 pos, ivec3 * out_local)
{
    
    vec3 pv_local = pos_to_pv_local(&pv->obb, pos);
    ivec3 pv_index = {clamp(pv_local.x * (pv->resolution.x), 0, pv->resolution.x-1), clamp(pv_local.y * (pv->resolution.y), 0, pv->resolution.y-1), clamp(pv_local.z * (pv->resolution.z), 0, pv->resolution.z-1)};
    *out_local = pv_index;
    return pv_index.z * pv->resolution.y * pv->resolution.x + pv_index.y * pv->resolution.x + pv_index.x;
}

int query_probe_volume(vec3 pos)
{
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);

    for (int i = 0; i < GLOBALS.scene_data.audio_probe_volumes.count; i++)
    {
        ProbeVolume * pv = &pvs[i];
        if (point_in_obb(&pv->obb, pos))
        {
            return i;
        }
    }
    return -1;
}

void remove_audio_pv(APV * apv, bool * apb)
{
    if (apv->audio_probes)
    {
        free(apv->audio_probes);
        apv->audio_probes = NULL;
    }
    apv->pv_id = 0;
    *apb = false;
}

void reset_audio_pvs()
{
    for (int i = 0; i < MAX_AUDIO_PROBE_VOLUMES; i++)
    {
        remove_audio_pv(&GLOBALS.scene_data.LOADED_AUDIO_PV[i], &GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[i]);
    }
}

void load_audio_pv(APV * apv, ProbeVolume * pv)
{
    
    fseek(GLOBALS.scene_data.APV_FILE, pv->byte_offset, SEEK_SET);

    apv->audio_probes = malloc(sizeof(AudioProbe) * pv->resolution.x * pv->resolution.y * pv->resolution.z);

    for (int i = 0; i < pv->resolution.z * pv->resolution.y * pv->resolution.x; i++)
    {

        AudioProbe * p = &apv->audio_probes[i];

        fread(p, sizeof(AudioProbe), 1, GLOBALS.scene_data.APV_FILE);
        if (p->ir_length <= 0)
        {
            p->final_ir = NULL;
            continue;
        }
        p->final_ir = malloc(sizeof(float) * SH_MAX * p->ir_length);
        fread(p->final_ir, sizeof(float) * SH_MAX, p->ir_length, GLOBALS.scene_data.APV_FILE);
        
    }

    
}

void check_audio_pv()
{
    
    if (GLOBALS.scene_data.APV_FILE == NULL) return;

    int current = query_probe_volume(player.skeleton.neck.pos);
    if (current == -1) return;

    bool loaded = false;
    for (int i = 0; i < MAX_AUDIO_PROBE_VOLUMES; i++)
    {
        if (GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[i] && GLOBALS.scene_data.LOADED_AUDIO_PV[i].pv_id == current)
        {
            loaded = true;
            break;
        }
    }
    
    if (loaded)
    {
        return;
    }

    int empty = -1;
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);
    float farthest_dist = -INFINITY;
    int farthest_index = -1;
    for (int i = 0; i < MAX_AUDIO_PROBE_VOLUMES; i++)
    {
        if (GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[i] == 0)
        {
            empty = i;
            break;
        }
        else
        {

            vec3 center = pvs[GLOBALS.scene_data.LOADED_AUDIO_PV[i].pv_id].obb.center;
            vec3 diff = vec3_sub(player.skeleton.neck.pos, center);
            float dist = vec3_length(diff);
            if (dist > farthest_dist)
            {
                farthest_index = i;
                farthest_dist = dist;
            }
        }
    }
    if (empty == -1)
    {
        empty = farthest_index;
    }
    
    SDL_LockMutex(AUDIO_SCRATCH.aprobe_mutex);
    APV * apv = &GLOBALS.scene_data.LOADED_AUDIO_PV[empty];

    remove_audio_pv(apv, &GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[empty]);

    ProbeVolume * pv = &pvs[current];

    load_audio_pv(apv, pv);


    apv->pv_id = current;

    GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[empty] = 1;

    SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
    
}



vec3 probe_world_position(ProbeVolume * pv, int x, int y, int z)
{

    float fx = (x / (float)(pv->resolution.x - 1)) - 0.5f;
    float fy = (y / (float)(pv->resolution.y - 1)) - 0.5f;
    float fz = (z / (float)(pv->resolution.z - 1)) - 0.5f;

    vec3 local_pos;
    local_pos.x = fx * (pv->obb.extents.x * 2.0f);
    local_pos.y = fy * (pv->obb.extents.y * 2.0f);
    local_pos.z = fz * (pv->obb.extents.z * 2.0f);
    
    vec3 fwd; vec3 right; vec3 up;
    OBB_vectors(pv->obb.rot, &fwd, &right, &up);

    vec3 world_pos = vec3_add(pv->obb.center, 
    vec3_add(vec3_scale(fwd, local_pos.x),
    vec3_add(vec3_scale(right, local_pos.y),
    vec3_scale(up, local_pos.z))));

    return world_pos;
    
}


void interpolate_nearest_aprobes(vec3 pos, AudioProbeWeights * out_w)
{

    AudioProbeWeights weights;
    weights.count = 0;
    weights.APV_id = -1;
    float weight_sum = 0.0f;
    int pv_idx = query_probe_volume(pos);
    if (pv_idx == -1)
    {
        *out_w = weights;
        return;
    }
    int apv_idx = -1;
    for (int i = 0; i < MAX_AUDIO_PROBE_VOLUMES; i++)
    {
        if (GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[i] == 0) continue;
        if (GLOBALS.scene_data.LOADED_AUDIO_PV[i].pv_id == pv_idx)
        {
            apv_idx = i;
            break;
        }
    }
    if (apv_idx == -1)
    {
        *out_w = weights;
        return;
    }
    weights.APV_id = apv_idx;
    
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);
    ivec3 local;
    int index = query_audio_probe(&pvs[pv_idx], pos, &local);
    DEBUG_CURRENT_APROBE = index;

    ProbeVolume * pv = &pvs[pv_idx];

    for (int z = -1; z <= 1; z++)
    for (int y = -1; y <= 1; y++)
    for (int x = -1; x <= 1; x++)
    {
        
        int idx_x = local.x + x;
        int idx_y = local.y + y;
        int idx_z = local.z + z;
        if (idx_x < 0 || idx_x >= pv->resolution.x) continue;
        if (idx_y < 0 || idx_y >= pv->resolution.y) continue;
        if (idx_z < 0 || idx_z >= pv->resolution.z) continue;
        int current_index = index + z * pv->resolution.y * pv->resolution.x + y * pv->resolution.x + x;
        ivec3 probe_local = {local.x + x, local.y + y, local.z + z};
        vec3 probe_world = GLOBALS.scene_data.LOADED_AUDIO_PV[apv_idx].audio_probes[current_index].pos;

        vec3 d = vec3_sub(probe_world, pos);
        float dist = vec3_length(d);
        vec3 dir = vec3_normalize(d);

        RayHit hit = raycast_scene(pos, dir, INFINITY, false);

        if (hit.hit && hit.t < dist)
        {
            continue;
        }

        float w = 1.0f / powf(dist + 0.05f, 2.0f);

        if (weights.count < MAX_INTERP_APROBES)
        {
            weights.probes[weights.count] = current_index;
            weights.weights[weights.count] = w;

            weights.count++;
            weight_sum += w;

        }
        else
        {
            int lowest = 0;
            float lowest_w = INFINITY;
            for (int i = 0; i < MAX_INTERP_APROBES; i++)
            {
                
                if (weights.weights[i] < lowest_w)
                {
                    lowest_w = weights.weights[i];
                    lowest = i;
                }
                
            }

            if (lowest_w < w)
            {
                weights.probes[lowest] = current_index;
                weight_sum -= weights.weights[lowest];
                weights.weights[lowest] = w;
                weight_sum += w;

            }
        }
    }

    float inv_w = 1.0f / weight_sum;
    for (int i = 0; i < MAX_INTERP_APROBES; i++)
    {
        weights.weights[i] *= inv_w;
    }

    if (weights.count == 0)
    {
        weights.probes[0] = index;
        weights.weights[0] = 1;
        weights.count = 1;
    }


    memcpy(out_w, &weights, sizeof(AudioProbeWeights));
    
}
void update_player_audio_probe()
{

    ivec3 local;
    AudioProbeWeights w;
    interpolate_nearest_aprobes(player.skeleton.neck.pos, &w);
    SDL_LockMutex(HRTF_TD.mutex);
    if (vec3_dist(player.skeleton.neck.pos, HRTF_TD.last_pos) > 0.05f || fabsf(vec3_dist(player.cam->forward, HRTF_TD.last_fwd)) > 0.05f)
    {
        HRTF_TD.new_probe_ready = true;
        memcpy(&HRTF_TD.w, &w, sizeof(AudioProbeWeights));
        SDL_CondSignal(HRTF_TD.work_cond);
    }
    SDL_UnlockMutex(HRTF_TD.mutex);
}


float process_biquad(Biquad* filter, float in_sample) 
{
    float out_sample = (filter->b0 * in_sample 
                      + filter->b1 * filter->x1 
                      + filter->b2 * filter->x2 
                      - filter->a1 * filter->y1 
                      - filter->a2 * filter->y2) /* / filter->a0 */;

    filter->x2 = filter->x1;
    filter->x1 = in_sample;
    filter->y2 = filter->y1;
    filter->y1 = out_sample;

    return out_sample;
}


void init_bandpass(Biquad * filter, float sample_rate, float center_freq, float q)
{

    memset(filter, 0, sizeof(Biquad));
    float w0 = 2.0f * M_PI * center_freq / sample_rate;
    float alpha = sinf(w0) / (2.0f * q);

    float a0 = 1.0f + alpha;
    filter->a0 = a0;
    filter->b0 = alpha / a0;
    filter->b1 = 0.0f;
    filter->b2 = -alpha / a0;
    filter->a1 = -2.0f * cosf(w0) / a0;
    filter->a2 = (1.0f - alpha) / a0;
    
}

void generate_synthetic_IR_tail(float* out_tail, float fade_start, AudioProbe * probe, float rms) 
{

    Biquad filter[AUDIO_NUM_BANDS] = {0};
    float decay[AUDIO_NUM_BANDS] = {0};
    float envelope[AUDIO_NUM_BANDS] = {0};


    const float RT60_DECAY_CONST = -6.907755278982137f;    
    for (int b = 0; b < AUDIO_NUM_BANDS; b++)
    {

        init_bandpass(&filter[b], SAMPLE_RATE, AUDIO_BAND_FREQ[b], 0.707f);

        float rt60_samples = MAX(0.01f, probe->rt60[b] * IR_SAMPLE_RATE);
        
        
        decay[b] = expf(RT60_DECAY_CONST / rt60_samples);

        envelope[b] = rms / sqrtf(AUDIO_NUM_BANDS);
        
    }

    float unit_noise_scaler = 1.0f / 0.577350269f;

    float lpf = 0.0f;


    for (int i = fade_start; i < MAX_IR_SAMPLES; i++) {
        
        float raw = ((float)rand() / RAND_MAX) * 2.0f - 1.0f;
        raw *= unit_noise_scaler;
        float combined = 0.0f;
        for (int b = 0; b < AUDIO_NUM_BANDS; b++)
        {
            
            float diff = process_biquad(&filter[b], raw * envelope[b]);
            combined += diff;
            envelope[b] *= decay[b];
        }

        out_tail[i] = combined;
    }
}




// dont need bins since we have no bins so we can just do 1 * sample
void compute_schroeder_edc(float* energy_histogram, float* edc_out) {

    float accumulated_energy = 0.0f;
    
    for (int i = MAX_EDC_SAMPLES - 1; i >= 0; i--) {
        accumulated_energy += energy_histogram[i];
        edc_out[i] = accumulated_energy;
    }
    
    float max_energy = edc_out[0];
    
    if (max_energy <= 0.0f) max_energy = 1e-12f; 
    
    for (int i = 0; i < MAX_EDC_SAMPLES; i++) {

        if (edc_out[i] > 0.0f) {

            edc_out[i] = 10.0f * log10f(edc_out[i] / max_energy);
            
        } else {

            edc_out[i] = -120.0f; 
            
        }

    }

}


float extract_rms(AudioProbe * probe, int fade_start, int window_size)
{

    int start = fade_start - window_size;
    float sum = 0.0f;
    int count = 0;
    for (int i = start; i < fade_start; i++)
    {

        sum += probe->final_ir[i][0] * probe->final_ir[i][0];

        count++;
    }

    return (sqrtf(sum / count));
    
}




void extract_t60(float * edc_db, AudioProbe * probe, int band, float bin_duration_seconds, int last_sample) {

    int start_idx = -1;
    int end_idx = -1;
    
    for (int i = 0; i < last_sample; i++) {
        if (start_idx == -1 && edc_db[i] <= -5.0f) {
            start_idx = i;
        }
        if (end_idx == -1 && edc_db[i] <= -35.0f) {
            end_idx = i;
            break; 
        }
    }
    
    if (start_idx == -1 || end_idx == -1) {
        probe->rt60[band] = 0.0f;
        probe->slope[band] = 0.0f;
    }
    
    float sum_x = 0.0f, sum_y = 0.0f, sum_xy = 0.0f, sum_xx = 0.0f;
    for (int i = start_idx; i <= end_idx; i++) {
        float x = i * bin_duration_seconds;
        float y = edc_db[i];
        
        sum_x += x;
        sum_y += y;
        sum_xy += x * y;
        sum_xx += x * x;
    }
    
    int n = end_idx - start_idx + 1;
    float denominator = (n * sum_xx) - (sum_x * sum_x);
    float slope = 0.0f;
    float t60 = 0.0f;
    
    if (denominator != 0.0f) {
        slope = ((n * sum_xy) - (sum_x * sum_y)) / denominator;
        
        t60 = -60.0f / slope;
    }
    probe->rt60[band] = t60;
    probe->slope[band] = slope;
    
}

#define SAMPLE_IDX(i, sh, b) \
    (((i) * SH_MAX * AUDIO_NUM_BANDS) + ((sh) * AUDIO_NUM_BANDS) + (b)) \



#define Y_00 0.28209479177387814f

void process_probe_chunk(int start, int count, GLuint aprobe_ssbo)
{

    int MAX_SAMPLES_PER_PROBE = EDITOR_SETTINGS.audio_probe_iterations * EDITOR_SETTINGS.audio_probe_bounces;
    int BOUNCE_SSBO_SIZE = count * MAX_SAMPLES_PER_PROBE;
    
    AudioSampleGPU * samples_init_data = calloc(BOUNCE_SSBO_SIZE, sizeof(AudioSampleGPU));
    GLuint bounce_ssbo;
    glGenBuffers(1, &bounce_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bounce_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(AudioSampleGPU) * BOUNCE_SSBO_SIZE, samples_init_data, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, bounce_ssbo); 
    free(samples_init_data);


    glUseProgram(AUDIO_PROBE_BAKE_SHADER);
    const int AUDIO_BATCHES = 256;

    Timer gpu_timer;
    start_timer(&gpu_timer);
    for (int i = 0; i < MAX(EDITOR_SETTINGS.audio_probe_iterations / AUDIO_BATCHES, 1); i++)
    {
    
        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uProbeSize"), count);
        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uProbeOffset"), start);
        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uRayOffset"), i * AUDIO_BATCHES);

        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uIterations"), EDITOR_SETTINGS.audio_probe_iterations);

        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uBounces"), EDITOR_SETTINGS.audio_probe_bounces);
        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uMaxSamplesPerProbe"), MAX_SAMPLES_PER_PROBE);
        glUniform1ui(glGetUniformLocation(AUDIO_PROBE_BAKE_SHADER, "uBatchSize"), MIN(AUDIO_BATCHES, EDITOR_SETTINGS.audio_probe_iterations));

        glDispatchCompute((MIN(AUDIO_BATCHES, EDITOR_SETTINGS.audio_probe_iterations) + 15) / 16, (count + 15) / 16, 1);                
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
    
    }

    glFinish();
    double gpu_time = end_timer(&gpu_timer);

    Timer cpu_timer;
    
    start_timer(&cpu_timer);
    


    glBindBuffer(GL_SHADER_STORAGE_BUFFER, bounce_ssbo);
    AudioSampleGPU * asample_gpu = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (!asample_gpu)
    {
        printf("NO SAMPLES\n");
    }



    float norm = 4.0f * M_PI / ((float)EDITOR_SETTINGS.audio_probe_iterations);
    
    float * edc_in[6];
    float * edc_out[6];

    for (int b = 0; b < AUDIO_NUM_BANDS; b++)
    {

        edc_in[b] = malloc(MAX_EDC_SAMPLES * sizeof(float));
        edc_out[b] = malloc(MAX_EDC_SAMPLES * sizeof(float));
        
    }
    float * late_reverb = malloc(MAX_IR_SAMPLES * sizeof(float));

    DEBUG_APROBE_RAY_COUNT = 0;
    
    float * samples = malloc(MAX_IR_SAMPLES * SH_MAX * AUDIO_NUM_BANDS * sizeof(float));
    for (int i = 0; i < count; i++)
    {
        AudioProbe * aprobe = &GLOBALS.audio_probes[start + i];

        memset(samples, 0, sizeof(float) * MAX_IR_SAMPLES * SH_MAX * AUDIO_NUM_BANDS);

        for (int b = 0; b < AUDIO_NUM_BANDS; b++)
        {
            
            memset(edc_in[b], 0, sizeof(float) * MAX_EDC_SAMPLES);
            memset(edc_out[b], 0, sizeof(float) * MAX_EDC_SAMPLES);

        }

        memset(late_reverb, 0, sizeof(float) * MAX_IR_SAMPLES);

        int oob = 0;
        const float MAX_IR_TIME_SECONDS = (float)MAX_SAMPLES / (float)SAMPLE_RATE;        

        int last_sample[AUDIO_NUM_BANDS] = {0};
        for (int j = 0; j < EDITOR_SETTINGS.audio_probe_iterations; j++)
        {
            for (int k = 0; k < EDITOR_SETTINGS.audio_probe_bounces; k++)
            {
                int index = i * MAX_SAMPLES_PER_PROBE + j * EDITOR_SETTINGS.audio_probe_bounces + k;
                AudioSampleGPU * sample = &asample_gpu[index];

                if (!sample->valid) continue;
                float sample_index = sample->time_of_flight * IR_SAMPLE_RATE;
                int i0 = (int)floorf(sample_index);
                float frac = sample_index - (float)i0;

                if (i0 >= MAX_IR_SAMPLES - 1 || i0 < 0) continue;

                   
                float SH[SH_MAX];
                vec3 tr_dir = {sample->arrival_dir.y, sample->arrival_dir.x, -sample->arrival_dir.z};
                evaluate_sh(tr_dir, SH);

                for (int sh = 0; sh < SH_MAX; sh++)
                {

                    for (int band = 0; band < AUDIO_NUM_BANDS; band++)
                    {
                        samples[SAMPLE_IDX(i0, sh, band)] += SH[sh] * sample->energy[band] * norm;
                    }
                }
                
                int edc_sample_index = sample->time_of_flight * EDC_SAMPLE_RATE;
                if (edc_sample_index < 0 || edc_sample_index >= MAX_EDC_SAMPLES) continue;
                for (int band = 0; band < AUDIO_NUM_BANDS; band++)
                {
                    edc_in[band][edc_sample_index] += sample->abs_energy[band] * norm;  
                    if (edc_sample_index > last_sample[band])
                    {
                        last_sample[band] = edc_sample_index;
                    }
                }
            }

        }

        for (int b = 0; b < AUDIO_NUM_BANDS; b++)
        {
            compute_schroeder_edc(edc_in[b], edc_out[b]);
            extract_t60(edc_out[b], aprobe, b, 1.0f/MAX_SAMPLES, last_sample[b]);
        }


        int last = MAX_SAMPLES;
        aprobe->ir_length = 0;
        if (last <= 0) continue;
        aprobe->final_ir = malloc(last * sizeof(float) * SH_MAX);
        if (!aprobe->final_ir) break;
        memset(aprobe->final_ir, 0, sizeof(float) * SH_MAX * last);
        aprobe->ir_length = last;
        

        int fade_start = 800;
        int fade_end = 1200;
        

        int samples_per_bin = MAX_SAMPLES / AUDIO_PROBE_BINS;
        float seconds_per_bin = (float)AUDIO_PROBE_BINS / SAMPLE_RATE;

        for (int k = 0; k < last; k++)
        {
            int bin_center_sample = k;
            int sign = 1;
            if (rand() % 2 == 0) sign = -1;

            for (int b = 0; b < AUDIO_NUM_BANDS; b++)
            {
                float sh0_energy = samples[SAMPLE_IDX(k, 0, b)];
                if (sh0_energy <= 0.000001f) continue;

                float total_energy = sh0_energy / Y_00;

                float band_amplitude = sqrtf(total_energy) * sign;


                float sh_ratios[SH_MAX];
                for (int sh = 0; sh < SH_MAX; sh++)
                {
                    sh_ratios[sh] = samples[SAMPLE_IDX(k, sh, b)] / sh0_energy;
                }

                for (int m = 0; m < AUDIO_KERNEL_SIZE; m++)
                {
                    int target_sample = k + m - AUDIO_KERNEL_CENTER;
                    if (target_sample < 0) continue;
                    if (target_sample >= last) break;

                    float filtered_sample = BAND_IMPULSES[b][m] * band_amplitude;

                    for (int sh = 0; sh < SH_MAX; sh++)
                    {
                        aprobe->final_ir[target_sample][sh] += filtered_sample * sh_ratios[sh];
                    }
                }
            }
        }

        float rms = extract_rms(aprobe, fade_start, 200);

        generate_synthetic_IR_tail(late_reverb, fade_start, aprobe, rms);

        for (int i = fade_start; i < last; i++)
        {
            float alpha = 1.0f;
            if (i < fade_end)
            {
                alpha = (float)(i - fade_start) / (fade_end - fade_start);
            }

                
            float er = cosf(alpha * 0.5 * M_PI);
            float lr = sinf(alpha * 0.5 * M_PI);

            aprobe->final_ir[i][0] = (aprobe->final_ir[i][0] * er) + (late_reverb[i] * lr);

            for (int sh = 1; sh < SH_MAX; sh++)
            {

                float sh_decorrelated_noise = (((float)rand() / (float)RAND_MAX) * 2.0f - 1.0f) * 0.2f;
                float late_sh = late_reverb[i] * sh_decorrelated_noise;
                

                aprobe->final_ir[i][sh] = (aprobe->final_ir[i][sh] * er) + (late_sh * lr);
                

            }
            
        }
    }
    
    DEBUG_LINE_COUNT += DEBUG_APROBE_RAY_COUNT;
    for (int b = 0; b < AUDIO_NUM_BANDS; b++)
    {
        
        free(edc_in[b]);
        free(edc_out[b]);

    }
    free(late_reverb);
    free(samples);

    
    glDeleteBuffers(1, &bounce_ssbo);
    double cpu_time = end_timer(&cpu_timer);

    printf("CPU: %f GPU: %f\n", cpu_time, gpu_time);
    
}


void bake_audio_probe_volume(ProbeVolume * pv)
{

    if (GLOBALS.scene_data.APV_FILE == NULL)
    {
        printf("NO APV FILE OPEN TO BAKE INTO\n");
        return;
    }

    size_t byte_size = sizeof(AudioProbe) * pv->resolution.x * pv->resolution.y * pv->resolution.z;
    if (byte_size > pv->size && pv->size > 0)
    {
        printf("PV SIZE GREW, SKIPPING WRITE\n");
        return;
    }

    SDL_LockMutex(AUDIO_SCRATCH.aprobe_mutex);

    if (GLOBALS.audio_probes) free(GLOBALS.audio_probes);
    GLOBALS.audio_probes = calloc(pv->resolution.x * pv->resolution.y * pv->resolution.z, sizeof(AudioProbe));
    if (!GLOBALS.audio_probes)
    {
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, GLOBAL_INSTANCE_SSBO); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, INSTANCE_SSBO); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, TEXT_BOX_SSBO); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, RPROBE_SSBO); 
        SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
        return;
    }

    int APROBE_COUNT = pv->resolution.x * pv->resolution.y * pv->resolution.z;
    AudioProbeGPU * aprobe_init_data = calloc(APROBE_COUNT, sizeof(AudioProbeGPU));


    OBB obb = pv->obb;
    
    vec3 step_x, step_y, step_z;
    PV_step_size(pv, &step_x, &step_y, &step_z);

    vec3 pos = OBB_min_corner(&obb);

    vec3 current_pos = pos;

    for (int z = 0; z < pv->resolution.z; z++)
    {
        for (int y = 0; y < pv->resolution.y; y++)
        {
            for (int x = 0; x < pv->resolution.x; x++)
            {
                int index = z * pv->resolution.y * pv->resolution.x + y * pv->resolution.x + x;
                current_pos.x = pos.x + x * step_x.x + y * step_y.x + z * step_z.x;
                current_pos.y = pos.y + x * step_x.y + y * step_y.y + z * step_z.y;
                current_pos.z = pos.z + x * step_x.z + y * step_y.z + z * step_z.z;
                GLOBALS.audio_probes[index].pos = current_pos;
                aprobe_init_data[index].pos = GLOBALS.audio_probes[index].pos;
            }
        }
    }
    
    bind_baking_ssbos();
    
    GLuint aprobe_ssbo;
    glGenBuffers(1, &aprobe_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, aprobe_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(AudioProbeGPU) * APROBE_COUNT, aprobe_init_data, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, aprobe_ssbo); 
    free(aprobe_init_data);

    int MAX_SAMPLES_PER_PROBE = EDITOR_SETTINGS.audio_probe_iterations * EDITOR_SETTINGS.audio_probe_bounces;
    int BOUNCE_SSBO_SIZE = APROBE_COUNT * MAX_SAMPLES_PER_PROBE;
    size_t BOUNCE_SSBO_BYTE_SIZE = BOUNCE_SSBO_SIZE * sizeof(AudioSampleGPU);
    const size_t SAMPLE_SSBO_CHUNK_SIZE = 400000000;
    int chunk_count = MAX(BOUNCE_SSBO_BYTE_SIZE / SAMPLE_SSBO_CHUNK_SIZE, 1);
    int probes_per_chunk = APROBE_COUNT / chunk_count;
    for (int i = 0; i < chunk_count; i++)
    {
        size_t current_byte_size = SAMPLE_SSBO_CHUNK_SIZE;
        if (i == chunk_count - 1)
        {
            current_byte_size = BOUNCE_SSBO_BYTE_SIZE - chunk_count * SAMPLE_SSBO_CHUNK_SIZE;
        }
        int start = i * probes_per_chunk;
        process_probe_chunk(start, probes_per_chunk, aprobe_ssbo);

    }


    glDeleteBuffers(1, &aprobe_ssbo);
    
    unbind_baking_ssbos();

    size_t byte_offset = 0;
    if (pv->size > 0)
    {
        byte_offset = pv->byte_offset;
        fseek(GLOBALS.scene_data.APV_FILE, byte_offset, SEEK_SET);
    }
    else
    {
        fseek(GLOBALS.scene_data.APV_FILE, 0, SEEK_END);
        byte_offset = ftell(GLOBALS.scene_data.APV_FILE);
    }

    for (int i = 0; i < pv->resolution.z * pv->resolution.y * pv->resolution.x; i++)
    {

        AudioProbe * p = &GLOBALS.audio_probes[i];
        fwrite(p, sizeof(AudioProbe), 1, GLOBALS.scene_data.APV_FILE);
        fwrite(p->final_ir, sizeof(float) * SH_MAX, p->ir_length, GLOBALS.scene_data.APV_FILE);
        
    }

    pv->size = ftell(GLOBALS.scene_data.APV_FILE) - byte_offset;
    pv->byte_offset = byte_offset;

    for (int i = 0; i < pv->resolution.z * pv->resolution.y * pv->resolution.x; i++)
    {
        if (GLOBALS.audio_probes[i].final_ir) free(GLOBALS.audio_probes[i].final_ir);
    }
    free(GLOBALS.audio_probes);
    GLOBALS.audio_probes = NULL;

    SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
    
}




void update_audio_occlusion(Player * player)
{
    
    int closest_l = find_closest_visible_aportal(player->skeleton.neck.pos);
    for (int i = 0; i < MAX_AUDIO_EMITTERS; i++) {
        AudioEmitter* emitter = &AUDIO_EMITTERS[i];
        if (emitter->free) continue;
        if (!ma_sound_is_playing(&emitter->sound_obj)) continue;

        
        if (!emitter->is_occludable)
        {
            emitter->virtual_pos = emitter->pos;
            emitter->current_volume = emitter->base_volume;
            continue;
        }
        
        vec3 diff = vec3_sub(emitter->pos, player->skeleton.neck.pos);
        
        float len = vec3_length(diff);
        
        vec3 dir = vec3_normalize(diff);
        
        vec3 inv_dir = (vec3){1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z};
        RayHit hit = raycast_scene(player->skeleton.neck.pos, dir, INFINITY, false);
        if (hit.hit && hit.t < len)
        {
            vec3 vpos;
            float vol = 0;
            float dist = len;
            bool res = pathfind_audio_occlusion(player->skeleton.neck.pos, closest_l, emitter->pos, &vpos, &vol, &dist);
            if (res)
            {
            
                emitter->virtual_pos = vpos;
                emitter->current_volume = vol;
                emitter->current_distance = dist;
            }
            else
            {
                emitter->virtual_pos = emitter->pos;
                emitter->current_volume = 0.0f;
                emitter->current_distance = dist;
            }
            
        }
        else
        {
            emitter->virtual_pos = emitter->pos;
            emitter->current_volume = calculate_path_volume_equal_power(len, 0.2f, 20.0f);
            emitter->current_distance = len;
        }
    }


}


static void init_normalized_lpf(float cutoff_hz, float sample_rate, float* out_kernel)
{
    float sum = 0.0f;

    for (int n = 0; n < AUDIO_KERNEL_SIZE; n++)
    {
        int k = n - AUDIO_KERNEL_CENTER;
        
        float w = 0.54f - 0.46f * cosf((2.0f * M_PI * (float)n) / (float)(AUDIO_KERNEL_SIZE - 1));

        if (k == 0)
        {
            out_kernel[n] = (2.0f * cutoff_hz / sample_rate) * w;
        }
        else
        {
            float x = 2.0f * M_PI * cutoff_hz * (float)k / sample_rate;
            out_kernel[n] = (sinf(x) / (M_PI * (float)k)) * w;
        }

        sum += out_kernel[n];
    }

    if (sum > 0.0f)
    {
        for (int n = 0; n < AUDIO_KERNEL_SIZE; n++)
        {
            out_kernel[n] /= sum;
        }
    }
}

void init_band_kernels(float sample_rate,  float f1_hz,  float f2_hz,  float f3_hz,  float f4_hz, float f5_hz, float out_kernels[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE])
{
    float lpf1[AUDIO_KERNEL_SIZE];
    float lpf2[AUDIO_KERNEL_SIZE];
    float lpf3[AUDIO_KERNEL_SIZE];
    float lpf4[AUDIO_KERNEL_SIZE];
    float lpf5[AUDIO_KERNEL_SIZE];

    init_normalized_lpf(f1_hz, sample_rate, lpf1);
    init_normalized_lpf(f2_hz, sample_rate, lpf2);
    init_normalized_lpf(f3_hz, sample_rate, lpf3);
    init_normalized_lpf(f4_hz, sample_rate, lpf4);
    init_normalized_lpf(f5_hz, sample_rate, lpf5);

    for (int n = 0; n < AUDIO_KERNEL_SIZE; n++)
    {
        out_kernels[0][n] = lpf1[n];

        out_kernels[1][n] = lpf2[n] - lpf1[n];

        out_kernels[2][n] = lpf3[n] - lpf2[n];

        out_kernels[3][n] = lpf4[n] - lpf3[n];
        out_kernels[4][n] = lpf5[n] - lpf4[n];

        float delta = (n == AUDIO_KERNEL_CENTER) ? 1.0f : 0.0f; 
        out_kernels[5][n] = delta - lpf5[n];
    }
}
