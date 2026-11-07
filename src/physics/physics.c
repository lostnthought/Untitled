
#include "core/camera.h"
#include "core/engine_types.h"
#include "utilities/types.h"
#include "load/init.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include "utilities/mathf.h"
#include "utilities/engine_utilities.h"
#include "core/mesh.h"
#include "physics/GJK.h"
#include "utilities/utilities.h"
#include "render/render.h"



// // Assume you already ran your Hull code to find the Hull's reference face and normal.
// // hull_normal = the collision normal pointing FROM the hull TO the capsule.
// // hull_plane_dist = the dot product of a point on the hull face and the normal.

void
    PC_apply_velocity
    (
        PhysicsComponent * pc,
        float    dt
    )

{

    static const float MAX_SPEED = 0.02f;
    static const float ACCEL = 0.01f;
    static const float GRAVITY = 9.8f;
    float accel_step = ACCEL * dt;

    pc->vel.z -= GRAVITY * dt;

    float nlen = sqrtf(pc->vel.x * pc->vel.x + pc->vel.y * pc->vel.y + pc->vel.z * pc->vel.z);

    pc->vel = vec3_scale(pc->vel, 0.999f); 
    pc->angular_velocity = vec3_scale(pc->angular_velocity, 0.995f);
    

}


int generate_capsule_hull_manifold(CapsuleData* cap, mat4 cap_transform,  vec3 hull_normal, float hull_plane_dist,  Manifold * manifold, PhysicsComponent * pcA, PhysicsComponent * pcB) 
{
    int contact_count = 0;

    vec3 world_A = mat4_mul_point(cap_transform, cap->a);
    vec3 world_B = mat4_mul_point(cap_transform, cap->b);
    // vec3 world_A = cap->a;
    // vec3 world_B = cap->b;

    float dist_A = vec3_dot(world_A, hull_normal) - hull_plane_dist;
    float dist_B = vec3_dot(world_B, hull_normal) - hull_plane_dist;

    float pen_A = dist_A - cap->radius;
    // float pen_A = -hull_plane_dist;
    float pen_B = dist_B - cap->radius;

    if (pen_A <= 0.0f) {
        manifold->contacts[contact_count].a = vec3_sub(cap->a, vec3_scale(hull_normal, hull_plane_dist));
        manifold->contacts[contact_count].pcA = pcA;
        manifold->contacts[contact_count].pcB = pcB;
        manifold->contacts[contact_count].b = cap->a;
        manifold->contacts[contact_count].d = -pen_A;
        manifold->contacts[contact_count].normal = hull_normal;
        contact_count++;
    }

    if (pen_B <= 0.0f) {
        manifold->contacts[contact_count].a = vec3_sub(cap->b, vec3_scale(hull_normal, hull_plane_dist));
        manifold->contacts[contact_count].pcA = pcA;
        manifold->contacts[contact_count].pcB = pcB;
        
        manifold->contacts[contact_count].b = cap->b;
        manifold->contacts[contact_count].d = -pen_B;
        manifold->contacts[contact_count].normal= hull_normal;
        contact_count++;
    }
    manifold->count = contact_count;

    return contact_count; // Will be 0, 1, or 2
}
void clip_manifold(Manifold * manifold, CollisionPoints * p, CollisionData a_cd, mat4 modelA, CollisionData b_cd, mat4 modelB)
{

    
    vec3 * vA = a_cd.data.hull.pts;
    size_t vAcnt = a_cd.data.hull.count;
    HullFace * fA = a_cd.data.hull.mfs;
    size_t fAcnt = a_cd.data.hull.face_count;
    vec3 * vB = b_cd.data.hull.pts;
    size_t vBcnt = b_cd.data.hull.count;
    HullFace * fB = b_cd.data.hull.mfs;
    size_t fBcnt = b_cd.data.hull.face_count;
    // if (manifold->processed) return;
    manifold->processed = true;
    vec3 n = vec3_scale(p->normal, -1);

    // find furthest along collision normal
    float maximum_dotA = -INFINITY;
    size_t largest_indexA = 0;
    for (int i = 0; i < vAcnt; i++)
    {
        float dot = vec3_dot(mat4_mul_point(modelA, vA[i]), n);
        if (dot > maximum_dotA)
        {
            maximum_dotA = dot;
            largest_indexA = i;
        }
    }

    float maximum_face_dotA = -INFINITY;
    size_t largest_face_indexA = 0;
    // find most parallel face that includes that point
    for (int i = 0; i < fAcnt; i++)
    {
        HullFace * f = &fA[i];
        bool found = false;
        for (int j = 0; j < f->count; j++)
        {
            if (f->indices[j] == largest_indexA)
            {
                
                found = true;
                break;
            }
        }
        if (!found) continue;

        float dot = vec3_dot(mat4_mul_direction(modelA, f->normal), n);
        if (dot > maximum_face_dotA)
        {
            maximum_face_dotA = dot;
            largest_face_indexA = i;
        }
    }

    float maximum_dotB = -INFINITY;
    size_t largest_indexB = 0;
    for (int i = 0; i < vBcnt; i++)
    {
        float dot = vec3_dot(mat4_mul_point(modelB, vB[i]), vec3_scale(n, -1));
        if (dot > maximum_dotB)
        {
            maximum_dotB = dot;
            largest_indexB = i;
        }
    }

    float maximum_face_dotB = -INFINITY;
    size_t largest_face_indexB = 0;
    // find most parallel face that includes that point
    for (int i = 0; i < fBcnt; i++)
    {
        HullFace * f = &fB[i];
        bool found = false;
        for (int j = 0; j < f->count; j++)
        {
            if (f->indices[j] == largest_indexB)
            {
                
                found = true;
                break;
            }
        }
        if (!found) continue;

        float dot = vec3_dot(mat4_mul_direction(modelB, f->normal), vec3_scale(n, -1));
        if (dot > maximum_face_dotB)
        {
            maximum_face_dotB = dot;
            largest_face_indexB = i;
        }
    }

    HullFace * reference_face = &fA[largest_face_indexA];
    vec3 * reference_points = vA;
    HullFace * incident_face = &fB[largest_face_indexB];
    int reference_idx = largest_face_indexA;
    int incident_idx = largest_face_indexB;
    vec3 * incident_points = vB;
    mat4 * reference_mat = &modelA;
    mat4 * incident_mat = &modelB;
    bool flipped = false;
    if (vec3_dot(mat4_mul_direction(modelA, fA[largest_face_indexA].normal), n) < vec3_dot(mat4_mul_direction(modelB, fB[largest_face_indexB].normal), n))
    {
        flipped = true;
        HullFace * temp = reference_face;
        vec3 * tempP = reference_points;
        mat4 * tempMat = reference_mat;
        reference_face = incident_face;
        reference_points = incident_points;
        reference_mat = incident_mat;
        incident_face = temp;
        incident_points = tempP;
        incident_mat = tempMat;
        incident_idx = largest_face_indexA;
        reference_idx = largest_face_indexB;
    }
    // for (int i = 0; i < incident_face->count; i++)
    // {
        
    //     DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(*incident_mat, incident_points[incident_face->indices[i]]);
    //     DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){0, 1, 0};
    //     DEBUG_LINE_COUNT++;
    //     DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(*incident_mat, incident_points[incident_face->indices[(i+1) % incident_face->count]]);
    //     DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){0, 1, 0};
    //     DEBUG_LINE_COUNT++;
    // }
    // for (int i = 0; i < reference_face->count; i++)
    // {
        
    //     DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(*reference_mat, reference_points[reference_face->indices[i]]);
    //     DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){0, 1, 1};
    //     DEBUG_LINE_COUNT++;
    //     DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(*reference_mat, reference_points[reference_face->indices[(i+1) % reference_face->count]]);
    //     DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){0, 1, 1};
    //     DEBUG_LINE_COUNT++;
    // }

    const int MAX_CLIP_PT = 64;
    ClipVertex ipA[MAX_CLIP_PT];
    for (int i = 0; i < incident_face->count; i++)
    {
        ipA[i].p = mat4_mul_point(*incident_mat, incident_points[incident_face->indices[i]]);
        ipA[i].edge0 = i;
        ipA[i].edge1 = (i+1) % incident_face->count;
    }
    ClipVertex ipB[MAX_CLIP_PT];
    int incident_count = incident_face->count;
    ClipVertex * ping = ipA;
    ClipVertex * pong = ipB;

    vec3 reference_origin = mat4_mul_point(*reference_mat, reference_points[reference_face->indices[0]]);
    vec3 reference_normal = mat4_mul_direction(*reference_mat, reference_face->normal);
    // clip against each plane
    
    for (int i = 0; i < reference_face->count; i++)
    {
        vec3 A = mat4_mul_point(*reference_mat, reference_points[reference_face->indices[i]]);
        vec3 B = mat4_mul_point(*reference_mat, reference_points[reference_face->indices[(i+1)%reference_face->count]]);
        vec3 edge = vec3_sub(B, A);
        // vec3 plane_normal = vec3_cross(edge, reference_face->normal);
        vec3 plane_normal = vec3_cross(reference_normal, edge);

        incident_count = clip_pts_against_plane(ping, incident_count, pong, A, plane_normal);
        ClipVertex * temp = ping;
        ping = pong;
        pong = temp;
    }
    if (incident_count >= MAX_CLIP_PT)
    {
        printf("INCIDENT CLIP OOB\n");
        return;
    }


    const int MAX_CLIP_AND_CONTACT = 68;
    float depths[MAX_CLIP_PT];
    CollisionPoints clip_contact[MAX_CLIP_AND_CONTACT];
    vec3 temp_arr[MAX_CLIP_AND_CONTACT];
    size_t clip_contact_cnt = 0;

    mat4 inverse_incident = mat4_inverse(*incident_mat);
    mat4 inverse_a;
    m4_inverse((float *)&inverse_a, (float*)&modelA);
    mat4 inverse_reference = mat4_inverse(*reference_mat);
    // mat4 inverse_b;
    mat4 inverse_b;
    m4_inverse((float *)&inverse_b, (float*)&modelB);

    
    for (int i = 0; i < incident_count; i++)
    {
        if (i >= incident_count) break;

        float depth = vec3_dot(vec3_sub(ping[i].p, reference_origin), reference_normal);
        if (depth > 0.01f)
        {
            for (int j = i; j < incident_count-1; j++)
            {
                ping[j] = ping[j+1];
            }
            incident_count--;
            i--;
        }
        else
        {
            if (!flipped)
            {
                clip_contact[clip_contact_cnt].b = mat4_mul_point(inverse_b, ping[i].p);
                clip_contact[clip_contact_cnt].a = mat4_mul_point(inverse_a, vec3_sub(ping[i].p, vec3_scale(reference_normal, depth)));
                clip_contact[clip_contact_cnt].reference_face = reference_idx;
                clip_contact[clip_contact_cnt].incident_face = incident_idx;
                clip_contact[clip_contact_cnt].edge_a = ping[i].edge0;
                clip_contact[clip_contact_cnt].edge_b = ping[i].edge1;

                clip_contact[clip_contact_cnt].normal = p->normal;
                clip_contact[clip_contact_cnt].d = -depth;
                clip_contact[clip_contact_cnt].pcA = p->pcA;
                clip_contact[clip_contact_cnt].pcB = p->pcB;
                clip_contact[clip_contact_cnt].mnA = p->mnA;
                clip_contact[clip_contact_cnt].mnB = p->mnB;
                clip_contact[clip_contact_cnt].normal_impulse = 0;
                clip_contact[clip_contact_cnt].tangent_impulse = 0;
                clip_contact[clip_contact_cnt].bias_impulse = 0;
                clip_contact[clip_contact_cnt].tangent = (vec3){0,0,0};
                clip_contact_cnt++;
                
            }
            else
            {
                
                clip_contact[clip_contact_cnt].a = mat4_mul_point(inverse_a, ping[i].p);
                clip_contact[clip_contact_cnt].b = mat4_mul_point(inverse_b, vec3_sub(ping[i].p, vec3_scale(reference_normal, depth)));
                clip_contact[clip_contact_cnt].normal = p->normal;
                clip_contact[clip_contact_cnt].reference_face = reference_idx;
                clip_contact[clip_contact_cnt].incident_face = incident_idx;
                clip_contact[clip_contact_cnt].edge_a = ping[i].edge0;
                clip_contact[clip_contact_cnt].edge_b = ping[i].edge1;
                clip_contact[clip_contact_cnt].d = -depth;
                clip_contact[clip_contact_cnt].pcA = p->pcA;
                clip_contact[clip_contact_cnt].pcB = p->pcB;
                clip_contact[clip_contact_cnt].mnA = p->mnA;
                clip_contact[clip_contact_cnt].mnB = p->mnB;
                clip_contact[clip_contact_cnt].normal_impulse = 0;
                clip_contact[clip_contact_cnt].tangent_impulse = 0;
                clip_contact[clip_contact_cnt].bias_impulse = 0;
                clip_contact[clip_contact_cnt].tangent = (vec3){0,0,0};
                clip_contact_cnt++;
            }
            
        }
    }

    for (int i = 0; i < clip_contact_cnt; i++)
    {

        DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(modelA, clip_contact[i].a);
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,1};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(modelA, clip_contact[(i+1)%clip_contact_cnt].a);
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,1};
        DEBUG_LINE_COUNT++;
    }
    bool not_found = false;

    for (int i = 0; i < manifold->count; i++)
    {
        bool found = false;

        for (int j = 0; j < clip_contact_cnt; j++)
        {
            if (manifold->contacts[i].incident_face == clip_contact[j].incident_face &&
             manifold->contacts[i].reference_face == clip_contact[j].reference_face &&
             manifold->contacts[i].edge_a == clip_contact[j].edge_a &&
             manifold->contacts[i].edge_b == clip_contact[j].edge_b)
            {
                // clip_contact[j] = manifold->contacts[i];
                
                // if (clip_contact[j].mnA == manifold->contacts[i].mnA && clip_contact[j].mnB == manifold->contacts[i].mnB)
                // {
                if ((vec3_equal(clip_contact[j].a, manifold->contacts[i].a, 0.01f) && vec3_equal(clip_contact[j].b, manifold->contacts[i].b, 0.01f)) || (vec3_equal(clip_contact[j].a, manifold->contacts[i].b, 0.01f) && vec3_equal(clip_contact[j].b, manifold->contacts[i].a, 0.01f)))
                {
                    
                    clip_contact[j].normal_impulse = manifold->contacts[i].normal_impulse;
                    clip_contact[j].tangent_impulse = manifold->contacts[i].tangent_impulse;
                    clip_contact[j].bias_impulse = manifold->contacts[i].bias_impulse;
                    clip_contact[j].tangent = manifold->contacts[i].tangent;
                    found = true;
                    break;
                }

            }
            
        }

    }
    

    size_t deepest_index = 0;
    if (clip_contact_cnt >= 1)
    {
        float deepest = -INFINITY;
        for (int i = 0; i < clip_contact_cnt; i++)
        {
            if (clip_contact[i].d > deepest)
            {
                deepest_index = i;
                deepest = clip_contact[i].d;
            }
        }

        
        manifold->contacts[0] = clip_contact[deepest_index];
    }
    size_t furthest_index_pt = 0;
    if (clip_contact_cnt >= 2)
    {
        float furthest = -INFINITY;

        for (int i = 0; i < clip_contact_cnt; i++)
        {
            if (deepest_index == i) continue;
            vec3 d = vec3_sub(manifold->contacts[0].a, clip_contact[i].a);
            float dist = vec3_dot(d, d);
            if (dist > furthest)
            {
                furthest_index_pt = i;
                furthest = dist;
            }
        }
        manifold->contacts[1] = clip_contact[furthest_index_pt];
    }
    size_t furthest_index_seg = 0;
    if (clip_contact_cnt >= 3)
    {

        float furthest = -INFINITY;

        for (int i = 0; i < clip_contact_cnt; i++)
        {
            if (deepest_index == i || furthest_index_pt == i) continue;
            vec3 pt = closest_point_segment(clip_contact[i].a, manifold->contacts[0].a, manifold->contacts[1].a);
            vec3 d = vec3_sub(pt, clip_contact[i].a);
            float dist = vec3_dot(d, d);
            if (dist > furthest)
            {
                furthest_index_seg = i;
                furthest = dist;
            }
        }
        manifold->contacts[2] = clip_contact[furthest_index_seg];
    }
    size_t furthest_index_tri = 0;
    if (clip_contact_cnt >= 4)
    {

        float furthest = -INFINITY;

        for (int i = 0; i < clip_contact_cnt; i++)
        {
            if (deepest_index == i || furthest_index_pt == i || furthest_index_seg == i) continue;
            vec3 pt = closest_point_triangle(clip_contact[i].a, manifold->contacts[0].a, manifold->contacts[1].a, manifold->contacts[2].a);
            vec3 d = vec3_sub(pt, clip_contact[i].a);
            float dist = vec3_dot(d, d);
            if (dist > furthest)
            {
                furthest_index_tri = i;
                furthest = dist;
            }
        }
        manifold->contacts[3] = clip_contact[furthest_index_tri];
    }

    
    manifold->count = MIN(clip_contact_cnt, 4);
    manifold->processed = true;
    
}



