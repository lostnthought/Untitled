#include <math.h>
#include <omp.h>
#include <assert.h>
#include "core/acceleration.h"
#include "audio/audio.h"
#include "core/engine_types.h"
#include "mesh.h"
#include "utilities/types.h"
#include "utilities/mathf.h"
#include "core/camera.h"
#include "load/init.h"
#include "utilities/utilities.h"
#include <stdio.h>
#include "physics/physics.h"
#include "physics/GJK.h"


void
    compute_camera_basis
    (
        Camera * cam,
        float yaw,
        float pitch,
        float roll,
        vec3 world_up
    )

{
    
    float cos_p = cosf(pitch);
    cam->forward.x = cosf(yaw) * cos_p;
    cam->forward.y = sinf(yaw) * cos_p;
    cam->forward.z = sinf(pitch);
    cam->forward = vec3_normalize(cam->forward);

    vec3 right = vec3_cross(cam->forward, world_up);
    right = vec3_normalize(right);

    vec3 up = vec3_cross(right, cam->forward);
    up = vec3_normalize(up);
    if (fabsf(roll) > 0.0001f)
    {

        float cos_r = cosf(roll);
        float sin_r = sinf(roll);

        vec3 final_rgt = vec3_normalize(vec3_add(vec3_scale(right, cos_r), vec3_scale(up, sin_r)));
        vec3 final_up  = vec3_normalize(vec3_sub(vec3_scale(up, cos_r), vec3_scale(right, sin_r)));

        cam->right = final_rgt;
        cam->up = final_up;        
    }
    else
    {
        cam->right = right;
        cam->up = up;
    }

}

mat4
    make_ortho
    (
        float left,
        float right,
        float bottom,
        float top,
        float near,
        float far
    )

{

    float rl = right - left;
    float tb = top - bottom;
    float fn = far - near;
    
    mat4 out;

    out.a0 = 2.0f / rl;
    out.a1 = 0;
    out.a2 = 0;
    out.a3 = 0;

    out.b0  = 0;
    out.b1  = 2.0f / tb;
    out.b2  = 0;
    out.b3  = 0;

    out.c0  = 0;
    out.c1  = 0;
    out.c2  = -2.0f / fn;
    out.c3  = 0;

    out.d0  = -(right + left) / rl;
    out.d1  = -(top + bottom) / tb;
    out.d2  = -(far + near) / fn;
    out.d3  = 1.0f;
    return out;
}


mat4
    make_projection
    (
        float fov_deg,
        float aspect,
        float near,
        float far
    )

{

    float fov_rad = fov_deg * (M_PI / 180.0f);
    float f = 1.0f / tanf(fov_rad / 2.0f);
    mat4 out;
    out.a0 = f / aspect;
    out.a1 = 0;
    out.a2 = 0;
    out.a3 = 0;

    out.b0 = 0;
    out.b1 = f;
    out.b2 = 0;
    out.b3 = 0;

    out.c0 = 0;
    out.c1 = 0;
    out.c2 = -((far + near) / (far - near));
    out.c3 = -1;

    out.d0 = 0;
    out.d1 = 0;
    out.d2 = -(2.0f * far * near) / (far - near);
    out.d3 = 0;
    return out;
}


mat4
    make_look_at
    (
        Camera * cam,
        vec3     eye
    )

{

    vec3 f = cam->forward;
    vec3 s = cam->right;
    vec3 u = cam->up;

    mat4 out;
    out.a0  = s.x;
    out.a1  = u.x;
    out.a2  = -f.x;
    out.a3  = 0;

    out.b0  = s.y;
    out.b1  = u.y;
    out.b2  = -f.y;
    out.b3  = 0;

    out.c0  = s.z;
    out.c1  = u.z;
    out.c2 = -f.z;
    out.c3 = 0;


    out.d0 = -vec3_dot(s, eye);
    out.d1 = -vec3_dot(u, eye);
    out.d2 = vec3_dot(f, eye);

    out.d3 = 1;
    return out;
}


// TODO CLEAN THIS UP
void
    update_light_camera
    (
      Camera * lcam,
      Light * light
    )

