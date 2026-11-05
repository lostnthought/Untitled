#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "utilities/types.h"
#include "utilities/utilities.h"
#include "utilities/mathf.h"
#include "core/mesh.h"
#include "uthash/uthash.h"



// ADJACENCY MAP
// Builds a map of triangles that are adjacent by storing references per triangle. Uses UTHASH.
typedef struct WeldedVertexKey
{
    int32_t x;
    int32_t y;
    int32_t z;
} WeldedVertexKey;

WeldedVertexKey get_welded_key(vec3 pos, float epsilon)
{
    WeldedVertexKey key;
    key.x = (int32_t)floorf(pos.x / epsilon);
    key.y = (int32_t)floorf(pos.y / epsilon);
    key.z = (int32_t)floorf(pos.z / epsilon);
    return key;
}

typedef struct WeldedEntry
{
    WeldedVertexKey key;
    size_t index;
    UT_hash_handle hh;

} WeldedEntry;

typedef struct EdgePairKey
{
    int32_t a;
    int32_t b;

} EdgePairKey;

typedef struct EdgePairEntry
{
    EdgePairKey key;
    Vector tris;
    UT_hash_handle hh;

} EdgePairEntry;

EdgePairEntry * edge_pair_table = NULL;
WeldedEntry * welded_table = NULL;


void build_adjacency_map(Mesh * mesh)
{
    
    if (!mesh->tri_adjacency)
    {
        
        mesh->tri_adjacency = calloc(mesh->tris.count, sizeof(TriangleAdjacency));
    }
    else
    {
        free(mesh->tri_adjacency);
        mesh->tri_adjacency = calloc(mesh->tris.count, sizeof(TriangleAdjacency));
    }
    MeshFace * fs = CAST_VEC(mesh->tris, MeshFace);
    Point * pts = CAST_VEC(mesh->vertices, Point);
    WeldedVertexKey * keys = NULL;
    size_t * index_map = NULL;
    index_map = calloc(mesh->vertices.count, sizeof(size_t));
    for (int i = 0; i < mesh->vertices.count; i++)
    {
        WeldedVertexKey key = get_welded_key(pts[i].pos, 0.0001f);
        WeldedEntry * entry = NULL;

        HASH_FIND(hh, welded_table, &key, sizeof(WeldedVertexKey), entry);
        if (entry)
        {
            index_map[i] = entry->index;
        }
        else
        {

            WeldedEntry * z = malloc(sizeof(WeldedEntry));
            memset(z, 0, sizeof(WeldedEntry));
            z->key = key;
            z->index = i;
            index_map[i] = i;
            HASH_ADD(hh, welded_table, key, sizeof(WeldedVertexKey), z); 
        }
    }
    for (int i = 0; i < mesh->tris.count; i++)
    {

        MeshFace * f = &fs[i];
        for (int j = 0; j < 3; j++)
        {
            size_t i0 = index_map[f->indices[j]];
            size_t i1 = index_map[f->indices[(j+1)%3]];

            EdgePairKey ep;
            if (i0 < i1)
            {
                ep = (EdgePairKey){i0, i1};
            }
            else
            {
                ep = (EdgePairKey){i1, i0};
                
            }
            EdgePairEntry * epe = NULL;
            HASH_FIND(hh, edge_pair_table, &ep, sizeof(EdgePairKey), epe);
            if (epe)
            {
                // add on to entry
                size_t * v = vector_expand(&epe->tris, 1);
                *v = i;
                
            }
            else
            {
                // add entry
                EdgePairEntry * z = malloc(sizeof(EdgePairEntry));
                memset(z, 0, sizeof(EdgePairEntry));
                z->key = ep;
                init_vector(&z->tris, 1, sizeof(size_t));
                size_t * v = CAST_VEC(z->tris,  size_t);
                *v = i;
                HASH_ADD(hh, edge_pair_table, key, sizeof(EdgePairKey), z); 
            }
            
        }
        
        
    }

    for (int i = 0; i < mesh->tris.count; i++)
    {
        
        init_vector(&mesh->tri_adjacency[i].adjacent_indices, 0, sizeof(size_t));
        
    }

    EdgePairEntry * s;
    EdgePairEntry * tmp;

    HASH_ITER(hh, edge_pair_table, s, tmp){


        size_t * tri_indices = CAST_VEC(s->tris, size_t);
        for (int i = 0; i < s->tris.count; i++)
        {
            
            MeshFace * mf = &fs[tri_indices[i]];
            for (int j = 0; j < s->tris.count; j++)
            {

                if (i == j || tri_indices[i] == tri_indices[j])
                {
                    continue;
                }

                
                size_t * idx = vector_expand(&mesh->tri_adjacency[tri_indices[i]].adjacent_indices, 1);
                *idx = tri_indices[j];
            }
        }

        
        if (s->tris.buffer) free(s->tris.buffer);
        HASH_DEL(edge_pair_table, s);
        free(s);
    }

    WeldedEntry * s0;
    WeldedEntry * tmp0;
    HASH_ITER(hh, welded_table, s0, tmp0){


        HASH_DEL(welded_table, s0);
        free(s0);
    }
    if (index_map) free(index_map);
}