Manifold * find_manifold_for_pair(int mn0, int mn1)
{
    for (int i = 0; i < MAX_MANIFOLDS; i++)
    {
        if (MANIFOLDS[i].free) continue;

        Manifold * m = &MANIFOLDS[i];
        if ((m->mnA == mn0 && m->mnB == mn1) || (m->mnB == mn0 && m->mnA == mn1))
        {
            return m;
        }
    }
	    
    return NULL;
}


Manifold * create_new_manifold(PhysicsComponent * a, int mn0, PhysicsComponent * b, int mn1)
{

    for (int i = 0; i < MAX_MANIFOLDS; i++)
    {
        if (MANIFOLDS[i].free)
        {
            MANIFOLDS[i].a = a;
            MANIFOLDS[i].b = b;
            MANIFOLDS[i].mnA = mn0;
            MANIFOLDS[i].mnB = mn1;
            MANIFOLDS[i].count = 0;
            MANIFOLDS[i].free = false;
            MANIFOLDS[i].processed = false;
            MANIFOLDS[i].deepest = -INFINITY;
            if (a) a->at_rest = false;
            if (b) b->at_rest = false;
            return &MANIFOLDS[i];
        }
    }
    return NULL;
}




vec3 DEBUG_POINT;

bool PC_process_collision_node(MeshNode * mn, PhysicsComponent * pc, Mesh * m0, MeshNode * mn0, BVHNode * node, CollisionData cd, mat4 mat, mat4 mat0, mat4 mat_inv, mat4 mat0_inv, Manifold * manifold)
{
    
    MeshFace * fs = CAST_VEC(m0->tris, MeshFace);
    Point * v = CAST_VEC(m0->vertices, Point);
    size_t * li = m0->bvh_indices;
    int tris = 0;
    for (int k = 0; k < node->count; k++)
    {

        MeshFace * f = &fs[li[k + node->first_index]];
        vec3 A = v[f->indices[0]].pos;
        vec3 B = v[f->indices[1]].pos;
        vec3 C = v[f->indices[2]].pos;

        tris++;
        vec3 vertices[3];
        vertices[0] = A;
        vertices[1] = B;
        vertices[2] = C;
        HullFace face;
        face.count = 3;
        face.normal = f->normal;
        face.indices[0] = 0;
        face.indices[1] = 1;
        face.indices[2] = 2;
        

        CollisionPoints p;
        p.collision = false;
        p.d = -INFINITY;
        CollisionData cd0;
        cd0.type = COL_TRIANGLE;
        cd0.data.hull.pts = vertices;
        cd0.data.hull.count = 3;
        cd0.data.hull.mfs = &face;
        cd0.data.hull.face_count = 1;
        
        if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p, EW[0]))
        {
            p.pcA = pc;
            p.pcB = NULL;

            p.mnA = mn->id;
            p.mnB = mn0->id;
            p.normal_impulse = 0;
            p.tangent_impulse = 0;
            manifold = find_manifold_for_pair(mn->id, mn0->id);
            if (!manifold)
            {
                manifold = create_new_manifold(pc, mn->id, NULL, mn0->id);
            }
            if (manifold->deepest < p.d)
            {
                
                if (cd.type != COL_CAPSULE)
                {
                    
                    clip_manifold(manifold, &p, cd, mn->global_matrix, cd0, mn0->global_matrix);
                }
                else
                {
                    // int cap_count = generate_capsule_hull_manifold(&cd.data.capsule, mn->global_matrix, f->normal, p.d, manifold, pc, NULL);
                    // manifold->count = cap_count;
                    manifold->count = 1;
                    manifold->contacts[0] = p;
                    manifold->contacts[0].normal = f->normal;
                    
                }
                manifold->processed = true;
                manifold->deepest = p.d;
            }
        }

        
    }
    return false;
}

