#include <GL/glew.h>
#include <assert.h>
#include "SDL2/SDL_timer.h"
#include "core/camera.h"
#include "utilities/mathf.h"
#include "load/init.h"
#include "utilities/types.h"
#include <float.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "ui/ui.h"
#include "utilities/utilities.h"
#include "render/render.h"
#include "utilities/engine_utilities.h"
#include "core/engine_types.h"
#include "core/animation.h"
#include "ui/fonts.h"


int master_frame = 0;
int cur_history = 0;


int current_frame = 0;
size_t meshes_in_frame[MAX_MESH_NODES];
size_t meshes_this_frame;
size_t shadow_meshes_in_frame[MAX_MESH_NODES];
size_t shadow_meshes_this_frame;

void bind_baking_ssbos()
{
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, VERTEX_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, TRI_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, MESH_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, MESH_BVH_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, SCENE_BVH_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, PROBE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, RPROBE_SSBO); 
}
void unbind_baking_ssbos()
{
    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, GLOBAL_INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, TEXT_BOX_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, RPROBE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, JOINT_SSBO); 

}


void bind_render_ssbos()
{


    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, MESH_LIGHT_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, MATERIAL_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, LIGHTMAP_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, GLOBAL_INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 7, INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 8, TEXT_BOX_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 9, UI_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 10, RPROBE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 11, JOINT_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 12, CLUSTER_AABB_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 13, GLOBAL_LIGHT_INDICES_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 14, CLUSTER_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 15, MESH_NODE_SSBO); 
    
}



size_t fill_buffer_with_mesh_tri(Mesh * mesh, MainVertex * verts, size_t offset)
{
    MeshFace * tris = (MeshFace *)mesh->tris.buffer;
    Point * pts = (Point*)mesh->vertices.buffer;
    size_t count = 0;
    if (mesh->static_mesh)
    {
        
        LightmapData * ld = &GLOBALS.scene_data.lightmap_data;
        UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        int current_offset = offset;
        for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
        {
            
            if (mns[i].mesh != mesh->id || mns[i].free) continue;
            MeshNode * mn = &mns[i];
            int current_count = 0;
            mn->offset = current_offset;
            UVMapping * uv_map = mn->uv_map;
            for (int j = 0; j < mesh->tris.count; j++)
            {
                MeshFace * tri = &tris[j];
                TriangleAdjacency * tri_adj = mesh->tri_adjacency;
                UVIsland * uvi = &islands[uv_map[j].uv_island];
                LightmapUV * uv = &uvi->uvs[uv_map[j].uv_idx];

                for (int p = 0; p < 3; p++)
                {
            
                    verts[current_offset + j * 3 + p].pos = pts[tri->indices[p]].pos;
                    verts[current_offset + j * 3 + p].normal = pts[tri->indices[p]].normal;
                    verts[current_offset + j * 3 + p].joint_indices= pts[tri->indices[p]].joints;
                    verts[current_offset + j * 3 + p].weights = pts[tri->indices[p]].weights;
                    verts[current_offset + j * 3 + p].tangent = pts[tri->indices[p]].tangent;
                    verts[current_offset + j * 3 + p].uv = pts[tri->indices[p]].uv;
                    if (islands)
                    {
                
                        LIGHTMAP_DATA.uvs[current_offset + j * 3 + p].uv = (vec4){uv->uvs[p].x, uv->uvs[p].y, 0, 0};
                    }

                    count++;

                    current_count++;
                }
            }
            mn->count = current_count;
            current_offset += current_count;
        }
    }
    else
    {
        for (int i = 0; i < mesh->tris.count; i++)
        {
            MeshFace * tri = &tris[i];
            TriangleAdjacency * tri_adj = mesh->tri_adjacency;
            for (int p = 0; p < 3; p++)
            {
            
                verts[offset + i * 3 + p].pos = pts[tri->indices[p]].pos;
                verts[offset + i * 3 + p].normal = pts[tri->indices[p]].normal;
                verts[offset + i * 3 + p].joint_indices = pts[tri->indices[p]].joints;
                verts[offset + i * 3 + p].weights = pts[tri->indices[p]].weights;
                verts[offset + i * 3 + p].tangent = pts[tri->indices[p]].tangent;
                verts[offset + i * 3 + p].uv = pts[tri->indices[p]].uv;

                count++;
            }
        }
        
    }
    
    return count;
}
size_t fill_vertex_buffer_mesh(Mesh * mesh, MainVertex * verts, size_t offset, size_t * lm_offset)
{
    MeshFace * tris = (MeshFace *)mesh->tris.buffer;
    Point * pts = (Point*)mesh->vertices.buffer;

    size_t current_lm_offset = *lm_offset;
    size_t count = 0;
    if (mesh->static_mesh)
    {
        
        LightmapData * ld = &GLOBALS.scene_data.lightmap_data;
        UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        int current_offset = offset;
        current_lm_offset = current_offset;
        SubMesh * sms = CAST_VEC(mesh->submeshes, SubMesh);

        for (int j = 0; j < mesh->submeshes.count; j++)
        {
            SubMesh * sm = &sms[j];
            sm->draw_offset = current_offset;

            int current_count = 0;
            sm->lm_offset = current_offset;

            for (int k = 0; k < sm->index_count / 3; k++)
            {
                size_t idx = k + sm->first_index / 3;
                MeshFace * tri = &tris[idx];
                TriangleAdjacency * tri_adj = mesh->tri_adjacency;

                for (int p = 0; p < 3; p++)
                {

                    int index = tri->indices[p];

                    verts[current_offset].pos = pts[index].pos;
                    verts[current_offset].normal = pts[index].normal;
                    verts[current_offset].tangent = pts[index].tangent;
                    verts[current_offset].joint_indices = pts[index].joints;
                    verts[current_offset].weights = pts[index].weights;
                    verts[current_offset].uv = pts[index].uv;
                    count++;
                    current_count++;
                    current_offset++;
                }
            }
            if (!islands) continue;
            int mesh_count = 0;
            for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
            {
    
                if (mns[i].mesh != mesh->id || mns[i].free || !mns[i].uv_map) continue;

                MeshNode * mn = &mns[i];
                UVMapping * uv_map = mn->uv_map;


                for (int k = 0; k < sm->index_count / 3; k++)
                {

                    size_t idx = k + sm->first_index / 3;
                    MeshFace * tri = &tris[idx];
                    TriangleAdjacency * tri_adj = mesh->tri_adjacency;

                    if (uv_map)
                    {
                
                        UVIsland * uvi = &islands[uv_map[idx].uv_island];
                        LightmapUV * uv = &uvi->uvs[uv_map[idx].uv_idx];

                        for (int p = 0; p < 3; p++)
                        {

                            LIGHTMAP_DATA.uvs[current_lm_offset + idx * 3 + p + mesh_count * sm->index_count].uv = (vec4){uv->uvs[p].x, uv->uvs[p].y, 0, 0};
                        }

                    }
                }
                mesh_count++;
            }
        
        }
    }
    else
    {
        
        for (int j = 0; j < mesh->vertices.count; j++)
        {
            verts[offset + j].pos = pts[j].pos;
            verts[offset + j].normal = pts[j].normal;
            verts[offset + j].tangent = pts[j].tangent;
            verts[offset + j].joint_indices = pts[j].joints;
            verts[offset + j].weights = pts[j].weights;
            verts[offset + j].uv = pts[j].uv;
    
            count++;
        }
    }
            
    return count;
}
size_t fill_index_buffer_mesh(Mesh * mesh, uint32_t * indices, size_t vertex_offset, size_t offset, size_t * lm_uv_offset)
{
    size_t current_lm_offset = *lm_uv_offset;

    MeshFace * tris = (MeshFace *)mesh->tris.buffer;
    Point * pts = (Point*)mesh->vertices.buffer;
    size_t count = 0;
    if (!mesh->static_mesh)
    {
        SubMesh * sms = CAST_VEC(mesh->submeshes, SubMesh);

        for (int j = 0; j < mesh->submeshes.count; j++)
        {
            SubMesh * sm = &sms[j];
            
            sm->draw_offset = offset + count;
            sm->lm_offset = 0;
            for (int k = 0; k < sm->index_count / 3; k++)
            {
                size_t idx = k + sm->first_index / 3;
                MeshFace * tri = &tris[idx];
                TriangleAdjacency * tri_adj = mesh->tri_adjacency;
                for (int p = 0; p < 3; p++)
                {
        
                    indices[offset + count] = tri->indices[p] + vertex_offset;
                    count++;

                }
            }
        }
    }
        

    *lm_uv_offset = current_lm_offset;
    
    return count;
    // }
    
}

size_t calc_mesh_size(Mesh * m)
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    if (m->static_mesh)
    {
        int count = 0;
        
        for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
        {
            MeshNode * mn = &mns[i];
            

            if (mns[i].mesh != m->id || mn->free) continue;
            count++;

        }
        return m->tris.count * sizeof(MainVertex) * 3 * count;
    }
    else
    {
        
        return m->vertices.count * sizeof(MainVertex);
    }
    
}
size_t calc_mesh_index_size(Mesh * m)
{
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    if (m->static_mesh)
    {
        int count = 0;
        return 0;
    }
    else
    {
        return m->tris.count * sizeof(uint32_t) * 3;
    
    }
}

size_t vsize = 0;
size_t visize = 0;

void add_mesh_to_buffer(Mesh * m, VertexLayout * layout)
{

    size_t * mesh_id = vector_expand(&layout->meshes, 1);
    *mesh_id = m->id;

    vsize = calc_mesh_size(m);
    visize = calc_mesh_index_size(m);
    glBindBuffer(GL_ARRAY_BUFFER, layout->vbo);

    layout->current_offset = 0;
    layout->current_index_offset = 0;
    size_t * ms = CAST_VEC(layout->meshes, size_t);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    vsize = 0;
    visize = 0;
    for (int i = 0; i < layout->meshes.count; i++)
    {
        Mesh * m = &meshes[ms[i]];
        vsize += calc_mesh_size(m);
        visize += calc_mesh_index_size(m);
    }
    layout->current_capacity = vsize * 2;
    layout->current_index_capacity = visize * 2;

    glBufferData(GL_ARRAY_BUFFER, layout->current_capacity, NULL, GL_DYNAMIC_DRAW);
    MainVertex * verts = malloc(vsize);
    uint32_t * indices = NULL;
    if (visize)
    {
        indices = malloc(visize);
    }

    size_t offset = 0;
    size_t index_offset = 0;
    size_t lm_offset = 0;

    for (int i = 0; i < layout->meshes.count; i++)
    {
        Mesh * m = &meshes[ms[i]];
        m->offset = index_offset;
        size_t current_offset = fill_vertex_buffer_mesh(m, verts, offset, &lm_offset);
        index_offset += fill_index_buffer_mesh(m, indices, offset, index_offset, &lm_offset);
        m->count = m->tris.count * 3;
        offset += current_offset;
    }


    glBufferSubData(GL_ARRAY_BUFFER, 0, vsize, verts);
    if (indices)
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, layout->ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, visize, indices, GL_DYNAMIC_DRAW);
    }
    layout->current_offset = vsize;
    layout->current_index_offset = visize;

    free(verts);
    if (indices) free(indices);

    
}


size_t allocate_lightmap_ssbo()
{
    
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    size_t index_count = 0;
    for (int i = 0; i < GLOBALS.meshes.count; i++)
    {
        
        Mesh * m = &meshes[i];
        if (m->static_mesh)
        {
            for (int j = 0; j < GLOBALS.mesh_nodes.count; j++)
            {
                
                MeshNode * mn = &mns[j];
                if (mn->mesh != i || mn->free) continue;

                index_count += m->tris.count * 3;
            }
            
        }
        else
        {
            index_count += m->tris.count * 3;
        }
    }
    if (LIGHTMAP_DATA.uvs) free(LIGHTMAP_DATA.uvs);
    LIGHTMAP_DATA.uvs = calloc(index_count, sizeof(LightmapGPU));
    return index_count;


}

void add_all_meshes_to_buffer(VertexLayout * layout)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    size_t index_count = allocate_lightmap_ssbo();
    if (layout->meshes.buffer) free(layout->meshes.buffer);
    init_vector(&layout->meshes, 0, sizeof(size_t));

    size_t lm_offset = 0;
    for (int i = 0; i < GLOBALS.meshes.count; i++)
    {
        Mesh * m = &meshes[i];
        if (m->vertices.count <= 0) continue;
        bool add = false;
        for (int j = 0; j < GLOBALS.mesh_nodes.count; j++)
        {
            
            MeshNode * mn = &mns[j];
            if (mn->mesh != i || mn->free) continue;
            add = true;

        }
        if (add) add_mesh_to_buffer(m, layout);
        
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, LIGHTMAP_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(vec4) * index_count, LIGHTMAP_DATA.uvs, GL_STATIC_DRAW);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, LIGHTMAP_SSBO); 
    
}


void aabb_to_vertices(DebugVertex * out, AABB * aabb, vec3 offset)
{


    vec3 corners[8];
    int count = 0;
    for (int z = -1; z <= 1; z += 2){
        for (int y = -1; y <= 1; y += 2){
            for (int x = -1; x <= 1; x += 2){
                corners[count++] = vec3_add(aabb->center, (vec3){x * aabb->extents.x, y * aabb->extents.y, z * aabb->extents.z});
            }
        }
    }
    out[0].pos = corners[1];
    out[1].pos = corners[0];
    out[2].pos = corners[3];
    out[3].pos = corners[2];
    out[4].pos = corners[5];
    out[5].pos = corners[4];
    out[6].pos = corners[7];
    out[7].pos = corners[6];
    out[8].pos = corners[0];
    out[9].pos = corners[2];
    out[10].pos = corners[1];
    out[11].pos = corners[3];
    out[12].pos = corners[4];
    out[13].pos = corners[6];
    out[14].pos = corners[5];
    out[15].pos = corners[7];
    out[16].pos = corners[1];
    out[17].pos = corners[5];
    out[18].pos = corners[0];
    out[19].pos = corners[4];
    out[20].pos = corners[3];
    out[21].pos = corners[7];
    out[22].pos = corners[2];
    out[23].pos = corners[6];
    

}


