
#include "utilities/types.h"
#include "utilities/mathf.h"
#include "core/camera.h"
#include "utilities/utilities.h"
#include <assert.h>


void EPA_check_face_visibility(vec3 p, EPAWorkspace * EW)
{
    for (int f = 0; f < EW->indices_count; f++)
    {

        MeshFace * face = &EW->faces[EW->indices[f]];
        float d = vec3_dot(vec3_sub(p, EW->vertices[face->indices[0]].p), face->normal);
        if (d > 0.0f)
        {
            EW->visible_faces_indices[EW->visible_faces_count++] = f;
        }

    }
}



void EPA_check_horizon_edges(MeshFace * face, EPAWorkspace * EW)
{

    float eps = 0.001f;
    for (int i = 0; i < 3; i++)
    {
        HorizonEdge c;
        c.ab[0] = face->indices[i];
        vec3 a = EW->vertices[face->indices[i]].p;
        c.ab[1] = face->indices[(i+1) % 3];
        vec3 b = EW->vertices[face->indices[(i+1) % 3]].p;
        bool found = false;

        for (int h = 0; h < EW->horizon_edges_count; h++)
        {

            HorizonEdge * edge = &EW->horizon_edges[h];
            vec3 a0 = EW->vertices[edge->ab[0]].p;
            vec3 b0 = EW->vertices[edge->ab[1]].p;

            // It may be correct to only check the opposite orientation (a == b, b == a)
            if ((vec3_equal(a0, a, eps) && vec3_equal(b0, b, eps) ||
                (vec3_equal(b0, a, eps) && vec3_equal(a0, b, eps))))
            {
                EW->horizon_counts[h]++;
                found = true;
            }

        }
        if (found == false)
        {
            EW->horizon_edges[EW->horizon_edges_count++] = c;
            EW->horizon_counts[EW->horizon_counts_count++] = 1;
        }
    }
    
}



bool EPA_add_horizon_face_to_hull_faces(size_t p_idx, HorizonEdge e, EPAWorkspace * EW, vec3 center)
{
    
    vec3 A = EW->vertices[p_idx].p;
    vec3 B = EW->vertices[e.ab[0]].p;
    vec3 C = EW->vertices[e.ab[1]].p;
    vec3 normal = compute_tri_normal(A, B, C);
    if (vec3_length(normal) <= 0)
    {
        PRINT_VEC3N(A);
        PRINT_VEC3N(B);
        PRINT_VEC3N(C);
        return false;
    }

    MeshFace * f = &EW->faces[EW->faces_count++];
    
    f->indices[0] = p_idx;
    f->indices[1] = e.ab[0];
    f->indices[2] = e.ab[1];
    f->normal = compute_tri_normal(A, B, C);
    if (vec3_dot(vec3_sub(center, A), f->normal) > 0)
    {
        
        f->indices[0] = e.ab[0];
        f->indices[1] = p_idx;
        f->normal = compute_tri_normal(B, A, C);
        if (vec3_length(f->normal) <= 0)
        {
            PRINT_VEC3N(A);
            PRINT_VEC3N(B);
            PRINT_VEC3N(C);
            EW->faces_count--;
            return false;
        }
    }
    assert(vec3_dot(vec3_sub(center, A), f->normal) <= 0);

    EW->indices[EW->indices_count++] = EW->faces_count - 1;
    return true;
}


void EPA_expand_hull
    (
        vec3 p,
        int index,
        EPAWorkspace * EW,
        vec3 centroid
    )
{

    EPA_check_face_visibility(p, EW);

    if (EW->visible_faces_count <= 0)
    {
        
        printf("NO VISIBLE\n");
        return;
    }
    for (int f = 0; f < EW->visible_faces_count; f++)
    {
        MeshFace * face = &EW->faces[EW->indices[EW->visible_faces_indices[f]]];
        EPA_check_horizon_edges(face, EW);
    }

    // delete visible faces from master list
    for (int f = 0; f < EW->visible_faces_count; f++)
    {
        REMOVE_FROM_ARR(EW->indices, EW->visible_faces_indices[f] - f, &EW->indices_count);
    }

    bool did_emit = false;
    for (int h = 0; h < EW->horizon_edges_count; h++)
    {
        HorizonEdge * edge = &EW->horizon_edges[h];
        if (EW->horizon_counts[h] == 1)
        {
            // connect p to horizon edges
            if (EPA_add_horizon_face_to_hull_faces(index, *edge, EW, centroid))
            {
                did_emit = true;
            }

        }
    }
    EW->visible_faces_count = 0;
    EW->horizon_counts_count = 0;
    EW->horizon_edges_count = 0;
    
}


