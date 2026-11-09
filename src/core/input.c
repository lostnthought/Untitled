#include <SDL2/SDL.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL_scancode.h>
#include <SDL2/SDL_timer.h>
#include <SDL2/SDL_video.h>
#include "core/acceleration.h"
#include "core/engine_types.h"
#include "ui/fonts.h"
#include "core/shaders.h"
#include "utilities/types.h"
#include "load/init.h"
#include "core/input.h"
#include "utilities/utilities.h"
#include "core/camera.h"
#include "audio/audio.h"
#include "editor/editor.h"

#include "load/load.h"
#include "ui/ui.h"

void handle_ui_selection_event()
{

    if (!SDL_GetRelativeMouseMode() && EDITOR_SETTINGS.editor_mode && !NUKLEAR_HOVERED)
    {
        bool dragged = false;
        if (EDITOR_SETTINGS.element_drag_mode)
        {

            vec2 tr = screenspace_to_ui((vec2){MOUSE_X, MOUSE_Y});
            if (EDITOR_SETTINGS.selected_ui_element_id >= 0 && EDITOR_SETTINGS.selected_ui_element_id < MAX_UI_ELEMENTS && EDITOR_SETTINGS.selected_ui_menu_id >= 0 && EDITOR_SETTINGS.selected_ui_menu_id < MENUS.count)
            {
                UIMenu * ms = CAST_VEC(MENUS, UIMenu);
                UIMenu * m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];
                UIElement * el = &m->elements[EDITOR_SETTINGS.selected_ui_element_id];
                el->translation = tr;
                dragged = true;
            }
            
        }
        if (EDITOR_SETTINGS.text_box_drag_mode)
        {
            
            vec2 tr = screenspace_to_ui((vec2){MOUSE_X, MOUSE_Y});
            if (EDITOR_SETTINGS.selected_text_id >= 0 && EDITOR_SETTINGS.selected_text_id < MAX_UI_ELEMENTS && EDITOR_SETTINGS.selected_ui_menu_id >= 0 && EDITOR_SETTINGS.selected_ui_menu_id < MENUS.count)
            {
                UIMenu * ms = CAST_VEC(MENUS, UIMenu);
                UIMenu * m = &ms[EDITOR_SETTINGS.selected_ui_menu_id];
                TextBox * tb = &m->text_boxes[EDITOR_SETTINGS.selected_text_id];
                tb->translation = tr;
                dragged = true;
            }
        }
        // if (!dragged)
        // {
            
            if (EDITOR_SETTINGS.SELMODE == SELMODE_TEXTBOX)
            {
            
                UIMenu * ms = CAST_VEC(MENUS, UIMenu);
                int menu_idx = -1;
                int tb_idx = get_text_box_from_coords((vec2){MOUSE_X, MOUSE_Y}, &menu_idx);
                if (tb_idx != -1 && menu_idx != -1)
                {
                    EDITOR_SETTINGS.selected_text_id = tb_idx;
                    EDITOR_SETTINGS.selected_ui_menu_id = menu_idx;
                    EDITOR_SETTINGS.text_menu_open = true;
                    EDITOR_SETTINGS.opening_menu = true;
                    nk_window_show(NK_CTX, menu_names[MENU_TEXT], NK_SHOWN);

                }
            }
            else if (EDITOR_SETTINGS.SELMODE == SELMODE_UIELEMENT)
            {

                int menu_idx = -1;
                int ui_element = get_ui_element_from_coords((vec2){MOUSE_X, MOUSE_Y}, &menu_idx);
                if (ui_element != -1 && menu_idx != -1)
                {

                    EDITOR_SETTINGS.selected_ui_element_id = ui_element;
                    EDITOR_SETTINGS.selected_ui_menu_id = menu_idx;
                    EDITOR_SETTINGS.ui_menu_open = true;
                    EDITOR_SETTINGS.opening_menu = true;
                    nk_window_show(NK_CTX, menu_names[MENU_UI], NK_SHOWN);
                }
            
            }
        // }
    }
    
}


