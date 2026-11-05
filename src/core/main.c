#include <GL/glew.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include "SDL2/SDL_audio.h"
#include "SDL2/SDL_opengl.h"
#include "fftw3.h"
#include "load/init.h"
#include "core/input.h"
#include "render/render.h"
#include "core/camera.h"
#include "core/mesh.h"
#include "utilities/types.h"
#include "utilities/mathf.h"
#include "light/lightmap.h"
#include "load/load.h"
#include "utilities/utilities.h"
#include "core/acceleration.h"
#include "ECS/entity.h"
#include "editor/editor.h"
#include "physics/physics.h"
#include "core/animation.h"
#include "load/gltf.h"
#include "ui/fonts.h"
#include "audio/audio.h"
#include "ui/ui.h"
#include <stdalign.h>
#include "audio/audio_ir.h"
#include "ui/ui_callbacks.h"

#define DEFAULT_WIDTH 1280
#define DEFAULT_HEIGHT 960

#include "stb_image.h"
#include "stb_image_write.h"

// #define MYSOFA_STATIC_DEFINE
// // #include "mysod


int LMB_HELD = 0;
int DEBUG_APROBE_RAY_COUNT = 0;

DebugVertex * DEBUG_APROBE = NULL;
int VALID_APROBE_SAMPLES = 0;
int DEBUG_CURRENT_APROBE = 0;



#include "nk_config.h"
#include "nuklear.h"
#include "nuklear_sdl_gl3.h"
#define MAX_VERTEX_MEMORY 1024 * 1024
#define MAX_ELEMENT_MEMORY 512 * 1024



alignas(32) ma_engine AUDIO_ENGINE;
AudioEmitter AUDIO_EMITTERS[MAX_AUDIO_EMITTERS];

float * STATIC_HRTF_LEFT[SH_MAX] = {0};
float * STATIC_HRTF_RIGHT[SH_MAX] = {0};
StereoAudio STEREO_AUDIO = {0};
int HRTF_LENGTH = 0;
alignas(32) ConvolverNode CONVOLVER_NODE = {0};
AudioScratchBuffers AUDIO_SCRATCH = {0};
HRTFThreadData HRTF_TD = {0};
AStarWorkspace AW = {0};

int SCREEN_WIDTH = 1280;
int SCREEN_HEIGHT = 720;
int RENDER_WIDTH = 1280;
int RENDER_HEIGHT = 720;
int BLOOM_WIDTH = 640;
int BLOOM_HEIGHT = 480;
int DOF_WIDTH = 640;
int DOF_HEIGHT = 480;
int SSR_WIDTH = 640;
int SSR_HEIGHT = 480;
int SSAO_WIDTH = 640;
int SSAO_HEIGHT = 480;

Framebuffer MAIN_FBO = {0};
Framebuffer MAIN_RESOLVE_FBO = {0};
Framebuffer SHADOW_MAP_FBO = {0};
Framebuffer GBUFFER_FBO = {0};
Framebuffer GBUFFER_RESOLVE_FBO = {0};
Framebuffer SSR_FBO = {0};
Framebuffer SSR_BLUR_FBO = {0};
Framebuffer SSAO_FBO = {0};
Framebuffer SSAO_BLUR_FBO = {0};
Framebuffer TAA_FBO[2] = {0};
Framebuffer COC_FBO = {0};
Framebuffer BOKEH_FBO = {0};
Framebuffer BLOOM_PYRAMID[6] = {0};
Framebuffer TEXT_FBO = {0};

