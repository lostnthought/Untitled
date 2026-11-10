/**
 * @file engine_types.h
 * @brief Engine specific types and defines.
 */

#ifndef ENGINE_TYPES_H
#define ENGINE_TYPES_H
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "SDL2/SDL_mutex.h"
#include "SDL2/SDL_thread.h"
#include "nk_config.h"
#include "nuklear/nuklear.h"
#include <omp.h>
#include "nuklear/nuklear_sdl_gl3.h"
#include "miniaudio/miniaudio.h"
#include <fftw3.h>
#include "uthash/uthash.h"
#include <stdatomic.h>
#include "utilities/types.h"

/**
 * @defgroup shader_structs_group Shader Structures
 * @brief Avoids polluting main C namespaces.
 * @{
 */
/** @} */

/**
 * @defgroup audio_group Audio System
 * @brief All of the required structures for the AudioEmitter system.
 * @{
 */
#define MAX_AUDIO_EMITTERS 1024 /**<The maximum amount of AudioEmitter%s*/
#define MAX_AUDIO_EMITTERS_PER_ENTITY 24 /**<How many AudioEmitters an entity can have attached*/
#define LPF_ORDER 2 /**<Used for miniaudio low pass filters*/
/** @} */
/**
 * @defgroup aprobe_group AudioProbe System
 * @brief All of the required structures for the path traced AudioProbe system.
 * @{
 */
/** @} */
/**
 * @defgroup editor_group Editor System
 * @brief All of the required structures for the Editor system.
 * @{
 */
/** @} */

/**
 * @defgroup physics_group Physics System
 * @brief All of the required structures for the Physics system.
 * @{
 */
/** @} */

/**
 * @defgroup ui_group UI System
 * @brief The structures, functions, constants, and variables within the UI and text rendering system.
 * @{
 */
/** @} */

#define PI 3.14159265358979323846

#define FOV 90 /**<FOV constant for player*/
#define VIEW_NEAR 0.1 /**<Near plane for player*/
#define VIEW_FAR 1000.0 /**<Far plane for player*/


/**
 * @brief Required defines for the AudioProbe system.
 * @addtogroup aprobe_group
 * @{
 */
#define MAX_INTERP_APROBES 4 /**<Max AudioProbe%s that can be sampled at once*/
#define SAMPLE_RATE 48000 /**<Audio sample rate*/

#define IR_SAMPLE_RATE 48000 /**<Sample rate for baked audio IRs*/
#define EDC_SAMPLE_RATE 48000 /**<EDC sample rate for schroeder integration*/

#define SPEED_OF_SOUND 343.0f /**<Speed of sound constant*/
#define MAX_APROBE_SECONDS 1 /**<Longest amount of AudioProbe samples when baking*/
#define MAX_IR_SAMPLES (IR_SAMPLE_RATE * MAX_APROBE_SECONDS) /**<Maximum IR sample length*/
#define MAX_EDC_SAMPLES (EDC_SAMPLE_RATE * MAX_APROBE_SECONDS) /**<Maximum EDC sample length*/
#define MAX_SAMPLES (SAMPLE_RATE * MAX_APROBE_SECONDS) /**<Max samples for runtime convolution*/
#define AUDIO_PROBE_BINS 48000 /**<The amount of bins (to reduce max sample size)*/
#define AUDIO_KERNEL_SIZE 31 /**<Frequency kernel size for distributing samples during audio bake*/
#define AUDIO_KERNEL_CENTER 15 /**<Center of the kernel for distributing samples during audio bake*/
#define AUDIO_NUM_BANDS 6 /**<The amount of bands to use for audio probe baking*/

#define NUM_PARTITIONS_MAX 96 /**<Amount of max partitions to be used for overlap-add for audio probes + 1 for the tail trick*/
#define NUM_PARTITIONS 95 /**<Amount of max partitions to be used for overlap-add*/
/**
 * @brief Audio frequencies for each band for audio probe baking.
 */
const static float AUDIO_BAND_FREQ[] = {125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f};  
/** @} */



#define MAX_NUKLEAR_EVENTS 256 /**<Max amount of events to poll in one render step*/
#define MAX_SDL_EVENTS 512 /**<Max amount of events to poll in one render step*/
#define MAX_MESH_NODES 1024 /**<Maximum MeshNode%s in a scene (used for SSBO sizing)*/
#define MAX_MESHES 512 /**<Maximum Mesh%es in a scene (used for SSBO sizing)*/
#define MAX_JOINT_MATRICES 512 /**<Maximum joints in a scene (used for SSBO sizing)*/
#define MAX_INSTANCES 1024 /**<Maximum amount of TransformGPU instances to push in one frame (used for SSBO sizing)*/
#define MAX_MATERIALS 256 /**<Maximum amount of Material%s (used for SSBO sizing)*/
#define MAX_LIGHTS 1024 /**<Maximum amount of Light%s to be pushed per frame (used for SSBO sizing)*/

// TODO remove this define
#define MINIMUM_LIGHT_VISIBILITY_DIST 0.01 /**<Light visibility dist used for culling (only used for spot Light%s(?) now)*/
#define MAX_LIGHTS_PER_MESH 4 /**<Max directional + spot Light%s to push at one time for a Mesh*/

// TODO this might be too small
#define MAX_DRAW_COMMANDS 512 /**<Maxmimum amount of draw commands to push to the gpu*/

#define MAX_DIRECTIONAL_LIGHTS 4 /**<Maxmimum directional Light%s at one time (used for shadowmap sizing)*/

#define MAX_ENTITY_STATES 256 /**<Maximim Entity states for behaviors*/

#define DEFAULT_TEXTURES 5 /**<How many dummy default textures are on init*/

// TODO move this to editor.c
#define MAX_UI_MENU_NAMES 128 /**<Used in editor to determine dropdown size*/

/**
 * @addtogroup editor_group
 * @{
 */
    /**
     * @name Editor Wireframe Sizes
     * @brief Sizes of wireframes for various engine structures.
     * @{
     */
    #define EDITOR_LIGHT_SIZE 0.5f

    #define EDITOR_RPROBE_SIZE 0.5f

    #define EDITOR_AUDIO_EMITTER_SIZE 0.5f 

    #define EDITOR_APROBE_SIZE 0.1f 

    #define EDITOR_APORTAL_SIZE 0.2f 
    /** @} */

/** @} */

#define PERFORMANCE_OVERLAY_TEXT_BOX_COUNT 13 /**<Offset of how many text boxes are used for the debug performance overlay*/

#define BUFFER_FRAMES 3 /**<Triple buffer count*/

#define UBERSHADER_VERSIONS 17 /**<Maxmimum main shader brances*/
#define GBUFFER_VERSIONS 1 /**<Maxmimum gbuffer shader branches*/

#define MAX_PLAYER_CONTACTS 10 /**<Maximum amount of CollisionPoints to consider per frame @ingroup physics_group*/

/**
 * @name Resolutions
 * @brief Sizes of various textures.
 * @{
 */
#define LIGHTMAP_WIDTH 2048  
#define LIGHTMAP_HEIGHT 2048

#define RPROBE_WIDTH 128
#define RPROBE_HEIGHT 128

#define DEFAULT_POINT_LIGHT_SHADOWS_SIZE 128

#define DEFAULT_SHADOW_MAP_SIZE 2048

#define FBM_TEXTURE_SIZE 128
/** @} */



#define MAX_POINT_LIGHT_SHADOWS_CMA 16 /**<Maximum amount of point light shadows in texture depth*/

#define SH_MAX 9 /**<\ingroup aprobe_group Second order spherical harmonics*/

#define MAX_RPROBE_MIPS 1 /**<The amount of mips to allocate for rprobes*/

#define MAX_RPROBES_PER_FRAME 4 /**<Max ReflectionProbe%s to push to UBOs per frame (for sizing)*/

#define SSAO_KERNEL_SIZE 16 /**<SSAO init kernel size*/

#define ACCEL_BETA 0.005f /**<Friction / air accel decay constant*/


/**
 * @name Clustered shading
 * @brief Sizes of cluster grid and constants.
 * @{
 */
#define GRID_X 16  
#define GRID_Y 9
#define GRID_Z 24
#define TOTAL_CLUSTERS (GRID_X * GRID_Y * GRID_Z)
#define MAX_LIGHTS_PER_CLUSTER 12
/** @} */


/**
 * @name ProbeVolume Constants
 * @brief ProbeVolume sizes, amounts, and dimensions. Audio probes have the ability to have different dimensions, while lighting probe volumes are standardized here.
 * @{
 */
#define LPV_SLOTS 6
#define MAX_AUDIO_PROBE_VOLUMES 4
#define PROBE_VOLUME_WIDTH 12
#define PROBE_VOLUME_HEIGHT 12
#define PROBE_VOLUME_DEPTH 12
#define PROBE_VOLUME_DEPTH_PADDED (PROBE_VOLUME_DEPTH + 2) /**<Padding to avoid leaking on edges in Z*/
#define LPV_COUNT (PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH)
#define LPV_PADDED_COUNT (PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH_PADDED)
/** @} */



/**
 * @brief Enum for debug views that are ubershader branches.
 */
typedef enum DebugView
{

    DVIEW_DEFAULT,
    DVIEW_ALBEDO,
    DVIEW_ARM,
    DVIEW_EMISSIVE,
    DVIEW_AO,
    DVIEW_UV,
    DVIEW_POM_VDIR,
    DVIEW_CLUSTERS,
    DVIEW_NORMAL,
    DVIEW_ORIGINAL_NORMAL,
    DVIEW_DIRECT,
    DVIEW_INDIRECT,
    DVIEW_LM_IRRADIANCE,
    DVIEW_LM_DIRECTIONAL,
    DVIEW_RPROBE,
    DVIEW_SPECULAR,
    DVIEW_MAX

} DebugView;

/**
 * @brief Debug view names used for choosing a debug view.
 */
static const char * DVIEW_NAMES[] =
{
    "DVIEW_DEFAULT",
    "DVIEW_ALBEDO",
    "DVIEW_ARM",
    "DVIEW_EMISSIVE",
    "DVIEW_AO",
    "DVIEW_UV",
    "DVIEW_POM_VDIR",
    "DVIEW_CLUSTERS",
    "DVIEW_NORMAL",
    "DVIEW_ORIGINAL_NORMAL",
    "DVIEW_DIRECT",
    "DVIEW_INDIRECT",
    "DVIEW_LM_IRRADIANCE",
    "DVIEW_LM_DIRECTIONAL",
    "DVIEW_RPROBE",
    "DVIEW_SPECULAR"
};

/**
 * @brief Postprocess options.
 */
typedef enum PostprocessToggles
{
    PP_MBLUR, PP_DOF, PP_BLOOM, PP_NOISE, PP_ACES, PP_LUT, PP_MAX
} PostprocessToggles;

/**
 * @brief Postprocess debug view shader branches.
 */
typedef enum PostprocessDView
{
    PPDVIEW_DEFAULT,
    PPDVIEW_DEPTH,
    PPDVIEW_VEL,
    PPDVIEW_BOKEH,
    PPDVIEW_LM,
    PPDVIEW_MAX
} PostprocessDView;

/**
 * @brief Postprocess debug view names used for choosing.
 */
static const char * PPDVIEW_NAMES[] =
{
    "PP_DVIEW_DEFAULT",
    "PPDVIEW_DEPTH",
    "PPDVIEW_VEL",
    "PPDVIEW_BOKEH",
    "PPDVIEW_LM",
    "PPDVIEW_MAX",
};

/**
 * @brief Fullscreen options.
 * @see render_main_menu().
 */
typedef enum FullscreenOptions
{

    FSOPT_FULLSCREEN,
    FSOPT_BORDERLESS,
    FSOPT_WINDOWED,
    FSOPT_MAX

} FullscreenOptions;

/**
 * @brief Fullscreen option names.
 * @see render_main_menu().
 */
static const char * FULLSCREEN_OPTION_NAMES[] =
{
    "FULLSCREEN",
    "BORDERLESS",
    "WINDOWED"
};

/**
 * @brief MSAA options handled in render_main_menu(). 
 */
typedef enum MSAASampleOptions
{

    MSAA_2, MSAA_4, MSAA_8, MSAA_16, MSAA_SAMPLE_OPTIONS_MAX

} MSAASampleOptions;


typedef struct PhysicsComponent PhysicsComponent;


/**
 * @brief ReflectionProb%es used for parallax corrected cubemaps.
 * @remark Located in Globals.scene_data.reflection_probes
 * @see ::GLOBALS
 * @see ::RPROBE_CUBE_MAP_ARRAY
 * @see ::RPROBE_UBO
 */
typedef struct ReflectionProbe
{

    AABB aabb; /**<Bounding box of area.*/
    vec3 fade_margin; /**<The margin to fade between this probe and another.*/
    vec3 capture_pos; /**<The position to capture the cubemap from.*/
    int cubemap_idx; /**<Which cubemap this is linked to.*/
    
} ReflectionProbe;

/**
 * @brief Type for one FFT partiton for overlap-add.
 * @ingroup aprobe_group
 */
typedef fftwf_complex * PartitionTableChannel[NUM_PARTITIONS];

/**
 * @brief Type for two channels (stereo) for overlap-add.
 * @ingroup aprobe_group
 */
typedef PartitionTableChannel PartitionTable[2];

/**
 * @brief Weights used for interpolating between audio probes.
 * @see interpolate_nearest_aprobes()
 * @ingroup aprobe_group
 */
typedef struct AudioProbeWeights
{

    int count; /**<The amount of probes that were sampled.*/
    int APV_id; /**<Which APV these probes belong to.*/ 
    int probes[MAX_INTERP_APROBES]; /**<The indices of the AudioProbe%s.*/
    float weights[MAX_INTERP_APROBES]; /**<The weights that correspond to each probe's value.*/
    
} AudioProbeWeights;

/**
 * @brief AudioPortal%s used for determining paths to transmit sound when emitter and listener are occluded.
 * @remark Located in Globals.audio_portals
 * @see ::GLOBALS
 * @ingroup audio_group
 */
