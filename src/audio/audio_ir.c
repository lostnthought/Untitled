
/**
 * @file audio_ir.c
 * @brief Audio IR processing, hrtf loading and parsing, and audio threading.
 * @see init.c for the audio thread convolution.
 */
#include "utilities/types.h"
#include "load/init.h"
#include "utilities/mathf.h"


/**
 * @brief Allocates ::AUDIO_SCRATCH based on HRTF size.
 * @ingroup aprobe_group
 * @param hrtf_length The length of the loaded HRTF.
 */ 
static void allocate_audio_scratch_hrtf(int hrtf_length);

/**
 * @brief Rotates neighboring IRs to the Player and then partitions them for convolution in the audio thread.
 * @details 
 * \begin{align}
 * X_i &= \mathrm{IR[]} \\\\
 * l &= \mathrm{IR\ Length} \\\\
 * c_l &= \mathrm{Complex\ Length} \\\\
 * M^r &= \mathrm{%Player\ Rotation} \\\\
 * b &= \mathrm{%Block\ Size} \\\\
 * p &= \mathrm{%Partiton\ Count} \\\\
 * P_o &= \mathrm{%Partitions\ Out} \\\\
 * X_r &= \sum_{i=0}^{l}(D^\ell(M^r)X_i)\\\\
 * X_f &= \mathrm{FFT}(X_r) \\\\
 * X_t &= \sum_{s=0}^{c_l}(a + bi)(c + di) = (ac - bd) + (ad + bc)i \\\\
 * X_o &= ||\mathrm{IFFT}(X_t)|| \\\\
 * & \textbf{for } i \in{}[0,p) \textbf{ do}\\\\
 * & \quad \textbf{for } k \in{}[0,b) \textbf{ do}\\\\
 * & \qquad Y_i[k] = X_o[i * b + k] \\\\
 * & \quad P_o = \mathrm{FFT}(Y_i) \\\\
 * \end{align}
 * @ingroup aprobe_group
 * @param OUT The PartitionTable to write into.
 */ 
static void update_player_ir(PartitionTable * OUT);

/**
 * @brief Main thread loop for HRTFThreadData. Waits for a job and then executes the job, which is calling update_player_ir().
 * @ingroup aprobe_group
 * @param params The HRTFThreadData.
 */ 
static int SDLCALL hrtf_worker(void * params);

void evaluate_sh(vec3 dir, float SH[SH_MAX])
{

    SH[0] = 0.282095;
    SH[1] = 0.488603 * dir.y;
    SH[2] = 0.488603 * dir.z;
    SH[3] = 0.488603 * dir.x;
    SH[4] = 1.092548 * dir.x * dir.y;
    SH[5] = 1.092548 * dir.y * dir.z;
    SH[6] = 0.315392 * (3 * dir.z * dir.z - 1);
    SH[7] = 1.092548 * (dir.x * dir.z);
    SH[8] = 0.546274 * (dir.x * dir.x - dir.y * dir.y);

}