int find_mn_by_name(char name[256])
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        MeshNode * mn = &mns[i];
        if (mn->free) continue;
        
        if (strcmp(mn->name, name) == 0) return i;
        
    }
    return -1;
    
    
}



void deduplicate(Vector * in_verts, Vector * faces)
{
    
    MeshFace * tris = CAST_VEC(*faces, MeshFace);
    vec3 * in_pts = CAST_VEC(*in_verts, vec3);
    Vector out_verts;
    init_vector(&out_verts, 0, sizeof(vec3));


    for (int i = 0; i < faces->count; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            
            MeshFace * f = &tris[i];
            vec3 pt = in_pts[f->indices[j]];
            bool unique = true;

            vec3 * verts = CAST_VEC(out_verts, vec3);
            for (int k = 0; k < out_verts.count; k++)
            {
                vec3 v = verts[k];
                if (vec3_equal(pt, v, 0.01f))
                {
                    unique = false;
                    f->indices[j] = k;
                    break;
                }
            }
    
            if (unique)
            {
                vec3 * point = vector_expand(&out_verts, 1);
                *point = pt;

                f->indices[j] = out_verts.count-1;
            
            }
        }

    }

    free(in_verts->buffer);
    *in_verts = out_verts;
}



// remove extraneous points from hull that aren't referenced by faces
void clean_up_hull(Vector * in_vertices, Vector * in_faces)
{

    deduplicate(in_vertices, in_faces);

    Vector new_points;
    init_vector(&new_points, 0, sizeof(vec3));
    MeshFace * faces = CAST_VEC(*in_faces, MeshFace);
    vec3 * old_vertices = CAST_VEC(*in_vertices, vec3);
    for (int i = 0; i < in_faces->count; i++)
    {
        MeshFace * face = &faces[i];
        for (int j = 0; j < 3; j++)
        {
            vec3 p = old_vertices[face->indices[j]];

            bool is_new = true;

            for (int k = 0; k < new_points.count; k++)
            {
                vec3 * np = CAST_VEC(new_points, vec3);
                if (vec3_equal(p, np[k], 0.01f))
                {
                    is_new = false;
                    
                    face->indices[j] = k;
                    break;
                    
                }
            }

            if (is_new)
            {
                vec3 * new_pt = vector_expand(&new_points, 1);
                *new_pt = p;
                face->indices[j] = new_points.count - 1;
            }

            
        }
        
    }
    // printf("%p\n", hull->vertices.buffer);
    free(in_vertices->buffer);
    *in_vertices = new_points;
    
}