bool PC_traverse_collision_bvh(MeshNode * mn, PhysicsComponent * pc, Mesh * m0, MeshNode * mn0, BVHNode * nodes, BVHNode * node, CollisionData cd, mat4 mat, mat4 mat0, mat4 mat_inv, mat4 mat0_inv, AABB tr_aabb, Manifold * manifold)
{

    

    int ni = 0;


    while (ni >= 0)
    {
        BVHNode * node = &nodes[ni];
        if (!aabbs_intersect(&tr_aabb, &node->aabb))
        {
            ni = node->next;
        }
        if (node->left == -1 && node->right == -1)
        {
            if (PC_process_collision_node(mn, pc, m0, mn0, node, cd, mat, mat0, mat_inv, mat0_inv, manifold))
            {
                
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



void PC_traverse_scene_bvh(MeshNode * mn, PhysicsComponent * pc, CollisionData cd, SceneData * sd, mat4 mat, mat4 mat_inv, AABB aabb)
{


    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

    int ni = 0;
    BVHNode * nodes = CAST_VEC(sd->bvh_nodes, BVHNode);

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

                MeshNode * mn0 = &mns[sd->bvh_indices[node->first_index + i]];
                mn0 = get_root_node(mns, mn0);
                if ((mn0->mesh == -1 && mn0->parent != -1) || mn0->free) continue;
                
                // if (mn_inside_of_root_tree(mns, sd->bvh_indices[node->first_index + i], mn->id))
                // {
                //     printf("LOCAL: %zu %zu\n", mn0->local_id, mn->local_id);
                //     continue;
                // }
                if (mn0->local_id == mn->local_id)
                {
                    continue;
                }
                Manifold * manifold = find_manifold_for_pair(mn->id, mn0->id);
                if (manifold)
                {
                    if (manifold->processed)
                    {
                        continue;
                    }
                }

                
                if (mn0->pc)
                {
                    switch(mn0->pc->cd.type)
                    {
                        case COL_TRIANGLE:
                            if (mn0->mesh == -1) continue;
                            {
                                
                                Mesh * m0 = &meshes[mn0->mesh];
                                BVHNode * mesh_nodes = CAST_VEC(m0->nodes, BVHNode);
        

                                mat4 mat0 = mn0->global_matrix;
                                mat4 mat0_inv;
                                m4_inverse((float*)&mat0_inv, (float*)&mat0);
                                AABB tr = transform_aabb_by_model_matrix(aabb, mat0_inv);
                                BVHNode * mesh_node = &mesh_nodes[0];

                                bool col = PC_traverse_collision_bvh(mn, pc, m0, mn0, mesh_nodes, mesh_node, cd, mat, mat0, mat_inv, mat0_inv, tr, manifold);
                            }
                            break;
                        case COL_HULL:
                            {
                                if (mn0->mesh == -1) continue;
                                Mesh * m0 = &meshes[mn0->mesh];
                                ConvexHull * hulls0 = CAST_VEC(m0->hulls, ConvexHull);
                                for (int k = 0; k < m0->hulls.count; k++)
                                {
                

                                    CollisionPoints p;
                                    p.collision = false;
                                    p.d = -INFINITY;
                                    CollisionData cd0;
                                    cd0.type = COL_HULL;
                                    cd0.data.hull.pts = hulls0[k].vertices.buffer;
                                    cd0.data.hull.count = hulls0[k].vertices.count;
                                    cd0.data.hull.mfs = hulls0[k].hull_faces.buffer;
                                    cd0.data.hull.face_count = hulls0[k].hull_faces.count;

                                    mat4 mat0 = mn0->global_matrix;
                                    mat4 mat0_inv;
                                    m4_inverse((float*)&mat0_inv, (float*)&mat0);
                                    if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p, EW[0]))
                                    {
                                        p.pcA = pc;
                                        p.pcB = mn0->pc;

                                        p.mnA = mn->id;
                                        p.mnB = mn0->id;
                                        p.normal_impulse = 0;
                                        p.tangent_impulse = 0;
                                        p.bias_impulse = 0;
                                        manifold = find_manifold_for_pair(mn->id, mn0->id);
                                        if (!manifold)
                                        {
                                            manifold = create_new_manifold(pc, mn->id, mn0->pc, mn0->id);
                                        }
                                        if (cd.type != COL_CAPSULE)
                                        {
                    
                                            clip_manifold(manifold, &p, cd, mn->global_matrix, cd0, mn0->global_matrix);
                                        }
                                        else
                                        {

                                            manifold->count = 1;
                                            manifold->contacts[0] = p;
                    
                                        }
                                        manifold->processed = true;
                                    }
                                }
                                
                            }
                            break;
                        case COL_CAPSULE:
                            {
                                CollisionData cd0 = mn0->pc->cd;
                                CollisionPoints p;
                                p.collision = false;
                                p.d = -INFINITY;
                                mat4 mat0 = mn0->global_matrix;
                                mat4 mat0_inv;
                                m4_inverse((float*)&mat0_inv, (float*)&mat0);
                                if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p, EW[0]))
                                {
                                    p.pcA = pc;
                                    p.pcB = mn0->pc;

                                    p.mnA = mn->id;
                                    p.mnB = mn0->id;
                                    p.normal_impulse = 0;
                                    p.tangent_impulse = 0;
                                    p.bias_impulse = 0;
                                    manifold = find_manifold_for_pair(mn->id, mn0->id);
                                    if (!manifold)
                                    {
                                        manifold = create_new_manifold(pc, mn->id, mn0->pc, mn0->id);
                                    }
                                    if (cd.type != COL_CAPSULE && cd0.type != COL_CAPSULE)
                                    {
            
                                        clip_manifold(manifold, &p, cd, mn->global_matrix, cd0, mn0->global_matrix);
                                    }
                                    else
                                    {

                                        manifold->count = 1;
                                        manifold->contacts[0] = p;
            
                                    }
                                    manifold->processed = true;
                                }
                                
                            }
                            break;
                    }
                }
                else
                {
                    Mesh * m0 = &meshes[mn0->mesh];
                    
                    ConvexHull * hulls0 = CAST_VEC(m0->hulls, ConvexHull);
                    if (m0->static_mesh)
                    {
                        BVHNode * mesh_nodes = CAST_VEC(m0->nodes, BVHNode);
        

                        mat4 mat0 = mn0->global_matrix;
                        mat4 mat0_inv;
                        m4_inverse((float*)&mat0_inv, (float*)&mat0);
                        AABB tr = transform_aabb_by_model_matrix(aabb, mat0_inv);
                        BVHNode * mesh_node = &mesh_nodes[0];

                        bool col = PC_traverse_collision_bvh(mn, pc, m0, mn0, mesh_nodes, mesh_node, cd, mat, mat0, mat_inv, mat0_inv, tr, manifold);
                    }
                    else
                    {
                        for (int k = 0; k < m0->hulls.count; k++)
                        {
                

                            CollisionPoints p;
                            p.collision = false;
                            p.d = -INFINITY;
                            CollisionData cd0;
                            cd0.type = COL_HULL;
                            cd0.data.hull.pts = hulls0[k].vertices.buffer;
                            cd0.data.hull.count = hulls0[k].vertices.count;
                            cd0.data.hull.mfs = hulls0[k].hull_faces.buffer;
                            cd0.data.hull.face_count = hulls0[k].hull_faces.count;

                            mat4 mat0 = mn0->global_matrix;
                            mat4 mat0_inv;
                            m4_inverse((float*)&mat0_inv, (float*)&mat0);
                            if (GJK(cd, cd0, mat, mat0, mat_inv, mat0_inv, &p, EW[0]))
                            {
                                p.pcA = pc;
                                p.pcB = mn0->pc;

                                p.mnA = mn->id;
                                p.mnB = mn0->id;
                                p.normal_impulse = 0;
                                p.tangent_impulse = 0;
                                p.bias_impulse = 0;
                                manifold = find_manifold_for_pair(mn->id, mn0->id);
                                if (!manifold)
                                {
                                    manifold = create_new_manifold(pc, mn->id, mn0->pc, mn0->id);
                                }
                                if (cd.type != COL_CAPSULE)
                                {
                    
                                    clip_manifold(manifold, &p, cd, mn->global_matrix, cd0, mn0->global_matrix);
                                }
                                else
                                {
                                    // generate_capsule_hull_manifold(&cd.data.capsule, mn->global_matrix, p.normal, p.d, manifold, pc, NULL);

                                    manifold->count = 1;
                                    manifold->contacts[0] = p;
                    
                                }
                                manifold->processed = true;
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
void search_collisions_from_pc(MeshNode * mn, PhysicsComponent * pc)
{
    CollisionData cd = pc->cd;
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    mat4 mat = mn->global_matrix;

    mat4 mat_inv;
    m4_inverse((float*)&mat_inv, (float*)&mat);
    AABB aabb = transform_aabb_by_model_matrix(pc->aabb, mn->global_matrix);
    PC_traverse_scene_bvh(mn, pc, cd, &GLOBALS.scene_data, mat, mat_inv, aabb);
}




void search_collisions_from_hull(MeshNode * mn, PhysicsComponent * pc)
{
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * m = &meshes[mn->mesh];
    if (m->static_mesh) return;
    CollisionData cd = pc->cd;

    // mat4 prop = compose_trs_from_vectors(proposed, mn->rotation, mn->scale);
    mat4 mat = mn->global_matrix;

    mat4 mat_inv;
    m4_inverse((float*)&mat_inv, (float*)&mat);
    
    AABB aabb = transform_aabb_by_model_matrix(m->aabb, mat);

            
    ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);

    

    for (int i = 0; i < m->hulls.count; i++)
    {
        ConvexHull * hull = &hulls[i];

        switch(cd.type)
        {
            case COL_TRIANGLE:
                break;

            case COL_HULL:
                cd.data.hull.pts = hull->vertices.buffer;
                cd.data.hull.count = hull->vertices.count;
                cd.data.hull.mfs = hull->hull_faces.buffer;
                cd.data.hull.face_count = hull->hull_faces.count;
                break;
            case COL_CAPSULE:
                break;
        }
        BVHNode * node = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);
        PC_traverse_scene_bvh(mn, pc, cd, &GLOBALS.scene_data, mat, mat_inv, aabb);
    }
    
}

void PC_find_collision(MeshNode * mn, PhysicsComponent * pc)
{
    
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    CollisionData cd = pc->cd;
    if (mn->mesh == -1)
    {
        

        search_collisions_from_pc(mn, pc);

        
    }
    else
    {
        search_collisions_from_hull(mn, pc);
        
    }
    
}

float normal_impulse0 = 0.0f;
float normal_impulse1 = 0.0f;
float normal_impulse2 = 0.0f;
float normal_impulse3 = 0.0f;
vec3 torque_sum = (vec3){0,0,0};
vec3 friction_sum = (vec3){0,0,0};
vec3 impulse_sum = (vec3){0,0,0};
vec3 ang_impulse_sum = (vec3){0,0,0};

void player_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p)
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Entity * entA = NULL;
    Entity * entB = NULL;
    MeshNode * mnA = NULL;
    MeshNode * mnB = NULL;


    vec3 vA = pcA->vel;
    vec3 vB = (vec3){0,0,0};
    float IMA = 1.0f / pcA->mass;
    float IMB = 0;
    mat3 IIA = pcA->inverse_world;
    vec3 IIB = (vec3){0,0,0};

    vec3 comA;
    vec3 pA;
    comA = vec3_add(pcA->center_of_mass, vec4_to_vec3(cam.cam_data.cam_pos));
    pA = vec3_add(p->a, vec4_to_vec3(cam.cam_data.cam_pos));
        

    
    vec3 rA = vec3_sub(pA, comA);

    vec3 CVA = vA;

    vec3 relative = vec3_sub(CVA, (vec3){0,0,0});
    float vn = vec3_dot(relative, p->normal);    
    float restitution = pcA->restitution;
    float angularA, angularB;
    if (vn < 0)
    {


        vec3 rnA = vec3_cross(rA, p->normal);
        vec3 IA = mat3_mul_vec3(IIA, rnA);
        vec3 tA = vec3_cross(IA, rA);
        angularA = vec3_dot(p->normal, tA);

        
        const float slop = 0.005f;
        const float beta = 0.2f;

        float bias = fmaxf(p->d - slop, 0) * beta / dt;

        float K = IMA + angularA;
        float j_vel = (-(1 + restitution) * vn) / K;
        float j_bias = bias / K;

        float old_vel = p->normal_impulse;
        p->normal_impulse = fmaxf(old_vel + j_vel, 0.0f);
        
        // float old_bias = p->bias_impulse;
        // p->bias_impulse = fmaxf(old_bias + j_bias, 0.0f);


        float j = (p->normal_impulse - old_vel);

        
        vec3 impulse = vec3_scale(p->normal, j);

        pcA->vel.x += impulse.x * IMA;
        pcA->vel.y += impulse.y * IMA;
        pcA->vel.z += impulse.z * IMA;
        
    }    
}

void player_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p)
{
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Entity * entA = NULL;
    Entity * entB = NULL;
    MeshNode * mnA = NULL;
    MeshNode * mnB = NULL;

    if (!pcB->player_pc)
    {
        entB = pcB->ent;
        if (!entB) return;
        if (entB->free) return;
        if (entB->mesh_node < 0) return;
        mnB = &mns[entB->mesh_node];
    }


    vec3 vA = pcA->vel;
    vec3 vB = pcB->vel;
    float IMA = 1.0f / pcA->mass;
    float IMB = 1.0f / pcB->mass;
    mat3 IIA = pcA->inverse_world;
    mat3 IIB = pcB->inverse_world;

    vec3 comA = pcA->center_of_mass;
    vec3 pA, pB;
    comA = vec3_add(comA, vec4_to_vec3(player.cam->cam_data.cam_pos));
    pA = vec3_add(p->a, vec4_to_vec3(cam.cam_data.cam_pos));
        

    vec3 comB = pcB->center_of_mass;
    if (pcB->player_pc)
    {
        comB = vec3_add(comB, vec4_to_vec3(player.cam->cam_data.cam_pos));
        pB = vec3_add(p->b, vec4_to_vec3(cam.cam_data.cam_pos));
        
    }
    else
    {
        comB = mat4_mul_point(mnB->global_matrix, comB);
        pB = mat4_mul_point(mnB->global_matrix, p->b);
    }

    
    vec3 rA = vec3_sub(pA, comA);
    vec3 rB = vec3_sub(pB, comB);

    // PRINT_VEC3N(rB);
    // PRINT_VEC3N(comB);
    // printf("NODE: %d\n", mnB->id);
    // A
    vec3 CVA = vA;
    vec3 CVB = vec3_add(vB, vec3_cross(pcB->angular_velocity, rB));


    vec3 relative = vec3_sub(CVA, CVB);
    float vn = vec3_dot(relative, p->normal);    
    float angularA, angularB;
    if (vn < 0)
    {


        vec3 rnA = vec3_cross(rA, p->normal);
        vec3 IA = mat3_mul_vec3(IIA, rnA);
        vec3 tA = vec3_cross(IA, rA);
        angularA = vec3_dot(p->normal, tA);

            
        vec3 rnB = vec3_cross(rB, p->normal);
        vec3 IB = mat3_mul_vec3(IIB, rnB);
        vec3 tB = vec3_cross(IB, rB);
        angularB = vec3_dot(p->normal, tB);
        float K = IMA + IMB + angularA + angularB;

        const float slop = 0.005f;
        const float beta = 0.05f;

        float bias = fmaxf(p->d - slop, 0) * beta / dt;

        float j_vel = (-(1 + pcA->restitution) * vn) / K;
        float j_bias = bias / K;

        float old_vel = p->normal_impulse;
        p->normal_impulse = fmaxf(old_vel + j_vel, 0.0f);
        
        float old_bias = p->bias_impulse;
        p->bias_impulse = fmaxf(old_bias + j_bias, 0.0f);


        float j = (p->normal_impulse - old_vel) + j_bias;
        // float j = (p->normal_impulse - old_vel);
        

        vec3 impulse = vec3_scale(p->normal, j);


        pcA->vel.x += impulse.x * IMA;
        pcA->vel.y += impulse.y * IMA;
        pcA->vel.z += impulse.z * IMA;

        pcB->vel.x -= impulse.x * IMB;
        pcB->vel.y -= impulse.y * IMB;
        pcB->vel.z -= impulse.z * IMB;
        vec3 torqueA = vec3_cross(rA, impulse);

        vec3 angA_impulse = mat3_mul_vec3(IIA, torqueA);
        // pcA->angular_velocity.x += angA_impulse.x;
        // pcA->angular_velocity.y += angA_impulse.y;
        // pcA->angular_velocity.z += angA_impulse.z;
        
        
        vec3 torqueB = vec3_cross(rB, vec3_scale(impulse, -1));

        vec3 angB_impulse = mat3_mul_vec3(IIB, torqueB);
        pcB->angular_velocity.x += angB_impulse.x;
        pcB->angular_velocity.y += angB_impulse.y;
        pcB->angular_velocity.z += angB_impulse.z;

        CVA = vec3_add(pcA->vel, vec3_cross(pcA->angular_velocity, rA));
        CVB = vec3_add(pcB->vel, vec3_cross(pcB->angular_velocity, rB));

        relative = vec3_sub(CVA, CVB);
        vn = vec3_dot(relative, p->normal);    
        // if (vn - j_bias < 0)
        // {
            vec3 vt = vec3_sub(relative, vec3_scale(p->normal, vn));

            float vt_len = vec3_length(vt);

            if (vt_len > 1e-6f)
            {
                vec3 tangent = vec3_scale(vt, 1.0f/vt_len);

                p->tangent = tangent;
                vec3 rtA = vec3_cross(rA, tangent);
                vec3 ITA = mat3_mul_vec3(IIA, rtA);
                vec3 ttA = vec3_cross(ITA, rA);
                float angulartA = vec3_dot(tangent, ttA);

            
                vec3 rtB = vec3_cross(rB, tangent);
                vec3 ITB = mat3_mul_vec3(IIB, rtB);
                vec3 ttB = vec3_cross(ITB, rB);
                float angulartB = vec3_dot(tangent, ttB);

                float jt = -vec3_dot(relative, tangent) / (IMA + IMB + angulartA + angulartB);                

                float max_frictionA = pcA->mu * p->normal_impulse;

                float old = p->tangent_impulse;

                p->tangent_impulse = clamp(old + jt, -max_frictionA, max_frictionA);

                jt = p->tangent_impulse - old;



                vec3 FIA = vec3_scale(tangent, jt);


                pcB->vel.x -= FIA.x * IMB;
                pcB->vel.y -= FIA.y * IMB;
                pcB->vel.z -= FIA.z * IMB;

                vec3 FTA = vec3_cross(rA, FIA);

                vec3 angFA_impulse = mat3_mul_vec3(IIA, FTA);
                // pcA->angular_velocity.x += angFA_impulse.x;
                // pcA->angular_velocity.y += angFA_impulse.y;
                // pcA->angular_velocity.z += angFA_impulse.z;
            
                vec3 FTB = vec3_cross(rB, vec3_scale(FIA, -1));
                vec3 angFB_impulse = mat3_mul_vec3(IIB, FTB);

                pcB->angular_velocity.x += angFB_impulse.x;
                pcB->angular_velocity.y += angFB_impulse.y;
                pcB->angular_velocity.z += angFB_impulse.z;
            
            
            }
            
        // }


    }
    
}

