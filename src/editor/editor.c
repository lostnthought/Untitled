
#include "SDL2/SDL_video.h"
#include "core/camera.h"
#include "core/engine_types.h"
#include "ui/fonts.h"
#include "load/gltf.h"
#include "load/load.h"
#include "editor/editor.h"
#include "utilities/engine_utilities.h"
#include "ECS/entity.h"
#include "load/init.h"
#include "light/lightmap.h"
#include "utilities/mathf.h"
#include "render/render.h"
#include "utilities/types.h"
#include "ui/ui.h"
#include "core/acceleration.h"
#include "core/nk_config.h"
#include "physics/physics.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"
#include "Windows.h"
#include "audio/audio.h"
#include "ui/ui_callbacks.h"
#include "utilities/utilities.h"
#include <rpcndr.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief The structures, functions, constants, and variables within the editor system.
 * @addtogroup editor_group 
 * @{
 */
LightEditorMenu LIGHT_MENU = {0};
MainEditorMenu MAIN_MENU = {0};
LProbeEditorMenu LPROBE_MENU = {0};
AProbeEditorMenu APROBE_MENU = {0};
UIElementEditorMenu UIELEMENT_MENU = {0};
TextBoxEditorMenu TEXTBOX_MENU = {0};
AssetInstanceEditorMenu AI_MENU = {0};
MeshNodeEditorMenu MN_MENU = {0};


#define MAX_GLTF_NAMES 256
char gltf_names[MAX_GLTF_NAMES][256];
int gltf_names_count = 0;
#define MAX_RPROBE_NAMES 256
char rprobe_names[MAX_RPROBE_NAMES][256];
int rprobe_names_count = 0;

FileDirectoryRegistry scene_registry = {0};
FileDirectoryRegistry lightmap_registry = {0};
FileDirectoryRegistry light_registry = {0};
FileDirectoryRegistry sound_registry = {0};
FileDirectoryRegistry aportal_registry = {0};
FileDirectoryRegistry rprobe_registry = {0};
FileDirectoryRegistry lprobe_registry = {0};
FileDirectoryRegistry aprobe_registry = {0};
FileDirectoryRegistry scene_apv_registry = {0};
FileDirectoryRegistry scene_lpv_registry = {0};
FileDirectoryRegistry ui_registry = {0};

void change_fullscreen_option(FullscreenOptions option)
{

    SDL_DisplayMode DM;
    if (SDL_GetDesktopDisplayMode(0, &DM) == 0)
    {
        switch(option)
        {
            case FSOPT_FULLSCREEN:
                SCREEN_WIDTH = DM.w;
                SCREEN_HEIGHT = DM.h;
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN);
                break;
            case FSOPT_BORDERLESS:
                SCREEN_WIDTH = DM.w;
                SCREEN_HEIGHT = DM.h;
                SDL_SetWindowFullscreen(window, SDL_WINDOW_FULLSCREEN_DESKTOP);
                break;
            case FSOPT_WINDOWED:
                SDL_SetWindowFullscreen(window, 0);
                SDL_GetWindowSize(window, &SCREEN_WIDTH, &SCREEN_HEIGHT);
                break;
            default:
                break;
        }
    }
    EDITOR_SETTINGS.current_fullscreen_option = option;
    
}

void change_render_resolution()
{
    
    if (EDITOR_SETTINGS.MSAA)
    {
        EDITOR_SETTINGS.FULL_GBUFFER = false;
        allocate_gbuffer_msaa();
        allocate_main_fbo();
        glBindFramebuffer(GL_FRAMEBUFFER, MAIN_FBO.fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GBUFFER_FBO.depth_rbo.i);
    }
    else
    {
        glBindFramebuffer(GL_FRAMEBUFFER, MAIN_RESOLVE_FBO.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.depth_texture.i, 0);
    }
    change_fbo_resolution(&DEPTH_DOWNSAMPLE_FBO, RENDER_WIDTH / 2, RENDER_HEIGHT / 2, false);

}


void change_ssao_res(int w, int h)
{
    SSAO_WIDTH = w;
    SSAO_HEIGHT = h;
    
    change_fbo_resolution(&SSAO_FBO, w, h, false);
    change_fbo_resolution(&SSAO_BLUR_FBO, w, h, false);
}
void change_dof_res(int w, int h)
{
    DOF_WIDTH = w;
    DOF_HEIGHT = h;
    change_fbo_resolution(&COC_FBO, w, h, false);
    change_fbo_resolution(&BOKEH_FBO, w, h, false);
    
}

void change_bloom_res(int w, int h)
{
    BLOOM_WIDTH = w;
    BLOOM_HEIGHT = h;
    
    int current_width = w;
    int current_height = h;
    for (int i = 0; i < 5; i++)
    {
        change_fbo_resolution(&BLOOM_PYRAMID[i], current_width, current_height, false);

        
        current_width /= 2;
        current_height /= 2;
    }
}

Undo UNDOS[MAX_UNDOS] = {0};
int undo_stack_idx = 0;
size_t undo_stack_size = 0;
Undo * top_undo = NULL;
Undo * next_redo = NULL;

void UNDO_INIT()
{
    for (int i = 0; i < MAX_UNDOS; i++)
    {
        UNDOS[i].id = i;
        UNDOS[i].free = true;
        UNDOS[i].prev = NULL;
        UNDOS[i].next = NULL;
        
    }
    top_undo = &UNDOS[0];
}

void clear_special_undo_bytes(Undo * undo)
{
    for (int i = 0; i < undo->special_count; i++)
    {
        if (undo->special_bytes[i]) free(undo->special_bytes[i]);
    }
    if (undo->special_bytes) free(undo->special_bytes);
    undo->special_count = 0;
    undo->special_bytes = NULL;
    undo->byte_size = 0;
    
}

void clear_undo(Undo * undo)
{
    undo->next = NULL;
    undo->free = true;
    if (undo->special_count > 0)
    {
        for (int i = 0; i < undo->special_count; i++)
        {
            if (undo->special_bytes) free(undo->special_bytes[i]);
            undo->special_bytes[i] = NULL;
        }
        if (undo->special_bytes) free(undo->special_bytes);
        undo->special_bytes = NULL;
    }
    undo->byte_offset = 0;
    undo->byte_size = 0;
    undo->special_count = 0;
    
}

void destroy_undo(Undo * undo)
{
    if (undo->next)
    {
        undo->next->prev = NULL;
    }
    
    Undo * current = undo;
    while(current->prev)
    {
        clear_undo(current->prev);
        Undo * tmp = current;
        current = current->prev;
        tmp->prev = NULL;
    }

    clear_undo(undo);

}

Undo * find_last_undo()
{

    Undo * current = &UNDOS[undo_stack_idx];
    while(current->prev)
    {
        current = current->prev;
    }
    return current;

}

Undo * find_empty_undo()
{
    for (int i = 0; i < MAX_UNDOS; i++)
    {
        if (UNDOS[i].free)
        {
            destroy_undo(&UNDOS[i]);
            return &UNDOS[i];
        }
    }

    Undo * undo = find_last_undo();
    destroy_undo(undo);
    return undo;
}



void link_undo(Undo * undo)
{

    if (undo->prev)
    {
        undo->prev->next = NULL;
    }
    undo->prev = top_undo;
    if (top_undo) top_undo->next = undo;
    if (undo->next)
    {
        undo->next->prev = NULL;
    }
    undo->next = NULL;
    undo->free = false;
    top_undo = undo;
    next_redo = NULL;
    
}


void check_stack_size()
{
    while (undo_stack_size > EDITOR_SETTINGS.undo_stack_max_byte_size && top_undo)
    {
        Undo * current = top_undo;
        if (!current) return;
        while(current->prev)
        {
            current = current->prev;
        }
        destroy_undo(current);
        if (top_undo == current) top_undo = NULL;
    }
}

void perform_newest_undo()
{

    if (!top_undo)
    {
        return;
    }
    Undo * undo = top_undo;
    execute_undo(undo);

    
    next_redo = top_undo;
    top_undo = top_undo->prev;

}

void perform_newest_redo()
{
    if (!next_redo) return;

    Undo * undo = next_redo;
    execute_undo(undo);
    
    top_undo = undo;
    next_redo = undo->next;
}


void destroy_redo_history()
{

    if (!top_undo) return;
    Undo * current = top_undo;
    Undo * next = current->next;
    while(next)
    {

        current = next;
        current->free = true;
        current->prev = NULL;
        next = current->next;
        clear_undo(current);

    }
    
}


void push_nk_slider_undo(NKSlider * s, UndoStructure structure, size_t byte_offset)
{

    Undo * undo = find_empty_undo();
    undo->byte_size = s->byte_size;
    undo->byte_offset = byte_offset;
    undo->special_count = 0;
    memcpy(undo->bytes, s->bytes, s->byte_size);
    memcpy(&undo->structure, &structure, sizeof(UndoStructure));
    link_undo(undo);
    undo_stack_size += s->byte_size;
    check_stack_size();

}

void push_raw_bytes_undo(void * bytes, size_t byte_size, UndoStructure structure, size_t byte_offset)
{

    Undo * undo = find_empty_undo();
    undo->byte_size = byte_size;
    undo->byte_offset = byte_offset;
    undo->special_count = 0;
    memcpy(undo->bytes, bytes, byte_size);
    memcpy(&undo->structure, &structure, sizeof(UndoStructure));
    link_undo(undo);
    undo_stack_size += byte_size;
    check_stack_size();

}

void push_special_undo(UndoStructure structure, uint8_t ** bytes, size_t total_byte_size, size_t bytes_count, size_t byte_offset)
{

    Undo * undo = find_empty_undo();
    undo->special_count = bytes_count;
    undo->byte_offset = byte_offset;
    undo->special_bytes = bytes;
    memcpy(&undo->structure, &structure, sizeof(UndoStructure));
    link_undo(undo);
    undo_stack_size += total_byte_size;
    check_stack_size();
    
}

void push_special_undo_normal_bytes(UndoStructure structure, void * bytes, size_t byte_size, size_t byte_offset)
{

    Undo * undo = find_empty_undo();
    undo->byte_offset = byte_offset;
    undo->special_count = 0;
    memcpy(undo->bytes, bytes, byte_size);
    memcpy(&undo->structure, &structure, sizeof(UndoStructure));
    link_undo(undo);
    undo_stack_size += byte_size;
    check_stack_size();
    
}

UndoStructure create_undo_structure(UndoStructureType type, int id[8], UndoSpecial special)
{
    UndoStructure a = (UndoStructure)
    {
        .special = special,
        .type = type
    };
    if (id)
    {
        memcpy(a.ids, id, sizeof(int) * 8);
    }
    return a;
}

UndoStructure create_singular_id_undo_structure(UndoStructureType type, int id, UndoSpecial special)
{
    UndoStructure a = (UndoStructure)
    {
        .special = special,
        .type = type
    };
    a.ids[0] = id;
    return a;
}

UndoStructure create_light_undo_structure(int id, UndoSpecial special)
{
    UndoStructure a = (UndoStructure)
    {
        .special = special,
        .type = UNDOSTRUCT_LIGHT
    };
    a.ids[0] = id;
    return a;
}


void copy_special_undo(Undo * undo, uint8_t *** special_bytes, int ids[8])
{
    if (undo->special_count > 0)
    {
        
        *special_bytes = malloc(sizeof(uint8_t*) * undo->special_count);
        for (int i = 0; i < undo->special_count; i++)
        {
            (*special_bytes)[i] = malloc(undo->byte_size);
            
        }

    }
    else
    {
        *special_bytes = NULL;
    }

    
}



void execute_non_special_undo(Undo * undo, uint8_t * p)
{
    uint8_t temp[MAX_UNDO_BYTES];
    memcpy(temp, p + undo->byte_offset, undo->byte_size);
    memcpy(p + undo->byte_offset, undo->bytes, undo->byte_size);
    memcpy(undo->bytes, temp, undo->byte_size);
}



void execute_light_undo(Undo * undo)
{
    Light * lights = CAST_VEC(GLOBALS.lights, Light);
    Light * light = &lights[undo->structure.ids[0]];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)light);
            break;
        case UNDOLIGHT_SPAWN:

            uint8_t ** bytes = malloc(sizeof(uint8_t*));
            bytes[0] = malloc(sizeof(Light));
            memcpy(bytes[0], light, sizeof(Light));
            size_t total_bytes = sizeof(uint8_t*) + sizeof(Light);
            undo->byte_size = total_bytes;
            undo->special_bytes = bytes;
            undo->special_count = 1;
            destroy_entity(&ENTITIES[lights[GLOBALS.lights.count-1].entity_id]);
            int temp = EDITOR_SETTINGS.selected_light_id;
            EDITOR_SETTINGS.selected_light_id = undo->structure.ids[1];
            undo->structure.ids[1] = temp;

            REMOVE_FROM_VEC(&GLOBALS.lights, undo->structure.ids[0], Light);
            
            undo->structure.special = UNDOLIGHT_DELETE;

            break;

        case UNDOLIGHT_DELETE:

            for (int i = GLOBALS.lights.count; i > undo->structure.ids[0]; i--)
            {
                lights[i] = lights[i-1];
            }
            GLOBALS.lights.count++;
            memcpy(&lights[undo->structure.ids[0]], undo->special_bytes[0], sizeof(Light));
            undo->structure.ids[1] = EDITOR_SETTINGS.selected_light_id;
            EDITOR_SETTINGS.selected_light_id = undo->structure.ids[0];
            free(undo->special_bytes[0]);
            free(undo->special_bytes);
            undo->special_bytes = NULL;
            undo->special_count = 0;
            undo->byte_size = 0;
            undo->structure.special = UNDOLIGHT_SPAWN;
            break;
        default:
            break;
    }
    light->sm_dirty = true;
    GLOBALS.lights_dirty = true;
}

void execute_globals_undo(Undo * undo)
{

    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)&GLOBALS);
            break;
        // this needs a redo
        case UNDOGLOBAL_SPAWN_ASSET_INSTANCE:

            destroy_asset_instance(undo->structure.ids[0]);

            VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
            upload_material_data();
            GPU_upload_scene_data();
            upload_lights();
            MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
            Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
            init_lightmap_data(&GLOBALS.scene_data.lightmap_data);
            for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
            {
                if (mns[i].mesh == -1 || mns[i].free) continue;
                if (!meshes[mns[i].mesh].static_mesh || !mns[i].static_mesh || mns[i].physics_mode) continue;
        
                build_uv_islands(&mns[i]);    
            }
            add_all_meshes_to_buffer(vl);
            draw_dirty = true;
            
            break;
        default:
            break;
    }
    
}


UndoStructure create_editor_settings_undo_structure(UndoSpecial special)
{
    UndoStructure a = (UndoStructure)
    {
        .special = special,
        .type = UNDOSTRUCT_EDITOR_SETTINGS
    };
    return a;
}
void execute_editor_settings_undo(Undo * undo)
{

    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)&EDITOR_SETTINGS);
            break;
        case UNDOEDITORSETTINGS_FULLSCREENOPT:
            temp[0] = EDITOR_SETTINGS.selected_fullscreen_option;
            EDITOR_SETTINGS.selected_fullscreen_option = undo->structure.ids[0];
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            change_fullscreen_option(EDITOR_SETTINGS.selected_fullscreen_option);
            break;
        case UNDOEDITORSETTINGS_SCREEN_RES:
            temp[0] = SCREEN_WIDTH;
            temp[1] = SCREEN_HEIGHT;
            SCREEN_WIDTH = undo->structure.ids[0];
            SCREEN_HEIGHT = undo->structure.ids[1];
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            break;
        case UNDOEDITORSETTINGS_RENDER_RES:
            temp[0] = RENDER_WIDTH;
            temp[1] = RENDER_HEIGHT;
            RENDER_WIDTH = undo->structure.ids[0];
            RENDER_HEIGHT = undo->structure.ids[1];
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            change_render_resolution();
            break;
        case UNDOEDITORSETTINGS_SSAO_RES:
            temp[0] = SSAO_WIDTH;
            temp[1] = SSAO_HEIGHT;
            change_ssao_res(undo->structure.ids[0], undo->structure.ids[1]);
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            break;
        case UNDOEDITORSETTINGS_DOF_RES:
            temp[0] = DOF_WIDTH;
            temp[1] = DOF_HEIGHT;
            change_dof_res(undo->structure.ids[0], undo->structure.ids[1]);
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            break;
        case UNDOEDITORSETTINGS_BLOOM_RES:
            temp[0] = BLOOM_WIDTH;
            temp[1] = BLOOM_HEIGHT;
            change_bloom_res(undo->structure.ids[0], undo->structure.ids[1]);
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            break;
        case UNDOEDITORSETTINGS_HALFSIZE_EFFECTS:
            temp[0] = SSAO_WIDTH;
            temp[1] = SSAO_HEIGHT;
            temp[2] = DOF_WIDTH;
            temp[3] = DOF_HEIGHT;
            temp[4] = BLOOM_WIDTH;
            temp[5] = BLOOM_HEIGHT;
            change_ssao_res(undo->structure.ids[0], undo->structure.ids[1]);
            change_dof_res(undo->structure.ids[2], undo->structure.ids[3]);
            change_bloom_res(undo->structure.ids[4], undo->structure.ids[5]);
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            break;
        default:
            break;
    }
    
}

UndoStructure create_rprobe_undo_structure(int id, UndoSpecial special)
{
    UndoStructure a = (UndoStructure)
    {
        .special = special,
        .type = UNDOSTRUCT_RPROBE
    };
    a.ids[0] = id;
    return a;
}

void execute_rprobe_undo(Undo * undo)
{

    ReflectionProbe * rprobes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);
    ReflectionProbe * rp = &rprobes[undo->structure.ids[0]];
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)rp);
            break;
        case UNDORPROBE_SPAWN:
            {

                uint8_t ** bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(ReflectionProbe));
                memcpy(bytes[0], rp, sizeof(ReflectionProbe));

                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_r_probe;
                
                REMOVE_FROM_VEC(&GLOBALS.scene_data.reflection_probes, undo->structure.ids[0], ReflectionProbe);
                EDITOR_SETTINGS.selected_r_probe = undo->structure.ids[1];

                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                undo->special_bytes = bytes;
                undo->byte_size = sizeof(uint8_t*) + sizeof(ReflectionProbe);
                undo->special_count = 1;

                undo->structure.special = UNDORPROBE_DELETE;
                
            }
            break;

        case UNDORPROBE_DELETE:
            {
                
                SHIFT_VEC_RIGHT(&GLOBALS.scene_data.reflection_probes, undo->structure.ids[0], ReflectionProbe);
                memcpy(rp, undo->special_bytes[0], sizeof(ReflectionProbe));
                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_r_probe;
                EDITOR_SETTINGS.selected_r_probe = undo->structure.ids[1];

                clear_special_undo_bytes(undo);
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);

                undo->structure.special = UNDORPROBE_SPAWN;
            }
            
            break;
        default:
            break;
    }
}

void execute_lprobe_undo(Undo * undo)
{
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);
    ProbeVolume * pv = &pvs[undo->structure.ids[0]];
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            
            execute_non_special_undo(undo, (uint8_t*)pv);
            break;

        case UNDOLPROBE_SPAWN:
            {

                // set up bytes to receive the current probe volume
                uint8_t ** bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(ProbeVolume));
                memcpy(bytes[0], pv, sizeof(ProbeVolume));
                // delete the probe volume to undo the spawn
                REMOVE_FROM_VEC(&GLOBALS.scene_data.lighting_probe_volumes, undo->structure.ids[0], ProbeVolume);
                // flip the ids that will be used for the redo
                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_lighting_probe_volume_id;
                EDITOR_SETTINGS.selected_lighting_probe_volume_id = undo->structure.ids[1];
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                // reset special fields to reflect how we set up the redo
                undo->special_bytes = bytes;
                undo->special_count = 1;
                undo->byte_size = sizeof(uint8_t*) + sizeof(ProbeVolume);
                // on redo we will "undo the delete"
                undo->structure.special = UNDOLPROBE_DELETE;

            }
            break;

        case UNDOLPROBE_RESET_LPVS:

            for (int i = 0; i < LPV_SLOTS; i++)
            {
                GLOBALS.scene_data.LOADED_LIGHTING_PV[i].pv_id = undo->structure.ids[i];
                if (GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes) free(GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes);
                if (undo->special_bytes[i])
                {
                    GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes = malloc(sizeof(float) * LPV_PADDED_COUNT * 3 * SH_MAX);
                    memcpy(GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes, undo->special_bytes[i], sizeof(float) * LPV_PADDED_COUNT * 3 * SH_MAX);
                    GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[i] = true;
                }
                else
                {
                    GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes = NULL;
                    GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[i] = false;
                }
            }
            for (int i = 0; i < LPV_SLOTS; i++)
            {
                if (undo->special_bytes[i])
                {
                    free(undo->special_bytes[i]);
                }
            }

            free(undo->special_bytes);
            undo->special_bytes = NULL;
            undo->special_count = 0;
            undo->byte_size = 0;

            undo->structure.special = REDOLPROBE_RESET_LPVS;

            break;
        case REDOLPROBE_RESET_LPVS:
            size_t total_bytes = sizeof(uint8_t*) * LPV_SLOTS;
            for (int i = 0; i < LPV_SLOTS; i++)
            {
                if (undo->special_bytes[i])
                {
                    free(undo->special_bytes[i]);
                }
                if (GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes)
                {
                    
                    size_t size = sizeof(float) * LPV_PADDED_COUNT * 3 * SH_MAX;
                    undo->special_bytes[i] = malloc(size);
                    memcpy(undo->special_bytes[i], GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes, size);
                    total_bytes += size;

                }
                else
                {
                    undo->special_bytes[i] = NULL;
                }
            }

            undo->structure.ids[0] = GLOBALS.scene_data.LOADED_LIGHTING_PV[0].pv_id;
            undo->structure.ids[1] = GLOBALS.scene_data.LOADED_LIGHTING_PV[1].pv_id;
            undo->structure.ids[2] = GLOBALS.scene_data.LOADED_LIGHTING_PV[2].pv_id;
            undo->structure.ids[3] = GLOBALS.scene_data.LOADED_LIGHTING_PV[3].pv_id;
            undo->structure.ids[4] = GLOBALS.scene_data.LOADED_LIGHTING_PV[4].pv_id;
            undo->structure.ids[5] = GLOBALS.scene_data.LOADED_LIGHTING_PV[5].pv_id;

            undo->special_count = 6;
            undo->byte_size = total_bytes;
            
            reset_lighting_pvs();

            break;
        case UNDOLPROBE_OBB_ROT:

            uint8_t * temp_b0 = malloc(sizeof(mat3));
            uint8_t * temp_b1 = malloc(sizeof(vec3));
            
            memcpy(temp_b0, (uint8_t*)pv + undo->byte_offset, sizeof(mat3));

            vec3 obbaxis = (vec3){EDITOR_SETTINGS.obb_axis_x_angle, EDITOR_SETTINGS.obb_axis_y_angle, EDITOR_SETTINGS.obb_axis_z_angle};

            memcpy(temp_b1, &obbaxis, sizeof(vec3));
            
            memcpy((uint8_t*)pv + undo->byte_offset, undo->special_bytes[0], sizeof(mat3));
            vec3 * axis_angle = (vec3*)undo->special_bytes[1];
            EDITOR_SETTINGS.obb_axis_x_angle = axis_angle->x;
            EDITOR_SETTINGS.obb_axis_y_angle = axis_angle->y;
            EDITOR_SETTINGS.obb_axis_z_angle = axis_angle->z;

            free(undo->special_bytes[0]);
            free(undo->special_bytes[1]);

            undo->special_bytes[0] = temp_b0;
            undo->special_bytes[1] = temp_b1;

            break;
        case UNDOLPROBE_DELETE:

            {
                
                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_lighting_probe_volume_id;
                SHIFT_VEC_RIGHT(&GLOBALS.scene_data.lighting_probe_volumes, undo->structure.ids[0], ProbeVolume);
                memcpy(pv, undo->special_bytes[0], sizeof(ProbeVolume));
                EDITOR_SETTINGS.selected_lighting_probe_volume_id = undo->structure.ids[0];

                free(undo->special_bytes[0]);
                free(undo->special_bytes);
                undo->special_bytes = NULL;
                undo->special_count = 0;
                undo->byte_size = 0;
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                undo->structure.special = UNDOLPROBE_SPAWN;
            }
            
            break;
        case UNDOLPROBE_RESET_LPV_SIZE:
            {

                temp[0] = undo->structure.ids[0];
                temp[1] = pv->byte_offset;
                temp[2] = pv->size;
                pv->byte_offset = undo->structure.ids[1];
                pv->size = undo->structure.ids[2];
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                
            }
            break;
        default:
            break;
    }
}

void execute_aprobe_undo(Undo * undo)
{
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);
    ProbeVolume * pv = &pvs[undo->structure.ids[0]];
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)pv);
            break;

        case UNDOAPROBE_SPAWN_APV:
            {
                
                uint8_t ** bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(ProbeVolume));
                memcpy(bytes[0], pv, sizeof(ProbeVolume));

           
                REMOVE_FROM_VEC(&GLOBALS.scene_data.audio_probe_volumes, undo->structure.ids[0], ProbeVolume);
                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_audio_probe_volume_id;
                EDITOR_SETTINGS.selected_audio_probe_volume_id = undo->structure.ids[1];
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                undo->special_bytes = bytes;
                undo->special_count = 1;
                undo->byte_size = sizeof(uint8_t*) + sizeof(ProbeVolume);
                undo->structure.special = UNDOAPROBE_DELETE_APV;
            }
            break;

        case UNDOAPROBE_RESET_APV_SIZE:

            {
                
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                bytes[0] = malloc(sizeof(size_t));
                bytes[1] = malloc(sizeof(size_t));
                *((size_t*)bytes[0]) = pv->byte_offset;
                *((size_t*)bytes[1]) = pv->size;
            
                pv->byte_offset = *(size_t*)undo->special_bytes[0];
                pv->size = *(size_t*)undo->special_bytes[1];
                clear_special_undo_bytes(undo);
                undo->byte_size = sizeof(uint8_t*) + sizeof(size_t) * 2;
                undo->special_count = 2;
                undo->special_bytes = bytes;
                
            }
            
            break;
        case UNDOAPROBE_DELETE_APV:
            {
                
                temp[0] = undo->structure.ids[0];
                temp[1] = EDITOR_SETTINGS.selected_audio_probe_volume_id;
                SHIFT_VEC_RIGHT(&GLOBALS.scene_data.audio_probe_volumes, undo->structure.ids[0], ProbeVolume);
                memcpy(pv, undo->special_bytes[0], sizeof(ProbeVolume));
                EDITOR_SETTINGS.selected_audio_probe_volume_id = undo->structure.ids[0];

                free(undo->special_bytes[0]);
                free(undo->special_bytes);
                undo->special_bytes = NULL;
                undo->special_count = 0;
                undo->byte_size = 0;
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                undo->structure.special = UNDOAPROBE_SPAWN_APV;

            }
            break;
        case UNDOAPROBE_OBB_ROT:

            uint8_t * temp_b0 = malloc(sizeof(mat3));
            uint8_t * temp_b1 = malloc(sizeof(vec3));
            
            memcpy(temp_b0, (uint8_t*)pv + undo->byte_offset, sizeof(mat3));

            vec3 obbaxis = (vec3){EDITOR_SETTINGS.obb_axis_x_angle, EDITOR_SETTINGS.obb_axis_y_angle, EDITOR_SETTINGS.obb_axis_z_angle};

            memcpy(temp_b1, &obbaxis, sizeof(vec3));
            

            memcpy((uint8_t*)pv + undo->byte_offset, undo->special_bytes[0], sizeof(mat3));
            vec3 * axis_angle = (vec3*)undo->special_bytes[1];
            EDITOR_SETTINGS.obb_axis_x_angle = axis_angle->x;
            EDITOR_SETTINGS.obb_axis_y_angle = axis_angle->y;
            EDITOR_SETTINGS.obb_axis_z_angle = axis_angle->z;

            free(undo->special_bytes[0]);
            free(undo->special_bytes[1]);

            undo->special_bytes[0] = temp_b0;
            undo->special_bytes[1] = temp_b1;

            break;
        default:
            break;
    }
}


void execute_aemitter_undo(Undo * undo)
{
    
    AudioEmitter * e = &AUDIO_EMITTERS[undo->structure.ids[0]];
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)e);
            break;
        case UNDOAEMITTER_SPAWN:
            destroy_emitter(undo->structure.ids[0]);

            temp[0] = undo->structure.ids[0];
            temp[1] = EDITOR_SETTINGS.selected_audio_emitter_id;
            EDITOR_SETTINGS.selected_audio_emitter_id = undo->structure.ids[1];
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            undo->structure.special = UNDOAEMITTER_DELETE_EMITTER;
            break;
        case UNDOAEMITTER_DELETE_EMITTER:
            load_emitter(e, e->path, e->is_loop, e->is_occludable, e->base_volume);
            e->free = false;
            undo->structure.ids[1] = EDITOR_SETTINGS.selected_audio_emitter_id;
            EDITOR_SETTINGS.selected_audio_emitter_id = undo->structure.ids[0];
            undo->structure.special = UNDOAEMITTER_SPAWN;
            break;
        case UNDOAEMITTER_DELETE_SOUND_OBJ:
            load_emitter(e, e->path, e->is_loop, e->is_occludable, e->base_volume);
            undo->structure.special = UNDOAEMITTER_LOAD_SOUND_OBJ;
            break;
        case UNDOAEMITTER_LOAD_SOUND_OBJ:
            unload_emitter(undo->structure.ids[0]);
            undo->structure.special = UNDOAEMITTER_DELETE_SOUND_OBJ;
            break;
        case UNDOAEMITTER_PLAY_EMITTER:
            end_emitter(e);
            undo->structure.special = UNDOAEMITTER_STOP_EMITTER;
            break;
        case UNDOAEMITTER_STOP_EMITTER:
            play_emitter(e, e->pos);
            undo->structure.special = UNDOAEMITTER_PLAY_EMITTER;
            break;
        default:
            break;
    }

}