{


    float bounds = 40.0f;
    vec3 cam_pos = (player.skeleton.neck.pos);

    vec3 offset = (vec3){0, 0, 0};

    vec3 pos = vec3_add(cam_pos, offset);

    vec3 sd = vec3_normalize((vec3){ light->dir.x, light->dir.y, light->dir.z });
    
    vec3 negated_sun_dir = {
        sd.x,
        sd.y,
        sd.z
    };
    

    float fov_rad = FOV * (M_PI / 180.0f);
    float near_height = tanf((float)fov_rad / 2) * -10.0f;
    float near_width = near_height * ((float)RENDER_WIDTH / RENDER_HEIGHT);
    float far_height = tanf((float)fov_rad / 2) * 10.0f;
    float far_width = far_height * ((float)RENDER_WIDTH / RENDER_HEIGHT);
    vec3 near_forward = vec3_scale(cam.forward, -10.0f);
    vec3 far_forward = vec3_scale(cam.forward, 10.0f);
    vec3 near_right = vec3_scale(cam.right, near_width);
    vec3 far_right = vec3_scale(cam.right, far_width);
    vec3 near_up = vec3_scale(cam.up, near_height);
    vec3 far_up = vec3_scale(cam.up, far_height);


    vec3 near_center = vec3_add(cam_pos,
                    vec3_scale(cam.forward, -10.0));    


    vec3 far_center = vec3_add(cam_pos,
                    vec3_scale(cam.forward, 10.0));    

    vec3 near_bl = vec3_sub(
                        vec3_sub(
                            near_center,
                            near_right),
                        near_up);
    vec3 near_ul = vec3_add(
                        vec3_sub(
                            near_center,
                            near_right),
                        near_up);
    vec3 near_br = vec3_sub(
                        vec3_add(
                            near_center,
                            near_right),
                        near_up);
    vec3 near_ur = vec3_add(
                        vec3_add(
                            near_center,
                            near_right),
                        near_up);
    vec3 far_bl = vec3_sub(
                        vec3_sub(
                            far_center,
                            far_right),
                        far_up);
    vec3 far_ul = vec3_add(
                        vec3_sub(
                            far_center,
                            far_right),
                        far_up);
    vec3 far_br = vec3_sub(
                        vec3_add(
                            far_center,
                            far_right),
                        far_up);
    vec3 far_ur = vec3_add(
                        vec3_add(
                            far_center,
                            far_right),
                        far_up);

    vec3 light_points[8];
    light_points[0] = mat4_mul_point(lcam->cam_data.view, near_bl);
    light_points[1] = mat4_mul_point(lcam->cam_data.view, near_ul);
    light_points[2] = mat4_mul_point(lcam->cam_data.view, near_br);
    light_points[3] = mat4_mul_point(lcam->cam_data.view, near_ur);
    light_points[4] = mat4_mul_point(lcam->cam_data.view, far_bl);
    light_points[5] = mat4_mul_point(lcam->cam_data.view, far_ul);
    light_points[6] = mat4_mul_point(lcam->cam_data.view, far_br);
    light_points[7] = mat4_mul_point(lcam->cam_data.view, far_ur);


    float slice_center_dist = (20.0f - -20.0f) / 2.0f; 
    vec3 frustum_center = vec3_add(cam_pos, vec3_scale(cam.forward, slice_center_dist));

    vec3 diff = vec3_sub(far_ur, frustum_center);
    float radius = sqrtf(diff.x*diff.x + diff.y*diff.y + diff.z*diff.z);

    float diameter = radius * 2.0f;
    float texel_size = diameter / (float)SHADOW_MAP_FBO.w;

    vec3 light_dir = vec3_normalize((vec3){ light->dir.x, light->dir.y, light->dir.z });
    vec3 pull_back = vec3_scale(light_dir, radius + 10.0f); 
    vec3 light_pos = vec3_sub(frustum_center, pull_back);


    vec3 world_up = (vec3){0, 1, 0};
    if (fabsf(vec3_dot(vec3_scale(light_dir, -1), world_up)) > 0.99f) {
        world_up = (vec3){0, 0, 1};
    }

    lcam->forward = vec3_normalize(vec3_scale(sd, -1));
    lcam->right = vec3_cross(lcam->forward, world_up);
    lcam->right = vec3_normalize(lcam->right);

    lcam->up = vec3_cross(lcam->right, lcam->forward);
    lcam->up = vec3_normalize(lcam->up);
    lcam->target = vec3_add(pos, lcam->forward);
    lcam->cam_data.view = make_look_at(lcam, pos);
    vec3 center_light_space = mat4_mul_point(lcam->cam_data.view, frustum_center);

    float snapped_x = floorf(center_light_space.x / texel_size) * texel_size;
    float snapped_y = floorf(center_light_space.y / texel_size) * texel_size;

    float offset_x = snapped_x - center_light_space.x;
    float offset_y = snapped_y - center_light_space.y;

    float minx = -radius + offset_x;
    float maxx =  radius + offset_x;
    float miny = -radius + offset_y;
    float maxy =  radius + offset_y;
    float minz = -20.0f; 
    float maxz = radius + 20.0f; 

    lcam->cam_data.proj = make_ortho(minx, maxx, miny, maxy, minz, maxz);
    

}



bool collision(vec3 proposed, vec3 pts[3], vec3 normal, CollisionPoints * p)
{
    
    float best = INFINITY;

    vec3 pt = closest_point_triangle(proposed, pts[0], pts[1], pts[2]);
    vec3 delta = vec3_sub(pt, proposed);
    float pt_dist = vec3_length(delta);
    if (pt_dist < 0.2f)
    {

        p->d = -(pt_dist - 0.2f);
    
        p->a = pt;
        return true;
    }

    return false;
}


static const float HALTON_8[8][2] = {
    { 0.000000f, -0.333333f },
    {-0.500000f,  0.333333f },
    { 0.250000f, -0.777778f },
    {-0.750000f, -0.111111f },
    { 0.125000f,  0.555556f },
    {-0.375000f, -0.555556f },
    { 0.375000f,  0.111111f },
    {-0.625000f,  0.777778f }
};

void
    update_camera
    (
        Skeleton * s,
        Camera * cam
    )