void render_mesh_aabbs_from_root(MeshNode * mn)
{

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    if (mn->mesh != -1)
    {
        Mesh * m = &meshes[mn->mesh];
        AABB a = transform_aabb_by_model_matrix(m->aabb, mn->global_matrix);
        aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &a, (vec3){0,0,0});
        for (int j = 0; j < 24; j++)
        {
            DEBUG_LINES[DEBUG_LINE_COUNT+j].color = (vec3){0,1,1};
        }
        DEBUG_LINE_COUNT += 24;
        
    }
    
    
    for (int i = 0; i < mn->child_count; i++)
    {
        
        render_mesh_aabbs_from_root(&mns[mn->children[i]]);
    }
    
}



void obb_to_vertices(DebugVertex * out, OBB * obb)
{


    vec3 axis_x = (vec3){obb->rot.a0, obb->rot.b0, obb->rot.c0};
    vec3 axis_y = (vec3){obb->rot.a1, obb->rot.b1, obb->rot.c1};
    vec3 axis_z = (vec3){obb->rot.a2, obb->rot.b2, obb->rot.c2};

    vec3 corners[8];
    int count = 0;
    for (int z = -1; z <= 1; z += 2){
        for (int y = -1; y <= 1; y += 2){
            for (int x = -1; x <= 1; x += 2){
                corners[count++] = 
                    vec3_add(obb->center, 
                    vec3_add(vec3_scale(axis_x, x * obb->extents.x),
                    vec3_add(vec3_scale(axis_y, y * obb->extents.y),
                    vec3_scale(axis_z, z * obb->extents.z))));
            }
        }
    }
    out[0].pos = corners[1];
    out[1].pos = corners[0];
    out[2].pos = corners[3];
    out[3].pos = corners[2];
    out[4].pos = corners[5];
    out[5].pos = corners[4];
    out[6].pos = corners[7];
    out[7].pos = corners[6];
    out[8].pos = corners[0];
    out[9].pos = corners[2];
    out[10].pos = corners[1];
    out[11].pos = corners[3];
    out[12].pos = corners[4];
    out[13].pos = corners[6];
    out[14].pos = corners[5];
    out[15].pos = corners[7];
    out[16].pos = corners[1];
    out[17].pos = corners[5];
    out[18].pos = corners[0];
    out[19].pos = corners[4];
    out[20].pos = corners[3];
    out[21].pos = corners[7];
    out[22].pos = corners[2];
    out[23].pos = corners[6];
    

}





LightGPU GPU_LIGHTS[MAX_LIGHTS];
int gpu_light_count = 0;

void copy_light_to_gpu_light(LightGPU * lgpu, Light * l)
{
    
    lgpu->dir = l->dir;
    lgpu->pos= l->pos;
    lgpu->entity_id= l->entity_id;
    lgpu->dir= l->dir;
    lgpu->rgb= l->rgb;
    lgpu->intensity= l->intensity;
    lgpu->light_type= l->light_type;


    l->outer_angle = l->cone_angle * 0.5f;
    l->inner_angle = l->outer_angle * (1 - l->penumbra);

    lgpu->inner_angle= l->inner_angle;
    lgpu->outer_angle= l->outer_angle;

    float outer_cos = cosf(l->outer_angle);
    float cos_diff = cosf(l->inner_angle) - outer_cos;
    lgpu->outer_cos = outer_cos;
    lgpu->cos_diff = 1.0f/cos_diff;

    l->radius = sqrtf(l->intensity / l->threshold);
    lgpu->radius = l->radius;
    lgpu->layer = l->layer;
    lgpu->mesh = l->mesh;
    lgpu->tri = l->tri;
    lgpu->area = l->surface_area;

}

void upload_lights()
{
    gpu_light_count = 0;
    
    Light * lights = CAST_VEC(GLOBALS.lights, Light);
    for (int i = 0; i < GLOBALS.lights.count; i++)
    {
        Light * l = &lights[i];
        copy_light_to_gpu_light(&GPU_LIGHTS[i], l);

        gpu_light_count++;
        
    }

    
    size_t light_offset = current_frame * light_ssbo_size;

    uint32_t * light_data = (uint32_t*)(light_ssbo_ptr + light_offset);
    memcpy(light_data, GPU_LIGHTS, sizeof(LightGPU) * gpu_light_count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 0, LIGHT_SSBO, light_offset, light_ssbo_size);
}



void determine_light_visibility_per_mesh()
{


    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Light * lights = CAST_VEC(GLOBALS.lights, Light);

    int offset = 0;


    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        
        MeshNode * mn = &mns[i];
        if (mn->mesh == -1 || mn->free) continue;
        
        Mesh * m = &meshes[mn->mesh];

        float closest[3][MAX_LIGHTS_PER_MESH] = {0.0f};
        int closest_counts[3] = {0};
        int closest_indices[3][MAX_LIGHTS_PER_MESH] = {0};
        int total_count = 0;

        for (int j = 0; j < GLOBALS.lights.count; j++)
        {

            Light * l = &lights[j];
            if (l->light_type == POINT_LIGHT) continue;

            vec3 dist = vec3_sub(l->pos, mn->translation);
            float d2 = vec3_length(dist);
            d2 *= d2;
            LightType type = l->light_type;
            if (type != DIRECTIONAL_LIGHT && l->intensity / d2 < MINIMUM_LIGHT_VISIBILITY_DIST) continue;
            // cap directional light at 1
            if (type == DIRECTIONAL_LIGHT)
            {
                if (closest_counts[DIRECTIONAL_LIGHT] >= MAX_DIRECTIONAL_LIGHTS) continue;
                d2 = -INFINITY;
            }

            if (total_count >= MAX_LIGHTS_PER_MESH)
            {
                float highest_val = -INFINITY;
                int highest_index = -1;
                for (int k = 0; k < closest_counts[type]; k++)
                {
                    if (closest[type][k] > highest_val)
                    {
                        highest_val = closest[type][k];
                        highest_index = k;
                    }
                }
                if (highest_val < d2)
                {
                    closest_indices[type][highest_index] = j;
                    closest[type][highest_index] = d2;
                }

            }
            else
            {
                closest_indices[type][closest_counts[type]] = j;
                closest[type][closest_counts[type]++] = d2;
                total_count++;
            }
          
        }

        mn->point_light_offset = offset;
        mn->point_light_count = closest_counts[POINT_LIGHT];
        mn->directional_light_offset = offset + closest_counts[POINT_LIGHT];
        mn->directional_light_count = closest_counts[DIRECTIONAL_LIGHT];
        mn->spot_light_offset = offset + closest_counts[POINT_LIGHT] + closest_counts[DIRECTIONAL_LIGHT];
        mn->spot_light_count = closest_counts[SPOT_LIGHT];

        int current_offset = 0;
        for (int i = 0; i < 3; i++)
        {
            
            for (int j = 0; j < closest_counts[i]; j++)
            {
                GLOBALS.mesh_light_indices[offset+current_offset] = closest_indices[i][j];
                current_offset++;
            }
        }
        
        offset += total_count;
        
    }

    size_t mesh_light_offset = current_frame * mesh_light_ssbo_size;

    uint32_t * mesh_light_data = (uint32_t*)(mesh_light_ssbo_ptr + mesh_light_offset);
    memcpy(mesh_light_data, GLOBALS.mesh_light_indices, sizeof(uint32_t) * offset);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 1, MESH_LIGHT_SSBO, mesh_light_offset, mesh_light_ssbo_size);


    
}


mat4 JOINT_MATS[MAX_JOINT_MATRICES];
int joint_mat_count = 0;
void upload_joint_matrices()
{
    joint_mat_count = 0;
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    
    JOINT_MATS[joint_mat_count++] = mat4_identity();
    for (int k = 0; k < MAX_ASSET_INSTANCES; k++)
    {
        
        AssetInstance * AI = &ASSET_INSTANCES[k];
        if (AI->free) continue;
        for (int i = 0; i < AI->skin_count; i++)
        {

            GLTFSkin * skin = &skins[AI->skin_indices[i]];
            skin->ssbo_offset = joint_mat_count;
            for (int j = 0; j < skin->joint_count; j++)
            {
                JOINT_MATS[joint_mat_count++] = skin->final_mat[j];
            }
        }
    }

    size_t joint_offset = current_frame * joint_ssbo_size;

    mat4 * joint_data = (mat4*)(joint_ssbo_ptr + joint_offset);
    memcpy(joint_data, JOINT_MATS, sizeof(mat4) * joint_mat_count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 11, JOINT_SSBO, joint_offset, joint_ssbo_size);

    
}


void update_node_global(MeshNode * nodes, int idx, mat4 parent) {

    MeshNode *n = &nodes[idx];
    n->local_matrix = compose_trs_from_mesh_node(n);

    n->global_matrix = m4_mul(parent, n->local_matrix);

    for (int c = 0; c < n->child_count; c++) {

        update_node_global(nodes, n->children[c], n->global_matrix);
        
    }
}    

void update_scene_nodes(){

    mat4 identity = mat4_identity();

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++) {
        AssetInstance * AI = &ASSET_INSTANCES[i];
        if (AI->free) continue;
        for (int n = 0; n < AI->node_count; n++){
            MeshNode * mn = &mns[AI->node_indices[n]];
            if (mn->root) {
                mn->local_matrix = compose_trs_from_mesh_node(mn);
                mn->global_matrix = mn->local_matrix;
                for (int c = 0; c < mn->child_count; c++) {
                    update_node_global(mns, mn->children[c], mn->global_matrix);
                }
            }
        }
    }
}

void update_skin_matrices(){

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++) {
        
        AssetInstance * AI = &ASSET_INSTANCES[i];
        if (AI->free) continue;

        for (int s = 0; s < AI->skin_count; s++) {

            GLTFSkin * skin = &skins[AI->skin_indices[s]];

            for (int j = 0; j < skin->joint_count; j++) {

                MeshNode * node = &mns[AI->node_indices[skin->joints[j]]];
                skin->final_mat[j] = m4_mul(node->global_matrix, skin->inverse_bind[j]);
            }
        }
    }
}


void upload_material_data()
{
    
    int mat_count = 0;
    Material * mats = CAST_VEC(GLOBALS.materials, Material);
    Image * images = CAST_VEC(GLOBALS.images, Image);
    for (int i = 0; i < GLOBALS.materials.count; i++)
    {
        
        Material * mat = &mats[i];
        if (mat->diffuse_tex != -1)
        {
            MATERIAL_DATA.materials[i].albedo_handle = images[mat->diffuse_tex].handle;
        }
        else
        {
            MATERIAL_DATA.materials[i].albedo_handle = images[DEFAULT_DIFFUSE].handle;
        }
        if (mat->normal_tex != -1)
        {
            MATERIAL_DATA.materials[i].normal_handle = images[mat->normal_tex].handle;
        }
        else
        {
            MATERIAL_DATA.materials[i].normal_handle = images[DEFAULT_NORMAL].handle;
        }
        if (mat->arm_tex != -1 && !mat->override_arm)
        {
            
            MATERIAL_DATA.materials[i].arm_handle = images[mat->arm_tex].handle;
        }
        else
        {
            MATERIAL_DATA.materials[i].arm_handle = images[DEFAULT_ARM].handle;
        }
        if (mat->emissive_tex != -1)
        {
            MATERIAL_DATA.materials[i].emissive_handle = images[mat->emissive_tex].handle;
        }
        else
        {
            MATERIAL_DATA.materials[i].emissive_handle = images[DEFAULT_EMISSIVE].handle;
        }
        MATERIAL_DATA.materials[i].ambient = mat->ka;
        MATERIAL_DATA.materials[i].diffuse = mat->kd;
        MATERIAL_DATA.materials[i].specular = mat->ks;
        MATERIAL_DATA.materials[i].emissive = mat->ke;
        memcpy(MATERIAL_DATA.materials[i].absorption_coefficients, mat->absorption_coefficients, sizeof(float) * AUDIO_NUM_BANDS);
        memcpy(MATERIAL_DATA.materials[i].scattering_coefficients, mat->scattering_coefficients, sizeof(float) * AUDIO_NUM_BANDS);

        
        MATERIAL_DATA.materials[i].metallic = mat->metallic;
        MATERIAL_DATA.materials[i].roughness = mat->roughness;
        MATERIAL_DATA.materials[i].ao_strength = mat->ao_strength;
        MATERIAL_DATA.materials[i].normal_scale = mat->normal_scale;
        MATERIAL_DATA.materials[i].alpha_mode = mat->alpha_mode;
        MATERIAL_DATA.materials[i].alpha_cutoff = mat->alpha_cutoff;
        MATERIAL_DATA.materials[i].double_sided = mat->double_sided;
        MATERIAL_DATA.materials[i].emissive_strength = mat->emissive_strength;
        
    }

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MATERIAL_SSBO);

    MaterialGPU * gpu_mapped_ptr = (MaterialGPU*)glMapBufferRange(
        GL_SHADER_STORAGE_BUFFER, 
        0, 
        sizeof(MaterialGPU) * GLOBALS.materials.count, 
        GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT
    );

    if (gpu_mapped_ptr)
    {
        memcpy(gpu_mapped_ptr, &MATERIAL_DATA, sizeof(MaterialGPU) * GLOBALS.materials.count);
        glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
    }    
    else
    {
        glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, sizeof(MaterialGPU) * GLOBALS.materials.count, &MATERIAL_DATA);
    }
}




// unused old frustum culling code
void
    get_view_planes
    (
        vec4     planes[6],
        Camera * cam
    )

{



    mat4 VP = m4_mul(cam->cam_data.proj, cam->cam_data.view);

    vec4 r0 = (vec4){VP.a0, VP.b0, VP.c0, VP.d0};
    vec4 r1 = (vec4){VP.a1, VP.b1, VP.c1, VP.d1};
    vec4 r2 = (vec4){VP.a2, VP.b2, VP.c2, VP.d2};
    vec4 r3 = (vec4){VP.a3, VP.b3, VP.c3, VP.d3};
    planes[0] = vec4_add(r3, r0);
    planes[1] = vec4_sub(r3, r0);
    planes[2] = vec4_add(r3, r1);
    planes[3] = vec4_sub(r3, r1);
    planes[4] = vec4_add(r3, r2);
    planes[5] = vec4_sub(r3, r2);

    for (int i = 0; i < 6; i++) {
        float length = sqrtf(planes[i].x * planes[i].x + planes[i].y * planes[i].y + planes[i].z * planes[i].z);


        if (length > 0.00001f) {
            float inv_len = 1.0f / length;
            planes[i].x *= inv_len;
            planes[i].y *= inv_len;
            planes[i].z *= inv_len;
            planes[i].w *= inv_len;
        }
        
    }
    
}