vec3 support_capsule(CapsuleData* cap, vec3 dir, mat4 inv) {
    dir = mat4_mul_direction(inv, dir);
    if (vec3_length(dir) <= 1e-6)
    {
        return cap->a;
    }
    float dot_a = vec3_dot(cap->a, dir);
    float dot_b = vec3_dot(cap->b, dir);
    
    vec3 furthest_line_point = (dot_a > dot_b) ? cap->a : cap->b;
    
    vec3 normalized_dir = vec3_normalize(dir);
    vec3 sphere_offset = vec3_scale(normalized_dir, cap->radius);
    
    return vec3_add(furthest_line_point, sphere_offset);
}

void support(vec3 * v, size_t * index, vec3 * verts, size_t cnt, mat4 model_matrix, mat4 inv, vec3 dir)
{
    
    float best_dot = -INFINITY;
    size_t best_idx = 0;
    vec3 best;
    dir = mat4_mul_direction(inv, dir);

    for(int i = 0; i < cnt; i++)
    {
        vec3 v = verts[i];
        float d = vec3_dot(v, dir);

        if(d > best_dot)
        {
            best_dot = d;
            best_idx = i;
            best = v;
        }
    }

    *v = best;
    *index = best_idx;
}

SupportVertex support_diff(CollisionData a_cd, CollisionData b_cd, mat4 a_mat, mat4 b_mat, mat4 a_inv, mat4 b_inv, vec3 dir)
{

    SupportVertex p;
    switch(a_cd.type)
    {
        case COL_TRIANGLE:

            support(&p.a, &p.index_a, a_cd.data.hull.pts, a_cd.data.hull.count, a_mat, a_inv, dir);
            break;
        case COL_HULL:
            support(&p.a, &p.index_a, a_cd.data.hull.pts, a_cd.data.hull.count, a_mat, a_inv, dir);
            break;
        case COL_CAPSULE:
            p.a = support_capsule(&a_cd.data.capsule, dir, a_inv);
            break;
    }
    switch(b_cd.type)
    {
        case COL_TRIANGLE:

            support(&p.b, &p.index_b, b_cd.data.hull.pts, b_cd.data.hull.count, b_mat, b_inv, vec3_scale(dir, -1));
            break;
     case COL_HULL:
            support(&p.b, &p.index_b, b_cd.data.hull.pts, b_cd.data.hull.count, b_mat, b_inv, vec3_scale(dir, -1));
            break;
        case COL_CAPSULE:
            p.b = support_capsule(&b_cd.data.capsule, vec3_scale(dir, -1), b_inv);
            break;
    }
    p.p = vec3_sub(mat4_mul_point(a_mat, p.a), mat4_mul_point(b_mat, p.b));
    return p;
}