{

    cam->cam_data.cam_pos = vec3_to_vec4(s->neck.pos);
    if (GLOBALS.noclip)
    {
        cam->cam_data.cam_pos = vec3_to_vec4(player.pos);
    }

    if (GLOBALS.noclip)
    {
        
        compute_camera_basis(cam, cam->yaw, cam->pitch, cam->roll, (vec3){0, 0, 1});
    }
    else
    {
        
        compute_camera_basis(cam, s->neck.rotation.x, s->neck.rotation.y, s->neck.rotation.z, (vec3){0, 0, 1});
    }

    cam->cam_data.unjittered_proj = make_projection(
        FOV,
        (float)RENDER_WIDTH / (float)RENDER_HEIGHT,
        VIEW_NEAR,
        VIEW_FAR
    );


    if(EDITOR_SETTINGS.TAA)
    {
        
        int sample_idx = master_frame % 8;
        float jitter_x = HALTON_8[sample_idx][0];
        float jitter_y = HALTON_8[sample_idx][1];

        JITTER_OFFSET.x = (jitter_x / (float)RENDER_WIDTH)  * 2.0f;
        JITTER_OFFSET.y = (jitter_y / (float)RENDER_HEIGHT) * 2.0f;

        mat4 jittered_proj = cam->cam_data.unjittered_proj;
        jittered_proj.c0 += JITTER_OFFSET.x; 
        jittered_proj.c1 += JITTER_OFFSET.y; 

        cam->cam_data.proj = jittered_proj;

    }
    else
    {
        cam->cam_data.proj = cam->cam_data.unjittered_proj;
    }
            
    

    cam->target = vec3_add(
        (vec3){
            cam->cam_data.cam_pos.x,
            cam->cam_data.cam_pos.y,
            cam->cam_data.cam_pos.z
        },
        cam->forward
    );

    cam->cam_data.view = make_look_at(
        cam,
        (vec3){
            cam->cam_data.cam_pos.x,
            cam->cam_data.cam_pos.y,
            cam->cam_data.cam_pos.z
        }
    );

    assert(fabsf(vec3_length(cam->right) - 1.0f) < 0.001f);
    assert(fabsf(vec3_length(cam->up) - 1.0f) < 0.001f);
    assert(fabsf(vec3_length(cam->forward) - 1.0f) < 0.001f);

    m4_inverse((float*)&cam->cam_data.inv_view, (const float *)&cam->cam_data.view);
    m4_inverse((float*)&cam->cam_data.inv_proj, (const float *)&cam->cam_data.proj);
    m4_inverse((float*)&cam->cam_data.unjittered_inv_proj, (const float *)&cam->cam_data.unjittered_proj);

}

float calculate_stride_governor(vec3 player_pos, LegIK *left_leg, LegIK *right_leg)
{
    vec3 support_pos;
    bool both_planted = !left_leg->stepping && !right_leg->stepping;
    
    if (both_planted) {
        support_pos = vec3_scale(vec3_add(left_leg->pos, right_leg->pos), 0.5f);
    } else if (left_leg->stepping) {
        support_pos = right_leg->pos;
    } else {
        support_pos = left_leg->pos;
    }

    vec2 p2d = { player_pos.x, player_pos.y };
    vec2 s2d = { support_pos.x, support_pos.y };
    float stretch_dist = vec2_length(vec2_sub(p2d, s2d));

    const float min_stretch = 0.15f;
    const float max_stretch = 0.4f; 

    if (stretch_dist <= min_stretch) {
        return 1.0f; 
    }
    
    if (stretch_dist >= max_stretch) {
        return 0.0f; 
    }

    float t = (stretch_dist - min_stretch) / (max_stretch - min_stretch);
    return MIN(1.0f - t + 0.1, 1.0f); 
}
void
    apply_planet_velocity
    (
        Camera * cam,
        vec3     move_dir,
        float    dt
    )

{

    float stride = calculate_stride_governor(player.pos, &player.skeleton.left_leg, &player.skeleton.right_leg);
    // player.pc->vel.x *= stride;
    // player.pc->vel.y *= stride;
    // static const float MAX_SPEED = 30.0f;
    static const float MAX_SPEED = 150.0f;
    float ACCEL = player.sprinting ? 31.0f : 11.0f;
    static const float GRAVITY = 9.8f;
    float accel_step = ACCEL * dt;

    move_dir.x *= stride;
    move_dir.y *= stride;
    
    
    if (cam->on_ground)
    {
        
        player.pc->vel = vec3_scale(player.pc->vel, 0.85f);
    }

    vec3 fwd = {cosf(cam->yaw), sinf(cam->yaw), sinf(cam->pitch)};
    vec3 rgt = {-sinf(cam->yaw), cosf(cam->yaw), 0.0f};
    fwd = vec3_normalize(fwd);
    rgt = vec3_normalize(rgt);
    float wish_x = move_dir.x * fwd.x + move_dir.y * rgt.x;
    float wish_y = move_dir.x * fwd.y + move_dir.y * rgt.y;

    float cur = player.pc->vel.x * wish_x + player.pc->vel.y * wish_y;
    if (cam->on_ground)
    {
         float add = MAX_SPEED - cur;
         if (add)
         {
             float push = fminf(add, accel_step);
             player.pc->vel.x += wish_x * push;
             player.pc->vel.y += wish_y * push;
         }
    }
    else
    {
        player.pc->vel.z -= GRAVITY * dt;
    }


    float nlen = sqrtf(player.pc->vel.x * player.pc->vel.x + player.pc->vel.y * player.pc->vel.y);
    if (nlen > MAX_SPEED)
    {
        float scale = MAX_SPEED / nlen;
        player.pc->vel.x *= scale;
        player.pc->vel.y *= scale;
    }


}



