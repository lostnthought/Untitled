
#include "core/camera.h"

#include "utilities/mathf.h"
#include "mesh.h"
#include "utilities/types.h"
#include "utilities/utilities.h"
#include <math.h>
#include "render/render.h"
#include "load/init.h"


#define MIN_LEAF_AMT 4


void swap_size_t(size_t * a, size_t * b)
{
    size_t c = *a;
    *a = *b;
    *b = c;
}



void assign_next_nodes(BVHNode * nodes, int index, int next)
{

    BVHNode * node = &nodes[index];

    node->next = next;
    if (node->left != -1)
    {
        assign_next_nodes(nodes, node->left, node->right);

        assign_next_nodes(nodes, node->right, next);
    }
    
}


bool BVH_slab_test_aabb(RayHit * hit, AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir)
{
    
    float tx1 = (aabb.min_x - origin.x) * inv_dir.x;
    float tx2 = (aabb.max_x - origin.x) * inv_dir.x;
    float ty1 = (aabb.min_y - origin.y) * inv_dir.y;
    float ty2 = (aabb.max_y - origin.y) * inv_dir.y;
    float tz1 = (aabb.min_z - origin.z) * inv_dir.z;
    float tz2 = (aabb.max_z - origin.z) * inv_dir.z;
    float txmin = fminf(tx1, tx2);
    float txmax = fmaxf(tx1, tx2);
    float tymin = fminf(ty1, ty2);
    float tymax = fmaxf(ty1, ty2);
    float tzmin = fminf(tz1, tz2);
    float tzmax = fmaxf(tz1, tz2);

    float t_enter = fmaxf(txmin, fmaxf(tymin, tzmin));            

    float t_exit = fminf(txmax, fminf(tymax, tzmax));

    return (t_exit >= fmaxf(0.0f, t_enter) && t_enter < hit->t);
}

bool ray_intersects_aabb(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir)
{
    
    float tx1 = (aabb.min_x - origin.x) * inv_dir.x;
    float tx2 = (aabb.max_x - origin.x) * inv_dir.x;
    float ty1 = (aabb.min_y - origin.y) * inv_dir.y;
    float ty2 = (aabb.max_y - origin.y) * inv_dir.y;
    float tz1 = (aabb.min_z - origin.z) * inv_dir.z;
    float tz2 = (aabb.max_z - origin.z) * inv_dir.z;
    float txmin = fminf(tx1, tx2);
    float txmax = fmaxf(tx1, tx2);
    float tymin = fminf(ty1, ty2);
    float tymax = fmaxf(ty1, ty2);
    float tzmin = fminf(tz1, tz2);
    float tzmax = fmaxf(tz1, tz2);

    float t_enter = fmaxf(txmin, fmaxf(tymin, tzmin));            

    float t_exit = fminf(txmax, fminf(tymax, tzmax));

    if (t_enter > t_exit) return false;
    if (t_exit < 0) return false;

    return true;
}
bool ray_intersects_aabb_with_point(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir, vec3 * intersection_point)
{
    
    float tx1 = (aabb.min_x - origin.x) * inv_dir.x;
    float tx2 = (aabb.max_x - origin.x) * inv_dir.x;
    float ty1 = (aabb.min_y - origin.y) * inv_dir.y;
    float ty2 = (aabb.max_y - origin.y) * inv_dir.y;
    float tz1 = (aabb.min_z - origin.z) * inv_dir.z;
    float tz2 = (aabb.max_z - origin.z) * inv_dir.z;
    float txmin = fminf(tx1, tx2);
    float txmax = fmaxf(tx1, tx2);
    float tymin = fminf(ty1, ty2);
    float tymax = fmaxf(ty1, ty2);
    float tzmin = fminf(tz1, tz2);
    float tzmax = fmaxf(tz1, tz2);

    float t_enter = fmaxf(txmin, fmaxf(tymin, tzmin));            

    float t_exit = fminf(txmax, fminf(tymax, tzmax));

    if (t_enter > t_exit) return false;
    if (t_exit < 0) return false;

    if (t_enter < 0)
    {
        *intersection_point = vec3_add(origin, vec3_scale(dir, t_exit));
    }
    else
    {
        *intersection_point = vec3_add(origin, vec3_scale(dir, t_enter));
    }
    return true;
}



static const float RAY_EPS = 1e-6f;