typedef struct AudioPortal
{

    vec3 pos; /**<The position of the AudioPortal.*/
    Vector adjacent_portals; /**<int - Vector of AudioPortal%s that can be seen from this one.*/
    
} AudioPortal;



#define PARTITION_BUFFERS 4 /**<The amount of audio partition back buffers.*/

/**
 * @brief The data for the thread that prepares IRs by rotating then convolving them with an HRTF before it presents it to the audio thread.
 * @see audio_ir.c
 * @ingroup aprobe_group
 */
typedef struct HRTFThreadData
{

    SDL_Thread * t; /**<The thread object.*/
    SDL_mutex * mutex; /**<The mutex used when signalling a job.*/
    SDL_cond * work_cond; /**<The job signal.*/

    SDL_SpinLock ir_swap_lock; /**<The spinlock used when presenting an IR to the audio thread.*/

    vec3 last_pos; /**<The last job's position.*/
    vec3 last_fwd; /**<The last job's direction.*/

    bool stop_requested; /**<Used to quit gracefully.*/
    bool new_probe_ready; /**<If a new job is ready.*/

    AudioProbeWeights w; /**<Current job's weights.*/

    PartitionTable tables[PARTITION_BUFFERS]; /**<The partitions for overlap-add quad-buffered. Writes into the empty one.*/

    PartitionTable * FRONT; /**<The current partition presented to the audio thread.*/
    
} HRTFThreadData;

/**
 * @brief Scratch buffers and dimensions used for audio convolution FFTs.
 * @see init_audio_scratch_buffers()
 * @see allocate_audio_scratch_hrtf()
 * @ingroup aprobe_group
 */
typedef struct AudioScratchBuffers
{

    SDL_mutex * aprobe_mutex; /**<Used when loading new APV%s to avoid concurrency with the HRTF thread.*/

    int block_size; /**<Currently 512.*/
    int padded_len; /**<Currently 1024.*/
    int complex_len; /**<(padded_len / 2) + 1*/
    int final_ir_len; /**<MAX_SAMPLES + hrtf_length - 1*/
    int ir_len; /**<MAX_IR_SAMPLES + hrtf_length - 1 and rounded up to next power of 2.*/
    int complex_ir_len; /**<(padded_len / 2) + 1*/

    float * HRTF_IR_IN; /**<Buffer used when splitting IR into partitions.*/
    fftwf_plan fwd_plan; /**<The plan used when splitting IR into partitions.*/
    fftwf_plan static_fwd_plan; /**<Unusued - was used when parsing in sofas.*/
    fftwf_plan IR_fwd_plan; /**<Forward plan used when convolving IR with HRTF.*/
    fftwf_plan IR_bwd_plan; /**<Inverse plan used when convolving IR with HRTF.*/
    fftwf_plan bwd_plan; /**<Unusued - was used when parsing in sofas.*/

    float * IR_SUM_L; /**<Used to hold the output of the inverse FFT of the IR with the HRTF.*/
    float * IR_SUM_R; /**<Used to hold the output of the inverse FFT of the IR with the HRTF.*/
    fftwf_complex * IR_SUM_L_FWD; /**<Used to accumulate the complex multiply of IR with the HRTF.*/
    fftwf_complex * IR_SUM_R_FWD; /**<Used to accumulate the complex multiply of IR with the HRTF.*/
    float * STATIC_HRTF_L[SH_MAX]; /**<The static HRTF loaded from HRTF.bin.*/
    float * STATIC_HRTF_R[SH_MAX]; /**<The static HRTF loaded from HRTF.bin.*/
    fftwf_complex * STATIC_HRTF_L_FWD[SH_MAX]; /**<The forward FFT of the static HRTF loaded from HRTF.bin.*/ 
    fftwf_complex * STATIC_HRTF_R_FWD[SH_MAX]; /**<The forward FFT of the static HRTF loaded from HRTF.bin.*/ 
    float * IR_ROT[SH_MAX]; /**<The IR with rotated SH.*/
    fftwf_complex * IR_ROT_FWD[SH_MAX]; /**<The forward FFT of the rotated IR.*/

} AudioScratchBuffers;


/**
 * @brief The IR ::CONVOLVER_NODE used to process frames in the audio thread.
 * @see init_audio_scratch_buffers()
 * @see convolver_node_process_pcm_frames()
 * @ingroup aprobe_group
 */
typedef struct ConvolverNode
{

    ma_node_base base; /**<miniaudio node base.*/
    ma_mutex lock; /**<Unused mutex for the audio thread.*/
    int block_size; /**<Currently 512.*/
    int padded_len; /**<Currently 1024.*/
    int complex_len; /**<(padded_len / 2) + 1*/
    int tail_len; /**<Currently: = block_size*/
    PartitionTable * ir; /**<The pointer to the new IR to convolve with.*/
    PartitionTable * prev_ir; /**<The pointer to the previous IR we were convolving with.*/

    fftwf_plan fwd_plan; /**<Forward plan used when convolving IR with current audio stream.*/
    fftwf_plan bwd_plan; /**<Inverse plan used when convolving IR with current audio stream.*/

    fftwf_complex * MONO_FWD; /**<Contains the output of the forward FFT of IR with current audio stream.*/
    fftwf_complex * AUDIO_HISTORY[NUM_PARTITIONS_MAX]; /**<Contains the history of MONO_FWD up to NUM_PARTITIONS_MAX.*/
    
    fftwf_complex * PREV_SUM_L; /**<The complex multiply of the previous IR with the partitions.*/
    fftwf_complex * PREV_SUM_R; /**<The complex multiply of the previous IR with the partitions.*/
    fftwf_complex * SUM_L; /**<The complex multiply of the new IR with the partitions.*/
    fftwf_complex * SUM_R; /**<The complex multiply of the new IR with the partitions.*/

    int history_head; /**<Contains the current position in the audio history buffer.*/
    
    float * PREV_LEFT_OUT; /**<Contains the output of the inverse FFT of the previous IR with current audio stream.*/
    float * PREV_RIGHT_OUT; /**<Contains the output of the inverse FFT of the previous IR with current audio stream.*/
    float * LEFT_OUT; /**<Contains the output of the inverse FFT of the new IR with current audio stream.*/
    float * RIGHT_OUT; /**<Contains the output of the inverse FFT of the new IR with current audio stream.*/

    float * PREV_TAIL_LEFT; /**<Contains the tail of the previous IR.*/
    float * PREV_TAIL_RIGHT; /**<Contains the tail of the previous IR.*/
    float * TAIL_LEFT; /**<Contains the tail of the new IR.*/
    float * TAIL_RIGHT; /**<Contains the tail of the new IR.*/

    int crossfade_pos; /**<The amount of samples crossfaded so far.*/
    bool crossfading; /**<Indicates if it is still crossfading between an old and new IR.*/
    
    
} ConvolverNode;


/**
 * @brief An AudioEmitter that plays audio possibily from a certain position or at a certain time.
 * @see audio/audio.c
 * @see ::AUDIO_EMITTERS
 * @ingroup audio_group
 */
typedef struct AudioEmitter
{

    char path[256]; /**<The path to the sound.*/
    uint32_t id; /**<The static ID of the AudioEmitter inside of ::AUDIO_EMITTERS.*/
    ma_sound sound_obj; /**<The miniaudio sound obj.*/
    ma_lpf_node lpf_node; /**<The low pass filter for this emitter.*/
    float current_cutoff; /**<The current low pass cutoff.*/
    float base_volume; /**<The starting volume.*/
    float current_volume; /**<The current volume.*/
    float current_distance; /**<The current distance from listener.*/
    vec3 pos; /**<The position of the AudioEmitter .*/
    vec3 virtual_pos; /**<The position that we spatialize the emitter at (for example: at the corner instead of behind the wall)*/
    bool is_loaded; /**<If the emitter is loaded in miniaudio.*/
    bool is_occludable; /**<If the AudioEmitter can be occluded and needs to go through the AudioPortal system.*/
    bool is_loop; /**<If the AudioEmitter loops.*/
    bool free; /**<Indicates if this AudioEmitter is empty or can be replaced.*/
    
} AudioEmitter;


/**
 * @brief Gbuffer textures. Gbuffer is unused currently as it is has been repurposed as a depth-only pass but is toggleable.
 */
typedef enum GBufferTextureNames
{
     GVEL, GUV, GNORMAL, GPOMDEPTH, GALBEDO, GEXTRA
} GBufferTextureNames;


/**
 * @brief The depth modes for initializing depth textures.
 * @see generate_gl_texture()
 */
typedef enum DepthMode
{
     DEPTH_MODE_NONE, DEPTH_MODE_NO_HW, DEPTH_MODE_HW
} DepthMode;


/**
 * @brief Texture abstraction for init. Each field corresponds to an OpenGL equivalent.
 * @see generate_gl_texture()
 * @see init_gl_texture()
 */
typedef struct GLTexture
{

    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
    GLint wrap_r;
    GLint internal_format;
    GLenum format;
    GLenum type;
    GLenum target;
    GLsizei w, h, d;
    GLsizei levels;
    vec4 border_color;
    GLuint i; /**<The image handle for this texture.*/
    DepthMode dm; /**<If this is a depth texture and what mode it is.*/
    bool multisampled; /**<If this is a multisampled texture.*/
    int samples; /**<If multisampled, how many samples.*/
    bool fixed_sample_locations; /**<This is set to true if multisampled and is standard for MSAA.*/
    
} GLTexture;

/**
 * @brief The abstraction for a renderbuffer in OpenGL. Mainly used for multisampled contexts.
 * @see init_fbo()
 */
typedef struct GLRenderbuffer
{
    
    GLuint i;
    GLint internal_format;
    GLsizei w, h, d;
    bool multisampled;
    int samples;

} GLRenderbuffer;


/**
 * @brief The abstraction for a framebuffer in OpenGL.
 * @see init_fbo()
 */
typedef struct Framebuffer
{

    int texture_count; /**<The amount of GLTexture%s bound.*/
    int rbo_count; /**<The amount of GLRenderbuffer%s bound.*/
    GLuint fbo; /**<The handle for this fbo.*/
    GLTexture texture[6]; /**<The textures bound to this fbo.*/
    GLRenderbuffer renderbuffers[6]; /**<The renderbuffers bound to this fbo.*/
    GLTexture depth_texture; /**<The normal depth texture, if there is one.*/
    GLRenderbuffer depth_rbo; /**<The renderbuffer depth texture, if there is one.*/
    int w, h; /**<Dimensions.*/
    
} Framebuffer;

/**
 * @brief The selection mode for the editor. Depending on which one is enabled, you can select specific structures via clicking in edit mode.
 * @see core/input.c
 */
typedef enum SelectionMode
{

    SELMODE_MN, /**<MeshNode*/
    SELMODE_LIGHT, /**<Light*/
    SELMODE_LPV, /**<Lighting ProbeVolume (LPV)*/
    SELMODE_RPROBE, /**<ReflectionProbe*/
    SELMODE_ENTITY, /**<Entity*/
    SELMODE_TEXTBOX, /**<TextBox*/
    SELMODE_UIELEMENT, /**<UIElement*/
    SELMODE_APV, /**<Audio ProbeVolume (APV)*/
    SELMODE_APORTAL, /**<AudioPortal*/
    SELMODE_AEM, /**<AudioEmitter*/
    SELMODE_ASSETINSTANCE, /**<AssetInstance*/
    
} SelectionMode;


/**
 * @brief The Editor menu names rendered in editor/editor.c.
 * @see render_main_menu()
 */
typedef enum MenuNames
{
    MENU_MAIN, MENU_MN, MENU_MATERIAL, MENU_LIGHT, MENU_ENT, MENU_TEXT, MENU_UI, MENU_AUDIO, MENU_LPROBE, MENU_AINST, MENU_MAX
} MenuNames;

/**
 * @brief Editor settings and engine global variables that can be changed through the editor. Also houses flags and indices for the editor UI.
 * @see editor/editor.c
 * @ingroup editor_group
 */
