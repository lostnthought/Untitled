
#include "core/camera.h"
#include "core/engine_types.h"
#include "core/mesh.h"
#include "render/render.h"
#include "stb/stb_image.h"
#include "stb/stb_image_write.h"
#include "load/load.h"
#include "utilities/mathf.h"
#include "utilities/engine_utilities.h"
#include "utilities/types.h"
#include "core/acceleration.h"
#include "load/init.h"
#include "render/render.h"
#include "audio/audio.h"
#include "utilities/utilities.h"

#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <omp.h>
#include <assert.h>
#include <winscard.h>





#ifdef _WIN32
    #include <windows.h>
    #define SLEEP(x) Sleep(x) 
#else
    #include <unistd.h>
    #define SLEEP(x) sleep(x / 1000)  
#endif

void walk_coplanar_adjacency(Mesh * mesh, UVIsland * island, UVMapping * uv_map, int island_idx, int current_face_index, bool * tri_list, TriangleAdjacency * tri_adj, vec3 normal, vec3 ref_point, float eps)
{

    
    MeshFace * fs = CAST_VEC(mesh->tris, MeshFace);
    Point * pts = CAST_VEC(mesh->vertices, Point);
    MeshFace * current_face = &fs[current_face_index];
    if (tri_list[current_face_index]) return;

    float angle_threshold = 0.7f;
    vec3 A = pts[fs->indices[0]].pos;
    vec3 B = pts[fs->indices[1]].pos;
    vec3 C = pts[fs->indices[2]].pos;

    float area = triangle_area(A, B, C);

    
    if (vec3_dot(current_face->normal, normal) < angle_threshold /* && area >= 0.005f */)
    {
        return;
    }
    uv_map[current_face_index].uv_island = island_idx;
    size_t * idx = vector_expand(&island->tris, 1);
    *idx = current_face_index;
    tri_list[current_face_index] = true;

    size_t * adj = CAST_VEC(tri_adj[current_face_index].adjacent_indices, size_t);

    for (int i = 0; i < tri_adj[current_face_index].adjacent_indices.count; i++)
    {

        walk_coplanar_adjacency(mesh, island, uv_map, island_idx, adj[i], tri_list, tri_adj, normal, ref_point, eps);
        
    }

    
}



LightmapAtlas init_lightmap_atlas(int width, int height, int block_size) {
    LightmapAtlas atlas = {0};
    atlas.width = width;
    atlas.height = height;
    atlas.block_size = block_size;
    int blocks_x = width / block_size;
    int blocks_y = height / block_size;
    atlas.occupied = calloc(blocks_x * blocks_y, sizeof(unsigned char));
    return atlas;
}
void free_lightmap_atlas(LightmapAtlas * atlas){
    if (atlas->occupied != NULL) free(atlas->occupied);
    atlas->width = 0;
    atlas->height = 0;
    atlas->block_size = 0;
}
void init_lightmap_data(LightmapData * ld)
{

    free_lightmap_atlas(&ld->atlas);
    ld->atlas = init_lightmap_atlas(LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, 2);

    if (ld->islands.buffer)
    {
        
        UVIsland * uvi = CAST_VEC(ld->islands, UVIsland);
        for (int i = 0; i < ld->islands.count; i++)
        {
            if (uvi[i].uvs) free(uvi[i].uvs);
            if (uvi[i].tris.buffer) free(uvi[i].tris.buffer);
        }

        free(ld->islands.buffer);

    }

    init_vector(&ld->islands, 0, sizeof(UVIsland));


    ld->sample_cubemap = true;
    
}





void find_least_parallel_axis(vec3 normal, vec3 * u_dir, vec3 * v_dir)
{
    vec3 n = vec3_normalize(normal);

    float dx = fabsf(vec3_dot(n, (vec3){1,0,0}));
    float dy = fabsf(vec3_dot(n, (vec3){0,1,0}));
    float dz = fabsf(vec3_dot(n, (vec3){0,0,1}));

    vec3 helper;

    if(dx < dy && dx < dz)
        helper = (vec3){1,0,0};
    else if(dy < dz)
        helper = (vec3){0,1,0};
    else
        helper = (vec3){0,0,1};
    vec3 up = (vec3){0,0,1};
    if (fabsf(n.z) >= 0.999f) up = (vec3){1,0,0};

    *u_dir = vec3_normalize(vec3_cross(up, n));
    *v_dir = vec3_cross(n, *u_dir);   
}

LightmapRect allocate_lightmap_rect(LightmapAtlas *atlas, int w_px, int h_px) {

    int padding = 2;


    int total_w = w_px + 2 * padding;
    int total_h = h_px + 2 * padding;    
    int blocks_w = (total_w + atlas->block_size + 1) / atlas->block_size;
    int blocks_h = (total_h + atlas->block_size + 1) / atlas->block_size;
    int blocks_x = atlas->width / atlas->block_size;
    int blocks_y = atlas->height / atlas->block_size;

    for (int y = 0; y <= blocks_y - blocks_h; y++) {
        for (int x = 0; x <= blocks_x - blocks_w; x++) {
            bool fits = true;
            for (int dy = 0; dy < blocks_h && fits; dy++) {
                for (int dx = 0; dx < blocks_w; dx++) {
                    int idx = (y + dy) * blocks_x + (x + dx);
                    if (atlas->occupied[idx]) {
                        fits = false;
                        break;
                    }
                }
            }

            if (fits) {
                for (int dy = 0; dy < blocks_h; dy++) {
                    for (int dx = 0; dx < blocks_w; dx++) {
                        int idx = (y + dy) * blocks_x + (x + dx);
                        atlas->occupied[idx] = 1;
                    }
                }

                return (LightmapRect){
                    .u = x * atlas->block_size + padding,
                    .v = y * atlas->block_size + padding,
                    .w = w_px + 2 * padding,
                    .h = h_px + 2 * padding,
                    .success = 1,
                    .padding = padding
                };
            }
        }
    }

    return (LightmapRect){ .success = 0 };
}

void deallocate_lightmap_rect(LightmapData * ld, LightmapRect * rect)
{

    int x = (rect->u - rect->padding) / ld->atlas.block_size;
    int y = (rect->v - rect->padding) / ld->atlas.block_size;
    
    int blocks_w = (rect->w + ld->atlas.block_size + 1) / ld->atlas.block_size;
    int blocks_h = (rect->h + ld->atlas.block_size + 1) / ld->atlas.block_size;
    
    int blocks_x = ld->atlas.width / ld->atlas.block_size;
    int blocks_y = ld->atlas.height / ld->atlas.block_size;

    for (int dy = 0; dy < blocks_h; dy++) {
        for (int dx = 0; dx < blocks_w; dx++) {
            int idx = (y + dy) * blocks_x + (x + dx);
            ld->atlas.occupied[idx] = 0;
        }
    }

    memset(rect, 0, sizeof(LightmapRect));
    
}