void
    apply_velocity
    (
        Camera * cam,
        vec3     move_dir,
        float    dt
    )

{

    static const float MAX_SPEED = 30.0f;
    static const float ACCEL = 20.0f;
    float accel_step = ACCEL * dt;
    float s_len = sqrtf(move_dir.x * move_dir.x + move_dir.y * move_dir.y);
    if (s_len < 0.01f)
    {
        player.pc->vel = vec3_scale(player.pc->vel, 0.85f);
    }

    vec3 fwd = {cosf(cam->yaw), sinf(cam->yaw), sinf(cam->pitch)};
    vec3 rgt = {-sinf(cam->yaw), cosf(cam->yaw), 0.0f};
    fwd = vec3_normalize(fwd);
    rgt = vec3_normalize(rgt);
    float wish_x = move_dir.x * fwd.x + move_dir.y * rgt.x;
    float wish_y = move_dir.x * fwd.y + move_dir.y * rgt.y;
    float wish_z = move_dir.x * fwd.z + move_dir.y * rgt.z;

    float proj_wish = wish_x * rgt.x + wish_y * rgt.y + wish_z * rgt.z;
    if (s_len > 0.0f)
    {
        wish_x /= s_len;
        wish_y /= s_len;
        wish_z /= s_len;
    }
    float cur = player.pc->vel.x * wish_x + player.pc->vel.y * wish_y + player.pc->vel.z * wish_z;
    float add = MAX_SPEED - cur;
    if (add)
    {
        float push = fminf(add, accel_step);
        player.pc->vel.x += wish_x * push;
        player.pc->vel.y += wish_y * push;
        player.pc->vel.z += wish_z * push;
    }
}


bool ground_hit;