void PC_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p, int contact)
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Entity * entA = NULL;
    Entity * entB = NULL;
    MeshNode * mnA = NULL;
    MeshNode * mnB = NULL;
    if (!pcA->player_pc)
    {
        entA = pcA->ent;
        if (!entA) return;
        if (entA->free) return;
        if (entA->mesh_node < 0) return;
        mnA = &mns[entA->mesh_node];
    }



    vec3 vA = pcA->vel;
    vec3 vB = (vec3){0,0,0};
    float IMA = 1.0f / pcA->mass;
    float IMB = 0;
    mat3 IIA = pcA->inverse_world;
    vec3 IIB = (vec3){0,0,0};

    vec3 comA;
    vec3 pA;
    if (pcA->player_pc)
    {
        comA = vec3_add(comA, vec4_to_vec3(player.cam->cam_data.cam_pos));
        pA = vec3_add(p->a, vec4_to_vec3(cam.cam_data.cam_pos));
        
    }
    else
    {
        
        comA = mat4_mul_point(mnA->global_matrix, pcA->center_of_mass);
        pA = mat4_mul_point(mnA->global_matrix, p->a);
    }

    
    vec3 rA = vec3_sub(pA, comA);
    // PRINT_VEC3N(rA);
    vec3 CVA = vec3_add(vA, vec3_cross(pcA->angular_velocity, rA));

    vec3 relative = vec3_sub(CVA, (vec3){0,0,0});
    float vn = vec3_dot(relative, p->normal);    
    // printf("VN: %f\n", vn);
    float restitution = pcA->restitution;
    // if (vn < 1.0f) restitution = 0;
    float angularA, angularB;
    if (vn < 0)
    {


        vec3 rnA = vec3_cross(rA, p->normal);
        vec3 IA = mat3_mul_vec3(IIA, rnA);
        vec3 tA = vec3_cross(IA, rA);
        angularA = vec3_dot(p->normal, tA);

        
        const float slop = 0.005f;
        const float beta = 0.05f;

        float bias = fmaxf(p->d - slop, 0) * beta / dt;

        float K = IMA + angularA;
        float j_vel = (-(1 + restitution) * vn) / K;
        float j_bias = bias / K;

        float old_vel = p->normal_impulse;
        p->normal_impulse = fmaxf(old_vel + j_vel, 0.0f);
        
        float old_bias = p->bias_impulse;
        p->bias_impulse = fmaxf(old_bias + j_bias, 0.0f);


        float j = (p->normal_impulse - old_vel) /* + j_bias */;

        // printf("J: %f\n", j);
        
        vec3 impulse = vec3_scale(p->normal, j);

        pcA->vel.x += impulse.x * IMA;
        pcA->vel.y += impulse.y * IMA;
        pcA->vel.z += impulse.z * IMA;


        vec3 torqueA = vec3_cross(rA, impulse);
        vec3 ang_impulse = mat3_mul_vec3(IIA, torqueA);
        ang_impulse_sum = vec3_add(ang_impulse_sum, ang_impulse);
        
        pcA->angular_velocity.x += ang_impulse.x;
        pcA->angular_velocity.y += ang_impulse.y;
        pcA->angular_velocity.z += ang_impulse.z;

        CVA = vec3_add(pcA->vel, vec3_cross(pcA->angular_velocity, rA));
        // CVA =pcA->vel;
        relative = CVA;
        vn = vec3_dot(relative, p->normal);
        // if (vn< 0)
        // {
            vec3 vt = vec3_sub(relative, vec3_scale(p->normal, vn));

            float vt_len = vec3_length(vt);
            if (vt_len > 1e-3f)
            {

                
                vec3 tangent = vec3_scale(vt, 1.0f/vt_len);
                
                p->tangent = tangent;
                // p->tangent = vec3_normalize(vec3_add(p->tangent, tangent));

                vec3 rtA = vec3_cross(rA, tangent);
                vec3 ITA = mat3_mul_vec3(IIA, rtA);
                vec3 ttA = vec3_cross(ITA, rA);
                float angulartA = vec3_dot(tangent, ttA);

                float jt = -vec3_dot(relative, tangent) / (IMA + angulartA);                

                
                float max_frictionA = pcA->mu * p->normal_impulse;

                float old = p->tangent_impulse;

                p->tangent_impulse = clamp(old + jt, -max_frictionA, max_frictionA);

                jt = p->tangent_impulse - old;

                float jtA = clamp(jt, -max_frictionA, max_frictionA);                

                vec3 FIA = vec3_scale(tangent, jt);

                pcA->vel.x += FIA.x * IMA;
                pcA->vel.y += FIA.y * IMA;
                pcA->vel.z += FIA.z * IMA;

                vec3 FTA = vec3_cross(rA, FIA);

                vec3 angF_impulse = mat3_mul_vec3(IIA, FTA);

                pcA->angular_velocity.x += angF_impulse.x;
                pcA->angular_velocity.y += angF_impulse.y;
                pcA->angular_velocity.z += angF_impulse.z;

            

            }
            
        // }

        
    }    
}