ShaderProgram SOBEL_SHADER = {0};
ShaderProgram HISTORY_SHADER = {0};
ShaderProgram SSAO_SHADER = {0};
ShaderProgram SSAO_BLUR_SHADER = {0};
ShaderProgram SSR_SHADER = {0};
ShaderProgram SSR_BLUR_SHADER = {0};
ShaderProgram SSR_HISTORY_SHADER = {0};
ShaderProgram COC_SHADER = {0};
ShaderProgram BOKEH_SHADER = {0};
ShaderProgram BLOOM_SHADER = {0};
ShaderProgram BLOOM_BLUR_SHADER = {0};
ShaderProgram STATIC_SHADER = {0};
ShaderProgram DYNAMIC_SHADER[32] = {0};
ShaderProgram SHADOW_MAP_SHADER = {0};
ShaderProgram DEPTH_PRE_PASS_SHADER[GBUFFER_VERSIONS] = {0};
ShaderProgram DEPTH_PRE_PASS_SHADER_MSAA[GBUFFER_VERSIONS] = {0};
ShaderProgram WIREFRAME_SHADER = {0};
ShaderProgram POSTPROCESS_SHADER[128] = {0};
ShaderProgram CULL_CLUSTERS_SHADER = {0};
ShaderProgram CULL_DRAW_CALLS_SHADER = {0};
ShaderProgram INDIRECT_PREFIX_SHADER = {0};
ShaderProgram ELEMENTS_PREFIX_SHADER = {0};
ShaderProgram CULL_POINT_LIGHT_SHADOWS_SHADER = {0};
ShaderProgram BLOOM_DOWNSCALE_SHADER = {0};
ShaderProgram BLOOM_UPSCALE_SHADER = {0};
ShaderProgram SKY_SHADER = {0};
ShaderProgram LIGHTMAP_SHADER = {0};
ShaderProgram BRDF_SHADER = {0};
ShaderProgram GGX_RPROBE_SHADER = {0};
ShaderProgram POINT_LIGHT_SHADOW_SHADER = {0};
ShaderProgram TEXT_SHADER = {0};
ShaderProgram UI_SHADER = {0};
ShaderProgram DEPTH_DOWNSAMPLE_SHADER = {0};

GLuint CAM_UBO = 0;
GLuint RENDER_UBO = 0;
GLuint LIGHT_CAM_UBO = 0;
GLuint RPROBE_UBO = 0;
GLuint SSAO_KERNEL_UBO = 0;
GLuint PROBE_VOLUME_UBO = 0;

GLuint RPROBE_BACK_FBO = 0;
GLuint RPROBE_DEPTH_TEXTURE = 0;
GLuint RPROBE_BACK_TEXTURE = 0;
GLuint BLENDED_RPROBE_CUBE_MAP = 0;
GLuint COLOR_GRADE_LUT = 0;


AudioEmitter * FOOTSTEP_EMITTER = NULL;
GLuint LIGHT_SSBO = 0;
GLuint MESH_LIGHT_SSBO = 0;
GLuint MATERIAL_SSBO = 0;
GLuint LIGHTMAP_SSBO = 0;
GLuint VERTEX_SSBO = 0;
GLuint TRI_SSBO = 0;
GLuint MESH_SSBO = 0;
GLuint MESH_BVH_SSBO = 0;
GLuint SCENE_BVH_SSBO = 0;
GLuint PROBE_SSBO = 0;
GLuint RPROBE_SSBO = 0;
GLuint JOINT_SSBO = 0;
GLuint CLUSTER_AABB_SSBO = 0;
GLuint GLOBAL_LIGHT_INDICES_SSBO = 0;
GLuint CLUSTER_SSBO = 0;
GLuint MESH_NODE_SSBO = 0;
GLuint GLOBAL_INSTANCE_SSBO = 0;
GLuint INDIRECT_ARRAYS_COMMAND_BUFFER = 0;
GLuint INDIRECT_ARRAYS_COMMAND_BUFFER_OUT = 0;
GLuint INDIRECT_ELEMENTS_COMMAND_BUFFER = 0;
GLuint INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT = 0;
GLuint INSTANCE_SSBO = 0;
GLuint TEXT_BOX_SSBO = 0;
GLuint UI_SSBO = 0;

GLuint SSAO_NOISE_FBO = 0;
GLuint SSAO_NOISE_TEXTURE = 0;
GLuint FBM_TEXTURE = 0;

mat4 UNJITTERED_PROJ;
mat4 UNJITTERED_PROJ_INV;
vec2 JITTER_OFFSET;

int MOUSE_X = 0, MOUSE_Y = 0;

GLuint BRDF_FBO = 0; 

VertexData VERTEX_DATA;
TriData TRI_DATA;
MeshData MESH_DATA;
BVHGPU MESH_BVH_DATA;
BVHGPU SCENE_BVH_DATA;