typedef struct UVAssignmentPair
{

    float pu, pv;
    size_t idx;
    vec3 p; // manually check for equivalency so that we avoid bad unwrapping on very close duplicate points

    
} UVAssignmentPair;


void assign_uvs(UVIsland * uvi, LightmapAtlas *atlas) {
    

    const float eps = 0.001f;

    float min_u = INFINITY, max_u = -INFINITY;
    float min_v = INFINITY, max_v = -INFINITY;
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    MeshNode * mn = &mns[uvi->mesh_node];
    if (mn->mesh == -1) return;
    Mesh * m = &meshes[mn->mesh];
    Point * pts = CAST_VEC(m->vertices, Point);
    MeshFace * mfs = CAST_VEC(m->tris, MeshFace);
    size_t * tris = CAST_VEC(uvi->tris, size_t);

    vec3 u_dir;
    vec3 v_dir;
    find_least_parallel_axis(uvi->normal, &u_dir, &v_dir);


    UVAssignmentPair * uv_pairs = malloc(sizeof(UVAssignmentPair) * uvi->tris.count * 3);
    int uv_pair_count = 0;
    
    size_t * uv_pair_indices = malloc(sizeof(size_t) * uvi->tris.count * 3);
    int uv_pair_indices_count = 0;
    
    vec3 pA = uvi->centroid;

    float tri_area = 0;

    bool one_valid = false;
    
    for (int i = 0; i < uvi->tris.count; i++)
    {

        MeshFace * mf = &mfs[tris[i]];
        float area = triangle_area(pts[mf->indices[0]].pos,pts[mf->indices[1]].pos,pts[mf->indices[2]].pos);
        tri_area += area;
    }
        
    for (int i = 0; i < uvi->tris.count; i++)
    {

        MeshFace * mf = &mfs[tris[i]];

        for (int j = 0; j < 3; j++) {

            vec3 p = vec3_sub(pts[mf->indices[j]].pos, pA);
            float pu = vec3_dot(p, u_dir);
            float pv = vec3_dot(p, v_dir);
            if (pu < min_u) min_u = pu;
            if (pu > max_u) max_u = pu;
            if (pv < min_v) min_v = pv;
            if (pv > max_v) max_v = pv;
            bool match_found = false;
            if (!match_found)
            {
                uv_pair_indices[uv_pair_indices_count++] = uv_pair_count;
                uv_pairs[uv_pair_count++] = (UVAssignmentPair)
                {
                    .pu = pu,
                    .pv = pv,
                    .idx = mf->indices[j],
                    .p = pts[mf->indices[j]].pos
                };
            }

        }
    }

    float range_u = max_u - min_u;
    float range_v = max_v - min_v;

    float mfactor = sqrtf(tri_area) * EDITOR_SETTINGS.mfactor;

    int texel_width  = MAX(ceilf(range_u * EDITOR_SETTINGS.texels_per_uv_unit * (EDITOR_SETTINGS.mfactor_enabled ? mfactor : 1)), 6);
    int texel_height = MAX(ceilf(range_v * EDITOR_SETTINGS.texels_per_uv_unit * (EDITOR_SETTINGS.mfactor_enabled ? mfactor : 1)), 6);
    LightmapRect rect = allocate_lightmap_rect(atlas, texel_width, texel_height);
    if (rect.success == 0) printf("OUT OF SPACE\n");

    uvi->rect = rect;
    int pad = rect.padding;
    float inner_w = rect.w - 2 * pad;
    float inner_h = rect.h - 2 * pad;
    float inner_u = rect.u;
    float inner_v = rect.v;

    int index = 0;

    for (int i = 0; i < uvi->tris.count; i++)
    {

        MeshFace * mf = &mfs[tris[i]];
        mn->uv_map[tris[i]].uv_idx = i;

        
        
        for (int j = 0; j < 3; j++) {

            float pu = uv_pairs[uv_pair_indices[index]].pu;
            float pv = uv_pairs[uv_pair_indices[index]].pv;

            float norm_u = (pu - min_u) / (range_u > 1e-6f ? range_u : 1.0f);
            float norm_v = (pv - min_v) / (range_v > 1e-6f ? range_v : 1.0f);
            float pixel_u = inner_u + norm_u * (inner_w > 0 ? (inner_w - 1) : 0.0f);
            float pixel_v = inner_v + norm_v * (inner_h > 0 ? (inner_h - 1) : 0.0f);

            uvi->uvs[i][j].x = (pixel_u + 0.5f) / (float)atlas->width;
            uvi->uvs[i][j].y = (pixel_v + 0.5f) / (float)atlas->height;
            index++;
        }
    }


    free(uv_pair_indices);
    free(uv_pairs);

}




void add_uv_island_to_lightmap(LightmapData * ld, UVIsland * uvi)
{
    
    float area = 0.0f;
    size_t * tris = CAST_VEC(uvi->tris, size_t);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    MeshNode * mn = &mns[uvi->mesh_node];
    if (mn->mesh == -1) return;
    Mesh * m = &meshes[mn->mesh];
    Point * pts = CAST_VEC(m->vertices, Point);
    MeshFace * mfs = CAST_VEC(m->tris, MeshFace);

    if (uvi->tris.count > 0)
    {
        
        uvi->uvs = malloc(uvi->tris.count * sizeof(LightmapUV));
    }
    else
    {
        uvi->uvs = NULL;
    }
    uvi->currently_rasterizing = false;

    assign_uvs(uvi, &ld->atlas);

    
}



void init_uv_islands(LightmapData * ld)
{

    UVIsland * islands = CAST_VEC(ld->islands, UVIsland);

    for (int i = 0; i < ld->islands.count; i++)
    {
        
        UVIsland * island = &islands[i];
        add_uv_island_to_lightmap(ld, island);

    }

}