static void allocate_audio_scratch_hrtf(int hrtf_length)
{
    
    HRTF_LENGTH = hrtf_length;
    int len_hrtf = MAX_SAMPLES + hrtf_length - 1;
    int padded_len = MAX_IR_SAMPLES + hrtf_length - 1;
    padded_len = round_up_power_of_two(padded_len, 48);
    int complex_len = (padded_len / 2) + 1;
    AUDIO_SCRATCH.ir_len = padded_len;
    AUDIO_SCRATCH.complex_ir_len = complex_len;
    for (int i = 0; i < SH_MAX; i++)
    {

        AUDIO_SCRATCH.STATIC_HRTF_L[i] = fftwf_malloc(padded_len * sizeof(float));
        AUDIO_SCRATCH.STATIC_HRTF_R[i] = fftwf_malloc(padded_len * sizeof(float));
        
        AUDIO_SCRATCH.STATIC_HRTF_L_FWD[i] = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
        AUDIO_SCRATCH.STATIC_HRTF_R_FWD[i] = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
        memset(AUDIO_SCRATCH.STATIC_HRTF_L[i], 0, sizeof(float) * padded_len);
        memset(AUDIO_SCRATCH.STATIC_HRTF_R[i], 0, sizeof(float) * padded_len);
        memset(AUDIO_SCRATCH.STATIC_HRTF_L_FWD[i], 0, sizeof(fftwf_complex) * complex_len);
        memset(AUDIO_SCRATCH.STATIC_HRTF_R_FWD[i], 0, sizeof(fftwf_complex) * complex_len);
        AUDIO_SCRATCH.IR_ROT[i] = fftwf_malloc(padded_len * sizeof(float));

        AUDIO_SCRATCH.IR_ROT_FWD[i] = fftwf_malloc(AUDIO_SCRATCH.complex_ir_len * sizeof(fftwf_complex));
        memset(AUDIO_SCRATCH.IR_ROT[i], 0, sizeof(float) * padded_len);
        memset(AUDIO_SCRATCH.IR_ROT_FWD[i], 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_ir_len);
    }
    AUDIO_SCRATCH.IR_SUM_L = fftwf_malloc(padded_len * sizeof(float));
    AUDIO_SCRATCH.IR_SUM_R = fftwf_malloc(padded_len * sizeof(float));
    AUDIO_SCRATCH.IR_SUM_L_FWD = fftwf_malloc(AUDIO_SCRATCH.complex_ir_len * sizeof(fftwf_complex));
    AUDIO_SCRATCH.IR_SUM_R_FWD = fftwf_malloc(AUDIO_SCRATCH.complex_ir_len * sizeof(fftwf_complex));
    memset(AUDIO_SCRATCH.IR_SUM_L, 0, sizeof(float) * padded_len);
    memset(AUDIO_SCRATCH.IR_SUM_R, 0, sizeof(float) * padded_len);
    memset(AUDIO_SCRATCH.IR_SUM_L_FWD, 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_ir_len);
    memset(AUDIO_SCRATCH.IR_SUM_R_FWD, 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_ir_len);
    
    AUDIO_SCRATCH.final_ir_len = len_hrtf;

    AUDIO_SCRATCH.static_fwd_plan = fftwf_plan_dft_r2c_1d(padded_len, NULL, NULL, FFTW_ESTIMATE);
    AUDIO_SCRATCH.IR_fwd_plan = fftwf_plan_dft_r2c_1d(padded_len, NULL, NULL, FFTW_ESTIMATE);
    AUDIO_SCRATCH.IR_bwd_plan = fftwf_plan_dft_c2r_1d(padded_len, NULL, NULL, FFTW_ESTIMATE);
}

void load_hrtf_from_binary(const char * filename)
{
    FILE * f = fopen(filename, "rb");
    if (!f) return;

    fread(&HRTF_LENGTH, sizeof(int), 1, f);

    allocate_audio_scratch_hrtf(HRTF_LENGTH);
    
    for (int i = 0; i < SH_MAX; i++)
    {
        fread(AUDIO_SCRATCH.STATIC_HRTF_L[i], sizeof(float), AUDIO_SCRATCH.ir_len, f);
        fread(AUDIO_SCRATCH.STATIC_HRTF_R[i], sizeof(float), AUDIO_SCRATCH.ir_len, f);
        fread(AUDIO_SCRATCH.STATIC_HRTF_L_FWD[i], sizeof(fftwf_complex), AUDIO_SCRATCH.complex_ir_len, f);
        fread(AUDIO_SCRATCH.STATIC_HRTF_R_FWD[i], sizeof(fftwf_complex), AUDIO_SCRATCH.complex_ir_len, f);
    }
    
    if (f) fclose(f);
}


/**
 * @brief Applies an impulse response filter via discrete convolution.
 * \f[
 * (x * h)[n] = \sum_{m=-\infty}^{\infty} x[m] \, h[n - m]
 * \f]
 */ 