LightmapSSBO LIGHTMAP_DATA;
MaterialData MATERIAL_DATA;

int DEFAULT_DIFFUSE = 0;
int DEFAULT_NORMAL = 0;
int DEFAULT_ARM = 0;
int DEFAULT_EMISSIVE = 0;
int DEFAULT_DISPLACEMENT = 0;
int DUMMY_LIGHTMAP = 0;

GLuint BRDF_LUT = 0;

vec3 CAM_POS;

GLuint SHADOW_MAP_TEMP = 0;
GLuint HISTORY_FBO[2] = {0};
GLuint HISTORY_TEXTURE[2] = {0};
GLuint SOBEL_FBO[2] = {0};
GLuint SOBEL_TEXTURE[2] = {0};

vec4 SSAO_KERNEL[16];

GLuint LIGHTMAP_TEX;

GLuint LIGHTMAP_FBO = 0;
GLuint LIGHTMAP_VAO = 0;
GLuint LIGHTMAP_VBO = 0;
GLuint LIGHTMAP_POS_TEXTURE = 0;
GLuint LIGHTMAP_NORM_TEXTURE = 0;
GLuint LIGHTMAP_ID_TEXTURE = 0;
GLuint DIRECTIONAL_MAP_TEXTURE = 0;
GLuint LIGHTMAP_PING = 0;
GLuint LIGHTMAP_PONG = 0;
GLuint LIGHTMAP_INIT_SHADER = 0;
GLuint LIGHTMAP_BOUNCE_SHADER = 0;
GLuint LIGHTMAP_DENOISE_SHADER = 0;
GLuint LIGHTMAP_DILATE_SHADER = 0;
GLuint PROBE_BAKE_SHADER = 0;
GLuint AUDIO_PROBE_BAKE_SHADER = 0;
GLuint PROBE_UNPACK_SHADER = 0;
GLuint DIRECTIONAL_MAP_NORMALIZE_SHADER = 0;
GLuint TEMP_RPROBE_CUBE_MAP = 0;
GLuint CONVOLVE_TRIANGLE_VBO = 0;
GLuint CONVOLVE_TRIANGLE_VAO = 0;

GLuint MAIN_VBO = 0;
GLuint MAIN_EBO = 0;
GLuint MAIN_VAO = 0;
GLuint INSTANCE_VBO = 0;
GLuint SHADOW_INSTANCE_VBO = 0;
GLuint SHADOW_VAO = 0;
GLuint WIREFRAME_VBO = 0;
GLuint WIREFRAME_VAO = 0;
GLuint FS_VAO = 0;

int PERFORMANCE_OVERLAY_TEXT_BOX_START = 0;
int PERFORMANCE_OVERLAY_MENU = 0;


GLuint TEXT_VAO = 0;
GLuint TEXT_VBO = 0;
GLuint TEXT_EBO = 0;

GLuint UI_VAO = 0;
GLuint UI_VBO = 0;
GLuint UI_INSTANCE_VBO = 0;

int CURRENT_DIRECTIONAL = 0;

DrawCommand DRAW_COMMANDS[MAX_DRAW_COMMANDS];
int draw_command_count;


SDL_AudioDeviceID SDL_AUDIO_DEVICE;

DrawCommand SHADOW_DRAW_COMMANDS[MAX_DRAW_COMMANDS];
int shadow_draw_command_count;

GLuint LPV_TEXTURE;

Manifold MANIFOLDS[MAX_MANIFOLDS];

uint32_t PP_SHADER_VERSIONS = 0; 

GLuint INIT_CLUSTER_AABBS_SHADER = 0;
Framebuffer DEPTH_DOWNSAMPLE_FBO = {0};

DebugVertex DEBUG_FACES[1024][3];
int DEBUG_FACE_COUNT;
DebugVertex DEBUG_POINTS[1024];
int DEBUG_POINT_COUNT;
DebugVertex DEBUG_LINES[DEBUG_LINES_MAX];
int DEBUG_LINE_COUNT;

char asset_dir[256];
char rprobe_dir[256];

Player player = {0};

vec3 spawn_point = {0, 0, 0};

EPAWorkspace * EW[8];