void execute_aportal_undo(Undo * undo)
{
    AudioPortal * apvs = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    AudioPortal * apv = &apvs[undo->structure.ids[0]];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)apv);
            break;
        case UNDOAPORTAL_SPAWN:

            uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
            bytes[0] = malloc(sizeof(AudioPortal));
            bytes[1] = malloc(sizeof(int) * apv->adjacent_portals.count);
            memcpy(bytes[0], apv, sizeof(AudioPortal));
            memcpy(bytes[1], apv->adjacent_portals.buffer, sizeof(int) * apv->adjacent_portals.count);
            
            remove_audio_portal(undo->structure.ids[0]);
            undo->structure.special = UNDOAPORTAL_DELETE;

            undo->special_bytes = bytes;
            undo->special_count = 2;
            undo->byte_size = sizeof(uint8_t*) * 2 + sizeof(AudioPortal) + sizeof(int) * apv->adjacent_portals.count;
            
            
            break;
        case UNDOAPORTAL_DELETE:

            SHIFT_VEC_RIGHT(&GLOBALS.audio_portals, undo->structure.ids[0], AudioPortal);
            memcpy(apv, undo->special_bytes[0], sizeof(AudioPortal));
            if (undo->special_bytes[1])
            {
                memcpy(apv->adjacent_portals.buffer, undo->special_bytes[1], sizeof(int) * apv->adjacent_portals.capacity);
            }
            clear_special_undo_bytes(undo);
            
            undo->structure.special = UNDOAPORTAL_SPAWN;

            break;
        default:
            break;
    }

}


void execute_ui_menu_undo(Undo * undo)
{
    
    UIMenu * menus = CAST_VEC(MENUS, UIMenu);
    UIMenu * menu = &menus[undo->structure.ids[1]];

    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)menu);
            break;
        default:
            break;
    }
    
}

void execute_ui_element_undo(Undo * undo)
{

    UIMenu * menus = CAST_VEC(MENUS, UIMenu);
    UIMenu * menu = &menus[undo->structure.ids[1]];
    UIElement * el = &menu->elements[undo->structure.ids[0]];
    
    UIContainer * cs = menu->containers;
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)el);
            break;
        case UNDOUIELEMENT_CREATE_ELEMENT:
            delete_ui_element(el->menu, el->uID);
            undo->structure.special = UNDOUIELEMENT_DELETE_ELEMENT;
            break;
        case UNDOUIELEMENT_DELETE_ELEMENT:
            memcpy(el, undo->bytes, sizeof(UIElement));
            undo->structure.special = UNDOUIELEMENT_CREATE_ELEMENT;
            break;
        case UNDOUIELEMENT_CHANGE_CONTAINER:
            
            if (undo->structure.ids[3] != -1)
            {
                remove_element_from_container(&cs[undo->structure.ids[3]], el);
            }
            if (undo->structure.ids[2] != -1)
            {
                assign_element_to_container(el, &cs[undo->structure.ids[2]]);
            }
            int s0 = undo->structure.ids[2];
            int s1 = undo->structure.ids[3];
            undo->structure.ids[2] = s1;
            undo->structure.ids[3] = s0;
            
            break;
        case UNDOUIELEMENT_REMOVE_FROM_CONTAINER:
            assign_element_to_container(el, &cs[undo->structure.ids[2]]);
            undo->structure.special = REDOUIELEMENT_ADD_TO_CONTAINER;
            break;
        case REDOUIELEMENT_ADD_TO_CONTAINER:
            remove_element_from_container(&cs[undo->structure.ids[2]], el);
            undo->structure.special = UNDOUIELEMENT_REMOVE_FROM_CONTAINER;
            break;
        case UNDOUIELEMENT_CREATE_CONTAINER:

            
            uint8_t ** bytes = malloc(sizeof(uint8_t*));
            size_t byte_size = sizeof(uint8_t*) + sizeof(UIContainer);

            bytes[0] = malloc(sizeof(UIContainer));
            UIContainer * container = &cs[el->container];
            memcpy(bytes[0], container, sizeof(UIContainer));
            
            temp[0] = undo->structure.ids[0];
            temp[1] = undo->structure.ids[1];
            temp[2] = el->container;
            temp[3] = undo->structure.ids[2];

            delete_ui_container(el);
            if (undo->special_bytes[0])
            {
                UIContainer * c = &cs[undo->structure.ids[2]];
                memcpy(c, undo->special_bytes[0], sizeof(UIContainer));
            }

            clear_special_undo_bytes(undo);
            undo->special_bytes = bytes;
            undo->byte_size = byte_size;
            undo->special_count = 1;
            memcpy(undo->structure.ids, temp, sizeof(int) * 8);

            undo->structure.special = UNDOUIELEMENT_DELETE_CONTAINER;
            
            break;
        case UNDOUIELEMENT_DELETE_CONTAINER:
            {
                
                UIContainer * c = &cs[undo->structure.ids[2]];
                memcpy(c, undo->special_bytes[0], sizeof(UIContainer));
                reassign_elements_to_container(el);

                temp[0] = undo->structure.ids[0];
                temp[1] = undo->structure.ids[1];
                temp[2] = el->container;
                temp[3] = undo->structure.ids[3];
                uint8_t ** bytes = NULL;
                size_t byte_size = 0;
                int special_count = 0;
                if (undo->structure.ids[3] != -1)
                {

                    UIContainer * del_container = &cs[undo->structure.ids[3]];
                    bytes = malloc(sizeof(uint8_t*));
                    bytes[0] = malloc(sizeof(UIContainer));
                    memcpy(bytes[0], del_container, sizeof(UIContainer));

                    byte_size += sizeof(uint8_t*);
                    byte_size += sizeof(UIContainer);
                    special_count = 1;

                    delete_ui_container(&menu->elements[del_container->element]);
                }

                clear_special_undo_bytes(undo);

                undo->special_bytes = bytes;
                undo->byte_size = byte_size;
                undo->special_count = special_count;
                undo->structure.special = UNDOUIELEMENT_CREATE_CONTAINER;
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            
            }
            break;
        case UNDOUIELEMENT_CREATE_LABEL:
            {
                if (undo->structure.ids[2] != -1)
                {
                    TextBox * tb = &menu->text_boxes[undo->structure.ids[2]];
                    memcpy(tb, undo->special_bytes[0], sizeof(TextBox));
                    tb->text = malloc(sizeof(char) * tb->text_cap);
                    memcpy(tb->text, undo->special_bytes[1], sizeof(char) * tb->text_cap);
                }

                TextBox * tb = &menu->text_boxes[undo->structure.ids[3]];
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                bytes[0] = malloc(sizeof(TextBox));
                bytes[1] = malloc(sizeof(char) * tb->text_cap);
                memcpy(bytes[0], tb, sizeof(TextBox));
                memcpy(bytes[1], tb->text, sizeof(char) * tb->text_cap);
                size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(TextBox) + sizeof(char) * tb->text_cap;
            
                delete_text_box(menu->uID, undo->structure.ids[3]);
                el->as.label.box = undo->structure.ids[2];
                add_all_text_boxes_to_draw_buffer();

                clear_special_undo_bytes(undo);
                temp[0] = undo->structure.ids[0];
                temp[1] = undo->structure.ids[1];
                temp[2] = undo->structure.ids[3];
                temp[3] = undo->structure.ids[2];

                undo->special_bytes = bytes;
                undo->byte_size = byte_size;
                undo->special_count = 2;
                
                memcpy(undo->structure.ids, temp, sizeof(int) * 8);
                
            }

            
            break;
        case UNDOUIELEMENT_JUSTIFY_LABEL:
            {
                
                int s0 = el->as.label.align;
                el->as.label.align = undo->structure.ids[2];

                align_text_box_label(el);
                undo->structure.ids[2] = s0;
            }
            break;
        default:
            break;
    }

}

void execute_ui_container_undo(Undo * undo)
{

    UIMenu * menus = CAST_VEC(MENUS, UIMenu);
    UIMenu * menu = &menus[undo->structure.ids[1]];
    
    UIContainer * cs = menu->containers;
    UIContainer * c = &cs[undo->structure.ids[2]];

    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)c);
            break;
        case UNDOUICONTAINER_CHANGE_SCROLLBAR:
            if (undo->structure.ids[3] != -1)
            {
                UIElement * old_scrollbar = &menu->elements[undo->structure.ids[3]];
                old_scrollbar->as.slider.container = undo->structure.ids[2];
            }
            if (undo->structure.ids[4] != -1)
            {
                
                UIElement * new_element = &menu->elements[undo->structure.ids[4]];
                new_element->as.slider.container = undo->structure.ids[5];
            }
            if (undo->structure.ids[2] != -1)
            {
                c->scrollbar = undo->structure.ids[3];
            }
            if (undo->structure.ids[5] != -1)
            {
                cs[undo->structure.ids[5]].scrollbar = undo->structure.ids[4];
            }

            temp[0] = undo->structure.ids[0];
            temp[1] = undo->structure.ids[1];
            temp[2] = undo->structure.ids[2];
            temp[3] = undo->structure.ids[4];
            temp[4] = undo->structure.ids[3];
            temp[5] = undo->structure.ids[6];
            temp[6] = undo->structure.ids[5];

            memcpy(undo->structure.ids, temp, sizeof(int) * 8);
            
            break;
        default:
            break;
    }
}

void execute_text_box_undo(Undo * undo)
{

    UIMenu * menus = CAST_VEC(MENUS, UIMenu);
    UIMenu * menu = &menus[undo->structure.ids[1]];
    
    TextBox * tbs = menu->text_boxes;
    TextBox * tb = &tbs[undo->structure.ids[0]];
    UIContainer * cs = menu->containers;
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)tb);
            break;
        case UNDOTEXTBOX_CREATE_TEXT_BOX:
            
            {
                
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                bytes[0] = malloc(sizeof(TextBox));
                bytes[1] = malloc(sizeof(char) * tb->text_cap);

                memcpy(bytes[0], tb, sizeof(TextBox));
                memcpy(bytes[1], tb->text, sizeof(char) * tb->text_cap);
                size_t byte_size = sizeof(TextBox) + sizeof(uint8_t*) * 2 + sizeof(char) * tb->text_cap;

            
                delete_text_box(undo->structure.ids[1], undo->structure.ids[0]);

                int s0 = EDITOR_SETTINGS.selected_text_id;
                EDITOR_SETTINGS.selected_text_id = undo->structure.ids[2];


                undo->structure.special = UNDOTEXTBOX_DELETE_TEXT_BOX;

                undo->special_bytes = bytes;
                undo->special_count = 2;
                undo->byte_size = byte_size;

                
                
            }
            
            break;
        case UNDOTEXTBOX_DELETE_TEXT_BOX:

            if (tb->text) free(tb->text);
            memcpy(tb, undo->special_bytes[0], sizeof(TextBox));
            tb->text = malloc(sizeof(char) * tb->text_cap);
            memcpy(tb->text, undo->special_bytes[1], sizeof(char) * tb->text_cap);
            EDITOR_SETTINGS.selected_text_id = undo->structure.ids[0];
            clear_special_undo_bytes(undo);

            undo->structure.special = UNDOTEXTBOX_CREATE_TEXT_BOX;

            break;
        case UNDOTEXTBOX_CHANGE_TEXT:

            {

                uint8_t ** bytes = NULL;
                size_t byte_size = 0;
                int byte_count = 0;

                int text_cap = tb->text_cap;
                if (tb->text)
                {
                    bytes = malloc(sizeof(uint8_t*));
                    bytes[0] = malloc(sizeof(char) * tb->text_cap);
                    byte_size += sizeof(uint8_t*) + sizeof(char) * tb->text_cap;
                    byte_count = 1;
                    memcpy(bytes[0], tb->text, sizeof(char) * tb->text_cap);
                    
                    free(tb->text);
                    tb->text_cap = 0;
                    tb->text = NULL;
                }
                if (undo->special_count > 0)
                {
                    tb->text_cap = undo->structure.ids[2];
                    tb->text = malloc(sizeof(char) * tb->text_cap);
                    memcpy(tb->text, undo->special_bytes[0], sizeof(char) * tb->text_cap);
                }

                clear_special_undo_bytes(undo);
                undo->special_bytes = bytes;
                undo->special_count = byte_count;
                undo->byte_size = byte_size;
                undo->structure.ids[2] = text_cap;

                add_all_text_boxes_to_draw_buffer();
            }
            
            break;
        case UNDOTEXTBOX_CHANGE_CONTAINER:
            if (undo->structure.ids[2] != -1)
            {
                remove_text_box_from_container(&cs[undo->structure.ids[2]], tb);
            }
            if (undo->structure.ids[3] != -1)
            {
                assign_text_box_to_container(tb, &cs[undo->structure.ids[3]]);
            }
            tb->parent_container = undo->structure.ids[3];
            int s0 = undo->structure.ids[2];
            int s1 = undo->structure.ids[3];
            undo->structure.ids[2] = s1;
            undo->structure.ids[3] = s0;
            
            break;

        case UNDOTEXTBOX_REMOVE_FROM_CONTAINER:
            assign_text_box_to_container(tb, &cs[undo->structure.ids[2]]);
            break;
        case UNDOTEXTBOX_ADD_TO_CONTAINER:
            remove_text_box_from_container(&cs[undo->structure.ids[2]], tb);
            break;

        default:
            break;

        
    }

    
}

void execute_asset_instance_undo(Undo * undo)
{
    
    AssetInstance * AI = &ASSET_INSTANCES[undo->structure.ids[0]];
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)AI);
            break;
        case UNDOASSETINSTANCE_ROTATION_ANGLE:
            {
                
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * 3);
                bytes[0] = malloc(sizeof(float));
                bytes[1] = malloc(sizeof(mat4));
                bytes[2] = malloc(sizeof(vec3));
                memcpy(bytes[0], &EDITOR_SETTINGS.rotation_angle, sizeof(float));
                memcpy(bytes[1], &AI->model, sizeof(mat4));
                memcpy(bytes[2], &AI->rot, sizeof(vec3));

                memcpy(&EDITOR_SETTINGS.rotation_angle, undo->special_bytes[0], sizeof(float));
                memcpy(&AI->model, undo->special_bytes[1], sizeof(mat4));
                memcpy(&AI->rot, undo->special_bytes[2], sizeof(vec3));

                free(undo->special_bytes[0]);
                free(undo->special_bytes[1]);
                free(undo->special_bytes[2]);
                free(undo->special_bytes);
                undo->special_bytes = bytes;
                AI->model = compose_trs_from_vectors(AI->tr, AI->rot, AI->scale);
                
            }
            break;
        case UNDOASSETINSTANCE_MAT:
            execute_non_special_undo(undo, (uint8_t*)AI);
            AI->model = compose_trs_from_vectors(AI->tr, AI->rot, AI->scale);
            break;
        default:
            break;
    }

}

void execute_mesh_node_undo(Undo * undo)
{
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    MeshNode * mn = &mns[undo->structure.ids[0]];
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    int temp[8];
    switch(undo->structure.special)
    {
        case UNDOSPECIAL_NONE:
            execute_non_special_undo(undo, (uint8_t*)mn);
            break;
        case UNDOMN_ROTATION_ANGLE:
            {
                
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * 3);
                bytes[0] = malloc(sizeof(float));
                bytes[1] = malloc(sizeof(mat4));
                bytes[2] = malloc(sizeof(vec3));
                memcpy(bytes[0], &EDITOR_SETTINGS.rotation_angle, sizeof(float));
                memcpy(bytes[1], &mn->global_matrix, sizeof(mat4));
                memcpy(bytes[2], &mn->rotation, sizeof(vec3));

                memcpy(&EDITOR_SETTINGS.rotation_angle, undo->special_bytes[0], sizeof(float));
                memcpy(&mn->global_matrix, undo->special_bytes[1], sizeof(mat4));
                memcpy(&mn->rotation, undo->special_bytes[2], sizeof(vec3));

                free(undo->special_bytes[0]);
                free(undo->special_bytes[1]);
                free(undo->special_bytes[2]);
                free(undo->special_bytes);
                undo->special_bytes = bytes;
                
            }
            break;
        case UNDOMN_MAT:
            execute_non_special_undo(undo, (uint8_t*)mn);
            mn->global_matrix = compose_trs_from_mesh_node(mn);
            break;
        case UNDOMN_CHANGE_MATERIAL:
            {
                Mesh * m = &meshes[mn->mesh];

                SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);
                SubMesh * sm = &sms[undo->structure.ids[1]];
                int temp = sm->material;
                sm->material = undo->structure.ids[2];
                undo->structure.ids[2] = temp;

                
                draw_dirty = true;
            }
            
            break;
        default:
            break;
    }

}

void execute_undo(Undo * undo)
{

    switch(undo->structure.type)
    {
        case UNDOSTRUCT_MENU:
            execute_ui_menu_undo(undo);
            break;
        case UNDOSTRUCT_UI_ELEMENT:
            execute_ui_element_undo(undo);
            break;
        case UNDOSTRUCT_TEXTBOX:
            execute_text_box_undo(undo);
            break;
        case UNDOSTRUCT_UI_CONTAINER:
            execute_ui_container_undo(undo);
            break;
        case UNDOSTRUCT_MATERIAL:
            break;
        case UNDOSTRUCT_ENTITY:
            break;
        case UNDOSTRUCT_MN:
            execute_mesh_node_undo(undo);
            break;
        case UNDOSTRUCT_EDITOR_SETTINGS:
            execute_editor_settings_undo(undo);
            break;
        case UNDOSTRUCT_GLOBAL:
            execute_globals_undo(undo);
            break;
        case UNDOSTRUCT_LIGHT:
            execute_light_undo(undo);
            break;
        case UNDOSTRUCT_RPROBE:
            execute_rprobe_undo(undo);
            break;
        case UNDOSTRUCT_LPROBE:
            execute_lprobe_undo(undo);
            break;
        case UNDOSTRUCT_APROBE:
            execute_aprobe_undo(undo);
            break;
        case UNDOSTRUCT_APORTAL:
            execute_aportal_undo(undo);
            break;
        case UNDOSTRUCT_AEMITTER:
            execute_aemitter_undo(undo);
            break;
        case UNDOSTRUCT_ASSETINSTANCE:
            execute_asset_instance_undo(undo);
            break;
        default:
            break;
    }
    
    
}



void store_into_nkslider(NKSlider * s, void * bytes, size_t byte_size)
{

    memcpy(s->bytes, bytes, byte_size);
    s->byte_size = byte_size;
    
}



static const char QUERY_NAMES[PQUERY_MAX][256]
=
{
    "Cull",
    "Point Light Shadows",
    "Shadowmap",
    "GBuffer",
    "Depth of Field",
    "SSAO",
    "TAA",
    "SSR",
    "Bloom",
    "Render",
    "Postprocessing",
};

void toggle_performance_overlay()
{
    
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[PERFORMANCE_OVERLAY_MENU];
    TextBox * boxes = m->text_boxes;
    int query = 0;

    double full_time = 0.0f;
    for (int i = PERFORMANCE_OVERLAY_TEXT_BOX_START; i < PERFORMANCE_OVERLAY_TEXT_BOX_START + PERFORMANCE_OVERLAY_TEXT_BOX_COUNT; i++)
    {
        TextBox * box = &boxes[i];


        box->visible = !box->visible;
    }
}


void update_performance_overlay(GLuint64 query_s[PQUERY_MAX])
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    UIMenu * m = &ms[PERFORMANCE_OVERLAY_MENU];
    TextBox * boxes = m->text_boxes;
    int query = 0;
    TextBox * box = &boxes[PERFORMANCE_OVERLAY_TEXT_BOX_START];

    sprintf(box->text, "Performance Overlay"); 

    move_text_box_to_origin(box);

    double full_time = 0.0f;
    for (int i = PERFORMANCE_OVERLAY_TEXT_BOX_START+1; i < PERFORMANCE_OVERLAY_TEXT_BOX_START + (PERFORMANCE_OVERLAY_TEXT_BOX_COUNT - 1); i++)
    {

        TextBox * box = &boxes[i];

        double time = query_s[query] / 1e6;
        full_time += time;
        sprintf(box->text, "%s: %.3f", QUERY_NAMES[query], time); 
        move_text_box_to_origin(box);
        

        query++;
    }
    TextBox * end = &boxes[PERFORMANCE_OVERLAY_TEXT_BOX_START + PERFORMANCE_OVERLAY_TEXT_BOX_COUNT-1];

    sprintf(end->text, "Frame Time: %.3f (%.3f FPS)", full_time, 1000.0f/full_time); 
    move_text_box_to_origin(end);

    

    add_all_text_boxes_to_draw_buffer();
    
}


void create_performance_overlay()
{
    
    vec2 translation = (vec2){0,0};
    PERFORMANCE_OVERLAY_TEXT_BOX_START = 0;

    UIMenu * menu = create_menu();
    PERFORMANCE_OVERLAY_MENU = menu->uID;

    TextBox * boxes = menu->text_boxes;
    
    for (int i = 0; i < PERFORMANCE_OVERLAY_TEXT_BOX_COUNT; i++)
    {
        
        TextBox * tb_title = &boxes[i];

        *tb_title = DEFAULT_TEXT_BOX;
        tb_title->uID = i;
        tb_title->translation = translation;
        tb_title->text = malloc(256 * sizeof(char));
        tb_title->text_cap = 256;
        tb_title->text[0] = '\0';
        tb_title->justify = JUSTIFY_LEFT;
        tb_title->menu = menu->uID;
        tb_title->visible = true;
        tb_title->free = false;
        
        move_text_box_to_origin(tb_title);
    
        translation.y += 40;

    }
    

    
}

// a is src, b is the potentially modified value
bool nk_check_equivalency(void * a, void * b, size_t byte_size, NKSlider * s, bool * dirty)
{
    bool push_undo = false;
    // if a != b, we are sliding
    if (memcmp(a, b, byte_size) != 0)
    {
        // if we were not yet sliding, store the value
        if (!s->is_held)
        {
            s->byte_size = byte_size;
            memcpy(s->bytes, a, byte_size);
        }
        // now we are sliding
        s->is_held = true;
        // if we need to store dirty state
        if (dirty != NULL)
        {
            *dirty = true;
        }
        // change a to b
        memcpy(a, b, byte_size);
        // if we have a new action, the redo history is reset
        destroy_redo_history();
    }
    else
    {
        // if we were held and now we have stopped, signal to push the undo
        if (s->is_held)
        {
            push_undo = true;
        }
        s->is_held = false;
    }
    return push_undo;
}

bool check_equivalencyv3(vec3 * a, vec3 b, NKSlider * s, bool * dirty)
{
    
    bool push_undo = false;
    if (!vec3_equal(*a, b, 1e-6))
    {
        if (!s->is_held)
        {
            s->byte_size = sizeof(vec3);
            memcpy(s->bytes, a, sizeof(vec3));
        }
        s->is_held = true;
        if (dirty != NULL)
        {
            *dirty = true;
        }
        *a = b;
        destroy_redo_history();
    }
    else
    {
        if (s->is_held)
        {
            // push onto undo stack
            push_undo = true;
        }
        s->is_held = false;
    }
    return push_undo;
}
bool check_equivalencyv4(vec4 * a, vec4 b, NKSlider * s, bool * dirty)
{
    
    bool push_undo = false;
    if (!vec4_equal(*a, b, 1e-6))
    {
        if (!s->is_held)
        {
            s->byte_size = sizeof(vec4);
            memcpy(s->bytes, a, sizeof(vec4));
            push_undo = true;
        }
        s->is_held = true;
        if (dirty != NULL)
        {
            *dirty = true;
        }
        *a = b;
        destroy_redo_history();
    }
    else
    {
        s->is_held = false;
    }
    return push_undo;
}

bool check_equivalencyf(float * a, float b, NKSlider * s, bool * dirty)
{
    bool push_undo = false;
    if (*a != b)
    {
        if (!s->is_held)
        {
            s->byte_size = sizeof(float);
            memcpy(s->bytes, a, sizeof(float));
            push_undo = true;
        }
        s->is_held = true;
        if (dirty != NULL)
        {
            *dirty = true;
        }
        *a = b;
        destroy_redo_history();
    }
    else
    {
        s->is_held = false;
    }
    return push_undo;
}
bool check_equivalencyi(int * a, int b, NKSlider * s, bool * dirty)
{
    bool push_undo = false;
    if (*a != b)
    {
        if (!s->is_held)
        {
            s->byte_size = sizeof(int);
            memcpy(s->bytes, a, sizeof(int));
            push_undo = true;
        }
        s->is_held = true;
        if (dirty != NULL)
        {
            *dirty = true;
        }
        *a = b;
        destroy_redo_history();
    }
    else
    {
        s->is_held = false;
    }

    return push_undo;
}


void FILE_DIR_REGISTRY_INIT()
{

    init_vector(&scene_registry.names, 0, sizeof(char) * 256);
    scene_registry.search_suffix = false;
    sprintf(scene_registry.directory, "./scenes/");
    scene_registry.current_selection = 0;
    scene_registry.open = nk_false;

    init_vector(&lightmap_registry.names, 0, sizeof(char) * 256);
    lightmap_registry.search_suffix = false;
    sprintf(lightmap_registry.directory, "./lightmaps/");
    lightmap_registry.current_selection = 0;
    lightmap_registry.open = nk_false;

    init_vector(&light_registry.names, 0, sizeof(char) * 256);
    light_registry.search_suffix = false;
    sprintf(light_registry.directory, "./lights/");
    light_registry.current_selection = 0;
    light_registry.open = nk_false;

    init_vector(&sound_registry.names, 0, sizeof(char) * 256);
    sound_registry.search_suffix = false;
    sprintf(sound_registry.directory, "./sounds/");
    sound_registry.current_selection = 0;
    sound_registry.open = nk_false;

    init_vector(&aportal_registry.names, 0, sizeof(char) * 256);
    aportal_registry.search_suffix = false;
    sprintf(aportal_registry.directory, "./aportals/");
    aportal_registry.current_selection = 0;
    aportal_registry.open = nk_false;

    init_vector(&rprobe_registry.names, 0, sizeof(char) * 256);
    rprobe_registry.search_suffix = false;
    sprintf(rprobe_registry.directory, "./rprobes/");
    rprobe_registry.current_selection = 0;
    rprobe_registry.open = nk_false;
    
    init_vector(&lprobe_registry.names, 0, sizeof(char) * 256);
    lprobe_registry.search_suffix = false;
    sprintf(lprobe_registry.directory, "./lprobes/");
    lprobe_registry.current_selection = 0;
    lprobe_registry.open = nk_false;

    init_vector(&aprobe_registry.names, 0, sizeof(char) * 256);
    aprobe_registry.search_suffix = false;
    sprintf(aprobe_registry.directory, "./aprobes/");
    aprobe_registry.current_selection = 0;
    aprobe_registry.open = nk_false;

    init_vector(&scene_apv_registry.names, 0, sizeof(char) * 256);
    scene_apv_registry.search_suffix = false;
    sprintf(scene_apv_registry.directory, "./scene_apvs/");
    scene_apv_registry.current_selection = 0;
    scene_apv_registry.open = nk_false;
    
    init_vector(&scene_lpv_registry.names, 0, sizeof(char) * 256);
    scene_lpv_registry.search_suffix = false;
    sprintf(scene_lpv_registry.directory, "./scene_lpvs/");
    scene_lpv_registry.current_selection = 0;
    scene_lpv_registry.open = nk_false;

    init_vector(&ui_registry.names, 0, sizeof(char) * 256);
    ui_registry.search_suffix = false;
    sprintf(ui_registry.directory, "./ui/");
    ui_registry.current_selection = 0;
    ui_registry.open = nk_false;
}