bool
    aabb_in_frustum
    (
        vec3 min,
        vec3 max,
        vec4 planes[6]
    )

{
    for (int i = 0; i < 6; i++)
    {
        vec3 p = {
            planes[i].x >= 0.0f ? max.x : min.x,
            planes[i].y >= 0.0f ? max.y : min.y,
            planes[i].z >= 0.0f ? max.z : min.z
        };

        if (vec3_dot((vec3){planes[i].x, planes[i].y, planes[i].z}, p) + planes[i].w < 0.0f)
        {
            return false;
        }
    }

    return true;
}

void determine_mesh_visibility(Camera * camera, size_t * meshes_frame, size_t * meshes_frame_cnt)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    *meshes_frame_cnt = 0;
    vec4 planes[6];
    get_view_planes(planes, camera);

    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {

        MeshNode * mn = &mns[i];
        if (mn->mesh == -1 || mn->free) continue;
        Mesh * m = &meshes[mn->mesh];
        AABB aabb = transform_aabb_by_model_matrix(m->aabb, mn->global_matrix);
        vec3 min = (vec3){aabb.min_x, aabb.min_y, aabb.min_z};
        vec3 max = (vec3){aabb.max_x, aabb.max_y, aabb.max_z};
        
        meshes_frame[(*meshes_frame_cnt)++] = i;
    }
    
}


void populate_draw_commands(DrawCommand * dcs, int * draw_command_cnt)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    *draw_command_cnt = 0;
    Material * mats = CAST_VEC(GLOBALS.materials, Material);

    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);

    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {

        MeshNode * mn = &mns[i];
        if (mn->mesh == -1 || mn->free) continue;

        Mesh * m = &meshes[mn->mesh];
        SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);

        for (int j = 0; j < m->submeshes.count; j++)
        {

            DrawCommand * dc = &dcs[*draw_command_cnt];

            if (sms[j].material == -1)
            {
                dc->material = 0;
            }
            else
            {
                
                dc->material = sms[j].material;
            }

            dc->start_index = sms[j].draw_offset;
            dc->fwd_shader = mats[dc->material].fwd_shader;
            dc->gbuffer_shader = mats[dc->material].gbuffer_shader;
            dc->index_count = sms[j].index_count;
            dc->lm_offset = sms[j].lm_offset;
            dc->matrix = mn->global_matrix;
            dc->static_mesh = m->static_mesh;
            dc->mesh_id = m->id;
            dc->point_light_count = mn->point_light_count;
            dc->point_light_offset = mn->point_light_offset;
            dc->directional_light_count = mn->directional_light_count;
            dc->directional_light_offset = mn->directional_light_offset;
            dc->spot_light_count = mn->spot_light_count;
            dc->spot_light_offset = mn->spot_light_offset;
            dc->sm_id = j;
            dc->mn_id = mn->id;
            

            if (mn->skin != -1)
            {
                dc->skin_offset = skins[mn->skin].ssbo_offset;
            }
            else
            {
                dc->skin_offset = 0;
            }
            if (mats[dc->material].override_transparent)
            {
                dc->opaque = 0;
            }
            else
            {
                
                dc->opaque = (uint8_t)(mats[dc->material].kd.w * 255);
            }


            dc->sort_key = 0;

            dc->sort_key |= ((uint64_t)(dc->opaque) << 56ULL);
            dc->sort_key |= ((uint64_t)(dc->mesh_id) << 40ULL);
            dc->sort_key |= ((uint64_t)(dc->material) << 24ULL);
            dc->sort_key |= ((uint64_t)(dc->static_mesh) << 16ULL);
            dc->sort_key |= ((uint64_t)(dc->fwd_shader) << 0ULL);
                
            (*draw_command_cnt)++;

        }
        
    }

}


int compare_draw_commands(const void * a, const void * b)
{
    DrawCommand * da = (DrawCommand *)a;
    DrawCommand * db = (DrawCommand *)b;
    if (da->sort_key < db->sort_key) return 1;
    if (da->sort_key > db->sort_key) return -1;
    return 0;
    
}


void create_and_sort_draw_commands(int * dc_cnt, DrawCommand * dcs)
{
    populate_draw_commands(dcs, dc_cnt);
    
    qsort(dcs, *dc_cnt, sizeof(DrawCommand), compare_draw_commands);
}






TextBoxGPU TB_GPU[MAX_TEXT_BOXES];
float angle_tr = 0;


float compute_ui_scale(int base_w, int base_h, int target_w, int target_h){
    float x = (float)target_w / (float)base_w;
    float y = (float)target_h / (float)base_h;
    return fminf(x, y);
}

void add_text_box(Vector * tvs, Vector * tis, TextBox * tb, int current_box)
{
    
    if (tb->text == NULL) return;
    int len = strlen(tb->text);
    
    float x = tb->start.x, y = tb->start.y;
    float scale = tb->text_size;
    Font * fonts = CAST_VEC(FONTS, Font);
    Font * f = &fonts[tb->font_id];
    
    float max_x = 0;
    float max_y = 0;
    Glyph * glyphs = f->glyphs;

    for (int i = 0; i < len; i++){ 
        char ch = tb->text[i];

        Glyph *g = NULL;
        for (int j = 0; j < f->glyph_count; j++) {
            if (glyphs[j].id == ch) {
                g = &glyphs[j];
                break;
            }
        }
        if (!g) continue; 

        float x0 = x + g->xoffset * scale;
        float y0 = (y - 7 * scale) + g->yoffset * scale;
        float x1 = x0 + g->width * scale;
        float y1 = y0 + g->height * scale;
        float u0 = g->u0;
        float v0 = g->v0;
        float u1 = g->u1;
        float v1 = g->v1;
        vec2 center = (vec2){(x1-x0) / 2 + x0, (y1-y0)/2 + y0};

        TextVertex * v = vector_expand(tvs, 4);

        uint32_t * idx = vector_expand(tis, 6);

        v[0] = (TextVertex){.pos = (vec2){x0, y0}, .uv = (vec2){u0, v0}, .center = center, .color = tb->color, .fill = tb->fill, .state = 0, .uID = current_box};

        v[1] = (TextVertex){.pos = (vec2){x1, y0}, .uv = (vec2){u1, v0}, .center = center, .color = tb->color, .fill = tb->fill, .state = 0, .uID = current_box};
        
        v[2] = (TextVertex){.pos = (vec2){x0, y1}, .uv = (vec2){u0, v1}, .center = center, .color = tb->color, .fill = tb->fill, .state = 0, .uID = current_box};
        
        v[3] = (TextVertex){.pos = (vec2){x1, y1}, .uv = (vec2){u1, v1}, .center = center, .color = tb->color, .fill = tb->fill, .state = 0, .uID = current_box};

        uint32_t i0 = tvs->count-4;
        uint32_t i1 = tvs->count-3;
        uint32_t i2 = tvs->count-2;
        uint32_t i3 = tvs->count-1;

        idx[0] = i0;
        idx[1] = i1;
        idx[2] = i2;
        idx[3] = i2;
        idx[4] = i1;
        idx[5] = i3;

        
        x += g->xadvance * scale;
        
    }
}
void add_all_text_boxes_to_draw_buffer()
{
    TEXT_INDEX_COUNT = 0;
    Vector tvs;
    init_vector(&tvs, 0, sizeof(TextVertex));

    Vector tis;
    init_vector(&tis, 0, sizeof(uint32_t));

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    int current_box = 0;
    for (int i = 0; i < MENUS.count; i++)
    {
        
        UIMenu * m = &ms[i];
        TextBox * tbs = m->text_boxes;

        for (int j = 0; j < MAX_TEXT_BOXES; j++)
        {
            if (tbs[j].free) continue;
            add_text_box(&tvs, &tis, &tbs[j], current_box);
            current_box++;

        }
    }

    glBindBuffer(GL_ARRAY_BUFFER, TEXT_VBO);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(TextVertex) * tvs.count, tvs.buffer);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEXT_EBO);
    glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, sizeof(uint32_t) * tis.count, tis.buffer);
    
    TEXT_INDEX_COUNT = tis.count;
    
    if (tvs.buffer) free(tvs.buffer);
    if (tis.buffer) free(tis.buffer);
    
}


