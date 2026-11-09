/**
 * @file init.h
 * @brief Important globals for the entire engine and initialization functions that are mainly called on engine start.
 */

#ifndef INIT_H
#define INIT_H
#include <SDL2/SDL.h>
#include <GL/glew.h>
#include "core/engine_types.h"
#include <stdbool.h>



/**
 * @name Important Runtime Variables 
 * @brief Variables that control player state and delta time.
 * @{
 */
extern float dt;                      /**<The 1.0f/60 simulation timestep.*/
extern float frame_time;              /**<The actual frame time in s.*/
extern float time;                    /**<The frame time but wrapping at 1.0.*/
extern int running;                   /**<The variable that tracks if we need to exit main().*/
extern int LMB_HELD;                  /**<If the left mouse button is held.*/
extern int MOUSE_X, MOUSE_Y;          /**<Mouse position.*/
extern vec3 CAM_POS;                  /**<Shorthand variable for the camera position.*/
extern bool draw_dirty;               /**<If we need to reupload and resort transforms in create_and_sort_draw_commands() and upload_instance_data().*/
extern double time_since_last_render; /**<The time since the last render (while running physics updates).*/
extern Player player;                 /**<The Player data.*/
extern vec3 spawn_point;              /**<Spawn point @todo encapsulate.*/
extern int current_frame;
extern int master_frame;
extern GLsync RENDER_FENCE[BUFFER_FRAMES];
extern const vec3 left_leg_offset;
extern const vec3 right_leg_offset;
extern bool right_leg_moving;
extern bool NUKLEAR_HOVERED;
extern EditorSettings EDITOR_SETTINGS;
extern size_t meshes_in_frame[MAX_MESH_NODES];
extern size_t meshes_this_frame;
extern int DIRECTIONAL_LIGHT_COUNT;
extern int PERFORMANCE_OVERLAY_TEXT_BOX_START;
extern int PERFORMANCE_OVERLAY_MENU;
extern struct nk_context * NK_CTX; /**<Nuklear context handle used for Editor UI*/ 
extern SDL_Window *window;
/** @} */

extern const char menu_names[][256]; /**<The array for all editor menu names (to avoid Nuklear hashing problems)*/
 
/**
 * @name FileDirectoryRegistries 
 * @brief All of the registries used for parsing working directories.
 * @{
 */
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
/** @} */


/**
 * @name Workspaces 
 * @brief Stack workspaces currently for EPA and ASTAR.
 * @{
 */
extern AStarWorkspace AW;
extern EPAWorkspace * EW[8];
/** @} */


/**
 * @name AudioProbe Related Variables 
 * @brief Parts of the path traced AudioProbe pipeline.
 * @{
 */
extern SDL_AudioDeviceID SDL_AUDIO_DEVICE;
extern ma_engine AUDIO_ENGINE;
extern AudioScratchBuffers AUDIO_SCRATCH;
extern float * STATIC_HRTF_LEFT[SH_MAX];
extern float * STATIC_HRTF_RIGHT[SH_MAX];
extern int HRTF_LENGTH;
extern ConvolverNode CONVOLVER_NODE;
extern int DEBUG_APROBE_RAY_COUNT;
extern int DEBUG_CURRENT_APROBE;
extern HRTFThreadData HRTF_TD;
extern float BAND_IMPULSES[AUDIO_NUM_BANDS][AUDIO_KERNEL_SIZE];
/** @} */

extern AudioEmitter * FOOTSTEP_EMITTER;
extern AudioEmitter AUDIO_EMITTERS[MAX_AUDIO_EMITTERS];

extern vec4 SSAO_KERNEL[16];

/**
 * @name Resolutions 
 * @brief Resolutions for Framebuffer%s.
 * @{
 */
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
/** @} */

extern mat4 UNJITTERED_PROJ;
extern mat4 UNJITTERED_PROJ_INV;
extern vec2 JITTER_OFFSET;

extern uint32_t TEXT_INDEX_COUNT;

/**
 * @name UBOs 
 * @see init().
 * @{
 */
extern GLuint CAM_UBO;
extern GLuint RENDER_UBO;
extern GLuint LIGHT_CAM_UBO;
extern GLuint RPROBE_UBO;
extern GLuint PROBE_VOLUME_UBO;
extern GLuint SSAO_KERNEL_UBO;
/** @} */

/**
 * @name VBOs, VAOs, and EBOs 
 * @see init().
 * @{
 */
extern GLuint CONVOLVE_TRIANGLE_VBO;
extern GLuint CONVOLVE_TRIANGLE_VAO;
extern GLuint SKY_VAO;
extern GLuint SKY_VBO;
extern GLuint INSTANCE_VBO;
extern GLuint WIREFRAME_VBO;
extern GLuint WIREFRAME_VAO;
extern GLuint FS_VAO;
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
extern GLuint LIGHTMAP_VAO;
extern GLuint LIGHTMAP_VBO;
/** @} */