GLsync RENDER_FENCE[BUFFER_FRAMES];

AnimationComponent * MOVING_LEG_ANIM = NULL;
vec3 left_hip_pos;
vec3 right_hip_pos;

bool right_leg_moving = false;

float frame_time = 0;

GLuint SKYBOX = 0;
GLuint SKY_VAO = 0;
GLuint SKY_VBO = 0;
GLuint RPROBE_CUBE_MAP_ARRAY = 0;
GLuint RPROBE_FBO = 0;
Framebuffer POINT_LIGHT_SHADOWS_FBO = {0};
ShaderProgram FOCAL_DISTANCE_SHADER = {0};

Globals GLOBALS = {0};

Entity ENTITIES[MAX_ENTITIES];
AssetInstance ASSET_INSTANCES[MAX_ASSET_INSTANCES];
PhysicsComponent PHYSICS_COMPONENTS[MAX_PHYSICS_COMPONENTS];
AnimationComponent ANIMATION_COMPONENTS[MAX_ANIMATION_COMPONENTS];

int running = 0;

float dt = 0.0f;
float time = 0;

Camera cam = {0};
Camera LIGHT_CAM[MAX_DIRECTIONAL_LIGHTS] = {0};
int DIRECTIONAL_LIGHT_COUNT = 0;


struct nk_context * NK_CTX = NULL;
bool NUKLEAR_HOVERED = false;

EditorSettings EDITOR_SETTINGS = {0}; 

size_t cam_ubo_padded;
uint8_t * cam_ubo_ptr;

size_t light_cam_ubo_padded;
uint8_t * light_cam_ubo_ptr;

size_t rprobe_ubo_padded;
uint8_t * rprobe_ubo_ptr;

size_t probe_volume_ubo_padded;
uint8_t * probe_volume_ubo_ptr;

size_t light_ssbo_size;
uint8_t * light_ssbo_ptr;

size_t mesh_light_ssbo_size;
uint8_t * mesh_light_ssbo_ptr;

size_t joint_ssbo_size;
uint8_t * joint_ssbo_ptr;

size_t instance_vbo_size;
uint8_t * instance_vbo_ptr;

uint8_t * shadow_instance_vbo_ptr;

size_t mesh_node_ssbo_size;
uint8_t * mesh_node_ssbo_ptr;

uint8_t * indirect_arrays_ptr;

uint8_t * visible_indices_ptr;

uint8_t * indirect_arrays_out_ptr;

uint8_t * indirect_elements_ptr;

uint8_t * indirect_elements_out_ptr;

size_t text_box_ssbo_size;
uint8_t * text_box_ssbo_ptr;

size_t instance_ssbo_size;
uint8_t * instance_ssbo_ptr;

size_t text_vbo_size;

size_t ui_ssbo_size;
uint8_t * ui_ssbo_ptr;

size_t ui_vbo_size;
uint8_t * ui_vbo_ptr;

size_t ui_instance_vbo_size;
uint8_t * ui_instance_vbo_ptr;

bool draw_dirty;


const vec3 left_leg_offset = {0, -0.1f, 0};
const vec3 right_leg_offset = {0, 0.1f, 0};

Cluster CLUSTER_GRID[TOTAL_CLUSTERS];

uint32_t GLOBAL_INDEX_LIST[TOTAL_CLUSTERS * MAX_LIGHTS_PER_CLUSTER];

// DONE
// save load - import asset via dropdown, spawn assets via asset dropdown

// DONE
// get rid of geometry shader
    // use gl_VertexID + uv offset for mesh
    // Index = sm->lm_offset + (gl_InstanceID * sm->index_count) + (gl_VertexID - sm->first_index)    


// DONE
// pack fbm noise better into 3 channels for one lookup

// DONE
// use ubo for ssao kernel

// DONE
// use subdata for debug stuff

// DONE FOR NOW
// movement system
// inject velocity based on footsteps and scale drop depth with speed, moving average for accel over time

// DONE
// fix scene bvh traversal on gpu

// DONE
// make ui for animations

// DONE
// point light shadows

// DONE
// text rendering

// DONE
// find rt60 using two formulas
// find volume by using the first length of each ray?
// (sum of (energy at hit * absorption_coefficient)) / (sum of absorption_coefficient) over each bounce of each ray

