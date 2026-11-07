#ifndef INIT_H
#define INIT_H
#include <SDL2/SDL.h>
#include <GL/glew.h>
// #include "types.h"
#include "core/engine_types.h"
#include <stdbool.h>


extern struct nk_context * NK_CTX;

 
extern FileDirectoryRegistry scene_registry;
extern FileDirectoryRegistry lightmap_registry;
extern FileDirectoryRegistry light_registry;
extern FileDirectoryRegistry sound_registry;
extern FileDirectoryRegistry aportal_registry;
extern FileDirectoryRegistry rprobe_registry;
extern FileDirectoryRegistry lprobe_registry;
extern FileDirectoryRegistry aprobe_registry;
extern FileDirectoryRegistry scene_apv_registry;
extern FileDirectoryRegistry ui_registry;

extern int LMB_HELD;

extern SDL_Window *window;
extern AStarWorkspace AW;

extern int DEBUG_APROBE_RAY_COUNT;

extern int DEBUG_CURRENT_APROBE;

extern float BAND_IMPULSES[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE];
extern bool NUKLEAR_HOVERED;
extern EditorSettings EDITOR_SETTINGS;
extern size_t meshes_in_frame[MAX_MESH_NODES];
extern size_t meshes_this_frame;

extern int DIRECTIONAL_LIGHT_COUNT;

extern HRTFThreadData HRTF_TD;

extern int PERFORMANCE_OVERLAY_TEXT_BOX_START;
extern int PERFORMANCE_OVERLAY_MENU;

extern SDL_AudioDeviceID SDL_AUDIO_DEVICE;
extern ma_engine AUDIO_ENGINE;
extern AudioScratchBuffers AUDIO_SCRATCH;

extern AudioEmitter * FOOTSTEP_EMITTER;

extern AudioEmitter AUDIO_EMITTERS[MAX_AUDIO_EMITTERS];

extern float * STATIC_HRTF_LEFT[SH_MAX];
extern float * STATIC_HRTF_RIGHT[SH_MAX];

extern DebugVertex * DEBUG_APROBE;
extern int VALID_APROBE_SAMPLES;

extern ConvolverNode CONVOLVER_NODE;

extern int HRTF_LENGTH;

extern int SCREEN_WIDTH;
extern int SCREEN_HEIGHT;
extern int RENDER_WIDTH;
extern int RENDER_HEIGHT;
extern int BLOOM_WIDTH;
extern int BLOOM_HEIGHT;
extern int DOF_WIDTH;
extern int DOF_HEIGHT;
extern int SSR_WIDTH;
extern int SSR_HEIGHT;
extern int SSAO_WIDTH;
extern int SSAO_HEIGHT;
extern GLuint COLOR_GRADE_LUT;

extern mat4 UNJITTERED_PROJ;
extern mat4 UNJITTERED_PROJ_INV;
extern vec2 JITTER_OFFSET;

extern uint32_t TEXT_INDEX_COUNT;

extern GLuint CAM_UBO;
extern GLuint RENDER_UBO;
extern GLuint LIGHT_CAM_UBO;
extern GLuint RPROBE_UBO;
extern GLuint PROBE_VOLUME_UBO;
extern GLuint SSAO_KERNEL_UBO;

extern GLuint BRDF_FBO;

// extern GLuint SHADOW_MAP_FBO;
extern Framebuffer SHADOW_MAP_FBO;
// extern GLuint SHADOW_MAP_TEMP;

extern Framebuffer MAIN_FBO;
extern Framebuffer MAIN_RESOLVE_FBO;

extern GLuint MAIN_VBO;
extern GLuint MAIN_EBO;
extern GLuint MAIN_VAO;
extern GLuint SHADOW_INSTANCE_VBO;
extern GLuint SHADOW_VAO;

extern Framebuffer TEXT_FBO;
extern GLuint TEXT_VAO;
extern GLuint TEXT_VBO;
extern GLuint TEXT_EBO;

extern GLuint UI_VAO;
extern GLuint UI_VBO;
extern GLuint UI_INSTANCE_VBO;

extern GLuint RPROBE_BACK_FBO;
extern GLuint RPROBE_BACK_TEXTURE;

extern Framebuffer GBUFFER_FBO;
extern Framebuffer GBUFFER_RESOLVE_FBO;
// extern GLuint GPOSITION_TEXTURE;
// extern GLuint GNORMAL_TEXTURE;
// extern GLuint GALBEDO_TEXTURE;
// extern GLuint GEXTRA_TEXTURE;
// extern GLuint GVEL_TEXTURE;
// extern GLuint GCOLORBUFFER_TEXTURE;
extern Framebuffer SSR_FBO;
// extern GLuint SSR_TEXTURE;
extern Framebuffer SSR_BLUR_FBO;
// extern GLuint SSR_BLUR_TEXTURE;
extern Framebuffer SSAO_FBO;
extern Framebuffer SSAO_BLUR_FBO;
// extern GLuint SSAO_TEXTURE;
extern GLuint SSAO_NOISE_TEXTURE;
// extern GLuint SSAO_BLUR_TEXTURE;
// extern GLuint SSAO_BLUR_FBO;
// extern GLuint COC_TEXTURE;
// extern GLuint COC_FBO;
extern Framebuffer COC_FBO;
// extern GLuint BOKEH_TEXTURE;
extern Framebuffer BOKEH_FBO;