void handle_held_events()
{
    
    if (LMB_HELD)
    {
        handle_ui_selection_event();
    }
}


void process_sdl_event(SDL_Event event)
{
    
    if (event.type == SDL_QUIT) running = 0;

    if (EDITOR_SETTINGS.SELMODE != SELMODE_UIELEMENT)
    {
        handle_ui_event(event, MOUSE_X, MOUSE_Y);
    }
    if (event.type == SDL_MOUSEBUTTONUP)
    {
        LMB_HELD = 0;
    }

    if (event.type == SDL_MOUSEBUTTONDOWN)
    {
        LMB_HELD = 1;
        if (EDITOR_SETTINGS.editor_mode && (!NUKLEAR_HOVERED && SDL_GetRelativeMouseMode()))
        {

            vec3 dir = cam.forward;
            vec3 inv_dir = {1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z};

            BVHNode * scene_nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);

            RayHit hit = raycast_scene(CAM_POS, cam.forward, INFINITY, false);


            Light * lights = CAST_VEC(GLOBALS.lights, Light);
            const float light_size = 0.5f;
            float closest_light_dist = INFINITY;
            int closest_light = -1;
            float closest_rprobe_dist = INFINITY;
            int closest_rprobe = -1;
            ReflectionProbe * rprobes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);

            float closest_emitter_dist = INFINITY;
            int closest_emitter = -1;

            float closest_aportal_dist = INFINITY;
            int closest_aportal = -1;
            AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);

            float closest_lprobe_volume_dist = INFINITY;
            int closest_lprobe_volume = -1;
            ProbeVolume * lighting_probe_volumes = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);

            float closest_aprobe_volume_dist = INFINITY;
            int closest_aprobe_volume = -1;
            ProbeVolume * audio_probe_volumes = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);

            switch(EDITOR_SETTINGS.SELMODE)
            {
                case SELMODE_ASSETINSTANCE:
                case SELMODE_MN:
                    if (hit.hit != HIT_NONE)
                    {
                        EDITOR_SETTINGS.selected_mn_id = hit.mesh_node_idx;
                        EDITOR_SETTINGS.selected_sm_id = 0;
                        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
                        MeshNode * mn = &mns[EDITOR_SETTINGS.selected_mn_id];
                        Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
                        EDITOR_SETTINGS.selected_asset_instance_id = mn->asset_instance;
                        if (EDITOR_SETTINGS.SELMODE == SELMODE_ASSETINSTANCE)
                        {
                            
                            EDITOR_SETTINGS.asset_instance_menu_open = true;
                            EDITOR_SETTINGS.opening_menu = true;
                            nk_window_show(NK_CTX, menu_names[MENU_AINST], NK_SHOWN);
                        }
                        else
                        {
                            if (mn->mesh != -1)
                            {
                    
                                Mesh * m = &meshes[mn->mesh];

                                SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);
                                if (sms) EDITOR_SETTINGS.selected_material_mn_id = sms[0].material;
                                EDITOR_SETTINGS.mesh_node_menu_open = true;
                                EDITOR_SETTINGS.opening_menu = true;
                                nk_window_show(NK_CTX, menu_names[MENU_MN], NK_SHOWN);

                            }
                            
                        }
                    }
                    break;
                case SELMODE_LIGHT:
                    for (int i = 0; i < GLOBALS.lights.count; i++)
                    {
                        Light * l = &lights[i];
                        AABB aabb;
                        aabb.center = l->pos;
                        aabb.extents = (vec3){light_size, light_size, light_size};
                        calculate_aabb_bounds(&aabb);
                        vec3 intersection;
                        if (ray_intersects_aabb_with_point(aabb, CAM_POS, dir, inv_dir, &intersection))
                        {
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_light_dist > dist)
                            {
                                closest_light_dist = dist;
                                closest_light = i;
                            }
            
                        }
        
                    }

                    if (closest_light != -1)
                    {
                        EDITOR_SETTINGS.selected_light_id = closest_light;
                        EDITOR_SETTINGS.light_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                    }
                    break;
                case SELMODE_LPV:
                    for (int i = 0; i < GLOBALS.scene_data.lighting_probe_volumes.count; i++)
                    {
                        ProbeVolume * pv = &lighting_probe_volumes[i];

                        vec3 intersection;
                        if (ray_intersects_obb_with_point(pv->obb, CAM_POS, dir, &intersection))
                        {
                    
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_lprobe_volume_dist > dist)
                            {
                                closest_lprobe_volume_dist = dist;
                                closest_lprobe_volume = i;
                            }
                        }
                    }
                    if (closest_lprobe_volume != -1)
                    {

                        EDITOR_SETTINGS.selected_lighting_probe_volume_id = closest_lprobe_volume;
                        EDITOR_SETTINGS.lprobe_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                        nk_window_show(NK_CTX, menu_names[MENU_LPROBE], NK_SHOWN);
            
                    }
                    break;
                case SELMODE_RPROBE:
                    for (int i = 0; i < GLOBALS.scene_data.reflection_probes.count; i++)
                    {
                        ReflectionProbe * l = &rprobes[i];
                        calculate_aabb_bounds(&l->aabb);
                        vec3 intersection;
                        if (ray_intersects_aabb_with_point(l->aabb, CAM_POS, dir, inv_dir, &intersection))
                        {
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_rprobe_dist > dist)
                            {
                                closest_rprobe_dist = dist;
                                closest_rprobe = i;
                            }
            
                        }
                    }

                    if (closest_rprobe != -1)
                    {
                        EDITOR_SETTINGS.selected_r_probe = closest_rprobe;
                        EDITOR_SETTINGS.light_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                        nk_window_show(NK_CTX, menu_names[MENU_LIGHT], NK_SHOWN);
                    }
                    
                    break;
                case SELMODE_ENTITY:
                    break;
                case SELMODE_TEXTBOX:
                    break;
                case SELMODE_UIELEMENT:
                    break;
                case SELMODE_APV:
                    for (int i = 0; i < GLOBALS.scene_data.audio_probe_volumes.count; i++)
                    {
                        ProbeVolume * pv = &audio_probe_volumes[i];

                        vec3 intersection;
                        if (ray_intersects_obb_with_point(pv->obb, CAM_POS, dir, &intersection))
                        {
                    
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_aprobe_volume_dist > dist)
                            {
                                closest_aprobe_volume_dist = dist;
                                closest_aprobe_volume = i;
                            }
                        }
                    }
                    if (closest_aprobe_volume != -1)
                    {
                        EDITOR_SETTINGS.selected_audio_probe_volume_id = closest_aprobe_volume;
                        EDITOR_SETTINGS.audio_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                        nk_window_show(NK_CTX, menu_names[MENU_AUDIO], NK_SHOWN);
            
                    }
                    break;
                case SELMODE_APORTAL:
                    for (int i = 0; i < GLOBALS.audio_portals.count; i++)
                    {
                        AudioPortal * p = &portals[i];
            
                        AABB aabb;
                        aabb.center = p->pos;
                        aabb.extents = (vec3){EDITOR_APORTAL_SIZE, EDITOR_APORTAL_SIZE, EDITOR_APORTAL_SIZE};
                        calculate_aabb_bounds(&aabb);
                        vec3 intersection;
                        if (ray_intersects_aabb_with_point(aabb, CAM_POS, dir, inv_dir, &intersection))
                        {
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_aportal_dist > dist)
                            {
                                closest_aportal_dist = dist;
                                closest_aportal = i;
                            }
                        }
                    }
                    if (closest_aportal != -1)
                    {

                        EDITOR_SETTINGS.selected_audio_portal_id = closest_aportal;
                        EDITOR_SETTINGS.audio_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                        nk_window_show(NK_CTX, menu_names[MENU_AUDIO], NK_SHOWN);
            
                    }
                    break;

                case SELMODE_AEM:
                    for (int i = 0; i < MAX_AUDIO_EMITTERS; i++)
                    {
                        const float emitter_size = 0.5f;
                        if (AUDIO_EMITTERS[i].free) continue;
                        AudioEmitter * l = &AUDIO_EMITTERS[i];

                        AABB aabb;
                        aabb.center = l->pos;
                        aabb.extents = (vec3){emitter_size, emitter_size, emitter_size };
                        calculate_aabb_bounds(&aabb);
                        vec3 intersection;
                        if (ray_intersects_aabb_with_point(aabb, CAM_POS, dir, inv_dir, &intersection))
                        {
                            float dist = vec3_length(vec3_sub(intersection, CAM_POS));
                            if (closest_emitter_dist > dist)
                            {
                                closest_emitter_dist = dist;
                                closest_emitter = i;
                            }
                        }
                    }
                    if (closest_emitter != -1)
                    {
                        EDITOR_SETTINGS.selected_audio_emitter_id = closest_emitter;
                        EDITOR_SETTINGS.audio_menu_open = true;
                        EDITOR_SETTINGS.opening_menu = true;
                        nk_window_show(NK_CTX, menu_names[MENU_AUDIO], NK_SHOWN);
            
                    }

                    break;
            }


        }
        else
        {
            handle_ui_selection_event();
        }
    }


    // Look
    if (event.type == SDL_MOUSEMOTION && SDL_GetRelativeMouseMode()) {

        int dx = event.motion.xrel;
        int dy = event.motion.yrel;

        float sensitivity = 0.0012f;
        cam.yaw -= dx * sensitivity;

        cam.pitch -= dy * sensitivity;

        float max_pitch = (M_PI / 2.0f);
        float min_pitch = -(M_PI / 2.0f) + 0.01f;
        cam.pitch =
            SDL_clamp(cam.pitch, min_pitch, max_pitch);
        cam.yaw = wrap_angle_pi(cam.yaw);

        
    }
}