void build_uv_islands(MeshNode * mn)
{

    const float eps = 1.0f;
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    if (mn->mesh == -1) return;
    Mesh * mesh = &meshes[mn->mesh];
    MeshFace * fs = CAST_VEC(mesh->tris, MeshFace);
    Point * pts = CAST_VEC(mesh->vertices, Point);
    TriangleAdjacency * tri_adj = mesh->tri_adjacency;

    LightmapData * ld = &GLOBALS.scene_data.lightmap_data;
    bool * tri_list = calloc(mesh->tris.count, sizeof(bool));

    if (mn->uv_map) free(mn->uv_map);
    mn->uv_map = calloc(mesh->tris.count, sizeof(UVMapping));

    
    Vector invalid_islands;
    init_vector(&invalid_islands, 0, sizeof(size_t));
    size_t starting_island = ld->islands.count;
    
    for (int i = 0; i < mesh->tris.count; i++)
    {
        MeshFace * f = &fs[i];
        if (tri_list[i]) continue;

        UVIsland * island = vector_expand(&ld->islands, 1);
        init_vector(&island->tris, 0, sizeof(size_t));
        island->mesh_node = mn->id;
        int island_idx = ld->islands.count-1;
        mn->uv_map[i].uv_island = island_idx;
        island->uvs = NULL;
        size_t * tris = vector_expand(&island->tris, 1);
        *tris = i;
        tri_list[i] = true;

        vec3 normal = f->normal;
        island->normal = normal;

        size_t * adj = CAST_VEC(tri_adj[i].adjacent_indices, size_t);


        for (int j = 0; j < tri_adj[i].adjacent_indices.count; j++)
        {

            walk_coplanar_adjacency(mesh, island, mn->uv_map, island_idx, adj[j], tri_list, tri_adj, normal, pts[f->indices[0]].pos, eps);
            
        }
        tris = CAST_VEC(island->tris, size_t);
        vec3 new_normal = {0,0,0};
        bool no_valid = true;
        vec3 centroids = (vec3){0,0,0};
        for (int j = 0; j < island->tris.count; j++)
        {
            new_normal = vec3_add(new_normal, fs[tris[j]].normal);
            vec3 A = pts[fs[tris[j]].indices[0]].pos;
            vec3 B = pts[fs[tris[j]].indices[1]].pos;
            vec3 C = pts[fs[tris[j]].indices[2]].pos;

            float area = triangle_area(A, B, C);

            centroids = vec3_add(centroids, triangle_centroid(A, B, C));
            
        }

        centroids = vec3_scale(centroids, 1.0f / island->tris.count);

        island->centroid = centroids;

        new_normal = vec3_normalize(new_normal);
        island->normal = new_normal;
        
    }
    UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
    for (int i = starting_island; i < ld->islands.count; i++)
    {
        add_uv_island_to_lightmap(ld, &islands[i]);
    }


    free(tri_list);
    
}


void LM_regenerate_texels()
{
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    init_lightmap_data(&GLOBALS.scene_data.lightmap_data);
    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        if (mns[i].mesh == -1 || mns[i].free) continue;
        if (!meshes[mns[i].mesh].static_mesh || !mns[i].static_mesh || mns[i].physics_mode) continue;

        build_uv_islands(&mns[i]);    
    }
    VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
    add_all_meshes_to_buffer(vl);
}

void LM_allocate_empty_mns()
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);

    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {

        if (mns[i].mesh == -1 || mns[i].free) continue;
        if (!meshes[mns[i].mesh].static_mesh || !mns[i].static_mesh || mns[i].physics_mode) continue;
        if (mns[i].uv_map) continue;

        build_uv_islands(&mns[i]);    

    }

    VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
    add_all_meshes_to_buffer(vl);

}


void save_lightmap_to_binary(LightmapData * ld, const char * name)
{

    FILE * f = fopen(name, "wb");

    if (!f) return;

    UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);


    
    fwrite(&EDITOR_SETTINGS.texels_per_uv_unit, sizeof(float), 1, f);

    
    fwrite(&ld->atlas.width, sizeof(int), 1, f);
    fwrite(&ld->atlas.height, sizeof(int), 1, f);
    fwrite(&ld->atlas.block_size, sizeof(int), 1, f);
    if (ld->atlas.occupied)
    {
        bool t = true;
        fwrite(&t, sizeof(bool), 1, f);
        fwrite(ld->atlas.occupied, sizeof(unsigned char), (ld->atlas.width / ld->atlas.block_size) * (ld->atlas.height / ld->atlas.block_size), f);
    }
    else
    {
        bool fl = false;

        fwrite(&fl, sizeof(bool), 1, f);
    }
    fwrite(&ld->sample_cubemap, sizeof(bool), 1, f);


    fwrite(&ld->islands.count, sizeof(size_t), 1, f);
    /**
     * @critical This is a bug. There is no need to write all Mesh UVMapping%s per UVIsland and it will explode data. 
     */
    for (int i = 0; i < ld->islands.count; i++)
    {

        UVIsland * uvi = &islands[i];

        MeshNode * mn = &mns[uvi->mesh_node];
        fwrite(&mn->name, sizeof(char) * 256, 1, f);
        fwrite(&uvi->tris.count, sizeof(size_t), 1, f);
        if (uvi->tris.buffer) fwrite(uvi->tris.buffer, sizeof(size_t), uvi->tris.count, f);
        if (uvi->uvs) fwrite(uvi->uvs, sizeof(LightmapUV), uvi->tris.count, f);
        fwrite(&uvi->normal, sizeof(vec3), 1, f);
        fwrite(&uvi->rect, sizeof(LightmapRect), 1, f);

        Mesh * m = &meshes[mn->mesh];
        
        fwrite(&m->tris.count, sizeof(size_t), 1, f);
        fwrite(mn->uv_map, sizeof(UVMapping), m->tris.count, f);



    }
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    vec4 * pixels = malloc(sizeof(vec4) * ld->atlas.height * ld->atlas.width);
    glBindTexture(GL_TEXTURE_2D, ld->texture_id);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, pixels);

    fwrite(pixels, sizeof(vec4), ld->atlas.width * ld->atlas.height, f);


    free(pixels);

    vec4 * dir_pixels = malloc(sizeof(vec4) * ld->atlas.width * ld->atlas.height);

    glBindTexture(GL_TEXTURE_2D, DIRECTIONAL_MAP_TEXTURE);
    glGetTexImage(GL_TEXTURE_2D, 0, GL_RGBA, GL_FLOAT, dir_pixels);

    fwrite(dir_pixels, sizeof(vec4), ld->atlas.width * ld->atlas.height, f);

    free(dir_pixels);

    fclose(f);

}


