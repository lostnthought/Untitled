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
#include "nuklear.h"
#include <omp.h>
#include "nuklear_sdl_gl3.h"
#include "miniaudio.h"
#include <fftw3.h>
#include "uthash.h"
#include <stdatomic.h>
#include "utilities/types.h"


#define PI 3.14159265358979323846

#define NUM_BANDS 4
#define FOV 90
#define VIEW_NEAR 0.1
#define VIEW_FAR 1000.0

#define MAX_INTERP_APROBES 4
#define MAX_AUDIO_EMITTERS 1024
#define SAMPLE_RATE 48000



#define IR_SAMPLE_RATE 48000

#define EDC_SAMPLE_RATE 48000

#define SPEED_OF_SOUND 343.0f 
#define MAX_APROBE_SECONDS 1
#define MAX_IR_SAMPLES (IR_SAMPLE_RATE * MAX_APROBE_SECONDS)
#define MAX_EDC_SAMPLES (EDC_SAMPLE_RATE * MAX_APROBE_SECONDS)
#define MAX_SAMPLES (SAMPLE_RATE * MAX_APROBE_SECONDS)
#define AUDIO_PROBE_BINS 48000
#define AUDIO_KERNEL_SIZE 31
#define AUDIO_KERNEL_CENTER 15
#define AUDIO_NUM_BANDS 6

const static float AUDIO_BAND_FREQ[] = {125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, 4000.0f};


#define MAX_AUDIO_EMITTERS_PER_ENTITY 24

#define MAX_NUKLEAR_EVENTS 256
#define MAX_SDL_EVENTS 512
#define MAX_MESH_NODES 1024
#define MAX_MESHES 512
#define MINIMUM_LIGHT_VISIBILITY_DIST 0.01
#define MAX_LIGHTS_PER_MESH 4
#define MAX_INSTANCES 1024
#define MAX_DRAW_COMMANDS 512

#define MAX_DIRECTIONAL_LIGHTS 4

#define MAX_ENTITY_STATES 256

#define DEFAULT_TEXTURES 5

#define EDITOR_LIGHT_SIZE 0.5f

#define EDITOR_RPROBE_SIZE 0.5f

#define EDITOR_AUDIO_EMITTER_SIZE 0.5f

#define EDITOR_APROBE_SIZE 0.1f

#define EDITOR_APORTAL_SIZE 0.2f


#define PERFORMANCE_OVERLAY_TEXT_BOX_COUNT 13

#define BUFFER_FRAMES 3

#define MAX_JOINT_MATRICES 512

#define UBERSHADER_VERSIONS 17
#define GBUFFER_VERSIONS 1

#define MAX_WORLD_HEIGHT 1000
#define MAX_WORLD_WIDTH 1000
#define MAX_WORLD_DEPTH 1000

#define MAX_MATERIALS 256
#define MAX_PLAYER_CONTACTS 10

#define LIGHTMAP_WIDTH 2048
#define LIGHTMAP_HEIGHT 2048

#define RPROBE_WIDTH 128
#define RPROBE_HEIGHT 128

#define LPF_ORDER 2



#define DEFAULT_POINT_LIGHT_SHADOWS_SIZE 128

#define MAX_POINT_LIGHT_SHADOWS_CMA 16

#define MAX_INSTANCES_PER_FRUSTUM 1024


#define SH_MAX 9
#define DEFAULT_SHADOW_MAP_SIZE 2048

#define MAX_RPROBE_MIPS 1

#define MAX_RPROBES_PER_FRAME 4

#define FBM_TEXTURE_SIZE 128

#define SSAO_KERNEL_SIZE 16


#define ACCEL_BETA 0.005f



#define GRID_X 16

#define GRID_Y 9

#define GRID_Z 24

#define TOTAL_CLUSTERS (GRID_X * GRID_Y * GRID_Z)

#define MAX_LIGHTS 1024

#define MAX_LIGHTS_PER_CLUSTER 12


#define MAX_PROBE_VOLUMES 4
#define MAX_AUDIO_PROBE_VOLUMES 4
#define PROBE_VOLUME_WIDTH 12
#define PROBE_VOLUME_HEIGHT 12
#define PROBE_VOLUME_DEPTH 12
#define PROBE_VOLUME_DEPTH_PADDED (PROBE_VOLUME_DEPTH + 2)
#define LPV_COUNT (PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH)
#define LPV_PADDED_COUNT (PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH_PADDED)
#define LPV_SLOTS 6
#define LPV_GLOBAL_SLOTS 2
#define GLOBAL_PROBE_VOLUME_WIDTH 32
#define GLOBAL_PROBE_VOLUME_HEIGHT 32
#define GLOBAL_PROBE_VOLUME_DEPTH 32

#define MAX_UI_MENU_NAMES 128



typedef struct {
    uint32_t offset; 
    uint32_t count;  
} Cluster;



typedef struct {
    uint64_t start;
    uint64_t freq;
} Timer;

static inline void start_timer(Timer* t) {
    t->start = SDL_GetPerformanceCounter();
    t->freq = SDL_GetPerformanceFrequency();
}

static inline double end_timer(Timer* t) {
    Uint64 end = SDL_GetPerformanceCounter();
    return (double)(end - t->start) * 1000.0 / (double)t->freq; 
}


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

typedef enum PostprocessToggles
{
    PP_MBLUR, PP_DOF, PP_BLOOM, PP_NOISE, PP_ACES, PP_LUT, PP_MAX
} PostprocessToggles;