void PC_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p)
{
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Entity * entA = NULL;
    Entity * entB = NULL;
    MeshNode * mnA = NULL;
    MeshNode * mnB = NULL;
    if (!pcA->player_pc)
    {
        entA = pcA->ent;
        if (!entA) return;
        if (entA->free) return;
        if (entA->mesh_node < 0) return;
        mnA = &mns[entA->mesh_node];
    }

    if (!pcB->player_pc)
    {
        entB = pcB->ent;
        if (!entB) return;
        if (entB->free) return;
        if (entB->mesh_node < 0) return;
        mnB = &mns[entB->mesh_node];
    }


    vec3 vA = pcA->vel;
    vec3 vB = pcB->vel;
    float IMA = 1.0f / pcA->mass;
    float IMB = 1.0f / pcB->mass;
    mat3 IIA = pcA->inverse_world;
    mat3 IIB = pcB->inverse_world;

    vec3 comA = pcA->center_of_mass;
    vec3 pA, pB;
    if (pcA->player_pc)
    {
        comA = vec3_add(comA, vec4_to_vec3(player.cam->cam_data.cam_pos));
        pA = vec3_add(p->a, vec4_to_vec3(cam.cam_data.cam_pos));
        
    }
    else
    {
        
        comA = mat4_mul_point(mnA->global_matrix, comA);
        pA = mat4_mul_point(mnA->global_matrix, p->a);
    }

    vec3 comB = pcB->center_of_mass;
    if (pcB->player_pc)
    {
        comB = vec3_add(comB, vec4_to_vec3(player.cam->cam_data.cam_pos));
        pB = vec3_add(p->b, vec4_to_vec3(cam.cam_data.cam_pos));
        
    }
    else
    {
        comB = mat4_mul_point(mnB->global_matrix, comB);
        pB = mat4_mul_point(mnB->global_matrix, p->b);
    }

    
    vec3 rA = vec3_sub(pA, comA);
    vec3 rB = vec3_sub(pB, comB);

    // A
    vec3 CVA = vec3_add(vA, vec3_cross(pcA->angular_velocity, rA));
    vec3 CVB = vec3_add(vB, vec3_cross(pcB->angular_velocity, rB));


    vec3 relative = vec3_sub(CVA, CVB);
    float vn = vec3_dot(relative, p->normal);    
    float angularA, angularB;
    if (vn < 0)
    {


        vec3 rnA = vec3_cross(rA, p->normal);
        vec3 IA = mat3_mul_vec3(IIA, rnA);
        vec3 tA = vec3_cross(IA, rA);
        angularA = vec3_dot(p->normal, tA);

            
        vec3 rnB = vec3_cross(rB, p->normal);
        vec3 IB = mat3_mul_vec3(IIB, rnB);
        vec3 tB = vec3_cross(IB, rB);
        angularB = vec3_dot(p->normal, tB);
        float K = IMA + IMB + angularA + angularB;

        const float slop = 0.005f;
        const float beta = 0.05f;

        float bias = fmaxf(p->d - slop, 0) * beta / dt;

        float j_vel = (-(1 + pcA->restitution) * vn) / K;
        float j_bias = bias / K;

        float old_vel = p->normal_impulse;
        p->normal_impulse = fmaxf(old_vel + j_vel, 0.0f);
        
        float old_bias = p->bias_impulse;
        p->bias_impulse = fmaxf(old_bias + j_bias, 0.0f);


        float j = (p->normal_impulse - old_vel) /* + j_bias */;
        

        vec3 impulse = vec3_scale(p->normal, j);


        pcA->vel.x += impulse.x * IMA;
        pcA->vel.y += impulse.y * IMA;
        pcA->vel.z += impulse.z * IMA;

        pcB->vel.x -= impulse.x * IMB;
        pcB->vel.y -= impulse.y * IMB;
        pcB->vel.z -= impulse.z * IMB;
        vec3 torqueA = vec3_cross(rA, impulse);

        vec3 angA_impulse = mat3_mul_vec3(IIA, torqueA);
        
        pcA->angular_velocity.x += angA_impulse.x;
        pcA->angular_velocity.y += angA_impulse.y;
        pcA->angular_velocity.z += angA_impulse.z;
        
        vec3 torqueB = vec3_cross(rB, vec3_scale(impulse, -1));

        vec3 angB_impulse = mat3_mul_vec3(IIB, torqueB);
        pcB->angular_velocity.x += angB_impulse.x;
        pcB->angular_velocity.y += angB_impulse.y;
        pcB->angular_velocity.z += angB_impulse.z;

        CVA = vec3_add(pcA->vel, vec3_cross(pcA->angular_velocity, rA));
        CVB = vec3_add(pcB->vel, vec3_cross(pcB->angular_velocity, rB));

        relative = vec3_sub(CVA, CVB);
        vn = vec3_dot(relative, p->normal);    
        // if (vn - j_bias < 0)
        // {
            vec3 vt = vec3_sub(relative, vec3_scale(p->normal, vn));

            float vt_len = vec3_length(vt);

            if (vt_len > 1e-6f)
            {
                vec3 tangent = vec3_scale(vt, 1.0f/vt_len);

                p->tangent = tangent;
                vec3 rtA = vec3_cross(rA, tangent);
                vec3 ITA = mat3_mul_vec3(IIA, rtA);
                vec3 ttA = vec3_cross(ITA, rA);
                float angulartA = vec3_dot(tangent, ttA);

            
                vec3 rtB = vec3_cross(rB, tangent);
                vec3 ITB = mat3_mul_vec3(IIB, rtB);
                vec3 ttB = vec3_cross(ITB, rB);
                float angulartB = vec3_dot(tangent, ttB);

                float jt = -vec3_dot(relative, tangent) / (IMA + IMB + angulartA + angulartB);                

                float max_frictionA = pcA->mu * p->normal_impulse;

                float old = p->tangent_impulse;

                p->tangent_impulse = clamp(old + jt, -max_frictionA, max_frictionA);

                jt = p->tangent_impulse - old;



                vec3 FIA = vec3_scale(tangent, jt);

                pcA->vel.x += FIA.x * IMA;
                pcA->vel.y += FIA.y * IMA;
                pcA->vel.z += FIA.z * IMA;

                pcB->vel.x -= FIA.x * IMB;
                pcB->vel.y -= FIA.y * IMB;
                pcB->vel.z -= FIA.z * IMB;

                vec3 FTA = vec3_cross(rA, FIA);

                vec3 angFA_impulse = mat3_mul_vec3(IIA, FTA);

                pcA->angular_velocity.x += angFA_impulse.x;
                pcA->angular_velocity.y += angFA_impulse.y;
                pcA->angular_velocity.z += angFA_impulse.z;
            
                vec3 FTB = vec3_cross(rB, vec3_scale(FIA, -1));
                vec3 angFB_impulse = mat3_mul_vec3(IIB, FTB);

                pcB->angular_velocity.x += angFB_impulse.x;
                pcB->angular_velocity.y += angFB_impulse.y;
                pcB->angular_velocity.z += angFB_impulse.z;
            
            
            }
            
        // }


    }
    
}