void GPU_upload_singular_text_box(UIContainer * cs, TextBox * tb, int * TC)
{
    
    int tb_count = *TC;
    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    float screen_scale_x = (float)SCREEN_WIDTH / UI_SCALE_WIDTH;
    float screen_scale_y = (float)SCREEN_HEIGHT / UI_SCALE_HEIGHT;
    vec2 text_tr = tb->translation;
    text_tr.x *= scale_x;
    text_tr.y *= scale_y;
    vec2 scale = tb->scale;
    mat4 mat = compose_2d_transform(text_tr, tb->angle, scale);
    uint32_t visible = tb->visible;

    float scale_screen = compute_ui_scale(UI_SCALE_WIDTH, UI_SCALE_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
    vec2 tr = tb->translation;
    tr.x *= screen_scale_x;
    tr.y *= screen_scale_y;
    vec2 tr_scale = tb->scale;
    tr_scale.x *= screen_scale_x;
    tr_scale.y *= screen_scale_y;
    mat4 mat_screen = compose_2d_transform(tr, tb->angle, tr_scale);
    uint32_t hovered = 0;

    float w = tb->w;
    float h = tb->h;
    vec2 p0 = (vec2){-w/2, -h/2};
    vec2 p1 = (vec2){w/2, -h/2};
    vec2 p2 = (vec2){-w/2, h/2};
    vec2 p3 = (vec2){w/2, h/2};
    vec3 pos0 = mat4_mul_point(mat_screen, (vec3){p0.x, p0.y, 0.0});
    vec3 pos1 = mat4_mul_point(mat_screen, (vec3){p1.x, p1.y, 0.0});
    vec3 pos2 = mat4_mul_point(mat_screen, (vec3){p2.x, p2.y, 0.0});
    vec3 pos3 = mat4_mul_point(mat_screen, (vec3){p3.x, p3.y, 0.0});
    float min_x = MIN(MIN(MIN(pos0.x, pos1.x), pos2.x), pos3.x);
    float max_x = MAX(MAX(MAX(pos0.x, pos1.x), pos2.x), pos3.x);
    float min_y = MIN(MIN(MIN(pos0.y, pos1.y), pos2.y), pos3.y);
    float max_y = MAX(MAX(MAX(pos0.y, pos1.y), pos2.y), pos3.y);

    if (MOUSE_X >= min_x && MOUSE_X <= max_x && MOUSE_Y >= min_y && MOUSE_Y <= max_y)
    {

        TB_GPU[tb_count].hovered = 1;
        tb->hovered = true;
    }
    else
    {

        tb->hovered = false;
        TB_GPU[tb_count].hovered = 0;

    }

    TB_GPU[tb_count].shadow = tb->shadow;
    TB_GPU[tb_count].shadow_dist = tb->shadow_dist;
    TB_GPU[tb_count].shadow_dir = tb->shadow_dir;

    TB_GPU[tb_count].color = (vec4){0,0,0,0};
    TB_GPU[tb_count].fill = (vec4){0,0,0,0};

    TB_GPU[tb_count].visible = visible;
    TB_GPU[tb_count].model = mat;
    if (tb->uID == EDITOR_SETTINGS.selected_text_id && tb->menu == EDITOR_SETTINGS.selected_ui_menu_id)
    {
        TB_GPU[tb_count].state = UISTATE_SELECTED;
    }
    else
    {
        TB_GPU[tb_count].state = tb->state;
    }
    (*TC)++;
}

UIElementGPU UI_GPU[MAX_UI_ELEMENTS] = {0};
typedef struct UIUIDPair
{

    uint32_t uid;
    uint32_t layer;
    
} UIUIDPair;

int compare_uid_pair(const void * a, const void * b)
{

    UIUIDPair * da = (UIUIDPair *)a;
    UIUIDPair * db = (UIUIDPair *)b;
    if (da->layer > db->layer) return 1;
    if (da->layer < db->layer) return -1;
    return 0;
    
}




UIUIDPair UI_UIDS[MAX_UI_ELEMENTS] = {0};
uint32_t UI_UID_OUT[MAX_UI_ELEMENTS] = {0};
int ui_element_count = 0;

void GPU_upload_singular_element(UIContainer * cs, UIElement * el, int * ui_el_count, int * el_count)
{
    int ec = *el_count;
    
    float scale_x = (float)TEXT_WIDTH / UI_SCALE_WIDTH;
    float scale_y = (float)TEXT_HEIGHT / UI_SCALE_HEIGHT;
    float screen_scale_x = (float)SCREEN_WIDTH / UI_SCALE_WIDTH;
    float screen_scale_y = (float)SCREEN_HEIGHT / UI_SCALE_HEIGHT;
    vec2 text_tr = el->translation;
    text_tr.x *= scale_x;
    text_tr.y *= scale_y;
    vec2 scale = el->scale;
    mat4 mat = compose_2d_transform(text_tr, el->angle, scale);
    uint32_t visible = el->visible;

    float scale_screen = compute_ui_scale(UI_SCALE_WIDTH, UI_SCALE_HEIGHT, SCREEN_WIDTH, SCREEN_HEIGHT);
    vec2 tr = el->translation;
    tr.x *= screen_scale_x;
    tr.y *= screen_scale_y;
    vec2 tr_scale = el->scale;
    tr_scale.x *= screen_scale_x;
    tr_scale.y *= screen_scale_y;
    mat4 mat_screen = compose_2d_transform(tr, el->angle, tr_scale);
    uint32_t hovered = 0;
    vec2 local = vec2_sub((vec2){MOUSE_X, MOUSE_Y}, text_tr);
    local.x *= 1.0f/el->scale.x;
    local.y *= 1.0f/el->scale.y;

    if (el->uID == EDITOR_SETTINGS.selected_ui_element_id && el->menu == EDITOR_SETTINGS.selected_ui_menu_id)
    {
        UI_GPU[ec].state = UISTATE_SELECTED;
    }
    else
    {
        UI_GPU[ec].state = el->state;
    }
    UI_GPU[ec].shadow = el->shadow;
    UI_GPU[ec].shadow_dist = el->shadow_dist;
    UI_GPU[ec].shadow_dir = el->shadow_dir;
    UI_GPU[ec].color = el->color;
    UI_GPU[ec].fill = el->fill;
    UI_GPU[ec].visible = visible;
    UI_GPU[ec].model = mat;
    UI_GPU[ec].pos = el->center;
    UI_GPU[ec].type = el->type;
    UI_GPU[ec].size = el->size;
    UI_GPU[ec].radius = el->radius;
    UI_GPU[ec].translation = el->translation;
    UI_GPU[ec].scale = el->scale;
    if (container_is_valid(el->parent_container, el->menu))
    {
        UIContainer * c = &cs[el->parent_container];
        UI_GPU[ec].masked = c->mask;
    }
    else
    {
        UI_GPU[ec].masked = false;
    }
    if (container_is_valid(el->container, el->menu))
    {
        UIContainer * c = &cs[el->container];
        UI_GPU[ec].padding = c->padding;
    }

    UI_UIDS[ui_element_count].uid = *el_count;
    UI_UIDS[ui_element_count].layer = el->layer;

    ui_element_count++;
    (*el_count)++;
}

int * container_ids = NULL;
size_t container_ids_cap = 0;


void upload_parent_container(UIElement * el, UIContainer * cs, UIElement * els, int * el_count, size_t offset)
{
    if (container_is_valid(el->parent_container, el->menu))
    {
        if (container_ids[el->parent_container + offset] == -1)
        {
            
            UIContainer * pc = &cs[el->parent_container];
            UIElement * pel = &els[pc->element];
            upload_parent_container(pel, cs, els, el_count, offset);

            container_ids[el->parent_container + offset] = *el_count;
            if (container_is_valid(pel->parent_container, el->menu))
            {
                UI_GPU[*el_count].container_id = container_ids[pel->parent_container + offset];
            } 
            else
            {
                UI_GPU[*el_count].container_id = -1;
            }
            GPU_upload_singular_element(cs, pel, &ui_element_count, el_count);

        }
    }
    
}

void GPU_upload_ui_elements()
{

    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    int el_count = 0;
    int tb_count = 0;

    size_t container_count = 0; 
    for (int i = 0; i < MENUS.count; i++)
    {
        for (int j = 0; j < MAX_UI_CONTAINERS; j++)
        {
            
            if (ms[i].containers[j].free) continue;
            container_count++;
        }
    }

    if (container_ids_cap < container_count)
    {
        int * new_cids = realloc(container_ids, sizeof(UIContainer) * container_count);
        container_ids_cap = container_count;
        if (new_cids)
        {
            container_ids = new_cids;
        }
        else
        {
            printf("Malloc Failed UI Ele\n");
            free(container_ids);
            container_ids = NULL;
            return;
        }
    }
    for (int i = 0; i < container_ids_cap; i++)
    {
        container_ids[i] = -1;
    }
    
    
    size_t offset = 0;
    ui_element_count = 0;
    for (int i = 0; i < MENUS.count; i++)
    {

        if (!EDITOR_SETTINGS.UI && i != PERFORMANCE_OVERLAY_MENU) continue;

        UIElement * els = ms[i].elements;
        TextBox * tbs = ms[i].text_boxes;
        UIContainer * cs = ms[i].containers;
        for (int j = 0; j < MAX_UI_ELEMENTS; j++)
        {

            UIElement * el = &els[j];
            if (el->free) continue;

            upload_parent_container(el, cs, els, &el_count, offset);
            if (container_is_valid(el->parent_container, el->menu))
            {
                UI_GPU[el_count].container_id = container_ids[el->parent_container + offset];
            }
            else
            {
                UI_GPU[el_count].container_id = -1;
            }
            GPU_upload_singular_element(cs, el, &ui_element_count, &el_count);
        }

        for (int j = 0; j < MAX_UI_ELEMENTS; j++)
        {

            TextBox * tb = &tbs[j];
            if (tb->free) continue;
            
            TB_GPU[tb_count].mask = 0;
            if (container_is_valid(tb->parent_container, tb->menu))
            {
                TB_GPU[tb_count].container_id = container_ids[tb->parent_container + offset];
                if (cs[tb->parent_container].mask)
                {
                    TB_GPU[tb_count].mask = 1;
                }
            }
            else
            {
                TB_GPU[tb_count].container_id = -1;
            }
            GPU_upload_singular_text_box(cs, tb, &tb_count);
        }
        
        for (int j = 0; j < MAX_UI_CONTAINERS; j++)
        {
            if (ms[i].containers[j].free) continue;
            offset++;
        }

    }

    qsort(UI_UIDS, ui_element_count, sizeof(UIUIDPair), compare_uid_pair);
    for (int i = 0; i < ui_element_count; i++)
    {
        UI_UID_OUT[i] = UI_UIDS[i].uid;
    }

    size_t text_box_ssbo_offset = current_frame * text_box_ssbo_size;
    uint32_t * text_box_ssbo_data = (uint32_t*)(text_box_ssbo_ptr + text_box_ssbo_offset);
    memcpy(text_box_ssbo_data, TB_GPU, sizeof(TextBoxGPU) * tb_count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 8, TEXT_BOX_SSBO, text_box_ssbo_offset, text_box_ssbo_size);

    size_t ui_ssbo_offset = current_frame * ui_ssbo_size;
    uint8_t * ui_ssbo_data = (uint8_t*)(ui_ssbo_ptr + ui_ssbo_offset);
    memcpy(ui_ssbo_data, UI_GPU, sizeof(UIElementGPU) * el_count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 9, UI_SSBO, ui_ssbo_offset, ui_ssbo_size);

    size_t ui_instance_vbo_offset = current_frame * ui_instance_vbo_size;
    uint32_t * ui_instance_vbo_data = (uint32_t*)(ui_instance_vbo_ptr + ui_instance_vbo_offset);
    memcpy(ui_instance_vbo_data, UI_UID_OUT, sizeof(uint32_t) * ui_element_count);

    

}





void bind_dynamic_shader(int shader, bool probe_bake)
{
    
    glUseProgram(DYNAMIC_SHADER[shader].program);
    Material * mats = CAST_VEC(GLOBALS.materials, Material);
    Image * images = CAST_VEC(GLOBALS.images, Image);
    

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, SHADOW_MAP_FBO.depth_texture.i);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[4].location, 1);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_3D, LPV_TEXTURE);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[5].location, 2);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SKYBOX);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[6].location, 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, BRDF_LUT);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[7].location, 4);
    
    if (shader == 1)
    {
        glUniform1f(DYNAMIC_SHADER[shader].uniforms[8].location, time);
    }

    
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, DIRECTIONAL_MAP_TEXTURE);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[14].location, 5);
    if (!probe_bake)
    {
        
        glActiveTexture(GL_TEXTURE6);
        glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, RPROBE_CUBE_MAP_ARRAY);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[9].location, 6);
    }
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, FBM_TEXTURE);
    glUniform1i(DYNAMIC_SHADER[shader].uniforms[10].location, 7);
    if (EDITOR_SETTINGS.SSAO)
    {
        
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, SSAO_BLUR_FBO.texture[0].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[22].location, 8);
    }
    else
    {
        glActiveTexture(GL_TEXTURE8);
        glBindTexture(GL_TEXTURE_2D, DUMMY_LIGHTMAP);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[22].location, 8);
    }
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, POINT_LIGHT_SHADOWS_FBO.depth_texture.i);
    glUniform1i(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uPLSCMA"), 9);
    
    glActiveTexture(GL_TEXTURE10);
    glBindTexture(GL_TEXTURE_2D, SSR_FBO.texture[0].i);
    glUniform1i(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uSSR"), 10);


    glUniform1f(DYNAMIC_SHADER[shader].uniforms[15].location, GLOBALS.probe_spacing);
    glUniform3f(DYNAMIC_SHADER[shader].uniforms[16].location, GLOBALS.world_min.x, GLOBALS.world_min.y, GLOBALS.world_min.z);
    glUniform3i(DYNAMIC_SHADER[shader].uniforms[17].location, GLOBALS.probe_tex_size.x, GLOBALS.probe_tex_size.y, GLOBALS.probe_tex_size.z);
    glUniform1ui(DYNAMIC_SHADER[shader].uniforms[23].location, PROBE_VOLUME_DEPTH_PADDED);
    glUniform2f(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uScreenRes"), RENDER_WIDTH, RENDER_HEIGHT);
    glUniform3ui(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uGridSize"), GRID_X, GRID_Y, GRID_Z);
    glUniform1f(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uNear"), VIEW_NEAR);
    glUniform1f(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uFar"), VIEW_FAR);
    glUniform1f(glGetUniformLocation(DYNAMIC_SHADER[shader].program, "uLogScale"), GRID_Z / log2f(VIEW_FAR / VIEW_NEAR));
}

void bind_material(Material * mat)
{
    Image * images = CAST_VEC(GLOBALS.images, Image);

    int shader = mat->fwd_shader;
    glActiveTexture(GL_TEXTURE11);
    if (mat->diffuse_tex == -1)
    {
    
        glBindTexture(GL_TEXTURE_2D, images[DEFAULT_DIFFUSE].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[18].location, 11);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, images[mat->diffuse_tex].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[18].location, 11);
    
    }
    glActiveTexture(GL_TEXTURE12);
    if (mat->normal_tex == -1)
    {
    
        glBindTexture(GL_TEXTURE_2D, images[DEFAULT_NORMAL].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[19].location, 12);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, images[mat->normal_tex].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[19].location, 12);
    
    }
    glActiveTexture(GL_TEXTURE13);
    if (mat->arm_tex == -1)
    {
    
        glBindTexture(GL_TEXTURE_2D, images[DEFAULT_ARM].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[20].location, 13);
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, images[mat->arm_tex].i);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[20].location, 13);
    
    }
    
}

InstanceData instance_matrices[MAX_INSTANCES];
int draw_command_offsets[MAX_DRAW_COMMANDS];
int draw_command_counts[MAX_DRAW_COMMANDS];
int draw_call_count = 0;
int instance_offset = 0;

InstanceData shadow_instance_matrices[MAX_INSTANCES];
int shadow_draw_command_offsets[MAX_DRAW_COMMANDS];
int shadow_draw_command_counts[MAX_DRAW_COMMANDS];
int shadow_draw_call_count = 0;
int shadow_instance_offset = 0;

MeshNodeSSBO mesh_node_data_gpu;


DrawArraysIndirectCommand arrays_indirect[MAX_MESH_NODES];
int array_offset = 0;
DrawElementsIndirectCommand elements_indirect[MAX_MESH_NODES];
int element_offset = 0;
TransformGPU transforms[MAX_MESH_NODES * MAX_INSTANCES];
int transform_count = 0; 
int static_count = 0;
int dynamic_count = 0;
void upload_instance_data(int * draw_calls, int * in_off, int dc_counts[MAX_DRAW_COMMANDS], int dc_off[MAX_DRAW_COMMANDS], int * draw_command_cnt, DrawCommand * dcs, MeshNodeSSBO * mn_data, uint8_t * instance_ptr, int current_frame_offset)
{

    int i = 0;
    int current_mat = -1;
    *in_off = 0;
    *draw_calls = 0;
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    int offset = 0;
    array_offset = 0;
    element_offset = 0;
    transform_count = 0;
    int total_instance_count = 0;
    while (i < *draw_command_cnt)
    {
        DrawCommand * first_command = &dcs[i];

        DrawCommand * current_command = first_command;
        int instance_count = 0;
        int arr = array_offset;
        int el = element_offset;

        Mesh * m = &meshes[current_command->mesh_id];
        if (m->static_mesh)
        {
            arrays_indirect[array_offset].count = current_command->index_count;
            arrays_indirect[array_offset].first = current_command->start_index;
            arrays_indirect[array_offset].baseInstance = 0;
            arrays_indirect[array_offset].instanceCount = 0;
            array_offset++;
        }
        else
        {
            elements_indirect[element_offset].count = current_command->index_count;
            elements_indirect[element_offset].firstIndex = current_command->start_index;
            elements_indirect[element_offset].baseVertex = 0;
            elements_indirect[element_offset].baseInstance = total_instance_count;
            elements_indirect[element_offset].instanceCount = 0;
            element_offset++;
            
        }
        mn_data->mns[offset].static_mesh = current_command->static_mesh;
        mn_data->mns[offset].lm_offset = current_command->lm_offset;
        mn_data->mns[offset].material = current_command->material;
        mn_data->mns[offset].first = current_command->start_index;
        mn_data->mns[offset].count = current_command->index_count;
        AABB aabb = meshes[current_command->mesh_id].aabb;
        mn_data->mns[offset].min = (vec4){aabb.min_x, aabb.min_y, aabb.min_z, 1.0};
        mn_data->mns[offset].max = (vec4){aabb.max_x, aabb.max_y, aabb.max_z, 1.0};
        
        mn_data->mns[offset].center = (vec4){aabb.center.x, aabb.center.y, aabb.center.z, 1.0};
        mn_data->mns[offset].extents = (vec4){aabb.extents.x, aabb.extents.y, aabb.extents.z, 1.0};


        
        while (i < *draw_command_cnt &&
                first_command->material == current_command->material &&
                first_command->static_mesh == current_command->static_mesh &&
                first_command->mesh_id == current_command->mesh_id &&
                first_command->sm_id == current_command->sm_id &&
                first_command->fwd_shader == current_command->fwd_shader &&
                i < MAX_INSTANCES)
        {


            int current_instance = i;

            current_command->array_offset = arr;
            current_command->element_offset = el;

            mns[current_command->mn_id].command_id[current_command->sm_id] = (m->static_mesh ? array_offset - 1 : element_offset - 1);
            mns[current_command->mn_id].mesh_data_id[current_command->sm_id] = offset;
            mns[current_command->mn_id].skin_offset = current_command->skin_offset;
            
            total_instance_count++;
            i++;
            current_command = &dcs[i];
            
        }
        mn_data->mns[offset].instance_count = instance_count;
        offset++;
    
    }


    memcpy(indirect_arrays_ptr, arrays_indirect, sizeof(DrawArraysIndirectCommand) * array_offset);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
    
    memcpy(indirect_elements_ptr, elements_indirect, sizeof(DrawElementsIndirectCommand) * element_offset);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 

    size_t mesh_node_ssbo_offset = current_frame * mesh_node_ssbo_size;

    uint32_t * mesh_node_ssbo_data = (uint32_t*)(mesh_node_ssbo_ptr + mesh_node_ssbo_offset);
    memcpy(mesh_node_ssbo_data, mn_data, sizeof(float) * 1 + sizeof(uint32_t) * 3 + sizeof(MeshNodeDataGPU) * *draw_command_cnt);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 15, MESH_NODE_SSBO, mesh_node_ssbo_offset, mesh_node_ssbo_size);

    
}