CollisionPoints epa(CollisionData a_cd, CollisionData b_cd, mat4 a_mat, mat4 b_mat, mat4 a_inv, mat4 b_inv, Simplex * s, EPAWorkspace * EW)
{



    EW->vertices_count = 0;
    EW->faces_count = 0;
    EW->indices_count = 0;
    EW->horizon_edges_count = 0;
    EW->horizon_counts_count = 0;
    EW->visible_faces_count = 0;

    
    // expands the simplex by adding vertices until we find the shortest normal from a face that is on the original polygon
    // simplex -> polytope
    // probably redefine faces as indices into vertex array instead of just raw points

    // break simplex up into faces and vertices here into a polytope
    // then expand it similarly to incremental hull
    
    // since we're in 3D, we need a list of unique edges
    // make sure to get face normals
    // find edge closest to the origin


    SupportVertex pA = s->points[A];
    SupportVertex pB = s->points[B];
    SupportVertex pC = s->points[C];
    SupportVertex pD = s->points[D];
    
    EW->vertices[A] = pA;
    EW->vertices[B] = pB;
    EW->vertices[C] = pC;
    EW->vertices[D] = pD;
    EW->vertices_count = 4;
    MeshFace * f = EW->faces;
    EW->faces_count = 4;

    vec3 centroid = vec3_add(vec3_add(vec3_add
        (pA.p, pB.p), pC.p), pD.p);
    centroid = vec3_scale(centroid, 0.25f);
    
    f[0].indices[A] = A;
    f[0].indices[B] = B;
    f[0].indices[C] = C;
    f[0].normal = compute_tri_normal(pA.p, pB.p, pC.p);
    vec3 inside = vec3_sub(pD.p, pA.p);
    if (vec3_dot(inside, f[0].normal) > 0)
    {
        f[0].indices[A] = B;
        f[0].indices[B] = A;
        f[0].normal = compute_tri_normal(pB.p, pA.p, pC.p);

    }

    
    f[1].indices[A] = D;
    f[1].indices[B] = C;
    f[1].indices[C] = B;
    f[1].normal = compute_tri_normal(pD.p, pC.p, pB.p);
    inside = vec3_sub(pA.p, pD.p);
    if (vec3_dot(inside, f[1].normal) > 0)
    {
        f[1].indices[A] = C;
        f[1].indices[B] = D;
        f[1].normal = compute_tri_normal(pC.p, pD.p, pB.p);
        
    }


    f[2].indices[A] = A;
    f[2].indices[B] = C;
    f[2].indices[C] = D;
    f[2].normal = compute_tri_normal(pA.p, pC.p, pD.p);
    inside = vec3_sub(pB.p, pA.p);
    if (vec3_dot(inside, f[2].normal) > 0)
    {
        f[2].indices[A] = C;
        f[2].indices[B] = A;
        f[2].normal = compute_tri_normal(pC.p, pA.p, pD.p);
        
    }


    f[3].indices[A] = A;
    f[3].indices[B] = D;
    f[3].indices[C] = B;
    f[3].normal = compute_tri_normal(pA.p, pD.p, pB.p);
    inside = vec3_sub(pC.p, pA.p);
    if (vec3_dot(inside, f[3].normal) > 0)
    {
        f[3].indices[A] = D;
        f[3].indices[B] = A;
        f[3].normal = compute_tri_normal(pD.p, pA.p, pB.p);
        
    }

    if (vec3_length(f[0].normal) <= 0)
    {
        PRINT_VEC3N(f[0].normal);
    }
    if (vec3_length(f[1].normal) <= 0)
    {
        PRINT_VEC3N(f[1].normal);
    }
    if (vec3_length(f[2].normal) <= 0)
    {
        PRINT_VEC3N(f[2].normal);
    }
    if (vec3_length(f[3].normal) <= 0)
    {
        PRINT_VEC3N(f[3].normal);
    }
    
    float min_dist = INFINITY;
    float s_dist = 0;


    int safety = 100;
    int iter = 0;

    int * indices = EW->indices;
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;
    EW->indices_count = 4;
    
    vec3 min_normal = f[0].normal;
    int smallest_index = 0;
    float last_dist = 0, cur_dist = INFINITY;

    bool term = false;
    while (fabsf(s_dist - min_dist) > 0.001f)
    {
        min_dist = INFINITY;
        last_dist = cur_dist;
        
        vec3 c = {0,0,0};
        

        for (int i = 0; i < EW->indices_count; i++)
        {
            MeshFace * face = &f[indices[i]];
            
            vec3 normal = face->normal;
            float dist = vec3_dot(normal, EW->vertices[face->indices[A]].p);


            if (dist < 0.0f)
            {
                dist *= -1;
            }
            if (dist < min_dist)
            {
                smallest_index = i;
                min_normal = normal;
                min_dist = dist;
                if (min_dist != min_dist) min_dist = 0;
            }
        }

        SupportVertex support = support_diff(a_cd, b_cd, a_mat, b_mat, a_inv, b_inv, min_normal);
        s_dist = vec3_dot(min_normal, support.p);
        cur_dist = fabsf(s_dist-min_dist);

        if (iter > safety)
        {
            term = true;
            for (int i = 0; i < EW->indices_count; i++)
            {
                MeshFace * face = &f[indices[i]];
                float dist = vec3_dot(face->normal, vec3_sub(support.p, EW->vertices[face->indices[A]].p));
                printf("FACE: %d DIST: %f\n", i, dist);
                PRINT_VEC3N(EW->vertices[face->indices[0]].p);
                PRINT_VEC3N(EW->vertices[face->indices[1]].p);
                PRINT_VEC3N(EW->vertices[face->indices[2]].p);
                PRINT_VEC3N(face->normal);
            }
            PRINT_VEC3N(support.p);
            printf("DIST: %f\n", cur_dist);
            printf("ILLEGAL BREAK\n");
            break;
        }
        if (cur_dist > 0.001f && !term)
        {
            EW->vertices[EW->vertices_count++] = support;
            int p_idx = EW->vertices_count-1;
            
            EPA_expand_hull
            (
                 support.p, p_idx,
                 EW,
                 centroid
             );
            
        }
        else
        {
            break;
        }
        iter++;
                
    }



    float max_dist = -INFINITY;
    int largest = 0;
    SupportVertex * sv = EW->vertices;

    
    MeshFace * face = &f[indices[smallest_index]];
    vec3 P = project_point_plane((vec3){0,0,0}, EW->vertices[face->indices[A]].p, face->normal);

    float b0, b1, b2;
    compute_3d_barycentric(EW->vertices[face->indices[A]].p, EW->vertices[face->indices[B]].p, EW->vertices[face->indices[C]].p, P, &b0, &b1, &b2);

    SupportVertex * v = &sv[face->indices[A]];
    MeshFace * closest_face = &f[indices[smallest_index]];
    vec3 v0A = vec3_scale(sv[closest_face->indices[A]].a, b0);
    vec3 v1A = vec3_scale(sv[closest_face->indices[B]].a, b1);
    vec3 v2A = vec3_scale(sv[closest_face->indices[C]].a, b2);
    vec3 coordA = {v0A.x + v1A.x + v2A.x, v0A.y + v1A.y + v2A.y, v0A.z + v1A.z + v2A.z};
    vec3 v0B = vec3_scale(sv[closest_face->indices[A]].b, b0);
    vec3 v1B = vec3_scale(sv[closest_face->indices[B]].b, b1);
    vec3 v2B = vec3_scale(sv[closest_face->indices[C]].b, b2);
    vec3 coordB = {v0B.x + v1B.x + v2B.x, v0B.y + v1B.y + v2B.y, v0B.z + v1B.z + v2B.z};
    
    
    CollisionPoints p;
    p.collision = true;
    p.d = min_dist;
    p.normal = vec3_scale(min_normal, -1);
    p.a = coordA;
    p.b = coordB;

    vec3 minkowski = vec3_sub(coordA, coordB);


    
    if (term) p.collision = false;


    return p;



    
    // while mindist == inf
    
        // for each face in simplex
            // ab = b - a
            // ac = c - a
            // normal = cross(ab, ac)
            // dist = dot(normal, a);
            // (change normal if outward facing (use center)
            // if dist < 0
                // normal *= -1
                // dist *= -1
            // get smallest distance and save normal and index (b?)

        // support = support(A, B, min_normal)
        // sdist = dot(min_normal, support)
        // if (fabsf(sdist-mindist) > 0.001

            // expand hull (basically incremental hull here)
            // collect visible faces
                // meaning if dot(p - face_vertex, face normal) > eps
                    // it is visible
            // get horizon edges
            // delete visible faces
            // add horizon faces with count == 1
            
            // OLD NOTES:
                // mindist = inf
                // polytope.splice(minindex, 0, support)

                // repair polytope
                // get a list of unique edges
                // for each face
                    // if same direction as support
                        // add if it is unique
                // for each unique edge
                    // add to new faces
                    // (A, B, polytope.size)
                // polytope.push support

            // calculate the normals of the new faces
                // retrieve new min face
                // test against the lowest min so far on the outer loop
            // push new face and normal

    // return min_normal * mindist + 0.001
    
}