// INCREMENTAL HULL
void check_face_visibility(vec3 p, Vector vertices, Vector hull_faces, Vector hull_indices, Vector * visible_faces_indices)
{
    MeshFace * faces = CAST_VEC(hull_faces, MeshFace);
    int * indices = CAST_VEC(hull_indices, int);
    vec3 * points = CAST_VEC(vertices, vec3);
    for (int f = 0; f < hull_indices.count; f++)
    {

        MeshFace * face = &faces[indices[f]];
        float d = vec3_dot(vec3_sub(p, points[face->indices[0]]), face->normal);
        if ((d) > 0.0f)
        {
            int * vfi = vector_expand(visible_faces_indices, 1);
            *vfi = f;
        }

    }
}



void check_horizon_edges(MeshFace * face, Vector vertices, Vector * edges, Vector * counts)
{

    vec3 * points = CAST_VEC(vertices, vec3);
    float eps = 0.001f;
    for (int i = 0; i < 3; i++)
    {
        HorizonEdge * horizon_edges = CAST_VEC(*edges, HorizonEdge);
        int * horizon_counts = CAST_VEC(*counts, int);
        HorizonEdge c;
        c.ab[0] = face->indices[i];
        vec3 a = points[face->indices[i]];
        c.ab[1] = face->indices[(i+1) % 3];
        vec3 b = points[face->indices[(i+1) % 3]];
        bool found = false;

        for (int h = 0; h < edges->count; h++)
        {

            HorizonEdge * edge = &horizon_edges[h];
            vec3 a0 = points[edge->ab[0]];
            vec3 b0 = points[edge->ab[1]];

            // It may be correct to only check the opposite orientation (a == b, b == a)
            if ((vec3_equal(a0, a, eps) && vec3_equal(b0, b, eps) ||
                (vec3_equal(b0, a, eps) && vec3_equal(a0, b, eps))))
            {
                horizon_counts[h]++;
                found = true;
            }

        }
        if (found == false)
        {
            HorizonEdge * ep = vector_expand(edges, 1);
            *ep = c;
            int * count = vector_expand(counts, 1);
            *count = 1;
        }
    }
    
}



void add_horizon_face_to_hull_faces(size_t p_idx, HorizonEdge e, Vector vertices, vec3 center, Vector * hull_faces, Vector * hull_indices, bool epa)
{
    
    vec3 * points = CAST_VEC(vertices, vec3);
    vec3 A = points[p_idx];
    vec3 B = points[e.ab[0]];
    vec3 C = points[e.ab[1]];

    MeshFace * f = vector_expand(hull_faces, 1);
    
    f->indices[0] = p_idx;
    f->indices[1] = e.ab[0];
    f->indices[2] = e.ab[1];
    f->normal = compute_tri_normal(A, B, C);
    if (vec3_dot(vec3_sub(center, A), f->normal) > 0)
    {
        
        f->indices[0] = e.ab[0];
        f->indices[1] = p_idx;
        f->normal = compute_tri_normal(B, A, C);
    }
    assert(vec3_dot(vec3_sub(center, A), f->normal) <= 0);

    int * i = vector_expand(hull_indices, 1);
    *i = hull_faces->count-1;
}


void expand_hull
    (
        vec3 p,
        int index,
        Vector * vertices,
        Vector * hull_faces,
        Vector * hull_indices,
        Vector * horizon_edges,
        Vector * horizon_counts,
        Vector * visible_faces_indices,
        vec3 centroid
    )
{

    MeshFace * faces = CAST_VEC(*hull_faces, MeshFace);
    int * indices = CAST_VEC(*hull_indices, int);

    check_face_visibility(p, *vertices, *hull_faces, *hull_indices, visible_faces_indices);

    if (visible_faces_indices->count <= 0) return;
    int * visible_indices = CAST_VEC(*visible_faces_indices, int);
    for (int f = 0; f < visible_faces_indices->count; f++)
    {
        MeshFace * face = &faces[indices[visible_indices[f]]];
        check_horizon_edges(face, *vertices, horizon_edges, horizon_counts);
    }

    // delete visible faces from master list
    int * counts = CAST_VEC(*horizon_counts, int);
            
    for (int f = 0; f < visible_faces_indices->count; f++)
    {
        REMOVE_FROM_VEC(hull_indices, visible_indices[f] - f, int);
    }

    HorizonEdge * edges = CAST_VEC(*horizon_edges, HorizonEdge);
    for (int h = 0; h < horizon_edges->count; h++)
    {
        HorizonEdge * edge = &edges[h];
        if (counts[h] == 1)
        {
            // connect p to horizon edges
            add_horizon_face_to_hull_faces(index, *edge, *vertices, centroid, hull_faces, hull_indices, true);

        }
    }
    
    visible_faces_indices->count = 0;
    horizon_edges->count = 0;
    horizon_counts->count = 0;
    
}