void load_filenames(FileDirectoryRegistry * registry)
{
    WIN32_FIND_DATAA find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", registry->directory);

    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;

    registry->names.count = 0;

    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {

            if (registry->search_suffix)
            {
                // may have no null terminator
                if (strstr(find_data.cFileName, registry->suffix))
                {
                    char * name = vector_expand(&registry->names, 1);
                    strncpy(name, find_data.cFileName, 254);
                    name[255] = '\0';
                }
            }
            else
            {
                char * name = vector_expand(&registry->names, 1);
                strncpy(name, find_data.cFileName, 254);
                name[255] = '\0';
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
    
}

void load_rprobe_filenames(const char *folder_path) {

    WIN32_FIND_DATAA find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);

    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;

    rprobe_names_count = 0;

    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            strncpy(rprobe_names[rprobe_names_count], find_data.cFileName, 255);
            rprobe_names_count++;
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
}

void load_gltf_filenames(const char *folder_path) {

    WIN32_FIND_DATAA find_data;
    char search_path[MAX_PATH];
    snprintf(search_path, sizeof(search_path), "%s\\*", folder_path);

    HANDLE hFind = FindFirstFileA(search_path, &find_data);
    if (hFind == INVALID_HANDLE_VALUE) return;

    gltf_names_count = 0;

    do {
        if (!(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            if (strstr(find_data.cFileName, "gltf"))
            {
                strncpy(gltf_names[gltf_names_count], find_data.cFileName, 254);
                gltf_names[gltf_names_count][255] = '\0';
                gltf_names_count++;
            }
        }
    } while (FindNextFileA(hFind, &find_data));

    FindClose(hFind);
}







nk_bool gltf_file_names_open = nk_false;
nk_bool rprobe_file_names_open = nk_false;

const char target_types[4][256] =
{
    "FLT",
    "V3",
    "INT",
    "BOOL"
};
void render_entity_menu()
{

    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_ENT],
            nk_rect(420, SCREEN_HEIGHT - 320, SCREEN_WIDTH - 420 * 2, 300),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }
        Entity * ent = &ENTITIES[EDITOR_SETTINGS.selected_ent_id];
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        PhysicsComponent * pc = NULL;
        AnimationComponent * ac = NULL;


        nk_layout_row_dynamic(NK_CTX, 20, 1);
        char sk[256];
        sprintf(sk, "Skin: %d", ent->skin);
        nk_label(NK_CTX, sk, NK_TEXT_ALIGN_LEFT);

        if (nk_button_label(NK_CTX, "Player Interact"))
        {

            if (ent->state)
            {
                ent->state->fn[ESTATE_PLAYER_INTERACT](ent);
            }
            
        }
                            

        if (ent->pc)
        {
            pc = ent->pc;
            
        }
        
        // possibly aabb controls

        if (ent->ac)
        {
            ac = ent->ac;
        }
                
        if (pc)
        {
            if (!pc->free)
            {

                nk_layout_row_dynamic(NK_CTX, 20, 1);
                nk_label(NK_CTX, "PC Menu", NK_TEXT_ALIGN_LEFT);

                bool pc_dirty = false;

                nk_layout_row_dynamic(NK_CTX, 20, 3);
                float mass = nk_propertyf(NK_CTX, "Mass", 0.0f, pc->mass, 10000.0f, 0.01f, 0.01f);
                if (mass != pc->mass)
                {
                    pc_dirty = true;
                    pc->mass = mass;
                }
                float restitution = nk_propertyf(NK_CTX, "Restitution", 0.0f, pc->restitution, 20.0f, 0.005f, 0.005f);
                if (restitution != pc->restitution)
                {
                    pc->restitution= restitution;
                }
                float mu = nk_propertyf(NK_CTX, "Mu", 0, pc->mu, 20.0f, 0.005f, 0.005f);
                if (mu != pc->mu)
                {
                    pc->mu= mu;
                }
                if (pc_dirty)
                {
                    pc->inverse_inertia = calc_inverse_inertia(pc);
                }

                if (nk_button_label(NK_CTX, "Delete PC"))
                {
                    pc->free = true;
                }
                
            }
            
        }
        else
        {
            
            if (nk_button_label(NK_CTX, "Create PC"))
            {

                create_physics_component(ent, COL_HULL, PHYSICS_FULL);
            }
        }

        if (ac)
        {
            if (!ac->free)
            {

                nk_layout_row_dynamic(NK_CTX, 20, 1);
                nk_label(NK_CTX, "AC Menu", NK_TEXT_ALIGN_LEFT);
                char tracks[256];
                sprintf(tracks, "Tracks: %d", ac->track_count);
                nk_label(NK_CTX, tracks, NK_TEXT_ALIGN_LEFT);

                for (int i = 0; i < ac->track_count; i++)
                {
                    AnimationTrack * t = &ac->tracks[i];
                    char keyframe[256];
                    sprintf(keyframe, "Keyframes t%d: %d", i, t->kf_count);
                    nk_label(NK_CTX, keyframe, NK_TEXT_ALIGN_LEFT);

                    
                    for (int j = 0; j < t->kf_count; j++)
                    {
                        AnimationKeyframe * kf = &t->kf[j];
                        char time[256];
                        sprintf(time, "Time k%d", j); 
                        float new_time = nk_propertyf(NK_CTX, time, 0.01f, kf->time, 10000, 0.01f, 0.01f);
                        if (new_time != kf->time)
                        {
                            kf->time = new_time;
                        }


                        switch(t->type)
                        {
                            case ANIM_TARGET_FLOAT:

                                char tg[256];
                                sprintf(tg, "Flt t%d k%d", i, j); 
                                float new_val = nk_propertyf(NK_CTX, tg, -10000.0f, kf->val.v, 10000, 0.01f, 0.01f);
                                if (new_val != kf->val.v)
                                {
                                    kf->val.v = new_val;
                                }
                                
                                break;
                            case ANIM_TARGET_VEC3:
                                char tgx[256];
                                sprintf(tgx, "x t%d k%d", i, j); 
                                float new_x = nk_propertyf(NK_CTX, tgx, -10000.0f, kf->val.v3.x, 10000, 0.01f, 0.01f);
                                if (new_x != kf->val.v3.x)
                                {
                                    kf->val.v3.x = new_x;
                                }
                                char tgy[256];
                                sprintf(tgy, "y t%d k%d", i, j); 
                                float new_y = nk_propertyf(NK_CTX, tgy, -10000.0f, kf->val.v3.y, 10000, 0.01f, 0.01f);
                                if (new_y != kf->val.v3.y)
                                {
                                    kf->val.v3.y = new_y;
                                }
                                char tgz[256];
                                sprintf(tgz, "z t%d k%d", i, j); 
                                float new_z = nk_propertyf(NK_CTX, tgz, -10000.0f, kf->val.v3.z, 10000, 0.01f, 0.01f);
                                if (new_z != kf->val.v3.z)
                                {
                                    kf->val.v3.z = new_z;
                                }
                                break;
                            case ANIM_TARGET_INT:
                                char it[256];
                                sprintf(it, "Int t%d k%d", i, j); 
                                int new_i = nk_propertyi(NK_CTX, it, -10000, kf->val.i, 10000, 1, 1);
                                if (new_i != kf->val.i)
                                {
                                    kf->val.i = new_i;
                                }
                                break;
                            case ANIM_TARGET_BOOL:
                                break;
                            default:
                                break;
                        }
                        
                    }
                    char dr[256];
                    sprintf(dr, "Duration t%d", i);
                    float new_duration = nk_propertyf(NK_CTX, dr, 0.01f, t->duration, 10000, 0.01f, 0.01f);
                    if (new_duration != t->duration)
                    {
                        t->duration = new_duration;
                    }

                    char lp[256];
                    sprintf(lp, "Loop t%d", i);
                    nk_bool looper = t->loop == 1;
                    if (nk_checkbox_label(NK_CTX, lp, &looper))
                    {
                        t->loop = looper;
                    }
                    char act[256];
                    sprintf(act, "Active t%d", i);
                    nk_bool acter = t->active == 1;
                    if (nk_checkbox_label(NK_CTX, act, &acter))
                    {
                        t->active = acter;
                    }

                    char target_type[256];
                    sprintf(target_type, "Type t%d: %s", i, target_types[t->type]);

                    if (nk_button_label(NK_CTX, target_type))
                    {
                        t->type = (t->type + 1) % ANIM_TARGET_MAX;
                    }


                    
                    char add[256];
                    sprintf(add, "Add t%d KF", i);
                    if (nk_button_label(NK_CTX, add))
                    {
                        t->kf_count++;
                    }
                    char rm[256];
                    sprintf(rm, "Remove Track t%d", i);
                    if (nk_button_label(NK_CTX, rm))
                    {

                        for (int j = i; j < ac->track_count-1; j++)
                        {
                            ac->tracks[j] = ac->tracks[j+1];
                            for (int k = 0; k < ac->tracks[j+1].kf_count; k++)
                            {
                                ac->tracks[j].kf[k] = ac->tracks[j+1].kf[k];
                            }
                        }
                        
                        ac->track_count--;
                    }
                    
                    
                }

                if (nk_button_label(NK_CTX, "Add Track"))
                {
                    ac->track_count++;
                }
                if (nk_button_label(NK_CTX, "Delete AC"))
                {
                    ac->free = true;
                }
                
                
            }
            

        }
        else
        {

            if (nk_button_label(NK_CTX, "Create AC"))
            {

                create_animation_component(ent);
            }
            
        }

        
        
    }
    nk_end(NK_CTX);
    
}




const char menu_names[][256]
=
{
    "Main",
    "Mesh Node",
    "Material",
    "Light Menu",
    "Entity Menu",
    "Text Menu",
    "UI Menu",
    "Audio Menu",
    "LProbe Menu",
    "AInst Menu"
};



void render_file_directory_dropdown(FileDirectoryRegistry * registry)
{
    
    ComboLabel label;
    label.item_count = 0;
    label.selected = 0;
    if (nk_combo_begin_label(NK_CTX, registry->directory, nk_vec2(200, 200))) {

        if (!registry->open)
        {
            load_filenames(registry);
        }
        char * names = (char *)registry->names.buffer;
        for (int i = 0; i < registry->names.count; i++)
        {
            snprintf(label.buffer[label.item_count], LABEL_LEN, "%s", &names[i * 256]);
            label.ptrs[label.item_count] = label.buffer[label.item_count];
            if (registry->current_selection == label.item_count) label.selected = label.item_count;
            label.item_count++;

        }
        registry->open = nk_true;
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        for (int i = 0; i < label.item_count; i++) {
            nk_bool sel = (i == label.selected);
            if (nk_selectable_label(NK_CTX, label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                registry->current_selection = i;
            }
        }
        nk_combo_end(NK_CTX);
    }
    else
    {
        registry->open = nk_false;
    }
}

void render_dropdown_labels(int * selected, char ** options, int option_count)
{
    
    ComboLabel label;
    label.item_count = 0;
    label.selected = 0;
    for (int i = 0; i < option_count; i++)
    {
        snprintf(label.buffer[label.item_count], LABEL_LEN, "%s", options[i]);
        label.ptrs[label.item_count] = label.buffer[label.item_count];
        if (*selected == label.item_count) label.selected = label.item_count;
        label.item_count++;

    }
    nk_layout_row_dynamic(NK_CTX, 20, 1);
    for (int i = 0; i < label.item_count; i++) {
        nk_bool sel = (i == label.selected);
        if (nk_selectable_label(NK_CTX, label.ptrs[i], NK_TEXT_LEFT, &sel)) {
            *selected = i;
        }
    }
}


void render_dropdown(const char * dropdown_name, int * selected, char ** options, int option_count, int x, int y)
{

    char name[512];
    if (*selected >= 0 && *selected < option_count)
    {
        strcpy(name, options[*selected]);
        
    }
    else
    {
        strcpy(name, dropdown_name);
    }
    
    if (nk_combo_begin_label(NK_CTX, name, nk_vec2(x, y))) {
        ComboLabel label;
        label.item_count = 0;
        label.selected = 0;
        for (int i = 0; i < option_count; i++)
        {
            snprintf(label.buffer[label.item_count], LABEL_LEN, "%s", options[i]);
            label.ptrs[label.item_count] = label.buffer[label.item_count];
            if (*selected == label.item_count) label.selected = label.item_count;
            label.item_count++;

        }
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        for (int i = 0; i < label.item_count; i++) {
            nk_bool sel = (i == label.selected);
            if (nk_selectable_label(NK_CTX, label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                *selected = i;
            }
        }
        nk_combo_end(NK_CTX);
    }
    
}

mat3 mat3_rotation_x(float angle_radians) {
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    return (mat3){
        1.0f,  0.0f,  0.0f,   
        0.0f, c,  -s,
       0.0f,  s,  c    
    };
}


mat3 mat3_rotation_y(float angle_radians) {
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    return (mat3){
        c,    0.0f,  s,   
        0.0f, 1.0f,  0.0f,
       -s,    0.0f,  c    
    };
}

mat3 mat3_rotation_z(float angle_radians) {
    float c = cosf(angle_radians);
    float s = sinf(angle_radians);
    return (mat3){
        c,   -s,  0.0f,   
        s, c,  0.0f,
       0.0f,    0.0f,  1.0f    
    };
}

void obb_orthonormalize(OBB *obb) {
    

    vec3 fwd = {obb->rot.a0, obb->rot.a1, obb->rot.a2};
    vec3 right = {obb->rot.b0, obb->rot.b1, obb->rot.b2};
    vec3 up = {obb->rot.c0, obb->rot.c1, obb->rot.c2};
    
    vec3 r = vec3_normalize(right);
    
    vec3 u = up;
    u = vec3_sub(u, vec3_scale(r, vec3_dot(u, r)));
    u = vec3_normalize(u);
    
    vec3 f = vec3_cross(r, u);
    
    obb->rot.a0 = f.x;
    obb->rot.a1 = f.y;
    obb->rot.a2 = f.z;
    obb->rot.b0 = r.x;
    obb->rot.b1 = r.y;
    obb->rot.b2 = r.z;
    obb->rot.c0 = u.x;
    obb->rot.c1 = u.y;
    obb->rot.c2 = u.z;

}
void render_ivec3_widget(const char * name, UndoStructureType undo_type, int id, ivec3 * vec, NKSlider * sliders)
{
    bool dirty = false;
    nk_layout_row_dynamic(NK_CTX, 20, 3);
    
    const char * pos_names[3] =
    {
        " x",
        " y",
        " z",
    };

    char final_names[3][256];

    for (int i = 0; i < 3; i++)
    {

        strcpy(final_names[i], name);
        strcat(final_names[i], pos_names[i]);
        
    }

    UndoSpecial special;
    size_t byte_offset = 0;
    switch(undo_type)
    {
        case UNDOSTRUCT_APROBE:
            special = UNDOSPECIAL_NONE;
            byte_offset = offsetof(ProbeVolume, resolution);
            break;
        default:
            break;
    }

    
    UndoStructure us = create_undo_structure(undo_type, (int[8]){id}, special);
    

    int vecx = nk_propertyi(NK_CTX, final_names[0], -100000, vec->x, 100000, 1, 1);
    if (check_equivalencyi(&vec->x, vecx, &sliders[0], NULL))
    {
        push_nk_slider_undo(&sliders[0], us, byte_offset + offsetof(ivec3, x));
    }

    
    float vecy = nk_propertyi(NK_CTX, final_names[1], -100000, vec->y, 100000, 1, 1);
    if (check_equivalencyi(&vec->y, vecy, &sliders[1], NULL))
    {
        push_nk_slider_undo(&sliders[1], us, byte_offset + offsetof(ivec3, y));
    }

    float vecz = nk_propertyi(NK_CTX, final_names[2], -100000, vec->z, 100000, 1, 1);
    if (check_equivalencyi(&vec->z, vecz, &sliders[2], NULL))
    {
        push_nk_slider_undo(&sliders[2], us, byte_offset + offsetof(ivec3, z));
    }


    
}

void render_pos_widget(const char * name, UndoStructureType undo_type, int id, vec3 * pos, NKSlider * sliders)
{
    bool dirty = false;
    nk_layout_row_dynamic(NK_CTX, 20, 3);
    
    const char * pos_names[3] =
    {
        " x",
        " y",
        " z",
    };

    char final_names[3][256];

    for (int i = 0; i < 3; i++)
    {

        strcpy(final_names[i], name);
        strcat(final_names[i], pos_names[i]);
        
    }

    UndoSpecial special;
    size_t byte_offset = 0;
    switch(undo_type)
    {
        case UNDOSTRUCT_AEMITTER:
            special = UNDOSPECIAL_NONE;
            byte_offset = offsetof(AudioEmitter, pos);
            break;
        default:
            break;
    }

    
    UndoStructure us = create_undo_structure(undo_type, (int[8]){id}, special);
    

    float posx = nk_propertyf(NK_CTX, final_names[0], -100000.0f, pos->x, 100000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&pos->x, posx, &sliders[0], NULL))
    {
        push_nk_slider_undo(&sliders[0], us, byte_offset + offsetof(vec3, x));
    }

    float posy = nk_propertyf(NK_CTX, final_names[1], -100000.0f, pos->y, 100000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&pos->y, posy, &sliders[1], NULL))
    {
        push_nk_slider_undo(&sliders[1], us, byte_offset + offsetof(vec3, y));
    }

    float posz = nk_propertyf(NK_CTX, final_names[2], -100000.0f, pos->z, 100000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&pos->z, posz, &sliders[2], NULL))
    {
        push_nk_slider_undo(&sliders[2], us, byte_offset + offsetof(vec3, z));
    }


    
}

void render_aabb_widget(const char * name, AABB * aabb)
{
    
    bool dirty = false;

    nk_layout_row_dynamic(NK_CTX, 20, 3);

    const char * aabb_names[6] =
    {
        "_aabb_x",
        "_aabb_y",
        "_aabb_z",
        "_aabbe_x",
        "_aabbe_y",
        "_aabbe_z"
    };

    char final_names[6][256];

    for (int i = 0; i < 6; i++)
    {

        strcpy(final_names[i], name);
        strcat(final_names[i], aabb_names[i]);
        
    }
    


    float aabbx = nk_propertyf(NK_CTX, final_names[0], -1000.0f, aabb->center.x, 1000.0f, 0.01f, 0.01f);
    if (aabbx != aabb->center.x)
    {
        dirty = true;
        aabb->center.x = aabbx;
    }
    float aabby = nk_propertyf(NK_CTX, final_names[1], -1000.0f, aabb->center.y, 1000.0f, 0.01f, 0.01f);
    if (aabby != aabb->center.y)
    {
        dirty = true;
        aabb->center.y = aabby;
    }
    float aabbz = nk_propertyf(NK_CTX, final_names[2], -1000.0f, aabb->center.z, 1000.0f, 0.01f, 0.01f);
    if (aabbz != aabb->center.z)
    {
        dirty = true;
        aabb->center.z = aabbz;
    }

    nk_layout_row_dynamic(NK_CTX, 20, 3);

    float aabbex = nk_propertyf(NK_CTX, final_names[3], -1000.0f, aabb->extents.x, 1000.0f, 0.01f, 0.01f);
    if (aabbex != aabb->extents.x)
    {
        dirty = true;
        aabb->extents.x = aabbex;
    }
    float aabbey = nk_propertyf(NK_CTX, final_names[4], -1000.0f, aabb->extents.y, 1000.0f, 0.01f, 0.01f);
    if (aabbey != aabb->extents.y)
    {
        dirty = true;
        aabb->extents.y = aabbey;
    }
    float aabbez = nk_propertyf(NK_CTX, final_names[5], -1000.0f, aabb->extents.z, 1000.0f, 0.01f, 0.01f);
    if (aabbez != aabb->extents.z)
    {
        dirty = true;
        aabb->extents.z = aabbez;
    }
    if (dirty)
    {
        calculate_aabb_bounds(aabb);
    }

    
}

void render_mat4_widget(const char * name, UndoStructureType undo_type, int id, mat4 * mat, vec3 * tr, vec4 * rot, vec3 * sc, NKSlider * sliders)
{
    
    UndoSpecial special;
    UndoSpecial mat_special;
    UndoSpecial rot_special;
    size_t pos_offset = 0;
    size_t scale_offset = 0;
    switch(undo_type)
    {
        case UNDOSTRUCT_ASSETINSTANCE:
            special = UNDOSPECIAL_NONE;
            mat_special = UNDOASSETINSTANCE_MAT;
            rot_special = UNDOASSETINSTANCE_ROTATION_ANGLE;
            pos_offset = offsetof(AssetInstance, tr);
            scale_offset = offsetof(AssetInstance, scale);
            break;
        case UNDOSTRUCT_MN:
            special = UNDOSPECIAL_NONE;
            mat_special = UNDOMN_MAT;
            rot_special = UNDOMN_ROTATION_ANGLE;
            pos_offset = offsetof(MeshNode, translation);
            scale_offset = offsetof(MeshNode, scale);
        default:
            break;
    }

    
    const char * obb_names[] =
    {
        "_mat_x",
        "_mat_y",
        "_mat_z",
        "_matr_x",
        "_matr_y",
        "_matr_z",
        "_maangle_z",
        "_matsc_x",
        "_matsc_y",
        "_matsc_z",
    };

    char final_names[10][256];

    for (int i = 0; i < 10; i++)
    {

        strcpy(final_names[i], name);
        strcat(final_names[i], obb_names[i]);
        
    }
    UndoStructure us = create_undo_structure(undo_type, (int[8]){id}, special);
    UndoStructure uss = create_undo_structure(undo_type, (int[8]){id}, mat_special);
    UndoStructure rot_us = create_undo_structure(undo_type, (int[8]){id}, rot_special);
    UndoStructure es = create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){0}, UNDOSPECIAL_NONE);

    bool dirty = false;
    vec3 pos;
    pos.x = nk_propertyf(NK_CTX, final_names[0], -1000.0f, tr->x, 1000.0f, 0.01f, 0.01f);

    pos.y = nk_propertyf(NK_CTX, final_names[1], -1000.0f, tr->y, 1000.0f, 0.01f, 0.01f);

    pos.z = nk_propertyf(NK_CTX, final_names[2], -1000.0f, tr->z, 1000.0f, 0.01f, 0.01f);

    if (nk_check_equivalency(tr, &pos, sizeof(vec3), &sliders[0], &dirty))
    {
        push_nk_slider_undo(&sliders[0], uss, pos_offset);
    }

    bool rot_dirty = false;
    vec3 r;
    r.x = nk_propertyf(NK_CTX, final_names[3], -1000.0f, EDITOR_SETTINGS.rotation_axis.x, 1000.0f, 0.01f, 0.01f);
    r.y = nk_propertyf(NK_CTX, final_names[4], -1000.0f, EDITOR_SETTINGS.rotation_axis.y, 1000.0f, 0.01f, 0.01f);
    r.z = nk_propertyf(NK_CTX, final_names[5], -1000.0f, EDITOR_SETTINGS.rotation_axis.z, 1000.0f, 0.01f, 0.01f);

    if (nk_check_equivalency(&EDITOR_SETTINGS.rotation_axis, &r, sizeof(vec3), &sliders[3], &rot_dirty))
    {
        push_nk_slider_undo(&sliders[3], es, offsetof(EditorSettings, rotation_axis));
    }
    
    if (rot_dirty)
    {
        EDITOR_SETTINGS.rotation_axis = vec3_normalize(EDITOR_SETTINGS.rotation_axis);
    }

    float angle = nk_propertyf(NK_CTX, final_names[6], 0, EDITOR_SETTINGS.rotation_angle, 2 * M_PI, 0.001f, 0.001f);
    bool angle_dirty = false;
    float rot_angle_old = EDITOR_SETTINGS.rotation_angle;
    if (check_equivalencyf(&EDITOR_SETTINGS.rotation_angle, angle, &sliders[4], &angle_dirty))
    {
        uint8_t ** bytes = malloc(sizeof(uint8_t*) * 3);
        bytes[0] = malloc(sizeof(float));
        bytes[1] = malloc(sizeof(mat4));
        bytes[2] = malloc(sizeof(vec3));
        memcpy(bytes[0], &rot_angle_old, sizeof(float));
        memcpy(bytes[1], mat, sizeof(mat4));
        memcpy(bytes[2], rot, sizeof(vec3));

        size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(float) + sizeof(mat4) + sizeof(vec3);
        
        push_special_undo(rot_us, bytes, byte_size, 3, 0);
    }
    if (angle_dirty)
    {
        
        EDITOR_SETTINGS.rotation_angle = angle;
        dirty = true;
        vec4 quat = quat_from_axis_angle(EDITOR_SETTINGS.rotation_axis, EDITOR_SETTINGS.rotation_angle);
        *rot = quat;
    }

    vec3 s;
    s.x = nk_propertyf(NK_CTX, final_names[7], -1000.0f, sc->x, 1000.0f, 0.01f, 0.01f);

    s.y = nk_propertyf(NK_CTX, final_names[8], -1000.0f, sc->y, 1000.0f, 0.01f, 0.01f);

    s.z = nk_propertyf(NK_CTX, final_names[9], -1000.0f, sc->z, 1000.0f, 0.01f, 0.01f);
    if (nk_check_equivalency(sc, &s, sizeof(vec3), &sliders[5], &dirty))
    {
        push_nk_slider_undo(&sliders[5], uss, scale_offset);
    }

    

    if (dirty)
    {
        mat4 out_mat = compose_trs_from_vectors(*tr, *rot, *sc);
        *mat = out_mat;

    }

    
}


void render_obb_widget(const char * name, UndoStructureType undo_type, int id, OBB * obb, NKSlider * sliders)
{
    
    bool dirty = false;

    nk_layout_row_dynamic(NK_CTX, 20, 3);

    const char * obb_names[] =
    {
        "_obb_x",
        "_obb_y",
        "_obb_z",
        "_obbe_x",
        "_obbe_y",
        "_obbe_z",
        "_obbr_x",
        "_obbr_y",
        "_obbr_z",
    };

    char final_names[9][256];

    for (int i = 0; i < 9; i++)
    {

        strcpy(final_names[i], name);
        strcat(final_names[i], obb_names[i]);
        
    }
    
    UndoSpecial special;
    UndoSpecial rot_special;
    switch(undo_type)
    {
        case UNDOSTRUCT_LPROBE:
            special = UNDOSPECIAL_NONE;
            rot_special = UNDOLPROBE_OBB_ROT;
            break;
        case UNDOSTRUCT_APROBE:
            special = UNDOSPECIAL_NONE;
            rot_special = UNDOAPROBE_OBB_ROT;
            break;
        default:
            break;
    }

    
    UndoStructure us = create_undo_structure(undo_type, (int[8]){id}, special);
    UndoStructure rot_us = create_undo_structure(undo_type, (int[8]){id}, rot_special);
    

    float obbx = nk_propertyf(NK_CTX, final_names[0], -1000.0f, obb->center.x, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->center.x, obbx, &sliders[0], NULL))
    {
        push_nk_slider_undo(&sliders[0], us, offsetof(ProbeVolume, obb.center.x));
    }
    float obby = nk_propertyf(NK_CTX, final_names[1], -1000.0f, obb->center.y, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->center.y, obby, &sliders[1], NULL))
    {
        push_nk_slider_undo(&sliders[1], us, offsetof(ProbeVolume, obb.center.y));
    }
    float obbz = nk_propertyf(NK_CTX, final_names[2], -1000.0f, obb->center.z, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->center.z, obbz, &sliders[2], NULL))
    {
        push_nk_slider_undo(&sliders[2], us, offsetof(ProbeVolume, obb.center.z));
    }

    nk_layout_row_dynamic(NK_CTX, 20, 3);

    float obbex = nk_propertyf(NK_CTX, final_names[3], -1000.0f, obb->extents.x, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->extents.x, obbex, &sliders[3], NULL))
    {
        push_nk_slider_undo(&sliders[3], us, offsetof(ProbeVolume, obb.extents.x));
    }
    float obbey = nk_propertyf(NK_CTX, final_names[4], -1000.0f, obb->extents.y, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->extents.y, obbey, &sliders[4], NULL))
    {
        push_nk_slider_undo(&sliders[4], us, offsetof(ProbeVolume, obb.extents.y));
    }
    float obbez = nk_propertyf(NK_CTX, final_names[5], -1000.0f, obb->extents.z, 1000.0f, 0.01f, 0.01f);
    if (check_equivalencyf(&obb->extents.z, obbez, &sliders[5], NULL))
    {
        push_nk_slider_undo(&sliders[5], us, offsetof(ProbeVolume, obb.extents.z));
    }
    bool a_dirty = false;
    bool b_dirty = false;
    bool c_dirty = false;

    float obbrx = nk_propertyf(NK_CTX, final_names[6], -M_PI, EDITOR_SETTINGS.obb_axis_x_angle, M_PI, 0.01f, 0.01f);
    bool s6_is_held = sliders[6].is_held;
    check_equivalencyf(&EDITOR_SETTINGS.obb_axis_x_angle, obbrx, &sliders[6], &a_dirty);
    if (!s6_is_held && sliders[6].is_held)
    {

        uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
        bytes[0] = malloc(sizeof(mat3));
        memcpy(bytes[0], &obb->rot, sizeof(mat3));
        bytes[1] = malloc(sizeof(vec3));
        vec3 axis = {EDITOR_SETTINGS.obb_axis_x_angle, EDITOR_SETTINGS.obb_axis_y_angle, EDITOR_SETTINGS.obb_axis_z_angle};
        memcpy(bytes[1], &axis, sizeof(vec3));
        size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(mat3) + sizeof(vec3);

        push_special_undo(rot_us, bytes, byte_size, 2, offsetof(ProbeVolume, obb.rot));

    }

    float obbry = nk_propertyf(NK_CTX, final_names[7], -M_PI, EDITOR_SETTINGS.obb_axis_y_angle, M_PI, 0.01f, 0.01f);
    bool s7_is_held = sliders[7].is_held;
    check_equivalencyf(&EDITOR_SETTINGS.obb_axis_y_angle, obbry, &sliders[7], &b_dirty);
    if (!s7_is_held && sliders[7].is_held)
    {

        uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
        bytes[0] = malloc(sizeof(mat3));
        memcpy(bytes[0], &obb->rot, sizeof(mat3));
        bytes[1] = malloc(sizeof(vec3));
        vec3 axis = {EDITOR_SETTINGS.obb_axis_x_angle, EDITOR_SETTINGS.obb_axis_y_angle, EDITOR_SETTINGS.obb_axis_z_angle};
        memcpy(bytes[1], &axis, sizeof(vec3));
        size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(mat3) + sizeof(vec3);

        push_special_undo(rot_us, bytes, byte_size, 2, offsetof(ProbeVolume, obb.rot));

    }
    float obbrz = nk_propertyf(NK_CTX, final_names[8], -M_PI, EDITOR_SETTINGS.obb_axis_z_angle, M_PI, 0.01f, 0.01f);

    bool s8_is_held = sliders[8].is_held;
    check_equivalencyf(&EDITOR_SETTINGS.obb_axis_z_angle, obbrz, &sliders[8], &c_dirty);

    if (!s8_is_held && sliders[8].is_held)
    {

        uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
        bytes[0] = malloc(sizeof(mat3));
        memcpy(bytes[0], &obb->rot, sizeof(mat3));
        bytes[1] = malloc(sizeof(vec3));
        vec3 axis = {EDITOR_SETTINGS.obb_axis_x_angle, EDITOR_SETTINGS.obb_axis_y_angle, EDITOR_SETTINGS.obb_axis_z_angle};
        memcpy(bytes[1], &axis, sizeof(vec3));
        size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(mat3) + sizeof(vec3);

        push_special_undo(rot_us, bytes, byte_size, 2, offsetof(ProbeVolume, obb.rot));

    }

    if (a_dirty || b_dirty || c_dirty)
    {


        obb->rot = mat3_identity();
        mat3 new_rot_x = mat3_rotation_x(EDITOR_SETTINGS.obb_axis_x_angle);
        obb->rot = mat3_mul(obb->rot, new_rot_x);
        mat3 new_rot_y = mat3_rotation_y(EDITOR_SETTINGS.obb_axis_y_angle);
        obb->rot = mat3_mul(obb->rot, new_rot_y);
        mat3 new_rot_z = mat3_rotation_z(EDITOR_SETTINGS.obb_axis_z_angle);
        obb->rot = mat3_mul(obb->rot, new_rot_z);

    }

    
}