void load_lightmap_from_binary(LightmapData * ld, const char * name)
{

    FILE * f = fopen(name, "rb");

    if (!f) return;

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

    if (ld->texture_id != -1)
    {
        glDeleteTextures(1, &ld->texture_id);
    }

    float texels_in = 1.5f;
    fread(&texels_in, sizeof(float), 1, f);
    if (texels_in != EDITOR_SETTINGS.texels_per_uv_unit)
    {
        EDITOR_SETTINGS.texels_per_uv_unit = texels_in;
    }


    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        MeshNode * mn = &mns[i];

        if (mn->uv_map)
        {
            free(mn->uv_map);
        }
        mn->uv_map = NULL;
        
    }

    
    fread(&ld->atlas.width, sizeof(int), 1, f);
    fread(&ld->atlas.height, sizeof(int), 1, f);
    fread(&ld->atlas.block_size, sizeof(int), 1, f);
    bool t = false;
    fread(&t, sizeof(bool), 1, f);
    if(ld->atlas.occupied) free(ld->atlas.occupied);
    ld->atlas.occupied = NULL;
    if (t)
    {
        ld->atlas.occupied = calloc((ld->atlas.width / ld->atlas.block_size) * (ld->atlas.height / ld->atlas.block_size), sizeof(unsigned char));
        fread(ld->atlas.occupied, sizeof(unsigned char), (ld->atlas.width / ld->atlas.block_size) * (ld->atlas.height / ld->atlas.block_size), f);
    }
    else
    {
        ld->atlas.occupied = NULL;
    }
    fread(&ld->sample_cubemap, sizeof(bool), 1, f);


    /**
     * @critical This does not free the current UVIsland%s and may be a memory leak. 
     */
    fread(&ld->islands.count, sizeof(size_t), 1, f);
    if (ld->islands.count> 0)
    {
        

        ld->islands.buffer = malloc(sizeof(UVIsland) * ld->islands.count);
        ld->islands.capacity = ld->islands.count;

    }
    else
    {
        
        init_vector(&ld->islands, 0, sizeof(UVIsland));
    }
    UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
    for (int i = 0; i < ld->islands.count; i++)
    {

        UVIsland * uvi = &islands[i];
        char name[256];
        fread(name, sizeof(char) * 256, 1, f);

        int mn_idx = find_mn_by_name(name);
        uvi->mesh_node = mn_idx;
            
        fread(&uvi->tris.count, sizeof(size_t), 1, f);
        if (uvi->tris.count > 0)
        {
            uvi->tris.buffer = malloc(sizeof(size_t) * uvi->tris.count);
            uvi->tris.capacity = uvi->tris.count;
            fread(uvi->tris.buffer, sizeof(size_t), uvi->tris.count, f);
            uvi->uvs = malloc(sizeof(LightmapUV) * uvi->tris.count);
            fread(uvi->uvs, sizeof(LightmapUV), uvi->tris.count, f);
        
        }
        else
        {
            init_vector(&uvi->tris, 0, sizeof(size_t));
            uvi->uvs = NULL;
        }
        fread(&uvi->normal, sizeof(vec3), 1, f);
        fread(&uvi->rect, sizeof(LightmapRect), 1, f);
        size_t tri_count = 0;
        fread(&tri_count, sizeof(size_t), 1, f);
        
        if (mn_idx == -1)
        {

            fseek(f, sizeof(UVMapping) * tri_count, SEEK_CUR);
            if (uvi->tris.buffer) free(uvi->tris.buffer);
            init_vector(&uvi->tris, 0, sizeof(size_t));
            deallocate_lightmap_rect(ld, &uvi->rect);
           
        }
        else
        {
            MeshNode * mn = &mns[mn_idx];
            Mesh * m = &meshes[mn->mesh];

            mn->uv_map = malloc(sizeof(UVMapping) * m->tris.count);
            fread(mn->uv_map, sizeof(UVMapping), m->tris.count, f);
            
        }


    }
    
    
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    vec4 * pixels = malloc(sizeof(vec4) * ld->atlas.height * ld->atlas.width);
    glGenTextures(1, &ld->texture_id);
    glBindTexture(GL_TEXTURE_2D, ld->texture_id);

    fread(pixels, sizeof(vec4), ld->atlas.width * ld->atlas.height, f);

    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);


    free(pixels);

    

    vec4 * dir_pixels = malloc(sizeof(vec4) * ld->atlas.width * ld->atlas.height);


    if (DIRECTIONAL_MAP_TEXTURE != -1)
    {
        glDeleteTextures(1, &DIRECTIONAL_MAP_TEXTURE);
    }
    glGenTextures(1, &DIRECTIONAL_MAP_TEXTURE);

    glBindTexture(GL_TEXTURE_2D, DIRECTIONAL_MAP_TEXTURE);

    fread(dir_pixels, sizeof(vec4), ld->atlas.width * ld->atlas.height, f);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, dir_pixels);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(dir_pixels);

    fclose(f);
    VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
    add_all_meshes_to_buffer(vl);

}