// notes on incremental hull
// start with a point cloud of deduplicated vertices
// mesh must be convex already (use a decomposition algo)
// before adding new face, check length(n) < eps, if so skip (degenerate faces)

// start with 4 points:
// pick a random point
// find furthest point from that point
// find futhest point from that edge:
// ab = B - A
// ap = P - A
// dist = length(cross(ap, ab)) / length(ab)
// if cross(B-A,C-A) is zero, there is no triangle
// if d < eps always, all points are colinear, this is 2D.

// find furthest point from that triangle:
    // a = reference point
    // n = normalize(cross(B-A, C-A))
    // dist = fabsf(dot(P-A,n))
    // if (dist == 0) always, hull is degenerate
// for each face:
    // compute normal
    // find tetrahedron vert that is not on the face (ABC would be D)
    // d = dot(D-A, n)
    // if d > 0, the normal must be flipped, swap two vertices

// now we have a tetrahedron, compute center
    // center = (A+B+C+D) * 0.25f

// for each point:
    // determine visibility:
        // for each face:
            // d = (dot(P-A, face normal))
            // test fabsf(d) < eps
            // if (d > 0)
            // add to visible faces
            // if (d < 0)
            // not visible
            // if (d == 0)
            // point lies on plane

    // find horizon edges:
    // for each visible face:
        // count edges that are shared between visible faces
        // if count == 1, this is a horizon edge
    // delete visible faces from master list
    // connect P to each horizon edge and emit new faces to master list
    // for each new face, compute normal and verify it faces correctly:
        // if (dot(center - face_vertex, n) > 0) flip normal (swap two vertices)





int pick_lowest_point(ConvexHull * hull)
{
    AXIS longest;
    if (hull->aabb.extents.x >= hull->aabb.extents.y && hull->aabb.extents.x >= hull->aabb.extents.z) longest = X;
    if (hull->aabb.extents.y >= hull->aabb.extents.x && hull->aabb.extents.y >= hull->aabb.extents.z) longest = Y;
    if (hull->aabb.extents.z >= hull->aabb.extents.x && hull->aabb.extents.z >= hull->aabb.extents.y) longest = Z;

    int index = 0;
    float lowest = INFINITY;
    vec3 * points = CAST_VEC(hull->vertices, vec3);
    switch(longest)
    {
        case X:
            {
                for (int i = 0; i < hull->vertices.count; i++)
                {
                    if (points[i].x < lowest) {
                        lowest = points[i].x;
                        index = i;
                    }
                }
            }
            break;
        case Y:
                for (int i = 0; i < hull->vertices.count; i++)
                {
                    if (points[i].y < lowest) {
                        lowest = points[i].y;
                        index = i;
                    }
                }
            break;
        case Z:
                for (int i = 0; i < hull->vertices.count; i++)
                {
                    if (points[i].z < lowest) {
                        lowest = points[i].z;
                        index = i;
                    }
                }
            break;
    }
    
    return index;

}