int tb_id_flags = NK_EDIT_INACTIVE;
int tb_text_flags = NK_EDIT_INACTIVE;
int tb_cont_flags = NK_EDIT_INACTIVE;
int ui_menu_id_flags = NK_EDIT_INACTIVE;
int ui_id_flags = NK_EDIT_INACTIVE;
int ui_parent_cont_flags = NK_EDIT_INACTIVE;
int ui_cont_flags = NK_EDIT_INACTIVE;
int ui_scrollbar_flags = NK_EDIT_INACTIVE;
int ui_label_id_flags = NK_EDIT_INACTIVE;
int aportal_flags = NK_EDIT_INACTIVE;
int apv_flags = NK_EDIT_INACTIVE;
int scene_apv_flags = NK_EDIT_INACTIVE;
int lpv_flags = NK_EDIT_INACTIVE;
int scene_lpv_flags = NK_EDIT_INACTIVE;
int scene_name_flags = NK_EDIT_INACTIVE;
int scene_file_name_flags = NK_EDIT_INACTIVE;
int ui_name_flags = NK_EDIT_INACTIVE;
int rprobe_name_flags = NK_EDIT_INACTIVE;
int light_name_flags = NK_EDIT_INACTIVE;
int lm_name_flags = NK_EDIT_INACTIVE;




char aportal_name_buffer[256];
char light_name_buffer[256];
char aprobe_name_buffer[256];
char lprobe_name_buffer[256];
char scene_apv_name_buffer[256];
char scene_lpv_name_buffer[256];
void render_selection_menu()
{
    
    if
    (
        nk_begin
        (
            NK_CTX,
            "Selection Mode",
            nk_rect(20, SCREEN_HEIGHT - 80, 500, 80),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }
        nk_layout_row_dynamic(NK_CTX, 12, 6);
        if (nk_option_label(NK_CTX, "MN", EDITOR_SETTINGS.SELMODE == SELMODE_MN)) EDITOR_SETTINGS.SELMODE = SELMODE_MN;
        if (nk_option_label(NK_CTX, "LIGHT", EDITOR_SETTINGS.SELMODE == SELMODE_LIGHT)) EDITOR_SETTINGS.SELMODE = SELMODE_LIGHT;
        if (nk_option_label(NK_CTX, "LPV", EDITOR_SETTINGS.SELMODE == SELMODE_LPV)) EDITOR_SETTINGS.SELMODE = SELMODE_LPV;
        if (nk_option_label(NK_CTX, "RPROBE", EDITOR_SETTINGS.SELMODE == SELMODE_RPROBE)) EDITOR_SETTINGS.SELMODE = SELMODE_RPROBE;
        if (nk_option_label(NK_CTX, "ENTITY", EDITOR_SETTINGS.SELMODE == SELMODE_ENTITY)) EDITOR_SETTINGS.SELMODE = SELMODE_ENTITY;
        if (nk_option_label(NK_CTX, "TEXTBOX", EDITOR_SETTINGS.SELMODE == SELMODE_TEXTBOX)) EDITOR_SETTINGS.SELMODE = SELMODE_TEXTBOX;
        if (nk_option_label(NK_CTX, "UIELEMENT", EDITOR_SETTINGS.SELMODE == SELMODE_UIELEMENT)) EDITOR_SETTINGS.SELMODE = SELMODE_UIELEMENT;
        if (nk_option_label(NK_CTX, "APV", EDITOR_SETTINGS.SELMODE == SELMODE_APV)) EDITOR_SETTINGS.SELMODE = SELMODE_APV;
        if (nk_option_label(NK_CTX, "APORTAL", EDITOR_SETTINGS.SELMODE == SELMODE_APORTAL)) EDITOR_SETTINGS.SELMODE = SELMODE_APORTAL;
        if (nk_option_label(NK_CTX, "AEM", EDITOR_SETTINGS.SELMODE == SELMODE_AEM)) EDITOR_SETTINGS.SELMODE = SELMODE_AEM;
        if (nk_option_label(NK_CTX, "AINST", EDITOR_SETTINGS.SELMODE == SELMODE_ASSETINSTANCE)) EDITOR_SETTINGS.SELMODE = SELMODE_ASSETINSTANCE;
        

    }
    nk_end(NK_CTX);
}


char text_container_buffer[256];
char text_id_buffer[256];

void render_text_menu()
{

    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_TEXT],
            nk_rect(420, 20, SCREEN_WIDTH - 420 * 2, 300),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }

        UIMenu * ms = CAST_VEC(MENUS, UIMenu);
        UIMenu * m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];
        TextBox * tbs = m->text_boxes;
        TextBox * tb = &tbs[EDITOR_SETTINGS.selected_text_id];

        UndoStructure tb_structure = create_undo_structure
            (
                 UNDOSTRUCT_TEXTBOX,
                 (int[8]){
                     EDITOR_SETTINGS.selected_text_id,
                     EDITOR_SETTINGS.selected_ui_menu_id,
             }, UNDOSPECIAL_NONE);
        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        char menu_id[512];
        sprintf(menu_id, "%s (%d)", m->id, m->uID);
        nk_label(NK_CTX, menu_id, NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(NK_CTX, 20, 1);


        char id[512];
        sprintf(id, "%s (%u)", tb->id, tb->uID);

        nk_label(NK_CTX, id, NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        char ui_menu_names[MAX_UI_MENU_NAMES][256];

        char * refs[MAX_UI_MENU_NAMES];
        for (int i = 0; i < MENUS.count; i++)
        {
            sprintf(ui_menu_names[i], "%s (%d)", ms[i].id, ms[i].uID);
            refs[i] = ui_menu_names[i];
        }

        int current_menu = tb->menu;
        render_dropdown("TB UI Menu", &current_menu, (char**)refs, MENUS.count, 300, 100);

        if (current_menu != tb->menu)
        {

            int new_slot = move_text_box_to_menu(EDITOR_SETTINGS.selected_text_id, EDITOR_SETTINGS.selected_ui_menu_id, current_menu);

            EDITOR_SETTINGS.selected_text_id = new_slot;
            EDITOR_SETTINGS.selected_ui_menu_id = current_menu;

            m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];
            tb = &m->text_boxes[EDITOR_SETTINGS.selected_text_id];
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        nk_bool tb_drag = EDITOR_SETTINGS.text_box_drag_mode;
        if (nk_checkbox_label(NK_CTX, "TB Drag Mode", &tb_drag))
        {
            EDITOR_SETTINGS.text_box_drag_mode = tb_drag;
        }
        
        nk_layout_row_dynamic(NK_CTX, 20, 2);

        tb_id_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, text_id_buffer, sizeof(text_id_buffer), nk_filter_default);


        if (nk_button_label(NK_CTX, "Change ID"))
        {
            push_raw_bytes_undo(tb->id, sizeof(char) * 256, tb_structure, offsetof(TextBox, id));
            strcpy(tb->id, text_id_buffer);
        }
        nk_layout_row_dynamic(NK_CTX, 20, 1);

        if (nk_button_label(NK_CTX, "Create Text Box"))
        {

            int old_sel = EDITOR_SETTINGS.selected_text_id;
            EDITOR_SETTINGS.selected_text_id = create_text_box(tb->menu);
            push_special_undo
            (
                create_undo_structure(
                    UNDOSTRUCT_TEXTBOX,
                    (int[8])
                    {
                     EDITOR_SETTINGS.selected_text_id,
                     EDITOR_SETTINGS.selected_ui_menu_id,
                     old_sel,
                }, UNDOTEXTBOX_CREATE_TEXT_BOX), NULL, 0, 0, 0);
        }

        if (nk_button_label(NK_CTX, "Delete Text Box"))
        {
            uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
            bytes[0] = malloc(sizeof(TextBox));
            bytes[1] = malloc(sizeof(char) * tb->text_cap);
            memcpy(bytes[0], tb, sizeof(TextBox));
            memcpy(bytes[1], tb->text, sizeof(char) * tb->text_cap);
            
            size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(TextBox) + sizeof(char) * tb->text_cap;
            push_special_undo
            (
                create_undo_structure(
                    UNDOSTRUCT_TEXTBOX,
                    (int[8])
                    {
                     EDITOR_SETTINGS.selected_text_id,
                     EDITOR_SETTINGS.selected_ui_menu_id,
                }, UNDOTEXTBOX_DELETE_TEXT_BOX), bytes, byte_size, 2, 0);

            
            delete_text_box(tb->menu, tb->uID);
            EDITOR_SETTINGS.selected_text_id = -1;
        }

        bool dirty = false;
        vec2 xyval;
        xyval.x = nk_propertyf(NK_CTX, "TB_X", -5000.0f, tb->translation.x, 5000.0f, 0.01f, 0.01f);
        xyval.y = nk_propertyf(NK_CTX, "TB_Y", -5000.0f, tb->translation.y, 5000.0f, 0.01f, 0.01f);


        
        if (nk_check_equivalency(&tb->translation, &xyval, sizeof(vec2), &TEXTBOX_MENU.translation, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.translation, tb_structure, offsetof(TextBox, translation));
        }
        
        float angle = nk_propertyf(NK_CTX, "TB_Angle", 0.0f, tb->angle, 3.1415f, 0.01f, 0.01f);
        if (nk_check_equivalency(&tb->angle, &angle, sizeof(float), &TEXTBOX_MENU.angle, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.angle, tb_structure, offsetof(TextBox, angle));
        }
        vec2 scale;
        scale.x = nk_propertyf(NK_CTX, "TB_Scale_X", 0.0f, tb->scale.x, 5000.0f, 0.01f, 0.01f);
        scale.y = nk_propertyf(NK_CTX, "TB_Scale_Y", 0.0f, tb->scale.y, 5000.0f, 0.01f, 0.01f);
        if (nk_check_equivalency(&tb->scale, &scale, sizeof(vec2), &TEXTBOX_MENU.scale, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.scale, tb_structure, offsetof(TextBox, scale));
        }
        float ts = nk_propertyf(NK_CTX, "Text Size", 0.0f, tb->text_size, 5000.0f, 0.01f, 0.01f);
        if (nk_check_equivalency(&tb->text_size, &ts, sizeof(float), &TEXTBOX_MENU.text_size, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.text_size, tb_structure, offsetof(TextBox, text_size));
        }
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        uint32_t layer = nk_propertyi(NK_CTX, "Text Layer", 0, tb->layer, 5000, 1, 1);
        if (nk_check_equivalency(&tb->layer, &layer, sizeof(uint32_t), &TEXTBOX_MENU.layer, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.layer, tb_structure, offsetof(TextBox, layer));
        }
        
        static char text_buffer[1024] = {0};

        tb_text_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, text_buffer, sizeof(text_buffer), nk_filter_default);
        if (nk_button_label(NK_CTX, "Change Text"))
        {

            uint8_t ** bytes = NULL;
            int bytes_count = 0;
            size_t byte_size = 0;
            if (tb->text && tb->text_cap > 0)
            {
                
                bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(char) * tb->text_cap);
                memcpy(bytes[0], tb->text, sizeof(char) * tb->text_cap); 
                byte_size = sizeof(uint8_t*) + sizeof(char) * tb->text_cap;
                bytes_count = 1;
                
            }
            push_special_undo
            (
                create_undo_structure(UNDOSTRUCT_TEXTBOX,
                    (int[8]){
                     EDITOR_SETTINGS.selected_text_id,
                     EDITOR_SETTINGS.selected_ui_menu_id,
                     tb->text_cap
                    
                }, UNDOTEXTBOX_CHANGE_TEXT), bytes, byte_size, bytes_count, 0);
            
            if (tb->text) free(tb->text);
            tb->text = NULL;
            tb->text_cap = (strlen(text_buffer) + 1);
            tb->text = malloc(sizeof(char) * tb->text_cap);
            strcpy(tb->text, text_buffer);
            dirty = true;
        }

        struct nk_colorf color = {
            tb->color.x,
            tb->color.y,
            tb->color.z,
            tb->color.w,
        };
        nk_label(NK_CTX, "TB_Color", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgba_cf(color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            color = nk_color_picker(NK_CTX, color, NK_RGBA);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, color.r, 1.0f, 0.01f, 0.005f);
            color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, color.g, 1.0f, 0.01f, 0.005f);
            color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, color.b, 1.0f, 0.01f, 0.005f);
            color.a = nk_propertyf(NK_CTX, "#A:", 0.0f, color.a, 1.0f, 0.01f, 0.005f);

            if (nk_check_equivalency(&tb->color, &color, sizeof(vec4), &TEXTBOX_MENU.color, NULL))
            {
                push_nk_slider_undo(&TEXTBOX_MENU.color, tb_structure, offsetof(TextBox, color));
            }
            nk_combo_end(NK_CTX);
        }
    
        struct nk_colorf fill = {
            tb->fill.x,
            tb->fill.y,
            tb->fill.z,
            tb->fill.w,
        };
        nk_label(NK_CTX, "TB_fill", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgba_cf(fill), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            fill = nk_color_picker(NK_CTX, fill, NK_RGBA);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            fill.r = nk_propertyf(NK_CTX, "#R:", 0.0f, fill.r, 1.0f, 0.01f, 0.005f);
            fill.g = nk_propertyf(NK_CTX, "#G:", 0.0f, fill.g, 1.0f, 0.01f, 0.005f);
            fill.b = nk_propertyf(NK_CTX, "#B:", 0.0f, fill.b, 1.0f, 0.01f, 0.005f);
            fill.a = nk_propertyf(NK_CTX, "#A:", 0.0f, fill.a, 1.0f, 0.01f, 0.005f);

            if (nk_check_equivalency(&tb->fill, &fill, sizeof(vec4), &TEXTBOX_MENU.fill, NULL))
            {
                push_nk_slider_undo(&TEXTBOX_MENU.fill, tb_structure, offsetof(TextBox, fill));
            }
            nk_combo_end(NK_CTX);
        }

        nk_bool sh = tb->shadow == 1;
        if (nk_checkbox_label(NK_CTX, "Shadow", &sh))
        {
            tb->shadow = sh;
        }
        float sh_d = nk_propertyf(NK_CTX, "Shadow Dist", 0.0f, tb->shadow_dist, 5000.0f, 0.01f, 0.01f);
        if (nk_check_equivalency(&tb->shadow_dist, &sh_d, sizeof(float), &TEXTBOX_MENU.shadow_dist, NULL))
        {
            push_nk_slider_undo(&TEXTBOX_MENU.shadow_dist, tb_structure, offsetof(TextBox, shadow_dist));
        }

        bool dirx_dirty = false;
        vec2 sh_dir;
        float shx = nk_propertyf(NK_CTX, "Shadow DirX", -1.0f, tb->shadow_dir.x, 1.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&tb->shadow_dir.x, shx, &TEXTBOX_MENU.shadow_dirx, &dirx_dirty))
        {
            push_raw_bytes_undo(&tb->shadow_dir, sizeof(vec2), tb_structure, offsetof(TextBox, shadow_dir));
        }

        if (dirx_dirty)
        {
            tb->shadow_dir = vec2_normalize((vec2){shx, tb->shadow_dir.y});
        }
        bool diry_dirty = false;
        float shy = nk_propertyf(NK_CTX, "Shadow DirY", -1.0f, tb->shadow_dir.y, 1.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&tb->shadow_dir.y, shy, &TEXTBOX_MENU.shadow_diry, &diry_dirty))
        {
            push_raw_bytes_undo(&tb->shadow_dir, sizeof(vec2), tb_structure, offsetof(TextBox, shadow_dir));
        }
        if (diry_dirty)
        {
            tb->shadow_dir = vec2_normalize((vec2){tb->shadow_dir.x, shy});
        }

        
        nk_layout_row_dynamic(NK_CTX, 20, 1);

        char container_names[MAX_UI_CONTAINERS][256];
        char * container_refs[MAX_UI_CONTAINERS];
        int found_containers = 0;
        int pc_indices[MAX_UI_CONTAINERS];
        int current_container_found_id = -1;
        for (int i = 0; i < MAX_UI_CONTAINERS; i++)
        {
            UIContainer * current = &m->containers[i];
            if (current->free) continue;
            sprintf(container_names[found_containers], "%s (%d)", current->id, current->uID);
            container_refs[found_containers] = container_names[found_containers];
            pc_indices[found_containers] = current->uID;
            if (tb->parent_container == current->uID) current_container_found_id = found_containers;
            found_containers++;

        }

        int current_container = current_container_found_id;
        render_dropdown("Container", &current_container, (char**)container_refs, found_containers, 300, 100);

        UIContainer * cs = m->containers;
        if (current_container != current_container_found_id)
        {
            int cont_id = pc_indices[current_container];
            int old_container = tb->parent_container;
            if (cont_id != -1)
            {
                if (tb->parent_container != -1)
                {
                    remove_text_box_from_container(&cs[tb->parent_container], tb);
                }
                UIContainer * c = &cs[cont_id];

                assign_text_box_to_container(tb, c);

                push_special_undo
                (
                    create_undo_structure(
                        UNDOSTRUCT_TEXTBOX,
                        (int[8])
                        {
                         EDITOR_SETTINGS.selected_text_id,
                         EDITOR_SETTINGS.selected_ui_menu_id,
                         cont_id,
                         old_container,
                    }, UNDOTEXTBOX_CHANGE_CONTAINER), NULL, 0, 0, 0
                );
            }
        }
        if (container_is_valid(tb->parent_container, tb->menu))
        {
            
            if (nk_button_label(NK_CTX, "Remove From TB Container"))
            {

                push_special_undo
                (
                    create_undo_structure(
                        UNDOSTRUCT_TEXTBOX,
                        (int[8])
                        {
                         EDITOR_SETTINGS.selected_text_id,
                         EDITOR_SETTINGS.selected_ui_menu_id,
                         tb->parent_container,
                    }, UNDOTEXTBOX_REMOVE_FROM_CONTAINER), NULL, 0, 0, 0
                );
                remove_text_box_from_container(&cs[tb->parent_container], tb);
                tb->parent_container = -1;
            }
        }

        if (dirty)
        {
            add_all_text_boxes_to_draw_buffer();
        }
    
        
    }
    nk_end(NK_CTX);

    
}

char menu_id_buffer[256];
char element_id_buffer[256];

char element_parent_container_buffer[256];

char element_cid_buffer[256];
char scrollbar_buffer[256];
char label_id_buffer[256];