void bake_reflection_probes()
{


    glBindFramebuffer(GL_FRAMEBUFFER, RPROBE_FBO);

    float yaws[6] = {0, M_PI, M_PI, M_PI, M_PI / 2.0f + M_PI, M_PI / 2.0f};
    float pitches[6] = {0, 0, M_PI / 2.0f, -M_PI / 2.0f, 0, 0};
    vec3 flips[6];
    flips[0] = (vec3){0, 0, -1};
    flips[1] = (vec3){0, 0, -1};
    flips[2] = (vec3){0, -1, 0};
    flips[3] = (vec3){0, 1, 0};
    flips[4] = (vec3){0, 0, -1};
    flips[5] = (vec3){0, 0, -1};


    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    ReflectionProbe * rprobes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);
    bool has_aa = EDITOR_SETTINGS.MSAA;
    EDITOR_SETTINGS.MSAA = false;
    for (int i = 0; i < GLOBALS.scene_data.reflection_probes.count; i++)
    {

        ReflectionProbe * probe = &rprobes[i];


        mat4 proj = make_projection(90, (float)RPROBE_WIDTH / RPROBE_HEIGHT, 0.01, 100.0f);
        for (int j = 0; j < 6; j++)
        {


            glBindFramebuffer(GL_FRAMEBUFFER, RPROBE_FBO);
            glFramebufferTextureLayer(
                GL_FRAMEBUFFER,
                GL_COLOR_ATTACHMENT0,
                RPROBE_CUBE_MAP_ARRAY,
                0,
                6 * i + j
            );
            glViewport(0, 0, RPROBE_WIDTH, RPROBE_HEIGHT);
            glClearColor(0, 0, 0, 1.0f);
            glClearDepth(1.0);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            vec3 pos = probe->capture_pos;
            
            Camera sky_cam;
            sky_cam.cam_data.cam_pos = vec3_to_vec4(pos);

            sky_cam.yaw = yaws[j];
            sky_cam.pitch = pitches[j];

            compute_camera_basis(&sky_cam, sky_cam.yaw, sky_cam.pitch, 0, flips[j]);

            vec3 target = vec3_add(
                pos,
                sky_cam.forward
            );

            mat4 view = make_look_at(
                &sky_cam,
                pos
            );
            sky_cam.cam_data.view = view;
            sky_cam.cam_data.proj = proj;

            m4_inverse((float*)&sky_cam.cam_data.inv_proj, (float*)&sky_cam.cam_data.proj);
            m4_inverse((float*)&sky_cam.cam_data.inv_view, (float*)&sky_cam.cam_data.view);
            sky_cam.cam_data.unjittered_proj = proj;
            sky_cam.cam_data.unjittered_inv_proj = sky_cam.cam_data.inv_proj;
            
            render_to_cubemap_face(&sky_cam, RPROBE_FBO, RPROBE_WIDTH, RPROBE_HEIGHT, true);


            glFinish();
            current_frame = (current_frame + 1) % BUFFER_FRAMES;
            
        }

        
    }


    glFinish();
    EDITOR_SETTINGS.MSAA = has_aa;
    // glBindVertexArray(CONVOLVE_TRIANGLE_VAO);
    // glBindBuffer(GL_ARRAY_BUFFER, CONVOLVE_TRIANGLE_VBO);

    

    // glUseProgram(GGX_RPROBE_SHADER.program);
    // glUniform1f(glGetUniformLocation(GGX_RPROBE_SHADER.program, "uResolution"), RPROBE_WIDTH);


    // glBindFramebuffer(GL_FRAMEBUFFER, 0);

    RProbe * probes = calloc(GLOBALS.scene_data.reflection_probes.count, sizeof(RProbe));

    for (int i = 0; i < GLOBALS.scene_data.reflection_probes.count; i++)
    {

        ReflectionProbe * probe = &rprobes[i];


        probes[i].center = probe->aabb.center;
        probes[i].min = (vec3){probe->aabb.min_x, probe->aabb.min_y, probe->aabb.min_z};
        probes[i].max = (vec3){probe->aabb.max_x, probe->aabb.max_y, probe->aabb.max_z};

    }
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, RPROBE_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(RProbe) * GLOBALS.scene_data.reflection_probes.count, probes, GL_STATIC_DRAW);

    free(probes);
    
    
}





#define MAX_DIRECTIONAL_LIGHTS_LM 4
#define MAX_POINT_LIGHTS_LM 2048
#define MAX_EMISSIVE_TRIS_LM 100000

typedef struct LMDebug
{
    vec3 pos;
    uint32_t pad0;
    vec3 dir;
    uint32_t pad1;
    vec3 origin;
    uint32_t pad2;
} LMDebug;
#define LMDEBUG_SIZE 4096
typedef struct LightGPUData
{

    int dir_count;
    int point_count;
    int tri_count;
    uint32_t sample_count;
    float dir_w;
    float point_w;
    float tri_w;
    float total_power;
    LightGPU directional_lights[MAX_DIRECTIONAL_LIGHTS_LM];
    LightGPU point_lights[MAX_POINT_LIGHTS_LM];
    LMDebug LMDEBUG[LMDEBUG_SIZE];
    
} LightGPUData;


void upload_bake_lights_gpu(GLuint * lights_ssbo)
{
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);

    float dir_power = 0.0f;
    float point_power = 0.0f;

    if (GLOBALS.scene_data.light_bvh.lights.count <= 0) return;
    LightGPU * lgpu = malloc(sizeof(LightGPU) * GLOBALS.scene_data.light_bvh.lights.count);
    Light * ls = CAST_VEC(GLOBALS.scene_data.light_bvh.lights, Light);
    BVHNode * light_nodes = CAST_VEC(GLOBALS.scene_data.light_bvh.nodes, BVHNode);
    BVHNodeGPU * snodes = calloc(GLOBALS.scene_data.light_bvh.nodes.count, sizeof(BVHNodeGPU));
    for (int i = 0; i < GLOBALS.scene_data.light_bvh.indices_count; i++)
    {

        Light * l = &ls[GLOBALS.scene_data.light_bvh.indices[i]];

        copy_light_to_gpu_light(&lgpu[i], l);
        lgpu[i].bvh_index = GLOBALS.scene_data.light_bvh.indices[i];
         
    }


    size_t light_offset = current_frame * light_ssbo_size;

    uint32_t * light_data = (uint32_t*)(light_ssbo_ptr + light_offset);
    memcpy(light_data, lgpu, sizeof(LightGPU) * GLOBALS.scene_data.light_bvh.lights.count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, LIGHT_SSBO, light_offset, light_ssbo_size);
    

    glFinish();


    for (int i = 0; i < GLOBALS.scene_data.light_bvh.nodes.count; i++)
    {

        BVHNode * node = &light_nodes[i];
        BVHNodeGPU * n = &snodes[i];
        n->parent = node->parent;
        n->left = node->left;
        n->right = node->right;
        n->next = node->next;

        n->first_index = node->first_index;
        n->count = node->count;
        n->center = node->aabb.center;
        n->extents = node->aabb.extents;
        n->min = (vec3){node->aabb.min_x, node->aabb.min_y, node->aabb.min_z};
        n->max = (vec3){node->aabb.max_x, node->aabb.max_y, node->aabb.max_z};
        n->flux = node->flux;
        n->emissive_flux = node->emissive_flux;
        
    }

    glGenBuffers(1, lights_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, *lights_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNodeGPU) * GLOBALS.scene_data.light_bvh.nodes.count, snodes, 0);

    

    free(lgpu);
    free(snodes);
}