typedef enum PostprocessDView
{
    PPDVIEW_DEFAULT,
    PPDVIEW_DEPTH,
    PPDVIEW_VEL,
    PPDVIEW_BOKEH,
    PPDVIEW_LM,
    PPDVIEW_MAX
} PostprocessDView;

static const char * PPDVIEW_NAMES[] =
{
    "PP_DVIEW_DEFAULT",
    "PPDVIEW_DEPTH",
    "PPDVIEW_VEL",
    "PPDVIEW_BOKEH",
    "PPDVIEW_LM",
    "PPDVIEW_MAX",
};

typedef enum FullscreenOptions
{

    FSOPT_FULLSCREEN,
    FSOPT_BORDERLESS,
    FSOPT_WINDOWED,
    FSOPT_MAX

} FullscreenOptions;

static const char * FULLSCREEN_OPTION_NAMES[] =
{
    "FULLSCREEN",
    "BORDERLESS",
    "WINDOWED"
};



typedef struct PhysicsComponent PhysicsComponent;



typedef struct ReflectionProbe
{

    AABB aabb;
    vec3 fade_margin;
    vec3 capture_pos;
    int cubemap_idx;
    
} ReflectionProbe;

#define NUM_PARTITIONS_MAX 96
#define NUM_PARTITIONS 95
typedef fftwf_complex * PartitionTableChannel[NUM_PARTITIONS];
typedef PartitionTableChannel PartitionTable[2];

typedef struct StereoAudio
{

    PartitionTable * left_ir;

} StereoAudio;

typedef struct AudioProbeWeights
{

    int count;
    int APV_id;
    int probes[MAX_INTERP_APROBES];
    float weights[MAX_INTERP_APROBES];
    
} AudioProbeWeights;


#define FDN_LINES 4

typedef struct {
    float buffer[FDN_LINES][8192]; 
    int write_pos[FDN_LINES];
    int delay_lengths[FDN_LINES]; 
    float feedback_gain;  
} SimpleFDN;


typedef struct AudioPortal
{

    vec3 pos;
    Vector adjacent_portals; // int
    
} AudioPortal;



#define PARTITION_BUFFERS 4
typedef struct HRTFThreadData
{

    SDL_Thread * t;
    SDL_mutex * mutex;
    SDL_mutex * ptr_swap_mutex;
    SDL_cond * work_cond;

    SDL_SpinLock ir_swap_lock;

    vec3 last_pos;
    vec3 last_fwd;

    bool stop_requested;
    bool new_probe_ready;

    AudioProbeWeights w;

    PartitionTable tables[PARTITION_BUFFERS];

    PartitionTable * FRONT;

    int current_probe; 
    
    
} HRTFThreadData;

typedef struct AudioScratchBuffers
{

    SDL_mutex * aprobe_mutex;

    int block_size;
    int padded_len;
    int complex_len;
    int final_ir_len;
    int ir_len;
    int complex_ir_len;
    float * HRTF_IR_IN;
    
    fftwf_plan fwd_plan;
    fftwf_plan static_fwd_plan;
    fftwf_plan IR_fwd_plan;
    fftwf_plan IR_bwd_plan;
    fftwf_plan bwd_plan;

    float * IR_SUM_L;
    float * IR_SUM_R;
    fftwf_complex * IR_SUM_L_FWD;
    fftwf_complex * IR_SUM_R_FWD;
    float * STATIC_HRTF_L[SH_MAX];
    float * STATIC_HRTF_R[SH_MAX];
    fftwf_complex * STATIC_HRTF_L_FWD[SH_MAX];
    fftwf_complex * STATIC_HRTF_R_FWD[SH_MAX];
    float * IR_ROT[SH_MAX]; 
    fftwf_complex * IR_ROT_FWD[SH_MAX];

} AudioScratchBuffers;


typedef struct ConvolverNode
{

    ma_node_base base;
    ma_mutex lock;
    int block_size;
    int padded_len;
    int complex_len;
    // StereoAudio LR;
    PartitionTable * ir;
    PartitionTable * prev_ir;


    fftwf_plan fwd_plan;
    fftwf_plan bwd_plan;

    fftwf_complex * MONO_FWD;
    fftwf_complex * AUDIO_HISTORY[NUM_PARTITIONS_MAX];
    
    fftwf_complex * PREV_SUM_L;
    fftwf_complex * PREV_SUM_R;

    fftwf_complex * SUM_L;
    fftwf_complex * SUM_R;

    int history_head;
    
    float * PREV_LEFT_OUT;
    float * PREV_RIGHT_OUT;
    float * LEFT_OUT;
    float * RIGHT_OUT;

    int tail_len;
    float * PREV_TAIL_LEFT;
    float * PREV_TAIL_RIGHT;
    float * TAIL_LEFT;
    float * TAIL_RIGHT;


    int crossfade_pos;
    bool crossfading;
    
    
} ConvolverNode;


typedef struct AudioEmitter
{

    char path[256];
    uint32_t id;
    ma_sound sound_obj;
    ma_lpf_node lpf_node;
    float current_cutoff;
    float base_volume;
    float current_volume;
    float current_distance;
    vec3 pos;
    vec3 virtual_pos;
    bool is_loaded;
    bool is_occludable;
    bool is_loop;
    bool free;
    
} AudioEmitter;


typedef enum GBufferTextureNames
{
     GVEL, GUV, GNORMAL, GPOMDEPTH, GALBEDO, GEXTRA
} GBufferTextureNames;

typedef enum DepthMode
{
     DEPTH_MODE_NONE, DEPTH_MODE_NO_HW, DEPTH_MODE_HW
} DepthMode;


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
    GLuint i;
    DepthMode dm;
    bool multisampled;
    int samples;
    bool fixed_sample_locations;
    
} GLTexture;