typedef struct EditorSettings
{

    /**
     * @name Debug View Selection
     * @brief Changes branches in main and postprocess shaders.
     * @see render/dynamic.frag
     * @see render/postprocess.frag
     * @{
     */
    DebugView current_debug_view; 
    PostprocessDView pp_dview;

    /**
     * @name Menu Toggles
     * @brief Toggles editor menus.
     * @see render_editor_menu()
     * @{
     */
    bool editor_menu_open;
    bool editor_mode;
    bool mesh_node_menu_open;
    bool material_menu_open;
    bool light_menu_open;
    bool main_menu_open;
    bool entity_menu_open;
    bool text_menu_open;
    bool audio_menu_open;
    bool selection_menu_open;
    bool lprobe_menu_open;
    bool ui_menu_open;
    bool asset_instance_menu_open;
    bool opening_menu;
    /** @} */

    SelectionMode SELMODE;

    bool element_drag_mode;
    bool text_box_drag_mode;

    /**
     * @name Rendering Toggles
     * @brief Toggles for individual rendering settings that change the rendering loop.
     * @see render_main_menu()
     * @{
     */
    bool SSAO;
    bool SSR;
    bool DOF;
    bool MBLUR;
    bool TAA;
    bool BLOOM;
    bool MSAA;
    bool VSYNC;
    bool NOISE;
    bool LUT;
    bool ACES;
    bool UI;
    bool FULL_GBUFFER;
    /** @} */

    /**
     * @name Depth of field options
     * @brief Changes the parameters for DOF.
     * @see render_main_menu()
     * @see render/coc.frag
     * @see render/postprocess.frag
     * @{
     */
    bool dof_expanded;
    float DOF_fstop;
    float DOF_focal_length;
    float DOF_sensor_height;
    float DOF_max_blur_radius;
    /** @} */

    bool wireframe_toggled; /**<Shows wireframes around many structures: Light%s, ReflectionProbe%s, AudioProbe%s, AudioPortal%s, AABB%s, etc.*/

    /**
     * @brief Tracks rendering dimensions in editor menu.
     * @see render_main_menu()
     * @{
     */
    int res_width;
    int res_height;
    /** @} */

    /**
     * @name Selected IDs
     * @brief Tracks selected ids for many engine structures.
     * @see core/input.c
     * @{
     */
    int selected_material_mn_id;
    int selected_sm_id;
    int selected_gltf_file_id;
    int selected_rprobe_file_id;
    int selected_asset_id;
    int selected_light_id;
    int selected_mn_id;
    int selected_ent_id;
    int selected_text_id;
    int selected_audio_emitter_id;
    int selected_audio_portal_id;
    int selected_audio_probe_volume_id;
    int selected_lighting_probe_volume_id;
    int selected_fullscreen_option;
    int current_fullscreen_option;
    int selected_ui_element_id;
    int selected_ui_menu_id;
    int selected_ui_action_id;
    int selected_ui_hover_id;
    int selected_asset_instance_id;
    int selected_asset_instance_mn_id;
    int selected_r_probe;
    /** @} */

    vec3 rotation_axis; /**<The rotation axis for rotation operations in the editor.*/
    float rotation_angle; /**<The current angle for rotation operations in the editor.*/
    
    /**
     * @name PV Overlay
     * @brief Toggles debug overlays for the OBB%s for ProbeVolume%s.
     * @{
     */
    bool audio_probe_volume_overlay;
    bool lighting_probe_volume_overlay;
    /** @} */

    /**
     * @name Light menu options
     * @brief Handles various lighting and lightmap options.
     * @see render_light_menu()
     * @see light/lightmap.c
     * @{
     */
    int lm_iterations; /**<The amount of iterations to bake a lightmap.*/
    int probe_iterations; /**<The amount of iterations to bake each probe.*/
    float texels_per_uv_unit; /**<Controls the texel density of the uv lightmapper.*/
    float ao_threshold; /**<Controls the distance threshold for AO detection.*/
    int ao_iterations; /**<Controls how many iterations per lightmap iteration perform the AO check.*/
    bool mfactor_enabled; /**<Controls how many iterations per lightmap iteration perform the AO check.*/
    float mfactor; /**<Controls how many iterations per lightmap iteration perform the AO check.*/
    /** @} */

    int audio_probe_iterations; /**<The amount of iterations (sample rays) to bake per AudioProbe.*/
    int audio_probe_bounces; /**<The amount of bounces to accept per AudioProbe iteration.*/

    float render_framerate_target; /**<The target framerate for rendering and processing ui events (a cap).*/

    float reverb_wet; /**<Controls the volume of the reverb signal.*/
    float volume; /**<Controls the engine volume.*/
        
    /**
     * @name OBB Axis
     * @brief Handles OBB rotation (mainly for LPV%s and APV%s).
     * @{
     */
    float obb_axis_x_angle;
    float obb_axis_y_angle;
    float obb_axis_z_angle;
    /** @} */

    AABB lm_aabb; /**<The AABB that sizes the current lightmap in world space.*/

    size_t undo_stack_max_byte_size; /**<The maximum memory value for the Undo stack.*/
    
} EditorSettings;


/**
 * @brief DrawArraysIndirect commands are used for static meshes (eg. those that can be lightmapped).
 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawArraysIndirect.xhtml
 */
typedef struct DrawArraysIndirectCommand {

    uint32_t count;
    uint32_t instanceCount;
    uint32_t first;
    uint32_t baseInstance;
    uint32_t instanceOffset; /**<This tracks how many offsets were pushed in a frame and is used in the prefix sum.*/
    uint32_t pad0;

} DrawArraysIndirectCommand;

/**
 * @brief DrawArraysIndirect commands are used for non-static meshes.
 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glDrawElementsIndirect.xhtml
 */
typedef struct DrawElementsIndirectCommand {

    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
    uint32_t instanceOffset; /**<This tracks how many offsets were pushed in a frame and is used in the prefix sum.*/ 

} DrawElementsIndirectCommand;

/**
 * @brief The global visibility buffer that holds all indices to instances per frame after they are pushed during culling.
 * @see ::GLOBAL_INSTANCE_SSBO
 * @see render/cull_draw_calls.comp
 */
typedef struct GlobalInstanceGPU
{
    uint32_t visibility[MAX_MESHES * MAX_INSTANCES];
} GlobalInstanceGPU;


/**
 * @brief MeshNode structure padded for GPU (::MESH_NODE_SSBO)
 * @see ::MESH_NODE_SSBO
 * @see render/dynamic.vert
 */
typedef struct MeshNodeDataGPU
{

    /**
     * @name AABB
     * @brief The transformed aabb of the mesh node.
     * @{
     */
    vec4 min;
    vec4 max;
    vec4 center;
    vec4 extents;
    /** @} */

    uint32_t instance_count; /**<Used to be used to find the instance being rendered, but now that's on the transform data itself.*/
    uint32_t static_mesh;
    uint32_t lm_offset; /**<The lightmap offset into ::LIGHTMAP_SSBO where this mesh node starts.*/
    uint32_t material;
    uint32_t first;
    uint32_t count;
    uint32_t pad0;
    uint32_t pad1;

} MeshNodeDataGPU;

/**
 * @brief MeshNode buffer structure for ::MESH_NODE_SSBO
 * @see ::MESH_NODE_SSBO
 */
typedef struct MeshNodeSSBO
{
    float focal_distance; /**<The current focal distance found via compute shader, transported via this ssbo to coc.*/
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    MeshNodeDataGPU mns[MAX_MESH_NODES];
} MeshNodeSSBO;

/**
 * @brief TransformGPU buffer structure for ::INSTANCE_SSBO. Triple buffered and submitted into each frame for each mesh node.
 * @see ::INSTANCE_SSBO
 */
typedef struct TransformGPU
{

    uint32_t mn_id; /**<The MeshNode this belongs to.*/
    uint32_t command_id; /**<The DrawCommand this refers to.*/
    uvec2 directional_lights; /**<Directional Light%s count and offset.*/
    uvec2 spot_lights; /**<Spot Light%s count and offset.*/
    uint32_t skin_offset; /**<The GLTFSkin matrix offset for this transform.*/
    uint32_t material; /**<The Material ID for this submesh.*/
    mat4 tr; /**<The Transform's matrix.*/
    
} TransformGPU;


typedef struct Filter Filter;
/**
 * @brief Filters to be used for filtering out entities for triggers.
 */
struct Filter
{
    
    void * val;
    size_t byte_size;
    bool (*fn)(void*, Filter*);
    
} ;

/**
 * @brief Types of Trigger%s.
 */
typedef enum TriggerType
{

    TRIGGER_NONE,
    TRIGGER_ONESHOT,
    TRIGGER_CONTINUOUS

} TriggerType;

/**
 * @brief Trigger%s that cause events or persisting effects.
 */
typedef struct Trigger
{

    TriggerType type;
    OBB obb;
    void (*fn)(void*);
    
} Trigger;


/**
 * @brief Points that track collision data such as direction, penetration depth, and more to generate Manifold%s and resolve impulses.
 * @see Manifold
 * @see ::MANIFOLDS
 * @see physics/physics.c
 * @ingroup physics_group
 */
typedef struct CollisionPoints
{

    vec3 a; /**<Furthest point of A into B.*/
    vec3 b; /**<Furthest point of B into A.*/
    size_t index_a;
    size_t index_b;
    vec3 normal; /**<The collision normal from B into A*/
    vec3 tangent; /**<The tangent used for frictional impulse resolution*/
    float d; /**<Length of B-A.*/
    bool collision;

    /**
     * @name Components
     * @brief The PhysicsComponent%s and their respective MeshNode%s.
     * @note -1 is empty for MeshNode%s
     * @note NULL is empty for PhysicsComponent%s
     * @{
     */
    PhysicsComponent * pcA;
    int mnA;
    PhysicsComponent * pcB;
    int mnB;
    /** @} */

    /**
     * @name Manifold Data
     * @brief These track Manifold metadata over frames to check equivalency later.
     * @{
     */
    int incident_face;
    int reference_face;
    int edge_a;
    int edge_b;
    /** @} */
    
    /**
     * @name Impulses
     * @brief Persistent impulses used over iterations in the solver.
     * @{
     */
    float normal_impulse;
    float bias_impulse;
    float tangent_impulse;
    /** @} */

    
} CollisionPoints;




/**
 * @brief Persistent Manifold%s that track collisions and how to resolve them.
 * @remark Stored in ::MANIFOLDS.
 * @see physics/physics.c
 * @ingroup physics_group
 */
typedef struct Manifold
{

    /**
     * @name Components
     * @brief The PhysicsComponent%s and their respective MeshNode%s. Stored to check for matches.
     * @note -1 is empty for MeshNode%s
     * @note NULL is empty for PhysicsComponent%s
     * @{
     */
    PhysicsComponent *a;
    int mnA;
    PhysicsComponent *b;
    int mnB;
    /** @} */

    CollisionPoints contacts[4]; /**<The collision contacts.*/
    int count; /**<The amount of valid contacts.*/

    bool processed; /**<Tracks if a Manifold has been processed this frame yet.*/
    bool free; /**<Tracks if a Manifold can be replaced.*/

    float deepest; /**<Current deepest contact for a search.*/

} Manifold;

// each lightmap texel gets a patch
// triangles link to uv lightmap triangles
// on import build adjacency map for triangles
// then find coplanar faces within some epsilon and then create surfaces with tri lists
// from there create uv islands for each surface and wrap
// then bake lighting



/**
 * @brief Performance query types used to profile the render loop.
 * @see main()
 */
typedef enum PerformanceQueryTypes
{

    PQUERY_CULL,
    PQUERY_PL,
    PQUERY_SM,
    PQUERY_GBUFFER,
    PQUERY_DOF,
    PQUERY_SSAO,
    PQUERY_TAA,
    PQUERY_SSR,
    PQUERY_BLOOM,
    PQUERY_RENDER,
    PQUERY_MAIN,
    PQUERY_MAX

} PerformanceQueryTypes;


/**
 * @brief Convex Hulls that are generated via incremental hull for non-static meshes. Used for collision.
 * @see Mesh
 * @see init_mesh()
 * @ingroup physics_group
 */
typedef struct ConvexHull
{

    Vector vertices; /**<Point%s that initially copy the Mesh vertices but then are deduplicated and modified in incremental_hull().*/
    Vector faces; /**<MeshFace%s that initially copy the Mesh faces but then are modified in incremental_hull().*/
    Vector hull_faces; /**<HullFace%s that are generated after incremental_hull() that combine contiguous triangles into singular faces. Used for manifold clipping.*/
    vec3 centroid; /**<The center of the hull.*/
    AABB aabb; /**<The bounding box of the hull.*/

} ConvexHull;

/**
 * @brief Groups of contiguous triangles with the same plane that are combined via jarvis marching.
 * @see compute_hull_faces()
 * @see compute_mesh_hulls()
 * @ingroup physics_group
 */
typedef struct TriangleGroup {

    vec3 normal; /**<The group's normal.*/
    vec3 ref_point; /**<A point on the plane.*/
    Vector indices; /**<size_t indices of each MeshFace that indexes into ConvexHull.faces.*/

} TriangleGroup;


/**
 * @brief Nodes for the acceleration structures of BVHs.
 * @see Scene MeshNode BVH: Globals.scene_data.bvh_nodes
 * @see Light BVH: Globals.scene_data.light_bvh.nodes
 * @see Mesh MeshFace BVH: Mesh.nodes
 * @see core/acceleration.c
 */
typedef struct BVHNode
{

    /**
     * @name Relations
     * @brief Children, parents, and IDs for related nodes for traversal.
     * @{
     */
    int left; /**<Left child. -1 if empty.*/
    int right; /**<Right child. -1 if empty.*/
    int parent; /**<Parent. -1 if empty.*/
    int index; /**<The index of the node inside of the array.*/
    int next; /**<The pointer to the next node (either right child or parent's right child) for recursion-less traversal. See: assign_next_nodes().*/
    /** @} */

    AABB aabb; /**<The bounding box of all of this node's children.*/
    size_t first_index; /**<The first index into the bvh_indices array.*/
    size_t count; /**<How many children are inside this node.*/
    float flux; /**<The total light flux of this node. Used for Light BVH importance sampling.*/
    float emissive_flux; /**<The only-emissive flux. Used for Light BVH importance sampling.*/
    
} BVHNode;


/**
 * @brief Main rendering vertex.
 * @see ::MAIN_VBO
 * @see render/dynamic.vert
 */
typedef struct MainVertex
{

    vec3 pos; /**<The position.*/
    vec3 normal; /**<The normal.*/
    vec4 tangent; /**<The tangent for TBN construction. This is supplied by GLTF.*/
    ivec4 joint_indices; /**<The indices for each joint matrix (skeletal animations).*/
    vec4 weights; /**<The weights each joint is multiplied by.*/
    vec2 uv; /**<The texture coordinates.*/

} MainVertex;

/**
 * @brief The structure used when rasterizing lightmap data to a texture before baking.
 * @see bake_lightmap()
 */
typedef struct LightmapVertex
{
   
    vec3 pos; /**<The position.*/
    vec3 normal; /**<The normal.*/
    vec2 uv; /**<The texture coordinates for the actual face's GLTexture.*/
    vec2 lm_uv; /**<The texture coordinates for the LightmapAtlas.*/
    int material_id; /**<The Material this belongs to.*/
    int tri_id; /**<The ID of the MeshFace.*/
    
} LightmapVertex;


/**
 * @brief The structure used for debug wireframes.
 * @see ::WIREFRAME_VBO
 */
typedef struct DebugVertex
{
    vec3 pos;
    vec3 color;
    
} DebugVertex;

/**
 * @brief The structure used for rendering TextBox%es.
 * @see bake_lightmap()
 */
typedef struct TextVertex
{

    vec2 pos; /**<The position.*/
    vec2 uv; /**<The Font atlas coordinates.*/
    vec2 center; /**<The center of the TextBox before transform (should be 0,0 if justified center).*/
    vec4 color; /**<The color of the text.*/
    vec4 fill; /**<The fill color for the TextBox.*/
    uint32_t state; /**<The current UIState.*/
    uint32_t uID; /**<The ID in the ::TEXT_BOX_SSBO.*/

} TextVertex;