void bake_lightmap(LightmapData * ld)
{
    
    if (GLOBALS.scene_data.light_bvh.nodes.buffer == NULL) return;
    

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    UVIsland * uvis = CAST_VEC(ld->islands, UVIsland);
    
    GLuint lights_ssbo;
    upload_bake_lights_gpu(&lights_ssbo);
    
    bind_baking_ssbos();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, lights_ssbo); 

    glBindFramebuffer(GL_FRAMEBUFFER, LIGHTMAP_FBO);
    glViewport(0, 0, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepth(1.0);
    glDisable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(LIGHTMAP_SHADER.program);
    glBindVertexArray(LIGHTMAP_VAO);
    glDisable(GL_CULL_FACE);


    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE);
    int * buf = malloc(LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT * sizeof(int) * 3);
    for (int i = 0; i < LIGHTMAP_WIDTH * LIGHTMAP_HEIGHT * 3; i++)
    {
        buf[i] = -1;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32I, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, 0, GL_RGB_INTEGER, GL_INT, buf);

    free(buf);
    for (int i = 0; i < GLOBALS.scene_data.bvh_indices_count; i++)
    {
        size_t mn_idx = GLOBALS.scene_data.bvh_indices[i];
        if (!meshes[mns[mn_idx].mesh].static_mesh) continue;
        MeshNode * mn = &mns[mn_idx];
        if (mn->mesh == -1 || mn->free || !mn->uv_map) continue;
        Mesh * m = &meshes[mn->mesh];

        glUniform1i(glGetUniformLocation(LIGHTMAP_SHADER.program, "uMeshNode"), i);
        LightmapVertex * lmv = NULL;
        lmv = calloc(m->tris.count * 3, sizeof(LightmapVertex));
        if (!lmv) continue;
        
        Point * pts = CAST_VEC(m->vertices, Point);
        MeshFace * mfs = CAST_VEC(m->tris, MeshFace);

        // NOTE: we can use submeshes and iterate them to pass material as a uniform rather than per vertex. not worth the complexity right now due to their indexing
        int count = 0;
        for (int j = 0; j < m->tris.count; j++)
        {

            uint32_t idx = m->bvh_indices[j];
            UVIsland * uvi = &uvis[mn->uv_map[idx].uv_island];
            if (!uvi->currently_rasterizing) continue;
            MeshFace * mf = &mfs[idx];

            for (int p = 0; p < 3; p++)
            {
                
                lmv[count].pos = pts[mf->indices[p]].pos;
                lmv[count].normal = pts[mf->indices[p]].normal;
                lmv[count].uv = pts[mf->indices[p]].uv;
                lmv[count].lm_uv = uvi->uvs[mn->uv_map[idx].uv_idx][p];
                lmv[count].material_id = mf->material_id;
                lmv[count].tri_id = j;
                count++;

            }
        }

        glBindBuffer(GL_ARRAY_BUFFER, LIGHTMAP_VBO);
        glBufferData(GL_ARRAY_BUFFER, sizeof(LightmapVertex) * count, lmv, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, count);


        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawArrays(GL_TRIANGLES, 0, count);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);        

        free(lmv);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    GLuint ping;
    glGenTextures(1, &ping);
    glBindTexture(GL_TEXTURE_2D, ping);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint pong;
    glGenTextures(1, &pong);
    glBindTexture(GL_TEXTURE_2D, pong);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glClearTexImage(GL_TEXTURE_2D, 0, GL_RGBA32F, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    
    GLuint directional_out;
    glGenTextures(1, &directional_out);
    glBindTexture(GL_TEXTURE_2D, directional_out);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glClearTexImage(GL_TEXTURE_2D, 0, GL_RGBA32F, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint directional_ping;
    glGenTextures(1, &directional_ping);
    glBindTexture(GL_TEXTURE_2D, directional_ping);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    GLuint directional_pong;
    glGenTextures(1, &directional_pong);
    glBindTexture(GL_TEXTURE_2D, directional_pong);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, ld->atlas.width, ld->atlas.height, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    glUseProgram(LIGHTMAP_BOUNCE_SHADER);
    int ITERATIONS = EDITOR_SETTINGS.lm_iterations;
    const int TILE_SIZE = 2048;
    // const int TILE_SIZE = 512;
    Timer timer;
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_POS_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightmapPosTex"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_NORM_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightmapNormalTex"), 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightmapIDTex"), 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SKYBOX);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uSkybox"), 4);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightmapWidth"), ld->atlas.width);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightmapHeight"), ld->atlas.height);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uLightCount"), GLOBALS.lights.count);
    glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uAOIterations"), EDITOR_SETTINGS.ao_iterations);
    glUniform1f(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uAOThreshold"), EDITOR_SETTINGS.ao_threshold);



    // this is done in chunks to avoid tdr crashes on big scenes
    for (int i = 0; i < ITERATIONS; i++)
    {

        if (i % 10 == 0)
        {
            start_timer(&timer);
        }

        glBindImageTexture(0, pong, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);
        glBindImageTexture(1, directional_out, 0, GL_FALSE, 0, GL_READ_WRITE, GL_RGBA32F);



        for (int y = 0; y < ld->atlas.height; y += TILE_SIZE) {
            for (int x = 0; x < ld->atlas.width; x += TILE_SIZE) { 


                glUniform2i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uTileOffset"), x, y);
                glUniform1i(glGetUniformLocation(LIGHTMAP_BOUNCE_SHADER, "uIteration"), i);

                int dispatchX = MIN(TILE_SIZE, ld->atlas.width - x);
                int dispatchY = MIN(TILE_SIZE, ld->atlas.height - y);

                glDispatchCompute((dispatchX + 15) / 16, (dispatchY + 15) / 16, 1);                
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
                
            }
        }

        glFinish();
        double iter_time = end_timer(&timer);
    }


    glUseProgram(DIRECTIONAL_MAP_NORMALIZE_SHADER);
    glBindImageTexture(0, pong, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, directional_out, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(3, directional_ping, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);
    
    glUniform1i(glGetUniformLocation(DIRECTIONAL_MAP_NORMALIZE_SHADER, "uLightmapWidth"), ld->atlas.width);
    glUniform1i(glGetUniformLocation(DIRECTIONAL_MAP_NORMALIZE_SHADER, "uLightmapHeight"), ld->atlas.height);
    glUniform1i(glGetUniformLocation(DIRECTIONAL_MAP_NORMALIZE_SHADER, "uSampleCount"), ITERATIONS);
    glDispatchCompute((ld->atlas.width+15) / 16, (ld->atlas.height+15) / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);

    glUseProgram(LIGHTMAP_DENOISE_SHADER);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_POS_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DENOISE_SHADER, "uLightmapPosTex"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_NORM_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DENOISE_SHADER, "uLightmapNormalTex"), 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DENOISE_SHADER, "uLightmapIDTex"), 2);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DENOISE_SHADER, "uLightmapWidth"), ld->atlas.width);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DENOISE_SHADER, "uLightmapHeight"), ld->atlas.height);
    
    glBindImageTexture(0, ping, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    glBindImageTexture(1, pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    glBindImageTexture(2, directional_ping, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
    glBindImageTexture(3, directional_pong, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    
    glDispatchCompute((ld->atlas.width+15) / 16, (ld->atlas.height+15) / 16, 1);
    glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);



    glUseProgram(LIGHTMAP_DILATE_SHADER);
    
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DILATE_SHADER, "uLightmapIDTex"), 2);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DILATE_SHADER, "uLightmapWidth"), ld->atlas.width);
    glUniform1i(glGetUniformLocation(LIGHTMAP_DILATE_SHADER, "uLightmapHeight"), ld->atlas.height);

    
    for (int i = 0; i < 8; i++)
    {
        glBindImageTexture(0, pong, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
        glBindImageTexture(1, ping, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
        glBindImageTexture(2, directional_pong, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA16F);
        glBindImageTexture(3, directional_ping, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA16F);

    
        glDispatchCompute((ld->atlas.width+15) / 16, (ld->atlas.height+15) / 16, 1);
        glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT);
        
        GLuint temp;
        temp = pong;
        pong = ping;
        ping = temp;

        GLuint directional_temp;
        directional_temp = directional_pong;
        directional_pong = directional_ping;
        directional_ping = directional_temp;
    }


    glBindTexture(GL_TEXTURE_2D, directional_pong);
    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, pong);
    glGenerateMipmap(GL_TEXTURE_2D);
    if (ld->texture_id != 0) {
        glDeleteTextures(1, &ld->texture_id);
        ld->texture_id = 0;
    }
    if (DIRECTIONAL_MAP_TEXTURE != 0) {
        glDeleteTextures(1, &DIRECTIONAL_MAP_TEXTURE);
        DIRECTIONAL_MAP_TEXTURE = 0;
    }
    ld->texture_id = pong;
    DIRECTIONAL_MAP_TEXTURE = directional_pong;
    glDeleteTextures(1, &ping);
    glDeleteTextures(1, &directional_ping);
    glDeleteTextures(1, &directional_out);
    
    
    for (int i = 0; i < ld->islands.count; i++)
    {
        UVIsland * uvi = &uvis[i];
        uvi->currently_rasterizing = false;
        
    }

    glDeleteBuffers(1, &lights_ssbo);
    
    unbind_baking_ssbos();
    
    
}