#define MAX_MESHES_PER_PT_LIGHT_SHADOW 32


void draw_arrays_elements(ShaderProgram * s, int start_arr, size_t start_array_offset, uint32_t array_count, int start_ele, size_t start_element_offset, uint32_t element_count)
{
    if (array_count > 0)
    {
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT); 
    
        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT);
        glUniform1ui(glGetUniformLocation(s->program, "uDynamicPass"), 0);
        glUniform1ui(glGetUniformLocation(s->program, "uStartArr"), start_arr);
        glMultiDrawArraysIndirect(GL_TRIANGLES, (void*)start_array_offset, array_count, sizeof(DrawArraysIndirectCommand));
    }

    if (element_count > 0)
    {
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT); 

        glBindBuffer(GL_DRAW_INDIRECT_BUFFER, INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT);
        glUniform1ui(glGetUniformLocation(s->program, "uOffset"), array_offset);
        glUniform1ui(glGetUniformLocation(s->program, "uStartEle"), start_ele);
        glUniform1ui(glGetUniformLocation(s->program, "uDynamicPass"), 1);
        glMultiDrawElementsIndirect(GL_TRIANGLES, GL_UNSIGNED_INT, (void*)start_element_offset, element_count, sizeof(DrawElementsIndirectCommand));
    }
}


void write_point_light_shadows()
{
    Light * lights = CAST_VEC(GLOBALS.lights, Light);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);

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
    glBindFramebuffer(GL_FRAMEBUFFER, POINT_LIGHT_SHADOWS_FBO.fbo);
    glViewport(0, 0, POINT_LIGHT_SHADOWS_FBO.w, POINT_LIGHT_SHADOWS_FBO.h);
    glFramebufferTexture3D(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_TEXTURE_CUBE_MAP_ARRAY,
        POINT_LIGHT_SHADOWS_FBO.depth_texture.i,
        0,
        0
    );
    mat4 proj = make_projection(90, 1, 0.05f, 100.0f);
    for (int i = 0; i < GLOBALS.lights.count; i++)
    {

        Light * l = &lights[i];

        if (l->light_type != POINT_LIGHT || master_frame % 2 == 0) continue;

        glUseProgram(CULL_POINT_LIGHT_SHADOWS_SHADER.program);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 

        glUniform3f(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[0].location, l->pos.x, l->pos.y, l->pos.z);
        glUniform1f(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[1].location, l->radius);
        glUniform1ui(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[2].location, transform_count);
        glUniform1ui(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[3].location, array_offset);

        glDispatchCompute((transform_count+63)/64, 1, 1);

        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);


        if (array_offset > 0)
        {
            
            glUseProgram(INDIRECT_PREFIX_SHADER.program);

            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT); 
            glUniform1ui(INDIRECT_PREFIX_SHADER.uniforms[0].location, array_offset);

            glDispatchCompute((array_offset+127)/128, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
        }
        
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ELEMENTS_COMMAND_BUFFER); 
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT); 
        if (element_offset > 0)
        {

            glUseProgram(ELEMENTS_PREFIX_SHADER.program);

            glUniform1ui(ELEMENTS_PREFIX_SHADER.uniforms[0].location, element_offset);

            glDispatchCompute((element_offset+127)/128, 1, 1);
            glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
        }
            

        float clear_color = 1.0f;
        glClearTexSubImage(POINT_LIGHT_SHADOWS_FBO.depth_texture.i, 0, 0, 0, 6 * i, POINT_LIGHT_SHADOWS_FBO.w, POINT_LIGHT_SHADOWS_FBO.h, 6, GL_DEPTH_COMPONENT, GL_FLOAT, &clear_color);
        

        mat4 VPs[6];
        for (int j = 0; j < 6; j++)
        {
            
            vec3 pos = l->pos;
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

            mat4 VP = m4_mul(proj, view);
            VPs[j] = VP;

        }
        glBindVertexArray(MAIN_VAO);
        glBindBuffer(GL_ARRAY_BUFFER, MAIN_VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MAIN_EBO);
        glBindBuffer(GL_ARRAY_BUFFER, INSTANCE_VBO);
        glUseProgram(POINT_LIGHT_SHADOW_SHADER.program);
        glUniformMatrix4fv(POINT_LIGHT_SHADOW_SHADER.uniforms[0].location, 6, GL_FALSE, (const GLfloat*)VPs);
        glUniform3f(POINT_LIGHT_SHADOW_SHADER.uniforms[1].location, l->pos.x, l->pos.y, l->pos.z);
        glUniform1f(POINT_LIGHT_SHADOW_SHADER.uniforms[2].location, 100.0f);
        glUniform1ui(POINT_LIGHT_SHADOW_SHADER.uniforms[3].location, 6 * i);

        draw_arrays_elements(&POINT_LIGHT_SHADOW_SHADER, 0, 0, array_offset, 0, 0, element_offset);

        
        l->sm_dirty = false;
        
    }
    glDisable(GL_CULL_FACE);
}




void upload_instances()
{
    
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Material * mats = CAST_VEC(GLOBALS.materials, Material);

    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);

    transform_count = 0;
    static_count = 0;
    dynamic_count = 0;

    for (int i = 0; i < draw_command_count; i++)
    {

        DrawCommand * dc = &DRAW_COMMANDS[i];

        MeshNode * mn = &mns[dc->mn_id];
        if (mn->mesh == -1 || mn->free) continue;
        Mesh * m = &meshes[mn->mesh];
        if (!m->static_mesh) continue;
        SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);
        SubMesh * sm = &sms[dc->sm_id];
        if (mn->skin != -1)
        {
            
            transforms[transform_count].tr = mat4_identity();
        }
        else
        {
            transforms[transform_count].tr = mn->global_matrix;
            
        }

        transforms[transform_count].command_id = mn->command_id[dc->sm_id];
        transforms[transform_count].mn_id = mn->mesh_data_id[dc->sm_id];
        transforms[transform_count].directional_lights = (uvec2){mn->directional_light_offset, mn->directional_light_count};
        transforms[transform_count].material = dc->material;
        transforms[transform_count].spot_lights = (uvec2){mn->spot_light_offset, mn->spot_light_count};
        transforms[transform_count].skin_offset = mn->skin_offset;

        transform_count++;

        static_count++;
    }


    for (int i = 0; i < draw_command_count; i++)
    {

        DrawCommand * dc = &DRAW_COMMANDS[i];

        MeshNode * mn = &mns[dc->mn_id];
        if (mn->mesh == -1 || mn->free) continue;
        Mesh * m = &meshes[mn->mesh];
        if (m->static_mesh) continue;
        SubMesh * sms = CAST_VEC(m->submeshes, SubMesh);
        SubMesh * sm = &sms[dc->sm_id];

        if (mn->skin != -1)
        {
            
            transforms[transform_count].tr = mat4_identity();
        }
        else
        {
            transforms[transform_count].tr = mn->global_matrix;
            
        }
        transforms[transform_count].command_id = mn->command_id[dc->sm_id];
        transforms[transform_count].mn_id = mn->mesh_data_id[dc->sm_id];
        transforms[transform_count].directional_lights = (uvec2){mn->directional_light_offset, mn->directional_light_count};
        transforms[transform_count].material = dc->material;
        transforms[transform_count].spot_lights = (uvec2){mn->spot_light_offset, mn->spot_light_count};
        transforms[transform_count].skin_offset = mn->skin_offset;

        transform_count++;
        dynamic_count++;

    }
    size_t instance_ssbo_offset = current_frame * instance_ssbo_size;

    uint32_t * instance_ssbo_data = (uint32_t*)(instance_ssbo_ptr + instance_ssbo_offset);
    memcpy(instance_ssbo_data, transforms, sizeof(TransformGPU) * transform_count);
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 7, INSTANCE_SSBO, instance_ssbo_offset, instance_ssbo_size);

    
    size_t prev_instance_ssbo_offset = ((current_frame + (BUFFER_FRAMES - 1)) % BUFFER_FRAMES) * instance_ssbo_size;
    glBindBufferRange(GL_SHADER_STORAGE_BUFFER, 9, INSTANCE_SSBO, prev_instance_ssbo_offset, instance_ssbo_size);

}

void render_all_draw_commands(bool probe_bake)
{


    if (probe_bake)
    {
        glEnable(GL_DEPTH_TEST);
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LEQUAL);
    }
    else
    {
        glDepthFunc(GL_EQUAL);
        glDepthMask(GL_FALSE);
    }
    glBindVertexArray(MAIN_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, MAIN_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, INSTANCE_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MAIN_EBO);

    Material * mats = CAST_VEC(GLOBALS.materials, Material);
    Image * images = CAST_VEC(GLOBALS.images, Image);
    int current_mat = 0;
    bool currently_static = false;

    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    int shader = -1;
    if (EDITOR_SETTINGS.current_debug_view != DVIEW_DEFAULT)
    {
        
        shader = EDITOR_SETTINGS.current_debug_view;
        bind_dynamic_shader(shader, probe_bake);
        if (probe_bake)
        {
            glUniform1i(DYNAMIC_SHADER[shader].uniforms[11].location, 1);

        }
        else
        {
            glUniform1i(DYNAMIC_SHADER[shader].uniforms[11].location, 0);

        }
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, GLOBALS.scene_data.lightmap_data.texture_id);
        glUniform1i(DYNAMIC_SHADER[shader].uniforms[0].location, 0);
    }

    int i = 0;
    while (i < draw_command_count)
    {
        DrawCommand * first_command = &DRAW_COMMANDS[i];
        DrawCommand * current_command = first_command;

        int starting_array_offset = first_command->array_offset;
        int starting_element_offset = first_command->element_offset;
        int arr = 0;
        int el = 0;
        
        while
            (
                first_command->opaque == current_command->opaque &&
                first_command->fwd_shader == current_command->fwd_shader &&
                i < draw_command_count
            )
        {
            
            if (current_command->static_mesh)
            {
                arr++;
            }
            else
            {
                el++;
            }
            i++;
            current_command = &DRAW_COMMANDS[i];
        }

        if (
                EDITOR_SETTINGS.current_debug_view == DVIEW_DEFAULT &&
                (
                    first_command->opaque != current_command->opaque ||
                    first_command->fwd_shader != shader
                )
            )
        {

            shader = first_command->fwd_shader;
            bind_dynamic_shader(shader, probe_bake);
            if (probe_bake)
            {
                glUniform1i(DYNAMIC_SHADER[shader].uniforms[11].location, 1);

            }
            else
            {
                glUniform1i(DYNAMIC_SHADER[shader].uniforms[11].location, 0);

            }
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, GLOBALS.scene_data.lightmap_data.texture_id);
            glUniform1i(DYNAMIC_SHADER[shader].uniforms[0].location, 0);
        }

        size_t start_array_offset = (sizeof(DrawArraysIndirectCommand) * starting_array_offset);
        size_t start_element_offset = (sizeof(DrawElementsIndirectCommand) * starting_element_offset);
        uint32_t array_count = arr;
        uint32_t element_count = el;
        
        if (first_command->opaque < 255)
        {
            glDepthFunc(GL_LEQUAL);
            glDepthMask(GL_FALSE);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            draw_arrays_elements(&DYNAMIC_SHADER[shader], starting_array_offset, start_array_offset, array_count, starting_element_offset, start_element_offset, element_count);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            draw_arrays_elements(&DYNAMIC_SHADER[shader], starting_array_offset, start_array_offset, array_count, starting_element_offset, start_element_offset, element_count);
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
        }
        else
        {
            
            draw_arrays_elements(&DYNAMIC_SHADER[shader], starting_array_offset, start_array_offset, array_count, starting_element_offset, start_element_offset, element_count);
        }
    }

}






RProbeUBO RPROBE_UBO_DATA;
int RPROBE_COUNT = 0;
void determine_closest_rprobes()
{


    
    RPROBE_UBO_DATA.probe_count = 0;
    float distances[MAX_RPROBES_PER_FRAME] = {INFINITY};

    int num_probes = GLOBALS.scene_data.reflection_probes.count;
    
    ReflectionProbe * rprobes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);
    const float MAX_DIST = 20.0f;
    for (int i = 0; i < num_probes; i++)
    {
        ReflectionProbe * p = &rprobes[i];
        
        float longest = -INFINITY;
        int longest_index = 0;
        if (RPROBE_UBO_DATA.probe_count < 4)
        {
            RPROBE_UBO_DATA.probe_indices[RPROBE_UBO_DATA.probe_count++].val = i;
            continue;
        }
        
        for (int j = 0; j < RPROBE_UBO_DATA.probe_count; j++)
        {
            if (distances[j] > longest)
            {
                longest_index = j;
                longest = distances[j];
            }
        }

        float current_dist = vec3_length(vec3_sub(CAM_POS, p->aabb.center));
        if (current_dist < longest)
        {
            RPROBE_UBO_DATA.probe_indices[longest_index].val = i;
        }
        

    }
    
    
}

ShadowmapUBO SHADOWMAP_FRAME_DATA;