void player_find_collision(vec3 start, vec3 proposed, CollisionPoints p[MAX_PLAYER_CONTACTS], int * contact_count)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    ConvexHull * hull = CAST_VEC(player.mesh.hulls, ConvexHull);

    SceneData * sd = &GLOBALS.scene_data;

    vec3 dir = vec3_sub(proposed, start);
    vec3_normalize(dir);
    vec3 inv_dir = {1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z};
    CollisionData cd;
    cd.type = COL_HULL;
    cd.data.hull.pts = hull->vertices.buffer;
    cd.data.hull.count = hull->vertices.count;
    cd.data.hull.mfs = hull->hull_faces.buffer;
    cd.data.hull.face_count = hull->hull_faces.count;
    vec3 centered = proposed;
    centered.z -= 0.2f;
    mat4 mat = compose_trs_from_vectors(centered, (vec4){0,0,0,1}, (vec3){1,1,1});
    mat4 mat_inv;
    m4_inverse((float*)&mat_inv, (float*)&mat);
    int ni = 0;
    BVHNode * nodes = CAST_VEC(sd->bvh_nodes, BVHNode);
    AABB aabb = player.mesh.aabb;
    aabb = transform_aabb_by_position(aabb, centered);

    while (ni >= 0)
    {

        BVHNode * node = &nodes[ni];
        
        if (!aabbs_intersect(&node->aabb, &aabb))
        {
            ni = node->next;
            continue;
        }
        if (node->left == -1 && node->right == -1)
        {

            for (int i = 0; i < node->count; i++)
            {
                MeshNode * mn = &mns[sd->bvh_indices[node->first_index + i]];
                if ((mn->mesh == -1 && mn->parent != -1) || mn->free) continue;
                if (*contact_count >= MAX_PLAYER_CONTACTS)
                {
                    printf("MAX\n");
                    return;
                }
                if (mn->pc)
                {
                    bool already_pushed = false;
                    for (int j = 0; j < *contact_count; j++)
                    {
                        if (mn->pc == p[j].pcB) already_pushed = true;
                    }
                    if (already_pushed) continue;
                    switch(mn->pc->cd.type)
                    {
                        case COL_TRIANGLE:
                            {
                                if (mn->mesh == -1) continue;
                                Mesh * m = &meshes[mn->mesh];
                                ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);
                                if (m->bvh_prim)
                                {
                                    BVHNode * nodes = CAST_VEC(m->nodes, BVHNode);
        

                                    mat4 model = mn->global_matrix;
                                    mat4 mod;
                                    m4_inverse((float*)&mod, (float*)&model);
                                    vec3 tr = mat4_mul_point(mod, proposed);
                                    vec3 sr = mat4_mul_point(mod, start);
                                    vec3 td = mat4_mul_direction(mod, dir);
                                    vec3 inv_dir = {1.0f/td.x, 1.0f/td.y, 1.0f/td.z};
                                    BVHNode * node = &nodes[0];
            
                                    bool col = traverse_collision_bvh(m, mn, nodes, node, proposed, tr, &p[*contact_count]);
                                    if (p[*contact_count].collision)
                                    {

                                        p[*contact_count].mnB = mn->id;
                                        p[*contact_count].pcB = mn->pc;
                                        (*contact_count)++;
                                        continue;
                                    }
                                }
                            }
                            break;
                        case COL_HULL:
                            {
                                
                                if (mn->mesh == -1) continue;
                                Mesh * m = &meshes[mn->mesh];
                                ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);
                                for (int j = 0; j < m->hulls.count; j++)
                                {
                

                                    CollisionData cd0;
                                    cd0 = mn->pc->cd;
                
                                    cd0.data.hull.pts = hulls[j].vertices.buffer;
                                    cd0.data.hull.count = hulls[j].vertices.count;
                                    cd0.data.hull.mfs = hulls[j].hull_faces.buffer;
                                    cd0.data.hull.face_count = hulls[j].hull_faces.count;
                                    mat4 mat0 = mn->global_matrix;
                                    mat4 mat0_inv;
                                    m4_inverse((float*)&mat0_inv, (float*)&mat0);
                

                                    if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p[*contact_count], EW[0]))
                                    {


                                        p[*contact_count].mnB = mn->id;
                                        p[*contact_count].pcB = mn->pc;

                                        (*contact_count)++;
                    
                                        break;
                    
                    
                                    }
                                }
            
                            }
                            break;
                        case COL_CAPSULE:
                            CollisionData cd0 = mn->pc->cd;
                            mat4 mat0 = mn->global_matrix;
                            mat4 mat0_inv;
                            m4_inverse((float*)&mat0_inv, (float*)&mat0);
        

                            if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p[*contact_count], EW[0]))
                            {
                                printf("CC: %d\n", *contact_count);


                                p[*contact_count].mnB = mn->id;
                                p[*contact_count].pcB = mn->pc;

                                (*contact_count)++;
            
                                break;
            
            
                            }

                            break;
                    }
                }
                else
                {
                    Mesh * m = &meshes[mn->mesh];
                    ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);
                    if (m->bvh_prim)
                    {
                        BVHNode * nodes = CAST_VEC(m->nodes, BVHNode);
        

                        mat4 model = mn->global_matrix;
                        mat4 mod;
                        m4_inverse((float*)&mod, (float*)&model);
                        vec3 tr = mat4_mul_point(mod, proposed);
                        vec3 sr = mat4_mul_point(mod, start);
                        vec3 td = mat4_mul_direction(mod, dir);
                        vec3 inv_dir = {1.0f/td.x, 1.0f/td.y, 1.0f/td.z};
                        BVHNode * node = &nodes[0];
            
                        bool col = traverse_collision_bvh(m, mn, nodes, node, proposed, tr, &p[*contact_count]);
                        if (p[*contact_count].collision)
                        {

                            p[*contact_count].mnB = mn->id;
                            p[*contact_count].pcB = mn->pc;
                            (*contact_count)++;
                            continue;
                        }
            
                    }
                    else
                    {
                        for (int j = 0; j < m->hulls.count; j++)
                        {
                

                            CollisionData cd0;
                            if (mn->pc)
                            {
                                cd0 = mn->pc->cd;
                                if (cd0.type == COL_HULL)
                                {
                        
                                    cd0.data.hull.pts = hulls[j].vertices.buffer;
                                    cd0.data.hull.count = hulls[j].vertices.count;
                                    cd0.data.hull.mfs = hulls[j].hull_faces.buffer;
                                    cd0.data.hull.face_count = hulls[j].hull_faces.count;
                                }
                            }
                            else
                            {
                    
                                cd0.type = COL_HULL;
                                cd0.data.hull.pts = hulls[j].vertices.buffer;
                                cd0.data.hull.count = hulls[j].vertices.count;
                                cd0.data.hull.mfs = hulls[j].hull_faces.buffer;
                                cd0.data.hull.face_count = hulls[j].hull_faces.count;
                            }

                            mat4 mat0 = mn->global_matrix;
                            mat4 mat0_inv;
                            m4_inverse((float*)&mat0_inv, (float*)&mat0);
                

                            if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p[*contact_count], EW[0]))
                            {


                                p[*contact_count].mnB = mn->id;
                                p[*contact_count].pcB = mn->pc;

                                (*contact_count)++;
                    
                                break;
                    
                    
                            }
                        }
            
                    }
                    
                }
            }

            ni = node->next;
        }
        else
        {
            ni = node->left;
        }
    }
    
}






void reset_skeleton(Skeleton * s, vec3 pos)
{
    

    s->pelvis.pos = pos;
    s->pelvis.pos.z += s->pelvis_height;
    s->pelvis.velocity = (vec3){0};
    s->neck.pos = pos;
    s->neck.pos.z += s->neck_height;
    s->neck.velocity = (vec3){0};
    s->neck.angular_vel = (vec3){0};
    s->left_leg.stepping = false;
    s->left_leg.pos = pos;
    s->right_leg.pos = pos;
    s->right_leg.stepping = false;

}


void define_M(vec3 P, vec3 D, vec3 minv[3], vec3 mfwd[3])
{

    minv[0] = vec3_normalize(P);

    float dot_dx = vec3_dot(D, minv[0]);
    minv[1] = vec3_normalize(vec3_sub(D, vec3_scale(minv[0], dot_dx)));


    minv[2] = vec3_cross(minv[0], minv[1]);


    for (int i = 0; i < 3; i++)
    {
        mfwd[0] = (vec3){minv[0].x, minv[1].x, minv[2].x};
        mfwd[1] = (vec3){minv[0].y, minv[1].y, minv[2].y};
        mfwd[2] = (vec3){minv[0].z, minv[1].z, minv[2].z};
    }
    
}


vec3 rot(vec3 M[3], vec3 a)
{
    vec3 b;

    b.x = vec3_dot(M[0], a);
    b.y = vec3_dot(M[1], a);
    b.z = vec3_dot(M[2], a);
    
    return b;
}