/**
 * @brief The structure used for uploading matrices for directional Light%s used when rendering shadowmaps.
 * @see ::LIGHT_CAM_UBO
 */
typedef struct ShadowmapUBO
{
    mat4 proj[MAX_DIRECTIONAL_LIGHTS];
    mat4 view[MAX_DIRECTIONAL_LIGHTS];
    
} ShadowmapUBO;


/**
 * @brief The structure uploaded to the GPU for providing render data for a Camera.
 * @see ::CAM_UBO
 */
typedef struct CamData
{
    vec4 cam_pos; /**<The position of the Camera (padded).*/
    vec4 cam_vel; /**<The velocity of the Camera (padded).*/
    mat4 proj; /**<The projection matrix.*/
    mat4 unjittered_proj; /**<The projection matrix without TAA jittering.*/
    mat4 view; /**<The view matrix.*/
    mat4 inv_proj; /**<The inverse projection matrix.*/
    mat4 unjittered_inv_proj; /**<The inverse projection matrix without TAA jittering.*/
    mat4 inv_view; /**<The inverse view matrix.*/
} CamData;

/**
 * @brief The structure uploaded to the GPU for providing the current resolution (fairly unused).
 * @see ::RENDER_UBO
 */
typedef struct Render
{
    ivec4 resolution;

} RenderData;

/**
 * @brief The spring used for determining Skeleton positions and eventually neck trajectory.
 * @see update_skeleton()
 */
typedef struct Spring
{
    float stiff; /**<Stiffness.*/
    float damp; /**<Dampening.*/
    vec3 velocity; /**<Velocity of this spring.*/
    vec3 pos; /**<Position of the spring.*/
    vec3 target; /**<Where the spring wants to move to based on Skeleton positions, before velocity is integrated.*/

    float stiff_rot; /**<Stiffness for rotation.*/
    float damp_rot; /**<Dampening for rotation.*/
    vec3 rotation; /**<Current rotation of the spring.*/
    vec3 angular_vel; /**<Current velocity for the rotation.*/
    vec3 target_rot; /**<Where the spring is facing based on Skeleton positions.*/
    
} Spring;

/**
 * @brief The controller for leg IKs for Skeleton%s.
 * @see update_skeleton()
 * @see move_foot()
 * @see update_body_from_legs()
 */
typedef struct LegIK
{

    bool stepping; /**<If the leg is currently stepping.*/
    bool stepping_off_ledge; /**<If the leg is over air.*/
    vec3 pos; /**<Position of the foot.*/
    vec3 start_pos; /**<Starting position of the foot.*/
    vec3 target_pos; /**<Target position for the current step.*/
    float duration; /**<The duration of the entire step.*/
    float time; /**<The current time to linearly interpolate between start and target pos.*/
    
} LegIK;

/**
 * @brief The controller for leg IKs for Skeleton%s.
 * @see update_skeleton()
 * @see move_foot()
 * @see update_body_from_legs()
 */
typedef struct Skeleton
{

    /**
     * @name Starting Measurements
     * @brief Measurements for where the skeleton is at rest.
     * @{
     */
    float pelvis_height; /**<Height from ground to pelvis.*/
    float hip_height; /**<Height from ground to hip.*/
    float neck_height; /**<Height from ground to neck.*/
    float pelvis_hip_diff; /**<Height from pelvis to hip.*/
    float neck_pelvis_diff; /**<Height from pelvis to neck.*/
    float upper_leg_length; /**<Upper leg length.*/
    float lower_leg_length; /**<Lower leg length.*/
    /** @} */
    vec3 q_left; /**<The current knee position for left leg.*/
    vec3 q_right; /**<The current knee position for right leg.*/
    vec3 p_left; /**<The current left foot position.*/
    vec3 p_right; /**<The current right foot position.*/
    vec3 left_hip_pos; /**<The left hip pos, offset by ::left_leg_offset.*/
    vec3 right_hip_pos; /**<The right hip pos, offset by ::right_leg_offset.*/

    Spring pelvis; /**<Pelvis Spring (target based on leg positions).*/
    Spring neck; /**<Neck Spring (target based on pelvis).*/
    LegIK left_leg; /**<Left leg.*/
    LegIK right_leg; /**<Right leg.*/
    
} Skeleton;


/**
 * @brief The camera structure for players and entities.
 * @see ::cam
 * @see Player
 * @see update_camera()
 */
typedef struct Camera
{
    vec3 forward, right, up, dir, target;
    float yaw, pitch, roll;
    CamData cam_data;
    bool on_ground; /**<If the camera is on the ground. Used in resolve_collisions().*/
} Camera;


/**
 * @brief The structure for ReflectionProbe%s padded for the GPU. Stores AABB bounds
 * @see ::RPROBE_SSBO
 */
typedef struct RProbe
{
    vec3 center;
    uint32_t pad0;
    vec3 min;
    uint32_t pad1;
    vec3 max;
    uint32_t pad2;
    
} RProbe;

#define MAX_RPROBES 256 /**<The maximum amount of RProbe%s to send to the GPU at once.*/
/**
 * @brief The structure for RProbe%s sent to the GPU.
 * @see ::RPROBE_SSBO
 */
typedef struct RProbeData
{

    RProbe probes[MAX_RPROBES];
    
} RProbeData;


/**
 * @brief Biquad filter. Used for audio probe baking.
 * @see process_biquad()
 */
typedef struct {
    float b0, b1, b2, a0, a1, a2;
    float x1, x2, y1, y2;
} Biquad;

/**
 * @brief Baked IR samples loaded in from binary into APV%s from ProbeVolume%s.
 * @see bake_audio_probe_volume()
 * @ingroup aprobe_group
 */
typedef struct AudioProbe
{

    size_t ir_length; /**<The amount of IR samples for this probe.*/
    vec3 pos; /**<The world position of this probe.*/
    float rt60[AUDIO_NUM_BANDS]; /**<The RT60 from schroeder integration.*/
    float slope[AUDIO_NUM_BANDS]; /**<The slope from EDC.*/
    float (* final_ir)[SH_MAX]; /**<The baked IR samples by 9 SH.*/
    
} AudioProbe;

/**
 * @brief Simply stores the positions for audio probes during the bake.
 * @see bake_audio_probe_volume()
 * @ingroup aprobe_group
 */
typedef struct AudioProbeGPU
{
    vec3 pos;
    uint32_t pad0;
} AudioProbeGPU;

/**
 * @brief Each ray's samples when baking AudioProbe%s, padded for GPU.
 * @see audio/audio_probe_bake.comp
 * @ingroup aprobe_group
 */
typedef struct AudioSampleGPU
{

    vec3 arrival_dir; /**<The direction the ray was traveling to this position.*/  
    float time_of_flight; /**<The time that it takes for the sample to reach this position in total dist + dist to probe.*/  
    vec3 pos; /**<The hit position.*/  
    uint32_t valid; /**<If it is occluded then it is not valid. Also, empty samples after the ray dies out are not valid.*/  
    float energy[6]; /**<The energy per band.*/  
    uint32_t pad0;
    uint32_t pad1;
    float abs_energy[6]; /**<The energy per band with only absorption and no scattering.*/  
    uint32_t pad4;
    uint32_t pad5;

} AudioSampleGPU;

/**
 * @brief Enum for types of Light%s.
 */
typedef enum LightType : uint32_t
{
    
    POINT_LIGHT,
    DIRECTIONAL_LIGHT,
    SPOT_LIGHT,
    AREA_LIGHT,
    MAX_LIGHT_TYPES

} LightType;

/**
 * @brief The CPU side structure for Light%s.
 * @see ::Globals.lights
 * @see LightGPU
 * @see upload_lights()
 * @see build_light_bvh()
 */
typedef struct Light
{
    
    vec3 pos; /**<Position of light.*/
    uint32_t entity_id; /**<Entity ID into ::ENTITIES.*/
    
    vec3 dir; /**<Direction for lights that are not #POINT_LIGHT%s.*/
    float cone_angle; /**<Cone angle for #SPOT_LIGHT%s.*/

    vec3 rgb; /**<Color of the light.*/
    float penumbra; /**<Penumbra (opening) for #SPOT_LIGHT%s.*/

    float intensity; /**<The intensity of the Light.*/
    LightType light_type; /**<The type of Light.*/
    float inner_angle; /**<Starting angle for #SPOT_LIGHT%s.*/
    float outer_angle; /**<Outer angle for #SPOT_LIGHT%s.*/

    float radius; /**<The radius of the Light. Computed via copy_light_to_gpu_light()*/
    float threshold; /**<The threshold (how far a light reaches).*/
    AABB area; /**<The physical area of #AREA_LIGHT%s.*/

    bool sm_dirty; /**<If a point light needs a new shadowmap.*/
    uint32_t layer; /**<Which layer a #DIRECTIONAL_LIGHT corresponds to in the ::SHADOW_MAP_FBO.*/

    int mesh; /**<The MeshNode this corresponds to. Used when building light BVH in build_light_bvh().*/
    int tri; /**<The MeshFace this corresponds to. Used when building light BVH in build_light_bvh().*/

    float lum; /**<The luminosity for importance sampling.*/
    float power; /**<The power for importance sampling.*/

    /**
     * @name Area Light Information
     * @brief Vertex data for area lights used in importance sampling and when calling build_light_bvh(). In world space.
     * @{
     */
    vec3 A; 
    vec3 B; 
    vec3 C; 
    vec3 normal; 
    float surface_area; /**<The surface area of the triangle.*/
    /** @} */
    
} Light;


/**
 * @brief The GPU side structure for Light%s padded for GPU. 
 * @see Globals.lights
 * @see ::LIGHT_SSBO
 * @see upload_lights()
 * @see copy_light_to_gpu_light()
 */
typedef struct LightGPU
{
    
    vec3 pos;
    uint32_t entity_id;
    
    vec3 dir;
    float outer_cos; /**<Precomputed when uploading to the GPU for #SPOT_LIGHT%s.*/ 

    vec3 rgb;
    float cos_diff; /**<Precomputed when uploading to the GPU for #SPOT_LIGHT%s.*/

    float intensity;
    LightType light_type;
    float inner_angle;
    float outer_angle;

    float radius;
    uint32_t layer;
    uint32_t mesh;
    uint32_t tri;

    float area;
    uint32_t bvh_index;
    uint32_t pad1;
    uint32_t pad2;
    
} LightGPU;

#define MAX_TEX_LABELS 600 /**<The maximum amount of labels for a ComboLabel.*/
#define LABEL_LEN 256 /**<The maximum label length of labels in a ComboLabel.*/

/**
 * @brief The structure used for **some** dropdown abstractions in the Editor UI. 
 * @see editor/editor.c
 * @ingroup editor_group
 */
typedef struct ComboLabel {

    char buffer[MAX_TEX_LABELS][LABEL_LEN];
    const char *ptrs[MAX_TEX_LABELS];
    int selected;
  
    int item_to_id[MAX_TEX_LABELS];
    int item_count;

} ComboLabel;

/**
 * @brief Alpha mode to determine Material blending. 
 */
typedef enum AlphaMode
{
    ALPHA_OPAQUE, ALPHA_MASK, ALPHA_BLEND
} AlphaMode;

/**
 * @brief The Materials parsed in from scene files that determine rendering pipeline setup, uploads, and ordering. 
 * @see MaterialGPU
 */
typedef struct Material
{

    char name[256]; /**<The Material path.*/
    int fwd_shader; /**<The corresponding forward rendering shader branch in ::DYNAMIC_SHADER.*/
    int gbuffer_shader; /**<The corresponding gbuffer rendering shader branch in ::DEPTH_PRE_PASS_SHADER or ::DEPTH_PRE_PASS_SHADER_MSAA.*/
    /**
     * @name Color and PBR Factors
     * @brief Factors used to change color and Material properties during rendering.
     * @{
     */
    vec4 ka; /**<Ambient.*/
    vec4 kd; /**<Diffuse.*/
    vec4 ks; /**<Specular (unused).*/
    vec4 ke; /**<Emissive.*/
    float metallic;
    float roughness;
    float ao_strength;
    float emissive_strength;
    /** @} */

    float absorption_coefficients[AUDIO_NUM_BANDS]; /**<How much audio is absorbed when hitting this Material.*/
    float scattering_coefficients[AUDIO_NUM_BANDS]; /**<How much audio specularly reflects when hitting this Material.*/

    /**
     * @name Texture Indices
     * @brief Texture indices into Globals.images.
     * @{
     */
    int diffuse_tex;
    int normal_tex;
    int emissive_tex;
    int arm_tex;
    int ao_tex;
    int mr_tex;
    /** @} */

    float normal_scale; /**<Unused - Used to determine how much of the texture normal to use.*/
    AlphaMode alpha_mode; /**<Determines transparency.*/
    float alpha_cutoff; /**<Used to discard transparent Material%s.*/

    
    bool double_sided; /**<If we should disable backface culling.*/
    bool override_transparent; /**<If we are overriding transparency of this Material.*/
    bool override_arm; /**<If we override the ARM texture and bind a dummy instead.*/
    
} Material;


/**
 * @brief The GPU side padded structure for Material%s. 
 * @see Material
 * @see ::MATERIAL_SSBO
 */
typedef struct MaterialGPU
{

    vec4 ambient;

    vec4 diffuse;

    vec4 specular;

    vec4 emissive;

    float absorption_coefficients[AUDIO_NUM_BANDS];
    uint32_t pad0;
    uint32_t pad1;

    float scattering_coefficients[AUDIO_NUM_BANDS];
    uint32_t pad2;
    uint32_t pad3;

    uint64_t albedo_handle;
    uint64_t normal_handle;
    uint64_t arm_handle;
    uint64_t emissive_handle;

    float metallic;
    float roughness;
    float ao_strength;
    float normal_scale;
    uint32_t alpha_mode;
    float alpha_cutoff;
    uint32_t double_sided;
    float emissive_strength;


} MaterialGPU;