typedef struct GLRenderbuffer
{
    
    GLuint i;
    GLint internal_format;
    GLsizei w, h, d;
    bool multisampled;
    int samples;
} GLRenderbuffer;
typedef struct Framebuffer
{
    GLuint fbo;
    GLTexture texture[6];
    GLRenderbuffer renderbuffers[6];
    int texture_count;
    int rbo_count;
    GLTexture depth_texture;
    GLRenderbuffer depth_rbo;
    int w, h;
    
} Framebuffer;

typedef enum SelectionMode
{

    SELMODE_MN,
    SELMODE_LIGHT,
    SELMODE_LPV,
    SELMODE_RPROBE,
    SELMODE_ENTITY,
    SELMODE_TEXTBOX,
    SELMODE_UIELEMENT,
    SELMODE_APV,
    SELMODE_APORTAL,
    SELMODE_AEM,
    SELMODE_ASSETINSTANCE,
    
} SelectionMode;



typedef struct EditorSettings
{

    DebugView current_debug_view;
    PostprocessDView pp_dview;
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

    SelectionMode SELMODE;

    bool element_drag_mode;
    bool text_box_drag_mode;

    

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

    bool dof_expanded;
    float DOF_fstop;
    float DOF_focal_length;
    float DOF_sensor_height;
    float DOF_max_blur_radius;

    bool wireframe_toggled;

    int res_width;
    int res_height;

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

    RayHit hit;

    vec3 rotation_axis;
    float rotation_angle;
    

    bool audio_probe_volume_overlay;
    bool lighting_probe_volume_overlay;

    int lm_iterations;
    int probe_iterations;
    float texels_per_uv_unit;
    float ao_threshold;
    int ao_iterations;
    int selected_r_probe;

    bool audio_probes_overlay;
    bool audio_probe_location_override;
    AABB audio_probe_override_aabb;
    int audio_probe_iterations;
    int audio_probe_bounces;

    float render_framerate_target;

    float mfactor;
    float reverb_wet;
    float volume;
    bool mfactor_enabled;
    
    
    float obb_axis_x_angle;
    float obb_axis_y_angle;
    float obb_axis_z_angle;

    

    AABB lm_aabb;

    size_t undo_stack_max_byte_size;
    

} EditorSettings;

typedef struct InstanceVisibilityGPU
{
    
    uint32_t count;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    uint32_t visibility[MAX_INSTANCES];
} InstanceVisibilityGPU;


typedef struct DrawArraysIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t first;
    uint32_t baseInstance;
    uint32_t instanceOffset;
    uint32_t pad0;
} DrawArraysIndirectCommand;

typedef struct ArraysIndirectGPU
{
    DrawArraysIndirectCommand indirect[MAX_MESHES];
} ArraysIndirectGPU;

typedef struct DrawElementsIndirectCommand {
    uint32_t count;
    uint32_t instanceCount;
    uint32_t firstIndex;
    uint32_t baseVertex;
    uint32_t baseInstance;
    uint32_t instanceOffset;
} DrawElementsIndirectCommand;

typedef struct ElementsIndirectGPU
{
    DrawElementsIndirectCommand indirect[MAX_MESHES];
} ElementsIndirectGPU;

typedef struct GlobalInstanceGPU
{
    uint32_t visibility[MAX_MESHES * MAX_INSTANCES];
} GlobalInstanceGPU;


typedef struct MeshNodeDataGPU
{
    vec4 min;
    vec4 max;
    vec4 center;
    vec4 extents;
    uint32_t instance_count;
    uint32_t static_mesh;
    uint32_t lm_offset;
    uint32_t material;
    uint32_t first;
    uint32_t count;
    uint32_t pad0;
    uint32_t pad1;
} MeshNodeDataGPU;

typedef struct MeshNodeSSBO
{
    float focal_distance;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    MeshNodeDataGPU mns[MAX_MESH_NODES];
} MeshNodeSSBO;

typedef struct TransformGPU
{

    uint32_t mn_id;
    uint32_t command_id;
    uvec2 directional_lights;
    uvec2 spot_lights;
    uint32_t skin_offset;
    uint32_t material;
    mat4 tr;
    
} TransformGPU;


typedef struct Filter Filter;
struct Filter
{
    
    void * val;
    size_t byte_size;
    bool (*fn)(void*, Filter*);
    
} ;

typedef enum TriggerType
{

    TRIGGER_NONE,
    TRIGGER_ONESHOT,
    TRIGGER_CONTINUOUS

} TriggerType;

typedef struct Trigger
{

    TriggerType type;
    OBB obb;
    void (*fn)(void*);
    
} Trigger;



typedef struct CollisionPoints
{

    vec3 a; // furthest point of A into B
    vec3 b; // furthest point of B into A
    size_t index_a;
    size_t index_b;
    vec3 normal;
    vec3 tangent;
    float d; // depth (length of B-A)
    bool collision;

    PhysicsComponent * pcA;
    int mnA;

    PhysicsComponent * pcB;
    int mnB;

    int incident_face;
    int reference_face;
    int edge_a;
    int edge_b;
    
    float normal_impulse;
    float bias_impulse;
    float tangent_impulse;

    
} CollisionPoints;


#define MAX_MANIFOLDS 64
typedef struct Manifold
{

    PhysicsComponent *a;
    int mnA;
    PhysicsComponent *b;
    int mnB;

    CollisionPoints contacts[4];
    int count;

    bool processed;
    bool free;

    float deepest;

} Manifold;