// Calculates physical body velocity derived entirely from leg positions
void update_body_from_legs(LegIK *left_leg, LegIK *right_leg, vec3 intended_vel, float dt)
{
    vec3 support_pos;
    bool left_planted = !left_leg->stepping;
    bool right_planted = !right_leg->stepping;

    vec3 fwd = {cosf(cam.yaw), sinf(cam.yaw), sinf(cam.pitch)};
    vec3 rgt = {-sinf(cam.yaw), cosf(cam.yaw), 0.0f};
    fwd = vec3_normalize(fwd);
    rgt = vec3_normalize(rgt);
    float wish_x = intended_vel.x * fwd.x + intended_vel.y * rgt.x;
    float wish_y = intended_vel.x * fwd.y + intended_vel.y * rgt.y;
    vec3 fw = vec3_normalize((vec3){wish_x,wish_y, 0.0});

    
    if (left_leg->stepping_off_ledge && right_leg->stepping_off_ledge)
    {
        
        BVHNode * scene_nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);
        vec3 down_dir = {0,0,-1};
        vec3 inv_down_dir = {1.0f/down_dir.x, 1.0f/down_dir.y, 1.0f/down_dir.z};

        RayHit hit = raycast_scene(player.pos, down_dir, INFINITY, false);

        if (!hit.hit)
        {
            printf("NO HIT\n");
        }
        vec3 leg_pos;
        if (left_leg->stepping_off_ledge)
        {
            leg_pos = left_leg->pos;
        }
        else
        {
            
            leg_pos = right_leg->pos;
        }
        if (!hit.hit || player.pos.z - hit.pos.z > 1.5f)
        {
            player.pc->vel.z -= 9.8f * dt;
            left_leg->pos = player.pos;
            right_leg->pos = player.pos;
            
            return;
            
        }
            
    }
    if (left_planted && right_planted)
    {
        support_pos = vec3_scale(vec3_add(left_leg->pos, right_leg->pos), 0.5f);
    }
    else if (left_planted)
    {
        support_pos = left_leg->pos;
    }
    else if (right_planted)
    {
        support_pos = right_leg->pos;
    }
    else 
    {
        support_pos = player.pos;
    }
    vec3 ideal_body_pos = support_pos;
    

    vec3 displacement = vec3_sub(ideal_body_pos, player.pos);

    float stiffness = 50.0f; 
    float damping = 2.0f * sqrtf(stiffness); 

    vec3 spring_force = vec3_scale(displacement, stiffness);
    vec3 damping_force = vec3_scale(player.pc->vel, -damping);

    vec3 acceleration = vec3_add(spring_force, damping_force);

    player.pc->vel = vec3_add(player.pc->vel, vec3_scale(acceleration, dt));

}
bool update_leg(LegIK * leg, LegIK * other_leg, vec3 tr, vec3 vel)
{
    


    const float step_height = 0.3f;
    const float raycast_height = 0.4f;

    float len = MAX(vec3_length(vel), 0.1f);

    float min_speed = 0.1f;
    float current_speed = MAX(len, min_speed);

    float desired_stride = 0.6f;

    vec3 fwd = {cosf(cam.yaw), sinf(cam.yaw), sinf(cam.pitch)};
    vec3 rgt = {-sinf(cam.yaw), cosf(cam.yaw), 0.0f};
    fwd = vec3_normalize(fwd);
    rgt = vec3_normalize(rgt);
    float wish_x = vel.x * fwd.x + vel.y * rgt.x;
    float wish_y = vel.x * fwd.y + vel.y * rgt.y;
    vec2 fw = vec2_normalize((vec2){wish_x,wish_y});
    vec3 fw_vel = {fw.x, fw.y, 0.0};
    vec3 forward_lead = vec3_scale(fw_vel, desired_stride);

    vec3 ideal_target = vec3_add(other_leg->pos, forward_lead);

    vec3 sweep_start = leg->pos;
    sweep_start.z += step_height;
    vec3 sweep_end = ideal_target;
    sweep_end.z += step_height;

    vec3 horiz_dir = vec3_sub(sweep_end, sweep_start);
    float horiz_dist = vec3_length(horiz_dir);
    if (horiz_dist > 0.001f) horiz_dir = vec3_scale(horiz_dir, 1.0f / horiz_dist);
    vec3 inv_horiz_dir = {1.0f/horiz_dir.x, 1.0f/horiz_dir.y, 1.0f/horiz_dir.z};

    BVHNode * scene_nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);
    RayHit hit = raycast_scene(sweep_start, horiz_dir, horiz_dist, false);

    vec3 drop_start = (hit.hit) ? hit.pos : sweep_end;

    if (hit.hit)
    {
        drop_start = vec3_sub(drop_start, vec3_scale(horiz_dir, 0.05f));
    }

    vec3 down_dir = {0,0,-1};
    vec3 inv_down_dir = {1.0f/down_dir.x, 1.0f/down_dir.y, 1.0f/down_dir.z};
    

    hit = raycast_scene(drop_start, down_dir, INFINITY, false);

    vec3 final_target = hit.pos;
    if (!hit.hit || vec3_dist(hit.pos, leg->pos) > 3.0f)
    {
        leg->stepping_off_ledge = true;
        final_target = sweep_end;
        final_target.z -= step_height;
    }
    else
    {
        leg->stepping_off_ledge = false;
    }

    bool falling = false;
    if (leg->stepping)
    {
        
        leg->time += dt;
        float t = MIN(leg->time / leg->duration, 1.0f);
        vec3 current = vec3_lerp(leg->start_pos, leg->target_pos, t);

        float height_offset = 4.0f * step_height * t * (1.0f - t);
        leg->pos = current;
        leg->pos.z += height_offset;
        

        if (leg->time > leg->duration)
        {
            leg->stepping = false;
            leg->time = 0;
            play_emitter(FOOTSTEP_EMITTER, leg->pos);
            return true;
        }
    }
    else
    {
        float dist = vec3_length(vec3_sub(final_target, leg->pos));

        if (dist > desired_stride * 0.4f && !(other_leg->stepping))
        {


            leg->stepping = true;
            leg->time = 0;
            leg->duration = clamp(dist / current_speed, 0.2f, 0.6f);
            leg->start_pos = leg->pos;
            leg->target_pos = final_target;

        }

    }
    
    return false;
}