/**
 * @brief The GPU side SSBO structure. 
 * @see Material
 * @see MaterialGPU
 * @see ::MATERIAL_SSBO
 */
typedef struct MaterialData
{
    
    MaterialGPU materials[MAX_MATERIALS];
    
} MaterialData;

/**
 * @brief The GPU side structure of a Point, used for path traced baking. 
 * @see Point
 * @see ::VERTEX_SSBO
 */
typedef struct VertexGPU
{

    vec3 pos;
    uint32_t pad0;
    vec3 normal;
    uint32_t pad1;
    vec2 uv;
    uint32_t pad2;
    uint32_t pad3;
    
} VertexGPU;

/**
 * @brief The GPU side structure for the SSBO of Point%s. 
 * @see Point
 * @see ::VERTEX_SSBO
 */
typedef struct VertexData
{
    
    VertexGPU * vertices;
    
} VertexData;

/**
 * @brief The GPU side structure of a MeshFace, used for path traced baking. 
 * @see MeshFace
 * @see ::TRI_SSBO
 */
typedef struct TriGPU
{

    uvec3 indices; /**<Unused - the indices of each point.*/
    uint32_t material_id;
    vec3 normal;
    uint32_t mesh_id; /**<The Mesh id.*/
    uint32_t bvh_index; /**<Global: The bvh index + the offset for the current Mesh.*/
    uint32_t tri_idx; /**<Local: The bvh index for this MeshFace.*/
    uint32_t pad1;
    uint32_t pad2;
    
    /**
     * @name Vertices
     * @brief The vertices for this MeshFace.
     * @{
     */
    VertexGPU v0;
    VertexGPU v1;
    VertexGPU v2;
    /** @} */
    
} TriGPU;

/**
 * @brief The GPU side structure for the SSBO of MeshFace%s. 
 * @see MeshFace
 * @see ::TRI_SSBO
 */
typedef struct TriData
{
    
    TriGPU * tris;
    
} TriData;

/**
 * @brief The GPU side structure of a Mesh, used for path traced baking. 
 * @see Mesh
 * @see ::MESH_SSBO
 */
typedef struct MeshGPU
{

    uint32_t mesh_node_id;
    uint32_t mesh_id;

    uint32_t node_offset; /**<Where the BVHNode%s start in the SSBO.*/
    uint32_t node_count; /**<How many BVHNode%s are in this Mesh.*/
    uint32_t vertex_offset; /**<Where the VertexGPU%s for this Mesh start in the SSBO.*/
    uint32_t vertex_count; /**<How many VertexGPU%s this Mesh has.*/
    uint32_t tri_offset; /**<Where the TriGPU%s for this Mesh start in the SSBO.*/
    uint32_t tri_count; /**<How many TriGPU%s there are for this Mesh.*/
    uint32_t bvh_index; /**<The BVH index for this Mesh in the scene BVH.*/
    uint32_t is_static; /**<If the mesh is not static it is not considered for path tracing.*/
    uint32_t skip_directional_light; /**<Skip directional lights so that they go through this Mesh.*/
    uint32_t pad2;

    mat4 global_matrix;
    mat4 inverse_global_matrix;
    
} MeshGPU;

/**
 * @brief The GPU side structure for the SSBO of Mesh%s. 
 * @see Mesh
 * @see ::MESH_SSBO
 */
typedef struct MeshData
{
    
    MeshGPU * meshes;
    
} MeshData;

/**
 * @brief The GPU side structure of an AABB, used for path traced baking. 
 */
typedef struct AABBGPU
{

    vec3 min;
    uint32_t pad0;
    vec3 max;
    uint32_t pad1;
    vec3 center;
    uint32_t pad2;
    vec3 extents;
    uint32_t pad3;
    
} AABBGPU;

/**
 * @brief The GPU side structure of a BVHNode, used for path traced baking. 
 * @see ::MESH_BVH_SSBO
 * @see ::SCENE_BVH_SSBO
 */
typedef struct BVHNodeGPU
{

    int32_t left;
    int32_t right;
    int32_t parent;
    int32_t index;

    int32_t first_index;
    int32_t count;
    int32_t next;
    float flux;

    vec3 min;
    float emissive_flux;
    vec3 max;
    uint32_t pad2;
    vec3 center;
    uint32_t pad3;
    vec3 extents;
    uint32_t pad4;
    
} BVHNodeGPU;

/**
 * @brief The GPU side structure for the SSBO of BVHNode%s. 
 * @see ::MESH_BVH_SSBO
 * @see ::SCENE_BVH_SSBO
 */
typedef struct BVHGPU
{

    BVHNodeGPU * nodes;
    
} BVHGPU;

/**
 * @brief Used to store SH coefficients when baking LPV%s into ProbeVolume%s. 
 * @see bake_lighting_probe_volume()
 */
typedef struct SHDataGPU
{
    vec3 pos;
    uint32_t pad0;
    vec4 coefficients[SH_MAX];
} SHDataGPU;

/**
 * @brief GPU side structure for an OBB. Used for LPV sampling. 
 * @see render/dynamic.vert
 */
typedef struct OBBGPU
{

    vec3 center;
    uint32_t loaded;
    vec3 axis_x;
    uint32_t pad1;
    vec3 axis_y;
    uint32_t pad2;
    vec3 axis_z;
    uint32_t pad3;
    vec3 extents;
    uint32_t pad4;

} OBBGPU;

/**
 * @brief GPU side structure for an OBB UBO. Used for LPV sampling. 
 * @see ::PROBE_VOLUME_UBO
 * @see render/dynamic.vert
 */
typedef struct ProbeVolumeUBO
{
    
    OBBGPU obb[LPV_SLOTS];
    
} ProbeVolumeUBO;

/**
 * @brief The UBO structure for pushing the closest ReflectionProbe%s to the main shader. 
 * @see ::RPROBE_UBO
 * @see render/dynamic.vert
 */
typedef struct RProbeUBO
{

    struct Align16
    {
        uint32_t val;
        uint32_t padding[3];
    } probe_indices[4];

    uint32_t probe_count;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    
} RProbeUBO;


/**
 * @brief The structure that stores lightmap uvs. 
 * @see ::LIGHTMAP_SSBO
 * @see add_all_meshes_to_buffer()
 */
typedef struct LightmapGPU
{
    vec4 uv;
} LightmapGPU;

/**
 * @brief The structure for the ::LIGHTMAP_SSBO 
 * @see add_all_meshes_to_buffer()
 */
typedef struct LightmapSSBO
{
    
    LightmapGPU * uvs;
    
} LightmapSSBO;

/**
 * @brief The structure for cluster AABB%s for clustered shading. 
 * @see ::CLUSTER_SSBO
 * @see render/init_cluster_aabbs.comp
 */
typedef struct ClusterAABBGPU
{

    vec4 min;
    vec4 max;

} ClusterAABBGPU;

/**
 * @brief The structure that we push clusters to when we cull them in clustered shading. 
 * @see ::CLUSTER_SSBO
 * @see render/cull_clusters.comp
 */
typedef struct ClusterGPU
{
    uint32_t offset;
    uint32_t count;
} ClusterGPU;


/**
 * @brief The atlas that UVIsland%s are allocated on in LightmapRect%s during UV lightmapping. 
 * @see SceneData.LightmapData
 * @see init_lightmap_atlas()
 * @see allocate_lightmap_rect()
 */
typedef struct LightmapAtlas{

    int width; /**<Width of the atlas.*/
    int height; /**<Height of the atlas.*/
    int block_size; /**<How much to allocate in one rect.*/
    unsigned char *occupied; /**<The map of what is currently occupied by LightmapRect%s.*/
    
} LightmapAtlas;

/**
 * @brief The rects that are slotted into thet LightmapAtlas during UV generation for UV lightmaps. 
 * @see SceneData.LightmapData
 * @see init_lightmap_atlas()
 * @see allocate_lightmap_rect()
 */
typedef struct LightmapRect {

    int u; /**<The width of the rect in UV space.*/
    int v; /**<The height of the rect in UV space.*/
    int w; /**<The width of the rect in pixels.*/
    int h; /**<The height of the rect in piexles.*/
    int padding; /**<How much padding to avoid light leaking.*/
    int success; /**<If the rect was allocated.*/

} LightmapRect;

/**
 * @brief The lightmap UVs stored on UVIsland%s per MeshFace. 
 * @see assign_uvs()
 */
typedef vec2 LightmapUV[3]; 


/**
 * @brief The UVIsland%s that are generated from contiguous MeshFace%s with similar planes for UV lightmapping. 
 * @see LightmapData.islands
 * @see assign_uvs()
 * @see build_uv_islands()
 */
typedef struct UVIsland
{

    int mesh_node; /**<The MeshNode this island belongs to.*/
    Vector tris; /**<size_t indices of the MeshFace%s in this island.*/
    vec3 normal; /**<The normal of the island.*/
    LightmapRect rect; /**<The rect that this island allocates.*/
    LightmapUV * uvs; /**<The uvs for each MeshFace on this islands.*/
    bool currently_rasterizing; /**<If we are going to rasterize this island on the next lightmap bake.*/
    bool valid; /**<If this island is valid to rasterize.*/
    vec3 centroid; /**<The centroid of the MeshFace%s on this island.*/
    
} UVIsland;

/**
 * @brief The metadata for the lightmap settings. 
 * @see Globals.scene_data
 * @see assign_uvs()
 * @see build_uv_islands()
 * @see init_lightmap_data()
 */
typedef struct LightmapData
{

    Vector islands;  /**<The UVIsland%s on the current lightmap.*/
    LightmapAtlas atlas; /**<The atlas settings for the lightmap.*/
    GLuint texture_id; /**<The texture ID for the current lightmap.*/
    bool sample_cubemap; /**<Unused - was used to toggle skybox sampling.*/
    
} LightmapData;


/**
 * @brief This bridges from MeshNode%'s MeshFace%s to their respective UVIsland%s. Used when uploading vertices to the GPU. 
 * @see MeshNode
 * @see assign_uvs()
 * @see build_uv_islands()
 * @see add_all_meshes_to_buffer()
 */
typedef struct UVMapping
{

    size_t uv_island; /**<What island inside of LightmapData.islands that this MeshFace refers to.*/
    size_t uv_idx; /**<The UV slot on the UVIsland.*/
    bool valid; /**<If this mapping is valid.*/
    
} UVMapping;

typedef struct Entity Entity;

/**
 * @brief Defines collision types with different behaviors and mesh setup and validation. 
 * @see PhysicsComponent
 * @see physics/physics.c
 * @see core/camera.c
 * @ingroup physics_group
 */
typedef enum CollisionType
{

    COL_TRIANGLE, /**<The triangle BVH is generated for this mesh and triangle tests are performed on it.*/
    COL_HULL, /**<Incremental hull generates a ConvexHull which is used for collision tests.*/
    COL_CAPSULE /**<A capsule is constructed via AABB dimensions and is used for collision tests.*/

} CollisionType;

/**
 * @brief Defines physics types with different behaviors in impulse resolution. 
 * @see PhysicsComponent
 * @see physics/physics.c
 * @see core/camera.c
 * @ingroup physics_group
 */
typedef enum PhysicsType
{

    PHYSICS_NONE, /**<Unused.*/
    PHYSICS_FULL, /**<All impulse resolution options applied.*/
    PHYSICS_NO_ROT, /**<Angular velocity is disabled.*/
    PHYSICS_GRAVITY_ONLY /**<Only gravity is applied and cannot be moved via external forces.*/

} PhysicsType;


/**
 * @brief The capsule data for collisions. 
 * @see PhysicsComponent
 * @see physics/physics.c
 * @see core/camera.c
 * @see init_PC()
 * @ingroup physics_group
 */
typedef struct CapsuleData
{

    vec3 a; /**<The bottom of the capsule (based on AABB).*/
    vec3 b; /**<The top of the capsule (based on AABB).*/
    float radius; /**<The radius of the capsule.*/

} CapsuleData;

/**
 * @brief Hull data that refers to ConvexHull%s of Mesh%es. Populated during traversal and passed into GJK.
 * @see PhysicsComponent
 * @see physics/physics.c
 * @see physics/GJK.c
 * @see core/camera.c
 * @see init_PC()
 * @ingroup physics_group
 */
typedef struct HullData
{

    vec3 * pts; /**<Points to ConvexHull.vertices*/
    size_t count; /**<Stored from ConvexHull.vertices.count*/
    HullFace * mfs; /**<Points to ConvexHull.hull_faces*/
    size_t face_count; /**<Stored from ConvexHull.hull_faces.count*/

} HullData;

/**
 * @brief The tagged union for collision type, stored in PhysicsComponent. 
 * @see PhysicsComponent
 * @see physics/physics.c
 * @see physics/GJK.c
 * @see core/camera.c
 * @see init_PC()
 * @ingroup physics_group
 */
typedef struct CollisionData
{
    CollisionType type;
    union {
        HullData hull;
        CapsuleData capsule;
    } data;
} CollisionData;


/**
 * @brief Entity Component used for physics resolution. 
 * @see ::PHYSICS_COMPONENTS
 * @see physics/physics.c
 * @see physics/GJK.c
 * @see core/camera.c
 * @see init_PC()
 * @see update_physics_components()
 * @see Entity
 * @ingroup physics_group
 */
struct PhysicsComponent
{

    size_t id; /**<The ID in ::PHYSICS_COMPONENTS*/
    bool free; /**<If this is an empty component.*/
    bool player_pc; /**<If this is the Player PhysicsComponent.*/
    bool at_rest; /**<If this component is at rest it does not update.*/
    int rest_counter; /**<The counter that decreases until at rest.*/

    /**
     * @name Physics Constants
     * @brief These are set on init and control how the component behaves.
     * @{
     */
    float mass; /**<Mass of the component.*/
    float restitution; /**<The restitution (bounce).*/
    float mu; /**<The friction constant for the component.*/
    vec3 center_of_mass; /**<COM based on the AABB.*/
    mat3 inverse_inertia; /**<IIA (currently just a calculated box)*/
    /** @} */