bool setup_initial_hull_points(ConvexHull * hull)
{
    
    int lowest_index = pick_lowest_point(hull);
    vec3 * points = CAST_VEC(hull->vertices, vec3);
    printf("POINTS: %p %zu\n", points, hull->vertices.count);
    // ConvexHull * hull = &mesh->hull;
    vec3 A = points[lowest_index];
    vec3 B; vec3 C; vec3 D;


    float eps = 0.001f;
    float best_dist = 0;
    vec3 best;
    size_t a_idx = lowest_index;
    size_t b_idx = 0;
    size_t c_idx = 0;
    size_t d_idx = 0;
    for (int i = 0; i < hull->vertices.count; i++)
    {
        float dist = fabsf(vec3_dist(A, points[i]));
        if (dist > best_dist)
        {
            best_dist = dist;
            best = points[i];
            b_idx = i;
        }
    }
    if (best_dist <= eps)
    {
        printf("B\n");
        return false;
    }
    else
    {
        
        B = best;
    }


    vec3 AB = vec3_sub(B, A);
    float ab_len = vec3_length(AB);
    best_dist = 0;
    
    for (int i = 0; i < hull->vertices.count; i++)
    {
        vec3 P = points[i];
        vec3 AP = vec3_sub(P, A);
        float dist = vec3_length(vec3_cross(AP, AB)) / ab_len;
        if (dist > best_dist)
        {
            best_dist = dist;
            best = P;
            c_idx = i;
        }

    }
    if (best_dist <= eps)
    {
        return false;
    }
    else
    {
        
        C = best;
    }

    if (fabsf(vec3_length(vec3_cross(vec3_sub(B, A), vec3_sub(C, A)))) <= 0.001f)
    {
        return false;
    }

    
    vec3 normal = vec3_normalize(vec3_cross(vec3_sub(B, A), vec3_sub(C, A)));
    best_dist = 0;

    for (int i = 0; i < hull->vertices.count; i++)
    {
        vec3 P = points[i];
        vec3 AP = vec3_sub(P, A);
        float dist = fabsf(vec3_dot(AP, normal));
        if (dist > best_dist)
        {
            best_dist = dist;
            best = P;
            d_idx = i;
        }

    }
    
    if (best_dist <= eps)
    {
        return false;
    }
    else
    {
        
        D = best;
    }


    printf("A: %f %f %f B: %f %f %f C: %f %f %f D: %f %f %f\n", A.x, A.y, A.z, B.x, B.y, B.z, C.x, C.y, C.z, D.x, D.y, D.z);
    init_vector(&hull->faces, 0, sizeof(MeshFace));

    vector_expand(&hull->faces, 4);
    hull->faces.count = 4;
    MeshFace * faces = CAST_VEC(hull->faces, MeshFace);

    // vec3 AB = vec3_sub(B, A);
    vec3 AC = vec3_sub(C, A);
    vec3 AD = vec3_sub(D, A);
    vec3 BC = vec3_sub(C, B);
    vec3 BD = vec3_sub(D, B);
    vec3 CD = vec3_sub(D, C);
    vec3 DA = vec3_sub(A, D);
    vec3 DC = vec3_sub(C, D);

    faces[0].indices[0] = a_idx;
    faces[0].indices[1] = b_idx;
    faces[0].indices[2] = c_idx;

    vec3 center = vec3_add(vec3_add(vec3_add(A, B), C), D);
    center = vec3_scale(center, 0.25f);
    hull->centroid = center;
    printf("CENTER: %f %f %f\n", center.x, center.y, center.z);
    printf("A: %f %f %f B: %f %f %f C: %f %f %f D: %f %f %f\n", A.x, A.y, A.z, B.x, B.y, B.z, C.x, C.y, C.z, D.x, D.y, D.z);
    faces[0].normal = compute_tri_normal(A, B, C);
    if (vec3_dot(vec3_sub(D, A), faces[0].normal) > 0)
    {
        faces[0].indices[0] = b_idx;
        faces[0].indices[1] = a_idx;
        faces[0].normal = compute_tri_normal(B, A, C);
    }
    faces[1].indices[0] = b_idx;
    faces[1].indices[1] = d_idx;
    faces[1].indices[2] = c_idx;
    faces[1].normal = compute_tri_normal(B, D, C);
    if (vec3_dot(vec3_sub(A, B), faces[1].normal) > 0)
    {
        faces[1].indices[0] = d_idx;
        faces[1].indices[1] = b_idx;
        faces[1].normal = compute_tri_normal(D, B, C);
        if (vec3_dot(vec3_sub(A, B), faces[1].normal) > 0)
        {
            printf("RECOMPUTING\n");
        }
    }
    faces[2].indices[0] = d_idx;
    faces[2].indices[1] = a_idx;
    faces[2].indices[2] = c_idx;
    faces[2].normal = compute_tri_normal(D, A, C);
    if (vec3_dot(vec3_sub(B, D), faces[2].normal) > 0)
    {
        faces[2].indices[0] = a_idx;
        faces[2].indices[1] = d_idx;
        faces[2].normal = compute_tri_normal(A, D, C);
        if (vec3_dot(vec3_sub(B, D), faces[2].normal) > 0)
        {
            printf("RECOMPUTING\n");
        }
    }
    faces[3].indices[0] = a_idx;
    faces[3].indices[1] = b_idx;
    faces[3].indices[2] = d_idx;
    faces[3].normal = compute_tri_normal(A, B, D);
    if (vec3_dot(vec3_sub(C, A), faces[3].normal) > 0)
    {
        faces[3].indices[0] = b_idx;
        faces[3].indices[1] = a_idx;
        faces[3].normal = compute_tri_normal(B, A, D);
        if (vec3_dot(vec3_sub(C, A), faces[3].normal) > 0)
        {
            printf("RECOMPUTING\n");
        }
    }
    return true;
    
}