// From Perlin's 2 joint IK implementation
float moving_offset = 0;
bool solve_Q(Skeleton * s, vec3 D, vec3 * Q, vec3 * P, vec3 hip_pos )
{

    vec3 HIP_POS = hip_pos;

    *P = vec3_sub(*P, HIP_POS);

    float A = s->upper_leg_length;
    float B = s->lower_leg_length;

    vec3 MINV[3], MFWD[3];

    define_M(*P, D, MINV, MFWD);

    vec3 R = rot(MINV, *P);

    
    float C = sqrtf(vec3_dot(R, R));
    float d = MAX(0, MIN(A, (C + (A * A - B * B) / C) / 2));

    float E = sqrtf(A * A - d * d);

    vec3 S = {d, E, 0};

    *Q = rot(MFWD, S);

    *P = vec3_add(*P, HIP_POS);
    *Q = vec3_add(*Q, HIP_POS);

    

    return (d > 0 && d < A);
    
}




void update_spring(Spring * s)
{

    vec3 diff = vec3_sub(s->pos, s->target);
    vec3 F = vec3_scale(diff, -s->stiff);
    vec3 damp = vec3_scale(s->velocity, -s->damp);

    vec3 acc = vec3_add(F, damp);

    s->velocity = vec3_add(s->velocity, vec3_scale(acc, dt));
    s->pos = vec3_add(s->pos, vec3_scale(s->velocity, dt));
    
}

void update_angular_spring(Spring * s, float dt) {

    vec3 displacement = vec3_sub(s->rotation, s->target_rot);
    
    displacement.x = wrap_angle_pi(displacement.x);
    displacement.y = wrap_angle_pi(displacement.y);
    displacement.z = wrap_angle_pi(displacement.z);

    vec3 spring_torque = vec3_scale(displacement, -s->stiff_rot);
    vec3 damping_torque = vec3_scale(s->angular_vel, -s->damp_rot);
    
    vec3 total_torque = vec3_add(spring_torque, damping_torque); 

    s->angular_vel = vec3_add(s->angular_vel, vec3_scale(total_torque, dt));
    s->rotation = vec3_add(s->rotation, vec3_scale(s->angular_vel, dt));

}


void calculate_camera_angles(Camera * cam, Spring* s, vec3 input_acceleration, float dt) {

    vec3 base_look_rot = (vec3){cam->yaw, cam->pitch, cam->roll}; 
    
    float right_accel = vec3_dot(cam->right, input_acceleration);
    
    float target_roll = right_accel * 0.01f; 
    
    s->target_rot = base_look_rot;
    s->target_rot.z = target_roll;
    
    update_angular_spring(s, dt);
    
}

void update_skeleton(Skeleton * s, Camera * cam, vec3 vel)
{

    

    mat3 rot;
    rot.a0 = cam->forward.x;
    rot.a1 = cam->forward.y;
    rot.a2 = cam->forward.z;
    rot.b0 = cam->right.x;
    rot.b1 = cam->right.y;
    rot.b2 = cam->right.z;
    rot.c0 = cam->up.x;
    rot.c1 = cam->up.y;
    rot.c2 = cam->up.z;

    vec3 left_tr = mat3_mul_vec3(rot, left_leg_offset);
    vec3 right_tr = mat3_mul_vec3(rot, right_leg_offset);

    s->left_hip_pos = vec3_add(s->pelvis.pos, left_tr);
    s->left_hip_pos.z -= 0.02f;

    s->right_hip_pos = vec3_add(s->pelvis.pos, right_tr);
    s->right_hip_pos.z -= 0.02f;

    bool hit_floor = false;
        
    bool left_hit_floor = update_leg(&s->left_leg, &s->right_leg, left_tr, vel);
    bool right_hit_floor = update_leg(&s->right_leg, &s->left_leg, right_tr, vel);

    s->p_left = s->left_leg.pos;
    s->p_left = vec3_add(s->p_left, left_tr);
    s->p_right = s->right_leg.pos;
    s->p_right = vec3_add(s->p_right, right_tr);
    
    vec3 D = vec3_normalize((vec3){cam->forward.x,cam->forward.y,0});
    bool solved = solve_Q(&player.skeleton, D, &s->q_left, &s->p_left, s->left_hip_pos);
    bool solved2 = solve_Q(&player.skeleton, D, &s->q_right, &s->p_right, s->right_hip_pos);


    float leg_length = player.skeleton.hip_height * 0.95f;

    float left_length;
    float right_length;


    left_length = vec3_length(vec3_sub(s->p_left, s->left_hip_pos));
    right_length = vec3_length(vec3_sub(s->p_right, s->right_hip_pos));
    
    float left_drop = MAX(left_length - leg_length, 0);

    float right_drop = MAX(right_length - leg_length, 0);

    float vel_len = vec3_length(player.pc->vel);
    
    float drop = MAX(left_drop, right_drop);

    bool left_drop_higher = left_drop > right_drop;

    vec3 base_pos = vec3_scale(vec3_add(s->left_leg.pos, s->right_leg.pos), 0.5);
    s->pelvis.target = base_pos;
    s->pelvis.target.z -= drop;
    s->pelvis.target.z += s->pelvis_height;
    s->pelvis.target.z = MAX(s->pelvis.target.z, base_pos.z);
    update_spring(&s->pelvis);
    

    float pelvis_drop_delta = (base_pos.z + s->pelvis_height) - s->pelvis.pos.z;
    vec3 neck_target = s->pelvis.pos;
    neck_target.z += s->neck_pelvis_diff + (pelvis_drop_delta * 0.5f);
    s->neck.target = neck_target;
    update_spring(&s->neck);

    
    vec3 current_accel;
    current_accel.x = (player.pc->vel.x - player.pc->old_vel.x) / dt;
    current_accel.y = (player.pc->vel.y - player.pc->old_vel.y) / dt;
    current_accel.z = (player.pc->vel.z - player.pc->old_vel.z) / dt;    

    player.pc->accel = vec3_add(vec3_scale(player.pc->accel, 1.0f - ACCEL_BETA), vec3_sub(player.pc->vel, player.pc->old_vel));

    calculate_camera_angles(cam, &s->neck, player.pc->accel, dt);

    
}