extern GLuint FBM_TEXTURE;


// extern GLuint SSR_HISTORY_FBO[2];
// extern GLuint SSR_HISTORY_TEXTURE[2];
extern Framebuffer TAA_FBO[2];
extern GLuint SOBEL_FBO[2];
extern GLuint SOBEL_TEXTURE[2];
extern GLuint HISTORY_FBO[2];
extern GLuint HISTORY_TEXTURE[2];
// extern GLuint BLOOM_BLUR_FBO;
// extern GLuint BLOOM_BLUR_TEXTURE;
// extern Framebuffer BLOOM_FBO;
// extern GLuint BLOOM_TEXTURE;
extern Framebuffer BLOOM_PYRAMID[6];

extern GLuint LIGHTMAP_FBO;
extern GLuint LIGHTMAP_VAO;
extern GLuint LIGHTMAP_VBO;
extern GLuint LIGHTMAP_POS_TEXTURE;
extern GLuint LIGHTMAP_NORM_TEXTURE;
extern GLuint LIGHTMAP_ID_TEXTURE;
extern GLuint DIRECTIONAL_MAP_TEXTURE;
extern GLuint RPROBE_CUBE_MAP_ARRAY;
extern GLuint RPROBE_DEPTH_TEXTURE;
extern GLuint TEMP_RPROBE_CUBE_MAP;
extern GLuint RPROBE_FBO;

extern GLuint BLENDED_RPROBE_CUBE_MAP;
extern GLuint CONVOLVE_TRIANGLE_VBO;
extern GLuint CONVOLVE_TRIANGLE_VAO;

extern GLuint LIGHTMAP_PING;
extern GLuint LIGHTMAP_PONG;
extern GLuint LIGHTMAP_BOUNCE_SHADER;
extern GLuint LIGHTMAP_DENOISE_SHADER;
extern GLuint LIGHTMAP_DILATE_SHADER;
extern GLuint PROBE_BAKE_SHADER;
extern GLuint AUDIO_PROBE_BAKE_SHADER;
extern GLuint PROBE_UNPACK_SHADER;
extern GLuint DIRECTIONAL_MAP_NORMALIZE_SHADER;
extern ShaderProgram SOBEL_SHADER;
extern ShaderProgram HISTORY_SHADER;
extern ShaderProgram SSAO_SHADER;
extern ShaderProgram SSAO_BLUR_SHADER;
extern ShaderProgram SSR_SHADER;
extern ShaderProgram SSR_BLUR_SHADER;
extern ShaderProgram SSR_HISTORY_SHADER;
extern ShaderProgram POINT_LIGHT_SHADOW_SHADER;
extern ShaderProgram COC_SHADER;
extern ShaderProgram BOKEH_SHADER;
extern ShaderProgram BLOOM_SHADER;
extern ShaderProgram BLOOM_BLUR_SHADER;
extern ShaderProgram BLOOM_DOWNSCALE_SHADER;
extern ShaderProgram BLOOM_UPSCALE_SHADER;
extern ShaderProgram DEPTH_DOWNSAMPLE_SHADER;
extern Framebuffer DEPTH_DOWNSAMPLE_FBO;
extern vec4 SSAO_KERNEL[16];

extern GLsync RENDER_FENCE[BUFFER_FRAMES];

// extern GLuint POINT_LIGHT_SHADOWS_CMA;

extern Framebuffer POINT_LIGHT_SHADOWS_FBO;


extern int CURRENT_DIRECTIONAL;

extern int MOUSE_X, MOUSE_Y;

extern AnimationComponent * MOVING_LEG_ANIM;
extern const vec3 left_leg_offset;
extern const vec3 right_leg_offset;

extern bool right_leg_moving;

extern GLuint INSTANCE_VBO;
extern GLuint WIREFRAME_VBO;
extern GLuint WIREFRAME_VAO;
extern GLuint FS_VAO;

extern GLuint LIGHT_SSBO;
extern GLuint MESH_LIGHT_SSBO;
extern GLuint LIGHTMAP_SSBO;