bool ray_intersects_obb_with_point(OBB obb, vec3 origin, vec3 dir, vec3 * intersection_point) {

    vec3 axis_x = (vec3){obb.rot.a0, obb.rot.a1, obb.rot.a2};
    vec3 axis_y = (vec3){obb.rot.b0, obb.rot.b1, obb.rot.b2};
    vec3 axis_z = (vec3){obb.rot.c0, obb.rot.c1, obb.rot.c2};

    vec3 center = obb.center;

    vec3 rel = vec3_sub(origin, center);

    vec3 local_orig = {
        vec3_dot(rel, axis_x),
        vec3_dot(rel, axis_y),
        vec3_dot(rel, axis_z)
    };

    vec3 norm_dir = dir;
    norm_dir = vec3_normalize(norm_dir);

    vec3 local_dir = {
        vec3_dot(norm_dir, axis_x),
        vec3_dot(norm_dir, axis_y),
        vec3_dot(norm_dir, axis_z)
    };
    float tmin = -INFINITY;
    float tmax =  INFINITY;

    if (fabsf(local_dir.x) < RAY_EPS) {
        if (local_orig.x < -obb.extents.x || local_orig.x > obb.extents.x) return false;
    } else {
        float invd = 1.0f / local_dir.x;
        float t1 = (-obb.extents.x - local_orig.x) * invd;
        float t2 = ( obb.extents.x - local_orig.x) * invd;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    if (fabsf(local_dir.y) < RAY_EPS) {
        if (local_orig.y < -obb.extents.y || local_orig.y > obb.extents.y) return false;
    } else {
        float invd = 1.0f / local_dir.y;
        float t1 = (-obb.extents.y - local_orig.y) * invd;
        float t2 = ( obb.extents.y - local_orig.y) * invd;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    if (fabsf(local_dir.z) < RAY_EPS) {
        if (local_orig.z < -obb.extents.z || local_orig.z > obb.extents.z) return false;
    } else {
        float invd = 1.0f / local_dir.z;
        float t1 = (-obb.extents.z - local_orig.z) * invd;
        float t2 = ( obb.extents.z - local_orig.z) * invd;
        if (t1 > t2) { float tmp = t1; t1 = t2; t2 = tmp; }
        if (t1 > tmin) tmin = t1;
        if (t2 < tmax) tmax = t2;
        if (tmin > tmax) return false;
    }

    if (tmax < 0.0f) return false;

    if (tmin < 0.0f)
    {
        *intersection_point = vec3_add(origin, vec3_scale(dir, tmax));
        return true;
    }

    *intersection_point = vec3_add(origin, vec3_scale(dir, tmin));

    return true;

}



void ray_traverse_mesh_bvh(Mesh * m, MeshNode * mn, BVHNode * nodes, vec3 origin, vec3 inv_pos, vec3 dir, vec3 mod_dir, vec3 inv_dir, RayHit * hit, size_t mesh, size_t tri, bool check_normal)
{

    int ni = 0;
    while (ni >= 0)
    {
        BVHNode * node = &nodes[ni];
        if (!BVH_slab_test_aabb(hit, node->aabb, inv_pos, dir, inv_dir))
        {
            ni = node->next;
            continue;
        }

        if (node->left == -1 && node->right == -1)
        {
            size_t * mesh_indices = m->bvh_indices;

            MeshFace * mfs = CAST_VEC(m->tris, MeshFace);
            Point * pts = CAST_VEC(m->vertices, Point);
        
            for (int j = 0; j < node->count; j++)
            {
                MeshFace * mf = &mfs[mesh_indices[j + node->first_index]];
                vec3 pA = pts[mf->indices[0]].pos;
                vec3 pB = pts[mf->indices[1]].pos;
                vec3 pC = pts[mf->indices[2]].pos;

                float tri_t;
                vec3 hit_pos;
                vec3 b = {0,0,0};
                if (ray_intersects_triangle(
                    inv_pos,
                    mod_dir,
                    pA,
                    pB,
                    pC,
                    &tri_t, &hit_pos, &b)) {

                    bool col = false;
                    if (check_normal)
                    {
                        col = tri_t < hit->t && vec3_dot(mf->normal, mod_dir) < 0;
                    }
                    else
                    {
                        col = tri_t < hit->t;
                    }
                    if (col && tri_t < hit->t) {
                        hit->pos = hit_pos;
                        hit->t = tri_t;
                        hit->mesh_node_idx = mn->id;
                        hit->tri = mesh_indices[j + node->first_index];
                        hit->b0 = b.x;
                        hit->b1 = b.y;
                        hit->b2 = b.z;
                        hit->hit = HIT_STATIC;
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


bool ray_traverse_scene_bvh(SceneData * sd, BVHNode * nodes, vec3 origin, vec3 inv_pos, vec3 dir, vec3 mod_dir, vec3 inv_dir, RayHit * hit, size_t mesh, size_t tri, bool check_normal)
{

    int ni = 0;
    while (ni >= 0)
    {
        
        BVHNode * node = &nodes[ni];
        if (!BVH_slab_test_aabb(hit, node->aabb, origin, dir, inv_dir))
        {
            ni = node->next;
            continue;
        }

        if (node->left == -1 && node->right == -1)
        {
            Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
            MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        
            for (int i = 0; i < node->count; i++)
            {
                MeshNode * mn = &mns[sd->bvh_indices[i + node->first_index]];
                if (mn->mesh == -1 || mn->free) continue;
                Mesh * m = &meshes[mn->mesh];
                BVHNode * mnodes = CAST_VEC(m->nodes, BVHNode);

                mat4 mod;
                m4_inverse((float*)&mod, (float*)&mn->global_matrix);
                vec3 pm = mat4_mul_point(mod, origin);
                vec3 md = mat4_mul_direction(mod, mod_dir);
                vec3 inv_md = (vec3){1.0f/md.x, 1.0f/md.y, 1.0f/md.z};
                if (m->static_mesh)
                {
                    ray_traverse_mesh_bvh(m, mn, mnodes, origin, pm, dir, md, inv_md, hit, mesh, tri, check_normal);
                }
                else
                {
                    vec3 intersection;
                    AABB a = transform_aabb_by_model_matrix(m->aabb, mn->global_matrix);
                    aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &a, (vec3){0,0,0});
                    for (int j = 0; j < 24; j++)
                    {
                        DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0,1,0};
                    }
                    DEBUG_LINE_COUNT += 24;
                    if (ray_intersects_aabb_with_point(m->aabb, pm, dir, inv_md, &intersection))
                    {


                        float t = vec3_length(vec3_sub(origin, intersection));
                        if (t < hit->t)
                        {

                            hit->pos = intersection;
                            hit->t = t;
                            hit->mesh_node_idx = sd->bvh_indices[i + node->first_index];
                            hit->hit = HIT_DYNAMIC;
                            
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
    return false;
    
}



RayHit raycast_scene(vec3 start, vec3 dir, float initial_t, bool check_normal)
{

    RayHit hit;
    hit.hit = 0;
    hit.t = initial_t;
    BVHNode * nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);
    vec3 inv_dir = {(dir.x == 0) ? 100000.0f : 1.0f/dir.x, (dir.y == 0) ? 100000.0f : 1.0f/dir.y, (dir.z == 0) ? 100000.0f : 1.0f/dir.z};

    ray_traverse_scene_bvh(&GLOBALS.scene_data, nodes, start, start, dir, dir, inv_dir, &hit, 0, 0, check_normal);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    if (hit.hit && hit.mesh_node_idx >= 0 && hit.mesh_node_idx < GLOBALS.mesh_nodes.count)
    {
        hit.pos = mat4_mul_point(mns[hit.mesh_node_idx].global_matrix, hit.pos);
    }

    return hit;
    
}



bool ray_intersects_collision_aabb(AABB aabb, vec3 origin, vec3 end, vec3 dir, vec3 inv_dir)
{
    float dist = vec3_length(vec3_sub(end, origin));
    
    float tx1 = (aabb.min_x - origin.x) * inv_dir.x;
    float tx2 = (aabb.max_x - origin.x) * inv_dir.x;
    float ty1 = (aabb.min_y - origin.y) * inv_dir.y;
    float ty2 = (aabb.max_y - origin.y) * inv_dir.y;
    float tz1 = (aabb.min_z - origin.z) * inv_dir.z;
    float tz2 = (aabb.max_z - origin.z) * inv_dir.z;
    float txmin = fminf(tx1, tx2);
    float txmax = fmaxf(tx1, tx2);
    float tymin = fminf(ty1, ty2);
    float tymax = fmaxf(ty1, ty2);
    float tzmin = fminf(tz1, tz2);
    float tzmax = fmaxf(tz1, tz2);

    float t_enter = fmaxf(txmin, fmaxf(tymin, tzmin));            

    float t_exit = fminf(txmax, fminf(tymax, tzmax));

    if (t_enter > t_exit) return false;
    if (t_exit < 0) return false;

    return true;
}






bool process_collision_node(Mesh * m, MeshNode * mn, BVHNode * node, vec3 proposed, CollisionPoints * p)
{
    
    MeshFace * fs = CAST_VEC(m->tris, MeshFace);
    Point * v = CAST_VEC(m->vertices, Point);
    size_t * li = m->bvh_indices;
    int tris = 0;
    for (int k = 0; k < node->count; k++)
    {

        MeshFace * f = &fs[li[k + node->first_index]];
        mat4 model = mn->global_matrix;
        mat4 tp = m4_transpose(model);
        mat4 inv;
        m4_inverse((float*)&inv, (float*)&tp);
        vec3 A = mat4_mul_point(mn->global_matrix, v[f->indices[0]].pos);
        vec3 B = mat4_mul_point(mn->global_matrix, v[f->indices[1]].pos);
        vec3 C = mat4_mul_point(mn->global_matrix, v[f->indices[2]].pos);
        vec3 nr = vec3_normalize(mat4_mul_direction(inv, f->normal));

        tris++;
        CollisionPoints pt;
        if (collision(proposed, (vec3[]){A, B, C}, nr, &pt))
        {
            if (pt.d > p->d)
            {
                *p = pt;
                p->bias_impulse = 0;
                p->normal_impulse = 0;
                p->tangent_impulse = 0;
                p->d = pt.d;
                p->normal = nr;
                p->collision = true;
                p->mnB = mn->id;
                p->pcB = mn->pc;
            }
        }

    }
    return false;
}



// TODO make this non-recursive
bool traverse_collision_bvh(Mesh * m, MeshNode * mn, BVHNode * nodes, BVHNode * node, vec3 proposed, vec3 inv_p, CollisionPoints * p)
{

    AABB aabb = node->aabb;

    float eps = 0.21f;

    aabb.min_x -= eps;
    aabb.min_y -= eps;
    aabb.min_z -= eps;
    aabb.max_x += eps;
    aabb.max_y += eps;
    aabb.max_z += eps;
    
    
    if (!point_in_aabb(inv_p, &aabb))
    {
        return false;
    }

    if (node->left == -1 && node->right == -1)
    {
        process_collision_node(m, mn, node, proposed, p);
    }
    
    if (node->left >= 0)
    {
        
        BVHNode * left = &nodes[node->left];

            traverse_collision_bvh(m, mn, nodes, left, proposed, inv_p, p);
    }
    if (node->right >= 0)
    {
        
        BVHNode * right = &nodes[node->right];

            traverse_collision_bvh(m, mn, nodes, right, proposed, inv_p, p);
    }
    return false;
    
}






bool subdivide_scene_bvh(Vector * bvh_nodes, size_t * indices, AABB * aabbs, int pidx, int depth, int axes_tried)
{
    

    BVHNode * nodes = CAST_VEC(*bvh_nodes, BVHNode);
    BVHNode * parent = &nodes[pidx];
    
    AXIS longest;
    if (parent->aabb.extents.x >= parent->aabb.extents.y && parent->aabb.extents.x >= parent->aabb.extents.z)
    {
        longest = X;
    }
    if (parent->aabb.extents.y >= parent->aabb.extents.x && parent->aabb.extents.y >= parent->aabb.extents.z) longest = Y;
    if (parent->aabb.extents.z >= parent->aabb.extents.x && parent->aabb.extents.z >= parent->aabb.extents.y) longest = Z;

    AXIS second_longest;
    AXIS third_longest;
    switch(longest)
    {
        case X:

            if (parent->aabb.extents.y > parent->aabb.extents.z)
            {
                second_longest = Y;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Y:
            if (parent->aabb.extents.x > parent->aabb.extents.z)
            {
                second_longest = X;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = X;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Z:
            if (parent->aabb.extents.y > parent->aabb.extents.x)
            {
                second_longest = Y;
                third_longest = X;
            }
            else
            {
                second_longest = X;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
    }

        
    BVHNode left_child = {0}, right_child = {0};
    left_child.parent = pidx;
    right_child.parent = pidx;

    vec3 split_pos = parent->aabb.center;
    
    int i = parent->first_index;
    int j = parent->first_index + parent->count - 1;
            
    switch(longest)
    {
        case X:

            {
                float split_pos = parent->aabb.min_x + parent->aabb.extents.x;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.x < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }

            }
            break;
        case Y:
            {
                float split_pos = parent->aabb.min_y + parent->aabb.extents.y;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.y < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }
            }
            break;
        case Z:
            {
                float split_pos = parent->aabb.min_z + parent->aabb.extents.z;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.z < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }
            }
            break;
    }

    size_t left_count = i - parent->first_index;
    left_child.first_index = parent->first_index;
    left_child.count = left_count;
    right_child.first_index = i;
    right_child.count = parent->count - left_count;



    left_child.aabb.min_x = INFINITY;
    left_child.aabb.min_y = INFINITY;
    left_child.aabb.min_z = INFINITY;
    left_child.aabb.max_x = -INFINITY;
    left_child.aabb.max_y = -INFINITY;
    left_child.aabb.max_z = -INFINITY;

    AABB * left_aabb = &left_child.aabb;

    for (int k = left_child.first_index; k < left_child.first_index + left_child.count; k++)
    {
        AABB tr = aabbs[indices[k]];

        if (tr.min_x < left_aabb->min_x) left_aabb->min_x = tr.min_x;
        if (tr.min_y < left_aabb->min_y) left_aabb->min_y = tr.min_y;
        if (tr.min_z < left_aabb->min_z) left_aabb->min_z = tr.min_z;
        if (tr.max_x > left_aabb->max_x) left_aabb->max_x = tr.max_x;
        if (tr.max_y > left_aabb->max_y) left_aabb->max_y = tr.max_y;
        if (tr.max_z > left_aabb->max_z) left_aabb->max_z = tr.max_z;
        
    }

    calculate_aabb_extents(left_aabb);
    

    right_child.aabb.min_x = INFINITY;
    right_child.aabb.min_y = INFINITY;
    right_child.aabb.min_z = INFINITY;
    right_child.aabb.max_x = -INFINITY;
    right_child.aabb.max_y = -INFINITY;
    right_child.aabb.max_z = -INFINITY;

    AABB * right_aabb = &right_child.aabb;

    for (int k = right_child.first_index; k < right_child.first_index + right_child.count; k++)
    {
        AABB tr = aabbs[indices[k]];
        if (tr.min_x < right_aabb->min_x) right_aabb->min_x = tr.min_x;
        if (tr.min_y < right_aabb->min_y) right_aabb->min_y = tr.min_y;
        if (tr.min_z < right_aabb->min_z) right_aabb->min_z = tr.min_z;
        if (tr.max_x > right_aabb->max_x) right_aabb->max_x = tr.max_x;
        if (tr.max_y > right_aabb->max_y) right_aabb->max_y = tr.max_y;
        if (tr.max_z > right_aabb->max_z) right_aabb->max_z = tr.max_z;
        

    }

    calculate_aabb_extents(right_aabb);
    


        
    const int split_margin = 0;
    bool can_split = (right_child.count > split_margin || left_count > split_margin);
    bool can_leaf = !(right_child.count > split_margin || left_count > split_margin) || axes_tried == 2;
    bool did_split = (left_count == 0 || right_child.count == 0);
    if (!can_split || axes_tried == 2)
    {
        parent = &nodes[pidx];

        size_t found_indices = 0;

        float eps = 0.01f;

        parent->aabb.min_x -= eps;
        parent->aabb.min_y -= eps;
        parent->aabb.min_z -= eps;
        parent->aabb.max_x += eps;
        parent->aabb.max_y += eps;
        parent->aabb.max_z += eps;

        calculate_aabb_extents(&parent->aabb);
        
        parent->left = -1;
        parent->right = -1;
        parent->next = -1;
        parent->index = bvh_nodes->count - 1;
        
        
        return false;
    }
    else if (did_split)
    {

        subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth, axes_tried + 1);
        return true;
        
    }

    parent = &nodes[pidx];
    

    left_child.left = -1;
    left_child.right = -1;
    BVHNode * left = vector_expand(bvh_nodes, 1);
    *left = left_child;
    left->index = bvh_nodes->count-1;
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];
    parent->left = bvh_nodes->count-1;
    bool divided = subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth + 1, 0);
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];

    right_child.left = -1;
    right_child.right = -1;
    BVHNode * right = vector_expand(bvh_nodes, 1);
    *right = right_child;
    right->index = bvh_nodes->count-1;
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];
    parent->right = bvh_nodes->count-1;
    divided = subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth + 1, 0);
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];

    return true;
    

    
}




void init_scene_bvh()
{

    SceneData * sd = &GLOBALS.scene_data;
    AABB * aabb = &sd->aabb;
    aabb->min_x = INFINITY;
    aabb->min_y = INFINITY;
    aabb->min_z = INFINITY;
    aabb->max_x = -INFINITY;
    aabb->max_y = -INFINITY;
    aabb->max_z = -INFINITY;
    
    Mesh * ms = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);


    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {

        MeshNode * mn = &mns[i];
        Mesh * m = &ms[mn->mesh];
        if (mn->free) continue;
        AABB a= transform_aabb_by_model_matrix(m->aabb, mn->global_matrix);
        if (a.min_x < aabb->min_x) aabb->min_x = a.min_x;
        if (a.min_y < aabb->min_y) aabb->min_y = a.min_y;
        if (a.min_z < aabb->min_z) aabb->min_z = a.min_z;
        if (a.max_x > aabb->max_x) aabb->max_x = a.max_x;
        if (a.max_y > aabb->max_y) aabb->max_y = a.max_y;
        if (a.max_z > aabb->max_z) aabb->max_z = a.max_z;
        
    }
    float ex_x = fabsf(aabb->max_x - aabb->min_x) / 2;
    float ex_y = fabsf(aabb->max_y - aabb->min_y) / 2;
    float ex_z = fabsf(aabb->max_z - aabb->min_z) / 2;
    float mid_x = ex_x + aabb->min_x;
    float mid_y = ex_y + aabb->min_y;
    float mid_z = ex_z + aabb->min_z;
    
    aabb->center = (vec3){mid_x, mid_y, mid_z};
    aabb->extents = (vec3){ex_x, ex_y, ex_z};
    
    if (sd->bvh_indices) free(sd->bvh_indices);
    if (sd->leaf_indices.buffer) free(sd->leaf_indices.buffer);
    if (sd->bvh_nodes.buffer) free(sd->bvh_nodes.buffer);

    sd->bvh_indices = malloc(sizeof(size_t) * GLOBALS.mesh_nodes.count);
    AABB * aabbs = malloc(sizeof(AABB) * GLOBALS.mesh_nodes.count);
    init_vector(&sd->leaf_indices, 0, sizeof(size_t));
    init_vector(&sd->bvh_nodes, 0, sizeof(BVHNode));

    int bvh_indices_count = 0;
    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        if (mns[i].mesh == -1 || mns[i].free)
        {
            continue;
        }
        sd->bvh_indices[bvh_indices_count++] = i;
        Mesh * m = &ms[mns[i].mesh];
        aabbs[i] = transform_aabb_by_model_matrix(m->aabb, mns[i].global_matrix);
    }

    sd->bvh_indices_count = bvh_indices_count;


    BVHNode * root = vector_expand(&sd->bvh_nodes, 1);
    root->left = -1;
    root->right = -1;
    root->parent = -1;
    root->first_index = 0;
    root->index = 0;
    root->count = bvh_indices_count;
    root->aabb = sd->aabb;
    root->next = -1;
    
    subdivide_scene_bvh(&sd->bvh_nodes, sd->bvh_indices, aabbs, 0, 0, 0);


    BVHNode * nodes = CAST_VEC(sd->bvh_nodes, BVHNode);
    assign_next_nodes(nodes, 0, -1);    
    
    free(aabbs);
}





bool subdivide_mesh_tri_bvh(Mesh * mesh, size_t * indices, int indices_count, int pidx, int depth, int axes_tried)
{
    
    BVHNode * nodes = CAST_VEC(mesh->nodes, BVHNode);
    BVHNode * parent = &nodes[pidx];
    size_t * bvh_indices = mesh->bvh_indices;
     
    
    AXIS longest;
    if (parent->aabb.extents.x >= parent->aabb.extents.y && parent->aabb.extents.x >= parent->aabb.extents.z) longest = X;
    if (parent->aabb.extents.y >= parent->aabb.extents.x && parent->aabb.extents.y >= parent->aabb.extents.z) longest = Y;
    if (parent->aabb.extents.z >= parent->aabb.extents.x && parent->aabb.extents.z >= parent->aabb.extents.y) longest = Z;
    
    AXIS second_longest;
    AXIS third_longest;
    switch(longest)
    {
        case X:

            if (parent->aabb.extents.y > parent->aabb.extents.z)
            {
                second_longest = Y;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Y:
            if (parent->aabb.extents.x > parent->aabb.extents.z)
            {
                second_longest = X;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = X;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Z:
            if (parent->aabb.extents.y > parent->aabb.extents.x)
            {
                second_longest = Y;
                third_longest = X;
            }
            else
            {
                second_longest = X;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
    }
    BVHNode left_child = {0}, right_child = {0};
    left_child.parent = pidx;
    right_child.parent = pidx;

    vec3 split_pos = parent->aabb.center;

    
    int i = parent->first_index;
    int j = parent->first_index + parent->count - 1;
    Point * pts = CAST_VEC(mesh->vertices, Point);
    MeshFace * faces = CAST_VEC(mesh->tris, MeshFace);
    AABB * aabbs = CAST_VEC(mesh->tri_aabbs, AABB);

    vec3 clip_plane;
    vec3 clip_point;
    



    size_t * left_indices = malloc(sizeof(size_t) * indices_count);
    size_t left_indices_count = 0;
    size_t * right_indices = malloc(sizeof(size_t) * indices_count);
    size_t right_indices_count = 0;
            
    switch(longest)
    {
        case X:

            {
                float split_pos = parent->aabb.min_x + parent->aabb.extents.x;
                while (i <= j)
                {
                    MeshFace * m = &faces[bvh_indices[i]];
                    AABB * aabb = &aabbs[bvh_indices[i]];
                    if (aabb->center.x < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&bvh_indices[i], &bvh_indices[j--]);
                    }
                }

            }
            break;
        case Y:
            {
                float split_pos = parent->aabb.min_y + parent->aabb.extents.y;

                while (i <= j)
                {
                    MeshFace * m = &faces[bvh_indices[i]];
                    AABB * aabb = &aabbs[bvh_indices[i]];
                    if (aabb->center.y < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&bvh_indices[i], &bvh_indices[j--]);
                    }
                }
            }
            break;
        case Z:
            {
                float split_pos = parent->aabb.min_z + parent->aabb.extents.z;

                while (i <= j)
                {
                    MeshFace * m = &faces[bvh_indices[i]];
                    AABB * aabb = &aabbs[bvh_indices[i]];
                    if (aabb->center.z < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&bvh_indices[i], &bvh_indices[j--]);
                    }
                }
            }
            break;
    }


    size_t left_count = i - parent->first_index;
    left_child.first_index = parent->first_index;
    left_child.count = left_count;
    right_child.first_index = i;
    right_child.count = parent->count - left_count;

    left_child.aabb.min_x = INFINITY;
    left_child.aabb.min_y = INFINITY;
    left_child.aabb.min_z = INFINITY;
    left_child.aabb.max_x = -INFINITY;
    left_child.aabb.max_y = -INFINITY;
    left_child.aabb.max_z = -INFINITY;

    AABB * left_aabb = &left_child.aabb;

    for (int k = left_child.first_index; k < left_child.first_index + left_child.count; k++)
    {

        AABB * aabb = &aabbs[bvh_indices[k]];

        if (aabb->min_x < left_aabb->min_x) left_aabb->min_x = aabb->min_x;
        if (aabb->min_y < left_aabb->min_y) left_aabb->min_y = aabb->min_y;
        if (aabb->min_z < left_aabb->min_z) left_aabb->min_z = aabb->min_z;
        if (aabb->max_x > left_aabb->max_x) left_aabb->max_x = aabb->max_x;
        if (aabb->max_y > left_aabb->max_y) left_aabb->max_y = aabb->max_y;
        if (aabb->max_z > left_aabb->max_z) left_aabb->max_z = aabb->max_z;
            
    }
    calculate_aabb_extents(left_aabb);
    

    right_child.aabb.min_x = INFINITY;
    right_child.aabb.min_y = INFINITY;
    right_child.aabb.min_z = INFINITY;
    right_child.aabb.max_x = -INFINITY;
    right_child.aabb.max_y = -INFINITY;
    right_child.aabb.max_z = -INFINITY;

    AABB * right_aabb = &right_child.aabb;

    for (int k = right_child.first_index; k < right_child.first_index + right_child.count; k++)
    {

        AABB * aabb = &aabbs[bvh_indices[k]];

        if (aabb->min_x < right_aabb->min_x) right_aabb->min_x = aabb->min_x;
        if (aabb->min_y < right_aabb->min_y) right_aabb->min_y = aabb->min_y;
        if (aabb->min_z < right_aabb->min_z) right_aabb->min_z = aabb->min_z;
        if (aabb->max_x > right_aabb->max_x) right_aabb->max_x = aabb->max_x;
        if (aabb->max_y > right_aabb->max_y) right_aabb->max_y = aabb->max_y;
        if (aabb->max_z > right_aabb->max_z) right_aabb->max_z = aabb->max_z;
        
    }

    calculate_aabb_extents(right_aabb);
    
    const int split_margin = 3;
    bool can_split = (right_child.count > split_margin || left_count > split_margin);
    bool can_leaf = !(right_child.count > split_margin || left_count > split_margin) || axes_tried == 2;
    bool did_split = (left_count == 0 || right_child.count == 0);
    if (!can_split || axes_tried == 2)
    {
        parent = &nodes[pidx];
        size_t first = mesh->leaf_indices.count;
        MeshFace * faces = CAST_VEC(mesh->tris, MeshFace);

        size_t found_indices = 0;

        float eps = 0.01f;

        parent->aabb.min_x -= eps;
        parent->aabb.min_y -= eps;
        parent->aabb.min_z -= eps;
        parent->aabb.max_x += eps;
        parent->aabb.max_y += eps;
        parent->aabb.max_z += eps;

        calculate_aabb_extents(&parent->aabb);
        
        parent->left = -1;
        parent->right = -1;
        parent->next = -1;
        parent->index = mesh->nodes.count - 1;
        
        free(left_indices);
        free(right_indices);
        
        
        return false;
    }
    else if (did_split)
    {

        bool divided = subdivide_mesh_tri_bvh(mesh, left_indices, left_indices_count, mesh->nodes.count-1, depth, axes_tried+1);
        
        return true;
    }

    size_t leaf_count = mesh->leaf_indices.count;
    parent = &nodes[pidx];
    

    left_child.left = -1;
    left_child.right = -1;
    BVHNode * left = vector_expand(&mesh->nodes, 1);
    *left = left_child;
    left->index = mesh->nodes.count-1;
    nodes = CAST_VEC(mesh->nodes, BVHNode);
    parent = &nodes[pidx];
    parent->left = mesh->nodes.count-1;
    bool divided = subdivide_mesh_tri_bvh(mesh, left_indices, left_indices_count, mesh->nodes.count-1, depth + 1, 0);
    nodes = CAST_VEC(mesh->nodes, BVHNode);
    parent = &nodes[pidx];

    right_child.left = -1;
    right_child.right = -1;
    BVHNode * right = vector_expand(&mesh->nodes, 1);
    *right = right_child;
    right->index = mesh->nodes.count-1;
    nodes = CAST_VEC(mesh->nodes, BVHNode);
    parent = &nodes[pidx];
    parent->right = mesh->nodes.count-1;
    divided = subdivide_mesh_tri_bvh(mesh, right_indices, right_indices_count, mesh->nodes.count-1, depth + 1, 0);
    nodes = CAST_VEC(mesh->nodes, BVHNode);
    parent = &nodes[pidx];

    free(left_indices);
    free(right_indices);
    return true;
    

    
}


// tri bvh over a mesh
void init_mesh_tri_bvh(Mesh * mesh)
{

    init_vector(&mesh->nodes, 0, sizeof(BVHNode));
    init_vector(&mesh->leaf_indices, 0, sizeof(size_t));

    BVHNode * root = vector_expand(&mesh->nodes, 1);

    root->aabb = mesh->aabb;
    root->first_index = 0;
    root->count = mesh->tris.count;
    root->left = -1;
    root->right = -1;
    root->parent = -1;
    root->next = -1;
    root->index = mesh->nodes.count-1;

    mesh->bvh_indices = malloc(sizeof(size_t) * mesh->tris.count);
    init_vector(&mesh->tri_aabbs, mesh->tris.count, sizeof(AABB));
    AABB * aabbs = CAST_VEC(mesh->tri_aabbs, AABB);
    MeshFace * mfs = CAST_VEC(mesh->tris, MeshFace);
    Point * pts = CAST_VEC(mesh->vertices, Point);
    size_t * indices = malloc(sizeof(size_t) * mesh->tris.count);
    for (int i = 0; i < mesh->tris.count; i++)
    {
        MeshFace * mf = &mfs[i];
        indices[i] = i;
        mesh->bvh_indices[i] = i;


        compute_aabb_from_points((vec3[]){pts[mf->indices[0]].pos, pts[mf->indices[1]].pos, pts[mf->indices[2]].pos}, 3, &aabbs[i]);

    }

    subdivide_mesh_tri_bvh(mesh, indices, mesh->tris.count, mesh->nodes.count-1, 0, 0);


    BVHNode * nodes = CAST_VEC(mesh->nodes, BVHNode);

    assign_next_nodes(nodes, 0, -1);
    

    free(indices);
    
    
}


bool subdivide_light_bvh(Vector * bvh_nodes, size_t * indices, AABB * aabbs, Light * lights, int pidx, int depth, int axes_tried)
{
    

    BVHNode * nodes = CAST_VEC(*bvh_nodes, BVHNode);
    BVHNode * parent = &nodes[pidx];
    
    AXIS longest;
    if (parent->aabb.extents.x >= parent->aabb.extents.y && parent->aabb.extents.x >= parent->aabb.extents.z)
    {
        longest = X;
    }
    if (parent->aabb.extents.y >= parent->aabb.extents.x && parent->aabb.extents.y >= parent->aabb.extents.z) longest = Y;
    if (parent->aabb.extents.z >= parent->aabb.extents.x && parent->aabb.extents.z >= parent->aabb.extents.y) longest = Z;

    AXIS second_longest;
    AXIS third_longest;
    switch(longest)
    {
        case X:

            if (parent->aabb.extents.y > parent->aabb.extents.z)
            {
                second_longest = Y;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Y:
            if (parent->aabb.extents.x > parent->aabb.extents.z)
            {
                second_longest = X;
                third_longest = Z;
            }
            else
            {
                second_longest = Z;
                third_longest = X;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
        case Z:
            if (parent->aabb.extents.y > parent->aabb.extents.x)
            {
                second_longest = Y;
                third_longest = X;
            }
            else
            {
                second_longest = X;
                third_longest = Y;
            }
            if (axes_tried == 1)
            {
                longest = second_longest;
            }
            else
            {
                longest = third_longest;
            }
            break;
    }

    
    // size_t * bvh_indices = sd->bvh_indices;
        
    BVHNode left_child = {0}, right_child = {0};
    left_child.parent = pidx;
    right_child.parent = pidx;

    vec3 split_pos = parent->aabb.center;
    
    int i = parent->first_index;
    int j = parent->first_index + parent->count - 1;
    // vec3 * pts = CAST_VEC(hull->vertices, vec3);
    // MeshFace * faces = CAST_VEC(hull->faces, MeshFace);
    // AABB * aabbs = CAST_VEC(mesh, AABB);


    // size_t * bvh_indices = CAST_VEC(sd->bvh_indices, size_t);
    vec3 clip_plane;
    vec3 clip_point;
            
    switch(longest)
    {
        case X:

            {
                float split_pos = parent->aabb.min_x + parent->aabb.extents.x;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.x < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }

            }
            break;
        case Y:
            {
                float split_pos = parent->aabb.min_y + parent->aabb.extents.y;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.y < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }
            }
            break;
        case Z:
            {
                float split_pos = parent->aabb.min_z + parent->aabb.extents.z;

                while (i <= j)
                {
                    AABB tr = aabbs[indices[i]];
                    if (tr.center.z < split_pos)
                    {
                        
                        i++;
                    }
                    else
                    {
                        swap_size_t(&indices[i], &indices[j--]);
                    }
                }
            }
            break;
    }

    size_t left_count = i - parent->first_index;
    left_child.first_index = parent->first_index;
    left_child.count = left_count;
    right_child.first_index = i;
    right_child.count = parent->count - left_count;



    left_child.aabb.min_x = INFINITY;
    left_child.aabb.min_y = INFINITY;
    left_child.aabb.min_z = INFINITY;
    left_child.aabb.max_x = -INFINITY;
    left_child.aabb.max_y = -INFINITY;
    left_child.aabb.max_z = -INFINITY;

    AABB * left_aabb = &left_child.aabb;

    float left_flux = 0.0f;
    float left_em_flux = 0.0f;
    for (int k = left_child.first_index; k < left_child.first_index + left_child.count; k++)
    {
        AABB tr = aabbs[indices[k]];
        Light * l = &lights[indices[k]];
        if (l->light_type == DIRECTIONAL_LIGHT) continue;

        if (l->light_type == AREA_LIGHT)
        {
            left_em_flux += l->power;
        }
        if (tr.min_x < left_aabb->min_x) left_aabb->min_x = tr.min_x;
        if (tr.min_y < left_aabb->min_y) left_aabb->min_y = tr.min_y;
        if (tr.min_z < left_aabb->min_z) left_aabb->min_z = tr.min_z;
        if (tr.max_x > left_aabb->max_x) left_aabb->max_x = tr.max_x;
        if (tr.max_y > left_aabb->max_y) left_aabb->max_y = tr.max_y;
        if (tr.max_z > left_aabb->max_z) left_aabb->max_z = tr.max_z;
        left_flux += l->power;

    }

    calculate_aabb_extents(left_aabb);
    
    left_child.flux = left_flux;
    left_child.emissive_flux = left_em_flux;

    right_child.aabb.min_x = INFINITY;
    right_child.aabb.min_y = INFINITY;
    right_child.aabb.min_z = INFINITY;
    right_child.aabb.max_x = -INFINITY;
    right_child.aabb.max_y = -INFINITY;
    right_child.aabb.max_z = -INFINITY;

    AABB * right_aabb = &right_child.aabb;

    float right_flux = 0.0f;
    float right_em_flux = 0.0f;
    for (int k = right_child.first_index; k < right_child.first_index + right_child.count; k++)
    {

        AABB tr = aabbs[indices[k]];
        Light * l = &lights[indices[k]];
        if (l->light_type == DIRECTIONAL_LIGHT) continue;

        if (l->light_type == AREA_LIGHT)
        {
            right_em_flux += l->power;
        }
        if (tr.min_x < right_aabb->min_x) right_aabb->min_x = tr.min_x;
        if (tr.min_y < right_aabb->min_y) right_aabb->min_y = tr.min_y;
        if (tr.min_z < right_aabb->min_z) right_aabb->min_z = tr.min_z;
        if (tr.max_x > right_aabb->max_x) right_aabb->max_x = tr.max_x;
        if (tr.max_y > right_aabb->max_y) right_aabb->max_y = tr.max_y;
        if (tr.max_z > right_aabb->max_z) right_aabb->max_z = tr.max_z;
        right_flux += l->power;

    }

    calculate_aabb_extents(right_aabb);
    right_child.flux = right_flux;
    right_child.emissive_flux = right_em_flux;

    

        
    const int split_margin = 3;
    bool can_split = (right_child.count > split_margin || left_count > split_margin);
    bool can_leaf = !(right_child.count > split_margin || left_count > split_margin) || axes_tried == 2;
    bool did_split = (left_count == 0 || right_child.count == 0);
    if (!can_split || axes_tried == 2)
    {
        parent = &nodes[pidx];

        size_t found_indices = 0;

        float eps = 0.01f;

        parent->aabb.min_x -= eps;
        parent->aabb.min_y -= eps;
        parent->aabb.min_z -= eps;
        parent->aabb.max_x += eps;
        parent->aabb.max_y += eps;
        parent->aabb.max_z += eps;

        calculate_aabb_extents(&parent->aabb);
        
        parent->left = -1;
        parent->right = -1;
        parent->next = -1;
        parent->index = bvh_nodes->count - 1;
        
        
        return false;
    }
    else if (did_split)
    {

        subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth, axes_tried + 1);
        return true;
        
    }

    parent = &nodes[pidx];
    

    left_child.left = -1;
    left_child.right = -1;
    BVHNode * left = vector_expand(bvh_nodes, 1);
    *left = left_child;
    left->index = bvh_nodes->count-1;
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];
    parent->left = bvh_nodes->count-1;
    bool divided = subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth + 1, 0);
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];

    right_child.left = -1;
    right_child.right = -1;
    BVHNode * right = vector_expand(bvh_nodes, 1);
    *right = right_child;
    right->index = bvh_nodes->count-1;
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];
    parent->right = bvh_nodes->count-1;
    divided = subdivide_scene_bvh(bvh_nodes, indices, aabbs, bvh_nodes->count-1, depth + 1, 0);
    nodes = CAST_VEC(*bvh_nodes, BVHNode);
    parent = &nodes[pidx];

    return true;
    

    
}