#define MIN_WALKABLE_SURFACE 0.5f ///< How steep the surface is for it to be walkable
#define STEP_HEIGHT 0.6f ///< How high legs are raised during step
#define RAYCAST_HEIGHT 0.6f ///< How high legs are raised to detect for steps


void
    resolve_collisions
    (
        Camera * cam
    )

{

    ground_hit = false;

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);


    vec3 cam_pos = player.pos;
    vec3 cam_vel = player.pc->vel;
    vec3 proposed = vec3_add(cam_pos, vec3_scale(cam_vel, dt));
    vec3 raycast_proposed = cam_pos;
    raycast_proposed.z += RAYCAST_HEIGHT;

    vec3 dir = {0,0,-1};
    vec3 inv_dir= {1.0f/dir.x, 1.0f/dir.y, 1.0f/dir.z};
    BVHNode * scene_nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);

    Mesh * m = &meshes[0];
    MeshNode * mn = &mns[0];

    MeshFace * mfs = CAST_VEC(m->tris, MeshFace);
    Point * pts = CAST_VEC(m->vertices, Point);
    
    RayHit hit = raycast_scene(raycast_proposed, dir, INFINITY, true);
    
    CollisionPoints p[MAX_PLAYER_CONTACTS];
    memset(p, 0, sizeof(CollisionPoints) * MAX_PLAYER_CONTACTS);
    for (int i = 0; i < MAX_PLAYER_CONTACTS; i++)
    {

        p[i].d = -INFINITY;
        
    }
    CollisionPoints * current_p = p;
    int current_count = 0;


    int contact_count = 0;
    player_find_collision(cam_pos, proposed, p, &contact_count);

    bool all_walkable = true;
    float current_height = 0.0f;
    for (int i = 0; i < contact_count; i++)
    {
        CollisionPoints * pt = &p[i];
        MeshNode * mn = &mns[pt->mnB];
        if (pt->normal.z <= MIN_WALKABLE_SURFACE)
        {
            all_walkable = false;

        }
        else
        {
            current_height = pt->a.z;
        }
    }


    bool is_resolved = false;
    bool on_ground = false;
    vec3 last_proposed = cam_pos;
    for (int i = 0; i < contact_count; i++)
    {
        CollisionPoints * pt = &p[i];
        MeshNode * mn = &mns[pt->mnB];
        if (pt->normal.z > MIN_WALKABLE_SURFACE)
        {
            on_ground = true;
            ground_hit = true;
            player.cam->on_ground = true;
            player.pc->vel.z = 0;
        }
    
        if (mn->pc != NULL)
        {
    
            mn->pc->at_rest = false;
        }
        int parent = mn->parent;
        bool impulse = false;
        while (parent != -1)
        {
            MeshNode * mn0 = &mns[parent];
            if (mn0->pc)
            {

                mn0->pc->at_rest = false;

                pt->b = mat4_mul_point(mn->local_matrix, pt->b);
        
                pt->mnB = mn0->id;
                break;
            }
            parent = mn0->parent;
        }
        if (impulse == true) break;
    }
    for (int i = 0; i < 12; i++)
    {
        
        for (int j = 0; j < contact_count; j++)
        {
            CollisionPoints * pt = &p[j];
            MeshNode * mn = &mns[pt->mnB];
            if (mn->static_mesh)
            {
                player_resolve_impulse_static(player.pc, pt);

            }
            else
            {
                player_resolve_impulse(player.pc, mn->pc, pt);
            }
        
        }
    }


    
    player.pos = vec3_add(cam_pos, vec3_scale(player.pc->vel, dt));
    if (!on_ground) player.cam->on_ground = false;

    
}