void render_ui_menu()
{

    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_UI],
            nk_rect(20, 20, 420, SCREEN_HEIGHT - 120),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE | NK_WINDOW_MOVABLE
        )
    )
    {
        
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }


        UIMenu * ms = CAST_VEC(MENUS, UIMenu);
        UIMenu * m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];
        UIElement * els = m->elements;
        UIElement * el = &els[EDITOR_SETTINGS.selected_ui_element_id];

        UndoStructure menu_structure = create_undo_structure(UNDOSTRUCT_MENU, (int[8]){EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id}, UNDOSPECIAL_NONE);
        UndoStructure el_structure = create_undo_structure(UNDOSTRUCT_UI_ELEMENT, (int[8]){EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id}, UNDOSPECIAL_NONE);
        
        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        char menu_id[512];
        sprintf(menu_id, "%s (%d)", m->id, m->uID);
        nk_label(NK_CTX, menu_id, NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        ui_menu_id_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, menu_id_buffer, sizeof(menu_id_buffer), nk_filter_default);
        
        if (nk_button_label(NK_CTX, "Menu Change ID"))
        {
            push_raw_bytes_undo(m->id, sizeof(m->id), menu_structure, offsetof(UIMenu, id));
            strcpy(m->id, menu_id_buffer);
        }


        nk_layout_row_dynamic(NK_CTX, 20, 1);

        char id[512];
        sprintf(id, "%s (%d)", el->id, el->uID);

        nk_label(NK_CTX, id, NK_TEXT_ALIGN_LEFT);

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        char ui_menu_names[MAX_UI_MENU_NAMES][256];

        char * refs[MAX_UI_MENU_NAMES];
        for (int i = 0; i < MENUS.count; i++)
        {
            sprintf(ui_menu_names[i], "%s (%d)", ms[i].id, ms[i].uID);
            refs[i] = ui_menu_names[i];
        }

        int current_menu = el->menu;
        render_dropdown("UI Menu", &current_menu, (char**)refs, MENUS.count, 300, 100);

        if (current_menu != el->menu)
        {

            int new_slot = move_element_to_menu(EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id, current_menu);
            EDITOR_SETTINGS.selected_ui_element_id = new_slot;
            EDITOR_SETTINGS.selected_ui_menu_id = current_menu;

        }


        nk_layout_row_dynamic(NK_CTX, 20, 1);

        nk_bool el_drag = EDITOR_SETTINGS.element_drag_mode;
        if (nk_checkbox_label(NK_CTX, "Element Drag", &el_drag))
        {
            EDITOR_SETTINGS.element_drag_mode = el_drag;
        }

        nk_layout_row_dynamic(NK_CTX, 20, 2);

        ui_id_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, element_id_buffer, sizeof(element_id_buffer), nk_filter_default);


        if (nk_button_label(NK_CTX, "Change ID"))
        {
            push_raw_bytes_undo(el->id, sizeof(el->id), el_structure, offsetof(UIElement, id));
            strcpy(el->id, element_id_buffer);
        }
        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        if (nk_button_label(NK_CTX, "Create Element"))
        {

            EDITOR_SETTINGS.selected_ui_element_id = create_ui_element(el->menu);
            push_special_undo(create_undo_structure(UNDOSTRUCT_UI_ELEMENT, (int[8]){EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id}, UNDOUIELEMENT_CREATE_ELEMENT), NULL, 0, 0, 0);
        }
        if (nk_button_label(NK_CTX, "Delete Element"))
        {

            push_special_undo_normal_bytes(create_undo_structure(UNDOSTRUCT_UI_ELEMENT, (int[8]){EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id}, UNDOUIELEMENT_DELETE_ELEMENT), el, sizeof(UIElement), 0);
            delete_ui_element(el->menu, el->uID);
        }

        bool dirty = false;
        float xval = nk_propertyf(NK_CTX, "UI_X", -5000.0f, el->translation.x, 5000.0f, 0.1f, 0.1f);
        if (check_equivalencyf(&el->translation.x, xval, &UIELEMENT_MENU.x, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.x, el_structure, offsetof(UIElement, translation.x));
        }

        float yval = nk_propertyf(NK_CTX, "UI_Y", -5000.0f, el->translation.y, 5000.0f, 0.1f, 0.1f);
        if (check_equivalencyf(&el->translation.y, yval, &UIELEMENT_MENU.y, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.y, el_structure, offsetof(UIElement, translation.y));
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        if (nk_button_label(NK_CTX, "New Base Pos"))
        {
            push_raw_bytes_undo(&el->base_pos, sizeof(vec2), el_structure, offsetof(UIElement, base_pos));
            el->base_pos = el->translation;
        }
        

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        float xsc = nk_propertyf(NK_CTX, "UI_Scale_X", 0.0f, el->scale.x, 5000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->scale.x, xsc, &UIELEMENT_MENU.sx, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.sx, el_structure, offsetof(UIElement, scale.x));
        }
        float ysc = nk_propertyf(NK_CTX, "UI_Scale_Y", 0.0f, el->scale.y, 5000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->scale.y, ysc, &UIELEMENT_MENU.sy, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.sy, el_structure, offsetof(UIElement, scale.y));
        }
        
        float r = nk_propertyf(NK_CTX, "UI_Radius", 0.0f, el->radius, 5000.0f, 0.001f, 0.001f);
        if (check_equivalencyf(&el->radius, r, &UIELEMENT_MENU.radius, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.radius, el_structure, offsetof(UIElement, radius));
        }
        
        float angle = nk_propertyf(NK_CTX, "UI_Angle", 0.0f, el->angle, 3.1415f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->angle, angle, &UIELEMENT_MENU.angle, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.angle, el_structure, offsetof(UIElement, angle));
        }

        float sx = nk_propertyf(NK_CTX, "UI_Size_X", 0.0f, el->size.x, 5000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->size.x, sx, &UIELEMENT_MENU.sizex, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.sizex, el_structure, offsetof(UIElement, size.x));
        }
        float sy = nk_propertyf(NK_CTX, "UI_Size_Y", 0.0f, el->size.y, 5000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->size.y, sy, &UIELEMENT_MENU.sizey, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.sizey, el_structure, offsetof(UIElement, size.y));
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        int el_layer = el->layer;
        int layer = nk_propertyi(NK_CTX, "UI_Layer", 0, el->layer, 5000, 1, 1);
        if (check_equivalencyi(&el_layer, layer, &UIELEMENT_MENU.layer, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.layer, el_structure, offsetof(UIElement, layer));
        }
        if (layer != el->layer)
        {
            el->layer = layer;
        }

        if (nk_combo_begin_label(NK_CTX, "Action", nk_vec2(200, 200))) {
            
            char ** options = malloc(sizeof(char*) * ACTION_CALLBACK_COUNT);
            for (int i = 0; i < ACTION_CALLBACK_COUNT; i++)
            {
                options[i] = malloc(sizeof(char) * 256);
                strcpy(options[i], ACTION_CALLBACKS[i].id);
            }

            int selected_id = EDITOR_SETTINGS.selected_ui_action_id;

            render_dropdown_labels(&EDITOR_SETTINGS.selected_ui_action_id, options, ACTION_CALLBACK_COUNT);
            
            if (EDITOR_SETTINGS.selected_ui_action_id != selected_id)
            {
                el->action_callback = ACTION_CALLBACKS[EDITOR_SETTINGS.selected_ui_action_id];
                strcpy(el->action_callback.id, ACTION_CALLBACKS[EDITOR_SETTINGS.selected_ui_action_id].id);
            }
            
            for (int i = 0; i < ACTION_CALLBACK_COUNT; i++)
            {
                free(options[i]);
            }
            free(options);

            nk_combo_end(NK_CTX);
        }
    
        if (nk_combo_begin_label(NK_CTX, "Hover", nk_vec2(200, 200))) {
            
            char ** options = malloc(sizeof(char*) * HOVER_CALLBACK_COUNT);
            for (int i = 0; i < HOVER_CALLBACK_COUNT; i++)
            {
                options[i] = malloc(sizeof(char) * 256);
                strcpy(options[i], HOVER_CALLBACKS[i].id);
            }
            
            int selected_id = EDITOR_SETTINGS.selected_ui_hover_id;
            
            render_dropdown_labels(&EDITOR_SETTINGS.selected_ui_hover_id, options, HOVER_CALLBACK_COUNT);

            if (EDITOR_SETTINGS.selected_ui_hover_id != selected_id)
            {
                el->hover_callback = HOVER_CALLBACKS[EDITOR_SETTINGS.selected_ui_hover_id];
                strcpy(el->hover_callback.id, HOVER_CALLBACKS[EDITOR_SETTINGS.selected_ui_hover_id].id);
            }
            
            for (int i = 0; i < HOVER_CALLBACK_COUNT; i++)
            {
                free(options[i]);
            }
            free(options);

            nk_combo_end(NK_CTX);
        }
    
        

        struct nk_colorf color = {
            el->color.x,
            el->color.y,
            el->color.z,
            el->color.w,
        };
        nk_label(NK_CTX, "UI_Color", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgba_cf(color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            color = nk_color_picker(NK_CTX, color, NK_RGBA);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, color.r, 1.0f, 0.01f, 0.005f);
            color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, color.g, 1.0f, 0.01f, 0.005f);
            color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, color.b, 1.0f, 0.01f, 0.005f);
            color.a = nk_propertyf(NK_CTX, "#A:", 0.0f, color.a, 1.0f, 0.01f, 0.005f);
            if (check_equivalencyv4(&el->color, (vec4){color.r, color.g, color.b, color.a}, &UIELEMENT_MENU.color, NULL))
            {
                push_nk_slider_undo(&UIELEMENT_MENU.color, el_structure, offsetof(UIElement, color));
            }
            nk_combo_end(NK_CTX);
        }
    
        struct nk_colorf fill = {
            el->fill.x,
            el->fill.y,
            el->fill.z,
            el->fill.w,
        };
        nk_label(NK_CTX, "UI_fill", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgba_cf(fill), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            fill = nk_color_picker(NK_CTX, fill, NK_RGBA);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            fill.r = nk_propertyf(NK_CTX, "#R:", 0.0f, fill.r, 1.0f, 0.01f, 0.005f);
            fill.g = nk_propertyf(NK_CTX, "#G:", 0.0f, fill.g, 1.0f, 0.01f, 0.005f);
            fill.b = nk_propertyf(NK_CTX, "#B:", 0.0f, fill.b, 1.0f, 0.01f, 0.005f);
            fill.a = nk_propertyf(NK_CTX, "#A:", 0.0f, fill.a, 1.0f, 0.01f, 0.005f);

            if (check_equivalencyv4(&el->fill, (vec4){fill.r, fill.g, fill.b, fill.a}, &UIELEMENT_MENU.fill, NULL))
            {
                push_nk_slider_undo(&UIELEMENT_MENU.fill, el_structure, offsetof(UIElement, fill));
            }
            
            nk_combo_end(NK_CTX);
        }

        nk_bool sh = el->shadow == 1;
        if (nk_checkbox_label(NK_CTX, "Shadow", &sh))
        {
            el->shadow = sh;
        }
        float sh_d = nk_propertyf(NK_CTX, "Shadow Dist", 0.0f, el->shadow_dist, 5000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->shadow_dist, sh_d, &UIELEMENT_MENU.shadow_dist, NULL))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.shadow_dist, el_structure, offsetof(UIElement, shadow_dist));
        }

        bool dir_dirty = false;
        float shx = nk_propertyf(NK_CTX, "Shadow DirX", -1.0f, el->shadow_dir.x, 1.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->shadow_dir.x, shx, &UIELEMENT_MENU.shadow_dirx, &dir_dirty))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.shadow_dirx, el_structure, offsetof(UIElement, shadow_dir.x));
        }
        float shy = nk_propertyf(NK_CTX, "Shadow DirY", -1.0f, el->shadow_dir.y, 1.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&el->shadow_dir.y, shy, &UIELEMENT_MENU.shadow_diry, &dir_dirty))
        {
            push_nk_slider_undo(&UIELEMENT_MENU.shadow_diry, el_structure, offsetof(UIElement, shadow_dir.y));
        }

        nk_layout_row_dynamic(NK_CTX, 20, 2);


        char container_names[MAX_UI_CONTAINERS][256];
        char * container_refs[MAX_UI_CONTAINERS];
        int found_containers = 0;
        int pc_indices[MAX_UI_CONTAINERS];
        int current_container_found_id = -1;
        for (int i = 0; i < MAX_UI_CONTAINERS; i++)
        {
            UIContainer * current = &m->containers[i];
            if (current->free) continue;
            sprintf(container_names[found_containers], "%s (%d)", current->id, current->uID);
            container_refs[found_containers] = container_names[found_containers];
            pc_indices[found_containers] = current->uID;
            if (el->parent_container == current->uID) current_container_found_id = found_containers;
            found_containers++;

        }

        int current_container = current_container_found_id;
        render_dropdown("Container", &current_container, (char**)container_refs, found_containers, 300, 100);

        if (current_container != current_container_found_id)
        {
            
            UIContainer * cs = m->containers;
            int cont_id = pc_indices[current_container];
            
            if (el->parent_container != -1)
            {
                remove_element_from_container(&cs[el->parent_container], el);
            }

            push_special_undo(create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                            (int[8])
                            {
                                EDITOR_SETTINGS.selected_ui_element_id,
                                EDITOR_SETTINGS.selected_ui_menu_id,
                                el->parent_container,
                                cont_id
                            }, UNDOUIELEMENT_CHANGE_CONTAINER), NULL, 0, 0, 0);


            if (cont_id != -1 && cont_id != el->container)
            {
                UIContainer * c = &cs[cont_id];

                assign_element_to_container(el, c);
            }

         }


        if (el->parent_container != -1)
        {
            nk_layout_row_dynamic(NK_CTX, 20, 1);

            if (nk_button_label(NK_CTX, "Remove From Container"))
            {
                
                push_special_undo(create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                (int[8])
                                {
                                    EDITOR_SETTINGS.selected_ui_element_id,
                                    EDITOR_SETTINGS.selected_ui_menu_id,
                                    el->parent_container,
                                }, UNDOUIELEMENT_REMOVE_FROM_CONTAINER), NULL, 0, 0, 0);
                UIContainer * cs = m->containers;
                remove_element_from_container(&cs[el->parent_container], el);
                
            }
            
        }

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        bool cont_valid = container_is_valid(el->container, el->menu);
        if (nk_button_label(NK_CTX, "Create Container"))
        {

            UIContainer * cs = m->containers;
            uint8_t ** bytes = NULL;
            size_t byte_size = 0;
            int special_count = 0;

            int old_container = el->container;
            
            if (cont_valid)
            {
                bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(UIContainer));
                UIContainer * cont = &cs[el->container];
                memcpy(bytes[0], cont, sizeof(UIContainer));
                delete_ui_container(el);
                byte_size += sizeof(uint8_t*);
                byte_size += sizeof(UIContainer);
                special_count = 1;
            }
            else
            {
                bytes[0] = NULL;
            }

            UIContainer * cont = create_ui_container(el, EDITOR_SETTINGS.selected_ui_menu_id);
            push_special_undo(create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                (int[8])
                                {
                                    EDITOR_SETTINGS.selected_ui_element_id,
                                    EDITOR_SETTINGS.selected_ui_menu_id,
                                    old_container,
                                }, UNDOUIELEMENT_CREATE_CONTAINER), bytes, byte_size, special_count, 0);

        }

        // we are here
        if (cont_valid)
        {
            UIContainer * cs = m->containers;
            UIContainer * c = &cs[el->container];
            if (nk_button_label(NK_CTX, "Delete Container"))
            {
                uint8_t ** bytes = malloc(sizeof(uint8_t*));
                size_t byte_size = sizeof(uint8_t*) + sizeof(UIContainer);;

                int old_container = el->container;
                UIContainer * cont = &cs[el->container];
                bytes[0] = malloc(sizeof(UIContainer));
                memcpy(bytes[0], cont, sizeof(UIContainer));

                push_special_undo(create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                    (int[8])
                                    {
                                        EDITOR_SETTINGS.selected_ui_element_id,
                                        EDITOR_SETTINGS.selected_ui_menu_id,
                                        old_container,
                                        -1,
                                    }, UNDOUIELEMENT_DELETE_CONTAINER), bytes, byte_size, 1, 0);
                delete_ui_container(el);
            }
            

            
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            if (nk_tree_push(NK_CTX, NK_TREE_NODE, "UI Containers", NK_MINIMIZED))
            {
            
                UndoStructure container_undo = create_undo_structure(UNDOSTRUCT_UI_CONTAINER, (int[8]){EDITOR_SETTINGS.selected_ui_element_id, EDITOR_SETTINGS.selected_ui_menu_id, c->uID}, UNDOSPECIAL_NONE);
                nk_layout_row_dynamic(NK_CTX, 20, 1);

                char id[256];
                sprintf(id, "%s (%d)", c->id, c->uID);

                nk_label(NK_CTX, id, NK_TEXT_ALIGN_LEFT);
        
                nk_layout_row_dynamic(NK_CTX, 20, 2);

                ui_cont_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, element_cid_buffer, sizeof(element_cid_buffer), nk_filter_default);
                
                nk_bool wrap = c->wrap == 1;
                if (nk_checkbox_label(NK_CTX, "Wrap", &wrap))
                {
                    c->wrap = wrap;
                }

                nk_bool mask = c->mask == 1;
                if (nk_checkbox_label(NK_CTX, "Mask", &mask))
                {
                    c->mask = mask;
                }

                nk_layout_row_dynamic(NK_CTX, 20, 1);

                float cx = nk_propertyf(NK_CTX, "CPAD_L", 0.0f, c->padding.x, 1.0f, 0.001f, 0.001f);
                
                float cy = nk_propertyf(NK_CTX, "CPAD_R", 0.0f, c->padding.y, 1.0f, 0.001f, 0.001f);
                float cz = nk_propertyf(NK_CTX, "CPAD_U", 0.0f, c->padding.z, 1.0f, 0.001f, 0.001f);
                float cw = nk_propertyf(NK_CTX, "CPAD_D", 0.0f, c->padding.w, 1.0f, 0.001f, 0.001f);
                if (check_equivalencyv4(&c->padding, (vec4){cx, cy, cz, cw}, &UIELEMENT_MENU.cpad, NULL))
                {
                    push_nk_slider_undo(&UIELEMENT_MENU.cpad, container_undo, offsetof(UIContainer, padding));
                }

                nk_layout_row_dynamic(NK_CTX, 20, 2);
                float xsc = nk_propertyf(NK_CTX, "C_Scroll_X", -1.0f, c->scroll_dir.x, 1.0f, 0.01f, 0.01f);
                bool xsc_dirty = false;
                
                if (check_equivalencyf(&c->scroll_dir.x, xsc, &UIELEMENT_MENU.cscrollx, &xsc_dirty))
                {
                    push_nk_slider_undo(&UIELEMENT_MENU.cscrollx, container_undo, offsetof(UIContainer, scroll_dir.x));
                }
                if (xsc_dirty)
                {
                    c->scroll_dir = vec2_normalize((vec2){xsc, c->scroll_dir.y});
                }
                float ysc = nk_propertyf(NK_CTX, "C_Scroll_Y", -1.0f, c->scroll_dir.y, 1.0f, 0.01f, 0.01f);
                bool ysc_dirty = false;
                if (check_equivalencyf(&c->scroll_dir.y, ysc, &UIELEMENT_MENU.cscrolly, &ysc_dirty))
                {
                    push_nk_slider_undo(&UIELEMENT_MENU.cscrolly, container_undo, offsetof(UIContainer, scroll_dir.y));
                }
                if (ysc_dirty)
                {
                    c->scroll_dir = vec2_normalize((vec2){c->scroll_dir.x, ysc});
                }


                nk_layout_row_dynamic(NK_CTX, 20, 1);
                if (c->scrollbar != -1)
                {
                    UIElement * sc = &m->elements[c->scrollbar];
                    char current_scrollbar_name[256];

                    sprintf(current_scrollbar_name, "%s (%d)",  sc->id, sc->uID);

                    nk_label(NK_CTX, current_scrollbar_name, NK_TEXT_ALIGN_LEFT);

                }

                

                
                char scrollbar_names[MAX_UI_ELEMENTS][256];
                char * scrollbar_refs[MAX_UI_ELEMENTS];
                int found_scrollbars = 0;
                int sc_indices[MAX_UI_ELEMENTS];
                int current_scrollbar_found_id = -1;
                for (int i = 0; i < MAX_UI_ELEMENTS; i++)
                {
                    UIElement * current = &m->elements[i];
                    if (!current->visible || current->free || current->interactable != UI_INTERACTABLE_SLIDER) continue;
                    sprintf(scrollbar_names[found_scrollbars], "%s (%d)", current->id, current->uID);
                    scrollbar_refs[found_scrollbars] = scrollbar_names[found_scrollbars];
                    sc_indices[found_scrollbars] = current->uID;
                    if (c->scrollbar == current->uID) current_scrollbar_found_id = found_scrollbars;
                    found_scrollbars++;

                }

                int current_scrollbar = current_scrollbar_found_id;
                render_dropdown("Scrollbar", &current_scrollbar, (char**)scrollbar_refs, found_scrollbars, 300, 100);

                if (current_scrollbar != current_scrollbar_found_id)
                {
                    
                    int element_id = sc_indices[current_scrollbar];
                    
                    if (element_id != -1)
                    {

                        int old_scrollbar = c->scrollbar;
                        int old_scrollbar_container = -1;
                        if (old_scrollbar != -1)
                        {
                            UIElement * old_sc = &els[old_scrollbar];
                            if (old_sc->interactable == UI_INTERACTABLE_SLIDER)
                            {
                                old_scrollbar_container = old_sc->as.slider.container;
                            
                                old_sc->as.slider.container = -1;
                            }
                        }
                        c->scrollbar = element_id;
                        UIElement * sc = &els[element_id];
                        int old_slider_container = sc->as.slider.container;
                        sc->as.slider.container = c->uID;

                        push_special_undo
                            (
                                create_undo_structure(UNDOSTRUCT_UI_CONTAINER, (int[8])
                                    {
                                        EDITOR_SETTINGS.selected_ui_element_id,
                                        EDITOR_SETTINGS.selected_ui_menu_id,
                                        c->uID,
                                        old_scrollbar,
                                        element_id,
                                        old_slider_container,
                                        -1,                                        
                                    }, UNDOUICONTAINER_CHANGE_SCROLLBAR)
                        , NULL, 0, 0, 0);
                    }
                    
                }
                

                

                nk_tree_pop(NK_CTX);

            }
            
        }


        nk_layout_row_dynamic(NK_CTX, 20, 1);
        int interactable_type = el->interactable;
        render_dropdown("Interactable", &interactable_type, (char**)UI_INTERACTABLE_NAMES, UI_INTERACTABLE_MAX, 300, 100);
        if (interactable_type != el->interactable)
        {
            
            el->interactable = interactable_type;
            switch(interactable_type)
            {
                case UI_INTERACTABLE_SLIDER:
                    UISlider * sl = &el->as.slider;
                    *sl = DEFAULT_SLIDER;
                    compute_slider_bounds(el);
                    break;
                case UI_INTERACTABLE_LABEL:
                    UILabel * l = &el->as.label;
                    *l = DEFAULT_LABEL;
                    break;
                default:
                    break;
            }

        }

        switch(el->interactable)
        {

            case UI_INTERACTABLE_SLIDER:
                {

                    UISlider * sl = &el->as.slider;

                    nk_layout_row_dynamic(NK_CTX, 20, 1);
                    if (nk_button_label(NK_CTX, "Compute Bounds"))
                    {
                        push_raw_bytes_undo(sl, sizeof(UISlider), el_structure, offsetof(UIElement, as));
                        compute_slider_bounds(el);
                    }

                    nk_layout_row_dynamic(NK_CTX, 20, 2);
                    float dirx = nk_propertyf(NK_CTX, "Slider_X", 0.0f, sl->dir.x, 1.0f, 0.001f, 0.001f);
                    bool dirx_dirty = false;
                    if (check_equivalencyf(&sl->dir.x, dirx, &UIELEMENT_MENU.sliderx, &dirx_dirty))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.sliderx, el_structure, offsetof(UIElement, as.slider.dir.x));
                    }
                    
                    if (dirx_dirty)
                    {
                        sl->dir = vec2_normalize((vec2){dirx, sl->dir.y});
                    }
                    float diry = nk_propertyf(NK_CTX, "Slider_Y", 0.0f, sl->dir.y, 1.0f, 0.001f, 0.001f);
                    bool diry_dirty = false;
                    if (check_equivalencyf(&sl->dir.y, diry, &UIELEMENT_MENU.slidery, &diry_dirty))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.slidery, el_structure, offsetof(UIElement, as.slider.dir.y));
                    }
                    if (diry_dirty)
                    {
                        sl->dir = vec2_normalize((vec2){sl->dir.x, diry});
                    }
                    
                    float minval = nk_propertyf(NK_CTX, "Slider_MinVal", -5000.0f, sl->min_val, 5000.0f, 0.01f, 0.01f);
                    if (check_equivalencyf(&sl->min_val, minval, &UIELEMENT_MENU.slidermin, NULL))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.slidermin, el_structure, offsetof(UIElement, as.slider.min_val));
                    }
                    float maxval = nk_propertyf(NK_CTX, "Slider_MaxVal", -5000.0f, sl->max_val, 5000.0f, 0.01f, 0.01f);
                    if (check_equivalencyf(&sl->max_val, maxval, &UIELEMENT_MENU.slidermax, NULL))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.slidermax, el_structure, offsetof(UIElement, as.slider.max_val));
                    }
                    float len = nk_propertyf(NK_CTX, "Slider_Len", -5000.0f, sl->len , 5000.0f, 0.01f, 0.01f);
                    if (check_equivalencyf(&sl->len, len, &UIELEMENT_MENU.sliderlen, NULL))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.sliderlen, el_structure, offsetof(UIElement, as.slider.len));
                    }
                    float scroll_scale = nk_propertyf(NK_CTX, "Slider_ScrollScale", -5000.0f, sl->scroll_scale , 5000.0f, 0.01f, 0.01f);

                    if (check_equivalencyf(&sl->scroll_scale, scroll_scale, &UIELEMENT_MENU.sliderscrollscale, NULL))
                    {
                        push_nk_slider_undo(&UIELEMENT_MENU.sliderscrollscale, el_structure, offsetof(UIElement, as.slider.scroll_scale));
                    }

                }
                break;
            case UI_INTERACTABLE_LABEL:
                {

                    UILabel * l = &el->as.label;

                    nk_layout_row_dynamic(NK_CTX, 20, 2);

                    ui_label_id_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, label_id_buffer, sizeof(label_id_buffer), nk_filter_default);

                    if (nk_button_label(NK_CTX, "Assign Label"))
                    {
                        int label = find_text_box_in_menu(m, label_id_buffer);
                        push_raw_bytes_undo(&l->box, sizeof(int), create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                                                                                            (int[8]){
                            EDITOR_SETTINGS.selected_ui_element_id,
                            EDITOR_SETTINGS.selected_ui_menu_id,
                        }, UNDOSPECIAL_NONE), offsetof(UIElement, as.label.box));
                        l->box = label;
                    }

                    if (nk_button_label(NK_CTX, "Create Label"))
                    {
                        // delete old label
                        uint8_t ** bytes = NULL;
                        int byte_count = 0;
                        size_t byte_size = 0;
                        int old_box = l->box;
                        if (l->box != -1)
                        {
                            TextBox * tb = &m->text_boxes[l->box];
                            uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                            bytes[0] = malloc(sizeof(TextBox));
                            bytes[1] = malloc(sizeof(char) * tb->text_cap);
                            memcpy(bytes[0], tb, sizeof(TextBox));
                            memcpy(bytes[1], tb->text, sizeof(char) * tb->text_cap);
                            delete_text_box(m->uID, l->box);
                            byte_size = sizeof(uint8_t*) + sizeof(TextBox) + sizeof(char) * tb->text_cap;
                            byte_count = 2;
                        }

                        int tb_id = create_text_box(m->uID);
                        l->box = tb_id;
                        align_text_box_label(el);
                        add_all_text_boxes_to_draw_buffer();
                        
                        push_special_undo
                            (
                                create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                (int[8])
                                {
                                    EDITOR_SETTINGS.selected_ui_element_id,
                                    EDITOR_SETTINGS.selected_ui_menu_id,
                                    old_box,
                                    tb_id,
                                }, UNDOUIELEMENT_CREATE_LABEL), bytes, byte_size, byte_count, 0);
                    }


                    if (l->box != -1)
                    {
                        nk_layout_row_dynamic(NK_CTX, 20, 1);
                        int selected_justify = l->align;
                        render_dropdown("Justify", &selected_justify, (char**)JUSTIFY_NAMES, JUSTIFY_MAX, 300, 100);
                        if (selected_justify != l->align)
                        {
                            push_special_undo
                                (
                                    create_undo_structure(UNDOSTRUCT_UI_ELEMENT,
                                    (int[8])
                                    {
                                        EDITOR_SETTINGS.selected_ui_element_id,
                                        EDITOR_SETTINGS.selected_ui_menu_id,
                                        l->align,
                                        
                                    }, UNDOUIELEMENT_JUSTIFY_LABEL), NULL, 0, 0, 0);
                            l->align = selected_justify;
                            align_text_box_label(el);
                        }
                        
                    }
                }

                break;
            default:
                break;
            
        }

        
        if (dir_dirty)
        {
            el->shadow_dir = vec2_normalize(el->shadow_dir);
        }

        

        
    }
    nk_end(NK_CTX);


    
}


void render_audio_menu()
{
    
    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_AUDIO],
            nk_rect(420, 20, SCREEN_WIDTH - 420 * 2, 300),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }


        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "Audio Probes", NK_MINIMIZED))
        {

            nk_layout_row_dynamic(NK_CTX, 20, 1);

            int iterations = nk_propertyi(NK_CTX, "Probe Iterations", 1, EDITOR_SETTINGS.audio_probe_iterations, 30000, 1, 1);
            if (iterations != EDITOR_SETTINGS.audio_probe_iterations)
            {
                EDITOR_SETTINGS.audio_probe_iterations = iterations;
            }
            int bounces = nk_propertyi(NK_CTX, "Probe Bounces", 1, EDITOR_SETTINGS.audio_probe_bounces, 100, 1, 1);
            if (bounces != EDITOR_SETTINGS.audio_probe_bounces)
            {
                EDITOR_SETTINGS.audio_probe_bounces = bounces;
            }


            float volume = nk_propertyf(NK_CTX, "Volume", 0.0, EDITOR_SETTINGS.volume, 10.0f, 0.01f, 0.01f);
            if (volume != EDITOR_SETTINGS.volume)
            {
                EDITOR_SETTINGS.volume = volume;
            }
            float wet = nk_propertyf(NK_CTX, "Wet", 0.0, EDITOR_SETTINGS.reverb_wet, 10.0f, 0.01f, 0.01f);
            if (wet != EDITOR_SETTINGS.reverb_wet)
            {
                EDITOR_SETTINGS.reverb_wet = wet;
            }
        

            nk_tree_pop(NK_CTX);
        }


        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "Audio Emitters", NK_MINIMIZED))
        {

            nk_layout_row_dynamic(NK_CTX, 20, 2);
            render_file_directory_dropdown(&sound_registry);

            if (nk_button_label(NK_CTX, "Spawn Emitter"))
            {

                AudioEmitter * fs_emitter = init_emitter("./sounds/footstep_01.wav", true, true, 1.0);
                push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){fs_emitter->id, EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOAEMITTER_SPAWN), NULL, 0, 0, 0);
                EDITOR_SETTINGS.selected_audio_emitter_id = fs_emitter->id;
            }
            
            if (EDITOR_SETTINGS.selected_audio_emitter_id >= 0 && EDITOR_SETTINGS.selected_audio_emitter_id < MAX_AUDIO_EMITTERS)
            {

                AudioEmitter * em = &AUDIO_EMITTERS[EDITOR_SETTINGS.selected_audio_emitter_id];
                UndoStructure emitter_us = create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOSPECIAL_NONE);

                render_pos_widget("AEM", UNDOSTRUCT_AEMITTER, EDITOR_SETTINGS.selected_audio_emitter_id, &em->pos, APROBE_MENU.emitter_pos);

            
                float volume = nk_propertyf(NK_CTX, "AEM Volume", 0.0, em->base_volume, 10.0f, 0.01f, 0.01f);
                if (check_equivalencyf(&em->base_volume, volume, &APROBE_MENU.volume, NULL))
                {
                    push_nk_slider_undo(&APROBE_MENU.volume, emitter_us, offsetof(AudioEmitter, base_volume));
                }
                
                nk_bool loop = em->is_loop == 1;
                if (nk_checkbox_label(NK_CTX, "AEM Loop", &loop))
                {
                    em->is_loop = loop;
                }
                nk_bool occludable = em->is_occludable == 1;
                if (nk_checkbox_label(NK_CTX, "AEM Occludable", &occludable))
                {
                    em->is_occludable = occludable;
                }

                if (nk_button_label(NK_CTX, "Delete Sound Obj"))
                {
                    if (em->is_loaded)
                    {
                        push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOSPECIAL_NONE), NULL, 0, 0, 0);
                        ma_sound_uninit(&em->sound_obj);
                        em->is_loaded = false;
                    }
                }

                nk_label(NK_CTX, em->path, NK_TEXT_ALIGN_LEFT);
                if (nk_button_label(NK_CTX, "Set Sound"))
                {

                    if (sound_registry.current_selection < sound_registry.names.count && sound_registry.current_selection >= 0)
                    {

                        push_raw_bytes_undo(em->path, sizeof(char) * 256, emitter_us, offsetof(AudioEmitter, path));

                        char complete_name[256];
                        strcpy(complete_name, sound_registry.directory);
                        strncat(complete_name, &(((char*)(sound_registry.names.buffer))[sound_registry.current_selection * 256]), 255 - strlen(sound_registry.directory));
                        complete_name[255] = '\0';
                        strcpy(em->path, complete_name);
                    }
                
                }
                if (nk_button_label(NK_CTX, "Load Sound Obj"))
                {
                    if (!em->is_loaded)
                    {
                        push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id},  UNDOAEMITTER_LOAD_SOUND_OBJ), NULL, 0, 0, 0);
                        load_emitter(em, em->path, em->is_loop, em->is_occludable, em->base_volume);
                    }
                }
                if (nk_button_label(NK_CTX, "Delete Emitter"))
                {
                    if (em->is_loaded)
                    {
                        
                        push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOAEMITTER_DELETE_EMITTER), NULL, 0, 0, 0);
                        destroy_emitter(EDITOR_SETTINGS.selected_audio_emitter_id);
                        
                    }
                }
                if (nk_button_label(NK_CTX, "Play Emitter"))
                {
                
                    push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOAEMITTER_PLAY_EMITTER), NULL, 0, 0, 0);
                    play_emitter(em, em->pos);

                }
                if (nk_button_label(NK_CTX, "Stop Emitter"))
                {
                    push_special_undo(create_undo_structure(UNDOSTRUCT_AEMITTER, (int[8]){EDITOR_SETTINGS.selected_audio_emitter_id}, UNDOAEMITTER_STOP_EMITTER), NULL, 0, 0, 0);
                    end_emitter(em);
                }
                
            
            }
            nk_tree_pop(NK_CTX);
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "Audio Portals", NK_MINIMIZED))
        {

            aportal_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, aportal_name_buffer, sizeof(aportal_name_buffer), nk_filter_default);
            if (nk_button_text(NK_CTX, "Save Audio Portals", 12))
            {

                char complete_name[256];
                strcpy(complete_name, aportal_registry.directory);
                strncat(complete_name, aportal_name_buffer, 255 - strlen(aportal_registry.directory));
                complete_name[255] = '\0';
                save_aportals(complete_name, &GLOBALS.audio_portals);
            }


            render_file_directory_dropdown(&aportal_registry);

            if (nk_button_label(NK_CTX, "Load Audio Portals"))
            {
                
                if (aportal_registry.current_selection < aportal_registry.names.count && aportal_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, aportal_registry.directory);
                    strncat(complete_name, &(((char*)(aportal_registry.names.buffer))[aportal_registry.current_selection * 256]), 255 - strlen(aportal_registry.directory));
                    complete_name[255] = '\0';
                    load_aportals(complete_name, &GLOBALS.audio_portals);
                }

            }

            if (nk_button_label(NK_CTX, "Spawn Audio Portal"))
            {

                push_special_undo(create_undo_structure(UNDOSTRUCT_APORTAL, (int[8]){EDITOR_SETTINGS.selected_audio_portal_id}, UNDOAPORTAL_SPAWN), NULL, 0, 0, 0);
                init_new_audio_portal((vec3){0, 0, 0});
                EDITOR_SETTINGS.selected_audio_portal_id = GLOBALS.audio_portals.count-1;

            }
        
            if (nk_button_label(NK_CTX, "Recalculate Audio Portal Paths"))
            {
                recalculate_audio_portal_paths();
            }
            if (EDITOR_SETTINGS.selected_audio_portal_id >= 0 && EDITOR_SETTINGS.selected_audio_portal_id < GLOBALS.audio_portals.count)
            {

                AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
                AudioPortal * p = &portals[EDITOR_SETTINGS.selected_audio_portal_id];

                render_pos_widget("APortal", UNDOSTRUCT_APORTAL, EDITOR_SETTINGS.selected_audio_portal_id, &p->pos, APROBE_MENU.aportal_pos);

                if (nk_button_label(NK_CTX, "Delete Audio Portal"))
                {

                    uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                    bytes[0] = malloc(sizeof(AudioPortal));
                    memcpy(bytes[0], p, sizeof(AudioPortal));
                    size_t byte_size = sizeof(AudioPortal) + sizeof(uint8_t*) * 2;

                    if (p->adjacent_portals.buffer)
                    {

                        bytes[1] = malloc(sizeof(int) * p->adjacent_portals.capacity);
                        memcpy(bytes[1], p->adjacent_portals.buffer, sizeof(int) * p->adjacent_portals.capacity);
                        byte_size += sizeof(int) * p->adjacent_portals.capacity;
                        
                    }
                    else
                    {
                        bytes[1] = NULL;
                    }

                    push_special_undo(create_undo_structure(UNDOSTRUCT_APORTAL, (int[8]){EDITOR_SETTINGS.selected_audio_portal_id}, UNDOAPORTAL_DELETE), bytes, byte_size, 2, 0);
                    
                    remove_audio_portal(EDITOR_SETTINGS.selected_audio_portal_id);
                    EDITOR_SETTINGS.selected_audio_portal_id = GLOBALS.audio_portals.count-1;
                }
        
            
            }
            nk_tree_pop(NK_CTX);
        }
        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "APVs", NK_MINIMIZED))
        {
            
            UndoStructure apv_structure = create_undo_structure(UNDOSTRUCT_APROBE, (int[8]){EDITOR_SETTINGS.selected_audio_probe_volume_id}, UNDOSPECIAL_NONE);
            if (nk_button_label(NK_CTX, "Reset APVs"))
            {
                reset_audio_pvs();
            }
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            apv_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, aprobe_name_buffer, sizeof(aprobe_name_buffer), nk_filter_default);

            nk_layout_row_dynamic(NK_CTX, 20, 2);
            if (nk_button_label(NK_CTX, "Open APVs"))
            {

                char complete_name[256];
                strcpy(complete_name, aprobe_registry.directory);
                strncat(complete_name, aprobe_name_buffer, 255 - strlen(aprobe_registry.directory));
                complete_name[255] = '\0';
                save_aprobes(complete_name);

            }


            render_file_directory_dropdown(&aprobe_registry);

            if (nk_button_label(NK_CTX, "Load APVs"))
            {
                
                if (aprobe_registry.current_selection < aprobe_registry.names.count && aprobe_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, aprobe_registry.directory);
                    strncat(complete_name, &(((char*)(aprobe_registry.names.buffer))[aprobe_registry.current_selection * 256]), 255 - strlen(aprobe_registry.directory));
                    complete_name[255] = '\0';
                    load_aprobes(complete_name);
                }

            }

            if (GLOBALS.scene_data.APV_FILE)
            {
                nk_label(NK_CTX, GLOBALS.scene_data.APV_PATH, NK_TEXT_ALIGN_LEFT);
            }

            nk_layout_row_dynamic(NK_CTX, 20, 1);
            scene_apv_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, scene_apv_name_buffer, sizeof(scene_apv_name_buffer), nk_filter_default);

            nk_layout_row_dynamic(NK_CTX, 20, 2);
            if (nk_button_label(NK_CTX, "Open Scene APV"))
            {

                char complete_name[256];
                strcpy(complete_name, scene_apv_registry.directory);
                strncat(complete_name, scene_apv_name_buffer, 255 - strlen(scene_apv_registry.directory));
                complete_name[255] = '\0';
                // save_scene_apvs(complete_name);
                if (GLOBALS.scene_data.APV_FILE)
                {
                    fclose(GLOBALS.scene_data.APV_FILE);
                    GLOBALS.scene_data.APV_FILE = NULL;
                }
                GLOBALS.scene_data.APV_FILE = fopen(complete_name, "rb+");
                if (!GLOBALS.scene_data.APV_FILE)
                {
                    GLOBALS.scene_data.APV_FILE = fopen(complete_name, "wb+");
                }
                strcpy(GLOBALS.scene_data.APV_PATH, complete_name);

            }


            render_file_directory_dropdown(&scene_apv_registry);

            if (nk_button_label(NK_CTX, "Load Scene APV"))
            {
                
                if (scene_apv_registry.current_selection < scene_apv_registry.names.count && scene_apv_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, scene_apv_registry.directory);
                    strncat(complete_name, &(((char*)(scene_apv_registry.names.buffer))[scene_apv_registry.current_selection * 256]), 255 - strlen(scene_apv_registry.directory));
                    complete_name[255] = '\0';
                    if (GLOBALS.scene_data.APV_FILE)
                    {
                        fclose(GLOBALS.scene_data.APV_FILE);
                        GLOBALS.scene_data.APV_FILE = NULL;
                    }
                    GLOBALS.scene_data.APV_FILE = fopen(complete_name, "rb+");
                    if (!GLOBALS.scene_data.APV_FILE)
                    {
                        GLOBALS.scene_data.APV_FILE = fopen(complete_name, "wb+");
                    }
                    strcpy(GLOBALS.scene_data.APV_PATH, complete_name);
                }

            }
            if (nk_button_label(NK_CTX, "Overwrite Scene APV"))
            {
                
                if (scene_apv_registry.current_selection < scene_apv_registry.names.count && scene_apv_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, scene_apv_registry.directory);
                    strncat(complete_name, &(((char*)(scene_apv_registry.names.buffer))[scene_apv_registry.current_selection * 256]), 255 - strlen(scene_apv_registry.directory));
                    complete_name[255] = '\0';
                    if (GLOBALS.scene_data.APV_FILE)
                    {
                        fclose(GLOBALS.scene_data.APV_FILE);
                        GLOBALS.scene_data.APV_FILE = NULL;
                    }
                    GLOBALS.scene_data.APV_FILE = fopen(complete_name, "wb+");
                    strcpy(GLOBALS.scene_data.APV_PATH, complete_name);
                }

            }

            if (nk_button_label(NK_CTX, "Spawn APV"))
            {

                push_special_undo(create_undo_structure(UNDOSTRUCT_APROBE, (int[8]){EDITOR_SETTINGS.selected_audio_probe_volume_id}, UNDOAPROBE_SPAWN_APV), NULL, 0, 0, 0);

                ProbeVolume * pv = vector_expand(&GLOBALS.scene_data.audio_probe_volumes, 1);
                memset(pv, 0, sizeof(ProbeVolume));
                pv->obb.extents = (vec3){1,1,1};
                pv->resolution = DEFAULT_APV_RES;
                pv->obb.rot = mat3_identity();
                pv->byte_offset = 0;
                pv->size = 0;

                EDITOR_SETTINGS.selected_audio_probe_volume_id = GLOBALS.scene_data.audio_probe_volumes.count-1;
                
            }


            if (EDITOR_SETTINGS.selected_audio_probe_volume_id >= 0 && EDITOR_SETTINGS.selected_audio_probe_volume_id < GLOBALS.scene_data.audio_probe_volumes.count)
            {

                ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);
                ProbeVolume * pv = &pvs[EDITOR_SETTINGS.selected_audio_probe_volume_id];
                

                nk_layout_row_dynamic(NK_CTX, 20, 1);
                char pv_data[256];
                sprintf(pv_data, "APV Offset: %zu Size: %zu", pv->byte_offset, pv->size);
                nk_label(NK_CTX, pv_data, NK_TEXT_ALIGN_LEFT);

                render_obb_widget("APV", UNDOSTRUCT_APROBE, EDITOR_SETTINGS.selected_audio_probe_volume_id, &pv->obb, APROBE_MENU.OBB_sliders);

                render_ivec3_widget("APV_Res", UNDOSTRUCT_APROBE, EDITOR_SETTINGS.selected_audio_probe_volume_id, &pv->resolution, APROBE_MENU.apv_res);
                
                if (nk_button_label(NK_CTX, "Reset APV Rotation"))
                {
                    push_raw_bytes_undo(&pv->obb.rot, sizeof(mat3), apv_structure, offsetof(ProbeVolume, obb.rot));
                    pv->obb.rot = mat3_identity();
                }
                if (nk_button_label(NK_CTX, "Reset APV Size"))
                {

                    uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                    bytes[0] = malloc(sizeof(size_t));
                    *bytes[0] = pv->byte_offset;
                    bytes[1] = malloc(sizeof(size_t));
                    *bytes[1] = pv->size;

                    size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(size_t) * 2;
                    push_special_undo(create_undo_structure(UNDOSTRUCT_APROBE, (int[8]){EDITOR_SETTINGS.selected_audio_probe_volume_id}, UNDOAPROBE_RESET_APV_SIZE), bytes, byte_size, 2, 0);
                    
                    pv->byte_offset = 0;
                    pv->size = 0;
                }
                if (nk_button_label(NK_CTX, "Delete APV"))
                {
                    uint8_t ** bytes = malloc(sizeof(uint8_t*) * 1);
                    bytes[0] = malloc(sizeof(ProbeVolume));
                    memcpy(bytes[0], pv, sizeof(ProbeVolume));
                    size_t byte_size = sizeof(uint8_t*) * 1 + sizeof(ProbeVolume);

                    push_special_undo(create_undo_structure(UNDOSTRUCT_APROBE, (int[8]){EDITOR_SETTINGS.selected_audio_probe_volume_id}, UNDOAPROBE_DELETE_APV), bytes, byte_size, 1, 0);
                    

                    REMOVE_FROM_VEC(&GLOBALS.scene_data.audio_probe_volumes, EDITOR_SETTINGS.selected_audio_probe_volume_id, ProbeVolume);
                    EDITOR_SETTINGS.selected_audio_probe_volume_id = -1;
                }
                nk_bool overlay = EDITOR_SETTINGS.audio_probe_volume_overlay == 1;
                if (nk_checkbox_label(NK_CTX, "APV Overlay", &overlay))
                {
                    EDITOR_SETTINGS.audio_probe_volume_overlay = overlay;
                }

                // we dont actually undo the bake, just return byte size and offset to what they were before so you can overwrite what was saved to file
                if (nk_button_label(NK_CTX, "Bake APV"))
                {

                    uint8_t ** bytes = malloc(sizeof(uint8_t*) * 2);
                    bytes[0] = malloc(sizeof(size_t));
                    *bytes[0] = pv->byte_offset;
                    bytes[1] = malloc(sizeof(size_t));
                    *bytes[1] = pv->size;

                    size_t byte_size = sizeof(uint8_t*) * 2 + sizeof(size_t) * 2;
                    push_special_undo(create_undo_structure(UNDOSTRUCT_APROBE, (int[8]){EDITOR_SETTINGS.selected_audio_probe_volume_id}, UNDOAPROBE_RESET_APV_SIZE), bytes, byte_size, 2, 0);
                    
                    bake_audio_probe_volume(pv);

                }

            }
            
            nk_tree_pop(NK_CTX);
        }



        

    }
    nk_end(NK_CTX);
}