void render_to_cubemap_face(Camera * rprobe_cam, GLuint render_fbo, int render_width, int render_height, bool probe_bake)
{
    
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    glBeginQuery(GL_TIME_ELAPSED, cull_queries[current_query_frame]);
    update_scene_nodes();
    update_skin_matrices();
    upload_lights();
    GLOBALS.lights_dirty = false;


    if (EDITOR_SETTINGS.MSAA)
    {
    
        glEnable(GL_MULTISAMPLE);
    }
    else
    {
    
        glDisable(GL_MULTISAMPLE);
    }


    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    size_t cam_ubo_offset = current_frame * cam_ubo_padded;
    CamData* frame_cam_data = (CamData*)(cam_ubo_ptr + cam_ubo_offset);    
    *frame_cam_data = rprobe_cam->cam_data;
    glBindBufferRange(GL_UNIFORM_BUFFER, 0, CAM_UBO, cam_ubo_offset, sizeof(CamData));
    
    

    size_t light_cam_ubo_offset = current_frame * light_cam_ubo_padded;
    ShadowmapUBO* frame_light_cam_data = (ShadowmapUBO*)(light_cam_ubo_ptr + light_cam_ubo_offset);    
    
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        SHADOWMAP_FRAME_DATA.proj[i] = LIGHT_CAM[i].cam_data.proj;
        SHADOWMAP_FRAME_DATA.view[i] = LIGHT_CAM[i].cam_data.view;
    }

    
    *frame_light_cam_data = SHADOWMAP_FRAME_DATA;

    glBindBufferRange(GL_UNIFORM_BUFFER, 2, LIGHT_CAM_UBO, light_cam_ubo_offset, sizeof(ShadowmapUBO));

    size_t rprobe_ubo_offset = current_frame * rprobe_ubo_padded;
    RProbeUBO* frame_rprobe_data = (RProbeUBO*)(rprobe_ubo_ptr + rprobe_ubo_offset);    
    *frame_rprobe_data = RPROBE_UBO_DATA;
    glBindBufferRange(GL_UNIFORM_BUFFER, 3, RPROBE_UBO, rprobe_ubo_offset, sizeof(RProbeUBO));

    size_t prev_cam_ubo_offset = ((current_frame + (BUFFER_FRAMES-1)) % BUFFER_FRAMES) * cam_ubo_padded;
    glBindBufferRange(GL_UNIFORM_BUFFER, 4, CAM_UBO, prev_cam_ubo_offset, sizeof(CamData));

    size_t probe_volume_ubo_offset = current_frame * probe_volume_ubo_padded;
    ProbeVolumeUBO* frame_probe_volume_data = (ProbeVolumeUBO*)(probe_volume_ubo_ptr + probe_volume_ubo_offset);    
    ProbeVolumeUBO frame_pv_data = {0};
    ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);
    for (int i = 0; i < LPV_SLOTS; i++)
    {
        if (!GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG[i]) continue;
        ProbeVolume * pv = &pvs[GLOBALS.scene_data.LOADED_LIGHTING_PV[i].pv_id];
        // frame_pv_data.loaded[i] = 1;
        frame_pv_data.obb[i].center = pv->obb.center;
        frame_pv_data.obb[i].extents = pv->obb.extents;
        frame_pv_data.obb[i].axis_x = (vec3){pv->obb.rot.a0, pv->obb.rot.a1, pv->obb.rot.a2};
        frame_pv_data.obb[i].axis_y = (vec3){pv->obb.rot.b0, pv->obb.rot.b1, pv->obb.rot.b2};
        frame_pv_data.obb[i].axis_z = (vec3){pv->obb.rot.c0, pv->obb.rot.c1, pv->obb.rot.c2};
        frame_pv_data.obb[i].loaded = 1;

    }

    *frame_probe_volume_data = frame_pv_data;
    glBindBufferRange(GL_UNIFORM_BUFFER, 6, PROBE_VOLUME_UBO, probe_volume_ubo_offset, sizeof(ProbeVolumeUBO));

    determine_light_visibility_per_mesh();

    glUseProgram(CULL_CLUSTERS_SHADER.program);
    glUniform3ui(CULL_CLUSTERS_SHADER.uniforms[0].location, GRID_X, GRID_Y, GRID_Z);
    glUniform1f(CULL_CLUSTERS_SHADER.uniforms[1].location, VIEW_NEAR);
    glUniform1f(CULL_CLUSTERS_SHADER.uniforms[2].location, VIEW_FAR);
    glUniform1ui(CULL_CLUSTERS_SHADER.uniforms[3].location, MAX_LIGHTS_PER_CLUSTER);
    glUniform1ui(CULL_CLUSTERS_SHADER.uniforms[4].location, GLOBALS.lights.count);

    glDispatchCompute(GRID_X, GRID_Y, GRID_Z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

    determine_closest_rprobes();
    upload_joint_matrices();

    if (draw_dirty)
    {
        create_and_sort_draw_commands(&draw_command_count, DRAW_COMMANDS);
        upload_instance_data(&draw_call_count, &instance_offset, draw_command_counts, draw_command_offsets, &draw_command_count, DRAW_COMMANDS, &mesh_node_data_gpu, instance_vbo_ptr, 0);
        draw_dirty = false;
        
    }

    upload_instances();
    glEndQuery(GL_TIME_ELAPSED);
    glBeginQuery(GL_TIME_ELAPSED, pl_queries[current_query_frame]);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, GLOBAL_INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 
    write_point_light_shadows();
        
    glEndQuery(GL_TIME_ELAPSED);
    glBeginQuery(GL_TIME_ELAPSED, sm_queries[current_query_frame]);
    glUseProgram(CULL_DRAW_CALLS_SHADER.program);

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, GLOBAL_INSTANCE_SSBO); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ARRAYS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER); 

    glUniform1ui(CULL_DRAW_CALLS_SHADER.uniforms[0].location, transform_count);
    vec4 planes[6];
    get_view_planes(planes, rprobe_cam);
    glUniform4fv(CULL_DRAW_CALLS_SHADER.uniforms[1].location, 6, (GLfloat*)planes);
    glUniform1ui(CULL_DRAW_CALLS_SHADER.uniforms[2].location, array_offset);

    glDispatchCompute((transform_count+63)/64, 1, 1);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
    

    if (array_offset > 0)
    {
        
        glUseProgram(INDIRECT_PREFIX_SHADER.program);

        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT); 
        glUniform1ui(INDIRECT_PREFIX_SHADER.uniforms[0].location, array_offset);

        glDispatchCompute((array_offset+127)/128, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);

    }

    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, INDIRECT_ELEMENTS_COMMAND_BUFFER); 
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 6, INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT); 
    if (element_offset > 0)
    {
        
        glUseProgram(ELEMENTS_PREFIX_SHADER.program);

        glUniform1ui(ELEMENTS_PREFIX_SHADER.uniforms[0].location, element_offset);

        glDispatchCompute((element_offset+127)/128, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT | GL_COMMAND_BARRIER_BIT);
    }

    
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, render_width, render_height);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDepthFunc(GL_LEQUAL);

    glBindVertexArray(SKY_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, SKY_VBO);

    
    glUseProgram(SKY_SHADER.program);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, SKYBOX);
    glUniform1i(SKY_SHADER.uniforms[0].location, 0);

    glDrawArrays(GL_TRIANGLES, 0, 36);


    glBindFramebuffer(GL_FRAMEBUFFER, SHADOW_MAP_FBO.fbo);
    glViewport(0, 0, SHADOW_MAP_FBO.w, SHADOW_MAP_FBO.h);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindVertexArray(MAIN_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, MAIN_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MAIN_EBO);
    glBindBuffer(GL_ARRAY_BUFFER, INSTANCE_VBO);

    glUseProgram(SHADOW_MAP_SHADER.program);
    for (int i = 0; i < DIRECTIONAL_LIGHT_COUNT; i++)
    {
        
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, SHADOW_MAP_FBO.depth_texture.i, 0, i);
        glClear(GL_DEPTH_BUFFER_BIT);

        glUniformMatrix4fv(SHADOW_MAP_SHADER.uniforms[0].location, 1, GL_FALSE, (const GLfloat*)&LIGHT_CAM[i].cam_data.proj);
        glUniformMatrix4fv(SHADOW_MAP_SHADER.uniforms[1].location, 1, GL_FALSE, (const GLfloat*)&LIGHT_CAM[i].cam_data.view);

        

        
        draw_arrays_elements(&SHADOW_MAP_SHADER, 0, 0, array_offset, 0, 0, element_offset);
    }


    
    
    glEndQuery(GL_TIME_ELAPSED);
    glDisable(GL_CULL_FACE);

    glBeginQuery(GL_TIME_ELAPSED, gbuffer_queries[current_query_frame]);


    if (EDITOR_SETTINGS.MSAA)
    {
        
        glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER_FBO.fbo);
        glViewport(0, 0, GBUFFER_FBO.w, GBUFFER_FBO.h);
    }
    else
    {
        
        glBindFramebuffer(GL_FRAMEBUFFER, GBUFFER_RESOLVE_FBO.fbo);
        glViewport(0, 0, GBUFFER_RESOLVE_FBO.w, GBUFFER_RESOLVE_FBO.h);
    }

    glBindBuffer(GL_ARRAY_BUFFER, INSTANCE_VBO);
    glClearColor(0,0,0,1.0);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    int i = 0;
    int shader = -1;
    while (i < draw_command_count)
    {
        DrawCommand * first_command = &DRAW_COMMANDS[i];
        DrawCommand * current_command = first_command;

        int starting_array_offset = first_command->array_offset;
        int starting_element_offset = first_command->element_offset;
        int arr = 0;
        int el = 0;
        
        while (
                first_command->opaque == current_command->opaque &&
                first_command->gbuffer_shader == current_command->gbuffer_shader &&
                i < draw_command_count)
        {
            
            // we are explicitly counting and this may become incorrectly phased in terms of the order of the actual draw commands on the gpu
            if (current_command->static_mesh)
            {
                arr++;
            }
            else
            {
                el++;
            }
            i++;
            current_command = &DRAW_COMMANDS[i];
        }

        if (first_command->fwd_shader != shader)
        {

            shader = first_command->gbuffer_shader;
            if (EDITOR_SETTINGS.MSAA)
            {
                
                glUseProgram(DEPTH_PRE_PASS_SHADER_MSAA[shader].program);
            }
            else
            {
                
                glUseProgram(DEPTH_PRE_PASS_SHADER[shader].program);
            }

        }
        size_t start_array_offset = (sizeof(DrawArraysIndirectCommand) * starting_array_offset);
        size_t start_element_offset = (sizeof(DrawElementsIndirectCommand) * starting_element_offset);
        uint32_t array_count = arr;
        uint32_t element_count = el;
        

        if (EDITOR_SETTINGS.MSAA)
        {
            
            draw_arrays_elements(&DEPTH_PRE_PASS_SHADER_MSAA[shader], starting_array_offset, start_array_offset, array_count, starting_element_offset, start_element_offset, element_count);
        }
        else
        {
            
            draw_arrays_elements(&DEPTH_PRE_PASS_SHADER[shader], starting_array_offset, start_array_offset, array_count, starting_element_offset, start_element_offset, element_count);
        }
    }
        
    if (EDITOR_SETTINGS.MSAA)
    {
        glBindFramebuffer(GL_READ_FRAMEBUFFER, GBUFFER_FBO.fbo);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, GBUFFER_RESOLVE_FBO.fbo);
        glBlitFramebuffer(0, 0, GBUFFER_FBO.w, GBUFFER_FBO.h,
                          0, 0, GBUFFER_RESOLVE_FBO.w, GBUFFER_RESOLVE_FBO.h,
                          GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
    }

    glUseProgram(DEPTH_DOWNSAMPLE_SHADER.program);
    glBindFramebuffer(GL_FRAMEBUFFER, DEPTH_DOWNSAMPLE_FBO.fbo);
    glViewport(0, 0, DEPTH_DOWNSAMPLE_FBO.w, DEPTH_DOWNSAMPLE_FBO.h);
    glClearColor(0,0,0,1.0);
    glClearDepth(1.0);
    glClear(GL_DEPTH_BUFFER_BIT);

    glActiveTexture(GL_TEXTURE9);
    // this will be pom depth since it's only used for screen space effects
    GLuint current_depth_texture = (!EDITOR_SETTINGS.FULL_GBUFFER) ? GBUFFER_RESOLVE_FBO.depth_texture.i : GBUFFER_RESOLVE_FBO.texture[GPOMDEPTH].i;
    glBindTexture(GL_TEXTURE_2D, current_depth_texture);
    glUniform1i(DEPTH_DOWNSAMPLE_SHADER.uniforms[0].location, 9);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    glEndQuery(GL_TIME_ELAPSED);

    glBeginQuery(GL_TIME_ELAPSED, dof_queries[current_query_frame]);

    if (EDITOR_SETTINGS.DOF && !probe_bake)
    {
            
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        glUseProgram(FOCAL_DISTANCE_SHADER.program);


        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        glUniform1i(FOCAL_DISTANCE_SHADER.uniforms[0].location, 10);

        glUniform2f(FOCAL_DISTANCE_SHADER.uniforms[1].location, 1.0f/RENDER_WIDTH, 1.0f/RENDER_HEIGHT);
        glUniform2f(FOCAL_DISTANCE_SHADER.uniforms[2].location, MAIN_FBO.w, MAIN_FBO.h);
        glUniform1f(FOCAL_DISTANCE_SHADER.uniforms[3].location, time_since_last_render);

        glDispatchCompute(1, 1, 1);
        glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    }

    glEndQuery(GL_TIME_ELAPSED);

    glBeginQuery(GL_TIME_ELAPSED, ssao_queries[current_query_frame]);
    if (EDITOR_SETTINGS.SSAO && !probe_bake)
    {
        
        glUseProgram(SSAO_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, SSAO_FBO.fbo);
        glViewport(0, 0, SSAO_FBO.w, SSAO_FBO.h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE9);
        if (SSAO_WIDTH == MAIN_FBO.w / 2 && SSAO_HEIGHT == MAIN_FBO.h / 2)
        {
            
            glBindTexture(GL_TEXTURE_2D, DEPTH_DOWNSAMPLE_FBO.texture[0].i);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        }
        glUniform1i(SSAO_SHADER.uniforms[0].location, 9);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, SSAO_NOISE_TEXTURE);
        glUniform1i(SSAO_SHADER.uniforms[2].location, 10);
        glUniform2i(SSAO_SHADER.uniforms[3].location, SSAO_FBO.w, SSAO_FBO.h);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(SSAO_BLUR_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, SSAO_BLUR_FBO.fbo);
        glViewport(0, 0, SSAO_BLUR_FBO.w, SSAO_BLUR_FBO.h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, SSAO_FBO.texture[0].i);
        glUniform1i(SSAO_BLUR_SHADER.uniforms[0].location, 9);
        glActiveTexture(GL_TEXTURE10);
        if (SSAO_WIDTH == MAIN_FBO.w / 2 && SSAO_HEIGHT == MAIN_FBO.h / 2)
        {
            
            glBindTexture(GL_TEXTURE_2D, DEPTH_DOWNSAMPLE_FBO.texture[0].i);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        }
        glUniform1i(SSAO_BLUR_SHADER.uniforms[1].location, 10);

        glDrawArrays(GL_TRIANGLES, 0, 3);
    }

    glEndQuery(GL_TIME_ELAPSED);
    
    
    glBeginQuery(GL_TIME_ELAPSED, render_queries[current_query_frame]);
    glBindFramebuffer(GL_FRAMEBUFFER, render_fbo);
    glViewport(0, 0, render_width, render_height);

    render_all_draw_commands(probe_bake);

    glEndQuery(GL_TIME_ELAPSED);

    glBeginQuery(GL_TIME_ELAPSED, ssr_queries[current_query_frame]);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    if (EDITOR_SETTINGS.MSAA)
    {
        
        glBindFramebuffer(GL_READ_FRAMEBUFFER, MAIN_FBO.fbo);
        
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, MAIN_RESOLVE_FBO.fbo);
        glBlitFramebuffer(0, 0, MAIN_FBO.w, MAIN_FBO.h,
                          0, 0, MAIN_RESOLVE_FBO.w, MAIN_RESOLVE_FBO.h,
                          GL_COLOR_BUFFER_BIT,
                          GL_NEAREST);
    }

    
    if (EDITOR_SETTINGS.SSR && !probe_bake)
    {
        
        glUseProgram(SSR_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, SSR_FBO.fbo);
        glViewport(0, 0, SSR_FBO.w, SSR_FBO.h);

        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

    
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.texture[GNORMAL].i);
        glUniform1i(SSR_SHADER.uniforms[0].location, 9);
        glActiveTexture(GL_TEXTURE10);
        if (SSR_WIDTH == MAIN_FBO.w / 2 && SSR_HEIGHT == MAIN_FBO.h / 2)
        {
        
            glBindTexture(GL_TEXTURE_2D, DEPTH_DOWNSAMPLE_FBO.texture[0].i);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        }
        glUniform1i(SSR_SHADER.uniforms[1].location, 10);
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.texture[GALBEDO].i);
        glUniform1i(SSR_SHADER.uniforms[2].location, 11);
        glActiveTexture(GL_TEXTURE12);
        glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.texture[GEXTRA].i);
        glUniform1i(SSR_SHADER.uniforms[3].location, 12);
        glActiveTexture(GL_TEXTURE13);
        glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);
        glUniform1i(SSR_SHADER.uniforms[4].location, 13);
        glUniform1f(SSR_SHADER.uniforms[5].location, time);

        glDrawArrays(GL_TRIANGLES, 0, 3);

    }
    glEndQuery(GL_TIME_ELAPSED);



    glBeginQuery(GL_TIME_ELAPSED, taa_queries[current_query_frame]);
    if (EDITOR_SETTINGS.TAA && !probe_bake)
    {
        
        glUseProgram(SSR_HISTORY_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, TAA_FBO[cur_history].fbo);
        glViewport(0, 0, TAA_FBO[cur_history].w, TAA_FBO[cur_history].h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);
        if (EDITOR_SETTINGS.SSR)
        {
        
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, SSR_FBO.texture[0].i);
        }
        else
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, 0);
        
        }
        glUniform1i(SSR_HISTORY_SHADER.uniforms[0].location, 0);
        glActiveTexture(GL_TEXTURE3);
        glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);
        glUniform1i(SSR_HISTORY_SHADER.uniforms[1].location, 3);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, TAA_FBO[cur_history ^ 1].texture[0].i);
        glUniform1i(SSR_HISTORY_SHADER.uniforms[2].location, 1);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.texture[GVEL].i);
        glUniform1i(SSR_HISTORY_SHADER.uniforms[3].location, 2);
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        glUniform1i(SSR_HISTORY_SHADER.uniforms[4].location, 10);
        glUniform2f(SSR_HISTORY_SHADER.uniforms[5].location, JITTER_OFFSET.x * 0.5, JITTER_OFFSET.y * 0.5);

        glDrawArrays(GL_TRIANGLES, 0, 3);

    }
    glEndQuery(GL_TIME_ELAPSED);
    glDisable(GL_BLEND);

    
    glBeginQuery(GL_TIME_ELAPSED, dof_queries[current_query_frame]);
    if (EDITOR_SETTINGS.DOF && !probe_bake)
    {
            
        glUseProgram(COC_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, COC_FBO.fbo);
        glViewport(0, 0, COC_FBO.w, COC_FBO.h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        if (DOF_WIDTH == MAIN_FBO.w / 2 && DOF_HEIGHT == MAIN_FBO.h / 2)
        {
        
            glBindTexture(GL_TEXTURE_2D, DEPTH_DOWNSAMPLE_FBO.texture[0].i);
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, current_depth_texture);
        }
        glUniform1i(COC_SHADER.uniforms[0].location, 0);
        glActiveTexture(GL_TEXTURE1);
        if (EDITOR_SETTINGS.TAA)
        {
            glBindTexture(GL_TEXTURE_2D, TAA_FBO[cur_history].texture[0].i);
            
        }
        else
        {
            glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);
            
        }
        glUniform1i(COC_SHADER.uniforms[1].location, 1);
        glUniform1f(COC_SHADER.uniforms[2].location, EDITOR_SETTINGS.DOF_focal_length);
        glUniform1f(COC_SHADER.uniforms[3].location, EDITOR_SETTINGS.DOF_fstop);
        glUniform1f(COC_SHADER.uniforms[4].location, EDITOR_SETTINGS.DOF_sensor_height);
        glUniform1f(COC_SHADER.uniforms[5].location, EDITOR_SETTINGS.DOF_max_blur_radius);

        glDrawArrays(GL_TRIANGLES, 0, 3);

        glUseProgram(BOKEH_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, BOKEH_FBO.fbo);
        glViewport(0, 0, BOKEH_FBO.w, BOKEH_FBO.h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, COC_FBO.texture[0].i);
        glUniform1i(BOKEH_SHADER.uniforms[0].location, 0);
        glUniform2f(BOKEH_SHADER.uniforms[1].location, 1.0f/DOF_WIDTH, 1.0f/DOF_HEIGHT);

        glDrawArrays(GL_TRIANGLES, 0, 3);

    }

    glEndQuery(GL_TIME_ELAPSED);
    glBeginQuery(GL_TIME_ELAPSED, bloom_queries[current_query_frame]);
    if (EDITOR_SETTINGS.BLOOM && !probe_bake)
    {
        glUseProgram(BLOOM_SHADER.program);
        glBindFramebuffer(GL_FRAMEBUFFER, BLOOM_PYRAMID[0].fbo);
        glViewport(0, 0, BLOOM_PYRAMID[0].w, BLOOM_PYRAMID[0].h);
        glClearColor(0,0,0,1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        if (EDITOR_SETTINGS.TAA)
        {
            
            glBindTexture(GL_TEXTURE_2D, TAA_FBO[cur_history].texture[0].i);
        }
        else
        {
            
            glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);
        }
        glUniform1i(BLOOM_SHADER.uniforms[0].location, 0);

        glDrawArrays(GL_TRIANGLES, 0, 3);


        glDisable(GL_BLEND);
        glUseProgram(BLOOM_DOWNSCALE_SHADER.program);
        for (int i = 0; i < 5; i++)
        {

            Framebuffer * src = &BLOOM_PYRAMID[i];
            Framebuffer * dst = &BLOOM_PYRAMID[i + 1];

            glViewport(0, 0, dst->w, dst->h);
            glBindFramebuffer(GL_FRAMEBUFFER, dst->fbo);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, src->texture[0].i);
            glUniform1i(BLOOM_DOWNSCALE_SHADER.uniforms[0].location, 0);
            
            glDrawArrays(GL_TRIANGLES, 0, 3);
            
        }


        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);        
        glUseProgram(BLOOM_UPSCALE_SHADER.program);
        
        for (int i = 4; i >= 0; i--)
        {

            Framebuffer * src = &BLOOM_PYRAMID[i+1];
            Framebuffer * dst = &BLOOM_PYRAMID[i];

            glViewport(0, 0, dst->w, dst->h);
            glBindFramebuffer(GL_FRAMEBUFFER, dst->fbo);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, src->texture[0].i);
            glUniform1i(BLOOM_UPSCALE_SHADER.uniforms[0].location, 0);
            
            glUniform1f(BLOOM_UPSCALE_SHADER.uniforms[1].location, 0.005f);

            glDrawArrays(GL_TRIANGLES, 0, 3);
            
        }
        glDisable(GL_BLEND);
    }
    
    glEndQuery(GL_TIME_ELAPSED);
}