    vec3 vel; /**<Velocity.*/
    vec3 old_vel; /**<Last step's velocity.*/
    vec3 accel; /**<Acceleration from this step + acceleration from last step * ::ACCEL_BETA.*/
    vec3 angular_velocity; /**<Rotational velocity.*/
    vec3 old_angular; /**<Last step's angular velocity.*/
    mat3 inverse_world; /**<PhysicsComponent.inverse_inertia transformed by rotation into world space for the current step..*/

    AABB aabb; /**<Bounding box of the PC, determined based on Mesh or MeshNode.*/

    CollisionData cd; /**<Tagged union for collision options.*/
    PhysicsType physics_type; /**<Impulse resolution options.*/

    Entity * ent; /**<Which Entity this component belongs to.*/

};


/**
 * @brief Animation targets based on type to cast. 
 * @see AnimationTrack
 * @see AnimationComponent
 * @see core/animation.c
 */
typedef enum AnimationTargetType
{
    ANIM_TARGET_FLOAT, ANIM_TARGET_VEC3, ANIM_TARGET_INT, ANIM_TARGET_BOOL, ANIM_TARGET_MAX
} AnimationTargetType;

/**
 * @brief Indicates what type of structure the animation is targeting. 
 * @see AnimationTrack
 * @see AnimationComponent
 * @see core/animation.c
 */
typedef enum AnimationDataType
{
    ANIM_DATA_MN, ANIM_DATA_MESH, ANIM_DATA_AC, ANIM_DATA_PC, ANIM_DATA_ENT, ANIM_DATA_LIGHT, ANIM_DATA_ASSET_INSTANCE, ANIM_DATA_ASSET, ANIM_DATA_RPROBE, ANIM_DATA_MAX
} AnimationDataType;

/**
 * @brief Tagged union for the value stored in a keyframe. 
 * @see core/animation.c
 * @see AnimationTrack
 * @see AnimationComponent
 */
typedef struct AnimationKeyframe
{
    float time;
    union
    {
        float v;
        vec3 v3;
        int i;
    } val;
    
} AnimationKeyframe;


#define MAX_KEYFRAMES 64 /**<Maximum AnimationKeyframe%s in an AnimationTrack.*/

/**
 * @brief Tagged union for the value stored in a keyframe. 
 * @see core/animation.c
 * @see AnimationComponent
 */
typedef struct AnimationTrack
{

    AnimationTargetType type; /**<The type of the data stored on AnimationKeyframe.*/
    AnimationKeyframe kf[MAX_KEYFRAMES]; /**<The AnimationKeyframe%s for this track.*/
    int kf_count; /**<The amount of AnimationKeyframe%s.*/
    float current_time; /**<How long the animation has been running.*/
    float duration; /**<The duration of all of the AnimationKeyframe%s.*/
    bool loop; /**<If the animation loops.*/

    AnimationDataType data_type; /**<The type of data that is changed during animation.*/
    size_t data_index_offset; /**<The index of the structure we are targeting.*/
    size_t data_byte_offset; /**<The byte offset in the structure we are targeting.*/
    void * data; /**<Stored pointer to data to change during animation.*/
    AnimationTargetType completion_type; /**<Type of data that is modified on completion.*/
    void * completion_data; /**<Pointer to the data that is modified on completion.*/
    bool active; /**<If this animation is currently running.*/
    
} AnimationTrack;

#define MAX_TRACKS 64  /**<Maximum AnimationTrack%s in an AnimationComponent.*/

/**
 * @brief The animation component for interpolation between AnimationKeyframe%s. 
 * @see ::ANIMATION_COMPONENTS
 * @see core/animation.c
 * @see AnimationTrack
 * @see AnimationKeyframe
 * @see Entity
 */
typedef struct AnimationComponent
{

    AnimationTrack tracks[MAX_TRACKS]; /**<The AnimationTracks with individual values that occur simultaneously.*/
    int track_count;                   /**<The amount of AnimationTrack%s.*/
    bool free;                         /**<If the component is empty.*/
    Entity * ent;                      /**<The pointer to the Entity.*/
    
} AnimationComponent;

/**
 * @brief Types of Entity used for Filter%s. 
 * @see Entity
 * @see Filter
 */
typedef enum EntityType
{

    ENTITY_NONE,
    ENTITY_TRIGGER,
    ENTITY_FILTER
    
} EntityType;

/**
 * @brief Entity State%s that change behaviors and available reactions to Trigger%s. 
 * @see Entity
 * @see Filter
 * @see State
 */
typedef enum StateType
{

    ESTATE_NONE,
    ESTATE_PLAYER_INTERACT,
    ESTATE_COMBAT,
    ESTATE_TRIGGER,
    ESTATE_IDLE,
    ESTATE_DAMAGED,
    ESTATE_KILLED,
    ESTATE_DISABLED,
    ESTATE_MAX
    
} StateType;


/**
 * @brief Entity State%s that control what functions are available for an Entity.
 * @see Entity
 * @see Filter
 */
typedef struct State
{

    char name[256];
    void (*fn[ESTATE_MAX])(void*);
    
} State;



/**
 * @brief Entity structure that holds components and references to the data it owns.
 * @see ::ENTITIES
 * @see Filter
 */
struct Entity
{

    size_t id;                       /**<ID inside of ::ENTITIES.*/
    bool free;                       /**<If the Entity is empty.*/

    int mesh_node;                   /**<The top-most MeshNode that belongs to this Entity.*/
    int skin;                        /**<The GLTFSkin to be modified by this Entity (skeletal animations).*/
    AABB aabb;                       /**<The bounding box over all MeshNode%s for this Entity.*/

    EntityType type;                 /**<Type of entity for Filter%s.*/
    union                            
    {                                
        Trigger trigger;             
        Filter filter;               
    } as;                            

    State * state;                   /**<The current State that describes actions for this Entity.*/
    struct AssetInstance * instance; /**<The AssetInstance that spawned this Entity.*/
    PhysicsComponent * pc;           /**<The PhysicsComponent inside of ::PHYSICS_COMPONENTS.*/
    AnimationComponent * ac;         /**<The AnimationComponent inside of ::ANIMATION_COMPONENTS.*/
    
};

// TODO Probably expand this or change this number.
#define MAX_NODE_CHILDREN 128 /**<Maximum amount of direct MeshNode children to be stored in a MeshNode.*/

/**
 * @brief MeshNode%s that describe the position of Mesh%es, GLTF nodes for GLTFAnimation%s, or hold references to other MeshNode%s.
 * @see Mesh
 * @see Entity
 * @see PhysicsComponent
 * @see GLTFSkin
 * @see Globals
 */
typedef struct MeshNode {
    
    char name[256];  /**<The name generated on loading from file.*/
    bool active;     /**<If the MeshNode can be effected.*/
    bool free;       /**<If the MeshNode can be replaced.*/
    size_t id;       /**<The ID inside of Globals.mesh_nodes.*/
    size_t local_id; /**<The ID local to the original file structure.*/
    /**
     * @name Transformation Fields
     * @brief Builds the TRS for the node within the node graph.
     * @{
     */
    vec3 translation;
    vec4 rotation;
    vec3 scale;
    mat4 local_matrix;
    mat4 global_matrix;
    /** @} */

    int mesh;                        /**<The Mesh it references.*/
    int asset;                       /**<The Asset it comes from.*/
    int asset_instance;              /**<The AssetInstance it belongs to.*/
    int skin;                        /**<The GLTFSkin it controls.*/
    uint32_t skin_offset;            /**<The offset into the ::JOINT_SSBO, comes from DrawCommand.*/

    int parent;                      /**<The parent to this node, a global index into Globals.mesh_nodes.*/
    /**
     * @todo Remove this static array and replace it with a safer memory system for larger node graphs.
     */
    int children[MAX_NODE_CHILDREN]; /**<The children to this node, a global index into Globals.mesh_nodes.*/
    int child_count;                 /**<The amount of children.*/

    bool static_mesh;                /**<If this mesh is static (DrawArrays) or dynamic (DrawElements), used to indicate Lightmap status.*/

    size_t offset;                   /**<Offset into the ::MAIN_VBO.*/
    size_t count;                    /**<Count of vertices pushed into the ::MAIN_VBO.*/


    /**
     * @name Light Frame Data
     * @brief Per frame offsets and counts for nearby Light%s.
     * @todo Get rid of the point light stuff, no longer needed.
     * @{
     */
    size_t point_light_offset;
    size_t point_light_count;
    size_t directional_light_offset;
    size_t directional_light_count;
    size_t spot_light_offset;
    size_t spot_light_count;
    /** @} */

    bool skip_directional_uv_lighting; /**<Skips directional Light%s and rays go through them. Not in use.*/
    UVMapping * uv_map;                /**<The mapping between MeshNode%s and UVIsland%s for uv lightmapping.*/

    /**
     * @name SubMesh Rendering IDs
     * @brief Indices that correspond to DrawCommand%s and MeshNodeDataGPU used during upload_instance_data() for MDI.
     * @todo Expand SubMesh count, handle memory otherwise, and define it.
     * @{
     */
    uint32_t mesh_data_id[32];
    uint32_t command_id[32];
    /** @} */
    
    Entity * ent;                 /**<The Entity this MeshNode refers to (all children point to the same Entity as parent).*/
    CollisionType collision_type; /**<The type of collision to apply for the PhysicsComponent that may not have been created yet.*/
    PhysicsType physics_mode;     /**<The type of physics to apply for the PhysicsComponent that may not have been created yet.*/
    PhysicsComponent * pc;        /**<The physics controller initialized based on data from this MeshNode.*/

    /**
     * @brief The AABB of the MeshNode and its children.
     * @note This is used for PhysicsComponents on MeshNode%s without Mesh%es.
     */
    AABB aabb;
        
} MeshNode;

/**
 * @brief Sections of Mesh%es that are divided based on Material for batching and DrawCommand ordering purposes.
 * @see Mesh
 * @see MeshNode
 * @see load_gltf()
 * @see add_all_meshes_to_buffer()
 */
typedef struct SubMesh
{
    
    size_t material;     /**<The Material.*/
    size_t first_vertex; /**<The first Point in Mesh.vertices that this SubMesh refers to.*/
    size_t vertex_count; /**<The amount of Point%s that this SubMesh refers to.*/
    size_t first_index;  /**<The first index that this SubMesh refers to (Mesh.tris count * 3).*/
    size_t index_count;  /**<The amount of indices that this SubMesh refers to: Mesh.tris -> MeshFace.indices.*/
    size_t lm_offset;    /**<The starting index in ::LIGHTMAP_DATA that this SubMesh points to.*/
    size_t draw_offset;  /**<Currently the same index as SubMesh.lm_offset.*/
    size_t command_id;   /**<The DrawCommand this is assigned to.*/
    size_t mesh_data_id; /**<The MeshNodeDataGPU this is assigned to.*/

} SubMesh;

/**
 * @brief Adjacent triangles for each MeshFace on a Mesh.
 * @see build_adjacency_map()
 * @see walk_coplanar_adjacency()
 */
typedef struct TriangleAdjacency
{
    Vector adjacent_indices; /**<size_t indices.*/
    
} TriangleAdjacency;

/**
 * @brief A Mesh with geometry data that is either initialized as static or dynamic and split into SubMesh%es.
 * @see MeshNode
 * @see Globals
 * @see init_mesh()
 */
typedef struct Mesh
{

    char name[256];   /**<The Mesh name assigned on load.*/
    size_t id;        /**<The ID into Globals.meshes.*/
    size_t asset_id;  /**<The Asset ID into Globals.assets.*/
    bool bvh_prim;    /**<This Mesh has a BVH computed.*/
    bool static_mesh; /**<This Mesh is static and must create a BVH or if not it must perform incremental_hull.*/
    AABB aabb;        /**<The AABB of the mesh.*/

    size_t offset; /**<The offset into ::MAIN_VBO.*/
    size_t count;  /**<The count pushed into ::MAIN_VBO.*/

    /**
     * @name Geometry
     * @brief Geometry vectors that are initialized when loading from file and on incremental hull.
     * @{
     */
    Vector submeshes; /**<The SubMesh%es split via Material%s.*/
    Vector hulls;     /**<The ConvexHull%s generated if this Mesh is dynamic.*/
    Vector vertices;  /**<The Point%s.*/
    Vector tris;      /**<The MeshFace%s.*/
    /** @} */

    TriangleAdjacency * tri_adjacency; /**<Adjacent triangles allocated per MeshFace.*/

    /**
     * @name BVH
     * @brief BVH fields filled if this Mesh is static.
     * @todo We may be able to remove tri_aabbs.
     * @{
     */
    size_t * bvh_indices; /**<Indices for the BVH, sorted as the BVH is generated.*/
    Vector nodes;         /**<BVHNode%s.*/
    Vector tri_aabbs;     /**<AABB%s for each triangle inside of this Mesh.*/
    /** @} */
    
} Mesh;


/**
 * @brief Where Material textures are saved upon loading from file.
 * @see Material
 * @see Globals
 */
typedef struct Image
{

    char path[256];      /**<The path to the Image.*/
    char mime_type[256]; /**<Unused, refer to GLTF spec.*/
    char name[256];      /**<The name of the Image file.*/
    int channels;        /**<The amount of channels for an Image (RGBA).*/
    unsigned char * buf; /**<If available, the raw bytes of the Image.*/
    GLuint i;            /**<The handle for the texture.*/
    GLuint64 handle;     /**<The handle for the bindless texture.*/
    int width;           
    int height;          
    bool sRGB;           /**<To load in this Image as sRGB.*/

    GLint min_filter;    /**<Refer to OpenGL texture loading.*/
    GLint mag_filter;    /**<Refer to OpenGL texture loading.*/
    GLint wrap_s;        /**<Refer to OpenGL texture loading.*/
    GLint wrap_t;        /**<Refer to OpenGL texture loading.*/
    
} Image;

/**
 * @brief The structure for Material%s when loaded in from file.
 * @see Material
 * @see GLTFDoc
 */