// each lightmap texel gets a patch
// triangles link to uv lightmap triangles
// on import build adjacency map for triangles
// then find coplanar faces within some epsilon and then create surfaces with tri lists
// from there create uv islands for each surface and wrap
// then bake lighting
    
typedef struct LightmapTexel
{
    
    vec3 pos;
    vec3 normal;

    vec3 emission;
    vec3 albedo;

    vec3 lighting;

    float current_energy;

    float b0, b1, b2; // barycentrics
    int tri;
    bool active;
    int mesh_node;
    
} LightmapTexel;



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



typedef struct ConvexHull
{

    Vector vertices;

    Vector faces;


    Vector hull_faces;
    vec3 centroid;
    AABB aabb;

} ConvexHull;

typedef struct TriangleGroup {
    vec3 normal;
    vec3 ref_point;
    Vector indices;
} TriangleGroup;


typedef struct BVHNode
{

    int left;
    int right;
    int parent;
    int index;
    int next;

    AABB aabb;

    size_t first_index;
    size_t count;

    float flux;
    float emissive_flux;
    
} BVHNode;


typedef struct TriangleAdjacency
{

    Vector adjacent_indices;
    
} TriangleAdjacency;


typedef struct MainVertex
{
    vec3 pos;
    vec3 normal;
    vec4 tangent;
    ivec4 joint_indices;
    vec4 weights;
    vec2 uv;
} MainVertex;

typedef struct LightmapVertex
{
   
    vec3 pos;
    vec3 normal;
    vec2 uv;
    vec2 lm_uv;
    int material_id;
    int tri_id;
    
} LightmapVertex;


typedef struct DebugVertex
{
    vec3 pos;
    vec3 color;
    
} DebugVertex;

typedef struct TextVertex
{
    vec2 pos;
    vec2 uv;
    vec2 center;
    vec4 color;
    vec4 fill;
    uint32_t state;
    uint32_t uID;
} TextVertex;

typedef struct UIVertex
{
    vec2 pos;
    uint32_t uID;
} UIVertex;

typedef struct ShadowmapUBO
{
    mat4 proj[MAX_DIRECTIONAL_LIGHTS];
    mat4 view[MAX_DIRECTIONAL_LIGHTS];
    
} ShadowmapUBO;


typedef struct CamData
{
    vec4 cam_pos;
    vec4 cam_vel;
    mat4 proj;
    mat4 unjittered_proj;
    mat4 view;
    mat4 inv_proj;
    mat4 unjittered_inv_proj;
    mat4 inv_view;
} CamData;

typedef struct Render
{
    ivec4 resolution;

} RenderData;

typedef struct Spring
{
    float stiff;
    float damp;
    vec3 velocity;
    vec3 pos;
    vec3 target;

    float stiff_rot;
    float damp_rot;
    vec3 rotation;
    vec3 angular_vel;
    vec3 target_rot;
    
} Spring;

typedef struct LegIK
{

    bool stepping;
    bool stepping_off_ledge;
    vec3 pos;
    vec3 start_pos;
    vec3 target_pos;
    float duration;
    float time;
    
} LegIK;

typedef struct Skeleton
{


    float pelvis_height;
    float hip_height;
    float neck_height;
    float pelvis_hip_diff;
    float neck_pelvis_diff;
    float upper_leg_length;
    float lower_leg_length;
    vec3 q_left;
    vec3 q_right;
    vec3 p_left;
    vec3 p_right;
    vec3 left_hip_pos;
    vec3 right_hip_pos;

    Spring pelvis;
    Spring neck;
    LegIK left_leg;
    LegIK right_leg;
    
} Skeleton;


typedef struct Camera
{
    vec3 forward, right, up, dir, target;
    float yaw, pitch, roll;
    CamData cam_data;
    bool on_ground;
    
} Camera;

typedef struct Probe
{
    vec4 rgb;
    vec4 pos;
    uint32_t active;
    uint32_t pad0;
    uint32_t pad1;
    uint32_t pad2;
    
} Probe;

#define MAX_PROBES 1024
typedef struct ProbeData
{

    Probe probes[MAX_PROBES];
    
} ProbeData;

typedef struct RProbe
{
    vec3 center;
    uint32_t pad0;
    vec3 min;
    uint32_t pad1;
    vec3 max;
    uint32_t pad2;
    
} RProbe;

#define MAX_RPROBES 256
typedef struct RProbeData
{

    RProbe probes[MAX_RPROBES];
    
} RProbeData;


typedef struct IRFrame
{

    float SHCoefficients[SH_MAX][AUDIO_NUM_BANDS];
    
} IRFrame;
typedef struct {
    float b0, b1, b2, a0, a1, a2;
    float x1, x2, y1, y2;
} Biquad;

typedef struct AudioProbe
{

    size_t ir_length;
    vec3 pos;
    float rt60[AUDIO_NUM_BANDS];
    float slope[AUDIO_NUM_BANDS];
    float (* final_ir)[SH_MAX];
    
} AudioProbe;


typedef struct AudioProbeGPU
{
    vec3 pos;
    uint32_t pad0;
} AudioProbeGPU;



typedef struct AudioSampleGPU
{

    vec3 arrival_dir;
    float time_of_flight;
    vec3 pos;
    uint32_t valid;
    float energy[6];
    uint32_t pad0;
    uint32_t pad1;
    float abs_energy[6];
    uint32_t pad4;
    uint32_t pad5;

} AudioSampleGPU;


typedef enum LightType : uint32_t
{
    
    POINT_LIGHT,
    DIRECTIONAL_LIGHT,
    SPOT_LIGHT,
    AREA_LIGHT,
    MAX_LIGHT_TYPES

} LightType;