void incremental_hull(ConvexHull * hull)
{

    // Simplex s;
    // decompose(mesh);
    
    if (!setup_initial_hull_points(hull)) return;

    printf("SETUP\n");
    
    vec3 * points = CAST_VEC(hull->vertices, vec3);
    int dup[4];
    int dc = 0;
    MeshFace * hf = CAST_VEC(hull->faces, MeshFace);
    for (int i = 0; i < hull->vertices.count; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            for (int p = 0; p < 3; p++)
            {
                
                if (vec3_equal(points[i], points[hf[j].indices[p]], 0.01f))
                {
                    bool skip = false;
                    for (int l = 0; l < dc; l++)
                    {
                        if (vec3_equal(points[i], points[dup[l]], 0.01f))
                        {
                            skip = true;
                        }
                    }
                    if (!skip)
                        dup[dc++] = i;
                }
            }
        }
    }

    Vector visible_faces_indices;
    Vector horizon_edges;
    Vector horizon_counts;
    Vector hull_indices;
    
    init_vector(&horizon_edges, 0, sizeof(HorizonEdge));
    init_vector(&horizon_counts, 0, sizeof(int));
    init_vector(&visible_faces_indices, 0, sizeof(int));
    init_vector(&hull_indices, 0, sizeof(int));
    vector_expand(&hull_indices, 4);
    int * indices = CAST_VEC(hull_indices, int);
    indices[0] = 0;
    indices[1] = 1;
    indices[2] = 2;
    indices[3] = 3;
    hull_indices.count = 4;
    
    
    for (int i = 0; i < hull->vertices.count; i++)
    {
        bool di = false;
        vec3 p = points[i];
        for (int j = 0; j < 4; j++)
        {
            if (dup[j] == i)
            {
                
                di = true;
            }
        }
        if (di) continue;
        MeshFace * faces = CAST_VEC(hull->faces, MeshFace);
        int * indices = CAST_VEC(hull_indices, int);
        for (int f = 0; f < hull_indices.count; f++)
        {

            MeshFace * face = &faces[indices[f]];
            float d = vec3_dot(vec3_sub(p, points[face->indices[0]]), face->normal);
            // if ((d) > 0.01f)
            if ((d) > 0.0f)
            {
                int * vfi = vector_expand(&visible_faces_indices, 1);
                *vfi = f;
            }
            
        }
        // check_face_visibility(p, mesh->hull.vertices, mesh->hull.faces, hull_indices, &visible_faces_indices);

        if (visible_faces_indices.count <= 0) continue;
        int * visible_indices = CAST_VEC(visible_faces_indices, int);
        for (int f = 0; f < visible_faces_indices.count; f++)
        {
            MeshFace * face = &faces[indices[visible_indices[f]]];
            check_horizon_edges(face, hull->vertices, &horizon_edges, &horizon_counts);
        }

        // delete visible faces from master list
        int * counts = CAST_VEC(horizon_counts, int);
                
        for (int f = 0; f < visible_faces_indices.count; f++)
        {
            // remove_int_from_vec(&hull_indices, visible_indices[f] - f);
            REMOVE_FROM_VEC(&hull_indices, visible_indices[f] - f, int);
        }

        HorizonEdge * edges = CAST_VEC(horizon_edges, HorizonEdge);
        for (int h = 0; h < horizon_edges.count; h++)
        {
            HorizonEdge * edge = &edges[h];
            if (counts[h] == 1)
            {
                // connect p to horizon edges
                add_horizon_face_to_hull_faces(i, *edge, hull->vertices, hull->centroid, &hull->faces, &hull_indices, false);
    
            }
        }
        
        visible_faces_indices.count = 0;
        horizon_edges.count = 0;
        horizon_counts.count = 0;
        // expand_hull(p, i, &mesh->hull.vertices, &mesh->hull.faces, &hull_indices, &horizon_edges, &horizon_counts, &visible_faces_indices, mesh->hull.centroid);

    }
    free(visible_faces_indices.buffer);
    free(horizon_edges.buffer);
    free(horizon_counts.buffer);

    int * hi = CAST_VEC(hull_indices, int);

    MeshFace * new_hulls = malloc(hull_indices.count * sizeof(MeshFace));
    
    MeshFace * faces = CAST_VEC(hull->faces, MeshFace);
    Vector unique_vertices;
    init_vector(&unique_vertices, 0, sizeof(vec3));

    for (int i = 0; i < hull_indices.count; i++)
    {
        
        new_hulls[i] = faces[hi[i]];

        for (int p = 0; p < 3; p++)
        {
            
            bool is_unique = true;
            vec3 * uv = CAST_VEC(unique_vertices, vec3);
            for (int v = 0; v < unique_vertices.count; v++)
            {
                

                if (vec3_equal(points[new_hulls[i].indices[p]], uv[v], 0.001f))
                {
                    is_unique = false;
                    new_hulls[i].indices[p] = v;
                    break;
                }
            }
            if (is_unique)
            {
                vec3 * v = vector_expand(&unique_vertices, 1);
                *v = points[new_hulls[i].indices[p]];
                new_hulls[i].indices[p] = unique_vertices.count-1;
            }
        }

    }
    MeshFace * old_hulls = hull->faces.buffer;
    hull->faces.buffer = new_hulls;
    free(old_hulls);
    hull->faces.count = hull_indices.count;
    hull->faces.capacity = hull_indices.capacity;

    free(hull_indices.buffer);
    free(hull->vertices.buffer);

    hull->vertices = unique_vertices;
    clean_up_hull(&hull->vertices, &hull->faces);
    compute_aabb_from_points(hull->vertices.buffer, hull->vertices.count, &hull->aabb);

}