void update_physics_components()
{

    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free || PHYSICS_COMPONENTS[i].at_rest || PHYSICS_COMPONENTS[i].player_pc) continue;
        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];
        pc->old_vel = pc->vel;
        pc->old_angular = pc->angular_velocity;
        PC_apply_velocity(pc, dt);
    }
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free || PHYSICS_COMPONENTS[i].player_pc) continue;
        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];

        Entity * ent = pc->ent;

        if (!ent) continue;
        if (ent->free) continue;
        if (ent->mesh_node < 0) continue;
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        MeshNode * mn = &mns[ent->mesh_node];
        if (pc->cd.type == COL_CAPSULE)
        {
            DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,0};
            DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(mn->global_matrix, pc->cd.data.capsule.a);
            DEBUG_LINE_COUNT++;
            DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,0};
            DEBUG_LINES[DEBUG_LINE_COUNT].pos = mat4_mul_point(mn->global_matrix, pc->cd.data.capsule.b);
            DEBUG_LINE_COUNT++;
            
            AABB a = transform_aabb_by_model_matrix(mn->aabb, mn->global_matrix);
            aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &a, (vec3){0,0,0});
            for (int j = 0; j < 24; j++)
            {
                DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0,1,0};
            }
            DEBUG_LINE_COUNT += 24;

            render_mesh_aabbs_from_root(mn);
            
        }

        if (pc->at_rest) continue;
        
        vec3 proposed = vec3_add(mn->translation, vec3_scale(pc->vel, dt));


        float angle = vec3_length(pc->angular_velocity) * dt;

        vec4 rotation = mn->rotation;
        if(angle > 0 && pc->physics_type == PHYSICS_FULL)
        {
            vec3 axis = vec3_normalize(pc->angular_velocity);

            vec4 dq = quat_from_axis_angle(axis, angle);

            rotation = vec4_normalize(quat_mul(dq, mn->rotation));
        }

    
        mn->global_matrix = compose_trs_from_vectors(proposed, rotation, mn->scale);
        mat3 rot = quat_to_mat3(mn->rotation);
        pc->inverse_world = mat3_mul(mat3_mul(rot, pc->inverse_inertia), mat3_transpose(rot));
        // print_aabb(&pc->aabb);
        

        
    }
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free || PHYSICS_COMPONENTS[i].at_rest || PHYSICS_COMPONENTS[i].player_pc) continue;
        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];

        Entity * ent = pc->ent;

        if (!ent) continue;
        if (ent->free) continue;
        if (ent->mesh_node < 0) continue;
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        MeshNode * mn = &mns[ent->mesh_node];
        // printf("CHECKING\n");
        // Timer PC_timer;
        // start_timer(&PC_timer);
        PC_find_collision(mn, pc);
        // double PC_time = end_timer(&PC_timer);
        // printf("TIME: %f\n", PC_time);
    }
    for (int i = 0; i < MAX_MANIFOLDS; i++)
    {
        if (MANIFOLDS[i].free) continue;
        // if this manifold did not detect a collision this frame, remove it
        if (!MANIFOLDS[i].processed)
        {
            MANIFOLDS[i].free = true;
            MANIFOLDS[i].a = NULL;
            MANIFOLDS[i].b = NULL;
            MANIFOLDS[i].count = 0;
            MANIFOLDS[i].processed = false;
            continue;
        }
        Manifold * m = &MANIFOLDS[i];
        m->deepest = -INFINITY;
        for (int k = 0; k < m->count; k++)
        {
            CollisionPoints * p = &m->contacts[k];

            MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
            MeshNode * mnA = &mns[p->mnA];
            MeshNode * mnB = &mns[p->mnB];

            vec3 worldA = mat4_mul_point(mnA->global_matrix, p->a);
            vec3 worldB = mat4_mul_point(mnB->global_matrix, p->b);
            float separation = vec3_dot(vec3_sub(worldA, worldB), p->normal);
            p->d = -separation;

            
        }
    }
    normal_impulse0 = 0.0f;
    normal_impulse1 = 0.0f;
    normal_impulse2 = 0.0f;
    normal_impulse3 = 0.0f;
    torque_sum = (vec3){0,0,0};
    impulse_sum = (vec3){0,0,0};
    friction_sum = (vec3){0,0,0};
    ang_impulse_sum = (vec3){0,0,0};
    for (int j = 0; j < 4; j++)
    {
        for (int i = 0; i < MAX_MANIFOLDS; i++)
        {
            MANIFOLDS[i].processed = false;
            if (MANIFOLDS[i].free) continue;
            Manifold * m = &MANIFOLDS[i];
            if(m->count == 0) continue;
            int seed = rand() % m->count;
            for (int k = 0; k < m->count; k++)
            {
                int idx = (seed + k) % m->count;
                // int idx = k;
                CollisionPoints * p = &m->contacts[idx];
                // PRINT_VEC3N(m->contacts[k].a);
                // PRINT_VEC3N(m->contacts[k].b);
                // PRINT_VEC3N(m->contacts[k].normal);
                // printf("DFINAL: %f\n", p->d);
                if (p->d < 0.0) continue;
                if (p->pcB && p->pcA)
                {
                    PC_resolve_impulse(p->pcA, p->pcB, p);
            
                }
                else if (p->pcA)
                {

                    PC_resolve_impulse_static(p->pcA, p, idx);
                }
                else
                {
                    PC_resolve_impulse_static(p->pcA, p, idx);
                }
                if (p->pcA)
                {
                    if (p->pcA->physics_type != PHYSICS_FULL)
                    {
                        p->pcA->angular_velocity = (vec3){0,0,0};
                    }
                }
                if (p->pcB)
                {
                    if (p->pcB->physics_type != PHYSICS_FULL)
                    {
                        p->pcB->angular_velocity = (vec3){0,0,0};
                    }
                    
                }

            }
        }
    }
    // for (int i = 0; i < MAX_MANIFOLDS; i++)
    // {
    //     if (MANIFOLDS[i].free) continue;
    //     Manifold * m = &MANIFOLDS[i];
    //     if (m->count == 0) continue;



    //     float max_depth = 0.0f;
    //     vec3 normal = m->contacts[0].normal;
    //     // vec3 normal = (vec3){0,0,0};

    //     for (int k = 0; k < m->count; ++k)
    //     {
    //         max_depth = fmaxf(max_depth, m->contacts[k].d);
    //         // normal = vec3_add(normal, m->contacts[k].normal);
    //     }
    //     // normal = vec3_normalize(normal);
    //     CollisionPoints * p = &m->contacts[0];
    //     // if (max_depth < 0) continue;

    //     float slop = 0.005f;

    //     float correction = fmaxf(max_depth - slop, 0) * 0.0005f;        

        
    //     // if (p->pcB && p->pcA)
    //     // {

    //     //     float IMA = 1.0f/p->pcA->mass;
    //     //     float IMB = 1.0f/p->pcB->mass;
    //     //     float total_inv = IMA + IMB;
    //     //     p->pcA->corrected_position = vec3_add(p->pcA->corrected_position, vec3_scale(vec3_scale(vec3_scale(p->normal, correction), IMA), 1.0f/total_inv));
    //     //     p->pcB->corrected_position = vec3_sub(p->pcB->corrected_position, vec3_scale(vec3_scale(vec3_scale(p->normal, correction), IMB), 1.0f/total_inv));
    
    //     // }
    //     // else if (p->pcA)
    //     // {

    //     //     p->pcA->corrected_position = vec3_add(p->pcA->corrected_position, vec3_scale(p->normal, correction));
            
    //     // }
    //     // else
    //     // {
    //     //     p->pcB->corrected_position = vec3_add(p->pcB->corrected_position, vec3_scale(p->normal, correction));
            
    //     // }
        

    //     MANIFOLDS[i].processed = false;

    // }
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free || PHYSICS_COMPONENTS[i].at_rest || PHYSICS_COMPONENTS[i].player_pc) continue;
        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];

        Entity * ent = pc->ent;

        if (!ent) continue;
        if (ent->free) continue;
        if (ent->mesh_node < 0) continue;
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
        MeshNode * mn = &mns[ent->mesh_node];
        vec3 proposed = vec3_add(mn->translation, vec3_scale(pc->vel, dt));
        float diff = vec3_length(vec3_sub(proposed, mn->translation));
        Light * lights = CAST_VEC(GLOBALS.lights, Light);
        if (mn->mesh != -1)
        {
            Mesh * m = &meshes[mn->mesh];
            int lights_in_range[MAX_LIGHTS];
            int light_count = 0;
            for (int j = 0; j < GLOBALS.lights.count; j++)
            {
                Light * l = &lights[j];
            
                if (l->light_type != POINT_LIGHT || l->sm_dirty) continue;

                vec3 min = mat4_mul_point(mn->global_matrix, (vec3){m->aabb.min_x, m->aabb.min_y, m->aabb.min_z});
                vec3 max = mat4_mul_point(mn->global_matrix, (vec3){m->aabb.max_x, m->aabb.max_y, m->aabb.max_z});
                if (test_sphere_aabb(l->pos, l->radius, min, max))
                {
                    lights_in_range[light_count++] = j;
                }
            
            }
            float lowest_dist = INFINITY;
            int closest = -1;
            for (int j = 0; j < light_count; j++)
            {
                Light * l = &lights[lights_in_range[j]];
                float dist = vec3_length(vec3_sub(l->pos, proposed));
                if (dist < lowest_dist)
                {
                    closest = lights_in_range[j];
                    lowest_dist = dist;
                }
            }
            if (closest >= 0)
            {
                lights[closest].sm_dirty = true;
            }
        }
        mn->translation.x = proposed.x;
        mn->translation.y = proposed.y;
        mn->translation.z = proposed.z;

        float angle = vec3_length(pc->angular_velocity) * dt;

        if(angle > 0 && pc->physics_type == PHYSICS_FULL)
        {
            vec3 axis = vec3_normalize(pc->angular_velocity);

            vec4 dq = quat_from_axis_angle(axis, angle);

            mn->rotation = vec4_normalize(quat_mul(dq, mn->rotation));
        }

    
        mn->global_matrix = compose_trs_from_mesh_node(mn);
        
        mat3 rot = quat_to_mat3(mn->rotation);
        pc->inverse_world = mat3_mul(mat3_mul(rot, pc->inverse_inertia), mat3_transpose(rot));
        

        pc->accel = vec3_add(vec3_scale(pc->accel, 1.0f - ACCEL_BETA), vec3_sub(pc->vel, pc->old_vel));
        
    }
    
    for (int i = 0; i < MAX_MANIFOLDS; i++)
    {
        if (MANIFOLDS[i].free) continue;
        Manifold * m = &MANIFOLDS[i];
        for (int k = 0; k < m->count; k++)
        {
            CollisionPoints * p = &m->contacts[k];

            MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
            MeshNode * mnA = &mns[p->mnA];
            MeshNode * mnB = &mns[p->mnB];

            vec3 worldA = mat4_mul_point(mnA->global_matrix, p->a);
            vec3 worldB = mat4_mul_point(mnB->global_matrix, p->b);
            vec3 dist = vec3_sub(worldA, worldB);
            float d = vec3_dot(dist, dist);
            float separation = vec3_dot(dist, p->normal);
            vec3 up;

            if (fabsf(p->normal.z) < 0.9f)
                up = (vec3){0,0,1};
            else
                up = (vec3){1,0,0};
    
            vec3 T = vec3_normalize(vec3_cross(up, p->normal));
            float td = vec3_dot(dist, T);


            if (separation > 0.01f || td > 0.01f)
            {
                for (int j = k; j < m->count-1; j++)
                {
                    m->contacts[j] = m->contacts[j+1];
                }
                m->count--;
                k--;
            }
            
        }
    }
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free || PHYSICS_COMPONENTS[i].at_rest || PHYSICS_COMPONENTS[i].player_pc) continue;
        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];

        float vel_diff = vec3_length(vec3_sub(pc->old_vel, pc->vel));
        float avel_diff = vec3_length(vec3_sub(pc->old_angular, pc->angular_velocity));
        // PRINT_VEC3N(pc->old_vel);
        // PRINT_VEC3N(pc->vel);
        // printf("VEL DIFF: %f\n", vel_diff);
        // PRINT_VEC3N(pc->old_angular);
        // PRINT_VEC3N(pc->angular_velocity);
        // printf("AVEL DIFF: %f\n", avel_diff);

        float vel_len = vec3_length(pc->vel);
        float angular_len = vec3_length(pc->angular_velocity);
        const float rest_eps = 0.1f;
        const float rest_ceil = 0.2f;
        // if (vel_diff < rest_eps && vel_len < rest_ceil && avel_diff < rest_eps && angular_len < rest_ceil)
        if (vel_len < rest_ceil && angular_len < rest_ceil)
        {
            pc->rest_counter++;
        }
        else
        {

            pc->rest_counter = 0;
            pc->at_rest = false;
        }
        
        const int REST_MARGIN = 50;
        if (pc->rest_counter >= REST_MARGIN)
        {
            pc->at_rest = true;
            pc->rest_counter = 0;
        }

    }
    
}