// fix sound artifacts while turning with a crossfade triple buffered pattern

// implement synthetic IR tail for late reverb

// DONE
// use defines for ubershader branches

// DONE
// for some reason nuklear isn't detecting entity menu close until the light menu is closed

// in order of importance

// IN PROGRESS

// fix mn ui selection

// light and physics smart distance culling

// make assets

// add sound / events




GLuint64 retrieve_gl_query(GLuint query)
{
    GLint available = 0;
    glGetQueryObjectiv(query, GL_QUERY_RESULT_AVAILABLE, &available);
    GLuint64 e = 0;
    if (available)
    {

        glGetQueryObjectui64v(query, GL_QUERY_RESULT, &e);
    }
    return e;
}


double time_since_last_render = 0.0;
GLuint main_queries[QUERY_FRAMES];
GLuint pl_queries[QUERY_FRAMES];
GLuint sm_queries[QUERY_FRAMES];
GLuint render_queries[QUERY_FRAMES];
GLuint cull_queries[QUERY_FRAMES];
GLuint gbuffer_queries[QUERY_FRAMES];
GLuint dof_queries[QUERY_FRAMES];
GLuint ssao_queries[QUERY_FRAMES];
GLuint taa_queries[QUERY_FRAMES];
GLuint ssr_queries[QUERY_FRAMES];
GLuint bloom_queries[QUERY_FRAMES];
int current_query_frame = 1;