// Combines contiguous triangles with same normals into one face. Important for more efficient physics / manifold clipping.

void compute_mesh_hulls(ConvexHull * hull)
{
    
    Vector groups;
    init_vector(&groups, 0, sizeof(TriangleGroup));
    init_vector(&hull->hull_faces, 0, sizeof(HullFace));

    MeshFace * tris = CAST_VEC(hull->faces, MeshFace);
    vec3 * pts = CAST_VEC(hull->vertices, vec3);
    

    for (int i = 0; i < hull->faces.count; i++){
        
        MeshFace * tri = &tris[i];

        if (triangle_area(pts[tri->indices[A]], pts[tri->indices[B]], pts[tri->indices[C]]) < 0.001f) continue;

        bool added = false;

        TriangleGroup * tri_groups = CAST_VEC(groups, TriangleGroup);

        for (int g = 0; g < groups.count; g++) {

            TriangleGroup * group = &tri_groups[g];
            
            if (is_similar_plane(tri->normal, group->normal, pts[tri->indices[A]], group->ref_point, 0.001f)) {
                bool contiguous_found = false;
                size_t * indices = CAST_VEC(group->indices, size_t);
                for (int j = 0; j < group->indices.count; j++)
                {
                    
                    MeshFace * tri2 = &tris[indices[j]];
                    if (mesh_faces_are_contiguous(tri, pts, tri2, pts)) contiguous_found = true;
                }
                if (contiguous_found)
                {
                    size_t * index = vector_expand(&group->indices, 1);
                    *index = i;
                    added = true;
                    break;
                }


            }
            
        }

        if (!added) {

            TriangleGroup * group = vector_expand(&groups, 1);

            init_vector(&group->indices, 0, sizeof(size_t));
            group->normal = tri->normal;

            group->ref_point = pts[tri->indices[A]];

            
            size_t * index = vector_expand(&group->indices, 1);
            *index = i;

        }
    }

    TriangleGroup * tri_groups = CAST_VEC(groups, TriangleGroup);
    for (int g = 0; g < groups.count; g++) {

        TriangleGroup * group = &tri_groups[g];
        size_t * points = malloc(group->indices.count * 16 * sizeof(size_t));
        int point_count = 0;
        float bottom = INFINITY, top = -INFINITY;
        size_t * indices = CAST_VEC(group->indices, size_t);


        for (int j = 0; j < group->indices.count; j++) {

            MeshFace * tri = &tris[indices[j]];

            points[point_count++] = tri->indices[A];
            points[point_count++] = tri->indices[B];
            points[point_count++] = tri->indices[C];

            
            // printf("TRI: %zu\n", indices[j]);

            if (pts[tri->indices[A]].z > top) top = pts[tri->indices[A]].z;
            if (pts[tri->indices[B]].z > top) top = pts[tri->indices[B]].z;
            if (pts[tri->indices[C]].z > top) top = pts[tri->indices[C]].z;
            if (pts[tri->indices[A]].z < bottom) bottom = pts[tri->indices[A]].z;
            if (pts[tri->indices[B]].z < bottom) bottom = pts[tri->indices[B]].z;
            if (pts[tri->indices[C]].z < bottom) bottom = pts[tri->indices[C]].z;
        }

        if (point_count > 0) {
            // if (all_colinear(points, point_count)) {
            //     fprintf(stderr, "Warning: all points colinear, skipping hull.\n");
            //     free(points);
            //     continue;
            // }

            size_t * unique_points = malloc(point_count * sizeof(size_t));
            int unique_count = 0;
            for (int i = 0; i < point_count; i++) {
                bool exists = false;
                for (int j = 0; j < unique_count; j++) {
                    if (vec3_equal(pts[points[i]], pts[unique_points[j]], 0.001f))
                    {
                    
                        exists = true;
                        break;
                    }
                }
                if (!exists) {
                    unique_points[unique_count++] = points[i];
                }
            }

            if (unique_count < 3) {
                fprintf(stderr, "Warning: not enough unique points to form a hull.\n");


                free(unique_points);
                free(points);
                continue;
            }
        
            HullFace * hf = vector_expand(&hull->hull_faces, 1);
            hf->count = 0;
            hf->normal = group->normal;
            compute_hull_faces(pts, unique_points, unique_count, hf->indices, &hf->count, group->normal);

            free(unique_points);
        }
        free(points);
    }

    // printf("GROUPS: %zu\n", groups.count);
    TriangleGroup * gs = (TriangleGroup*)groups.buffer;
    if (groups.buffer)
    {
    
        for (int i = 0; i < groups.count; i++)
        {
            // TriangleGroup * group = &(CAST_VEC(groups, TriangleGroup))[i];
            if (gs[i].indices.buffer) free(gs[i].indices.buffer);
            // free(group->indices.buffer);
        }
        free(groups.buffer);
    }

}