typedef struct GLTFMaterial {

    char name[64];

    float base_color_factor[4];
    float metallic_factor;     
    float roughness_factor;    
    float emissive_factor[3];  
    
    int base_color_tex; 
    int metallic_roughness_tex;
    int normal_tex;            
    int occlusion_tex;         
    int emissive_tex;          
    
    float normal_scale;       
    float occlusion_strength; 
    
    AlphaMode alpha_mode;
    float alpha_cutoff;       
    bool double_sided;

    float absorption_coefficients[AUDIO_NUM_BANDS];
    float scattering_coefficients[AUDIO_NUM_BANDS];

    float emissive_strength;
    int fwd_shader;
    int gbuffer_shader;
    bool override_transparent;

} GLTFMaterial;

/**
 * @brief The raw bytes of a GLTF file.
 * @see GLTFDoc
 */
typedef struct {

    char *uri;
    int byteLength;
    unsigned char *data;

} GLTFBuffer;

/**
 * @brief The views of a GLTF file.
 * @see GLTFDoc
 */
typedef struct {
    int buffer;
    int byteOffset;
    int byteLength;
    int byteStride;
} GLTFBufferView;

/**
 * @brief The accessors of a GLTF file.
 * @see GLTFDoc
 */
typedef struct {
    int bufferView;
    int byteOffset;
    int componentType;
    int count;
    char type[8];     
} GLTFAccessor;

/**
 * @brief The primitives of a GLTF file.
 * @see GLTFDoc
 */
typedef struct {
    int position;     
    int normal;       
    int tangent;
    int texcoord0;    
    int joints0;      
    int weights0;     
    int indices;      
    int material;     
} GLTFPrimitive;

/**
 * @brief The Mesh%es of a GLTF file.
 * @see Mesh
 * @see GLTFDoc
 */
typedef struct {
    GLTFPrimitive *prims;
    int primCount;
} GLTFMesh;

/**
 * @brief Types of Animation interpolation.
 * @see Animation
 * @see GLTFDoc
 */
typedef enum {LINEAR, STEP, CUBICSPLINE } AnimationInterpolation;

/**
 * @brief What an GLTFAnimationChannel is targeting.
 * @see Animation
 * @see GLTFDoc
 */
typedef enum { TARGET_TRANSLATION, TARGET_ROTATION, TARGET_SCALE, TARGET_WEIGHTS }AnimationPath;

/**
 * @brief Engine-side sampler version of GLTFAnimationSampler with the field allocated.
 * @see Animation
 * @see GLTFDoc
 */
typedef struct {
    // count is times, elem is values
    int count;        
    float *times;     

    int elem_count;   
    float *values;    
    AnimationInterpolation interpolation;
} AnimationSampler;

/**
 * @brief The individual interpolation of an Animation.
 * @see Animation
 * @see GLTFDoc
 */
typedef struct {
    int input;  
    int output;  
    int interpolation;
} GLTFAnimationSampler;

/**
 * @brief The channels of an Animation.
 * @see Animation
 * @see GLTFDoc
 */
typedef struct {
    int sampler;      
    int target_node;  
    AnimationPath path;
} GLTFAnimationChannel;


/**
 * @brief The GLTF structure of an Animation.
 * @see Animation
 * @see GLTFDoc
 */
 
typedef struct {
    char name[64];
    int sampler_count;
    GLTFAnimationSampler *samplers;
    int channel_count;
    GLTFAnimationChannel *channels;
} GLTFAnimation;


/**
 * @brief The structure of an Skeletatal Animation loaded in from GLTFAnimation.
 * @see GLTFAnimation
 * @see GLTFDoc
 */
typedef struct {
    
    char name[256];
    int asset_id;
    int sampler_count;
    AnimationSampler * samplers;
    int channel_count;
    GLTFAnimationChannel * channels;
    
} Animation;


/**
 * @brief The maximum amount of animation indices per GLTFSkin.
 * @todo Remove this maximum or increase it.
 */
#define MAX_ANIMATION_INDICES 256 



/**
 * @brief The structure of an Skeletatal Animation loaded in from GLTFAnimation.
 * @see MeshNode
 * @see Globals
 * @see update_skin_matrices()
 * @see GLTFDoc
 */
typedef struct GLTFSkin {
    
    char name[256];                               /**<The name of the skin from load.*/
    bool free;                                    /**<If this skin is free.*/
    size_t id;                                    /**<The ID of this skin in Globals.skins.*/
    int joint_count;                              /**<The amount of joints.*/
    int * joints;                                 /**<The joints for skeletal animating.*/
    mat4 * final_mat;                             /**<The transformation after update_skin_matrices().*/
    mat4 * inverse_bind;                          /**<The inverse_bind that is computed on load_gltf().*/
    int ibm_accessor;                             /**<Which GLTFAccessor this refers to.*/
    int animation_indices[MAX_ANIMATION_INDICES]; /**<Animation indices into Asset.*/
    int animation_indices_count;                  /**<The amount of Animation indices.*/
    int ssbo_offset;                              /**<The offset into ::JOINT_SSBO.*/
    
} GLTFSkin;

/**
 * @brief The GLTFTexture before it is processed.
 * @see GLTFDoc
 */
typedef struct GLTFTexture
{
    
    int source;
    int sampler;

} GLTFTexture;

/**
 * @brief The Image before it is processed.
 * @see GLTFDoc
 */
typedef struct GLTFImage
{
    
    char uri[256];
    int buffer_view;
    char mime_type[256];
    char name[256];

} GLTFImage;

/**
 * @brief The sampler for the Image before it is processed.
 * @see GLTFDoc
 */
typedef struct GLTFSampler
{
    
    int mag_filter;
    int min_filter;
    int wrap_s;
    int wrap_t;
    char name[256];
    
    
} GLTFSampler;

/**
 * @brief The entire GLTF after it is parsed via load_gltf().
 */
typedef struct {
    
    GLTFBuffer * buffers;   
    int bufferCount;
    GLTFBufferView * views;     
    int viewCount;
    GLTFAccessor * accessors; 
    int accessorCount;
    GLTFMesh * meshes;    
    int meshCount;
    GLTFMaterial * materials;    
    int material_count;
    MeshNode * nodes;
    int node_count;
    GLTFSkin * skins;
    int skin_count;
    GLTFAnimation * animations;
    int animation_count;
    GLTFTexture * textures;
    int texture_count;
    GLTFImage * images;
    int image_count;
    GLTFSampler * samplers;
    int sampler_count;
    
} GLTFDoc;

/**
 * @brief A instance of an Asset that retains its node structure but can be duplicated into multiple instances that are independent.
 * @see Asset
 * @see ::ASSET_INSTANCES
 * @see init_new_asset_instance()
 * @warning @todo Very important: The first_skin is not actually in use and needs to be resolved to use skin_indices instead in init_new_asset_instance()
 */
typedef struct AssetInstance
{

    size_t id;             /**<The ID in ::ASSET_INSTANCES.*/
    size_t first_skin;     /**<Refer to todo*/
    size_t skin_count;     /**<The amount of GLTFSkin%s in this instance.*/

    int asset_id;          /**<The Asset ID in Globals.assets.*/
    size_t node_count;     /**<The amount of MeshNode%s in this instance.*/
    size_t * node_indices; /**<The indices of MeshNode%s spawned from this instance inside of Globals.mesh_nodes.*/
    size_t * skin_indices; /**<The indices of GLTFSkin%s spawned from this instance inside of Globals.skins.*/

    /**
     * @name Model Matrix
     * @brief This transform is applied before all transforms in update_scene_nodes(). It is the transform for the entire instance.
     * @{
     */
    vec3 tr;               
    vec4 rot;              
    vec3 scale;            
    mat4 model;            
    /** @} */

    bool free;             /**<Indicates an empty slot in ::ASSET_INSTANCES.*/
    AABB aabb;             /**<The AABB over each MeshNode via Asset.*/
    
} AssetInstance;


/**
 * @brief An Asset after it is loaded in from file into Globals.assets. It is a template for AssetInstance%s.
 * @see AssetInstance
 * @see Globals.assets
 */
typedef struct Asset
{

    char path[256];         /**<The path to the Asset.*/
    Vector mesh_indices;    /**<int - Indices into Globals.meshes which indicate what Mesh%es are within this Asset.*/
    MeshNode * nodes;       
    size_t node_count;      
    GLTFSkin * skins;       
    size_t skin_count;      
    Animation * animations; 
    size_t animation_count; 

    int asset_id;           /**<The ID into Globals.assets.*/
    AABB aabb;              /**<AABB over each MeshNode for this Asset.*/
    
} Asset;

/**
 * @brief Simply the size we use to allocate the ::INSTANCE_VBO (unused).
 */
typedef struct InstanceData
{

    mat4 model;
    ivec2 point_lights;
    ivec2 directional_lights;
    ivec2 spot_lights;
    int skin_offset;
    
} InstanceData;

/**
 * @brief The structure used to sort unique submeshes by materials, and shaders in the main render pass.
 * @see create_and_sort_draw_commands()
 * @see upload_instance_data()
 * @todo Possibly sort 128 keys instead of 64
 */
typedef struct DrawCommand
{

    uint8_t gbuffer_shader;            /**<The shader for the ::DEPTH_PRE_PASS_SHADER.*/
    uint8_t fwd_shader;                /**<The shader in ::DYNAMIC_SHADER.*/
    uint8_t material;                  /**<The Material ID into Globals.materials.*/
    uint8_t opaque;                    /**<How opaque a Material is.*/
    uint8_t static_mesh;               /**<If it is a static mesh.*/
    uint16_t mesh_id;                  /**<The Mesh.*/
    uint16_t skin_offset;              /**<The offset into ::JOINT_SSBO.*/

    /**
     * @name Light Offsets
     * @brief The offsets and counts into the Light arrays based on which are effecting this MeshNode.
     * @{
     */
    uint16_t point_light_offset;       
    uint16_t point_light_count;        
    uint16_t directional_light_offset; 
    uint16_t directional_light_count;  
    uint16_t spot_light_offset;        
    uint16_t spot_light_count;         
    /** @} */

    uint32_t start_index;              /**<Where the indices / vertices start for this SubMesh in ::MAIN_VBO.*/
    uint32_t index_count;              /**<How many indices in this SubMesh.*/
    uint32_t lm_offset;                /**<Lightmap offset into ::LIGHTMAP_DATA / ::LIGHTMAP_SSBO.*/
    uint32_t mn_id;                    /**<The MeshNode.*/
    uint32_t sm_id;                    /**<The SubMesh.*/
    uint32_t array_offset;             /**<The offset into ::INDIRECT_ARRAYS_COMMAND_BUFFER.*/
    uint32_t element_offset;           /**<The offset into ::INDIRECT_ELEMENTS_COMMAND_BUFFER.*/
    uint64_t sort_key;                 /**<The key that is sorted in create_and_sort_draw_commands().*/
    mat4 matrix;                       /**<The transformation matrix of the MeshNode.*/
    
} DrawCommand;




/**
 * @name EPAWorkspace Maximum Sizes
 * @brief Sizes of statically sized arrays for EPA.
 * @see physics/GJK.c
 * @{
 */
#define MAX_EPA_VERTICES 1024
#define MAX_EPA_FACES 1024
#define MAX_EPA_INDICES 1024
#define MAX_EPA_HORIZON_EDGES 1024
#define MAX_EPA_HORIZON_COUNTS 1024
#define MAX_EPA_VISIBLE_FACES_INDICES 1024
/** @} */

/**
 * @brief A workspace used for EPA during collision detection. 
 * @see epa()
 * @see physics/GJK.c
 * @see CollisionPoints
 */
typedef struct EPAWorkspace
{

    SupportVertex vertices[MAX_EPA_VERTICES];                 /**<The vertices of the current polyhedron.*/
    size_t vertices_count;                                    /**<The amount of vertices.*/
    MeshFace faces[MAX_EPA_FACES];                            /**<The faces of the current polyhedron.*/
    size_t faces_count;                                       /**<The amount of faces.*/
    int indices[MAX_EPA_INDICES];                             /**<Indices into EPAWorkspace.faces that describe which faces are still used (when faces are removed they are simply deleted from this array).*/
    size_t indices_count;                                     /**<The amount of current faces.*/
    HorizonEdge horizon_edges[MAX_EPA_HORIZON_EDGES];         /**<The amount of unique edges that are not shared between visible faces.*/
    size_t horizon_edges_count;                               /**<The amount of horizon edges.*/
    int horizon_counts[MAX_EPA_HORIZON_COUNTS];               /**<The amount of times each horizon edge appears (if it only appears once then it is evaluated).*/
    size_t horizon_counts_count;                              /**<The amount of horizon counts.*/
    int visible_faces_indices[MAX_EPA_VISIBLE_FACES_INDICES]; /**<The faces that are visibile from the newest SupportVertex.*/
    size_t visible_faces_count;                               /**<The amount of visible faces found.*/
    
} EPAWorkspace;

/**
 * @brief A node inside of the min_heap of an AStarWorkspace. 
 * @see a_star_aportal()
 * @see AStarWorkspace
 */
typedef struct OpenNode
{

    int i;      /**<The index into the array of whatever is being traversed.*/
    float g;    /**<The g score: current score of the path.*/
    float f;    /**<The f score: score from neighbor to end.*/
    int parent; /**<The index of the parent of this neighbor.*/

} OpenNode;


#define MAX_PORTALS 256 /**<The maximum amount of portals that are allocated to AStarWorkspace.path*/

/**
 * @brief A running stack of nodes in a path that tracks where they came from so they can be retraced at the end. 
 * @see a_star_aportal()
 */
typedef struct AStarStateStack
{

    float g; /**<The g score: current score of the path.*/
    float f; /**<The f score: score from neighbor to end.*/
    int came_from; /**<The index into the array of traversed items that this node came from.*/
    int run_id; /**<Makes sure that no old stack items are used. A new run is started on each a_star call.*/

} AStarStateStack;

/**
 * @brief The workspace for conducting a-star pathfinding/. 
 * @see a_star_aportal()
 * @todo Expand path size or make it reallocatable because the current limit is minimal.
 */