/**
 * @name Global Textures 
 * @see init().
 * @{
 */
extern GLuint RPROBE_BACK_TEXTURE;
extern GLuint SSAO_NOISE_TEXTURE;
extern GLuint FBM_TEXTURE;
extern GLuint SOBEL_TEXTURE[2];
extern GLuint HISTORY_TEXTURE[2];
extern GLuint LIGHTMAP_POS_TEXTURE;
extern GLuint LIGHTMAP_NORM_TEXTURE;
extern GLuint LIGHTMAP_ID_TEXTURE;
extern GLuint DIRECTIONAL_MAP_TEXTURE;
extern GLuint RPROBE_CUBE_MAP_ARRAY;
extern GLuint RPROBE_DEPTH_TEXTURE;
extern GLuint TEMP_RPROBE_CUBE_MAP;
extern GLuint LPV_TEXTURE;
extern GLuint LIGHTMAP_PING;
extern GLuint LIGHTMAP_PONG;
extern GLuint BLENDED_RPROBE_CUBE_MAP;
extern GLuint COLOR_GRADE_LUT;
extern GLuint BRDF_LUT;
extern GLuint SKYBOX;
/** @} */

/**
 * @name Dummy Textures 
 * @see init().
 * @{
 */
extern int DEFAULT_DIFFUSE;
extern int DEFAULT_NORMAL;
extern int DEFAULT_ARM;
extern int DEFAULT_EMISSIVE;
extern int DEFAULT_DISPLACEMENT;
extern int DUMMY_LIGHTMAP;
/** @} */

/**
 * @name Global Shaders 
 * @see SHADERS_INIT().
 * @todo remove SSR
 * @{
 */
extern ShaderProgram LIGHTMAP_SHADER;                              /**<light/lightmap.vert light/lightmap.frag*/
extern GLuint        LIGHTMAP_BOUNCE_SHADER;                       /**<light/lightmap_bounce.comp*/
extern GLuint        LIGHTMAP_DENOISE_SHADER;                      /**<light/lightmap_denoise.comp*/
extern GLuint        LIGHTMAP_DILATE_SHADER;                       /**<light/lightmap_dilate.comp*/
extern GLuint        DIRECTIONAL_MAP_NORMALIZE_SHADER;             /**<light/directional_map_normalize.comp*/
extern GLuint        PROBE_BAKE_SHADER;                            /**<light/probe_bake.comp*/
extern GLuint        PROBE_UNPACK_SHADER;                          /**<light/probe_unpack.comp*/
extern GLuint        AUDIO_PROBE_BAKE_SHADER;                      /**<audio/audio_probe_bake.comp*/
extern GLuint        INIT_CLUSTER_AABBS_SHADER;                    /**<render/init_cluster_aabbs.comp*/
extern ShaderProgram CULL_CLUSTERS_SHADER;                         /**<render/cull_clusters.comp*/
extern ShaderProgram CULL_DRAW_CALLS_SHADER;                       /**<render/cull_draw_calls.comp*/
extern ShaderProgram INDIRECT_PREFIX_SHADER;                       /**<render/indirect_prefix.comp*/
extern ShaderProgram ELEMENTS_PREFIX_SHADER;                       /**<render/elements_prefix.comp*/
extern ShaderProgram CULL_POINT_LIGHT_SHADOWS_SHADER;              /**<render/cull_point_light_shadows.comp*/
extern ShaderProgram SKY_SHADER;                                   /**<render/sky.vert render/sky.frag*/
extern ShaderProgram SHADOW_MAP_SHADER;                            /**<render/shadow.vert render/shadow.frag*/
extern ShaderProgram POINT_LIGHT_SHADOW_SHADER;                    /**<render/point_light_shadows.vert render/point_light_shadows.geom render/point_light_shadows.frag*/
extern ShaderProgram DEPTH_PRE_PASS_SHADER[GBUFFER_VERSIONS];      /**<render/depth_pre_pass.vert render/depth_pre_pass.frag*/
extern ShaderProgram DEPTH_PRE_PASS_SHADER_MSAA[GBUFFER_VERSIONS]; /**<render/depth_pre_pass_no_gbuffer.vert render/depth_pre_pass_no_gbuffer.frag*/
extern ShaderProgram DEPTH_DOWNSAMPLE_SHADER;                      /**<render/depth_downsample.frag*/
extern ShaderProgram SSAO_SHADER;                                  /**<render/ssao.frag*/
extern ShaderProgram SSAO_BLUR_SHADER;                             /**<render/ssao_blur.frag*/
extern ShaderProgram DYNAMIC_SHADER[32];                           /**<render/dynamic.vert render/dynamic.frag*/
extern ShaderProgram SSR_SHADER;                                   
extern ShaderProgram SSR_BLUR_SHADER;                              
extern ShaderProgram SSR_HISTORY_SHADER;                           
extern ShaderProgram COC_SHADER;                                   /**<render/coc.frag*/
extern ShaderProgram BOKEH_SHADER;                                 /**<render/bokeh.frag*/
extern ShaderProgram BLOOM_SHADER;                                 /**<render/bloom.frag*/
extern ShaderProgram BLOOM_BLUR_SHADER;                            /**<render/bloom_blur.frag*/
extern ShaderProgram FOCAL_DISTANCE_SHADER;                        /**<render/focal_distance.comp*/
extern ShaderProgram BLOOM_DOWNSCALE_SHADER;                       /**<render/bloom_downscale.frag*/
extern ShaderProgram BLOOM_UPSCALE_SHADER;                         /**<render/bloom_upscale.frag*/
extern ShaderProgram WIREFRAME_SHADER;                             /**<render/wireframe.vert render/wireframe.frag*/
extern ShaderProgram BRDF_SHADER;                                  /**<utilities/brdf.frag*/
extern ShaderProgram TEXT_SHADER;                                  /**<ui/text.vert ui/text.frag*/
extern ShaderProgram UI_SHADER;                                    /**<ui/ui.vert ui/ui.frag*/
extern ShaderProgram POSTPROCESS_SHADER[128];                      /**<render/postprocess.vert render/postprocess.frag*/
/** @} */