void remove_lighting_pv(LPV * lpv, bool * lpb)
{
    if (lpv->lprobes)
    {
        free(lpv->lprobes);
        lpv->lprobes = NULL;
    }
    lpv->pv_id = 0;
    *lpb = false;
}

void reset_lighting_pvs()
{
    for (int i = 0; i < LPV_SLOTS; i++)
    {
        if (GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes)
        {
            free(GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes);
        }
        GLOBALS.scene_data.LOADED_LIGHTING_PV[i].lprobes = NULL;
        GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[i] = false;
    }
}


void load_lighting_pv(LPV * lpv, ProbeVolume * pv, int replacing_id)
{
    
    fseek(GLOBALS.scene_data.LPV_FILE, pv->byte_offset, SEEK_SET);

    lpv->lprobes = malloc(sizeof(float) * LPV_PADDED_COUNT * 3 * SH_MAX);

    fread(lpv->lprobes, sizeof(float), LPV_PADDED_COUNT * 3 * SH_MAX, GLOBALS.scene_data.LPV_FILE);
       
    glBindTexture(GL_TEXTURE_3D, LPV_TEXTURE);
    glTexSubImage3D(
                        GL_TEXTURE_3D,
                        0,
                        0, 0, replacing_id * SH_MAX * PROBE_VOLUME_DEPTH_PADDED,
                        PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH_PADDED * SH_MAX,
                        GL_RGB, GL_FLOAT, lpv->lprobes
                    );

}

typedef struct DistancePair
{
    float d;
    int id;
} DistancePair;

int compare_distances(const void * a, const void * b)
{
    DistancePair * da = (DistancePair *)a;
    DistancePair * db = (DistancePair *)b;
    if (da->d < db->d) return 1;
    if (da->d > db->d) return -1;
    return 0;
    
}
int compare_current_distances(const void * a, const void * b)
{
    DistancePair * da = (DistancePair *)a;
    DistancePair * db = (DistancePair *)b;
    if (da->d > db->d) return 1;
    if (da->d < db->d) return -1;
    return 0;
    
}



void check_lighting_pv()
{
    
    if (GLOBALS.scene_data.LPV_FILE == NULL) return;


    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);
    vec3 current_pos = player.skeleton.neck.pos;

    DistancePair distances[LPV_SLOTS];
    DistancePair current_distances[LPV_SLOTS];
    for (int i = 0; i < LPV_SLOTS; i++)
    {
        distances[i].d = INFINITY;
        distances[i].id = -1;
        current_distances[i].d = INFINITY;
        current_distances[i].id = i;
    }

    for (int i = 0; i < GLOBALS.scene_data.lighting_probe_volumes.count; i++)
    {

        bool already_loaded = false;
        for (int j = 0; j < LPV_SLOTS; j++)
        {
            if (GLOBALS.scene_data.LOADED_LIGHTING_PV[j].pv_id == i && GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[j] == true)
            {

                already_loaded = true;
                break;
            }
        }

        if (already_loaded) continue;
        ProbeVolume * pv = &pvs[i];
        vec3 diff = vec3_sub(pv->obb.center, current_pos);
        vec3 dir = vec3_normalize(diff);
        vec3 intersection_point = current_pos;
        bool hit = ray_intersects_obb_with_point(pv->obb, current_pos, dir, &intersection_point);
        float dist = vec3_dist(current_pos, intersection_point);
        if (!hit)
        {
            printf("how the hell did the lighting pv ray not hit it is uhm like directed at the volume dude?\n");
            continue;
        }

        for (int j = 0; j < LPV_SLOTS; j++)
        {
            if (distances[j].d > dist)
            {
                distances[j].d = dist;
                distances[j].id = i;
                break;
            }
        }
    }
    int count = 0;
    for (int i = 0; i < LPV_SLOTS; i++)
    {
        if (distances[i].id != -1)
        {
            count++;
        }
    }

    qsort(distances, count, sizeof(DistancePair), compare_distances);
    
    int current_count = 0;
    for (int i = 0; i < LPV_SLOTS; i++)
    {

        if (!GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[i]) continue;
        
        ProbeVolume * pv = &pvs[GLOBALS.scene_data.LOADED_LIGHTING_PV[i].pv_id];
        vec3 diff = vec3_sub(pv->obb.center, current_pos);
        vec3 dir = vec3_normalize(diff);
        vec3 intersection_point = current_pos;
        bool hit = ray_intersects_obb_with_point(pv->obb, current_pos, dir, &intersection_point);
        float dist = vec3_dist(current_pos, intersection_point);
        if (!hit)
        {
            printf("how the hell did the lighting pv ray not hit it is uhm like directed at the volume dude?\n");
            continue;
        }
        
        current_distances[current_count++].d = dist;
        current_distances[current_count++].id = i;
    }    
    
    qsort(current_distances, LPV_SLOTS, sizeof(DistancePair), compare_current_distances);
    
    
    int replaced_offset = 0;
    for (int i = 0; i < count; i++)
    {
        for (int j = replaced_offset; j < LPV_SLOTS; j++)
        {
            if (distances[i].d < current_distances[j].d)
            {

                int replacing_id = current_distances[j].id;
                remove_lighting_pv(&GLOBALS.scene_data.LOADED_LIGHTING_PV[replacing_id], &GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[replacing_id]);

                load_lighting_pv(&GLOBALS.scene_data.LOADED_LIGHTING_PV[replacing_id], &pvs[distances[i].id], replacing_id);
                GLOBALS.scene_data.LOADED_LIGHTING_PV[replacing_id].pv_id = distances[i].id;
                GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[replacing_id] = true;
                
                replaced_offset = j + 1;
                break;

            }
        }
    }
    
}