void build_light_bvh()
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    if (GLOBALS.scene_data.light_bvh.lights.buffer) free(GLOBALS.scene_data.light_bvh.lights.buffer);
    init_vector(&GLOBALS.scene_data.light_bvh.lights, 0, sizeof(Light));
    
    AABB parent_aabb;
    parent_aabb.min_x = INFINITY;
    parent_aabb.min_y = INFINITY;
    parent_aabb.min_z = INFINITY;
    parent_aabb.max_x = -INFINITY;
    parent_aabb.max_y = -INFINITY;
    parent_aabb.max_z = -INFINITY;
    
    float total_flux = 0.0f;
    float emissive_flux = 0.0f;
    Light * ls = CAST_VEC(GLOBALS.lights, Light);
    uint32_t directional_lights = 0;
    for (int i = 0; i < GLOBALS.lights.count; i++)
    {

        Light * l = &ls[i];
        float lum = (l->rgb.x * 0.2126f) + (l->rgb.y * 0.7152f) + (l->rgb.z * 0.0722f);
        float power;
        Light * new_light = vector_expand(&GLOBALS.scene_data.light_bvh.lights, 1);
        *new_light = *l;
        new_light->lum = lum;

        if (l->light_type == POINT_LIGHT)
        {
            
            power = lum * l->intensity * M_PI * 4;
            new_light->power = power;
        }
        else if (l->light_type == SPOT_LIGHT)
        {
            
            power = lum * l->intensity * M_PI * 2 * (1.0f - cosf(new_light->outer_angle));
            new_light->power = power;
        }
        if (l->light_type == DIRECTIONAL_LIGHT)
        {
            power = 0.0f;
            new_light->power = 0.0f;
            directional_lights++;
        }
        else
        {
            
            new_light->area.center = new_light->pos;
            new_light->area.extents = (vec3){0, 0, 0};
            calculate_aabb_bounds(&new_light->area);
            if (new_light->area.min_x < parent_aabb.min_x) parent_aabb.min_x = new_light->area.min_x;
            if (new_light->area.min_y < parent_aabb.min_y) parent_aabb.min_y = new_light->area.min_y;
            if (new_light->area.min_z < parent_aabb.min_z) parent_aabb.min_z = new_light->area.min_z;
            if (new_light->area.max_x > parent_aabb.max_x) parent_aabb.max_x = new_light->area.max_x;
            if (new_light->area.max_y > parent_aabb.max_y) parent_aabb.max_y = new_light->area.max_y;
            if (new_light->area.max_z > parent_aabb.max_z) parent_aabb.max_z = new_light->area.max_z;
        

            total_flux += power;
        }
    }
    

    Material * mats = CAST_VEC(GLOBALS.materials, Material);

    float * matem = malloc(sizeof(float) * GLOBALS.materials.count);
    for (int i = 0; i < GLOBALS.materials.count; i++)
    {
        Material * mat = &mats[i];
        float em_len = vec4_length(mat->ke);
        float str = mat->emissive_strength;
        matem[i] = em_len * str;
    }
    
    for (int i = 0; i < GLOBALS.scene_data.bvh_indices_count; i++)
    {

        MeshNode * mn = &mns[GLOBALS.scene_data.bvh_indices[i]];
        if (mn->mesh == -1 || mn->free) continue;
        if (mn->static_mesh == 0) continue;
        Mesh * m = &meshes[mn->mesh];
        MeshFace * mfs = CAST_VEC(m->tris, MeshFace);
        Point * pts = CAST_VEC(m->vertices, Point);
        for (int j = 0; j < m->tris.count; j++)
        {
            
            MeshFace * mf = &mfs[m->bvh_indices[j]];
            Material * mat = &mats[mf->material_id];
            float val = matem[mf->material_id];
            float area = triangle_area(pts[mf->indices[0]].pos, pts[mf->indices[1]].pos, pts[mf->indices[2]].pos);
            if (val > 0)
            {

                Light * l = vector_expand(&GLOBALS.scene_data.light_bvh.lights, 1);
                l->light_type = AREA_LIGHT;
                l->threshold = 0.0006f;
                l->A = mat4_mul_point(mn->global_matrix, pts[mf->indices[0]].pos);
                l->B = mat4_mul_point(mn->global_matrix, pts[mf->indices[1]].pos);
                l->C = mat4_mul_point(mn->global_matrix, pts[mf->indices[2]].pos);
                mat3 rot = mat3_from_mat4(&mn->global_matrix);
                l->normal = vec3_normalize(mat3_mul_vec3(mat3_inverse_transpose(rot), mf->normal));

                l->rgb = vec3_scale(vec4_to_vec3(mat->ke), mat->emissive_strength);
                l->surface_area = area;
                float lum = (l->rgb.x * 0.2126f) + (l->rgb.y * 0.7152f) + (l->rgb.z * 0.0722f);
                float power = l->surface_area * lum * M_PI;
                l->lum = lum;
                l->power = power;

                l->radius = sqrtf((lum * area) / (M_PI * l->threshold));
                l->mesh = i;
                l->tri = j;
                l->area.center = triangle_centroid(l->A, l->B, l->C);
                l->area.extents = (vec3){0.01, 0.01, 0.01};
                calculate_aabb_bounds(&l->area);
                if (l->area.min_x < parent_aabb.min_x) parent_aabb.min_x = l->area.min_x;
                if (l->area.min_y < parent_aabb.min_y) parent_aabb.min_y = l->area.min_y;
                if (l->area.min_z < parent_aabb.min_z) parent_aabb.min_z = l->area.min_z;
                if (l->area.max_x > parent_aabb.max_x) parent_aabb.max_x = l->area.max_x;
                if (l->area.max_y > parent_aabb.max_y) parent_aabb.max_y = l->area.max_y;
                if (l->area.max_z > parent_aabb.max_z) parent_aabb.max_z = l->area.max_z;
                total_flux += power;
                
                emissive_flux += power;
            }
        }
    }

    free(matem);
    if (GLOBALS.scene_data.light_bvh.lights.count <= 0) return;

    if (GLOBALS.scene_data.light_bvh.nodes.buffer) free(GLOBALS.scene_data.light_bvh.nodes.buffer);
    if (GLOBALS.scene_data.light_bvh.indices) free(GLOBALS.scene_data.light_bvh.indices);
    GLOBALS.scene_data.light_bvh.nodes.buffer = NULL;
    GLOBALS.scene_data.light_bvh.indices = NULL;


    init_vector(&GLOBALS.scene_data.light_bvh.nodes, 0, sizeof(BVHNode));

    calculate_aabb_extents(&parent_aabb);

    uint32_t bvh_size = GLOBALS.scene_data.light_bvh.lights.count - directional_lights;
    GLOBALS.scene_data.light_bvh.indices = calloc(bvh_size, sizeof(size_t));

    AABB * aabbs = malloc(sizeof(AABB) * GLOBALS.scene_data.light_bvh.lights.count);
    
    Light * new_lights = CAST_VEC(GLOBALS.scene_data.light_bvh.lights, Light);
    uint32_t count = 0;

    GLOBALS.scene_data.light_bvh.indices_count = 0;
    for (int i = 0; i < GLOBALS.scene_data.light_bvh.lights.count; i++)
    {
        if (new_lights[i].light_type == DIRECTIONAL_LIGHT) continue;
        GLOBALS.scene_data.light_bvh.indices[GLOBALS.scene_data.light_bvh.indices_count++] = i;
        aabbs[i] = new_lights[i].area;
    }


    BVHNode * root = vector_expand(&GLOBALS.scene_data.light_bvh.nodes, 1);
    root->left = -1;
    root->right = -1;
    root->parent = -1;
    root->first_index = 0;
    root->index = 0;
    root->count = GLOBALS.scene_data.light_bvh.indices_count;
    root->aabb = parent_aabb;
    root->next = -1;
    root->flux = total_flux;
    root->emissive_flux = emissive_flux;
    
    subdivide_light_bvh(&GLOBALS.scene_data.light_bvh.nodes, GLOBALS.scene_data.light_bvh.indices, aabbs, new_lights, 0, 0, 0);

    BVHNode * nodes = CAST_VEC(GLOBALS.scene_data.light_bvh.nodes, BVHNode);
    assign_next_nodes(nodes, 0, -1);    

    free(aabbs);
    
}