// on hull decomposition
// check reflex edges
// a mesh is convex only if each point lies behind the plane of each face
// so compute normal or face, then dot(vertex - A, normal) and if > EPS, then it is in front of a face and the mesh is no longer convex. use that information to spli the mesh, clip triangles, and continue forward

void add_point_to_simplex(Simplex * s, SupportVertex p)
{
    if (s->count >= 4)
    {
        printf("Too many points\n");
        return;
    }

    for (int i = s->count; i > 0; i--)
    {
        s->points[i] = s->points[i-1];
    }
    s->points[A] = p;
    s->count++;
    
}

void remove_point_from_simplex(Simplex * s, int i)
{
    if (i >= 4)
    {
        printf("Index exceeded\n");
        return;
    }
    
    for (int idx = i; idx < s->count-1; idx++)
    {
        s->points[idx] = s->points[idx+1];
    }
    s->count--;

}

void replace_simplex_points(Simplex * s, SupportVertex abcd[4], int count)
{
    s->count = count;
    for (int i = 0; i < count; i++)
    {
        s->points[i] = abcd[i];
    }
}



vec3 find_next_direction(Simplex * s, vec3 direction)
{
    

    int arr[4][3]
        = {
            A, B, C,
            A, C, D,
            A, D, B,
            B, C, D
        };
    int other_points[4] = {D, B, C, A};

    vec3 dir = {-999, -999, -999};
    switch(s->count)
    {
        case 1:
            {
                
                dir = (vec3_scale(s->points[A].p, -1));

            }
            break;
        case 2:
            {

                vec3 AB = vec3_sub(s->points[B].p, s->points[A].p);
                vec3 AO = vec3_scale(s->points[A].p, -1);

                if (vec3_dot(AB, AO) < 0)
                {
                    remove_point_from_simplex(s, B);
                    dir = AO;
                    break;
                }
                else
                {
                    dir = (vec3_cross(vec3_cross(AB, AO), AB));
                    if (vec3_length(dir) <= 1e-6)
                    {
                        
                            vec3 axis =
                                fabsf(AB.x) < fabsf(AB.y)
                                    ? (fabsf(AB.x) < fabsf(AB.z) ? (vec3){1,0,0} : (vec3){0,0,1})
                                    : (fabsf(AB.y) < fabsf(AB.z) ? (vec3){0,1,0} : (vec3){0,0,1});

                            dir = vec3_cross(AB, axis);     
                            if (vec3_length(dir) < 1e-6)
                            {

                                dir = vec3_cross(AB, (vec3){0,0,1});
                            }
                    }
                    break;
                }

            }
            break;
        case 3:
            {

                vec3 normal = compute_tri_normal(s->points[A].p, s->points[B].p, s->points[C].p);
                vec3 AB = vec3_sub(s->points[B].p, s->points[A].p);
                vec3 AC = vec3_sub(s->points[C].p, s->points[A].p);
                vec3 AO = vec3_scale(s->points[A].p, -1);

                vec3 ABp = vec3_cross(AB, normal);
                vec3 ACp = vec3_cross(normal, AC);

                float ACd = vec3_dot(ACp, AO);
                if (ACd > 0)
                {
                    if (vec3_dot(AC, AO) > 0)
                    {
                        
                        dir = vec3_cross(vec3_cross(AC, AO), AC);
                        if (vec3_length(dir) <= 1e-6)
                        {
                        
                            vec3 axis =
                                fabsf(AC.x) < fabsf(AC.y)
                                    ? (fabsf(AC.x) < fabsf(AC.z) ? (vec3){1,0,0} : (vec3){0,0,1})
                                    : (fabsf(AC.y) < fabsf(AC.z) ? (vec3){0,1,0} : (vec3){0,0,1});

                            dir = vec3_cross(AC, axis);     
                            if (vec3_length(dir) < 1e-6)
                            {

                                dir = vec3_cross(AC, (vec3){0,0,1});
                            }
                        }
                        remove_point_from_simplex(s, B);
                    }
                    else
                    {
                        dir = vec3_cross(vec3_cross(AB, AO), AB);
                        if (vec3_length(dir) <= 1e-6)
                        {
                        
                            vec3 axis =
                                fabsf(AB.x) < fabsf(AB.y)
                                    ? (fabsf(AB.x) < fabsf(AB.z) ? (vec3){1,0,0} : (vec3){0,0,1})
                                    : (fabsf(AB.y) < fabsf(AB.z) ? (vec3){0,1,0} : (vec3){0,0,1});

                            dir = vec3_cross(AB, axis);     
                            if (vec3_length(dir) < 1e-6)
                            {

                                dir = vec3_cross(AB, (vec3){0,0,1});
                            }
                        }
                        
                    }
                    break;
                    
                }
                float ABd = vec3_dot(ABp, AO);
                if (ABd > 0)
                {
                    if (vec3_dot(AB, AO) > 0)
                    {
                        
                        dir = vec3_cross(vec3_cross(AB, AO), AB);
                        if (vec3_length(dir) <= 1e-6)
                        {
                        
                            vec3 axis =
                                fabsf(AB.x) < fabsf(AB.y)
                                    ? (fabsf(AB.x) < fabsf(AB.z) ? (vec3){1,0,0} : (vec3){0,0,1})
                                    : (fabsf(AB.y) < fabsf(AB.z) ? (vec3){0,1,0} : (vec3){0,0,1});

                            dir = vec3_cross(AB, axis);     
                            if (vec3_length(dir) < 1e-6)
                            {

                                dir = vec3_cross(AB, (vec3){0,0,1});
                            }
                        }
                        remove_point_from_simplex(s, C);
                        break;
                    }
                }
                if (vec3_dot(normal, AO) > 0)
                {

                    dir = normal;
                    break;
                }
                else
                {
                    
                    SupportVertex Cr = s->points[C];
                    SupportVertex Br = s->points[B];
                    s->points[B] = Cr;
                    s->points[C] = Br;
                    dir = vec3_scale(normal, -1);
                    break;
                }
                

            }
            break;
        case 4:
            {

                vec3 AO = vec3_scale(s->points[A].p, -1);

                vec3 centroid = vec3_add(vec3_add(vec3_add
                    (s->points[A].p, s->points[B].p), s->points[C].p), s->points[D].p);
                centroid = vec3_scale(centroid, 0.25f);
                s->centroid = centroid;

                vec3 nabc = vec3_cross
                (
                    vec3_sub(s->points[B].p, s->points[A].p),
                    vec3_sub(s->points[C].p, s->points[A].p)
                );
                float d = vec3_dot(vec3_sub(s->points[D].p, s->points[A].p), nabc);
                bool swap = false;
                if (vec3_dot(nabc, AO) > 0)
                {
                    if (swap)
                    {
                        
                        replace_simplex_points(s, (SupportVertex[4]){s->points[B], s->points[A], s->points[C]}, 3);
                    }
                    else
                    {
                        
                        remove_point_from_simplex(s, D);
                    }
                    dir = nabc;
                    break;
                }
                swap = false;
                vec3 nacd = vec3_cross
                (
                    vec3_sub(s->points[C].p, s->points[A].p),
                    vec3_sub(s->points[D].p, s->points[A].p)
                );
                d = vec3_dot(vec3_sub(s->points[B].p, s->points[A].p), nacd);
                if (vec3_dot(nacd, AO) > 0)
                {
                    if (swap)
                    {
                        replace_simplex_points(s, (SupportVertex[4]){s->points[C], s->points[A], s->points[D]}, 3);
                        
                    }
                    else
                    {
                        remove_point_from_simplex(s, B);
                    }
                    dir = nacd;
                    break;
                }
                vec3 nadb =  vec3_cross
                (
                    vec3_sub(s->points[D].p, s->points[A].p),
                    vec3_sub(s->points[B].p, s->points[A].p)
                );
                d = vec3_dot(vec3_sub(s->points[C].p, s->points[A].p), nadb);
                swap = false;

                if (vec3_dot(nadb, AO) > 0)
                {

                    if (swap)
                    {

                        replace_simplex_points(s, (SupportVertex[4]){s->points[D], s->points[A], s->points[B]}, 3);
                        
                    }
                    else
                    {
                        SupportVertex Br = s->points[B];
                        s->points[B] = s->points[D];
                        s->points[C] = Br;
                        s->count--;
                        
                    }
                    
                    dir = nadb;
                    break;
                }
            }
            break;
    }

    return dir;
}