extern uint32_t PP_SHADER_VERSIONS;

/**
 * @name FBOs 
 * @see init().
 * @{
 */
extern GLuint BRDF_FBO;
extern GLuint RPROBE_BACK_FBO;
extern GLuint SOBEL_FBO[2];
extern GLuint HISTORY_FBO[2];
extern GLuint LIGHTMAP_FBO;
extern GLuint RPROBE_FBO;
extern Framebuffer TAA_FBO[2];
extern Framebuffer POINT_LIGHT_SHADOWS_FBO;
extern Framebuffer GBUFFER_FBO;
extern Framebuffer GBUFFER_RESOLVE_FBO;
extern Framebuffer SSR_FBO;
extern Framebuffer SSR_BLUR_FBO;
extern Framebuffer SSAO_FBO;
extern Framebuffer SSAO_BLUR_FBO;
extern Framebuffer COC_FBO;
extern Framebuffer BOKEH_FBO;
extern Framebuffer SHADOW_MAP_FBO;
extern Framebuffer MAIN_FBO;
extern Framebuffer MAIN_RESOLVE_FBO;
extern Framebuffer BLOOM_PYRAMID[6];
extern Framebuffer DEPTH_DOWNSAMPLE_FBO;
/** @} */




/**
 * @name Baking SSBOs 
 * @see init().
 * @{
 */
extern GLuint LIGHT_SSBO;
extern GLuint MESH_LIGHT_SSBO;
extern GLuint LIGHTMAP_SSBO;
extern GLuint VERTEX_SSBO; /**<Vertices: vec4 (x, y, z, padding) or just float arrays. (Don't forget you also need vertex Normals and UVs so your ray hits can sample textures!).*/
extern GLuint TRI_SSBO; /**<Indices (Tris): uvec3 for triangle lookups.*/
extern GLuint MESH_SSBO; /**<Instances (Transforms): Needs the Model Matrix, Inverse Model Matrix (crucial for transforming rays into local mesh space), and the underlying Mesh ID. */
extern GLuint MESH_BVH_SSBO; /**<Mesh BVH Nodes: Flattened array of nodes. Must contain AABB min/max, left child index, right child index, and triangle count/offset.*/
extern GLuint SCENE_BVH_SSBO; /**<Scene BVH Nodes (TLAS): Same structure as above, but pointing to instances instead of triangles.*/
extern GLuint MATERIAL_SSBO; /**<Material Data: Roughness, emission multipliers, albedo colors for each instance.*/
extern GLuint PROBE_SSBO;
/** @} */

/**
 * @name Rendering SSBOs 
 * @see init().
 * @{
 */
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
/** @} */


/**
 * @name GPU Scratch Buffers 
 * @brief The temp GPU data for lightmap baking.
 * @see init().
 * @{
 */
extern VertexData VERTEX_DATA;
extern TriData TRI_DATA;
extern MeshData MESH_DATA;
extern BVHGPU MESH_BVH_DATA;
extern BVHGPU SCENE_BVH_DATA;
extern LightmapSSBO LIGHTMAP_DATA;
extern MaterialData MATERIAL_DATA;
/** @} */


/**
 * @name Debug Draw Buffers 
 * @brief Pushed to each frame for wireframes and debug information.
 * @see render().
 * @{
 */