int find_depth(BVHNode * nodes, BVHNode * node, int depth)
{
    int l = depth;
    if (node->left != -1)
    {
        l = find_depth(nodes, &nodes[node->left], depth + 1);
    }
    int r = depth;
    if (node->right != -1)
    {
        r = find_depth(nodes, &nodes[node->right], depth + 1);
    }
    
    return MAX(l, r);
}

void render_tree(BVHNode * nodes, BVHNode * node, mat4 tr, DebugVertex * points, int * count, int depth, int max_depth, vec3 start_color, vec3 end_color)
{



    if (node->left == -1 && node->right == -1)
    {
        
        DebugVertex * v = &points[*count];
        AABB a = transform_aabb_by_model_matrix(node->aabb, tr);
        aabb_to_vertices(v, &a, (vec3){0,0,0});
        for (int j = 0; j < 24; j++)
        {
            v[j].color = vec3_lerp(start_color, end_color, (float)depth / max_depth);
        }
        *count += 24;
    }
    
    if (node->left != -1)
    {
        render_tree(nodes, &nodes[node->left], tr, points, count, depth + 1, max_depth, start_color, end_color);
    }
    if (node->right != -1)
    {
        render_tree(nodes, &nodes[node->right], tr, points, count, depth + 1, max_depth, start_color, end_color);
    }
    
}

void render_bvh(BVHNode * nodes, int node_count, mat4 tr)
{

    int depth = find_depth(nodes, nodes, 0);


    DebugVertex * points = malloc(sizeof(DebugVertex) * node_count * 24);
    int count = 0;
    render_tree(nodes, nodes, tr, points, &count, 0, depth, (vec3){0,0,1}, (vec3){1,0,0});
    
    glBindBuffer(GL_ARRAY_BUFFER, WIREFRAME_VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(DebugVertex) * count, points, GL_STREAM_DRAW);
    glDrawArrays(GL_LINES, 0, count);

    free(points);
    
}


void render_probe_volume(ProbeVolume * pv, bool audio)
{
    
    OBB obb = pv->obb;
    obb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &obb);
    for (int j = 0; j < 24; j++)
    {
        DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0,0,1};
    }
    DEBUG_LINE_COUNT += 24;
    if ((EDITOR_SETTINGS.audio_probe_volume_overlay && audio) || (EDITOR_SETTINGS.lighting_probe_volume_overlay && (!audio)))
    {
        float dx = (obb.extents.x * 2) / (pv->resolution.x-1);
        float dy = (obb.extents.y * 2) / (pv->resolution.y-1);
        float dz = (obb.extents.z * 2) / (pv->resolution.z-1);

        vec3 fwd = {obb.rot.a0, obb.rot.b0, obb.rot.c0};
        vec3 right = {obb.rot.a1, obb.rot.b1, obb.rot.c1};
        vec3 up = {obb.rot.a2, obb.rot.b2, obb.rot.c2};

        vec3 step_x, step_y, step_z;
        step_x.x = fwd.x * dx;
        step_x.y = fwd.y * dx;
        step_x.z = fwd.z * dx;
        
        step_y.x = right.x * dy;
        step_y.y = right.y * dy;
        step_y.z = right.z * dy;

        step_z.x = up.x * dz;
        step_z.y = up.y * dz;
        step_z.z = up.z * dz;

        vec3 pos = vec3_add(obb.center, 
        vec3_add(vec3_scale(fwd, -obb.extents.x),
        vec3_add(vec3_scale(right, -obb.extents.y),
        vec3_scale(up, -obb.extents.z))));
        
        vec3 current_pos = pos;
        for (int z = 0; z < pv->resolution.z; z++)
        {
            for (int y = 0; y < pv->resolution.y; y++)
            {
                
                for (int x = 0; x < pv->resolution.x; x++)
                {
            
                    int index = z * pv->resolution.y * pv->resolution.x + y * pv->resolution.x + x;
                    vec3 color = {0.3, 0.3, 0};
                    if (index == DEBUG_CURRENT_APROBE)
                    {
                        color = (vec3){1,1,1};
                    }
                    if (index == 0)
                    {
                        color = (vec3){1,0,0};
                    }
                    current_pos.x = pos.x + x * step_x.x + y * step_y.x + z * step_z.x;
                    current_pos.y = pos.y + x * step_x.y + y * step_y.y + z * step_z.y;
                    current_pos.z = pos.z + x * step_x.z + y * step_y.z + z * step_z.z;
                    AABB aabb;
                    aabb.center = current_pos;
                    aabb.extents = (vec3){EDITOR_APROBE_SIZE, EDITOR_APROBE_SIZE, EDITOR_APROBE_SIZE};
                    if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
                    aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
                    for (int j = 0; j < 24; j++)
                    {

                        DEBUG_LINES[DEBUG_LINE_COUNT + j].color = color;
                    }
                    DEBUG_LINE_COUNT += 24;
                }
            }
        }
    }
}