float cooldown_time = 0.0f;
float cooldown_interval = 0.1f;

// Process keyboard / mouse input
void
    process_input
    ( SDL_Window **window, SDL_Event * event_queue, int * event_queue_count, SDL_Event * sdl_queue, int * sdl_queue_count, float physics_dt)

{



    cooldown_time -= physics_dt;

    bool accept_toggle_input = false;
    if (cooldown_time <= 0)
    {
        
        accept_toggle_input = true;
    }

    
    vec3 move_dir = {0};
    
    Uint32 buttons = SDL_GetMouseState(NULL, NULL);
    bool lmouse_down = buttons & SDL_BUTTON(SDL_BUTTON_LEFT);


    if (!SDL_GetRelativeMouseMode()) {
        SDL_GetMouseState(&MOUSE_X, &MOUSE_Y);
    }

    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        
        if (!SDL_GetRelativeMouseMode() && *event_queue_count < MAX_NUKLEAR_EVENTS) {
            event_queue[(*event_queue_count)++] = event;
        }
        if (*sdl_queue_count < MAX_SDL_EVENTS) {
            sdl_queue[(*sdl_queue_count)++] = event;
        }
    }

    bool move_to_spawn = false;

    const Uint8 *keystates = SDL_GetKeyboardState(NULL);

    // Exit

    if (keystates[SDL_SCANCODE_ESCAPE]) running = 0;
    if (accept_toggle_input)
    {
        
        if (keystates[SDL_SCANCODE_F1])
        {
            EDITOR_SETTINGS.main_menu_open = true;
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F2])
        {
            SHADERS_INIT();
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F3])
        {
            toggle_performance_overlay();
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F4])
        {
            EDITOR_SETTINGS.audio_menu_open = true;
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F5])
        {
            EDITOR_SETTINGS.wireframe_toggled = !EDITOR_SETTINGS.wireframe_toggled;
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F6])
        {
            EDITOR_SETTINGS.selection_menu_open = !EDITOR_SETTINGS.selection_menu_open;
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F7])
        {
            EDITOR_SETTINGS.lprobe_menu_open = !EDITOR_SETTINGS.lprobe_menu_open;
            cooldown_time = cooldown_interval;
        }
        if (keystates[SDL_SCANCODE_F8])
        {
            EDITOR_SETTINGS.ui_menu_open = !EDITOR_SETTINGS.ui_menu_open;
            cooldown_time = cooldown_interval;
        }
    
    }
    if (!GLOBALS.keyboard_locked)
    {
        
        // WASD
        if (keystates[SDL_SCANCODE_W]) move_dir.x += 1.0f;
        if (keystates[SDL_SCANCODE_S]) move_dir.x -= 1.0f;
        if (keystates[SDL_SCANCODE_A]) move_dir.y += 1.0f;
        if (keystates[SDL_SCANCODE_D]) move_dir.y -= 1.0f;
        if (keystates[SDL_SCANCODE_Q])
        {
            EDITOR_SETTINGS.editor_menu_open = true;
        }


        if (accept_toggle_input)
        {
            
            if (keystates[SDL_SCANCODE_T])
            {
        
                GLOBALS.noclip = true;
            cooldown_time = cooldown_interval;
            }
            if (keystates[SDL_SCANCODE_U])
            {
                GLOBALS.noclip = false;
                vec3 current_pos = vec4_to_vec3(cam.cam_data.cam_pos);
                reset_skeleton(&player.skeleton, current_pos);
                player.pos = current_pos;
                player.pos.z -= player.skeleton.neck_height;

                player.skeleton.neck.rotation = (vec3){cam.yaw, cam.pitch, cam.roll};


            cooldown_time = cooldown_interval;

            }
            if (keystates[SDL_SCANCODE_R])
            {
                player.cam->cam_data.cam_pos.x = spawn_point.x;
                player.cam->cam_data.cam_pos.y = spawn_point.y;
                player.cam->cam_data.cam_pos.z = spawn_point.z;
                player.pos = spawn_point;
                player.pc->vel = (vec3){0,0,0};
                move_to_spawn = true;
            cooldown_time = cooldown_interval;
            }
            if (keystates[SDL_SCANCODE_C])
            {
                player.noclip = true;
            cooldown_time = cooldown_interval;
            }
            if (keystates[SDL_SCANCODE_V])
            {
                player.noclip = false;
            cooldown_time = cooldown_interval;
            }
            if (keystates[SDL_SCANCODE_P]) {

                SDL_SetWindowGrab(*window, SDL_FALSE);
                SDL_SetRelativeMouseMode(SDL_FALSE);
            cooldown_time = cooldown_interval;

            }

            // Lock mouse
            if (keystates[SDL_SCANCODE_O]) {

                SDL_SetWindowGrab(*window, SDL_TRUE);
                SDL_SetRelativeMouseMode(SDL_TRUE);
            cooldown_time = cooldown_interval;

            }
            if (keystates[SDL_SCANCODE_N]) {
                
                perform_newest_undo();
                cooldown_time = cooldown_interval;
            }
            if (keystates[SDL_SCANCODE_M]) {

                perform_newest_redo();
                cooldown_time = cooldown_interval;
            }
        }


        // Unlock mouse


        // Jump
        if (keystates[SDL_SCANCODE_SPACE] && cam.on_ground) {

            player.pc->vel.z += 4.0f;
            cam.on_ground = false;

        }
        if (keystates[SDL_SCANCODE_LSHIFT])
        {
            player.sprinting = true;
        }
        else
        {
            player.sprinting = false;
            
        }

    }

    // Apply velocity and resolve collisions
    if (move_to_spawn) return;
    player.pc->old_vel = player.pc->vel;
    if (!GLOBALS.noclip) {

        update_body_from_legs(&player.skeleton.left_leg, &player.skeleton.right_leg, move_dir, dt);
        if (!player.noclip)
        {
            resolve_collisions(&cam);
        }
        else
        {
            player.pos = (vec3_add(player.pos, vec3_scale(player.pc->vel, dt)));
            
        }
        update_skeleton(&player.skeleton, &cam, move_dir);
            
        
        
    } else {
        
        apply_velocity(&cam, move_dir, dt);
        if (!player.noclip)
        {
            
            resolve_collisions(&cam);
        }
        else
        {
            player.pos = (vec3_add(player.pos, vec3_scale(player.pc->vel, dt)));
        }

    }

}