void fft_channels(ConvolverNode * node, bool crossfading)
{
    


    fftwf_execute_dft_r2c(node->fwd_plan, node->LEFT_OUT, node->MONO_FWD);

    memset(CONVOLVER_NODE.PREV_LEFT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);
    memset(CONVOLVER_NODE.PREV_RIGHT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);
    memset(CONVOLVER_NODE.LEFT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);
    memset(CONVOLVER_NODE.RIGHT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);

    node->history_head = (node->history_head + 1) % NUM_PARTITIONS_MAX;

    memcpy(node->AUDIO_HISTORY[node->history_head], node->MONO_FWD, sizeof(fftwf_complex) * node->complex_len);

    for (int i = 0; i < NUM_PARTITIONS; i++)
    {

        int history_idx = (node->history_head - i + NUM_PARTITIONS_MAX) % NUM_PARTITIONS_MAX;

        fftwf_complex * X = node->AUDIO_HISTORY[history_idx];
        fftwf_complex * prev_ir_left = (*node->prev_ir)[0][i];
        fftwf_complex * prev_ir_right = (*node->prev_ir)[1][i];

        
        for (int j = 0; j < node->complex_len; j++)
        {

            node->PREV_SUM_L[j][0] += X[j][0] * prev_ir_left[j][0] - X[j][1] * prev_ir_left[j][1];
            node->PREV_SUM_L[j][1] += X[j][0] * prev_ir_left[j][1] + X[j][1] * prev_ir_left[j][0];
            
            node->PREV_SUM_R[j][0] += X[j][0] * prev_ir_right[j][0] - X[j][1] * prev_ir_right[j][1];
            node->PREV_SUM_R[j][1] += X[j][0] * prev_ir_right[j][1] + X[j][1] * prev_ir_right[j][0];

        }

        if (crossfading)
        {

            fftwf_complex * ir_left = (*node->ir)[0][i];
            fftwf_complex * ir_right = (*node->ir)[1][i];
            
            for (int j = 0; j < node->complex_len; j++)
            {

                node->SUM_L[j][0] += X[j][0] * ir_left[j][0] - X[j][1] * ir_left[j][1];
                node->SUM_L[j][1] += X[j][0] * ir_left[j][1] + X[j][1] * ir_left[j][0];
            
                node->SUM_R[j][0] += X[j][0] * ir_right[j][0] - X[j][1] * ir_right[j][1];
                node->SUM_R[j][1] += X[j][0] * ir_right[j][1] + X[j][1] * ir_right[j][0];

            }
        }
        
    }

    fftwf_execute_dft_c2r(node->bwd_plan, node->PREV_SUM_L, node->PREV_LEFT_OUT);
    fftwf_execute_dft_c2r(node->bwd_plan, node->PREV_SUM_R, node->PREV_RIGHT_OUT);

    if (crossfading)
    {

        fftwf_execute_dft_c2r(node->bwd_plan, node->SUM_L, node->LEFT_OUT);
        fftwf_execute_dft_c2r(node->bwd_plan, node->SUM_R, node->RIGHT_OUT);
        
    }
    
}

/**
 * @brief Converts a position to local OBB space and returns the coord as if it was a texture from \f$0 \rightarrow 1\f$.
 * @ingroup aprobe_group
 * \begin{align}
 * X_i &= \mathrm{IR[]} \\\\
 * l &= \mathrm{IR\ Length} \\\\
 * c_l &= \mathrm{Complex\ Length} \\\\
 * M^r &= \mathrm{%Player\ Rotation} \\\\
 * b &= \mathrm{%Block\ Size} \\\\
 * p &= \mathrm{%Partiton\ Count} \\\\
 * P_o &= \mathrm{%Partitions\ Out} \\\\
 * X_r &= \sum_{i=0}^{l}(X_iM^r)\\\\
 * X_f &= \mathrm{FFT}(X_r) \\\\
 * X_t &= \sum_{s=0}^{c_l}(a + bi)(c + di) = (ac - bd) + (ad + bc)i \\\\
 * X_o &= ||\mathrm{IFFT}(X_t)|| \\\\
 * \end{align}
 * \f[
 * \begin{aligned}
 * \textbf{for } i \in{}[0,p) \textbf{ do}\\\\
 * \textbf{for } k \in{}[0,b) \textbf{ do}\\\\
 * Y_i[k] = X_o[i * b + k] \\\\
 * P_o = \mathrm{FFT}(Y_i) \\\\
 * \end{aligned}
 * \f]
 */ 