// Vertices: vec4 (x, y, z, padding) or just float arrays. (Don't forget you also need vertex Normals and UVs so your ray hits can sample textures!).
extern GLuint VERTEX_SSBO;
// Indices (Tris): uvec3 for triangle lookups.
extern GLuint TRI_SSBO;
// Instances (Transforms): Needs the Model Matrix, Inverse Model Matrix (crucial for transforming rays into local mesh space), and the underlying Mesh ID.
extern GLuint MESH_SSBO;

extern GLuint MESH_BVH_INDICES_SSBO;
// Mesh BVH Nodes: Flattened array of nodes. Must contain AABB min/max, left child index, right child index, and triangle count/offset.
extern GLuint MESH_BVH_SSBO;

extern GLuint SCENE_BVH_INDICES_SSBO;
// Scene BVH Nodes (TLAS): Same structure as above, but pointing to instances instead of triangles.
extern GLuint SCENE_BVH_SSBO;
// Material Data: Roughness, emission multipliers, albedo colors for each instance.
extern GLuint MATERIAL_SSBO;
// Texel Data: An array mapping each lightmap pixel (that actually contains an island) to its world-space position and normal.
extern GLuint PROBE_SSBO;

extern GLuint RPROBE_SSBO;

extern GLuint JOINT_SSBO;

extern GLuint CLUSTER_AABB_SSBO;

extern GLuint GLOBAL_LIGHT_INDICES_SSBO;

extern GLuint CLUSTER_SSBO;

extern GLuint MESH_NODE_SSBO;

extern GLuint GLOBAL_INSTANCE_SSBO;

extern GLuint INDIRECT_ARRAYS_COMMAND_BUFFER;

extern GLuint INDIRECT_ARRAYS_COMMAND_BUFFER_OUT;

extern GLuint INDIRECT_ELEMENTS_COMMAND_BUFFER;

extern GLuint INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT;

extern GLuint INSTANCE_SSBO;

extern GLuint TEXT_BOX_SSBO;

extern GLuint UI_SSBO;


extern VertexData VERTEX_DATA;
extern TriData TRI_DATA;
extern MeshData MESH_DATA;
extern BVHGPU MESH_BVH_DATA;
extern BVHGPU SCENE_BVH_DATA;

extern LightmapSSBO LIGHTMAP_DATA;
extern MaterialData MATERIAL_DATA;

extern GLuint LPV_TEXTURE;
extern vec3 DEBUG_POINT;

extern DebugVertex DEBUG_FACES[1024][3];
extern int DEBUG_FACE_COUNT;
extern DebugVertex DEBUG_POINTS[1024];
extern int DEBUG_POINT_COUNT;
#define DEBUG_LINES_MAX 500000
extern DebugVertex DEBUG_LINES[DEBUG_LINES_MAX];
extern int DEBUG_LINE_COUNT;


extern Manifold MANIFOLDS[MAX_MANIFOLDS];

extern int DEFAULT_DIFFUSE;
extern int DEFAULT_NORMAL;
extern int DEFAULT_ARM;
extern int DEFAULT_EMISSIVE;
extern int DEFAULT_DISPLACEMENT;

extern int DUMMY_LIGHTMAP;
extern GLuint BRDF_LUT;

extern GLuint SKYBOX;

extern GLuint SKY_VAO;
extern GLuint SKY_VBO;
// extern ProbeData PROBE_DATA;
extern int current_frame;
extern int master_frame;

extern ShaderProgram STATIC_SHADER;
extern ShaderProgram DYNAMIC_SHADER[32];
extern ShaderProgram SHADOW_MAP_SHADER;
extern ShaderProgram DEPTH_PRE_PASS_SHADER[GBUFFER_VERSIONS];
extern ShaderProgram DEPTH_PRE_PASS_SHADER_MSAA[GBUFFER_VERSIONS];
extern ShaderProgram WIREFRAME_SHADER;
extern ShaderProgram POSTPROCESS_SHADER[128];
extern uint32_t PP_SHADER_VERSIONS;
extern ShaderProgram FULLSCREEN_SHADER;
extern ShaderProgram SKY_SHADER;
extern ShaderProgram LIGHTMAP_SHADER;
extern ShaderProgram BRDF_SHADER;
extern ShaderProgram GGX_RPROBE_SHADER;
extern ShaderProgram TEXT_SHADER;
extern ShaderProgram UI_SHADER;
extern GLuint LIGHTMAP_INIT_SHADER;

extern GLuint INIT_CLUSTER_AABBS_SHADER;
extern ShaderProgram CULL_CLUSTERS_SHADER;

extern ShaderProgram CULL_DRAW_CALLS_SHADER;
extern ShaderProgram INDIRECT_PREFIX_SHADER;
extern ShaderProgram ELEMENTS_PREFIX_SHADER;

extern ShaderProgram CULL_POINT_LIGHT_SHADOWS_SHADER;
extern ShaderProgram FOCAL_DISTANCE_SHADER;