void bake_lighting_probe_volume(ProbeVolume * pv)
{

    if (GLOBALS.scene_data.light_bvh.nodes.buffer == NULL) return;
    if (GLOBALS.scene_data.LPV_FILE == NULL) return;
    if (EDITOR_SETTINGS.probe_iterations == 0) return;
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    
    GLuint lights_ssbo;
    GLuint probe_out_ssbo;
    upload_bake_lights_gpu(&lights_ssbo);
    
    bind_baking_ssbos();
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, lights_ssbo); 

    glDeleteBuffers(1, &PROBE_SSBO);
    glGenBuffers(1, &PROBE_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, PROBE_SSBO);

    SHDataGPU * sh_data = calloc(PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH, sizeof(SHDataGPU));
    OBB obb = pv->obb;
    
    vec3 step_x, step_y, step_z;
    PV_step_size(pv, &step_x, &step_y, &step_z);

    vec3 pos = OBB_min_corner(&obb);

    vec3 current_pos = pos;
    for (int z = 0; z < PROBE_VOLUME_DEPTH; z++)
    {
        for (int y = 0; y < PROBE_VOLUME_HEIGHT; y++)
        {
            for (int x = 0; x < PROBE_VOLUME_WIDTH; x++)
            {
    
                current_pos.x = pos.x + x * step_x.x + y * step_y.x + z * step_z.x;
                current_pos.y = pos.y + x * step_x.y + y * step_y.y + z * step_z.y;
                current_pos.z = pos.z + x * step_x.z + y * step_y.z + z * step_z.z;
                sh_data[z * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_WIDTH + y * PROBE_VOLUME_WIDTH + x].pos = current_pos;
            }
        }
    }

    glBufferStorage(GL_SHADER_STORAGE_BUFFER, PROBE_VOLUME_WIDTH * PROBE_VOLUME_HEIGHT * PROBE_VOLUME_DEPTH * sizeof(SHDataGPU), sh_data, GL_MAP_WRITE_BIT | GL_MAP_READ_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, PROBE_SSBO); 
    glGenBuffers(1, &probe_out_ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, probe_out_ssbo);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, LPV_PADDED_COUNT * sizeof(float) * 3 * SH_MAX, NULL, GL_MAP_READ_BIT);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, probe_out_ssbo); 
    

    int PROBE_ITERATIONS = EDITOR_SETTINGS.probe_iterations;
    glUseProgram(PROBE_BAKE_SHADER);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SKYBOX);
    glUniform1i(glGetUniformLocation(PROBE_BAKE_SHADER, "uSkybox"), 4);
    glUniform1i(glGetUniformLocation(PROBE_BAKE_SHADER, "uLightCount"), GLOBALS.lights.count);
    glUniform3i(glGetUniformLocation(PROBE_BAKE_SHADER, "uProbeSize"), PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH);
    glUniform1f(glGetUniformLocation(PROBE_BAKE_SHADER, "uSpacing"), GLOBALS.probe_spacing);
    glUniform1i(glGetUniformLocation(PROBE_BAKE_SHADER, "uIterations"), PROBE_ITERATIONS);

    for (int i = 0; i < PROBE_ITERATIONS; i++)
    {
    
        glUniform1i(glGetUniformLocation(PROBE_BAKE_SHADER, "uCurrentIteration"), i);
        glDispatchCompute((PROBE_VOLUME_WIDTH + 7) / 8, (PROBE_VOLUME_HEIGHT+ 7) / 8, (PROBE_VOLUME_DEPTH + 7) / 8);                
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
        glFinish();
    
    }

    glFinish();

    glUseProgram(PROBE_UNPACK_SHADER);
    glUniform3i(glGetUniformLocation(PROBE_UNPACK_SHADER, "uProbeSize"), PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH);
    glUniform1i(glGetUniformLocation(PROBE_UNPACK_SHADER, "uIterations"), PROBE_ITERATIONS);
    glUniform1ui(glGetUniformLocation(PROBE_UNPACK_SHADER, "uPaddedDepth"), PROBE_VOLUME_DEPTH_PADDED);

    glDispatchCompute((PROBE_VOLUME_WIDTH + 7) / 8, (PROBE_VOLUME_HEIGHT+ 7) / 8, (PROBE_VOLUME_DEPTH + 7) / 8);                
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    glFinish();

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, probe_out_ssbo);
    float * out = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_READ_ONLY);
    if (!out)
    {
        printf("CANNOT READ OUT\n");
    }
    else
    {
        
        size_t byte_offset = 0;
        if (pv->size > 0)
        {
            byte_offset = pv->byte_offset;
            fseek(GLOBALS.scene_data.LPV_FILE, byte_offset, SEEK_SET);
        }
        else
        {
            fseek(GLOBALS.scene_data.LPV_FILE, 0, SEEK_END);
            byte_offset = ftell(GLOBALS.scene_data.LPV_FILE);
        }

        fwrite(out, sizeof(float), LPV_PADDED_COUNT * 3 * SH_MAX, GLOBALS.scene_data.LPV_FILE);


        pv->size = ftell(GLOBALS.scene_data.LPV_FILE) - byte_offset;
        pv->byte_offset = byte_offset;

    }

    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


    glDeleteBuffers(1, &lights_ssbo);    
    glDeleteBuffers(1, &probe_out_ssbo);    

    free(sh_data);
    
    unbind_baking_ssbos();

    
    
}