const char * MSAA_OPTIONS[] =
{
    "2", "4", "8", "16"
};



void render_lprobe_menu()
{
    
    if (EDITOR_SETTINGS.lprobe_menu_open)
    {
        
        if
        (
            nk_begin
            (
                NK_CTX,
                menu_names[MENU_LPROBE],
                nk_rect(420, 20, SCREEN_WIDTH - 420 * 2, 300),
                NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
            )
        )
        {

            struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
            if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }

            nk_layout_row_dynamic(NK_CTX, 20, 1);
            if (nk_button_label(NK_CTX, "Spawn LProbe Volume"))
            {

                ProbeVolume * pv = vector_expand(&GLOBALS.scene_data.lighting_probe_volumes, 1);
                memset(pv, 0, sizeof(ProbeVolume));
                pv->obb.extents = (vec3){1,1,1};
                pv->resolution = DEFAULT_LPV_RES;
                pv->obb.rot = mat3_identity();
                pv->byte_offset = 0;
                pv->size = 0;

                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_LPROBE, (int[8]) {
                    GLOBALS.scene_data.lighting_probe_volumes.count-1,
                    EDITOR_SETTINGS.selected_audio_probe_volume_id},
                    UNDOLPROBE_SPAWN),
                    NULL,
                    0,
                    0,
                    0
                );

                EDITOR_SETTINGS.selected_lighting_probe_volume_id = GLOBALS.scene_data.lighting_probe_volumes.count-1;
                
            }

            lpv_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, lprobe_name_buffer, sizeof(lprobe_name_buffer), nk_filter_default);

            nk_layout_row_dynamic(NK_CTX, 20, 2);
            if (nk_button_label(NK_CTX, "Open LPVs"))
            {

                char complete_name[256];
                strcpy(complete_name, lprobe_registry.directory);
                strncat(complete_name, lprobe_name_buffer, 255 - strlen(lprobe_registry.directory));
                complete_name[255] = '\0';
                save_lprobes(complete_name, &GLOBALS.scene_data.lighting_probe_volumes);

            }

            render_file_directory_dropdown(&lprobe_registry);

            if (nk_button_label(NK_CTX, "Load LPVs"))
            {
                
                if (lprobe_registry.current_selection < lprobe_registry.names.count && lprobe_registry.current_selection >= 0)
                {

                    char complete_name[256];
                    strcpy(complete_name, lprobe_registry.directory);
                    strncat(complete_name, &(((char*)(lprobe_registry.names.buffer))[lprobe_registry.current_selection * 256]), 255 - strlen(lprobe_registry.directory));
                    complete_name[255] = '\0';
                    load_lprobes(complete_name, &GLOBALS.scene_data.lighting_probe_volumes);

                }

            }
            if (GLOBALS.scene_data.LPV_FILE)
            {
                nk_label(NK_CTX, GLOBALS.scene_data.LPV_PATH, NK_TEXT_ALIGN_LEFT);
            }
            nk_layout_row_dynamic(NK_CTX, 20, 1);

            scene_apv_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, scene_lpv_name_buffer, sizeof(scene_lpv_name_buffer), nk_filter_default);

            nk_layout_row_dynamic(NK_CTX, 20, 2);
            if (nk_button_label(NK_CTX, "Open Scene LPV"))
            {

                char complete_name[256];
                strcpy(complete_name, scene_lpv_registry.directory);
                strncat(complete_name, scene_lpv_name_buffer, 255 - strlen(scene_lpv_registry.directory));
                complete_name[255] = '\0';
                // save_scene_lpvs(complete_name);
                if (GLOBALS.scene_data.LPV_FILE)
                {
                    fclose(GLOBALS.scene_data.LPV_FILE);
                    GLOBALS.scene_data.LPV_FILE = NULL;
                }
                GLOBALS.scene_data.LPV_FILE = fopen(complete_name, "rb+");
                if (!GLOBALS.scene_data.LPV_FILE)
                {
                    GLOBALS.scene_data.LPV_FILE = fopen(complete_name, "wb+");
                }
                strcpy(GLOBALS.scene_data.LPV_PATH, complete_name);

            }


            render_file_directory_dropdown(&scene_lpv_registry);

            if (nk_button_label(NK_CTX, "Load Scene LPV"))
            {
                
                if (scene_lpv_registry.current_selection < scene_lpv_registry.names.count && scene_lpv_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, scene_lpv_registry.directory);
                    strncat(complete_name, &(((char*)(scene_lpv_registry.names.buffer))[scene_lpv_registry.current_selection * 256]), 255 - strlen(scene_lpv_registry.directory));
                    complete_name[255] = '\0';
                    if (GLOBALS.scene_data.LPV_FILE)
                    {
                        fclose(GLOBALS.scene_data.LPV_FILE);
                        GLOBALS.scene_data.LPV_FILE = NULL;
                    }
                    GLOBALS.scene_data.LPV_FILE = fopen(complete_name, "rb+");
                    if (!GLOBALS.scene_data.LPV_FILE)
                    {
                        GLOBALS.scene_data.LPV_FILE = fopen(complete_name, "wb+");
                    }
                    strcpy(GLOBALS.scene_data.LPV_PATH, complete_name);
                }

            }
            if (nk_button_label(NK_CTX, "Overwrite Scene LPV"))
            {
                
                if (scene_lpv_registry.current_selection < scene_lpv_registry.names.count && scene_lpv_registry.current_selection >= 0)
                {
                    char complete_name[256];
                    strcpy(complete_name, scene_lpv_registry.directory);
                    strncat(complete_name, &(((char*)(scene_lpv_registry.names.buffer))[scene_lpv_registry.current_selection * 256]), 255 - strlen(scene_lpv_registry.directory));
                    complete_name[255] = '\0';
                    if (GLOBALS.scene_data.LPV_FILE)
                    {
                        fclose(GLOBALS.scene_data.LPV_FILE);
                        GLOBALS.scene_data.LPV_FILE = NULL;
                    }
                    GLOBALS.scene_data.LPV_FILE = fopen(complete_name, "wb+");
                    strcpy(GLOBALS.scene_data.LPV_PATH, complete_name);
                }

            }
            if (nk_button_label(NK_CTX, "Reset LPVs"))
            {

                // im assuming that the lighting pv flag is equivalent to if there is a buffer loaded, therefore we don't need to do any more byte buffer shit
                uint8_t ** bytes = malloc(sizeof(uint8_t*) * LPV_SLOTS);
                size_t total_bytes = sizeof(uint8_t*) * LPV_SLOTS;
                for (int i = 0; i < LPV_SLOTS; i++)
                {
                    if (GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes)
                    {
                        
                        size_t size = sizeof(float) * LPV_PADDED_COUNT * 3 * SH_MAX;
                        bytes[i] = malloc(size);
                        memcpy(bytes[i], GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes, size);
                        total_bytes += size;

                    }
                    else
                    {
                        bytes[i] = NULL;
                    }
                }
                push_special_undo(
                create_undo_structure(UNDOSTRUCT_LPROBE,
                                      (int[8])
                                      {
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[0].pv_id,
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[1].pv_id,
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[2].pv_id,
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[3].pv_id,
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[4].pv_id,
                                          GLOBALS.scene_data.LOADED_LIGHTING_PV[5].pv_id,
                                      }, UNDOLPROBE_RESET_LPVS), bytes, total_bytes, LPV_SLOTS, 0);
                reset_lighting_pvs();
            }


            if (EDITOR_SETTINGS.selected_lighting_probe_volume_id >= 0 && EDITOR_SETTINGS.selected_lighting_probe_volume_id < GLOBALS.scene_data.lighting_probe_volumes.count)
            {

                ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);
                ProbeVolume * pv = &pvs[EDITOR_SETTINGS.selected_lighting_probe_volume_id];
                
                nk_layout_row_dynamic(NK_CTX, 20, 1);
                char pv_data[256];
                sprintf(pv_data, "LPV Offset: %zu Size: %zu", pv->byte_offset, pv->size);
                nk_label(NK_CTX, pv_data, NK_TEXT_ALIGN_LEFT);

                // render_obb_widget("LPV", &pv->obb);
                render_obb_widget("LPV", UNDOSTRUCT_LPROBE, EDITOR_SETTINGS.selected_lighting_probe_volume_id, &pv->obb, LPROBE_MENU.OBB_sliders);

                if (nk_button_label(NK_CTX, "Reset LPV Rotation"))
                {
                    push_raw_bytes_undo(&pv->obb.rot, sizeof(mat3), create_undo_structure(UNDOSTRUCT_LPROBE, (int[8]){EDITOR_SETTINGS.selected_lighting_probe_volume_id}, UNDOSPECIAL_NONE), offsetof(ProbeVolume, obb.rot));
                    pv->obb.rot = mat3_identity();
                }
                if (nk_button_label(NK_CTX, "Reset LPV Size"))
                {
                    push_special_undo(create_undo_structure(UNDOSTRUCT_LPROBE, (int[8]){EDITOR_SETTINGS.selected_lighting_probe_volume_id, pv->byte_offset, pv->size}, UNDOLPROBE_RESET_LPV_SIZE), NULL, 0, 0, 0);
                    pv->byte_offset = 0;
                    pv->size = 0;
                }
                if (nk_button_label(NK_CTX, "Delete LProbe Volume"))
                {

                    uint8_t ** bytes = malloc(sizeof(uint8_t*));
                    bytes[0] = malloc(sizeof(ProbeVolume));
                    memcpy(bytes[0], pv, sizeof(ProbeVolume));
                    size_t byte_size = sizeof(uint8_t*) + sizeof(ProbeVolume);
                    int byte_count = 1;
                    
                    push_special_undo(create_undo_structure(UNDOSTRUCT_LPROBE, (int[8]){EDITOR_SETTINGS.selected_lighting_probe_volume_id}, UNDOLPROBE_DELETE), bytes, byte_size, byte_count, 0);

                    REMOVE_FROM_VEC(&GLOBALS.scene_data.lighting_probe_volumes, EDITOR_SETTINGS.selected_lighting_probe_volume_id, ProbeVolume);
                    EDITOR_SETTINGS.selected_lighting_probe_volume_id = -1;
                }
                nk_bool overlay = EDITOR_SETTINGS.lighting_probe_volume_overlay == 1;
                if (nk_checkbox_label(NK_CTX, "LPV Overlay", &overlay))
                {
                    EDITOR_SETTINGS.lighting_probe_volume_overlay = overlay;
                }
                if (nk_button_label(NK_CTX, "Bake LPV"))
                {
                    bake_lighting_probe_volume(pv);
                }

            }
            
        }
        nk_end(NK_CTX);
    }
}