typedef struct AStarWorkspace
{

    AStarStateStack * stack; /**<The stack that tracks where each node connects so that it can be backtracked.*/
    int * path;              /**<The path indices that are returned.*/
    int path_count;          /**<The final path count.*/

    Vector min_heap;         /**<OpenNode - the heap of nodes that are pushed and popped until there are none left or it reaches the end.*/
    int current_run;         /**<The current run indicates what AStarStateStack entries we can use.*/
    
} AStarWorkspace;

/**
 * @brief The Player. 
 * @details There can currently only be one Player, located in ::player.
 * @see update_camera()
 * @see process_input()
 * @see update_audio_occlusion()
 * @see audio_update()
 */
typedef struct Player
{

    float radius;          /**<The radius of the Player body.*/
    float height;          /**<The height of the Player body.*/
    bool sprinting;        /**<Currently unused flag for sprinting.*/
    bool noclip;           /**<Enables flying (but not collisions).*/
    Camera * cam;          /**<Player Camera.*/
    PhysicsComponent * pc; /**<PhysicsComponent in ::PHYSICS_COMPONENTS.*/
    vec3 pos;              /**<Currently anchors the Skeleton.*/
    Skeleton skeleton;     /**<Player Skeleton.*/
    Mesh mesh;             /**<The Mesh for the ConvexHull used for collisions (an AABB).*/
    
} Player;


static const ivec3 DEFAULT_APV_RES = {6, 6, 6}; /**<When APV%s are spawned, they use this.*/
static const ivec3 DEFAULT_LPV_RES = {PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH}; /**<What LPV%s should always be at to match texture dimensions.*/


/**
 * @brief The ProbeVolume%s for APV%s and LPV%s, controlled by OBB%s. 
 * @see load_audio_pv()
 * @see APV
 * @see LPV
 * @ingroup aprobe_group
 */
typedef struct ProbeVolume
{

    size_t byte_offset; /**<The byte offset into the binary file storing this volume.*/
    size_t size;        /**<The byte size of this volume.*/
    OBB obb;            /**<The OBB that defines the size.*/
    ivec3 resolution;   /**<The resolution in X, Y, Z.*/
    
} ProbeVolume;

/**
 * @brief A ProbeVolume for AudioProbe%s. 
 * @see load_audio_pv()
 * @see AudioProbe
 * @ingroup aprobe_group
 */
typedef struct APV
{
    
    int pv_id;                 /**<Which ProbeVolume in SceneData.audio_probe_volumes.*/
    AudioProbe * audio_probes; /**<The probes in the volume, loaded in from binary.*/

} APV;


/**
 * @brief A ProbeVolume for indirect lighting probes. 
 * @see load_lighting_pv()
 * @see bake_lighting_probe_volume()
 */
typedef struct LPV
{
    
    int pv_id;       /**<The ProbeVolume in SceneData.lighting_probe_volume.*/

    /**
     * @brief The raw SH data. 
     * @details P = ::LPV_PADDED_COUNT, 3 = [R, G, B], SH = ::SH_MAX
     * \f$(P * 3 * SH)\f$.
     */
    float * lprobes;

} LPV;

/**
 * @brief The BVH for Light%s. 
 * @see init_light_bvh()
 * @see bake_lightmap()
 */
typedef struct LightBVH
{

    Vector nodes;         /**<Each node of the BVH.*/
    size_t * indices;     /**<The indices sorted when initialized.*/
    size_t indices_count; /**<The amount of indices on init (some lights are omitted).*/
    Vector lights;        /**<The Light%s that were created (emissive triangles become ::AREA_LIGHT%S).*/
    
} LightBVH;

/**
 * @brief The data for the current scene. Stored in Globals. 
 * @see init()
 * @see init_globals()
 * @see scene_load()
 */
typedef struct SceneData
{

    char name[256]; /**<Modifiable name of the scene.*/
    AABB aabb;      /**<AABB over the entire scene.*/

    /**
     * @name BVH fields
     * @brief The BVH over each MeshNode in the scene, updated each frame.
     * @{
     */
    Vector bvh_nodes;
    int bvh_indices_count;
    size_t * bvh_indices;
    /** @} */


    Vector reflection_probes;      /**<ReflectionProbe%s in the scene.*/
    Vector audio_probe_volumes;    /**<All of the AudioProbe% volumes, regardless of if they have been loaded yet.*/
    Vector lighting_probe_volumes; /**<All of the lighting probe volumes, regardless of if they have been loaded yet.*/

    LightBVH light_bvh; /**<The LightBVH, required for baking lighting.*/

    /**
     * @name Loaded LPV%s and APV%s.
     * @brief The loaded ProbeVolume%s for both AudioProbe%s and lighting probes.
     * @see load_lighting_pv().
     * @see load_audio_pv().
     * @{
     */
    APV LOADED_AUDIO_PV[MAX_AUDIO_PROBE_VOLUMES];       /**<Load slots for Audio ProbeVolume%s.*/
    bool LOADED_AUDIO_PV_FLAG[MAX_AUDIO_PROBE_VOLUMES]; /**<Flags that indicate if the slot is loaded.*/
    LPV LOADED_LIGHTING_PV[LPV_SLOTS];                  /**<Load slots for lighting ProbeVolume%s.*/
    bool LOADED_LIGHTING_PV_FLAG[LPV_SLOTS];            /**<Flags that indicate if the slot is loaded.*/
    char LPV_PATH[256];                                 /**<Path to binary file with lighting ProbeVolume%s.*/
    FILE * LPV_FILE;                                    /**<FILE pointer to the opened binary file.*/
    char APV_PATH[256];                                 /**<Path to binary file with Audio ProbeVolume%s.*/
    FILE * APV_FILE;                                    /**<FILE pointer to the opened binary file.*/
    /** @} */

    LightmapData lightmap_data; /**<The loaded lightmap and lightmap configuration for the scene.*/
   
    
} SceneData;


/**
 * @brief Registry for folders that are used for dropdowns in the editor UI. 
 * @see render_editor_menu()
 * @see init_file_directory_registries()
 * @see render_file_directory_dropdown()
 */
typedef struct FileDirectoryRegistry
{

    Vector names;          /**<The name of each file in the directory.*/
    char suffix[256];      /**<The suffix to append to the end of the file when searching.*/
    bool search_suffix;    /**<If there is a suffix.*/
    char directory[256];   /**<Path to directory for the file.*/
    int current_selection; /**<The file that is currently selected (starts at 0).*/
    nk_bool open;          /**<If the directory is currently open in a dropdown.*/
    
} FileDirectoryRegistry;



/**
 * @brief All of these track slider states in the editor UI so that Undo%s with current values can be pushed when sliders start.
 * @addtogroup editor_group
 * @{
 */
typedef struct MainEditorMenu
{

    NKSlider res_width;
    NKSlider res_height;

    NKSlider focal_length;
    NKSlider fstop;
    NKSlider sensor_height;
    NKSlider blur_radius;

} MainEditorMenu;

typedef struct LightEditorMenu
{
    
    NKSlider color_r;
    NKSlider color_g;
    NKSlider color_b;

    NKSlider x;
    NKSlider y;
    NKSlider z;

    NKSlider rx;
    NKSlider ry;
    NKSlider rz;

    NKSlider intensity;
    NKSlider threshold;
    
    NKSlider cone_angle;
    NKSlider penumbra;
    
    NKSlider light_type;

    NKSlider mfactor;

    NKSlider mfactor_enabled;

    NKSlider texels_per_unit;
    
    NKSlider lm_iterations;
    NKSlider probe_iterations;
    NKSlider ao_iterations;
    NKSlider ao_threshold;

    NKSlider lm_aabbx;
    NKSlider lm_aabby;
    NKSlider lm_aabbz;

    NKSlider aabbe_x;
    NKSlider aabbe_y;
    NKSlider aabbe_z;

    NKSlider rp_aabbx;
    NKSlider rp_aabby;
    NKSlider rp_aabbz;

    NKSlider rp_aabbe_x;
    NKSlider rp_aabbe_y;
    NKSlider rp_aabbe_z;
    
    NKSlider rp_fade_x;
    NKSlider rp_fade_y;
    NKSlider rp_fade_z;
    
    NKSlider rp_capposx;
    NKSlider rp_capposy;
    NKSlider rp_capposz;

    NKSlider center_pos;
    
} LightEditorMenu;

typedef struct LProbeEditorMenu
{

    NKSlider OBB_sliders[9];
    
} LProbeEditorMenu;

typedef struct AProbeEditorMenu
{

    NKSlider iterations;
    NKSlider bounces;

    NKSlider volume;
    NKSlider wet;
    
    NKSlider aportal_pos[3];
    
    NKSlider emitter_pos[3];
    NKSlider emitter_vol;
    
    NKSlider apv_res[3];
    
    NKSlider OBB_sliders[9];
    
} AProbeEditorMenu;

typedef struct UIElementEditorMenu
{

    NKSlider x;
    NKSlider y;
    NKSlider z;

    NKSlider sx;
    NKSlider sy;

    NKSlider radius;
    NKSlider angle;

    NKSlider sizex;
    NKSlider sizey;

    NKSlider layer;
    
    NKSlider color;

    NKSlider fill;
    
    NKSlider shadow_dist;
    NKSlider shadow_dirx;
    NKSlider shadow_diry;

    NKSlider cpadl;
    NKSlider cpadr;
    NKSlider cpadu;
    NKSlider cpadd;
    NKSlider cpad;
    NKSlider cscrollx;
    NKSlider cscrolly;

    
    NKSlider sliderx;
    NKSlider slidery;
    
    NKSlider slidermin;
    NKSlider slidermax;
    
    NKSlider sliderlen;

    NKSlider sliderscrollscale;
    
} UIElementEditorMenu;

typedef struct AssetInstanceEditorMenu
{
    
    NKSlider model[10];
    
} AssetInstanceEditorMenu;

typedef struct MeshNodeEditorMenu
{
    
    NKSlider model[10];
    
} MeshNodeEditorMenu;

typedef struct TextBoxEditorMenu
{

    NKSlider translation;
    NKSlider angle;
    NKSlider scale;
    NKSlider text_size;
    NKSlider layer;
    NKSlider color;
    NKSlider fill;
    NKSlider shadow_dist;
    NKSlider shadow_dirx;
    NKSlider shadow_diry;
    
    
} TextBoxEditorMenu;

/** @} */


/**
 * @brief Global settings and structures for each system in the engine. 
 * @see SceneData
 * @see init_globals()
 * @see scene_load()
 * @todo rename noclip
 * @todo change world min and size
 * @todo remove probe spacing - should no longer be needed, use resolution
 */
typedef struct Globals
{
    
    bool lights_dirty;             /**<If lights need to be reuploaded.*/
    bool noclip;                   /**<If collision is enabled.*/
    bool keyboard_locked;          /**<If the keyboard is unlocked to register text events in the editor UI.*/
    int msaa_samples;              /**<How many samples our multisampled textures are using.*/
    vec3 world_min;                /**<Unused but uploaded to main shader.*/
    vec3 world_size;               /**<Unused but uploaded to main shader.*/
    ivec3 probe_tex_size;          /**<The texture size for lighting probes.*/
    float probe_spacing;           /**<The spacing between lighting probes.*/

    /**
     * @name Engine Structure Arrays.
     * @brief Each main system that is in Vector%s is here.
     * @{
     */
    Vector meshes;                 /**<Mesh%es.*/
    Vector mesh_nodes;             /**<MeshNode%s.*/
    Vector skins;                  /**<GLTFSkin%s.*/
    Vector assets;                 /**<Asset%s.*/
    Vector materials;              /**<Material%s.*/
    Vector images;                 /**<Image%s.*/
    Vector lights;                 /**<Light%s.*/
    Vector audio_portals;          /**<AudioPortal%s.*/
    Vector vertex_layouts;         /**<VertexLayout%s.*/
    AudioProbe * audio_probes;     /**<A scratch buffer used when baking AudioProbe%s.*/
    /** @} */

    uint32_t * mesh_light_indices; /**<Buffer used when calling determine_light_visibility_per_mesh.*/
    SceneData scene_data;          /**<SceneData that is more-so scene specific.*/

} Globals;

/**
 * @brief OpenGL shader Uniforms. 
 * @see core/shaders.c
 */
typedef struct Uniform
{
    GLuint location;
    char name[256];
} Uniform;

/**
 * @brief OpenGL ShaderProgram%s: vertex, fragment, geometry, and compute. 
 * @see core/shaders.c
 * @todo Expand uniform count or make it dynamic?
 */
typedef struct ShaderProgram
{

    GLuint program;       /**<Program handle.*/
    uint32_t perm;        /**<Which permutation of a shader branch this is.*/
    char name[256];       /**<Name of the shader.*/
    char vert_path[256];  /**<Path to vertex shader (or compute shader).*/
    char geom_path[256];  /**<Path to geometry shader.*/
    char frag_path[256];  /**<Path to fragment shader.*/

    Uniform uniforms[32]; /**<Uniform%s for shader data.*/
    
} ShaderProgram;

/**
 * @brief An unused abstraction for VertexLayout%s. 
 * @note In an older engine, there was the capability to design your own layouts and render passes. This functionality is not necessary in this engine, and so there is only one VertexLayout currently.
 */
typedef struct VertexLayout {
    
    char name[256];
    size_t attrib_count;
    GLsizei vertex_size;   
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLenum buffer_usage; // GL_STATIC_DRAW
    int buffer_override; // for static buffers
    GLenum buffer_target; // GL_ARRAY_BUFFER
    void * buffer_override_data;
    size_t current_offset;
    size_t current_capacity;
    size_t current_index_offset;
    size_t current_index_capacity;
    Vector meshes;
    
} VertexLayout;

extern Globals GLOBALS; /**<Engine Globals*/
extern Camera cam; /**<The Camera attached to the Player.*/
extern Camera LIGHT_CAM[MAX_DIRECTIONAL_LIGHTS]; /**<::DIRECTIONAL_LIGHT Camera%s for shadowmaps*/
    

#endif 