mat3 calc_inverse_inertia(PhysicsComponent * pc)
{
    AABB aabb = pc->aabb;
    float w = aabb.extents.x * 2;
    float d = aabb.extents.y * 2;
    float h = aabb.extents.z * 2;
    float w2 = w*w;
    float d2 = d*d;
    float h2 = h*h;
    
    float mass = pc->mass;
    mat3 iia;
    iia.a0 = 12.0f / (mass * (h2 + d2));
    iia.a1 = 0;
    iia.a2 = 0;
    iia.b0 = 0;
    iia.b1 = 12.0f / (mass * (w2 + d2));
    iia.b2 = 0;
    iia.c0 = 0;
    iia.c1 = 0;
    iia.c2 = 12.0f / (mass * (w2 + h2));

    return iia;
    
}

void init_PC(int mesh_node, PhysicsComponent * pc, AABB ai_aabb, CollisionType collision_type, PhysicsType physics_type)
{
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mn = &mns[mesh_node];
    // if (mn->mesh == -1) return;
    AABB aabb;
    if (mn->mesh != -1)
    {
        Mesh * m = &meshes[mn->mesh];
        aabb = m->aabb;

        // aabb = mn->aabb;
        // aabb = ai_aabb;
    }
    else
    {
        aabb = mn->aabb;
        // aabb = ai_aabb;
    }

    pc->center_of_mass = aabb.center;
    pc->aabb = aabb;
    pc->vel = (vec3){0,0,0};
    pc->angular_velocity = (vec3){0,0,0};
    float mass = 200.0f;
    pc->mass = mass;
    pc->restitution = 0.2f;
    pc->mu = 0.5f;
    pc->player_pc = false;
    pc->physics_type = physics_type;

    pc->cd.type = collision_type;
    switch(collision_type)
    {
        case COL_TRIANGLE:
            break;
        case COL_HULL:
            break;
        case COL_CAPSULE:
            print_aabb(&aabb);
            pc->cd.data.capsule.a = (vec3){aabb.center.x,aabb.center.y,aabb.min_z};
            pc->cd.data.capsule.b = (vec3){aabb.center.x,aabb.center.y,aabb.max_z};
            // pc->cd.data.capsule.a = (vec3){aabb.center.x,aabb.center.y,-aabb.extents.z};
            // pc->cd.data.capsule.b = (vec3){aabb.center.x,aabb.center.y,aabb.extents.z};
            // pc->cd.data.capsule.a = (vec3){aabb.center.x,aabb.center.y,0};
            // pc->cd.data.capsule.b = (vec3){aabb.center.x,aabb.center.y,1};
            // pc->cd.data.capsule.radius = MAX(aabb.max_y - aabb.min_y, aabb.max_x - aabb.min_x) / 2;
            pc->cd.data.capsule.radius = 0.2;
            break;
    }
    // pc->cd.type = COL_HULL;
    // pc->cd.data.capsule.a = (vec3){aabb.center.x,aabb.center.y,0};
    // pc->cd.data.capsule.b = (vec3){aabb.center.x,aabb.center.y,1};
    // // causes a bug with epa
    // pc->cd.data.capsule.radius = 0.6;
    // printf("W: %f %f\n", pc->cd.data.capsule.height, pc->cd.data.capsule.radius);
    // PRINT_VEC3N(pc->cd.data.capsule.a);
    // PRINT_VEC3N(pc->cd.data.capsule.b);

    
    // try 6 after


    // float I = 2.0f/5.0f * mass * m->aabb.extents.x * m->aabb.extents.x;

    // pc->inverse_inertia = (vec3)
    // {
    //     1/I,
    //     1/I,
    //     1/I
    // };

    pc->inverse_inertia = calc_inverse_inertia(pc);
    
}