void render_main_menu()
{
    
    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_MAIN],
            nk_rect(420, 20, SCREEN_WIDTH - 420 * 2, 300),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }

        nk_layout_row_dynamic(NK_CTX, 20, 4);


        nk_text(NK_CTX, "Scene Name", 10, NK_TEXT_ALIGN_LEFT);
        
        char scene_name_buffer[256];
        
        scene_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, scene_name_buffer, sizeof(scene_name_buffer), nk_filter_default);

        if (nk_button_text(NK_CTX, "Change Name", 11))
        {

            push_raw_bytes_undo(GLOBALS.scene_data.name, sizeof(char) * 256, create_undo_structure(UNDOSTRUCT_GLOBAL, NULL, UNDOSPECIAL_NONE), offsetof(Globals, scene_data.name));

            strcpy(GLOBALS.scene_data.name, scene_name_buffer);
        }


        nk_layout_row_dynamic(NK_CTX, 20, 3);
        char file_name_buffer[256];
        scene_file_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, file_name_buffer, sizeof(file_name_buffer), nk_filter_default);
        if (nk_button_label(NK_CTX, "Save Scene"))
        {
            scene_save(file_name_buffer);
        }
        if (nk_button_label(NK_CTX, "Save Scene Volumes"))
        {
            scene_save_volumes(file_name_buffer);
        }
        nk_layout_row_dynamic(NK_CTX, 20, 3);
        render_file_directory_dropdown(&scene_registry);
        if (nk_button_label(NK_CTX, "Load Scene"))
        {
            if (scene_registry.current_selection < scene_registry.names.count && scene_registry.current_selection >= 0)
            {
                char complete_name[256];
                strcpy(complete_name, &(((char*)(scene_registry.names.buffer))[scene_registry.current_selection * 256]));
                // remove bin
                char * bin = strstr(complete_name, ".bin");
                if (bin) *bin = '\0';
                scene_load(complete_name);
            }
        }
        if (nk_button_label(NK_CTX, "Load Scene Volumes"))
        {
            if (scene_registry.current_selection < scene_registry.names.count && scene_registry.current_selection >= 0)
            {
                char complete_name[256];
                strcpy(complete_name, &(((char*)(scene_registry.names.buffer))[scene_registry.current_selection * 256]));
                // remove bin
                char * bin = strstr(complete_name, ".bin");
                if (bin) *bin = '\0';
                scene_load_volumes(complete_name);
            }
        }

        nk_layout_row_dynamic(NK_CTX, 20, 3);
        char ui_buffer[256];
        ui_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, ui_buffer, sizeof(ui_buffer), nk_filter_default);
        if (nk_button_label(NK_CTX, "Save UI"))
        {
            save_ui(ui_buffer);
        }

        render_file_directory_dropdown(&ui_registry);

        if (nk_button_label(NK_CTX, "Load UI"))
        {
            if (ui_registry.current_selection < ui_registry.names.count && ui_registry.current_selection >= 0)
            {
                char complete_name[256];
                strcpy(complete_name, &(((char*)(ui_registry.names.buffer))[ui_registry.current_selection * 256]));
                char * bin = strstr(complete_name, ".bin");
                if (bin) *bin = '\0';
                load_ui(complete_name);
            }
        }

        nk_layout_row_dynamic(NK_CTX, 20, 4);
        ComboLabel gltf_label;
        gltf_label.item_count = 0;
        gltf_label.selected = 0;
        nk_bool gltf_names_expanded = nk_false;
        if (nk_combo_begin_label(NK_CTX, "Expand", nk_vec2(200, 200))) {

            if (!gltf_file_names_open)
            {
                load_gltf_filenames(asset_dir);
            }
            for (int i = 0; i < gltf_names_count; i++)
            {
                snprintf(gltf_label.buffer[gltf_label.item_count], LABEL_LEN, "%s", gltf_names[i]);
                gltf_label.ptrs[gltf_label.item_count] = gltf_label.buffer[gltf_label.item_count];
                if (EDITOR_SETTINGS.selected_gltf_file_id == gltf_label.item_count) gltf_label.selected = gltf_label.item_count;
                gltf_label.item_count++;

            }
            gltf_file_names_open = nk_true;
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            for (int i = 0; i < gltf_label.item_count; i++) {
                nk_bool sel = (i == gltf_label.selected);
                if (nk_selectable_label(NK_CTX, gltf_label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                    EDITOR_SETTINGS.selected_gltf_file_id = i;
                }
            }
            nk_combo_end(NK_CTX);
        }
        else
        {
            gltf_file_names_open = nk_false;
        }
        
        
        if (nk_button_label(NK_CTX, "Load Asset"))
        {
            load_gltf_filenames(asset_dir);
            if (EDITOR_SETTINGS.selected_gltf_file_id < gltf_names_count)
            {
                char complete_name[256];
                strcpy(complete_name, asset_dir);
                strncat(complete_name, gltf_names[EDITOR_SETTINGS.selected_gltf_file_id], 255 - strlen(asset_dir));
                load_gltf(complete_name);
                
            }
        }

        ComboLabel asset_label;
        asset_label.item_count = 0;
        asset_label.selected = 0;
        Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
        for (int i = 0; i < GLOBALS.assets.count; i++)
        {
            snprintf(asset_label.buffer[asset_label.item_count], LABEL_LEN, "%s", assets[i].path);
            asset_label.ptrs[asset_label.item_count] = asset_label.buffer[asset_label.item_count];
            if (EDITOR_SETTINGS.selected_asset_id == asset_label.item_count) asset_label.selected = asset_label.item_count;
            asset_label.item_count++;

        }
        if (nk_combo_begin_label(NK_CTX, asset_label.ptrs[asset_label.selected], nk_vec2(200, 200))) {

            gltf_file_names_open = nk_true;
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            for (int i = 0; i < asset_label.item_count; i++) {
                nk_bool sel = (i == asset_label.selected);
                if (nk_selectable_label(NK_CTX, asset_label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                    EDITOR_SETTINGS.selected_asset_id = i;
                }
            }
            nk_combo_end(NK_CTX);
        }
        if (nk_button_label(NK_CTX, "Spawn Instance"))
        {

            AssetInstance * ai = init_new_asset_instance(asset_label.ptrs[EDITOR_SETTINGS.selected_asset_id], (vec3){0,0,0}, (vec4){0,0,0,1}, (vec3){1,1,1});

            push_special_undo(create_undo_structure(UNDOSTRUCT_GLOBAL, (int[8]){ai->id}, UNDOGLOBAL_SPAWN_ASSET_INSTANCE), NULL, 0, 0, 0);
            
            VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
            init_scene_bvh();
            upload_material_data();
            GPU_upload_scene_data();
            upload_lights();
            MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
            Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
            init_lightmap_data(&GLOBALS.scene_data.lightmap_data);
            for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
            {
                if (mns[i].mesh == -1 || mns[i].free) continue;
                if (!meshes[mns[i].mesh].static_mesh || !mns[i].static_mesh || mns[i].physics_mode) continue;
        
                build_uv_islands(&mns[i]);    
            }
            add_all_meshes_to_buffer(vl);
            draw_dirty = true;
            

        }

        char rprobe_name_buffer[256];
        rprobe_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, rprobe_name_buffer, sizeof(rprobe_name_buffer), nk_filter_default);
        if (nk_button_text(NK_CTX, "Save RProbes", 12))
        {

            char complete_name[256];
            strcpy(complete_name, rprobe_dir);
            strncat(complete_name, rprobe_name_buffer, 255 - strlen(rprobe_dir));
            complete_name[255] = '\0';
            save_vector(complete_name, &GLOBALS.scene_data.reflection_probes);

        }
        ComboLabel rprobe_label;
        rprobe_label.item_count = 0;
        rprobe_label.selected = 0;
        nk_bool rprobe_names_expanded = nk_false;
        if (nk_combo_begin_label(NK_CTX, "Expand", nk_vec2(200, 200))) {

            if (!rprobe_file_names_open)
            {
                load_rprobe_filenames(rprobe_dir);
            }
            for (int i = 0; i < rprobe_names_count; i++)
            {
                snprintf(rprobe_label.buffer[rprobe_label.item_count], LABEL_LEN, "%s", rprobe_names[i]);
                rprobe_label.ptrs[rprobe_label.item_count] = rprobe_label.buffer[rprobe_label.item_count];
                if (EDITOR_SETTINGS.selected_rprobe_file_id == rprobe_label.item_count) rprobe_label.selected = rprobe_label.item_count;
                rprobe_label.item_count++;

            }
            rprobe_file_names_open = nk_true;
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            for (int i = 0; i < rprobe_label.item_count; i++) {
                nk_bool sel = (i == rprobe_label.selected);
                if (nk_selectable_label(NK_CTX, rprobe_label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                    EDITOR_SETTINGS.selected_rprobe_file_id = i;
                }
            }
            nk_combo_end(NK_CTX);
        }
        else
        {
            rprobe_file_names_open = nk_false;
        }
        if (nk_button_text(NK_CTX, "Load RProbes", 12))
        {
            load_rprobe_filenames(rprobe_dir);
            if (EDITOR_SETTINGS.selected_rprobe_file_id < rprobe_names_count)
            {
                char complete_name[256];
                strcpy(complete_name, rprobe_dir);
                strncat(complete_name, rprobe_names[EDITOR_SETTINGS.selected_rprobe_file_id], 255 - strlen(rprobe_dir));
                complete_name[255] = '\0';
                load_rprobes(complete_name, &GLOBALS.scene_data.reflection_probes);
                
            }
        }

        
        int view = EDITOR_SETTINGS.current_debug_view;
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        render_dropdown("Debug Views", &view, (char**)DVIEW_NAMES, DVIEW_MAX, 300, 100);
        if (view != EDITOR_SETTINGS.current_debug_view)
        {
            EDITOR_SETTINGS.current_debug_view = view;
        }

        int ppview = EDITOR_SETTINGS.pp_dview;
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        render_dropdown("PP Debug Views", &ppview, (char**)PPDVIEW_NAMES, PPDVIEW_MAX, 300, 100);
        if (ppview != EDITOR_SETTINGS.pp_dview)
        {
            EDITOR_SETTINGS.pp_dview = ppview;
        }

        nk_layout_row_dynamic(NK_CTX, 20, 4);
        nk_bool ssao = EDITOR_SETTINGS.SSAO == 1;
        if (nk_checkbox_label(NK_CTX, "SSAO", &ssao))
        {
            EDITOR_SETTINGS.SSAO = ssao;
        }
        nk_bool ssr = EDITOR_SETTINGS.SSR == 1;
        if (nk_checkbox_label(NK_CTX, "SSR", &ssr))
        {
            EDITOR_SETTINGS.SSR = ssr;
        }
        nk_bool dof = EDITOR_SETTINGS.DOF == 1;
        if (nk_checkbox_label(NK_CTX, "DOF", &dof))
        {
            EDITOR_SETTINGS.DOF = dof;
        }

        nk_bool mblur = EDITOR_SETTINGS.MBLUR == 1;
        if (nk_checkbox_label(NK_CTX, "MBLUR", &mblur))
        {
            EDITOR_SETTINGS.MBLUR = mblur;
        }
        nk_bool noise = EDITOR_SETTINGS.NOISE == 1;
        if (nk_checkbox_label(NK_CTX, "NOISE", &noise))
        {
            EDITOR_SETTINGS.NOISE = noise;
        }
        nk_bool aces = EDITOR_SETTINGS.ACES == 1;
        if (nk_checkbox_label(NK_CTX, "ACES", &aces))
        {
            EDITOR_SETTINGS.ACES = aces;
        }
        nk_bool lut = EDITOR_SETTINGS.LUT == 1;
        if (nk_checkbox_label(NK_CTX, "LUT", &lut))
        {
            EDITOR_SETTINGS.LUT = lut;
        }

        nk_bool taa = EDITOR_SETTINGS.TAA == 1;
        if (nk_checkbox_label(NK_CTX, "TAA", &taa))
        {
            EDITOR_SETTINGS.TAA = taa;
        }
        nk_bool msaa = EDITOR_SETTINGS.MSAA == 1;
        if (nk_checkbox_label(NK_CTX, "MSAA", &msaa))
        {
            EDITOR_SETTINGS.MSAA = msaa;
            if (EDITOR_SETTINGS.MSAA)
            {
                EDITOR_SETTINGS.FULL_GBUFFER = false;
                allocate_gbuffer_msaa();
                allocate_main_fbo();
                glBindFramebuffer(GL_FRAMEBUFFER, MAIN_FBO.fbo);
                glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GBUFFER_FBO.depth_rbo.i);
                
            }
            else
            {
                
                glBindFramebuffer(GL_FRAMEBUFFER, MAIN_RESOLVE_FBO.fbo);
                glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.depth_texture.i, 0);
            }
            // else
            // {
            //     allocate_gbuffer();
            // }
        }
        nk_bool full_gbuffer = EDITOR_SETTINGS.FULL_GBUFFER == 1;
        if (nk_checkbox_label(NK_CTX, "FULL_GBUFFER", &full_gbuffer))
        {
            EDITOR_SETTINGS.FULL_GBUFFER = full_gbuffer;
            if (full_gbuffer)
            {
                EDITOR_SETTINGS.MSAA = false;
                allocate_gbuffer();
            }
            else
            {
                allocate_gbuffer_msaa();
            }
        }


        nk_bool bloom = EDITOR_SETTINGS.BLOOM == 1;
        if (nk_checkbox_label(NK_CTX, "BLOOM", &bloom))
        {
            EDITOR_SETTINGS.BLOOM = bloom;
        }
        nk_bool vsync = EDITOR_SETTINGS.VSYNC == 1;
        if (nk_checkbox_label(NK_CTX, "VSYNC", &vsync))
        {
            EDITOR_SETTINGS.VSYNC = vsync;
            SDL_GL_SetSwapInterval(vsync);
        }
        nk_bool ui = EDITOR_SETTINGS.UI == 1;
        if (nk_checkbox_label(NK_CTX, "UI", &ui))
        {
            EDITOR_SETTINGS.UI = ui;
        }

        int sample_sel;
        switch(GLOBALS.msaa_samples)
        {
            case 2:
                sample_sel = 0;
                break;
            case 4:
                sample_sel = 1;
                break;
            case 8:
                sample_sel = 2;
                break;
            case 16:
                sample_sel = 3;
                break;
            default:
                sample_sel = 0;
                break;
        }

        render_dropdown("MSAA Samples", &sample_sel, (char**)MSAA_OPTIONS, MSAA_SAMPLE_OPTIONS_MAX, 300, 100);
        switch(sample_sel)
        {
            case 0:
                GLOBALS.msaa_samples = 2;
                break;
            case 1:
                GLOBALS.msaa_samples = 4;
                break;
            case 2:
                GLOBALS.msaa_samples = 8;
                break;
            case 3:
                GLOBALS.msaa_samples = 16;
                break;
            default:
                GLOBALS.msaa_samples = 2;
                break;
        }




        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        float render_frame = nk_propertyf(NK_CTX, "Render framerate", 0.0f, EDITOR_SETTINGS.render_framerate_target, 120.0f, 0.01f, 0.01f);
        if (render_frame != EDITOR_SETTINGS.render_framerate_target)
        {
            EDITOR_SETTINGS.render_framerate_target = render_frame;
        }
        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "Resolution", NK_MINIMIZED))
        {
            nk_layout_row_dynamic(NK_CTX, 20, 2);

            int res_width = nk_propertyi(NK_CTX, "Res Width", 0, EDITOR_SETTINGS.res_width, 10000, 1, 1);

            if (check_equivalencyi(&EDITOR_SETTINGS.res_width, res_width, &MAIN_MENU.res_width, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.res_width, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, res_width));
            }

            int res_height = nk_propertyi(NK_CTX, "Res Height", 0, EDITOR_SETTINGS.res_height, 10000, 1, 1);
            if (check_equivalencyi(&EDITOR_SETTINGS.res_height, res_height, &MAIN_MENU.res_height, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.res_height, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, res_height));
            }


            nk_layout_row_dynamic(NK_CTX, 20, 2);
            int selected_fs = EDITOR_SETTINGS.selected_fullscreen_option;
            render_dropdown("Fullscreen Options", &EDITOR_SETTINGS.selected_fullscreen_option, (char**)FULLSCREEN_OPTION_NAMES, FSOPT_MAX, 300, 100);

            if (nk_button_label(NK_CTX, "Change FS"))
            {

                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){EDITOR_SETTINGS.current_fullscreen_option},
                     UNDOEDITORSETTINGS_FULLSCREENOPT),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );
                
                change_fullscreen_option(EDITOR_SETTINGS.selected_fullscreen_option);
            }
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            if (nk_button_label(NK_CTX, "Screen"))
            {

                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){SCREEN_WIDTH, SCREEN_HEIGHT},
                     UNDOEDITORSETTINGS_SCREEN_RES),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );

                SCREEN_WIDTH = res_width;
                SCREEN_HEIGHT = res_height;

                SDL_SetWindowSize(window, SCREEN_WIDTH, SCREEN_HEIGHT);
            }
            char render_res[64];
            sprintf(render_res, "%dx%d", RENDER_WIDTH, RENDER_HEIGHT);
            nk_label(NK_CTX, render_res, NK_TEXT_ALIGN_LEFT);
            if (nk_button_label(NK_CTX, "Render"))
            {
                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){RENDER_WIDTH, RENDER_HEIGHT},
                     UNDOEDITORSETTINGS_RENDER_RES),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );

                RENDER_WIDTH = res_width;
                RENDER_HEIGHT = res_height;
                change_render_resolution();
            }
            char ssao_res[64];
            sprintf(ssao_res, "%dx%d", SSAO_FBO.w, SSAO_FBO.h);
            nk_label(NK_CTX, ssao_res, NK_TEXT_ALIGN_LEFT);
            if (nk_button_label(NK_CTX, "SSAO"))
            {
            
                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){SSAO_WIDTH, SSAO_HEIGHT},
                     UNDOEDITORSETTINGS_SSAO_RES),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );
                change_ssao_res(EDITOR_SETTINGS.res_width, EDITOR_SETTINGS.res_height);
            }
            char coc_res[64];
            sprintf(coc_res, "%dx%d", COC_FBO.w, COC_FBO.h);
            nk_label(NK_CTX, coc_res, NK_TEXT_ALIGN_LEFT);
            if (nk_button_label(NK_CTX, "DOF"))
            {
                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){DOF_WIDTH, DOF_HEIGHT},
                     UNDOEDITORSETTINGS_DOF_RES),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );
                change_dof_res(EDITOR_SETTINGS.res_width, EDITOR_SETTINGS.res_height);
            }
            char bloom_res[64];
            sprintf(bloom_res, "%dx%d", BLOOM_PYRAMID[0].w, BLOOM_PYRAMID[0].h);
            nk_label(NK_CTX, bloom_res, NK_TEXT_ALIGN_LEFT);
            if (nk_button_label(NK_CTX, "BLOOM"))
            {
            
                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, (int[8]){BLOOM_WIDTH, BLOOM_HEIGHT},
                    UNDOEDITORSETTINGS_BLOOM_RES),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );
                change_bloom_res(EDITOR_SETTINGS.res_width, EDITOR_SETTINGS.res_height);
            }
            if (nk_button_label(NK_CTX, "Make Effects Halfsize"))
            {
                push_special_undo
                (
                    create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS,
                      (int[8]){
                          SSAO_WIDTH, SSAO_HEIGHT,
                          DOF_WIDTH, DOF_HEIGHT,
                          BLOOM_WIDTH, BLOOM_HEIGHT
                      },
                    UNDOEDITORSETTINGS_HALFSIZE_EFFECTS),
                    NULL,
                    0,
                    0,
                    offsetof(EditorSettings, selected_fullscreen_option)
                );
                int half_width = RENDER_WIDTH / 2;
                int half_height = RENDER_HEIGHT / 2;
                change_ssao_res(half_width, half_height);
                change_dof_res(half_width, half_height);
                change_bloom_res(half_width, half_height);

            }
            nk_tree_pop(NK_CTX);
        }

        if (nk_tree_push(NK_CTX, NK_TREE_NODE, "DOF Settings", NK_MINIMIZED))
        {
            
            nk_layout_row_dynamic(NK_CTX, 20, 2);
            
            float flength = nk_propertyf(NK_CTX, "Focal Length", -5.0, EDITOR_SETTINGS.DOF_focal_length, 5.0f, 0.001f, 0.001f);
            if (check_equivalencyf(&EDITOR_SETTINGS.DOF_focal_length, flength, &MAIN_MENU.focal_length, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.focal_length, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, DOF_focal_length));
            }
            float fstop = nk_propertyf(NK_CTX, "FStop", -5.0, EDITOR_SETTINGS.DOF_fstop, 10.0f, 0.001f, 0.001f);
            if (check_equivalencyf(&EDITOR_SETTINGS.DOF_fstop, fstop, &MAIN_MENU.fstop, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.fstop, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, DOF_fstop));
            }
            float sheight = nk_propertyf(NK_CTX, "Sensor Height", -5.0, EDITOR_SETTINGS.DOF_sensor_height, 5.0f, 0.0001f, 0.0001f);
            if (check_equivalencyf(&EDITOR_SETTINGS.DOF_sensor_height, sheight, &MAIN_MENU.sensor_height, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.sensor_height, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, DOF_sensor_height));
            }
            float mblurradius = nk_propertyf(NK_CTX, "Blur Radius", -5.0, EDITOR_SETTINGS.DOF_max_blur_radius, 5.0f, 0.001f, 0.001f);
            if (check_equivalencyf(&EDITOR_SETTINGS.DOF_max_blur_radius, mblurradius, &MAIN_MENU.blur_radius, NULL))
            {
                push_nk_slider_undo(&MAIN_MENU.blur_radius, create_undo_structure(UNDOSTRUCT_EDITOR_SETTINGS, NULL, UNDOSPECIAL_NONE), offsetof(EditorSettings, DOF_max_blur_radius));
            }

            nk_tree_pop(NK_CTX);
        }
        
    }
    nk_end(NK_CTX);

}



void render_mn_menu()
{
    
    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_MN],
            nk_rect(20, 20, 400, SCREEN_HEIGHT - 120),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {
        bool dirty = false;

        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }



        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        MeshNode * mn = &mns[EDITOR_SETTINGS.selected_mn_id];
        nk_layout_row_dynamic(NK_CTX, 20, 1);


        char idx[256];
        sprintf(idx, "%zu", mn->id);
        nk_label(NK_CTX, idx, NK_TEXT_ALIGN_LEFT);

        render_mat4_widget("MN", UNDOSTRUCT_MN, EDITOR_SETTINGS.selected_mn_id, &mn->global_matrix, &mn->translation, &mn->rotation, &mn->scale, MN_MENU.model);
        

        Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
        Mesh * m = &meshes[mn->mesh];

        SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);
        Material * mats = CAST_VEC(GLOBALS.materials, Material);


        ComboLabel sm_label;
        sm_label.item_count = 0;
        sm_label.selected = 0;
        for (int i = 0; i < m->submeshes.count; i++)
        {
            snprintf(sm_label.buffer[sm_label.item_count], LABEL_LEN, "%d", i);
            sm_label.ptrs[sm_label.item_count] = sm_label.buffer[sm_label.item_count];
            if (EDITOR_SETTINGS.selected_sm_id == sm_label.item_count) sm_label.selected = sm_label.item_count;
            sm_label.item_count++;

        }
        if (nk_combo_begin_label(NK_CTX, sm_label.ptrs[sm_label.selected], nk_vec2(200, 200))) {
            nk_layout_row_dynamic(NK_CTX, 15, 1);
            for (int i = 0; i < sm_label.item_count; i++) {
                nk_bool sel = (i == sm_label.selected);
                if (nk_selectable_label(NK_CTX, sm_label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                    EDITOR_SETTINGS.selected_sm_id = i;
                    EDITOR_SETTINGS.selected_material_mn_id = sms[i].material;
                }
            }
            nk_combo_end(NK_CTX);
        }

        ComboLabel mat_label;
        mat_label.item_count = 0;
        mat_label.selected = 0;
        SubMesh * sm = &sms[EDITOR_SETTINGS.selected_sm_id];
        UndoStructure mn_st = create_undo_structure(UNDOSTRUCT_MN, (int[8]){EDITOR_SETTINGS.selected_mn_id, EDITOR_SETTINGS.selected_sm_id}, UNDOSPECIAL_NONE);
        bool found = false;
        for (int i = 0; i < GLOBALS.materials.count; i++)
        {
            
            Material * mat = &mats[i];
            snprintf(mat_label.buffer[mat_label.item_count], LABEL_LEN, "%s", mats[i].name);
            mat_label.ptrs[mat_label.item_count] = mat_label.buffer[mat_label.item_count];
            if (EDITOR_SETTINGS.selected_material_mn_id == mat_label.item_count) mat_label.selected = mat_label.item_count;
            mat_label.item_count++;
        }
            
            
        int selection = -1;
        
        if (nk_combo_begin_label(NK_CTX, mat_label.ptrs[mat_label.selected], nk_vec2(200, 200))) {
            nk_layout_row_dynamic(NK_CTX, 15, 1);
            for (int i = 0; i < mat_label.item_count; i++) {
                nk_bool sel = (i == mat_label.selected);
                if (nk_selectable_label(NK_CTX, mat_label.ptrs[i], NK_TEXT_LEFT, &sel)) {
                    EDITOR_SETTINGS.selected_material_mn_id = i;
                }
            }
            nk_combo_end(NK_CTX);
        }


        

        if (nk_button_text(NK_CTX, "Change Material", 15))
        {


            push_special_undo(create_undo_structure(UNDOSTRUCT_MN, (int[8]){EDITOR_SETTINGS.selected_mn_id, EDITOR_SETTINGS.selected_sm_id, sm->material}, UNDOMN_CHANGE_MATERIAL), NULL, 0, 0, 0);
            
            sm->material = EDITOR_SETTINGS.selected_material_mn_id;            


            
            
            draw_dirty = true;
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        if (nk_button_text(NK_CTX, "Open Material", 8))
        {
            EDITOR_SETTINGS.opening_menu = true;
            EDITOR_SETTINGS.material_menu_open = true;
            nk_window_show(NK_CTX, "Material", NK_SHOWN);
        }

        if (nk_option_label(NK_CTX, "Skip Directional Light", mn->skip_directional_uv_lighting == 1))
        {
            mn->skip_directional_uv_lighting = true;
        }
        else
        {
            
            mn->skip_directional_uv_lighting = false;
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        if (mn->ent)
        {
            Entity * ent = mn->ent;
            if (!ent->free)
            {


                if (nk_button_label(NK_CTX, "Select Entity MN"))
                {
                    EDITOR_SETTINGS.selected_ent_id = ent->id;
                    EDITOR_SETTINGS.entity_menu_open = true;
                    EDITOR_SETTINGS.opening_menu = true;
                }

                
            }
        }

        if (dirty)
        {
            mn->global_matrix = compose_trs_from_mesh_node(mn);
        }

    }
    nk_end(NK_CTX);
}

void render_material_menu()
{
    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_MATERIAL],
            nk_rect(SCREEN_WIDTH - 420, 20, 400, SCREEN_HEIGHT),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {

        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) {
            NUKLEAR_HOVERED = true;
        }

        Material * mats = CAST_VEC(GLOBALS.materials, Material);
        Material * mat = &mats[EDITOR_SETTINGS.selected_material_mn_id];


        bool dirty = false;
        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        struct nk_colorf diffuse_color = {
            mat->kd.x,
            mat->kd.y,
            mat->kd.z,
            mat->kd.w,
        };
        nk_label(NK_CTX, "Diffuse", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgba_cf(diffuse_color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            diffuse_color = nk_color_picker(NK_CTX, diffuse_color, NK_RGBA);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            diffuse_color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, diffuse_color.r, 1.0f, 0.01f, 0.005f);
            diffuse_color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, diffuse_color.g, 1.0f, 0.01f, 0.005f);
            diffuse_color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, diffuse_color.b, 1.0f, 0.01f, 0.005f);
            diffuse_color.a = nk_propertyf(NK_CTX, "#A:", 0.0f, diffuse_color.a, 1.0f, 0.01f, 0.005f);

            if (diffuse_color.r != mat->kd.x)
            {
                mat->kd.x = diffuse_color.r;
                dirty = true;
            }
            if (diffuse_color.g != mat->kd.y)
            {
                mat->kd.y = diffuse_color.g;
                dirty = true;
            }
            if (diffuse_color.b != mat->kd.z)
            {
                mat->kd.z = diffuse_color.b;
                dirty = true;
            }
            if (diffuse_color.a != mat->kd.w)
            {
                mat->kd.w = diffuse_color.a;
                dirty = true;
            }
            nk_combo_end(NK_CTX);
        }
        struct nk_colorf ambient_color = {
            mat->ka.x,
            mat->ka.y,
            mat->ka.z,
            1.0f
        };
        nk_label(NK_CTX, "Ambient", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgb_cf(ambient_color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            ambient_color = nk_color_picker(NK_CTX, ambient_color, NK_RGB);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            ambient_color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, ambient_color.r, 1.0f, 0.01f, 0.005f);
            ambient_color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, ambient_color.g, 1.0f, 0.01f, 0.005f);
            ambient_color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, ambient_color.b, 1.0f, 0.01f, 0.005f);

            if (ambient_color.r != mat->ka.x)
            {
                mat->ka.x = ambient_color.r;
                dirty = true;
            }
            if (ambient_color.g != mat->ka.y)
            {
                mat->ka.y = ambient_color.g;
                dirty = true;
            }
            if (ambient_color.b != mat->ka.z)
            {
                mat->ka.z = ambient_color.b;
                dirty = true;
            }
            nk_combo_end(NK_CTX);
        }
        struct nk_colorf specular_color = {
            mat->ks.x,
            mat->ks.y,
            mat->ks.z,
            1.0f
        };
        nk_label(NK_CTX, "Specular", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgb_cf(specular_color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            specular_color = nk_color_picker(NK_CTX, specular_color, NK_RGB);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            specular_color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, specular_color.r, 1.0f, 0.01f, 0.005f);
            specular_color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, specular_color.g, 1.0f, 0.01f, 0.005f);
            specular_color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, specular_color.b, 1.0f, 0.01f, 0.005f);

            if (specular_color.r != mat->ks.x)
            {
                mat->ks.x = specular_color.r;
                dirty = true;
            }
            if (specular_color.g != mat->ks.y)
            {
                mat->ks.y = specular_color.g;
                dirty = true;
            }
            if (specular_color.b != mat->ks.z)
            {
                mat->ks.z = specular_color.b;
                dirty = true;
            }
            nk_combo_end(NK_CTX);
        }

        struct nk_colorf emissive_color = {
            mat->ke.x,
            mat->ke.y,
            mat->ke.z,
            1.0f
        };
        nk_label(NK_CTX, "Emissive", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgb_cf(emissive_color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            emissive_color = nk_color_picker(NK_CTX, emissive_color, NK_RGB);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            emissive_color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, emissive_color.r, 1.0f, 0.01f, 0.005f);
            emissive_color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, emissive_color.g, 1.0f, 0.01f, 0.005f);
            emissive_color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, emissive_color.b, 1.0f, 0.01f, 0.005f);

            if (emissive_color.r != mat->ke.x)
            {
                mat->ke.x = emissive_color.r;
                dirty = true;
            }
            if (emissive_color.g != mat->ke.y)
            {
                mat->ke.y = emissive_color.g;
                dirty = true;
            }
            if (emissive_color.b != mat->ke.z)
            {
                mat->ke.z = emissive_color.b;
                dirty = true;
            }
            nk_combo_end(NK_CTX);
        }


        nk_layout_row_dynamic(NK_CTX, 20, 1);
        nk_bool arm_override = mat->override_arm == 1;
        if (nk_checkbox_label(NK_CTX, "Arm Override", &arm_override))
        {
            dirty = true;
            mat->override_arm = arm_override;
        }
        
        float metallic = nk_propertyf(NK_CTX, "Metallic", -1, mat->metallic, 2, 0.001, 0.001);
        if (metallic != mat->metallic)
        {
            dirty = true;
            mat->metallic = metallic;
        }
        float roughness = nk_propertyf(NK_CTX, "Roughness", -1, mat->roughness, 2, 0.001, 0.001);
        if (roughness != mat->roughness)
        {
            dirty = true;
            mat->roughness = roughness;
        }
        char am[256];
        sprintf(am, "Alpha Mode: %d", mat->alpha_mode);
        if (nk_button_label(NK_CTX, am))
        {
            dirty = true;
            mat->alpha_mode = !mat->alpha_mode;
        }
        float em_str = nk_propertyf(NK_CTX, "Emissive Strength", 0, mat->emissive_strength, 100000, 0.01, 0.01);
        if (em_str != mat->emissive_strength)
        {
            dirty = true;
            mat->emissive_strength = em_str;
        }
        float ao_str = nk_propertyf(NK_CTX, "AO Strength", 0, mat->ao_strength, 100000, 0.01, 0.01);
        if (ao_str != mat->ao_strength)
        {
            dirty = true;
            mat->ao_strength = ao_str;
        }


        if (dirty)
        {
            upload_material_data();
        }



        
    }
    nk_end(NK_CTX);
    
}