SDL_Window *window = NULL;
int main(int argc, char *argv[]) {

    GLOBALS.noclip = true;
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = 0;
    SDL_GLContext context;
    NK_UNUSED(argc);
    NK_UNUSED(argv);


    if (!init(&window, &context))
        return -1;

    running = 1;

    load_gltf("../engine-assets/assets/apt.gltf");

    VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);

    AssetInstance * ai1 = init_new_asset_instance("../engine-assets/assets/apt.gltf", (vec3){0, 0, 0}, (vec4){0,0,0,1}, (vec3){1,1,1});

    init_scene_bvh();

    upload_material_data();

    GPU_upload_scene_data();
    upload_lights();

    add_all_meshes_to_buffer(vl);


    
    update_camera(&player.skeleton, &cam);
    size_t cam_ubo_offset = current_frame * cam_ubo_padded;
    CamData* frame_cam_data = (CamData*)(cam_ubo_ptr + cam_ubo_offset);    
    *frame_cam_data = cam.cam_data;
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, CAM_UBO, cam_ubo_offset, sizeof(CamData));
    GPU_init_cluster_aabbs();

    glGenQueries(QUERY_FRAMES, main_queries);
    glGenQueries(QUERY_FRAMES, pl_queries);
    glGenQueries(QUERY_FRAMES, sm_queries);
    glGenQueries(QUERY_FRAMES, render_queries);
    glGenQueries(QUERY_FRAMES, gbuffer_queries);
    glGenQueries(QUERY_FRAMES, dof_queries);
    glGenQueries(QUERY_FRAMES, ssao_queries);
    glGenQueries(QUERY_FRAMES, taa_queries);
    glGenQueries(QUERY_FRAMES, ssr_queries);
    glGenQueries(QUERY_FRAMES, cull_queries);
    glGenQueries(QUERY_FRAMES, bloom_queries);
    bool stop = false;
    float stime = 0;

    create_performance_overlay();


    // UIMenu * menu = create_menu();
    // create_ui_element(menu->uID);
    // create_ui_element(menu->uID);
    // create_ui_element(menu->uID);
    // create_ui_element(menu->uID);
    // UIElement * elements = menu->elements;
    // elements[0].translation = (vec2){1000, 500};
    // elements[0].base_pos = (vec2){1000, 500};
    // elements[0].type = UIELEMENT_RECT;
    // elements[0].scale = (vec2){30, 30};
    // elements[0].radius = 0;
    // elements[0].action_callback = ACTION_CALLBACKS[1];
    // elements[0].hover_callback = HOVER_CALLBACKS[0];
    // elements[1].translation = (vec2){1000, 500};
    // elements[1].base_pos = (vec2){1000, 500};
    // elements[1].type = UIELEMENT_RECT;
    // elements[1].radius = 0;
    // elements[1].size = (vec2){1, 1};
    // elements[1].color = (vec4){0,0,0,1};
    // elements[1].layer = 0;
    // elements[1].scale = (vec2){200, 200};
    // elements[0].layer = 1;
    // UIContainer * c = create_ui_container(&elements[1], menu->uID);
    
    // elements[0].parent_container = elements[1].container;

    // c->child_count = 1;
    // c->children[0] = elements[0].uID;

    // elements[2].translation = (vec2){1120, 500};
    // elements[2].base_pos = (vec2){1120, 500};
    // elements[2].type = UIELEMENT_RECT;
    // elements[2].scale = (vec2){30, 30};
    // elements[2].radius = 0;
    // elements[2].layer = 1;
    // elements[2].size = (vec2){1, 1};
    // create_slider(&elements[2], (vec2){0,1}, 100);
    // elements[2].as.slider.container = elements[1].container;
    // c->scrollbar = elements[2].uID;
    // c->scroll_dir = (vec2){0, 1};

    // elements[3].translation = (vec2){1500, 500};
    // elements[3].base_pos = (vec2){1120, 500};
    // elements[3].type = UIELEMENT_RECT;
    // elements[3].scale = (vec2){20, 20};
    // elements[3].radius = 0;
    // elements[3].layer = 1;
    // elements[3].size = (vec2){1, 1};
    // create_slider(&elements[3], (vec2){0,1}, 100);
        
    add_all_text_boxes_to_draw_buffer();

    draw_dirty = true;
    

    double time_current = SDL_GetTicks() / 1000.0f;
    double physics_accumulator = 0.0;
    const double physics_dt = 1.0 / 60.0;
    SDL_Event event_queue[MAX_NUKLEAR_EVENTS];
    int event_queue_count = 0;
    SDL_Event sdl_queue[MAX_SDL_EVENTS];
    int sdl_queue_count = 0;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (ENTITIES[i].free) continue;
        Entity * ent = &ENTITIES[i];
        printf("MN: %d\n", ent->mesh_node);
    }
    
    
    while (running)
    {
        
        double render_dt = 1.0 / EDITOR_SETTINGS.render_framerate_target;
        Timer master_timer, cam_bvh_timer, pc_timer, render_timer;
        start_timer(&master_timer);
        now = SDL_GetPerformanceCounter();

        double time_new = SDL_GetTicks() / 1000.0f;
        frame_time = time_new - time_current;
        if (frame_time > 0.25) frame_time = 0.25;
        time = fmodf(time_new, 1.0f);
        dt = 1.0f/60;

        time_current = time_new;
        physics_accumulator += frame_time;
        time_since_last_render += frame_time;
        
        bool updated_editor_menu = false;


        while (physics_accumulator >= physics_dt)
        {

            update_camera(&player.skeleton, &cam);
            process_input(&window, event_queue, &event_queue_count, sdl_queue, &sdl_queue_count, physics_dt);

            check_lighting_pv();
            check_audio_pv();
            update_player_audio_probe();
                
            start_timer(&cam_bvh_timer);
            update_audio_occlusion(&player);
            double cam_bvh_time = end_timer(&cam_bvh_timer);
            SDL_LockAudioDevice(SDL_AUDIO_DEVICE);
            audio_update(&player);
            SDL_UnlockAudioDevice(SDL_AUDIO_DEVICE);
            CAM_POS = (vec3){cam.cam_data.cam_pos.x, cam.cam_data.cam_pos.y, cam.cam_data.cam_pos.z};
            Light * lights = CAST_VEC(GLOBALS.lights, Light);

            CURRENT_DIRECTIONAL = 0;
            for (int i = 0; i < GLOBALS.lights.count; i++)
            {
                Light * l = &lights[i];
                if (l->light_type != DIRECTIONAL_LIGHT) continue;
                if (point_in_aabb(CAM_POS, &l->area))
                {
                    CURRENT_DIRECTIONAL = i;
                    break;
                }
            }
        
            DIRECTIONAL_LIGHT_COUNT = 0;
            for (int i = 0; i < GLOBALS.lights.count; i++)
            {
                
                Light * l = &lights[i];
                if (l->light_type != DIRECTIONAL_LIGHT) continue;
                update_light_camera(&LIGHT_CAM[DIRECTIONAL_LIGHT_COUNT],&lights[i]);
                l->layer = DIRECTIONAL_LIGHT_COUNT;
                DIRECTIONAL_LIGHT_COUNT++;
            }

            init_scene_bvh();


            start_timer(&pc_timer);

            update_physics_components();

            CollisionPoints p;
            double pc_time = end_timer(&pc_timer);

            physics_accumulator -= physics_dt;
            
        
            updated_editor_menu = true;
        }

        if (time_since_last_render >= render_dt)
        {

            update_ui_state(MOUSE_X, MOUSE_Y);
            handle_held_events();
            nk_input_begin(NK_CTX);
            for (int i = 0; i < event_queue_count; i++)
            {
                nk_sdl_handle_event(&event_queue[i]);
            }
            event_queue_count = 0;
            for (int i = 0; i < sdl_queue_count; i++)
            {
                process_sdl_event(sdl_queue[i]);
            }
            sdl_queue_count = 0;

            nk_input_end(NK_CTX);
            render_editor_menu();
            render();
            
            nk_sdl_render(NK_ANTI_ALIASING_ON, MAX_VERTEX_MEMORY, MAX_ELEMENT_MEMORY);
            SDL_GL_SwapWindow(window);


            time_since_last_render -= render_dt;

            int oldest_frame = (current_query_frame + 1) % QUERY_FRAMES;        
            GLuint64 query_s[PQUERY_MAX];
            GLint available = 0;
            
            query_s[PQUERY_SM] = retrieve_gl_query(sm_queries[oldest_frame]);
            query_s[PQUERY_CULL] = retrieve_gl_query(cull_queries[oldest_frame]);
            query_s[PQUERY_MAIN] = retrieve_gl_query(main_queries[oldest_frame]);
            query_s[PQUERY_PL] = retrieve_gl_query(pl_queries[oldest_frame]);
            query_s[PQUERY_GBUFFER] = retrieve_gl_query(gbuffer_queries[oldest_frame]);
            query_s[PQUERY_DOF] = retrieve_gl_query(dof_queries[oldest_frame]);
            query_s[PQUERY_SSAO] = retrieve_gl_query(ssao_queries[oldest_frame]);
            query_s[PQUERY_TAA] = retrieve_gl_query(taa_queries[oldest_frame]);
            query_s[PQUERY_SSR] = retrieve_gl_query(ssr_queries[oldest_frame]);
            query_s[PQUERY_BLOOM] = retrieve_gl_query(bloom_queries[oldest_frame]);
            query_s[PQUERY_RENDER] = retrieve_gl_query(render_queries[oldest_frame]);


            update_performance_overlay(query_s);

            
            current_query_frame = (current_query_frame + 1) % QUERY_FRAMES;
            DEBUG_FACE_COUNT = 0;
            DEBUG_POINT_COUNT = 0;
            DEBUG_LINE_COUNT = DEBUG_APROBE_RAY_COUNT;
        }


        
        double end = end_timer(&master_timer);
        update_animation_components(dt);
        last = now;
    }

    if (GLOBALS.scene_data.APV_FILE)
    {
        fclose(GLOBALS.scene_data.APV_FILE);
        GLOBALS.scene_data.APV_FILE = NULL;
    }
    if (GLOBALS.scene_data.LPV_FILE)
    {
        fclose(GLOBALS.scene_data.LPV_FILE);
        GLOBALS.scene_data.LPV_FILE = NULL;
    }
    SDL_DestroyMutex(AUDIO_SCRATCH.aprobe_mutex);
    SDL_CloseAudioDevice(SDL_AUDIO_DEVICE);
    ma_engine_uninit(&AUDIO_ENGINE);
    delete_hrtf_worker_thread(&HRTF_TD);
    fftwf_cleanup_threads();
    fftwf_cleanup();
    nk_sdl_shutdown();
    SDL_GL_DeleteContext(context);

    SDL_DestroyWindow(window);
    
    SDL_Quit();

    return 0;
}