#define DEBUG_LINES_MAX 500000
extern DebugVertex DEBUG_FACES[1024][3];
extern int DEBUG_FACE_COUNT;
extern DebugVertex DEBUG_POINTS[1024];
extern int DEBUG_POINT_COUNT;
extern DebugVertex DEBUG_LINES[DEBUG_LINES_MAX];
extern int DEBUG_LINE_COUNT;
/** @} */

/**
 * @name Performance Queries 
 * @brief GLQuery results that are pushed to performance overlay.
 * @see update_performance_overlay().
 * @{
 */
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
/** @} */

extern char asset_dir[256];
extern char rprobe_dir[256];

extern Triangle * mesh_tris;


/**
 * @name Persistent Buffer Pointers 
 * @brief Pointers to persistent OpenGL GPU-side buffers, some of which are triple buffered and require pointer arithmetic.
 * @see init().
 * @{
 */
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
/** @} */


extern DrawCommand DRAW_COMMANDS[MAX_DRAW_COMMANDS];
extern int draw_command_count;

extern DrawCommand SHADOW_DRAW_COMMANDS[MAX_DRAW_COMMANDS];
extern int shadow_draw_command_count;

/**
 * @name Stack Arrays 
 * @brief Stack arrays for components and engine structures that use a free list to manage destruction and lifetime.
 * @{
 */
#define MAX_ENTITIES 256
extern Entity ENTITIES[MAX_ENTITIES];

#define MAX_ASSET_INSTANCES 256
extern AssetInstance ASSET_INSTANCES[MAX_ASSET_INSTANCES];

#define MAX_PHYSICS_COMPONENTS 256
extern PhysicsComponent PHYSICS_COMPONENTS[MAX_PHYSICS_COMPONENTS];

#define MAX_MANIFOLDS 64 /**<The maximum amount of Manifolds in ::MANIFOLDS.*/
extern Manifold MANIFOLDS[MAX_MANIFOLDS];

#define MAX_ANIMATION_COMPONENTS 256
extern AnimationComponent ANIMATION_COMPONENTS[MAX_ANIMATION_COMPONENTS];
/** @} */


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





/**
 * @brief Allocates the main FBO, changes if MSAA is enabled.
 */
void allocate_main_fbo();

/**
 * @brief Changes Framebuffer%'s resolution, and deletes the old textures.
 * @param fbo The Framebuffer that changes.
 * @param w The width.
 * @param h The height.
 * @param skip_depth If depth texture needs to also change.
 */
void change_fbo_resolution(Framebuffer * fbo, int w, int h, bool skip_depth);

/**
 * @brief Uploads current Globals and SceneData structures to the GPU for baking SSBOs.
 */
void GPU_upload_scene_data();

/**
 * @brief Initializes AABB%s on the GPU for clustered forward shading.
 */
void GPU_init_cluster_aabbs();

/**
 * @brief Initializes and spawns a new AssetInstance based on an Asset name, and places the AssetInstance.model at a certain transformation.
 * @param asset_name The name of the Asset.
 * @param tr The translation.
 * @param rot The rotation.
 * @param scale The scale.
 */
AssetInstance * init_new_asset_instance(const char * asset_name, vec3 tr, vec4 rot, vec3 scale);

/**
 * @brief Destroys and frees an AssetInstance.
 * @param id The ID of the AssetInstance into ::ASSET_INSTANCES to destroy.
 */
void destroy_asset_instance(uint32_t id);

/**
 * @brief Initializes dummy default textures inside of Globals.images that are used as fallback for empty Material%s.
 */
void init_default_textures();


/**
 * @brief Expands Globals.materials and initializes a material to default.
 * @param filename The path and name of the file.
 * @returns Pointer to the Material.
 * @note Only used for default material at index 0.
 */
Material * init_new_material(const char * filename);

/**
 * @brief Searches Globals.assets for a certain Mesh.
 * @param asset_name The Asset name.
 * @param mesh_name The Mesh name.
 * @returns Pointer to the Mesh.
 */
Mesh * retrieve_mesh_by_name(const char * asset_name, const char * mesh_name);

/**
 * @brief Initializes a static or dynamic Mesh.
 * @param mesh Pointer to Mesh.
 * @see init_mesh_tri_bvh()
 * @see incremental_hull()
 * @see build_adjacency_map()
 */
void init_mesh(Mesh * mesh);

/**
 * @brief Allocated a gbuffer with multisampled textures.
 */
void allocate_gbuffer_msaa();

/**
 * @brief Allocates a gbuffer without multisampled textures.
 */
void allocate_gbuffer();

/**
 * @brief Initializes the engine, including SDL, OpenGL, GLEW, Nuklear, and engine-specific systems.
 * @param window SDL Window.
 * @param context SDL Context.
 */
bool init(SDL_Window **window, SDL_GLContext *context);

#endif