void render()
{

    master_frame++;


    if (RENDER_FENCE[current_frame] != NULL)
    {
        GLenum waitReturn = glClientWaitSync(RENDER_FENCE[current_frame], GL_SYNC_FLUSH_COMMANDS_BIT, GL_TIMEOUT_IGNORED);

        glDeleteSync(RENDER_FENCE[current_frame]);
        RENDER_FENCE[current_frame] = NULL;
        
    }
    
    Animation * anim = find_animation("Shrieker_Walk", 0);
    if (anim)
    {
        bool still_has_time = (time <= animation_length(anim));
        play_animation(anim, time, 0);
        time += frame_time;
        if (!still_has_time) time = 0;
    }


    Light * lights = CAST_VEC(GLOBALS.lights, Light);


    if (EDITOR_SETTINGS.MSAA)
    {
        
        render_to_cubemap_face(&cam, MAIN_FBO.fbo, RENDER_WIDTH, RENDER_HEIGHT, false);
    }
    else
    {
        render_to_cubemap_face(&cam, MAIN_RESOLVE_FBO.fbo, RENDER_WIDTH, RENDER_HEIGHT, false);
        
    }

    glBeginQuery(GL_TIME_ELAPSED, main_queries[current_query_frame]);


    glDisable(GL_DEPTH_TEST);

    glDisable(GL_CULL_FACE);

    GPU_upload_ui_elements();

    glUseProgram(UI_SHADER.program);
    glBindFramebuffer(GL_FRAMEBUFFER, TEXT_FBO.fbo);
    glViewport(0, 0, TEXT_FBO.w, TEXT_FBO.h);
    glClearColor(0.0,0.0,0.0,0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glBindVertexArray(UI_VAO);
    mat4 text_ortho = make_ortho(0, TEXT_FBO.w, TEXT_FBO.h, 0, -1.0f, 1.0f);

    glUniformMatrix4fv(UI_SHADER.uniforms[0].location, 1, GL_FALSE, (GLfloat*)&text_ortho);

    glDrawArraysInstancedBaseInstance(GL_TRIANGLES, 0, 6, ui_element_count, current_frame * MAX_UI_ELEMENTS);


    

    glUseProgram(TEXT_SHADER.program);
    glBindFramebuffer(GL_FRAMEBUFFER, TEXT_FBO.fbo);
    glViewport(0, 0, TEXT_FBO.w, TEXT_FBO.h);
    glBindVertexArray(TEXT_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, TEXT_VBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEXT_EBO);

    glUniformMatrix4fv(TEXT_SHADER.uniforms[0].location, 1, GL_FALSE, (GLfloat*)&text_ortho);

    Image * images = CAST_VEC(GLOBALS.images, Image);
    Font * fonts = CAST_VEC(FONTS, Font);
    Font * font = &fonts[0];
    Image * image = &images[font->texture];

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, image->i);
    glUniform1i(TEXT_SHADER.uniforms[1].location, 0);
    glUniform1ui(TEXT_SHADER.uniforms[2].location, 1);


    glDrawElements(GL_TRIANGLES, TEXT_INDEX_COUNT, GL_UNSIGNED_INT, (void*)0);

    glUniform1ui(TEXT_SHADER.uniforms[2].location, 0);
    glDrawElements(GL_TRIANGLES, TEXT_INDEX_COUNT, GL_UNSIGNED_INT, (void*)0);

    
    glBindVertexArray(WIREFRAME_VAO);
    glBindBuffer(GL_ARRAY_BUFFER, WIREFRAME_VBO);
    if (EDITOR_SETTINGS.wireframe_toggled)
    {
        
        glUseProgram(WIREFRAME_SHADER.program);
        ConvexHull * hull = CAST_VEC(player.mesh.hulls, ConvexHull);
        SceneData * sd = &GLOBALS.scene_data;
        BVHNode * nodes = CAST_VEC(sd->bvh_nodes, BVHNode);

        Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

        LightmapData * ld = &GLOBALS.scene_data.lightmap_data;


        UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
        Vector points;

        if (EDITOR_SETTINGS.mesh_node_menu_open && EDITOR_SETTINGS.selected_mn_id >= 0 && EDITOR_SETTINGS.selected_mn_id < GLOBALS.mesh_nodes.count)
        {
            
            MeshNode * mn = &mns[EDITOR_SETTINGS.selected_mn_id];
            
            DEBUG_LINES[DEBUG_LINE_COUNT + 0].color = (vec3){1,0.6,0};
            DEBUG_LINES[DEBUG_LINE_COUNT + 0].pos = vec3_add(mn->translation, EDITOR_SETTINGS.rotation_axis);
            DEBUG_LINES[DEBUG_LINE_COUNT + 1].color = (vec3){1,0.2,1};
            DEBUG_LINES[DEBUG_LINE_COUNT + 1].pos = vec3_sub(mn->translation, EDITOR_SETTINGS.rotation_axis);

            DEBUG_LINE_COUNT += 2;
            
        }

        if (EDITOR_SETTINGS.audio_menu_open)
        {


            AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
            for (int i = 0; i < GLOBALS.audio_portals.count; i++)
            {

                AudioPortal * p = &portals[i];
                AABB aabb;
                aabb.center = p->pos;
                aabb.extents = (vec3){EDITOR_APORTAL_SIZE, EDITOR_APORTAL_SIZE, EDITOR_APORTAL_SIZE};
                if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
                aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
                for (int j = 0; j < 24; j++)
                {
                    DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0.2,0.6,1.0};
                }
                DEBUG_LINE_COUNT += 24;
            }
        
        }

        if (EDITOR_SETTINGS.audio_menu_open)
        {
        
            for (int i = 0; i < MAX_AUDIO_EMITTERS; i++)
            {
                if (AUDIO_EMITTERS[i].free) continue;
                AudioEmitter * e = &AUDIO_EMITTERS[i];
                AABB aabb;
                aabb.center = e->pos;
                aabb.extents = (vec3){EDITOR_AUDIO_EMITTER_SIZE, EDITOR_AUDIO_EMITTER_SIZE, EDITOR_AUDIO_EMITTER_SIZE};
                if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
                aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
                for (int j = 0; j < 24; j++)
                {
                    DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){1,0.6,0.7};
                }
                DEBUG_LINE_COUNT += 24;
            }
        }
    
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.p_left;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,0};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.q_left;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,0};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.p_right;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,1};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.q_right;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,1};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.q_left;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,0};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.left_hip_pos;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,1,0};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.q_right;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,1};
        DEBUG_LINE_COUNT++;
        DEBUG_LINES[DEBUG_LINE_COUNT].pos = player.skeleton.right_hip_pos;
        DEBUG_LINES[DEBUG_LINE_COUNT].color = (vec3){1,0,1};
        DEBUG_LINE_COUNT++;

    
    
        for (int i = 0; i < GLOBALS.lights.count; i++)
        {
            Light * l = &lights[i];
            AABB aabb;
            aabb.center = l->pos;
            aabb.extents = (vec3){EDITOR_LIGHT_SIZE, EDITOR_LIGHT_SIZE, EDITOR_LIGHT_SIZE};
            if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
            aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
            for (int j = 0; j < 24; j++)
            {
                DEBUG_LINES[DEBUG_LINE_COUNT + j].color = l->rgb;
            }
            DEBUG_LINE_COUNT += 24;
        }
        ReflectionProbe * probes = CAST_VEC(GLOBALS.scene_data.reflection_probes, ReflectionProbe);
        for (int i = 0; i < GLOBALS.scene_data.reflection_probes.count; i++)
        {

            ReflectionProbe * probe = &probes[i];
            AABB aabb = probe->aabb;
            if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
            aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
            for (int j = 0; j < 24; j++)
            {
                DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0,1,0};
            }
            DEBUG_LINE_COUNT += 24;

            AABB cap_pos;
            aabb.center = probe->capture_pos;
            aabb.extents = (vec3){EDITOR_LIGHT_SIZE, EDITOR_LIGHT_SIZE, EDITOR_LIGHT_SIZE};
            if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
            aabb_to_vertices(&DEBUG_LINES[DEBUG_LINE_COUNT], &aabb, (vec3){0,0,0});
            for (int j = 0; j < 24; j++)
            {
                DEBUG_LINES[DEBUG_LINE_COUNT + j].color = (vec3){0,1,1};
            }
            DEBUG_LINE_COUNT += 24;
        
        }

        // for now only displaying audio probe volumes
        ProbeVolume * pvs = CAST_VEC(GLOBALS.scene_data.audio_probe_volumes, ProbeVolume);
        for (int i = 0; i < GLOBALS.scene_data.audio_probe_volumes.count; i++)
        {
            if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
            render_probe_volume(&pvs[i], true);
       
        }
        ProbeVolume * lpvs = CAST_VEC(GLOBALS.scene_data.lighting_probe_volumes, ProbeVolume);
        for (int i = 0; i < GLOBALS.scene_data.lighting_probe_volumes.count; i++)
        {
            if (DEBUG_LINE_COUNT + 24 >= DEBUG_LINES_MAX) break;
            render_probe_volume(&lpvs[i], false);
        }
    

        glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(DebugVertex) * DEBUG_LINE_COUNT, DEBUG_LINES);
        glDrawArrays(GL_LINES, 0, DEBUG_LINE_COUNT);
    }


    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    int pp_shader_idx = -1;
    
    if (EDITOR_SETTINGS.pp_dview == PPDVIEW_DEFAULT)
    {
        
        uint32_t pp_perm = 0;
        if (EDITOR_SETTINGS.MBLUR)
        {
    
            pp_perm |= (1 << PP_MBLUR);
        }
        if (EDITOR_SETTINGS.DOF)
        {
    
            pp_perm |= (1 << PP_DOF);
        }
        if (EDITOR_SETTINGS.BLOOM)
        {
    
            pp_perm |= (1 << PP_BLOOM);
        }
        if (EDITOR_SETTINGS.NOISE)
        {
            pp_perm |= (1 << PP_NOISE);
        }
        if (EDITOR_SETTINGS.ACES)
        {
            pp_perm |= (1 << PP_ACES);
        }
        if (EDITOR_SETTINGS.LUT)
        {
            pp_perm |= (1 << PP_LUT);
        }
        for (int i = 0; i < PP_SHADER_VERSIONS; i++)
        {
            if (pp_perm == POSTPROCESS_SHADER[i].perm)
            {
                pp_shader_idx = i;
                break;
            }
        }
        if (pp_shader_idx == -1)
        {
            printf("ERROR: PP SHADER NOT FOUND\n");
            pp_shader_idx = 0;
        }
    }
    else
    {
        pp_shader_idx = PP_SHADER_VERSIONS + EDITOR_SETTINGS.pp_dview - 1;
    }


    ShaderProgram * pp_shader = &POSTPROCESS_SHADER[pp_shader_idx];
    
    glUseProgram(pp_shader->program);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);

    glUniform1i(pp_shader->uniforms[0].location, 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, HISTORY_TEXTURE[cur_history]);
    glUniform1i(pp_shader->uniforms[1].location, 1);
    glUniform1f(pp_shader->uniforms[2].location, time);
    glUniform1ui(pp_shader->uniforms[3].location, EDITOR_SETTINGS.MBLUR);
    glUniform1ui(pp_shader->uniforms[4].location, EDITOR_SETTINGS.TAA);
    glUniform1ui(pp_shader->uniforms[13].location, EDITOR_SETTINGS.DOF);
    glActiveTexture(GL_TEXTURE7);
    glBindTexture(GL_TEXTURE_2D, FBM_TEXTURE);
    glUniform1i(pp_shader->uniforms[5].location, 7);
    glActiveTexture(GL_TEXTURE8);
    glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.depth_texture.i);
    glUniform1i(pp_shader->uniforms[6].location, 8);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, TEXT_FBO.texture[0].i);
    glUniform1i(pp_shader->uniforms[7].location, 2);
    if (EDITOR_SETTINGS.TAA)
    {
        
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, TAA_FBO[cur_history].texture[0].i);
        glGenerateMipmap(GL_TEXTURE_2D);
        glUniform1i(pp_shader->uniforms[8].location, 10);
    }
    else
    {
        
        glActiveTexture(GL_TEXTURE10);
        glBindTexture(GL_TEXTURE_2D, MAIN_RESOLVE_FBO.texture[0].i);
        glUniform1i(pp_shader->uniforms[8].location, 10);
    }
    glActiveTexture(GL_TEXTURE11);
    // we aren't blurring ssr?
    glBindTexture(GL_TEXTURE_2D, SSR_FBO.texture[0].i);
    glUniform1i(pp_shader->uniforms[9].location, 11);
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.texture[GVEL].i);
    glUniform1i(pp_shader->uniforms[10].location, 3);
    glActiveTexture(GL_TEXTURE4);
    glBindTexture(GL_TEXTURE_2D, BOKEH_FBO.texture[0].i);
    glUniform1i(pp_shader->uniforms[11].location, 4);
    glActiveTexture(GL_TEXTURE5);
    glBindTexture(GL_TEXTURE_2D, COC_FBO.texture[0].i);
    glUniform1i(pp_shader->uniforms[12].location, 5);
    glActiveTexture(GL_TEXTURE6);
    glBindTexture(GL_TEXTURE_2D, BLOOM_PYRAMID[0].texture[0].i);
    glUniform1i(pp_shader->uniforms[14].location, 6);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_3D, COLOR_GRADE_LUT);
    glUniform1i(pp_shader->uniforms[15].location, 9);
    glActiveTexture(GL_TEXTURE12);
    glBindTexture(GL_TEXTURE_2D, GLOBALS.scene_data.lightmap_data.texture_id);
    glUniform1i(pp_shader->uniforms[20].location, 12);
    glUniform1f(glGetUniformLocation(pp_shader->program, "uPhysicsFramerate"), 60);
    glUniform1f(glGetUniformLocation(pp_shader->program, "uRenderFramerate"), EDITOR_SETTINGS.render_framerate_target);
    glUniform1ui(glGetUniformLocation(pp_shader->program, "uWireframeToggled"), EDITOR_SETTINGS.wireframe_toggled);
    glUniform1f(pp_shader->uniforms[16].location, EDITOR_SETTINGS.DOF_focal_length);
    glUniform1f(pp_shader->uniforms[17].location, EDITOR_SETTINGS.DOF_fstop);
    glUniform1f(pp_shader->uniforms[18].location, EDITOR_SETTINGS.DOF_sensor_height);
    glUniform1f(pp_shader->uniforms[19].location, EDITOR_SETTINGS.DOF_max_blur_radius);
    
    glDrawArrays(GL_TRIANGLES, 0, 3);

    glEndQuery(GL_TIME_ELAPSED);



    RENDER_FENCE[current_frame] = glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0);    

    current_frame = (current_frame + 1) % BUFFER_FRAMES;
    cur_history ^= 1;
    if (master_frame % 2000 == 0)
    {
        printf("FINISHING\n");
        glFinish();
    }
    
}