void render_light_menu()
{
    if
    (
        nk_begin
        (
            NK_CTX,
            "Light Menu",
            nk_rect(SCREEN_WIDTH - 420, 20, 400, SCREEN_HEIGHT),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {
        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }


        Light * lights = CAST_VEC(GLOBALS.lights, Light);
        Light * light = &lights[EDITOR_SETTINGS.selected_light_id];

        bool dirty = false;

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        light_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, light_name_buffer, sizeof(light_name_buffer), nk_filter_default);
        
        nk_layout_row_dynamic(NK_CTX, 20, 2);
        if (nk_button_label(NK_CTX, "Save Lights"))
        {

            char complete_name[256];
            strcpy(complete_name, light_registry.directory);
            strncat(complete_name, light_name_buffer, 255 - strlen(light_registry.directory));
            complete_name[255] = '\0';
            save_vector(complete_name, &GLOBALS.lights);
        }


        render_file_directory_dropdown(&light_registry);

        if (nk_button_label(NK_CTX, "Load Lights"))
        {
            
            if (light_registry.current_selection < light_registry.names.count && light_registry.current_selection >= 0)
            {
                char complete_name[256];
                strcpy(complete_name, light_registry.directory);
                strncat(complete_name, &(((char*)(light_registry.names.buffer))[light_registry.current_selection * 256]), 255 - strlen(light_registry.directory));
                complete_name[255] = '\0';
                load_vector(complete_name, &GLOBALS.lights);
            }

        }

        
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        char buf[128];
        sprintf(buf, "Selected Light %d", EDITOR_SETTINGS.selected_light_id);
        nk_text(NK_CTX, buf, strlen(buf), NK_TEXT_ALIGN_LEFT);
        if (light->entity_id != -1)
        {
            Entity * ent = &ENTITIES[light->entity_id];
            if (!ent->free)
            {
                if (nk_button_label(NK_CTX, "Select Entity Light"))
                {
                    EDITOR_SETTINGS.selected_ent_id = ent->id;
                    EDITOR_SETTINGS.entity_menu_open = true;
                    EDITOR_SETTINGS.opening_menu = true;
                }
                
            }
        }

        if (nk_button_label(NK_CTX, "Build Light BVH"))
        {
            build_light_bvh();
        }

        if (nk_button_label(NK_CTX, "Create Light"))
        {
            Light * new_light = vector_expand(&GLOBALS.lights, 1);
            memset(new_light, 0, sizeof(Light));
            *new_light = default_light;
            Entity * ent = create_entity(NULL, -1, false, false);
            new_light->entity_id = ent->id;

            push_special_undo(
              create_undo_structure(
                    UNDOSTRUCT_LIGHT,
                    (int[8]){GLOBALS.lights.count-1, EDITOR_SETTINGS.selected_light_id},
                    UNDOLIGHT_SPAWN),
              NULL, 0, 0, 0);
            
            EDITOR_SETTINGS.selected_light_id = GLOBALS.lights.count-1;
        }

        if (nk_button_label(NK_CTX, "Delete Light"))
        {

            Light * ls = CAST_VEC(GLOBALS.lights, Light);

            uint8_t ** bytes = malloc(sizeof(uint8_t*));
            bytes[0] = malloc(sizeof(Light));
            memcpy(bytes[0], light, sizeof(Light));
            size_t total_bytes = sizeof(uint8_t*) + sizeof(Light);
            push_special_undo(create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOLIGHT_DELETE), bytes, total_bytes, 1, 0);

            for (int i = EDITOR_SETTINGS.selected_light_id; i < GLOBALS.lights.count-1; i++)
            {
                Light * l = &ls[i];
                *l = ls[i+1];
            }
            GLOBALS.lights.count--;
        }
        
        
        struct nk_colorf light_color = {
            light->rgb.x,
            light->rgb.y,
            light->rgb.z,
            1.0f
        };
        nk_label(NK_CTX, "Color", NK_TEXT_LEFT);

        if (nk_combo_begin_color(NK_CTX, nk_rgb_cf(light_color), nk_vec2(150, 150))) {
            nk_layout_row_dynamic(NK_CTX, 120, 1);
            light_color = nk_color_picker(NK_CTX, light_color, NK_RGB);


            nk_layout_row_dynamic(NK_CTX, 25, 1);
            light_color.r = nk_propertyf(NK_CTX, "#R:", 0.0f, light_color.r, 1.0f, 0.01f, 0.005f);
            light_color.g = nk_propertyf(NK_CTX, "#G:", 0.0f, light_color.g, 1.0f, 0.01f, 0.005f);
            light_color.b = nk_propertyf(NK_CTX, "#B:", 0.0f, light_color.b, 1.0f, 0.01f, 0.005f);

            if (check_equivalencyf(&light->rgb.x, light_color.r, &LIGHT_MENU.color_r, &dirty))
            {
                push_nk_slider_undo(&LIGHT_MENU.color_r, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, rgb.x));
            }
            if (check_equivalencyf(&light->rgb.y, light_color.g, &LIGHT_MENU.color_g, &dirty))
            {
                
                push_nk_slider_undo(&LIGHT_MENU.color_g, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, rgb.y));
            }
            if (check_equivalencyf(&light->rgb.z, light_color.b, &LIGHT_MENU.color_b, &dirty))
            {
                push_nk_slider_undo(&LIGHT_MENU.color_b, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, rgb.z));
                
            }
            nk_combo_end(NK_CTX);
        }

        nk_layout_row_dynamic(NK_CTX, 20, 3);
        float xval = nk_propertyf(NK_CTX, "X", -1000.0f, light->pos.x, 1000.0f, 0.01f, 0.01f);
        float yval = nk_propertyf(NK_CTX, "Y", -1000.0f, light->pos.y, 1000.0f, 0.01f, 0.01f);
        float zval = nk_propertyf(NK_CTX, "Z", -1000.0f, light->pos.z, 1000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&light->pos.x, xval, &LIGHT_MENU.x, &dirty))
        {
            push_nk_slider_undo(&LIGHT_MENU.x, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, pos.x));
        }
        if (check_equivalencyf(&light->pos.y, yval, &LIGHT_MENU.y, &dirty))
        {
            push_nk_slider_undo(&LIGHT_MENU.y, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, pos.y));
        }
        if (check_equivalencyf(&light->pos.z, zval, &LIGHT_MENU.z, &dirty))
        {
            push_nk_slider_undo(&LIGHT_MENU.z, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, pos.z));
        }
        bool dir_dirty = false;

        xval = nk_propertyf(NK_CTX, "RX", -1000.0f, light->dir.x, 1000.0f, 0.01f, 0.01f);
        yval = nk_propertyf(NK_CTX, "RY", -1000.0f, light->dir.y, 1000.0f, 0.01f, 0.01f);
        zval = nk_propertyf(NK_CTX, "RZ", -1000.0f, light->dir.z, 1000.0f, 0.01f, 0.01f);
        if (check_equivalencyv3(&light->dir, (vec3){xval, yval, zval}, &LIGHT_MENU.rx, &dir_dirty))
        {
            
            push_nk_slider_undo(&LIGHT_MENU.rx, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, dir));
        }
        if (dir_dirty)
        {
            light->dir = vec3_normalize(light->dir);
            dirty = true;
        }
        nk_layout_row_dynamic(NK_CTX, 20, 1);
        float intensity = nk_propertyf(NK_CTX, "Intensity", 0, light->intensity, 10000, 0.1, 0.1);
        if (check_equivalencyf(&light->intensity, intensity, &LIGHT_MENU.intensity, &dirty))
        {
            
            push_nk_slider_undo(&LIGHT_MENU.intensity, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id,UNDOSPECIAL_NONE), offsetof(Light, intensity));
        }

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        float threshold = nk_propertyf(NK_CTX, "Threshold", 0, light->threshold, 100, 0.0001, 0.0001);
        if (check_equivalencyf(&light->threshold, threshold, &LIGHT_MENU.threshold, &dirty))
        {
            push_nk_slider_undo(&LIGHT_MENU.threshold, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, threshold));
        }

        char radius[256];
        sprintf(radius, "Radius: %f", light->radius);
        nk_label(NK_CTX, radius, NK_TEXT_ALIGN_LEFT);

        
        if (light->light_type == SPOT_LIGHT)
        {

            light->cone_angle = clamp(light->cone_angle, 0, 20);
            light->penumbra = clamp(light->penumbra, 0, 20);

            float cone_angle= nk_propertyf(NK_CTX, "Cone Angle", 0, light->cone_angle, 20, 0.1, 0.1);
            if (check_equivalencyf(&light->cone_angle, cone_angle, &LIGHT_MENU.cone_angle, &dirty))
            {
                
                push_nk_slider_undo(&LIGHT_MENU.cone_angle, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, cone_angle));
            }
            float penumbra= nk_propertyf(NK_CTX, "Penumbra", 0, light->penumbra, 20, 0.1, 0.1);
            if (check_equivalencyf(&light->penumbra, penumbra, &LIGHT_MENU.penumbra, &dirty))
            {
                push_nk_slider_undo(&LIGHT_MENU.penumbra, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, penumbra));
                
            }

            
        }
        int light_type = nk_propertyi(NK_CTX, "Light Type", 0, (int)light->light_type, 2, 1, 1);

        // this one will not work btw the type is not an int so dont cry when it doesnt work
        int lt = light->light_type;
        if (check_equivalencyi(&lt, light_type, &LIGHT_MENU.light_type, &dirty))
        {
            push_nk_slider_undo(&LIGHT_MENU.light_type, create_light_undo_structure(EDITOR_SETTINGS.selected_light_id, UNDOSPECIAL_NONE), offsetof(Light, light_type));
        }
        if (light->light_type != light_type)
        {
            light->light_type = light_type;
        }

        
        nk_layout_row_dynamic(NK_CTX, 20, 2);
        float mfactor = nk_propertyf(NK_CTX, "mfactor", 0.0f, EDITOR_SETTINGS.mfactor, 10, 0.001, 0.001);

        if (check_equivalencyf(&EDITOR_SETTINGS.mfactor, mfactor, &LIGHT_MENU.mfactor, NULL))
        {
            push_nk_slider_undo(&LIGHT_MENU.mfactor, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, mfactor));
            
        }

        nk_bool mf = EDITOR_SETTINGS.mfactor_enabled == 1;
        if (nk_checkbox_label(NK_CTX, "M Factor", &mf))
        {

            
            LIGHT_MENU.mfactor_enabled.byte_size = sizeof(bool);
            memcpy(LIGHT_MENU.mfactor_enabled.bytes, &EDITOR_SETTINGS.mfactor_enabled, sizeof(bool));
            push_nk_slider_undo(&LIGHT_MENU.mfactor_enabled, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, mfactor_enabled));

            EDITOR_SETTINGS.mfactor_enabled = mf;
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        float texels_per_unit = nk_propertyf(NK_CTX, "Texels Per Unit", 0, EDITOR_SETTINGS.texels_per_uv_unit, 10, 0.1, 0.1);

        if (check_equivalencyf(&EDITOR_SETTINGS.texels_per_uv_unit, texels_per_unit, &LIGHT_MENU.texels_per_unit, NULL))
        {

            push_nk_slider_undo(&LIGHT_MENU.texels_per_unit, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, texels_per_uv_unit));
            
        }

        
        nk_layout_row_dynamic(NK_CTX, 20, 2);
        if (nk_button_label(NK_CTX, "Regenerate Texels"))
        {
            
            LM_regenerate_texels();
        }

        if (nk_button_label(NK_CTX, "Assign New Texels"))
        {
            
            LM_allocate_empty_mns();
        }

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        int lm_iterations  = nk_propertyi(NK_CTX, "Iterations", 0, EDITOR_SETTINGS.lm_iterations, 2048, 1, 1);
        
        if (check_equivalencyi(&EDITOR_SETTINGS.lm_iterations, lm_iterations, &LIGHT_MENU.lm_iterations, NULL))
        {
            push_nk_slider_undo(&LIGHT_MENU.lm_iterations, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_iterations));
        }

        int probe_iterations  = nk_propertyi(NK_CTX, "Probe Iterations", 0, EDITOR_SETTINGS.probe_iterations, 2048, 1, 1);
         
        if (check_equivalencyi(&EDITOR_SETTINGS.probe_iterations, probe_iterations, &LIGHT_MENU.probe_iterations, NULL))
        {
            
            push_nk_slider_undo(&LIGHT_MENU.probe_iterations, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, probe_iterations));
        }

        int ao_iterations  = nk_propertyi(NK_CTX, "AO Iterations", 0, EDITOR_SETTINGS.ao_iterations, 2048, 1, 1);
         
        if (check_equivalencyi(&EDITOR_SETTINGS.ao_iterations, ao_iterations, &LIGHT_MENU.ao_iterations, NULL))
        {
            push_nk_slider_undo(&LIGHT_MENU.ao_iterations, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, ao_iterations));
        }

        float ao_threshold = nk_propertyf(NK_CTX, "AO Threshold", 0, EDITOR_SETTINGS.ao_threshold, 20, 0.1, 0.1);
        
        if (check_equivalencyf(&EDITOR_SETTINGS.ao_threshold, ao_threshold, &LIGHT_MENU.ao_threshold, NULL))
        {
            push_nk_slider_undo(&LIGHT_MENU.ao_threshold, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, ao_threshold));
        }

        
        if (nk_button_text(NK_CTX, "Regenerate", 10))
        {

            dirty = true;
            VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
            upload_material_data();

            GPU_upload_scene_data();
            upload_lights();
            UVIsland * uvis = CAST_VEC(GLOBALS.scene_data.lightmap_data.islands, UVIsland);
            for (int i = 0; i < GLOBALS.scene_data.lightmap_data.islands.count; i++)
            {
                uvis[i].currently_rasterizing = true;
            }
            bake_lightmap(&GLOBALS.scene_data.lightmap_data);
            add_all_meshes_to_buffer(vl);

        };


        static char lightmap_buffer[256] = {0};

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        lm_name_flags = nk_edit_string_zero_terminated(NK_CTX, NK_EDIT_FIELD, lightmap_buffer, sizeof(lightmap_buffer), nk_filter_default);

        if (nk_button_label(NK_CTX, "Save LM"))
        {

            char lm_name[256];
            strcpy(lm_name, lightmap_registry.directory);
            strcat(lm_name, lightmap_buffer);
            strcat(lm_name, "_lm.bin");
            save_lightmap_to_binary(&GLOBALS.scene_data.lightmap_data, lm_name);
        }

        nk_layout_row_dynamic(NK_CTX, 20, 2);
        render_file_directory_dropdown(&lightmap_registry);
        if (nk_button_label(NK_CTX, "Load LM"))
        {
            
            if (lightmap_registry.current_selection < lightmap_registry.names.count && lightmap_registry.current_selection >= 0)
            {
                char complete_name[256];
                strcpy(complete_name, lightmap_registry.directory);
                strncat(complete_name, &(((char*)(lightmap_registry.names.buffer))[lightmap_registry.current_selection * 256]), 255 - strlen(lightmap_registry.directory));
                complete_name[255] = '\0';
                load_lightmap_from_binary(&GLOBALS.scene_data.lightmap_data, complete_name);
            }

        }

        
        
        bool expand_aabb = false;

        nk_layout_row_dynamic(NK_CTX, 20, 3);

        float aabbx = nk_propertyf(NK_CTX, "lm_aabbX", -1000.0f, EDITOR_SETTINGS.lm_aabb.center.x, 1000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.center.x, aabbx, &LIGHT_MENU.lm_aabbx, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.lm_aabbx, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.center.x));
        }
        
        float aabby = nk_propertyf(NK_CTX, "lm_aabbY", -1000.0f, EDITOR_SETTINGS.lm_aabb.center.y, 1000.0f, 0.01f, 0.01f);
        
        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.center.y, aabby, &LIGHT_MENU.lm_aabby, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.lm_aabby, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.center.y));
        }

        float aabbz = nk_propertyf(NK_CTX, "lm_aabbZ", -1000.0f, EDITOR_SETTINGS.lm_aabb.center.z, 1000.0f, 0.01f, 0.01f);
        
        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.center.z, aabbz, &LIGHT_MENU.lm_aabbz, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.lm_aabbz, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.center.z));
        }
        
        nk_layout_row_dynamic(NK_CTX, 20, 3);

        float aabbex = nk_propertyf(NK_CTX, "AABBE_X", -1000.0f, EDITOR_SETTINGS.lm_aabb.extents.x, 1000.0f, 0.01f, 0.01f);
        
        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.extents.x, aabbex, &LIGHT_MENU.aabbe_x, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.aabbe_x, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.extents.x));
            
        }

        float aabbey = nk_propertyf(NK_CTX, "AABBE_Y", -1000.0f, EDITOR_SETTINGS.lm_aabb.extents.y, 1000.0f, 0.01f, 0.01f);
        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.extents.y, aabbey, &LIGHT_MENU.aabbe_y, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.aabbe_y, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.extents.y));
        }

        float aabbez = nk_propertyf(NK_CTX, "AABBE_Z", -1000.0f, EDITOR_SETTINGS.lm_aabb.extents.z, 1000.0f, 0.01f, 0.01f);

        if (check_equivalencyf(&EDITOR_SETTINGS.lm_aabb.extents.z, aabbez, &LIGHT_MENU.aabbe_z, &expand_aabb))
        {
            push_nk_slider_undo(&LIGHT_MENU.aabbe_z, create_editor_settings_undo_structure(UNDOSPECIAL_NONE), offsetof(EditorSettings, lm_aabb.extents.z));
        }

        AABB current_aabb;
        calculate_aabb_bounds(&EDITOR_SETTINGS.lm_aabb);

        nk_layout_row_dynamic(NK_CTX, 20, 1);
        // if (nk_button_text(NK_CTX, "UV Lightmap Partial Bake", 24))
        // {

        //     dirty = true;
        //     traverse_scene_bvh_uv_lightmap(&GLOBALS.scene_data, CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode), EDITOR_SETTINGS.lm_aabb);
        //     upload_material_data();

        //     GPU_upload_scene_data();
        //     upload_lights();
        //     bake_lightmap(&GLOBALS.scene_data.lightmap_data);
        //     add_all_meshes_to_buffer(GLOBALS.vertex_layouts.buffer);

        // }


        
        if (nk_button_text(NK_CTX, "Spawn New R Probe", 17))
        {

            ReflectionProbe * probe = vector_expand(&GLOBALS.scene_data.reflection_probes, 1);


            probe->aabb.center = (vec3){0,0,0};
            probe->aabb.extents = (vec3){1,1,1};
            probe->capture_pos = (vec3){0,0,0};
            calculate_aabb_bounds(&probe->aabb);
            probe->cubemap_idx = GLOBALS.scene_data.reflection_probes.count-1;
            probe->fade_margin = (vec3){1,1,1};
            reallocate_rprobe_array();
            
            
            push_special_undo(create_undo_structure(UNDOSTRUCT_RPROBE, (int[8]){ EDITOR_SETTINGS.selected_r_probe, GLOBALS.scene_data.reflection_probes.count-1}, UNDORPROBE_SPAWN), NULL, 0, 0, 0);

            EDITOR_SETTINGS.selected_r_probe = GLOBALS.scene_data.reflection_probes.count-1;


        }

        if (EDITOR_SETTINGS.selected_r_probe != -1 && EDITOR_SETTINGS.selected_r_probe < GLOBALS.scene_data.reflection_probes.count)
        {

            char buf[128];
            sprintf(buf, "Selected Probe %d", EDITOR_SETTINGS.selected_r_probe);
            nk_text(NK_CTX, buf, strlen(buf), NK_TEXT_ALIGN_LEFT);

            ReflectionProbe * probes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);
            ReflectionProbe * p = &probes[EDITOR_SETTINGS.selected_r_probe];

            if (nk_button_text(NK_CTX, "Delete R Probe", 17))
            {

                uint8_t ** bytes = malloc(sizeof(uint8_t*));
                bytes[0] = malloc(sizeof(ReflectionProbe));
                memcpy(bytes[0], p, sizeof(ReflectionProbe));
                size_t total_bytes = sizeof(uint8_t*) + sizeof(ReflectionProbe);
                int byte_count = 1;

                push_special_undo(create_undo_structure(UNDOSTRUCT_RPROBE, (int[8]){ EDITOR_SETTINGS.selected_r_probe, GLOBALS.scene_data.reflection_probes.count-1}, UNDORPROBE_DELETE), bytes, total_bytes, 1, 0);
                
                REMOVE_FROM_VEC(&GLOBALS.scene_data.reflection_probes, EDITOR_SETTINGS.selected_r_probe, ReflectionProbe);

                
            }

            nk_layout_row_dynamic(NK_CTX, 20, 3);

            bool rp_expand_aabb = false;

            float aabbx = nk_propertyf(NK_CTX, "rp_aabbX", -1000.0f, p->aabb.center.x, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.center.x, aabbx, &LIGHT_MENU.rp_aabbx, &rp_expand_aabb))
            {

                push_nk_slider_undo(&LIGHT_MENU.rp_aabbx, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.center.x));
                
            }

            float aabby = nk_propertyf(NK_CTX, "rp_aabbY", -1000.0f, p->aabb.center.y, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.center.y, aabby, &LIGHT_MENU.rp_aabby, &rp_expand_aabb))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_aabby, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.center.y));
                
            }
            float aabbz = nk_propertyf(NK_CTX, "rp_aabbZ", -1000.0f, p->aabb.center.z, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.center.z, aabbz, &LIGHT_MENU.rp_aabbz, &rp_expand_aabb))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_aabbz, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.center.z));
                
            }
        
            nk_layout_row_dynamic(NK_CTX, 20, 3);

            float aabbex = nk_propertyf(NK_CTX, "rp_AABBE_X", -1000.0f, p->aabb.extents.x, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.extents.x, aabbex, &LIGHT_MENU.rp_aabbe_x, &rp_expand_aabb))
            {
                
                push_nk_slider_undo(&LIGHT_MENU.rp_aabbe_x, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.extents.x));
            }

            float aabbey = nk_propertyf(NK_CTX, "rp_AABBE_Y", -1000.0f, p->aabb.extents.y, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.extents.y, aabbey, &LIGHT_MENU.rp_aabbe_y, &rp_expand_aabb))
            {
                
                push_nk_slider_undo(&LIGHT_MENU.rp_aabbe_y, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.extents.y));
            }

            float aabbez = nk_propertyf(NK_CTX, "rp_AABBE_Z", -1000.0f, p->aabb.extents.z, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->aabb.extents.z, aabbez, &LIGHT_MENU.rp_aabbe_z, &rp_expand_aabb))
            {
                
                push_nk_slider_undo(&LIGHT_MENU.rp_aabbe_z, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, aabb.extents.z));
            }

            if (rp_expand_aabb)
            {
                calculate_aabb_bounds(&p->aabb);
            }
            
            float rp_fadex = nk_propertyf(NK_CTX, "rp_fade_X", -1000.0f, p->fade_margin.x, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->fade_margin.x, rp_fadex, &LIGHT_MENU.rp_fade_x, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_fade_x, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, fade_margin.x));
            }

            float rp_fadey = nk_propertyf(NK_CTX, "rp_fade_Y", -1000.0f, p->fade_margin.y, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->fade_margin.y, rp_fadey, &LIGHT_MENU.rp_fade_y, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_fade_y, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, fade_margin.y));
            }

            float rp_fadez = nk_propertyf(NK_CTX, "rp_fade_Z", -1000.0f, p->fade_margin.z, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->fade_margin.z, rp_fadez, &LIGHT_MENU.rp_fade_z, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_fade_z, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, fade_margin.z));
            }

            float capture_posx = nk_propertyf(NK_CTX, "rp_capposX", -1000.0f, p->capture_pos.x, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->capture_pos.x, capture_posx, &LIGHT_MENU.rp_capposx, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_capposx, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, capture_pos.x));
            }

            float capture_posy = nk_propertyf(NK_CTX, "rp_capposY", -1000.0f, p->capture_pos.y, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->capture_pos.y, capture_posy, &LIGHT_MENU.rp_capposy, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_capposy, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, capture_pos.y));
            }

            float capture_posz = nk_propertyf(NK_CTX, "rp_capposZ", -1000.0f, p->capture_pos.z, 1000.0f, 0.01f, 0.01f);
            if (check_equivalencyf(&p->capture_pos.z, capture_posz, &LIGHT_MENU.rp_capposz, NULL))
            {
                push_nk_slider_undo(&LIGHT_MENU.rp_capposz, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, capture_pos.z));
            }
            
            nk_layout_row_dynamic(NK_CTX, 20, 1);
            if (nk_button_text(NK_CTX, "Center Cap Pos", 14))
            {
                store_into_nkslider(&LIGHT_MENU.center_pos, &p->capture_pos, sizeof(vec3));
                push_nk_slider_undo(&LIGHT_MENU.center_pos, create_singular_id_undo_structure(UNDOSTRUCT_RPROBE, EDITOR_SETTINGS.selected_r_probe, UNDOSPECIAL_NONE), offsetof(ReflectionProbe, capture_pos));

                p->capture_pos = p->aabb.center;
            }
            if (nk_button_text(NK_CTX, "Bake R Probes", 13))
            {
                bake_reflection_probes();
            }
            
        }
        

        if (dirty == 1)
        {
            GLOBALS.lights_dirty = dirty;
            light->sm_dirty = dirty;
        }

    }
    nk_end(NK_CTX);
    
}

void render_asset_instance_menu()
{
    
    if
    (
        nk_begin
        (
            NK_CTX,
            menu_names[MENU_AINST],
            nk_rect(SCREEN_WIDTH - 420, 20, 400, SCREEN_HEIGHT),
            NK_WINDOW_TITLE | NK_WINDOW_SCROLL_AUTO_HIDE | NK_WINDOW_CLOSABLE
        )
    )
    {

        struct nk_rect win_bounds = nk_window_get_bounds(NK_CTX);
        if (nk_input_is_mouse_hovering_rect(&NK_CTX->input, win_bounds)) { NUKLEAR_HOVERED = true; }



        AssetInstance * ai = &ASSET_INSTANCES[EDITOR_SETTINGS.selected_asset_instance_id];
        
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

        nk_layout_row_dynamic(NK_CTX, 20, 1);

        char (* node_names)[256] = calloc(ai->node_count, sizeof(*node_names));
        char ** name_ptrs = malloc(sizeof(char*) * ai->node_count);

        for (int i = 0; i < ai->node_count; i++)
        {

            MeshNode * mn = &mns[ai->node_indices[i]];

            strcpy(node_names[i], mn->name);
            name_ptrs[i] = node_names[i];
            
        }
        
        
        
        int selected = EDITOR_SETTINGS.selected_asset_instance_mn_id;
        render_dropdown("Nodes", &selected, name_ptrs, ai->node_count, 300, 100);

        if (selected != EDITOR_SETTINGS.selected_asset_instance_mn_id)
        {
            EDITOR_SETTINGS.selected_asset_instance_mn_id = selected;
        }

        free(node_names);
        free(name_ptrs);

        
        if (nk_button_label(NK_CTX, "Select Node"))
        {
            if (EDITOR_SETTINGS.selected_asset_instance_mn_id >= 0 || EDITOR_SETTINGS.selected_asset_instance_mn_id < ai->node_count)
            {
            
                EDITOR_SETTINGS.selected_mn_id = ai->node_indices[EDITOR_SETTINGS.selected_asset_instance_mn_id];
                EDITOR_SETTINGS.mesh_node_menu_open = true;
            }
        }

        
        if (nk_button_label(NK_CTX, "Delete Asset Instance"))
        {
            destroy_asset_instance(EDITOR_SETTINGS.selected_asset_instance_id);
            EDITOR_SETTINGS.selected_asset_instance_id = -1;
        }
        
        render_mat4_widget("TR ASSET", UNDOSTRUCT_ASSETINSTANCE, EDITOR_SETTINGS.selected_asset_instance_id, &ai->model, &ai->tr, &ai->rot, &ai->scale, AI_MENU.model);


        
    }
    nk_end(NK_CTX);

}



void render_editor_menu()
{

    NUKLEAR_HOVERED = false;
    tb_id_flags = NK_EDIT_INACTIVE;
    tb_text_flags = NK_EDIT_INACTIVE;
    tb_cont_flags = NK_EDIT_INACTIVE;
    ui_id_flags = NK_EDIT_INACTIVE;
    ui_parent_cont_flags = NK_EDIT_INACTIVE;
    ui_cont_flags = NK_EDIT_INACTIVE;
    ui_scrollbar_flags = NK_EDIT_INACTIVE;
    ui_label_id_flags = NK_EDIT_INACTIVE;
    aportal_flags = NK_EDIT_INACTIVE;
    apv_flags = NK_EDIT_INACTIVE;
    scene_apv_flags = NK_EDIT_INACTIVE;
    lpv_flags = NK_EDIT_INACTIVE;
    scene_lpv_flags = NK_EDIT_INACTIVE;
    scene_name_flags = NK_EDIT_INACTIVE;
    scene_file_name_flags = NK_EDIT_INACTIVE;
    ui_name_flags = NK_EDIT_INACTIVE;
    rprobe_name_flags = NK_EDIT_INACTIVE;
    light_name_flags = NK_EDIT_INACTIVE;
    lm_name_flags = NK_EDIT_INACTIVE;



    if (EDITOR_SETTINGS.selection_menu_open)
    {
        render_selection_menu();
    }
    if (EDITOR_SETTINGS.lprobe_menu_open)
    {
        render_lprobe_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_LPROBE]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_LPROBE]))
        {
            EDITOR_SETTINGS.lprobe_menu_open = false;
        }
    }

    if (EDITOR_SETTINGS.main_menu_open)
    {
        render_main_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_MAIN]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_MAIN]))
        {
            EDITOR_SETTINGS.main_menu_open = false;
        }
    }
    
    if (EDITOR_SETTINGS.mesh_node_menu_open && EDITOR_SETTINGS.selected_mn_id < GLOBALS.mesh_nodes.count && EDITOR_SETTINGS.selected_mn_id >= 0)
    {

        render_mn_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_MN]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_MN]))
        {
            EDITOR_SETTINGS.mesh_node_menu_open = false;
        }
       
    }


    if (EDITOR_SETTINGS.material_menu_open && EDITOR_SETTINGS.selected_material_mn_id < GLOBALS.materials.count)
    {
        render_material_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_MATERIAL]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_MATERIAL]))
        {
            EDITOR_SETTINGS.material_menu_open = false;
        }
        
    }
    
    
    if (EDITOR_SETTINGS.light_menu_open && EDITOR_SETTINGS.selected_light_id < GLOBALS.lights.count)
    {

        render_light_menu();
        if (nk_window_is_closed(NK_CTX, "Light Menu") || nk_window_is_hidden(NK_CTX, "Light Menu"))
        {
            EDITOR_SETTINGS.light_menu_open = false;
        }
    }
    if (EDITOR_SETTINGS.entity_menu_open)
    {

        Entity * ent = &ENTITIES[EDITOR_SETTINGS.selected_ent_id];
        if (!ent->free)
        {
            render_entity_menu();
            if (nk_window_is_closed(NK_CTX, menu_names[MENU_ENT]))
            {
                EDITOR_SETTINGS.entity_menu_open = false;
            }
            if (nk_window_is_closed(NK_CTX, menu_names[MENU_ENT]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_ENT]))
            {
                EDITOR_SETTINGS.entity_menu_open = false;
            }
        }
    }
    if (EDITOR_SETTINGS.audio_menu_open)
    {

        render_audio_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_AUDIO]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_AUDIO]))
        {
            EDITOR_SETTINGS.audio_menu_open = false;
        }
    }
    if (EDITOR_SETTINGS.asset_instance_menu_open && EDITOR_SETTINGS.selected_asset_instance_id >= 0 && EDITOR_SETTINGS.selected_asset_instance_id < MAX_ASSET_INSTANCES)
    {
        render_asset_instance_menu();
        if (nk_window_is_closed(NK_CTX, menu_names[MENU_AINST]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_AINST]))
        {
            EDITOR_SETTINGS.asset_instance_menu_open = false;
        }
    }
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    if (EDITOR_SETTINGS.selected_ui_menu_id >= 0 && EDITOR_SETTINGS.selected_ui_menu_id < MENUS.count)
    {
        UIMenu * m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];

        if (EDITOR_SETTINGS.text_menu_open && EDITOR_SETTINGS.selected_text_id >= 0 && EDITOR_SETTINGS.selected_text_id < MAX_TEXT_BOXES)
        {
            render_text_menu();
            if (nk_window_is_closed(NK_CTX, menu_names[MENU_TEXT]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_TEXT]))
            {
                EDITOR_SETTINGS.text_menu_open = false;
            }
        }
        if (EDITOR_SETTINGS.ui_menu_open && EDITOR_SETTINGS.selected_ui_element_id >= 0 && EDITOR_SETTINGS.selected_ui_element_id < MAX_UI_ELEMENTS)
        {
            render_ui_menu();
            if (nk_window_is_closed(NK_CTX, menu_names[MENU_UI]) || nk_window_is_hidden(NK_CTX, menu_names[MENU_UI]))
            {
                EDITOR_SETTINGS.ui_menu_open = false;
            }
        }
    }

    if
    (
        tb_id_flags & NK_EDIT_ACTIVE || 
        tb_text_flags & NK_EDIT_ACTIVE || 
        tb_cont_flags & NK_EDIT_ACTIVE || 
        ui_menu_id_flags & NK_EDIT_ACTIVE || 
        ui_id_flags & NK_EDIT_ACTIVE || 
        ui_parent_cont_flags & NK_EDIT_ACTIVE || 
        ui_cont_flags & NK_EDIT_ACTIVE || 
        ui_scrollbar_flags & NK_EDIT_ACTIVE || 
        ui_label_id_flags & NK_EDIT_ACTIVE || 
        aportal_flags & NK_EDIT_ACTIVE || 
        apv_flags & NK_EDIT_ACTIVE || 
        scene_apv_flags & NK_EDIT_ACTIVE || 
        lpv_flags & NK_EDIT_ACTIVE || 
        scene_lpv_flags & NK_EDIT_ACTIVE || 
        scene_name_flags & NK_EDIT_ACTIVE || 
        scene_file_name_flags & NK_EDIT_ACTIVE || 
        ui_name_flags & NK_EDIT_ACTIVE || 
        rprobe_name_flags & NK_EDIT_ACTIVE || 
        light_name_flags & NK_EDIT_ACTIVE || 
        lm_name_flags & NK_EDIT_ACTIVE  
    )
    {
        GLOBALS.keyboard_locked = true;
    }
    else if
    (
        (tb_id_flags & NK_EDIT_INACTIVE || tb_id_flags == 0) && 
        (tb_text_flags & NK_EDIT_INACTIVE || tb_text_flags == 0) && 
        (tb_cont_flags & NK_EDIT_INACTIVE || tb_cont_flags == 0) && 
        (ui_menu_id_flags & NK_EDIT_INACTIVE || ui_menu_id_flags == 0) && 
        (ui_id_flags & NK_EDIT_INACTIVE || ui_id_flags == 0) && 
        (ui_parent_cont_flags & NK_EDIT_INACTIVE || ui_parent_cont_flags == 0) && 
        (ui_cont_flags & NK_EDIT_INACTIVE || ui_cont_flags == 0) && 
        (ui_scrollbar_flags & NK_EDIT_INACTIVE || ui_scrollbar_flags == 0) && 
        (ui_label_id_flags & NK_EDIT_INACTIVE || ui_label_id_flags == 0) && 
        (aportal_flags & NK_EDIT_INACTIVE || aportal_flags == 0) && 
        (apv_flags & NK_EDIT_INACTIVE || apv_flags == 0) && 
        (scene_apv_flags & NK_EDIT_INACTIVE || scene_apv_flags == 0) && 
        (lpv_flags & NK_EDIT_INACTIVE || lpv_flags == 0) && 
        (scene_lpv_flags & NK_EDIT_INACTIVE || scene_lpv_flags == 0) && 
        (scene_name_flags & NK_EDIT_INACTIVE || scene_name_flags == 0) && 
        (scene_file_name_flags & NK_EDIT_INACTIVE || scene_file_name_flags == 0) && 
        (ui_name_flags & NK_EDIT_INACTIVE || ui_name_flags == 0) && 
        (rprobe_name_flags & NK_EDIT_INACTIVE || rprobe_name_flags == 0) && 
        (light_name_flags & NK_EDIT_INACTIVE || light_name_flags == 0) && 
        (lm_name_flags & NK_EDIT_INACTIVE || lm_name_flags == 0)
    )
    {
        GLOBALS.keyboard_locked = false;
    }
    
    

    
}


/** @} */