// CODE FROM JOHN HABLE
void rotate_sh(float * OUT, float SH[SH_MAX], mat3 rot)
{
    
    const static float s_c3 = 0.94617469575f; 
    const static float s_c4 = -0.31539156525f;
    const static float s_c5 = 0.54627421529f; 

    const static float s_c_scale = 1.0 / 0.91529123286551084f;
    const static float s_c_scale_inv = 0.91529123286551084f;

    const static float s_rc2 = 1.5853309190550713f * s_c_scale;
    const static float s_c4_div_c3 = s_c4 / s_c3;
    const static float s_c4_div_c3_x2 = (s_c4 / s_c3) * 2.0;

    const static float s_scale_dst2 = s_c3 * s_c_scale_inv;
    const static float s_scale_dst4 = s_c5 * s_c_scale_inv;
    
    float src[SH_MAX];
    memcpy(src, SH, sizeof(float) * SH_MAX);

    OUT[0] = src[0];
    OUT[1] = rot.b1 * src[1] - rot.b2 * src[2] + rot.b0 * src[3];
    OUT[2] = -rot.c1 * src[1] + rot.c2 * src[2] - rot.c0 * src[3];
    OUT[3] = rot.a1 * src[1] - rot.a2 * src[2] + rot.a0 * src[3];
    


    float sh0 = src[7] + src[8] + src[8] - src[5];
    float sh1 = src[4] + s_rc2 * src[6] + src[7] + src[8];
    float sh2 = src[4];
    float sh3 = -src[7];
    float sh4 = -src[5];

    float r2x = rot.a0 + rot.a1;
    float r2y = rot.b0 + rot.b1;
    float r2z = rot.c0 + rot.c1;

    float r3x = rot.a0 + rot.a2;
    float r3y = rot.b0 + rot.b2;
    float r3z = rot.c0 + rot.c2;

    float r4x = rot.a1 + rot.a2;
    float r4y = rot.b1 + rot.b2;
    float r4z = rot.c1 + rot.c2;
 
    float sh0_x = sh0 * rot.a0;
    float sh0_y = sh0 * rot.b0;
    float d0 = sh0_x * rot.b0;
    float d1 = sh0_y * rot.c0;
    float d2 = sh0 * (rot.c0 * rot.c0 + s_c4_div_c3);
    float d3 = sh0_x * rot.c0;
    float d4 = sh0_x * rot.a0 - sh0_y * rot.b0;

    float sh1_x = sh1 * rot.a2;
    float sh1_y = sh1 * rot.b2;
    d0 += sh1_x * rot.b2;
    d1 += sh1_y * rot.c2;
    d2 += sh1 * (rot.c2 * rot.c2 + s_c4_div_c3);
    d3 += sh1_x * rot.c2;
    d4 += sh1_x * rot.a2 - sh1_y * rot.b2;

    float sh2_x = sh2 * r2x;
    float sh2_y = sh2 * r2y;
    d0 += sh2_x * r2y;
    d1 += sh2_y * r2z;
    d2 += sh2 * (r2z * r2z + s_c4_div_c3_x2);
    d3 += sh2_x * r2z;
    d4 += sh2_x * r2x - sh2_y * r2y;

    float sh3_x = sh3 * r3x;
    float sh3_y = sh3 * r3y;
    d0 += sh3_x * r3y;
    d1 += sh3_y * r3z;
    d2 += sh3 * (r3z * r3z + s_c4_div_c3_x2);
    d3 += sh3_x * r3z;
    d4 += sh3_x * r3x - sh3_y * r3y;

    float sh4_x = sh4 * r4x;
    float sh4_y = sh4 * r4y;
    d0 += sh4_x * r4y;
    d1 += sh4_y * r4z;
    d2 += sh4 * (r4z * r4z + s_c4_div_c3_x2);
    d3 += sh4_x * r4z;
    d4 += sh4_x * r4x - sh4_y * r4y;
    
    OUT[4] = d0;
    OUT[5] = -d1;
    OUT[6] = d2 * s_scale_dst2;
    OUT[7] = -d3;
    OUT[8] = d4 * s_scale_dst4;    
}