typedef struct Light
{
    
    vec3 pos;
    uint32_t entity_id;
    
    vec3 dir;
    float cone_angle;

    vec3 rgb;
    float penumbra;

    float intensity;
    LightType light_type;
    float inner_angle;
    float outer_angle;

    float radius;
    float threshold;
    AABB area;

    bool sm_dirty;
    uint32_t layer;

    int mesh;
    int tri;

    float lum;
    float power;

    vec3 A;
    vec3 B;
    vec3 C;
    vec3 normal;
    float surface_area;
    
} Light;

typedef enum MenuNames
{
    MENU_MAIN, MENU_MN, MENU_MATERIAL, MENU_LIGHT, MENU_ENT, MENU_TEXT, MENU_UI, MENU_AUDIO, MENU_LPROBE, MENU_AINST, MENU_MAX
} MenuNames;

typedef struct LightGPU
{
    
    vec3 pos;
    uint32_t entity_id;
    
    vec3 dir;
    float outer_cos;

    vec3 rgb;
    float cos_diff;

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

typedef struct EmissiveTriangle
{

    vec3 A;
    uint32_t pad0;
    vec3 B;
    uint32_t pad1;
    vec3 C;
    uint32_t pad2;
    vec3 normal;
    uint32_t pad3;
    vec3 emissive;
    uint32_t pad4;
    float area;
    float strength;
    float cdf;
    uint32_t pad5;

} EmissiveTriangle;



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

typedef struct MaterialData
{
    
    MaterialGPU materials[MAX_MATERIALS];
    
} MaterialData;

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

typedef struct VertexData
{
    
    VertexGPU * vertices;
    
} VertexData;

typedef struct TriGPU
{

    uvec3 indices;
    uint32_t material_id;
    vec3 normal;
    uint32_t mesh_id;
    uint32_t bvh_index;
    uint32_t tri_idx;
    uint32_t pad1;
    uint32_t pad2;
    
    VertexGPU v0;
    VertexGPU v1;
    VertexGPU v2;
    
} TriGPU;

typedef struct TriData
{
    
    TriGPU * tris;
    
} TriData;

typedef struct MeshGPU
{

    uint32_t mesh_node_id;
    uint32_t mesh_id;

    uint32_t node_count;
    uint32_t node_offset;
    uint32_t vertex_offset;
    uint32_t vertex_count;
    uint32_t tri_offset;
    uint32_t tri_count;
    uint32_t bvh_index;
    uint32_t is_static;
    uint32_t skip_directional_light;
    uint32_t pad2;

    mat4 global_matrix;
    mat4 inverse_global_matrix;
    
} MeshGPU;

typedef struct MeshData
{
    
    MeshGPU * meshes;
    
} MeshData;

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

typedef struct BVHGPU
{

    BVHNodeGPU * nodes;
    
} BVHGPU;

typedef struct SHDataGPU
{
    vec3 pos;
    uint32_t pad0;
    vec4 coefficients[SH_MAX];
} SHDataGPU;

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

typedef struct ProbeVolumeUBO
{
    
    OBBGPU obb[LPV_SLOTS];
    
} ProbeVolumeUBO;

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


typedef struct LightmapGPU
{
    vec4 uv;
} LightmapGPU;

typedef struct LightmapSSBO
{
    
    LightmapGPU * uvs;
    
} LightmapSSBO;

typedef struct ClusterAABBGPU
{
    vec4 min;
    vec4 max;
} ClusterAABBGPU;
typedef struct ClusterGPU
{
    uint32_t offset;
    uint32_t count;
} ClusterGPU;


typedef struct LightmapAtlas{

    int width;
    int height;
    int block_size; 
    unsigned char *occupied; 
    
} LightmapAtlas;

typedef struct {

    int u, v, w, h;
    int padding;
    int success;

} LightmapRect;

typedef struct {

    vec2 uvs[3];

    int min_y, max_y;
    int min_x, max_x;


} LightmapUV;


typedef struct UVIsland
{

    int mesh_node;
    Vector tris; // size_t
    vec3 normal;
    LightmapRect rect;
    LightmapUV * uvs;
    bool currently_rasterizing;
    bool valid;
    vec3 centroid;
    
} UVIsland;

typedef struct LightmapData
{


    Vector islands; // UVIsland
    LightmapAtlas atlas;
    vec4 *buffer;

    int tri_count; // the count when the lightmap was created. used to free the right amount of uvs later.
    int tri_capacity;
    GLuint texture_id;
    
    bool sample_cubemap;
    
} LightmapData;


// each mesh needs submeshes to draw with different materials
typedef struct UVMapping
{

    size_t uv_island;
    size_t uv_idx;
    bool valid;
    
} UVMapping;

typedef struct Entity Entity;


typedef enum CollisionType
{
    COL_TRIANGLE, COL_HULL, COL_CAPSULE
} CollisionType;
typedef enum PhysicsType
{
    PHYSICS_NONE, PHYSICS_FULL, PHYSICS_NO_ROT, PHYSICS_GRAVITY_ONLY
} PhysicsType;


typedef struct CapsuleData
{
    vec3 a, b; float radius;
    float height;
} CapsuleData;

typedef struct HullData
{
    vec3 * pts;
    size_t count;
    HullFace * mfs;
    size_t face_count;
} HullData;

typedef struct CollisionData
{
    CollisionType type;
    union {
        HullData hull;
        CapsuleData capsule;
    } data;
} CollisionData;


struct PhysicsComponent
{

    size_t id;
    bool free;
    bool at_rest;
    bool player_pc;

    int rest_counter;
    
    float mass;
    float restitution;
    float mu;

    vec3 vel;
    vec3 old_vel;
    vec3 accel;
    vec3 angular_velocity;
    vec3 old_angular;
    vec3 center_of_mass;
    mat3 inverse_inertia;
    mat3 inverse_world;

    vec3 corrected_position;

    AABB aabb;

    CollisionData cd;
    PhysicsType physics_type;

    Entity * ent;
    // size_t entity_id;


};


typedef enum AnimationTargetType
{
    ANIM_TARGET_FLOAT, ANIM_TARGET_VEC3, ANIM_TARGET_INT, ANIM_TARGET_BOOL, ANIM_TARGET_MAX
} AnimationTargetType;

typedef enum AnimationDataType
{
    ANIM_DATA_MN, ANIM_DATA_MESH, ANIM_DATA_AC, ANIM_DATA_PC, ANIM_DATA_ENT, ANIM_DATA_LIGHT, ANIM_DATA_ASSET_INSTANCE, ANIM_DATA_ASSET, ANIM_DATA_RPROBE, ANIM_DATA_MAX
} AnimationDataType;

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


#define MAX_KEYFRAMES 64
typedef struct AnimationTrack
{

    AnimationTargetType type;
    AnimationKeyframe kf[MAX_KEYFRAMES];
    int kf_count;
    float current_time;
    float duration;
    bool loop;

    AnimationDataType data_type;
    size_t data_index_offset;
    size_t data_byte_offset;
    void * data;
    AnimationTargetType completion_type;
    void * completion_data;
    bool active;
    
} AnimationTrack;

    


#define MAX_TRACKS 64
typedef struct AnimationComponent
{

    
    AnimationTrack tracks[MAX_TRACKS];
    int track_count;
    bool free;
    Entity * ent;
    
    
} AnimationComponent;

typedef enum EntityType
{
    ENTITY_NONE,
    ENTITY_TRIGGER,
    ENTITY_FILTER
    
} EntityType;



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


typedef struct State
{

    char name[256];
    void (*fn[ESTATE_MAX])(void*);
    
} State;


struct Entity
{

    size_t id;
    bool free;

    int mesh_node;
    int skin;
    AABB aabb;

    EntityType type;
    union
    {
        Trigger trigger;
        Filter filter;
    } as;
    
    State * state;
    
    struct AssetInstance * instance;
    PhysicsComponent * pc;
    AnimationComponent * ac;
    
};

#define MAX_NODE_CHILDREN 128
typedef struct MeshNode {
    
    char name[256];
    bool active;
    bool free;
    size_t id;
    size_t local_id;
    int parent;        
    vec3 translation;
    vec4 rotation;
    vec3 scale;
    mat4 local_matrix;
    mat4 global_matrix;
    int mesh;
    int asset;
    int asset_instance;
    int skin;
    int children[MAX_NODE_CHILDREN];
    int child_count;
    CollisionType collision_type;
    bool static_mesh;
    bool root;

    size_t offset;
    size_t count;

    PhysicsType physics_mode;

    size_t point_light_offset;
    size_t point_light_count;
    size_t directional_light_offset;
    size_t directional_light_count;
    size_t spot_light_offset;
    size_t spot_light_count;

    bool skip_directional_uv_lighting;

    UVMapping * uv_map;

    uint32_t mesh_data_id[32];
    uint32_t command_id[32];
    uint32_t skin_offset;

    
    Entity * ent;
    PhysicsComponent * pc;
    AABB aabb;
    
    
} MeshNode;

typedef struct SubMesh
{
    
    size_t material;
    size_t first_vertex;
    size_t vertex_count;
    size_t first_index;
    size_t index_count;
    size_t lm_offset;
    size_t draw_offset;
    size_t command_id;
    size_t mesh_data_id;

} SubMesh;

typedef struct Mesh
{

    char name[256];

    bool bvh_prim;
   
    size_t asset_id;
    
    size_t id;

    AABB aabb;

    size_t offset;

    size_t count;

    bool static_mesh;

    Vector submeshes;

    Vector hulls;

    Vector nodes;

    Vector vertices;

    Vector tris;

    TriangleAdjacency * tri_adjacency;

    Vector tri_aabbs;

    // leaf indices is a massive array with all of the leaves' indices
    // it is unordered and only leaves index into it
    Vector leaf_indices;

    // this is a working sorted array to make the algorithm faster in terms of splitting the bvh evenly
    size_t * bvh_indices;


    
} Mesh;




typedef struct Image
{

    char path[256];
    char mime_type[256];
    char name[256];
    int channels;
    unsigned char * buf;
    GLuint i;
    GLuint64 handle;
    int width;
    int height;
    bool sRGB;

    GLint min_filter;
    GLint mag_filter;
    GLint wrap_s;
    GLint wrap_t;
    
} Image;

#define MAX_TEX_LABELS 600
#define LABEL_LEN 256
typedef struct ComboLabel {

    char buffer[MAX_TEX_LABELS][LABEL_LEN];
    const char *ptrs[MAX_TEX_LABELS];
    int selected;
  
    int item_to_id[MAX_TEX_LABELS];
    int item_count;

} ComboLabel;

typedef enum AlphaMode
{
    ALPHA_OPAQUE, ALPHA_MASK, ALPHA_BLEND
} AlphaMode;


typedef struct Material
{

    char name[256];
    int fwd_shader;
    int gbuffer_shader;

    // ambient
    vec4 ka;
    // diffuse
    vec4 kd;
    // specular
    vec4 ks;
    // emissive
    vec4 ke;

    float absorption_coefficients[AUDIO_NUM_BANDS];

    float scattering_coefficients[AUDIO_NUM_BANDS];

    int diffuse_tex;
    int normal_tex;
    int emissive_tex;
    int arm_tex;
    int ao_tex;
    int mr_tex;

    float normal_scale;
    float ao_strength;
    float metallic;
    float roughness;
    AlphaMode alpha_mode;
    float alpha_cutoff;

    float emissive_strength;
    
    
    bool double_sided;
    bool override_transparent;
    bool override_arm;
    
} Material;


typedef struct {
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

typedef struct {
    char *uri;
    int byteLength;
    unsigned char *data;
} GLTFBuffer;

typedef struct {
    int buffer;
    int byteOffset;
    int byteLength;
    int byteStride;
} GLTFBufferView;

typedef struct {
    int bufferView;
    int byteOffset;
    int componentType;
    int count;
    char type[8];     
} GLTFAccessor;

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

typedef struct {
    GLTFPrimitive *prims;
    int primCount;
} GLTFMesh;

typedef enum {LINEAR, STEP, CUBICSPLINE } AnimationInterpolation;
typedef enum { TARGET_TRANSLATION, TARGET_ROTATION, TARGET_SCALE, TARGET_WEIGHTS }AnimationPath;

typedef struct {
    // count is times, elem is values
    int count;        
    float *times;     

    int elem_count;   
    float *values;    
    AnimationInterpolation interpolation;
} AnimationSampler;

typedef struct {
    int input;  
    int output;  
    int interpolation;
} GLTFAnimationSampler;

typedef struct {
    int sampler;      
    int target_node;  
    AnimationPath path;
} GLTFAnimationChannel;

typedef struct {
    char name[64];
    int sampler_count;
    GLTFAnimationSampler *samplers;
    int channel_count;
    GLTFAnimationChannel *channels;
} GLTFAnimation;

typedef struct {
    
    float time;
    void (*fn)(void);
    
} AnimationEvent;

typedef struct {

    AnimationEvent event;
    char name[64];
    
} AnimationEventDefinition;

typedef struct {
    
    char name[256];
    int asset_id;
    int sampler_count;
    AnimationSampler * samplers;
    int channel_count;
    GLTFAnimationChannel * channels;
    
} Animation;


#define MAX_ANIMATION_INDICES 256
typedef struct GLTFSkin {
    
    char name[256];
    bool free;
    size_t id;
    int joint_count;
    int * joints;         
    mat4 * final_mat;
    mat4 * inverse_bind;  
    int ibm_accessor;
    bool is_idled;
    int animation_indices[MAX_ANIMATION_INDICES];
    int animation_indices_count;

    int ssbo_offset;
    
} GLTFSkin;

typedef struct GLTFTexture
{
    
    int source;
    int sampler;

} GLTFTexture;

typedef struct GLTFImage
{
    
    char uri[256];
    int buffer_view;
    char mime_type[256];
    char name[256];

} GLTFImage;

typedef struct GLTFSampler
{
    
    int mag_filter;
    int min_filter;
    int wrap_s;
    int wrap_t;
    char name[256];
    
    
} GLTFSampler;

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

typedef struct AssetInstance
{

    size_t id;
    size_t first_skin;
    size_t skin_count;

    int asset_id;
    int root_id;
    size_t first_node;
    size_t node_count;
    size_t * node_indices;
    size_t * skin_indices;

    vec3 tr;
    vec4 rot;
    vec3 scale;
    mat4 model;

    bool free;
    AABB aabb;
    
} AssetInstance;


typedef struct Asset
{

    // int indices into mesh_nodes
    char path[256];

    bool is_obj;
    Vector mesh_indices;

    MeshNode * nodes;
    size_t node_count;
    GLTFSkin * skins;
    size_t skin_count;
    Animation * animations;
    size_t animation_count;

    int skin_id;
    int asset_id;
    int root_id;
    AABB aabb;
    
} Asset;


typedef struct InstanceData
{

    mat4 model;
    ivec2 point_lights;
    ivec2 directional_lights;
    ivec2 spot_lights;
    int skin_offset;
    
} InstanceData;

typedef struct DrawCommand
{

    uint64_t sort_key;
    uint8_t gbuffer_shader;
    uint8_t fwd_shader;
    // figure out how to sort 128
    uint8_t material;
    uint8_t opaque;
    uint16_t mesh_id;
    uint8_t static_mesh;
    uint32_t start_index;
    uint32_t index_count;
    uint32_t lm_offset;
    uint16_t point_light_offset;
    uint16_t point_light_count;
    uint16_t directional_light_offset;
    uint16_t directional_light_count;
    uint16_t spot_light_offset;
    uint16_t spot_light_count;
    uint32_t mn_id;
    uint32_t sm_id;
    mat4 matrix;
    uint16_t skin_offset;
    uint32_t array_offset;
    uint32_t element_offset;
    
} DrawCommand;




#define MAX_EPA_VERTICES 1024
#define MAX_EPA_FACES 1024
#define MAX_EPA_INDICES 1024
#define MAX_EPA_HORIZON_EDGES 1024
#define MAX_EPA_HORIZON_COUNTS 1024
#define MAX_EPA_VISIBLE_FACES_INDICES 1024

typedef struct EPAWorkspace
{

    SupportVertex vertices[MAX_EPA_VERTICES];
    size_t vertices_count;
    MeshFace faces[MAX_EPA_FACES];
    size_t faces_count;
    int indices[MAX_EPA_INDICES];
    size_t indices_count;
    HorizonEdge horizon_edges[MAX_EPA_HORIZON_EDGES];
    size_t horizon_edges_count;
    int horizon_counts[MAX_EPA_HORIZON_COUNTS];
    size_t horizon_counts_count;
    int visible_faces_indices[MAX_EPA_VISIBLE_FACES_INDICES];
    size_t visible_faces_count;
    
} EPAWorkspace;

typedef struct OpenNode {

    int i;
    float g, f;
    int parent;

} OpenNode;


typedef struct ClosedNodeEntry{
    int key;    
    UT_hash_handle hh; 
} ClosedNodeEntry;


typedef struct CameFromEntry{

    int key;   
    int from;  
    UT_hash_handle hh;

} CameFromEntry;

typedef struct GScoreEntry {
    int key;
    float g;
    UT_hash_handle hh;
} GScoreEntry;

#define MAX_PORTALS 256
typedef struct AStarStateStack
{

    float g;
    float f;
    int came_from;
    int run_id;

} AStarStateStack;

typedef struct AStarWorkspace
{

    AStarStateStack * stack;
    int * path;
    int path_count;

    Vector min_heap; // open node
    ClosedNodeEntry * closed_set;
    CameFromEntry * came_from_table;
    GScoreEntry * g_score_table;

    int current_run;

    
} AStarWorkspace;

typedef struct Player
{

    float radius;
    float height;
    bool sprinting;
    bool noclip;
    Camera * cam;
    PhysicsComponent * pc;
    vec3 pos;
    Skeleton skeleton;
    Mesh mesh;
    
} Player;


static const ivec3 DEFAULT_APV_RES = {6, 6, 6};
static const ivec3 DEFAULT_LPV_RES = {PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH};

typedef struct ProbeVolume
{

    size_t byte_offset;
    size_t size;
    OBB obb;
    ivec3 resolution;
    char path[256];
    
} ProbeVolume;

typedef struct APV
{
    
    int pv_id;
    AudioProbe * audio_probes;

} APV;

typedef struct LProbe
{

    float val[SH_MAX];
    
} LProbe;


typedef struct LPV
{
    
    int pv_id;
    float * lprobes;

} LPV;

typedef struct LightBVH
{

    Vector nodes;
    size_t * indices;
    size_t indices_count;
    Vector lights;
    
} LightBVH;

typedef struct SceneData
{

    char name[256];

    AABB aabb;

    Vector bvh_nodes;

    Vector leaf_indices;

    Vector reflection_probes;

    Vector audio_probe_volumes;
    Vector lighting_probe_volumes;

    LightBVH light_bvh;

    ProbeVolume LOADED_PV[MAX_PROBE_VOLUMES];
    APV LOADED_AUDIO_PV[MAX_AUDIO_PROBE_VOLUMES];
    bool LOADED_AUDIO_PV_FLAG[MAX_AUDIO_PROBE_VOLUMES];
    LPV LOADED_LIGHTING_PV[LPV_SLOTS];
    bool LOADED_LIGHTING_PV_FLAG[LPV_SLOTS];

    char LPV_PATH[256];
    FILE * LPV_FILE;

    char APV_PATH[256];
    FILE * APV_FILE;

    int bvh_indices_count;
    size_t * bvh_indices;
    

    LightmapData lightmap_data;

    
    
} SceneData;


typedef struct FileDirectoryRegistry
{

    Vector names;
    char suffix[256];
    bool search_suffix;
    char directory[256];
    int current_selection;
    nk_bool open;
    
} FileDirectoryRegistry;

typedef enum MSAASampleOptions
{

    MSAA_2, MSAA_4, MSAA_8, MSAA_16, MSAA_SAMPLE_OPTIONS_MAX

} MSAASampleOptions;


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



typedef struct PostprocessVersion
{

    bool m_blur;
    bool dof;
    bool bloom;
    
} PostprocessVersion;


typedef struct Globals
{
    
    Vector meshes;
    Vector mesh_nodes;
    Vector skins;
    Vector assets;
    Vector materials;
    Vector images;
    Vector lights;
    Vector audio_portals;
    uint32_t * mesh_light_indices;
    bool lights_dirty;
    int current_directional;

    SceneData scene_data;

    Vector vertex_layouts;
    
    bool noclip;

    vec3 world_min;
    vec3 world_size;
    ivec3 probe_tex_size;
    float probe_spacing;

    AudioProbe * audio_probes;

    vec3 aprobe_start;
    
    float aprobe_spacing;
    ivec3 aprobe_size;

    int WORLD_WIDTH;
    int WORLD_HEIGHT;
    int WORLD_DEPTH;
    
    bool keyboard_locked;
    
    int msaa_samples;

    
} Globals;

typedef struct Uniform
{
    GLuint location;
    char name[256];
} Uniform;

typedef struct ComputeShaderProgram
{
    char path[256];
    GLuint program;

    Uniform uniforms[32];
    
    
} ComputeShaderProgram;

typedef struct ShaderProgram
{

    GLuint program;
    uint32_t perm;
    char name[256];
    char vert_path[256];
    char geom_path[256];
    char frag_path[256];

    Uniform uniforms[32];
    
} ShaderProgram;

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


extern Globals GLOBALS;


extern GLsync fence;

extern int VCNT;
extern int ICNT;


extern Camera cam;
extern Camera LIGHT_CAM[MAX_DIRECTIONAL_LIGHTS];
    
extern GLuint64 density_time;
extern GLuint64 mc_time;
extern GLuint d_q;
extern GLuint mc_q;
extern bool within_planet_atmosphere;
float noise3d(vec3 p);
void render_sky_cubemap();




#endif 