bool GJK(CollisionData a_cd, CollisionData b_cd, mat4 a_mat, mat4 b_mat, mat4 a_inv, mat4 b_inv, CollisionPoints * collision_points, EPAWorkspace * EW)
{


    vec3 O = {0, 0, 0};

    bool searching = true;
    bool collision = false;
    vec3 dir = {0, 0, 1};

    Simplex s = {0};

    int safety = 1000;
    int iter = 0;
    while(searching && iter < safety)
    {
        iter++;

        SupportVertex support = support_diff(a_cd, b_cd, a_mat, b_mat, a_inv, b_inv, dir);
        if (vec3_dot(support.p, dir) <= 0)
        {

            break;
        }

        for (int i = 0; i < s.count; i++)
        {
            if (vec3_equal(support.p, s.points[i].p, 0.001f))
            {
                return false;
            }
        }
        add_point_to_simplex(&s, support);
        
        dir = find_next_direction(&s, dir);
        if (dir.x == -999 && dir.y == -999 && dir.z == -999)
        {
            searching = false;
            collision = true;
            break;
        }
    }
        
    double epa_time = 0;
    
    if (collision)
    {
        CollisionPoints p = epa(a_cd, b_cd, a_mat, b_mat, a_inv, b_inv, &s, EW);
        p.normal_impulse = 0;
        p.tangent_impulse = 0;
        if (p.d > collision_points->d && p.collision)
        {
            
            p.collision = true;
            *collision_points = p;
            return true;
        }
        else
        {
            return false;
        }
    }
    else
    {
        return false;
    }
    

    // origin is 0,0,0
    // pick random direction
    // newest point always becomes A (and always stays first)
    // possibly add a centroid calc for tetrahedron to check face normals against
    // loop
    
        // vec3 support = support_diff(a, b, dir);
        // if dot(diff, dir) <= 0 no collision
        // add support to simplex
        // if (process_simplex(simplex, dir))
            // if (origin is outside of bounds, discard opposite point)
                // case 1 - just change dir to -A
                // case 2 - because origin can never be beyond B (because we just searched in that direction) we only check if origin is behind A (dot (AB, AO) where AO = -A)
                // if dot <= 0, discard B and dir becomes -A
                // else dir = cross(cross(AB, AO), AB)
                // case 3:
                    // compute ABC normal
                    // cross normal with two edges (cannot lie within BC):
                    // ABp = cross(AB, ABC)
                    // ACp = cross(ABC, AC)
                    // take dot product with AO
                    // if positive, throw opposite point away
                        // if above or below, dot (ABC, AO)
                        // if positive, dir becomes ABC
                        // if negative, dir becomes -ABC
                // case 4:
                    // repeat for all faces:
                        // normal = cross(B-A, C-A)
                        // if (dot(normal, AO) > 0
                            // discard other point
                            // direction = normal of remaining points
                    // if none discard, it must be a collision

        return false;
}