static void update_player_ir(PartitionTable * OUT)
{


    SDL_LockMutex(AUDIO_SCRATCH.aprobe_mutex);
    if (HRTF_TD.w.count == 0)
    {
        SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
        return;
        
    }
    if (HRTF_TD.w.APV_id == -1)
    {
        
        SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
        return;
    }
    if (!GLOBALS.scene_data.LOADED_AUDIO_PV[HRTF_TD.w.APV_id].audio_probes || !GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG[HRTF_TD.w.APV_id])
    {
        
        SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
        return;
    }

    HRTF_TD.last_pos = player.skeleton.neck.pos;
    HRTF_TD.last_fwd = player.cam->forward;
    

    PartitionTableChannel * chl = &((*OUT)[0]);
    PartitionTableChannel * chr = &((*OUT)[1]);
    for (int j = 0; j < NUM_PARTITIONS; j++)
    {

        memset((*chl)[j], 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_len);
        memset((*chr)[j], 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_len);
    }


    memset(AUDIO_SCRATCH.IR_SUM_L, 0, sizeof(float) * AUDIO_SCRATCH.final_ir_len);
    memset(AUDIO_SCRATCH.IR_SUM_R, 0, sizeof(float) * AUDIO_SCRATCH.final_ir_len);

    int samples_per_bin = MAX_SAMPLES / AUDIO_PROBE_BINS;

    mat3 rot = rotation_matrix_r(cam.right, cam.up, cam.forward);

    for (int sh = 0; sh < SH_MAX; sh++)
    {
        memset(AUDIO_SCRATCH.IR_ROT[sh], 0, sizeof(float) * AUDIO_SCRATCH.ir_len);
    }
    for (int j = 0; j < HRTF_TD.w.count; j++)
    {
        AudioProbe * probe = &GLOBALS.scene_data.LOADED_AUDIO_PV[HRTF_TD.w.APV_id].audio_probes[HRTF_TD.w.probes[j]];
        for (int p = 0; p < probe->ir_length; p++) 
        {
            float out[SH_MAX];
            rotate_sh(out, probe->final_ir[p], rot);

            for (int sh = 0; sh < SH_MAX; sh++)
            {
                AUDIO_SCRATCH.IR_ROT[sh][p] += out[sh] * HRTF_TD.w.weights[j];
            }
        }
    }


    memset(AUDIO_SCRATCH.IR_SUM_L_FWD, 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_ir_len);
    memset(AUDIO_SCRATCH.IR_SUM_R_FWD, 0, sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_ir_len);
    for (int sh = 0; sh < SH_MAX; sh++)
    {
        fftwf_execute_dft_r2c(AUDIO_SCRATCH.IR_fwd_plan, AUDIO_SCRATCH.IR_ROT[sh], AUDIO_SCRATCH.IR_ROT_FWD[sh]);

        for (int i = 0; i < AUDIO_SCRATCH.complex_ir_len; i++)
        {
            
            AUDIO_SCRATCH.IR_SUM_L_FWD[i][0] += AUDIO_SCRATCH.IR_ROT_FWD[sh][i][0] * AUDIO_SCRATCH.STATIC_HRTF_L_FWD[sh][i][0] - AUDIO_SCRATCH.IR_ROT_FWD[sh][i][1] * AUDIO_SCRATCH.STATIC_HRTF_L_FWD[sh][i][1];
            AUDIO_SCRATCH.IR_SUM_L_FWD[i][1] += AUDIO_SCRATCH.IR_ROT_FWD[sh][i][0] * AUDIO_SCRATCH.STATIC_HRTF_L_FWD[sh][i][1] + AUDIO_SCRATCH.IR_ROT_FWD[sh][i][1] * AUDIO_SCRATCH.STATIC_HRTF_L_FWD[sh][i][0];

            AUDIO_SCRATCH.IR_SUM_R_FWD[i][0] += AUDIO_SCRATCH.IR_ROT_FWD[sh][i][0] * AUDIO_SCRATCH.STATIC_HRTF_R_FWD[sh][i][0] - AUDIO_SCRATCH.IR_ROT_FWD[sh][i][1] * AUDIO_SCRATCH.STATIC_HRTF_R_FWD[sh][i][1];
            AUDIO_SCRATCH.IR_SUM_R_FWD[i][1] += AUDIO_SCRATCH.IR_ROT_FWD[sh][i][0] * AUDIO_SCRATCH.STATIC_HRTF_R_FWD[sh][i][1] + AUDIO_SCRATCH.IR_ROT_FWD[sh][i][1] * AUDIO_SCRATCH.STATIC_HRTF_R_FWD[sh][i][0];
        }
    }
    memset(AUDIO_SCRATCH.IR_SUM_L, 0, sizeof(float) * AUDIO_SCRATCH.ir_len);
    memset(AUDIO_SCRATCH.IR_SUM_R, 0, sizeof(float) * AUDIO_SCRATCH.ir_len);
    fftwf_execute_dft_c2r(AUDIO_SCRATCH.IR_bwd_plan, AUDIO_SCRATCH.IR_SUM_L_FWD, AUDIO_SCRATCH.IR_SUM_L);
    fftwf_execute_dft_c2r(AUDIO_SCRATCH.IR_bwd_plan, AUDIO_SCRATCH.IR_SUM_R_FWD, AUDIO_SCRATCH.IR_SUM_R);

    float scale = 1.0f / (float)AUDIO_SCRATCH.ir_len;
    int fade_samples = 128; 
    int fade_start = AUDIO_SCRATCH.final_ir_len - fade_samples;    
    for (int k = 0; k < AUDIO_SCRATCH.final_ir_len; k++) {
        AUDIO_SCRATCH.IR_SUM_L[k] *= scale;
        AUDIO_SCRATCH.IR_SUM_R[k] *= scale;
    }
    

    SDL_UnlockMutex(AUDIO_SCRATCH.aprobe_mutex);
    for (int i = 0; i < NUM_PARTITIONS; i++)
    {


        
        memset(AUDIO_SCRATCH.HRTF_IR_IN, 0, sizeof(float) * AUDIO_SCRATCH.padded_len);
        for (int k = 0; k < AUDIO_SCRATCH.block_size; k++) 
        {
            int ir_idx = (i * AUDIO_SCRATCH.block_size) + k;
            if (ir_idx < AUDIO_SCRATCH.final_ir_len) {
                AUDIO_SCRATCH.HRTF_IR_IN[k] = AUDIO_SCRATCH.IR_SUM_L[ir_idx];
            }
        }
        fftwf_execute_dft_r2c(AUDIO_SCRATCH.fwd_plan, AUDIO_SCRATCH.HRTF_IR_IN, (*chl)[i]);

        memset(AUDIO_SCRATCH.HRTF_IR_IN, 0, sizeof(float) * AUDIO_SCRATCH.padded_len);
        for (int k = 0; k < AUDIO_SCRATCH.block_size; k++) 
        {
            int ir_idx = (i * AUDIO_SCRATCH.block_size) + k;
            if (ir_idx < AUDIO_SCRATCH.final_ir_len) {
                AUDIO_SCRATCH.HRTF_IR_IN[k] = AUDIO_SCRATCH.IR_SUM_R[ir_idx];
            }
        }
        fftwf_execute_dft_r2c(AUDIO_SCRATCH.fwd_plan, AUDIO_SCRATCH.HRTF_IR_IN, (*chr)[i]);        
    }
   
}