#define QUERY_FRAMES 3
extern GLuint main_queries[QUERY_FRAMES];
extern GLuint pl_queries[QUERY_FRAMES];
extern GLuint sm_queries[QUERY_FRAMES];
extern GLuint render_queries[QUERY_FRAMES];
extern GLuint gbuffer_queries[QUERY_FRAMES];
extern GLuint dof_queries[QUERY_FRAMES];
extern GLuint ssao_queries[QUERY_FRAMES];
extern GLuint taa_queries[QUERY_FRAMES];
extern GLuint ssr_queries[QUERY_FRAMES];
extern GLuint bloom_queries[QUERY_FRAMES];
extern GLuint cull_queries[QUERY_FRAMES];
extern int current_query_frame;

extern float time;
extern vec3 CAM_POS;

extern char asset_dir[256];
extern char rprobe_dir[256];

extern Triangle * mesh_tris;

extern Player player;
extern vec3 spawn_point;

extern float dt;
extern int running;

extern GLuint LIGHTMAP_TEX;

extern EPAWorkspace * EW[8];

extern size_t cam_ubo_padded;
extern uint8_t * cam_ubo_ptr;

extern size_t light_cam_ubo_padded;
extern uint8_t * light_cam_ubo_ptr;

extern size_t rprobe_ubo_padded;
extern uint8_t * rprobe_ubo_ptr;

extern size_t probe_volume_ubo_padded;
extern uint8_t * probe_volume_ubo_ptr;

extern size_t light_ssbo_size;
extern uint8_t * light_ssbo_ptr;

extern size_t mesh_light_ssbo_size;
extern uint8_t * mesh_light_ssbo_ptr;

extern size_t joint_ssbo_size;
extern uint8_t * joint_ssbo_ptr;

extern size_t instance_vbo_size;
extern uint8_t * instance_vbo_ptr;

extern uint8_t * shadow_instance_vbo_ptr;

extern size_t mesh_node_ssbo_size;
extern uint8_t * mesh_node_ssbo_ptr;

extern uint8_t * indirect_arrays_ptr;

extern uint8_t * indirect_arrays_out_ptr;

extern uint8_t * indirect_elements_ptr;

extern uint8_t * indirect_elements_out_ptr;

extern uint8_t * visible_indices_ptr;

extern size_t instance_ssbo_size;
extern uint8_t * instance_ssbo_ptr;

extern size_t text_box_ssbo_size;
extern uint8_t * text_box_ssbo_ptr;

extern size_t ui_ssbo_size;
extern uint8_t * ui_ssbo_ptr;

extern size_t text_vbo_size;
extern uint8_t * text_vbo_ptr;

extern size_t ui_vbo_size;
extern uint8_t * ui_vbo_ptr;

extern size_t ui_instance_vbo_size;
extern uint8_t * ui_instance_vbo_ptr;

extern bool draw_dirty;
extern double time_since_last_render;

#define MAX_ENTITIES 256

extern Entity ENTITIES[MAX_ENTITIES];

#define MAX_ASSET_INSTANCES 256

extern AssetInstance ASSET_INSTANCES[MAX_ASSET_INSTANCES];

extern DrawCommand DRAW_COMMANDS[MAX_DRAW_COMMANDS];
extern int draw_command_count;

extern DrawCommand SHADOW_DRAW_COMMANDS[MAX_DRAW_COMMANDS];
extern int shadow_draw_command_count;

#define MAX_PHYSICS_COMPONENTS 256

extern PhysicsComponent PHYSICS_COMPONENTS[MAX_PHYSICS_COMPONENTS];

#define MAX_ANIMATION_COMPONENTS 256
extern AnimationComponent ANIMATION_COMPONENTS[MAX_ANIMATION_COMPONENTS];


static Light default_light =
{
    .light_type = POINT_LIGHT,
    .cone_angle = 0,
    .inner_angle = 0,
    .outer_angle = 0,
    .dir = (vec3){0,0,1},
    .entity_id = 0,
    .intensity = 1.0f,
    .penumbra = 0,
    .pos = (vec3){0,0,0},
    .rgb = (vec3){1,1,1},
    .area = (AABB){0},
};

extern float frame_time;

extern const char menu_names[][256];



void allocate_main_fbo();
void change_fbo_resolution(Framebuffer * fbo, int w, int h, bool skip_depth);
void GPU_upload_scene_data();
void GPU_init_cluster_aabbs();
AssetInstance * init_new_asset_instance(const char * asset_name, vec3 tr, vec4 rot, vec3 scale);
void destroy_asset_instance(uint32_t id);
void init_default_textures();
Material * init_new_material(const char * filename);
Mesh * retrieve_mesh_by_name(const char * asset_name, const char * mesh_name);
void init_mesh(Mesh * mesh);
void allocate_gbuffer_msaa();
void allocate_gbuffer();
bool init(SDL_Window **window, SDL_GLContext *context);

#endif