static int SDLCALL hrtf_worker(void * params)
{

    HRTFThreadData * td = (HRTFThreadData*)params;

    while(1)
    {

        if (td->stop_requested)
        {
            SDL_UnlockMutex(td->mutex);
            break;
        }

        if ((!td->new_probe_ready && !td->stop_requested))
        {
            SDL_CondWait(td->work_cond, td->mutex);
            continue;
        }
        

        PartitionTable * t = NULL;
        SDL_AtomicLock(&td->ir_swap_lock);
        {
            PartitionTable * f = td->FRONT;
            PartitionTable * a = CONVOLVER_NODE.ir;
            PartitionTable * b = CONVOLVER_NODE.prev_ir;
        
            for (int i = 0; i < PARTITION_BUFFERS; i++)
            {
                if (b != &td->tables[i] && f != &td->tables[i] && a != &td->tables[i])
                {

                    t = &td->tables[i];
                    break;
                }
            }
        }
        SDL_AtomicUnlock(&td->ir_swap_lock);

        if (t == NULL)
        {
            continue;
        }

        td->new_probe_ready = false;
        update_player_ir(t);

            
        SDL_AtomicLock(&td->ir_swap_lock);
        {
            td->FRONT = t;
        }
        SDL_AtomicUnlock(&td->ir_swap_lock);
       
        
    }

    return 1;

}

void init_hrtf_worker_thread(HRTFThreadData * td)
{

    memset(td, 0, sizeof(HRTFThreadData));

    td->mutex = SDL_CreateMutex();
    td->work_cond = SDL_CreateCond();
    td->ir_swap_lock = 0;
    
    td->w.count = 0;

    for (int i = 0; i < PARTITION_BUFFERS; i++)
    {
        for (int j = 0; j < NUM_PARTITIONS; j++)
        {
        
            td->tables[i][0][j] = fftwf_malloc(sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_len);
            td->tables[i][1][j] = fftwf_malloc(sizeof(fftwf_complex) * AUDIO_SCRATCH.complex_len);

        }
    }

    
    td->stop_requested = false;
    td->new_probe_ready = false;

    td->t = SDL_CreateThread(hrtf_worker, "HRTF Worker Thread", td);
    SDL_SetThreadPriority(SDL_THREAD_PRIORITY_LOW);

}

void delete_hrtf_worker_thread(HRTFThreadData * td)
{

    SDL_LockMutex(td->mutex);
    td->stop_requested = true;
    SDL_CondSignal(td->work_cond);

    SDL_UnlockMutex(td->mutex);

    SDL_WaitThread(td->t, NULL);

    SDL_DestroyCond(td->work_cond);

    SDL_DestroyMutex(td->mutex);

    for (int i = 0; i < PARTITION_BUFFERS; i++)
    {
        for (int j = 0; j < NUM_PARTITIONS; j++)
        {
            fftwf_free(td->tables[i][0][j]);
            fftwf_free(td->tables[i][1][j]);
        }
    }

    
}
