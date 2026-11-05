#include <SDL2/SDL.h>
#include <GL/glew.h>
#include <SDL2/SDL_events.h>
#include <SDL2/SDL_mouse.h>
#include <SDL2/SDL_video.h>
#include "SDL2/SDL_atomic.h"
#include "SDL2/SDL_mutex.h"
#include "SDL2/SDL_opengl.h"
#include "editor/editor.h"
#include "fftw3.h"
#include "utilities/mathf.h"
#include <complex.h>
#include <math.h>
#include <stdatomic.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "init.h"
#include "core/mesh.h"
#include "render/render.h"
#include "core/engine_types.h"
#include "ui/ui.h"
#include "core/shaders.h"
#include "stb/stb_image.h"
#include "utilities/utilities.h"
#include "core/acceleration.h"
#include "ECS/entity.h"
#include "load/load.h"
#include "ui/fonts.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"
#include "audio/audio.h"
#include "audio/audio_ir.h"
#include "ui/ui_callbacks.h"




Material * init_new_material(const char * filename)
{
    
    Material * mat = vector_expand(&GLOBALS.materials, 1);
    if (!mat) return NULL;
    mat->ka = (vec4){0.5f, 0.5f, 0.5f, 0.5f};
    mat->kd = (vec4){1.0f, 1.0f, 1.0f, 1.0f};
    mat->ks = (vec4){0.0f, 0.0f, 0.0f, 1.0f};
    mat->ke = (vec4){0.0f, 0.0f, 0.0f, 1.0f};
    mat->diffuse_tex = -1;
    mat->arm_tex = -1;
    mat->normal_tex = -1;
    mat->emissive_tex = -1; 
    mat->ao_tex = -1; 
    mat->mr_tex = -1; 
    mat->metallic = 0.0;
    mat->roughness = 0.5;
    mat->normal_scale = 1.0;
    mat->alpha_cutoff = 0;
    mat->alpha_mode = 0;
    mat->double_sided = 0;
    mat->ao_strength = 1.0;
    char *str = NULL;
    strncpy(mat->name, filename, 126);
    mat->name[127] = '\0';
    return mat;
}
void init_mesh_new(Mesh * mesh)
{

    
    init_vector(&mesh->hulls, 0, sizeof(ConvexHull));
    mesh->tri_adjacency = NULL;
    mesh->nodes.buffer = NULL;
    mesh->tri_aabbs.buffer = NULL;
    mesh->leaf_indices.buffer = NULL;
    mesh->tri_adjacency = NULL;
    mesh->bvh_indices = NULL;
    ConvexHull * hull = vector_expand(&mesh->hulls, 1);
    hull->vertices.buffer = NULL;
    hull->faces.buffer = NULL;
    hull->hull_faces.buffer = NULL;
    hull->aabb = mesh->aabb;
    init_vector(&hull->vertices, 0, sizeof(vec3));

    Point * pts = CAST_VEC(mesh->vertices, Point);
    for (int i = 0; i < mesh->vertices.count; i++)
    {
        vec3 * p = vector_expand(&hull->vertices, 1);
        *p = pts[i].pos;
        
    }

    init_vector(&hull->faces, 0, sizeof(MeshFace));
    MeshFace * ts = CAST_VEC(mesh->tris, MeshFace);

    for (int i = 0; i < mesh->tris.count; i++)
    {
        MeshFace * p = vector_expand(&hull->faces, 1);
        *p = ts[i];
        
    }
    init_vector(&hull->hull_faces, 0, sizeof(HullFace));
    if (mesh->static_mesh)
    {
        
        init_mesh_tri_bvh(mesh);
        mesh->bvh_prim = true;
    }
    else
    {
        incremental_hull(hull);
        compute_mesh_hulls(hull);

        mesh->bvh_prim = false;
        
    }
    
    build_adjacency_map(mesh);
    
}

#define MN_STARTING_SIZE 10000
void MN_INIT()
{

    init_vector(&GLOBALS.mesh_nodes, MN_STARTING_SIZE, sizeof(MeshNode));

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < MN_STARTING_SIZE; i++)
    {
        MeshNode * mn = &mns[i];
        mn->free = true;
        mn->id = i;
    }
    
}

#define SKIN_STARTING_SIZE 10000
void SKIN_INIT()
{

    init_vector(&GLOBALS.skins, SKIN_STARTING_SIZE, sizeof(GLTFSkin));

    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    for (int i = 0; i < SKIN_STARTING_SIZE; i++)
    {
        GLTFSkin * s = &skins[i];
        s->free = true;
        s->id = i;
    }
    
}

MeshNode * find_free_mn()
{
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < GLOBALS.mesh_nodes.capacity; i++)
    {
        
        MeshNode * mn = &mns[i];
        if (mn->free)
        {
            mn->free = false;
            return mn;
        }

    }

    size_t count_before = GLOBALS.mesh_nodes.capacity;
    MeshNode * mn = vector_expand(&GLOBALS.mesh_nodes, 1);
    mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = count_before; i < GLOBALS.mesh_nodes.capacity; i++)
    {
        mns[i].free = true;
        mns[i].id = i;
    }
    
    mn->free = false;
    return mn;
    
}

void * find_free_in_vector(Vector * v, size_t byte_size, size_t free_byte_offset, size_t id_byte_offset)
{
    uint8_t * p = v->buffer;
    for (int i = 0; i < v->capacity; i++)
    {
        
        uint8_t * pt = p + byte_size * i;
        bool * f = (bool*)(pt + free_byte_offset);
        if (*f)
        {
            *f = false;
            return pt;
        }

    }

    size_t count_before = v->capacity;
    void * mn = vector_expand(v, 1);
    
    p = v->buffer;
    for (int i = count_before; i < v->capacity; i++)
    {
        uint8_t * pt = p + byte_size * i;
        bool * f = (bool*)(pt + free_byte_offset);
        size_t * id = (size_t*)(pt + id_byte_offset);
        *f = true;
        *id = i;
    }
    
    bool * f = (bool*)(p + free_byte_offset);
    *f = false;
    return mn;

    
}


Asset * retrieve_asset_by_name(const char * asset_name)
{
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
    int aidx = -1;
    for (int i = 0; i < GLOBALS.assets.count; i++)
    {
        Asset * a = &assets[i];
        if (strncmp(a->path, asset_name, 127) == 0)
        {
            aidx = i;
            break;
        }

    }
    if (aidx == -1)
    {
        printf("Asset %s not found.\n", asset_name);
        return NULL;
    }
    return &assets[aidx];
}

Mesh * retrieve_mesh_by_name(const char * asset_name, const char * mesh_name)
{
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
    int aidx = -1;
    for (int i = 0; i < GLOBALS.assets.count; i++)
    {
        Asset * a = &assets[i];
        if (strncmp(a->path, asset_name, 127) == 0)
        {
            aidx = i;
            break;
        }

    }
    if (aidx == -1)
    {
        printf("Asset %s not found.\n", asset_name);
        return NULL;
    }
    Asset * asset = &assets[aidx];

    int * mesh_indices = CAST_VEC(asset->mesh_indices, int);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    int idx = -1;
    for (int i = 0; i < asset->mesh_indices.count; i++)
    {

        Mesh * mesh = &meshes[mesh_indices[i]];
        
        if (strncmp(mesh->name, mesh_name, 127) == 0)
        {
            idx = mesh_indices[i];
            break;
        }
        
    }

    if (idx == -1)
    {
        printf("Mesh %s not found.\n", mesh_name);
        return NULL;
    }
    Mesh * mesh = &meshes[idx];

    return mesh;

    
}

void set_mn_components_to_children(MeshNode * mns, MeshNode * mn, MeshNode * parent)
{
    mn->ent = parent->ent;
    mn->pc = parent->pc;

    for (int i = 0; i < mn->child_count; i++)
    {
        set_mn_components_to_children(mns, &mns[mn->children[i]], mn);
    }
    
}


AssetInstance * init_new_asset_instance(const char * asset_name, vec3 tr, vec4 rot, vec3 scale)
{
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
    
    Asset * asset = retrieve_asset_by_name(asset_name);
    if (!asset) return NULL;
    mat4 mat = compose_trs_from_vectors(tr, rot, scale);

    AssetInstance * instance = NULL;
    
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++)
    {
        if (ASSET_INSTANCES[i].free)
        {
            instance = &ASSET_INSTANCES[i];
            instance->free = false;
            instance->root_id = asset->root_id;
            instance->node_count = asset->node_count;
            instance->skin_count = asset->skin_count;
            break;
        }
        
    }
    if (instance == NULL) return NULL;

    instance->first_node = GLOBALS.mesh_nodes.count;
    instance->first_skin = GLOBALS.skins.count;
    instance->aabb = asset->aabb;
    instance->node_indices = malloc(sizeof(size_t) * instance->node_count);
    instance->tr = tr;
    instance->rot = rot;
    instance->scale = scale;
    instance->model = compose_trs_from_vectors(tr, rot, scale);
    
    for (int i = 0; i < asset->node_count; i++)
    {
        
        MeshNode * mn = find_free_in_vector(&GLOBALS.mesh_nodes, sizeof(MeshNode), offsetof(MeshNode, free), offsetof(MeshNode, id));
        size_t mn_id = mn->id;
        *mn = asset->nodes[i];
        mn->free = false;
        mn->id = mn_id;
        mn->local_id = asset->nodes[i].id;
        instance->node_indices[i] = mn->id;
        
        if (mn->parent == -1)
        {
            mn->root = true;
        }
        else
        {
            mn->root = false;
        }
        mn->ent = NULL;
        mn->pc = NULL;
        mn->asset_instance = instance->id;
        mn->uv_map = NULL;
        mn->skip_directional_uv_lighting = false;
        strcpy(mn->name, asset->nodes[i].name);
        if (mn->skin != -1)
        {
            
            mn->skin += instance->first_skin;
        }
        
        memcpy(mn->children, asset->nodes[i].children, sizeof(int) * asset->nodes[i].child_count);
        
    }
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < asset->node_count; i++)
    {

        MeshNode * mn = &mns[instance->node_indices[i]];
        Entity * ent = NULL;
        if (mn->physics_mode >= 1)
        {
            ent = create_entity(instance, mn->id, true, false);
        }
        if (mn->parent != -1)
        {
            mn->parent = instance->node_indices[mn->parent];
        }
        for (int j = 0; j < mn->child_count; j++)
        {
            if (mn->children[j] != -1)
            {
                mn->children[j] = instance->node_indices[mn->children[j]];
                
                set_mn_components_to_children(mns, &mns[mn->children[j]], mn);
            }

        }
    }
    
    instance->skin_indices = malloc(sizeof(size_t) * instance->skin_count);

    for (int i = 0; i < asset->skin_count; i++)
    {
        GLTFSkin * skin = find_free_in_vector(&GLOBALS.skins, sizeof(GLTFSkin), offsetof(GLTFSkin, free), offsetof(GLTFSkin, id));
        size_t skin_index = skin->id;
        *skin = asset->skins[i];
        skin->id = skin_index;
        skin->free = false;
        instance->skin_indices[i] = skin->id;
        
        skin->joint_count = asset->skins[i].joint_count;
        skin->joints = malloc(sizeof(int) * skin->joint_count);
        memcpy(skin->joints, asset->skins[i].joints, sizeof(int) * skin->joint_count);
        skin->inverse_bind = malloc(sizeof(mat4) * skin->joint_count);
        memcpy(skin->inverse_bind, asset->skins[i].inverse_bind, sizeof(mat4) * skin->joint_count);
        skin->final_mat = calloc(asset->skins[i].joint_count, sizeof(mat4));
        strcpy(skin->name, asset->skins[i].name);
        memcpy(skin->animation_indices, asset->skins[i].animation_indices, sizeof(int) * MAX_ANIMATION_INDICES);
    }

    instance->asset_id = asset->asset_id;

    return instance;
    
}


// this only works if asset instance is newest in the array, for undos
void destroy_asset_instance(uint32_t id)
{

    AssetInstance * a = &ASSET_INSTANCES[id];
    
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < a->node_count; i++)
    {

        MeshNode * mn = &mns[a->node_indices[i]];
        mn->free = true;
        
    }
   
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    for (int i = 0; i < a->skin_count; i++)
    {

        GLTFSkin * s = &skins[a->skin_indices[i]];
        if (s->joints) free(s->joints);
        if (s->inverse_bind) free(s->inverse_bind);
        if (s->final_mat) free(s->final_mat);
        s->free = true;

    }

    if (a->node_indices) free(a->node_indices);
    if (a->skin_indices) free(a->skin_indices);
    a->free = true;
    
}





void init_player_PC(PhysicsComponent * pc, Player * player)
{

    pc->center_of_mass = (vec3){0,0,player->height/2};
    pc->vel = (vec3){0,0,0};
    pc->angular_velocity = (vec3){0,0,0};
    float mass = 100.0f;
    pc->mass = mass;
    pc->restitution = 0.0f;
    pc->mu = 0.5f;
    pc->player_pc = true;

    float w = player->radius * 2;
    float d = player->radius * 2;
    float h = player->height;
    float w2 = w*w;
    float d2 = d*d;
    float h2 = h*h;
    
    pc->inverse_inertia.a0 = 12.0f / (mass * (h2 + d2));
    pc->inverse_inertia.a1 = 0;
    pc->inverse_inertia.a2 = 0;
    pc->inverse_inertia.b0 = 0;
    pc->inverse_inertia.b1 = 12.0f / (mass * (w2 + d2));
    pc->inverse_inertia.b2 = 0;
    pc->inverse_inertia.c0 = 0;
    pc->inverse_inertia.c1 = 0;
    pc->inverse_inertia.c2 = 12.0f / (mass * (w2 + h2));

    
}
void init_player()
{
    player.radius = 0.4f;
    player.height = 0.8f;
    player.cam = &cam;
    
    player.cam->cam_data.cam_pos.x = spawn_point.x;
    player.cam->cam_data.cam_pos.y = spawn_point.y;
    player.cam->cam_data.cam_pos.z = spawn_point.z;
    player.pos = spawn_point;

    Skeleton * s = &player.skeleton;

    s->hip_height = 0.42f;
    s->neck_height = 0.75f;
    s->pelvis_hip_diff = 0.02f;
    s->pelvis_height = s->hip_height + s->pelvis_hip_diff;
    s->neck_pelvis_diff = s->neck_height - s->pelvis_height;
    s->upper_leg_length = 0.2f;
    s->lower_leg_length = 0.28f;
    s->pelvis.damp = 11;
    s->pelvis.stiff = 120;
    s->pelvis.pos = spawn_point;
    
    s->pelvis.pos.z += s->pelvis_height;
    s->pelvis.velocity = (vec3){0,0,0};
    s->pelvis.target = (vec3){0,0,0};

    s->neck.damp = 12;
    s->neck.stiff = 90;
    s->neck.damp_rot = 24;
    s->neck.stiff_rot = 144;
    s->neck.pos = spawn_point;
    s->neck.pos.z += s->neck_height;
    s->neck.velocity = (vec3){0,0,0};
    s->neck.target = (vec3){0,0,0};

    s->left_leg.pos = spawn_point;
    s->right_leg.pos = spawn_point;
    
    init_vector(&player.mesh.hulls, 1, sizeof(ConvexHull));
    ConvexHull * hulls = CAST_VEC(player.mesh.hulls, ConvexHull);
    ConvexHull * hull = &hulls[0];
    Vector * faces = &hull->faces;
    Vector * vertices = &hull->vertices;
    init_vector(faces, 0, sizeof(MeshFace));
    init_vector(vertices, 0, sizeof(vec3));
    vec3 * v = vector_expand(vertices, 8);
    PhysicsComponent * pc = NULL;
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free)
        {
            pc = &PHYSICS_COMPONENTS[i];
            PHYSICS_COMPONENTS[i].free = false;
            PHYSICS_COMPONENTS[i].ent = NULL;

            break;
        
        }
    }
    if (!pc)
    {
        
        printf("NO PC SLOT FOUND\n");
        player.pc = NULL;
    }
    else
    {
        player.pc = pc;
        init_player_PC(pc, &player);

    }

    


    vec3 extents = {player.radius / 2, player.radius / 2, player.height / 2 + 0.1};
    vec3 center = (vec3){0, 0, player.height / 2 - 0.1};
    hull->centroid = center;
    hull->aabb.center = center;


    int count = 0;
    for (int z = -1; z <= 1; z += 2){
        for (int y = -1; y <= 1; y += 2){
            for (int x = -1; x <= 1; x += 2){
                v[count++] = vec3_add(center, vec3_mul((vec3){x, y, z}, extents));
            }
        }
    }

    incremental_hull(hull);
    compute_mesh_hulls(hull);

    player.mesh.aabb.center = center;
    player.mesh.aabb.extents = extents;
    calculate_aabb_bounds(&player.mesh.aabb);

   
    
}


void GPU_unwrap_mesh_to_buffers(Mesh * m, Vector * vertices, Vector * tris, Vector * nodes)
{

    size_t vertex_offset = vertices->count;
    size_t tri_offset = tris->count;
    size_t node_offset = nodes->count;
    VertexGPU * vs = vector_expand(vertices, m->vertices.count);
    TriGPU * ts = vector_expand(tris, m->tris.count);
    BVHNodeGPU * ns = vector_expand(nodes, m->nodes.count);

    Point * pts = CAST_VEC(m->vertices, Point);
    MeshFace * mfs = CAST_VEC(m->tris, MeshFace);
    BVHNode * bvh_nodes = CAST_VEC(m->nodes, BVHNode);
    
    for (int i = 0; i < m->vertices.count; i++)
    {

        Point * pt = &pts[i];
        VertexGPU * v = &vs[i];
        v->normal = pt->normal;
        v->pos = pt->pos;
        v->uv = pt->uv;
        
    }

    for (int i = 0; i < m->tris.count; i++)
    {
        MeshFace * mf = &mfs[m->bvh_indices[i]];
        TriGPU * t = &ts[i];
        t->mesh_id = m->id;
        t->indices.x = (uint32_t)(mf->indices[0] + vertex_offset);
        t->indices.y = (uint32_t)(mf->indices[1] + vertex_offset);
        t->indices.z = (uint32_t)(mf->indices[2] + vertex_offset);
        t->normal = mf->normal;
        t->material_id = mf->material_id;
        t->bvh_index = m->bvh_indices[i] + tri_offset;
        t->tri_idx = m->bvh_indices[i];
        t->v0.pos = pts[mf->indices[0]].pos;
        t->v0.normal = pts[mf->indices[0]].normal;
        t->v0.uv = pts[mf->indices[0]].uv;
        t->v1.pos = pts[mf->indices[1]].pos;
        t->v1.normal = pts[mf->indices[1]].normal;
        t->v1.uv = pts[mf->indices[1]].uv;
        t->v2.pos = pts[mf->indices[2]].pos;
        t->v2.normal = pts[mf->indices[2]].normal;
        t->v2.uv = pts[mf->indices[2]].uv;
        
    }
    
    for (int i = 0; i < m->nodes.count; i++)
    {

        BVHNode * node = &bvh_nodes[i];
        BVHNodeGPU * n = &ns[i];
        n->parent = (node->parent == -1) ? -1 : node->parent + node_offset;
        n->left = (node->left == -1) ? -1 : node->left + node_offset;
        n->right = (node->right == -1) ? -1 : node->right + node_offset;
        n->next = (node->next == -1) ? -1 : node->next + node_offset;
        n->index = node->index + node_offset;
        n->first_index = node->first_index + tri_offset;
        n->count = node->count;
        n->center = node->aabb.center;
        n->extents = node->aabb.extents;
        n->min = (vec3){node->aabb.min_x, node->aabb.min_y, node->aabb.min_z};
        n->max = (vec3){node->aabb.max_x, node->aabb.max_y, node->aabb.max_z};
        
    }
    
    
}


void GPU_upload_scene_data()
{


    MeshGPU * mesh_data = calloc(GLOBALS.mesh_nodes.count, sizeof(MeshGPU));
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

    Vector vertices;
    init_vector(&vertices, 0, sizeof(VertexGPU));
    Vector tris;
    init_vector(&tris, 0, sizeof(TriGPU));
    Vector nodes;
    init_vector(&nodes, 0, sizeof(BVHNodeGPU));

    uint32_t vertex_offset = 0;
    uint32_t vertex_count = 0;
    uint32_t face_offset = 0;
    uint32_t face_count = 0;
    uint32_t node_offset = 0;
    uint32_t node_count = 0;

    bool * mesh_processed = calloc(GLOBALS.meshes.count, sizeof(bool));
    for (int j = 0; j < GLOBALS.scene_data.bvh_indices_count; j++)
    {
        MeshNode * mn = &mns[GLOBALS.scene_data.bvh_indices[j]];
        if (mn->mesh == -1 || mn->free) continue;
        mesh_data[j].bvh_index = GLOBALS.scene_data.bvh_indices[j];
        if (!meshes[mn->mesh].static_mesh) continue;
        if (!mesh_processed[mn->mesh])
        {
            vertex_offset = vertices.count;
            face_offset = tris.count;
            node_offset = nodes.count;

            GPU_unwrap_mesh_to_buffers(&meshes[mn->mesh], &vertices, &tris, &nodes);

            vertex_count = vertices.count - vertex_offset;
            face_count = tris.count - face_offset;
            node_count = nodes.count - node_offset;

        
            mesh_processed[mn->mesh] = true;
            for (int i = 0; i < GLOBALS.scene_data.bvh_indices_count; i++)
            {
                size_t idx = GLOBALS.scene_data.bvh_indices[i];
                if (mns[idx].mesh != mn->mesh || mns[idx].free) continue;
                mesh_data[i].vertex_offset = vertex_offset;
                mesh_data[i].vertex_count = vertex_count;
                mesh_data[i].tri_offset = face_offset;
                mesh_data[i].tri_count = face_count;
                mesh_data[i].node_offset = node_offset;
                mesh_data[i].node_count = node_count;
                
            }
        }

        mesh_data[j].mesh_node_id = j;
        mesh_data[j].mesh_id = mn->mesh;
        mesh_data[j].is_static = 1;
        mesh_data[j].global_matrix = mn->global_matrix;
        mesh_data[j].skip_directional_light = mn->skip_directional_uv_lighting;
        m4_inverse((float*)&mesh_data[j].inverse_global_matrix, (float*)&mn->global_matrix);

    }

    
    BVHNode * scene_nodes = CAST_VEC(GLOBALS.scene_data.bvh_nodes, BVHNode);
    BVHNodeGPU * snodes = calloc(GLOBALS.scene_data.bvh_nodes.count, sizeof(BVHNodeGPU));
    for (int i = 0; i < GLOBALS.scene_data.bvh_nodes.count; i++)
    {

        BVHNode * node = &scene_nodes[i];
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
        
    }
    
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, VERTEX_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(VertexGPU) * vertices.count, vertices.buffer, GL_STATIC_DRAW);
    
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, TRI_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(TriGPU) * tris.count, tris.buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MESH_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MeshGPU) * GLOBALS.mesh_nodes.count, mesh_data, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MESH_BVH_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNodeGPU) * nodes.count, nodes.buffer, GL_STATIC_DRAW);

    glBindBuffer(GL_SHADER_STORAGE_BUFFER, SCENE_BVH_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(BVHNodeGPU) * GLOBALS.scene_data.bvh_nodes.count, snodes, GL_STATIC_DRAW);

    if (MESH_DATA.meshes) free(MESH_DATA.meshes);
    MESH_DATA.meshes = mesh_data;
    if (VERTEX_DATA.vertices) free(VERTEX_DATA.vertices);
    VERTEX_DATA.vertices = vertices.buffer;
    if (TRI_DATA.tris) free(TRI_DATA.tris);
    TRI_DATA.tris = tris.buffer;
    if (MESH_BVH_DATA.nodes) free(MESH_BVH_DATA.nodes);
    MESH_BVH_DATA.nodes = nodes.buffer;
    if (SCENE_BVH_DATA.nodes) free(SCENE_BVH_DATA.nodes);
    SCENE_BVH_DATA.nodes = snodes;
   
}

void GPU_init_cluster_aabbs()
{


    glUseProgram(INIT_CLUSTER_AABBS_SHADER);
    glUniform3ui(glGetUniformLocation(INIT_CLUSTER_AABBS_SHADER, "uGridSize"), GRID_X, GRID_Y, GRID_Z);
    glUniform1f(glGetUniformLocation(INIT_CLUSTER_AABBS_SHADER, "uNear"), VIEW_NEAR);
    glUniform1f(glGetUniformLocation(INIT_CLUSTER_AABBS_SHADER, "uFar"), VIEW_FAR);
    glUniform2ui(glGetUniformLocation(INIT_CLUSTER_AABBS_SHADER, "uScreenDimensions"), RENDER_WIDTH, RENDER_HEIGHT);

    glDispatchCompute(GRID_X, GRID_Y, GRID_Z);
    glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
    
}


void init_default_textures()
{

    Image * DD = vector_expand(&GLOBALS.images, 1);
    DEFAULT_DIFFUSE = GLOBALS.images.count - 1;
    sprintf(DD->path, "DD");
    uint8_t dd[3] = {255,255,255};
    DD->buf = malloc(sizeof(uint8_t) * 3);
    DD->buf[0] = 255;
    DD->buf[1] = 255;
    DD->buf[2] = 255;
    DD->width = 1;
    DD->height = 1;
    DD->channels = 3;

    glGenTextures(1, &DD->i);
    glBindTexture(GL_TEXTURE_2D, DD->i);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &dd);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLuint64 dd_handle = glGetTextureHandleARB(DD->i);
    glMakeTextureHandleResidentARB(dd_handle);
    DD->handle = dd_handle;

    Image * DN = vector_expand(&GLOBALS.images, 1);
    DEFAULT_NORMAL = GLOBALS.images.count - 1;
    sprintf(DN->path, "DN");
    DN->channels = 3;
    DN->buf = NULL;
    uint8_t dn[3] = {128,128,255};
    DN->buf = malloc(sizeof(uint8_t) * 3);
    DN->buf[0] = 128;
    DN->buf[1] = 128;
    DN->buf[2] = 255;
    DN->width = 1;
    DN->height = 1;

    glGenTextures(1, &DN->i);
    glBindTexture(GL_TEXTURE_2D, DN->i);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &dn);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    GLuint64 dn_handle = glGetTextureHandleARB(DN->i);
    glMakeTextureHandleResidentARB(dn_handle);
    DN->handle = dn_handle;
    
    Image * DA = vector_expand(&GLOBALS.images, 1);
    DEFAULT_ARM = GLOBALS.images.count - 1;
    sprintf(DA->path, "DA");
    DA->channels = 3;
    DA->buf = NULL;
    uint8_t darm[3] = {255,255,255};
    DA->buf = malloc(sizeof(uint8_t) * 3);
    DA->buf[0] = 255;
    DA->buf[1] = 255;
    DA->buf[2] = 255;
    DA->width = 1;
    DA->height = 1;

    glGenTextures(1, &DA->i);
    glBindTexture(GL_TEXTURE_2D, DA->i);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &darm);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    GLuint64 da_handle = glGetTextureHandleARB(DA->i);
    glMakeTextureHandleResidentARB(da_handle);
    DA->handle = da_handle;

    Image * EM = vector_expand(&GLOBALS.images, 1);
    DEFAULT_EMISSIVE = GLOBALS.images.count - 1;
    sprintf(EM->path, "EM");
    EM->channels = 3;
    EM->buf = NULL;
    uint8_t dem[3] = {255,255,255};
    EM->buf = malloc(sizeof(uint8_t) * 3);
    EM->buf[0] = 255;
    EM->buf[1] = 255;
    EM->buf[2] = 255;
    EM->width = 1;
    EM->height = 1;

    glGenTextures(1, &EM->i);
    glBindTexture(GL_TEXTURE_2D, EM->i);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, &dem);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    
    GLuint64 em_handle = glGetTextureHandleARB(EM->i);
    glMakeTextureHandleResidentARB(em_handle);
    EM->handle = em_handle;

    Image * DL = vector_expand(&GLOBALS.images, 1);
    DUMMY_LIGHTMAP = GLOBALS.images.count - 1;
    sprintf(DL->path, "DL ");
    DL->channels = 4;
    DL->buf = NULL;
    vec4 dlight = (vec4){0.5f,0.5f,0.5f, 1.0};
    DL->buf = NULL;
    DL->width = 1;
    DL->height = 1;

    glGenTextures(1, &DL->i);
    glBindTexture(GL_TEXTURE_2D, DL->i);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, 1, 1, 0, GL_RGBA, GL_FLOAT, &dlight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);


    GLuint64 dl_handle = glGetTextureHandleARB(DL->i);
    glMakeTextureHandleResidentARB(dl_handle);
    DL->handle = dl_handle;
    
}

bool init_globals()
{


    init_vector(&GLOBALS.meshes, 0, sizeof(Mesh));
    MN_INIT();
    SKIN_INIT();
    init_vector(&GLOBALS.assets, 0, sizeof(Asset));
    init_vector(&GLOBALS.vertex_layouts, 0, sizeof(VertexLayout));
    init_vector(&GLOBALS.materials, 0, sizeof(Material));
    init_vector(&GLOBALS.images, 0, sizeof(Image));
    init_vector(&GLOBALS.lights, 0, sizeof(Light));
    
    init_vector(&GLOBALS.audio_portals, 0, sizeof(AudioPortal));

    init_vector(&AW.min_heap, 1000, sizeof(OpenNode));
    AW.path = malloc(sizeof(int) * MAX_PORTALS);
    AW.path_count = 0;
    AW.stack = calloc(MAX_PORTALS, sizeof(AStarStateStack));
    
    // init_vector(&GLOBALS.audio_emitters, 0, sizeof(AudioEmitter));

    GLOBALS.audio_probes = NULL;
    GLOBALS.aprobe_size = (ivec3){PROBE_VOLUME_WIDTH,PROBE_VOLUME_HEIGHT,PROBE_VOLUME_DEPTH};
    GLOBALS.msaa_samples = 4;
    LIGHTMAP_DATA.uvs = NULL;
    GLOBALS.probe_tex_size = (ivec3){PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH};

    init_default_textures();
    
    GLOBALS.scene_data.bvh_indices = NULL;
    GLOBALS.scene_data.bvh_nodes.buffer = NULL;
    GLOBALS.scene_data.leaf_indices.buffer = NULL;
    GLOBALS.scene_data.light_bvh.nodes.buffer = NULL;
    GLOBALS.scene_data.light_bvh.indices = NULL;
    GLOBALS.scene_data.light_bvh.lights.buffer = NULL;
    GLOBALS.lights_dirty = true;
    EDITOR_SETTINGS.SELMODE = SELMODE_LIGHT;
    GLOBALS.scene_data.lightmap_data.buffer = NULL;
    init_vector(&GLOBALS.scene_data.lightmap_data.islands, 0, sizeof(UVIsland));
    init_vector(&GLOBALS.scene_data.reflection_probes, 0, sizeof(ReflectionProbe));
    init_vector(&GLOBALS.scene_data.audio_probe_volumes, 0, sizeof(ProbeVolume));
    init_vector(&GLOBALS.scene_data.lighting_probe_volumes, 0, sizeof(ProbeVolume));
    memset(GLOBALS.scene_data.LOADED_AUDIO_PV, 0, sizeof(APV) * MAX_AUDIO_PROBE_VOLUMES);
    memset(GLOBALS.scene_data.LOADED_AUDIO_PV_FLAG, 0, sizeof(bool) * MAX_AUDIO_PROBE_VOLUMES);
    memset(GLOBALS.scene_data.LOADED_LIGHTING_PV, 0, sizeof(LPV) * LPV_SLOTS);
    memset(GLOBALS.scene_data.LOADED_LIGHTING_PV_FLAG, 0, sizeof(bool) * LPV_SLOTS);

    GLOBALS.scene_data.APV_FILE = NULL;
    GLOBALS.scene_data.LPV_FILE = NULL;
    

    EDITOR_SETTINGS.editor_mode = true;
    EDITOR_SETTINGS.wireframe_toggled = true;
    EDITOR_SETTINGS.texels_per_uv_unit = 1.5f;
    EDITOR_SETTINGS.mfactor = 0.001f;
    EDITOR_SETTINGS.mfactor_enabled = false;
    EDITOR_SETTINGS.lm_iterations = 2;
    EDITOR_SETTINGS.probe_iterations = 500;
    EDITOR_SETTINGS.ao_iterations = 1;
    EDITOR_SETTINGS.ao_threshold = 0.1;
    EDITOR_SETTINGS.lm_aabb.center = (vec3){0};
    EDITOR_SETTINGS.lm_aabb.extents = (vec3){0};
    EDITOR_SETTINGS.selected_r_probe = -1;
    EDITOR_SETTINGS.selected_audio_emitter_id = -1;
    EDITOR_SETTINGS.SSAO = false;
    EDITOR_SETTINGS.TAA = false;
    EDITOR_SETTINGS.MBLUR = true;
    EDITOR_SETTINGS.DOF = true;
    EDITOR_SETTINGS.SSR = false;
    EDITOR_SETTINGS.BLOOM = false;
    EDITOR_SETTINGS.MSAA = true;
    EDITOR_SETTINGS.LUT = true;
    EDITOR_SETTINGS.ACES = false;
    EDITOR_SETTINGS.NOISE = true;
    EDITOR_SETTINGS.UI = true;
    EDITOR_SETTINGS.FULL_GBUFFER = false;
    EDITOR_SETTINGS.DOF_focal_length = 0.06f;
    EDITOR_SETTINGS.DOF_fstop = 2.7f;
    EDITOR_SETTINGS.DOF_sensor_height = 0.024f;
    EDITOR_SETTINGS.DOF_max_blur_radius = 0.05f;
    EDITOR_SETTINGS.render_framerate_target = 24.0f;
    EDITOR_SETTINGS.audio_probe_iterations = 3000;
    EDITOR_SETTINGS.audio_probe_bounces = 30;
    EDITOR_SETTINGS.reverb_wet = 0.5f;
    EDITOR_SETTINGS.undo_stack_max_byte_size = 100000000;

    GLOBALS.probe_spacing = 0.5f;
    GLOBALS.aprobe_spacing = 2.0f;

    sprintf(asset_dir, "./assets/");
    sprintf(rprobe_dir, "./rprobes/");



    LIGHTMAP_INIT_SHADER = 0;
    LIGHTMAP_BOUNCE_SHADER = 0;
    LIGHTMAP_DENOISE_SHADER = 0;
    LIGHTMAP_DILATE_SHADER = 0;
    PROBE_BAKE_SHADER = 0;
    PROBE_UNPACK_SHADER = 0;
    DIRECTIONAL_MAP_NORMALIZE_SHADER = 0;
    STATIC_SHADER.program = 0;
    for (int i = 0; i < UBERSHADER_VERSIONS; i++)
    {
        DYNAMIC_SHADER[i].program = 0;
        
    }
    SHADOW_MAP_SHADER.program = 0;
    WIREFRAME_SHADER.program = 0;
    for (int i = 0; i < 64; i++)
    {
        POSTPROCESS_SHADER[i].program = 0;
        
    }
    SKY_SHADER.program = 0;
    LIGHTMAP_SHADER.program = 0;
    for (int i = 0; i < GBUFFER_VERSIONS; i++)
    {
        DEPTH_PRE_PASS_SHADER[i].program = 0;
        
    }
    POINT_LIGHT_SHADOW_SHADER.program = 0;
    SSAO_SHADER.program = 0;
    SSAO_BLUR_SHADER.program = 0;
    SSR_SHADER.program = 0;
    SSR_BLUR_SHADER.program = 0;
    TEXT_SHADER.program = 0;
    

    MESH_DATA.meshes = NULL;
    VERTEX_DATA.vertices = NULL;
    TRI_DATA.tris = NULL;
    MESH_BVH_DATA.nodes = NULL;
    SCENE_BVH_DATA.nodes = NULL;

    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        ENTITIES[i].id = i;
        ENTITIES[i].free = true;
    }
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++)
    {
        ASSET_INSTANCES[i].id = i;
        ASSET_INSTANCES[i].free = true;
        
    }

    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        PHYSICS_COMPONENTS[i].id = i;
        PHYSICS_COMPONENTS[i].free = true;
        PHYSICS_COMPONENTS[i].at_rest = false;
    }
    for (int i = 0; i < MAX_ANIMATION_COMPONENTS; i++)
    {
        ANIMATION_COMPONENTS[i].free = true;
        ANIMATION_COMPONENTS[i].track_count = 0;
        for (int j = 0; j < MAX_TRACKS; j++)
        {
            
            ANIMATION_COMPONENTS[i].tracks[j].data = NULL;
            ANIMATION_COMPONENTS[i].tracks[j].completion_data = NULL;
        }
    }
    for (int i = 0; i < MAX_MANIFOLDS; i++)
    {
        MANIFOLDS[i].a = NULL;
        MANIFOLDS[i].b = NULL;
        MANIFOLDS[i].free = true;
    }
    for (int i = 0; i < MAX_AUDIO_EMITTERS; i++)
    {
        AUDIO_EMITTERS[i].free = true;
        AUDIO_EMITTERS[i].id = i;
    }

    EW[0] = malloc(sizeof(EPAWorkspace));
    GLOBALS.mesh_light_indices = malloc(sizeof(uint32_t) * MAX_MESH_NODES * MAX_LIGHTS_PER_MESH);



    
    Light * lights = vector_expand(&GLOBALS.lights, 2 );


    lights[0].light_type = DIRECTIONAL_LIGHT;
    lights[0].rgb = (vec3){0.7, 0.6, 0.6};
    lights[0].pos = (vec3){0, 4, 2};
    lights[0].dir = (vec3){0.0, 0.0, 1.0};
    lights[0].dir = vec3_normalize(lights[0].dir);
    lights[0].intensity = 1;
    Entity * l1 = create_entity(NULL, -1, false, true);
    lights[0].entity_id = l1->id;
    lights[0].threshold = 0.005f;


    
         
    lights[1].light_type = POINT_LIGHT;
    lights[1].rgb = (vec3){0.77, 0.66, 0.45};
    lights[1].pos = (vec3){0, 0, 2.0};
    lights[1].intensity = 20;
    Entity * l3 = create_entity(NULL, -1, false, true);
    lights[1].entity_id = l3->id;
    lights[1].threshold = 0.001f;
    lights[1].sm_dirty = true;
    


    Font * font = read_json_msdf("../engine-assets/fonts/MxPlus_IBM_VGA_8x16.json", "../engine-assets/fonts/MyFont.png");
    if (font)
    {
        
        font->texture = add_image_to_registry(
            font->name,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            false);
        font->image_id = GLOBALS.images.count-1;
    }

    
    Material * default_mat = init_new_material("default_mat");
    default_mat->diffuse_tex = DEFAULT_DIFFUSE;
    default_mat->normal_tex = DEFAULT_NORMAL;
    default_mat->arm_tex = DEFAULT_ARM;
    default_mat->emissive_tex = DEFAULT_EMISSIVE;
    default_mat->fwd_shader = 0;
    default_mat->gbuffer_shader = 0;
    default_mat->mr_tex = -1;
    for (int i = 0; i < AUDIO_NUM_BANDS; i++)
    {
        default_mat->absorption_coefficients[i] = 0.05f;
        default_mat->scattering_coefficients[i] = 0.05f;
    }
    
    
    return true;
    
}



bool init_vertex_layouts()
{
    

    VertexLayout * vl = vector_expand(&GLOBALS.vertex_layouts, 1);

    vl->current_offset = 0;
    vl->current_capacity = 0;
    vl->current_index_offset = 0;
    vl->current_index_capacity = 0;
    init_vector(&vl->meshes, 0, sizeof(size_t));
    vl->vao = MAIN_VAO;
    vl->vbo = MAIN_VBO;
    vl->ebo = MAIN_EBO;
    

    return true;
    
}


vec3 * fbm_tex = NULL;


void
    precompute_fbm_texture ()
    
{
    
    fbm_tex = calloc(FBM_TEXTURE_SIZE * FBM_TEXTURE_SIZE, sizeof(vec3));

    for (int y = 0; y < FBM_TEXTURE_SIZE; y++)
    {
    
        for (int x = 0; x < FBM_TEXTURE_SIZE; x++)
        {
            vec3 p = {(float)x, (float)y, 0};
            float d = fbm(p);

            fbm_tex[y*FBM_TEXTURE_SIZE+x].x = (d + 1) * 0.5f;

            float d1 = fbm(vec3_scale(p, 0.1f));
            fbm_tex[y*FBM_TEXTURE_SIZE+x].y = (d1 + 1) * 0.5f;

            float d2 = fbm(vec3_scale(p, 0.05f));
            fbm_tex[y*FBM_TEXTURE_SIZE+x].z = (d2 + 1) * 0.5f;

            
        }
    }
    
    glBindTexture(GL_TEXTURE_2D, FBM_TEXTURE);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, FBM_TEXTURE_SIZE, FBM_TEXTURE_SIZE, GL_RGB, GL_FLOAT, fbm_tex);
    free(fbm_tex);
    fbm_tex = NULL;
    
}



void precompute_ssao_kernel()
{
    

    for (int i = 0; i < 16; i++)
    {
        vec3 sample =
        {
            ((float)rand() / RAND_MAX) * 2.0f - 1.0f,
            ((float)rand() / RAND_MAX) * 2.0f - 1.0f,
            ((float)rand() / RAND_MAX)
        };

        sample = vec3_normalize(sample);

        float r = (float)rand() / RAND_MAX;
        sample = vec3_scale(sample, r);

        float scale = (float)i / 16;
        scale = lerp(0.1, 1.0f, scale * scale);
        sample = vec3_scale(sample, scale);
        SSAO_KERNEL[i] = vec3_to_vec4(sample);
    }
}
int generate_gl_texture(GLTexture tex)
{
    GLuint i = 0;
    glGenTextures(1, &i);
    glBindTexture(tex.target, i);
    if (tex.target == GL_TEXTURE_CUBE_MAP_ARRAY)
    {
        glTexStorage3D(tex.target, tex.levels, tex.internal_format, tex.w, tex.h, tex.d);
    }
    else if (tex.target == GL_TEXTURE_2D_ARRAY)
    {
        
        glTexImage3D(tex.target, 0, tex.internal_format, tex.w, tex.h, tex.d, 0, tex.format, tex.type, NULL);
    }
    else if (tex.target == GL_TEXTURE_2D_MULTISAMPLE)
    {
        
        glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, tex.samples, tex.internal_format, tex.w, tex.h, tex.fixed_sample_locations);
    }
    else
    {
        
        glTexImage2D(tex.target, 0, tex.internal_format, tex.w, tex.h, 0, tex.format, tex.type, NULL);
    }
    glTexParameteri(tex.target, GL_TEXTURE_MIN_FILTER, tex.min_filter);
    glTexParameteri(tex.target, GL_TEXTURE_MAG_FILTER, tex.mag_filter);
    glGenerateMipmap(tex.target);
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_S, tex.wrap_s);
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_T, tex.wrap_t);
    glTexParameteri(tex.target, GL_TEXTURE_WRAP_R, tex.wrap_r);

    switch(tex.dm)
    {
        case DEPTH_MODE_NONE:
            break;
        case DEPTH_MODE_NO_HW:
            glTexParameteri(tex.target, GL_TEXTURE_COMPARE_MODE, GL_NONE);
            break;
        case DEPTH_MODE_HW:
            glTexParameteri(tex.target, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(tex.target, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);
            break;
    }
    return i;
}
GLTexture init_gl_texture(GLenum target, GLint internal_format, GLenum format, GLenum type, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, GLint wrap_r, int w, int h, int d, int levels, DepthMode dm)
{
    
    GLTexture tex;
    tex.target = target;
    tex.internal_format = internal_format;
    tex.format = format;
    tex.type = type;
    tex.min_filter = min_filter;
    tex.mag_filter = mag_filter;
    tex.wrap_s = wrap_s;
    tex.wrap_t = wrap_t;
    tex.wrap_r = wrap_r;
    tex.w = w;
    tex.h = h;
    tex.d = d;
    tex.levels = levels;
    tex.dm = dm;
    if (tex.target == GL_TEXTURE_2D_MULTISAMPLE)
    {
        tex.multisampled = true;
        tex.fixed_sample_locations = GL_TRUE;
        tex.samples = 4;
    }

    tex.i = generate_gl_texture(tex);
    
    return tex;
}

void init_fbo(Framebuffer * fbo)
{
    
    glGenFramebuffers(1, &fbo->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo->fbo);
    GLenum drawBuffers[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4,  GL_COLOR_ATTACHMENT5};
    for (int i = 0; i < fbo->texture_count; i++)
    {
        
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, fbo->texture[i].target, fbo->texture[i].i, 0);
    }
    for (int i = fbo->texture_count; i < fbo->rbo_count; i++)
    {
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_RENDERBUFFER, fbo->renderbuffers[i - fbo->texture_count].i);
    }
    glDrawBuffers(fbo->texture_count + fbo->rbo_count, drawBuffers);
    if (fbo->depth_rbo.i != 0)
    {
        
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, fbo->depth_rbo.i);
    }
    else
    {
        if (fbo->depth_texture.target == GL_TEXTURE_CUBE_MAP_ARRAY)
        {
        
            glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->depth_texture.i, 0);
        }
        else
        {
        
            glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, fbo->depth_texture.target, fbo->depth_texture.i, 0);
        }
        
    }
    if (fbo->texture_count == 0 && fbo->rbo_count == 0)
    {
        
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
    }
}

void change_fbo_resolution(Framebuffer * fbo, int w, int h, bool skip_depth)
{
    glDeleteFramebuffers(1, &fbo->fbo);
    fbo->w = w;
    fbo->h = h;
    for (int i = 0; i < fbo->texture_count; i++)
    {
        glDeleteTextures(1, &fbo->texture[i].i);
        fbo->texture[i].w = w;
        fbo->texture[i].h = h;
        fbo->texture[i].i = generate_gl_texture(fbo->texture[i]);
    }
    
    if (!skip_depth && fbo->depth_texture.i > 0)
    {
        glDeleteTextures(1, &fbo->depth_texture.i);
        fbo->depth_texture.w = w;
        fbo->depth_texture.h = h;
        fbo->depth_texture.i = generate_gl_texture(fbo->depth_texture);
    }

    init_fbo(fbo);
    
}

int estimated_frame_count = 0;
void init_sdl_audio()
{
    
    SDL_AudioSpec desired, obtained;
    SDL_zero(desired);
    desired.freq = 48000;
    desired.format = AUDIO_F32SYS; 
    desired.channels = 2;
    desired.samples = 512;
    desired.callback = SDLAudioCallback;
    desired.userdata = &AUDIO_ENGINE; 

    SDL_AUDIO_DEVICE = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0);
    estimated_frame_count = obtained.samples;

    SDL_PauseAudioDevice(SDL_AUDIO_DEVICE, 0);
}

#define CROSSFADE_FRAMES 512

// we found out that the aliasing bug is actually due to the hdmi cable probably since it sounds clean plugged in raw
// write ma_mutex
static void convolver_node_process_pcm_frames(ma_node* pNode, const float** ppFramesIn, ma_uint32* pFrameCountIn, float** ppFramesOut, ma_uint32* pFrameCountOut)
{

    ConvolverNode * node = (ConvolverNode *)pNode;
    
    const float* pFramesIn_0 = ppFramesIn[0]; 
    const float* pFramesIn_1 = ppFramesIn[1]; 
    float* pFramesOut_0 = ppFramesOut[0];     
    int frame_count = *pFrameCountOut;

    PartitionTable * front = NULL;
    bool crossfade_initialized = false;
    SDL_AtomicLock(&HRTF_TD.ir_swap_lock);
    {
        front = HRTF_TD.FRONT;
        if (CONVOLVER_NODE.prev_ir == NULL)
        {
            CONVOLVER_NODE.prev_ir = front;
            CONVOLVER_NODE.ir = NULL;
            CONVOLVER_NODE.crossfading = false;
        }
        if ((front != CONVOLVER_NODE.prev_ir) && front != NULL && !CONVOLVER_NODE.crossfading)
        {
            crossfade_initialized = true;
            CONVOLVER_NODE.ir = front;
        }
    }
    SDL_AtomicUnlock(&HRTF_TD.ir_swap_lock);

    if (crossfade_initialized)
    {
        
        CONVOLVER_NODE.crossfade_pos = 0;
        CONVOLVER_NODE.crossfading = true;


        memset(CONVOLVER_NODE.SUM_L, 0, sizeof(fftwf_complex) * CONVOLVER_NODE.complex_len);
        memset(CONVOLVER_NODE.SUM_R, 0, sizeof(fftwf_complex) * CONVOLVER_NODE.complex_len);
        memset(CONVOLVER_NODE.LEFT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);
        memset(CONVOLVER_NODE.RIGHT_OUT, 0, sizeof(float) * CONVOLVER_NODE.padded_len);


        for (int i = 0; i < NUM_PARTITIONS; i++)
        {

            int history_idx = (node->history_head - i + NUM_PARTITIONS_MAX) % NUM_PARTITIONS_MAX;

            fftwf_complex * X = node->AUDIO_HISTORY[history_idx];
            fftwf_complex * prev_ir_left = (*node->ir)[0][i];
            fftwf_complex * prev_ir_right = (*node->ir)[1][i];

            for (int j = 0; j < node->complex_len; j++)
            {

                node->SUM_L[j][0] += X[j][0] * prev_ir_left[j][0] - X[j][1] * prev_ir_left[j][1];
                node->SUM_L[j][1] += X[j][0] * prev_ir_left[j][1] + X[j][1] * prev_ir_left[j][0];
        
                node->SUM_R[j][0] += X[j][0] * prev_ir_right[j][0] - X[j][1] * prev_ir_right[j][1];
                node->SUM_R[j][1] += X[j][0] * prev_ir_right[j][1] + X[j][1] * prev_ir_right[j][0];

            }

        }
        

        fftwf_execute_dft_c2r(node->bwd_plan, node->SUM_L, node->LEFT_OUT);
        fftwf_execute_dft_c2r(node->bwd_plan, node->SUM_R, node->RIGHT_OUT);

        


        memcpy(CONVOLVER_NODE.TAIL_LEFT,  &node->LEFT_OUT[node->block_size], node->tail_len * sizeof(float));
        memcpy(CONVOLVER_NODE.TAIL_RIGHT, &node->RIGHT_OUT[node->block_size], node->tail_len * sizeof(float));
        float scale = 1.0f / node->padded_len;
        for (int i = 0; i < node->tail_len; i++)
        {
            CONVOLVER_NODE.TAIL_LEFT[i] *= scale;
            CONVOLVER_NODE.TAIL_RIGHT[i] *= scale;
        }
    }

    if (CONVOLVER_NODE.prev_ir == NULL)
    {

        for (int i = 0; i < node->block_size; i++)
        {
            pFramesOut_0[i * 2 + 0] = pFramesIn_0[i * 2 + 0] * EDITOR_SETTINGS.volume;
            pFramesOut_0[i * 2 + 1] = pFramesIn_0[i * 2 + 1] * EDITOR_SETTINGS.volume;
        }
        return;
        
    }    

        
    for (int i = 0; i < node->block_size; i++)
    {

        node->LEFT_OUT[i] = (pFramesIn_0[i * 2 + 0] + pFramesIn_0[i * 2 + 1]) * 0.5f;

    }
    memset(&CONVOLVER_NODE.LEFT_OUT[node->block_size], 0, sizeof(float) * (CONVOLVER_NODE.padded_len - node->block_size));

    memset(node->PREV_SUM_L, 0, sizeof(fftwf_complex) * node->complex_len);
    memset(node->PREV_SUM_R, 0, sizeof(fftwf_complex) * node->complex_len);

    if (CONVOLVER_NODE.crossfading)
    {
        
        memset(node->SUM_L, 0, sizeof(fftwf_complex) * node->complex_len);
        memset(node->SUM_R, 0, sizeof(fftwf_complex) * node->complex_len);

    }

    fft_channels(node, node->block_size, CONVOLVER_NODE.crossfading);
    
    float norm = 1.0f / node->padded_len;
    for (int i = 0; i < node->block_size; i++)
    {
        
        float alpha = (float)CONVOLVER_NODE.crossfade_pos / CROSSFADE_FRAMES;
        if (alpha > 1.0f) alpha = 1.0f;
        float gain_cur = 1.0f-alpha;
        float gain_target = alpha;

        float interp_l = 0.0f;
        float interp_r = 0.0f;
        if (CONVOLVER_NODE.crossfading)
        {
             

            interp_l = ((node->LEFT_OUT[i] * norm + node->TAIL_LEFT[i]) * gain_target + (node->PREV_LEFT_OUT[i] * norm + node->PREV_TAIL_LEFT[i]) * gain_cur);
            interp_r = ((node->RIGHT_OUT[i] * norm + node->TAIL_RIGHT[i]) * gain_target + (node->PREV_RIGHT_OUT[i] * norm + node->PREV_TAIL_RIGHT[i]) * gain_cur);

        }
        else
        {
            
            interp_l = (node->PREV_LEFT_OUT[i] * norm + node->PREV_TAIL_LEFT[i]);
            interp_r = (node->PREV_RIGHT_OUT[i] * norm + node->PREV_TAIL_RIGHT[i]);

        }
        
        pFramesOut_0[i * 2 + 0] = pFramesIn_0[i * 2 + 0] * EDITOR_SETTINGS.volume + interp_l * EDITOR_SETTINGS.reverb_wet;
        pFramesOut_0[i * 2 + 1] = pFramesIn_0[i * 2 + 1] * EDITOR_SETTINGS.volume + interp_r * EDITOR_SETTINGS.reverb_wet;

        if (CONVOLVER_NODE.crossfading)
        {
             
            node->TAIL_LEFT[i] = (node->LEFT_OUT[i + node->block_size] * norm);
            node->TAIL_RIGHT[i] = (node->RIGHT_OUT[i + node->block_size] * norm);

        }
        // since we only crossfade one frame this is fine to optimize since we will always be swapping to tail left / right instead of keeping this one anyway
        node->PREV_TAIL_LEFT[i] = (node->PREV_LEFT_OUT[i + node->block_size] * norm);
        node->PREV_TAIL_RIGHT[i] = (node->PREV_RIGHT_OUT[i + node->block_size] * norm);

        
        if (CONVOLVER_NODE.crossfading) CONVOLVER_NODE.crossfade_pos++;
    }


    if (CONVOLVER_NODE.crossfading && CONVOLVER_NODE.crossfade_pos >= CROSSFADE_FRAMES)
    {
        
        SDL_AtomicLock(&HRTF_TD.ir_swap_lock);
        {
            CONVOLVER_NODE.prev_ir = CONVOLVER_NODE.ir;
            CONVOLVER_NODE.ir = NULL;
        }
        SDL_AtomicUnlock(&HRTF_TD.ir_swap_lock);
        memcpy(node->PREV_TAIL_LEFT, node->TAIL_LEFT, sizeof(float) * node->tail_len);
        memcpy(node->PREV_TAIL_RIGHT, node->TAIL_RIGHT, sizeof(float) * node->tail_len);
        CONVOLVER_NODE.crossfading = false;
        CONVOLVER_NODE.crossfade_pos = 0;

    }
    
}





static ma_node_vtable my_custom_node_vtable =
{
    convolver_node_process_pcm_frames, 
    NULL,  
    2,     
    1,     
    0      
};

void init_audio_scratch_buffers()
{
    

    int tail_len = 512;
    int padded_size = 1024;
    int complex_len = (padded_size / 2) + 1;

    AUDIO_SCRATCH.aprobe_mutex = SDL_CreateMutex();
    
    AUDIO_SCRATCH.block_size = tail_len;
    AUDIO_SCRATCH.padded_len = padded_size;
    AUDIO_SCRATCH.complex_len = complex_len;

    AUDIO_SCRATCH.fwd_plan = fftwf_plan_dft_r2c_1d(padded_size, NULL, NULL, FFTW_ESTIMATE);
    AUDIO_SCRATCH.bwd_plan = fftwf_plan_dft_c2r_1d(padded_size, NULL, NULL, FFTW_ESTIMATE);

    AUDIO_SCRATCH.HRTF_IR_IN = fftwf_malloc(sizeof(float) * padded_size);
    memset(AUDIO_SCRATCH.HRTF_IR_IN, 0, sizeof(float) * padded_size);

    CONVOLVER_NODE.tail_len = tail_len;
    CONVOLVER_NODE.PREV_TAIL_LEFT = calloc(tail_len, sizeof(float));
    CONVOLVER_NODE.PREV_TAIL_RIGHT = calloc(tail_len, sizeof(float));
    CONVOLVER_NODE.TAIL_LEFT = calloc(tail_len, sizeof(float));
    CONVOLVER_NODE.TAIL_RIGHT = calloc(tail_len, sizeof(float));

    CONVOLVER_NODE.block_size = tail_len;
    CONVOLVER_NODE.padded_len = padded_size;
    CONVOLVER_NODE.complex_len = complex_len;

    
    CONVOLVER_NODE.fwd_plan = fftwf_plan_dft_r2c_1d(padded_size, NULL, NULL, FFTW_ESTIMATE);
    CONVOLVER_NODE.bwd_plan = fftwf_plan_dft_c2r_1d(padded_size, NULL, NULL, FFTW_ESTIMATE);
    
    CONVOLVER_NODE.MONO_FWD = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
    CONVOLVER_NODE.PREV_SUM_L = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
    CONVOLVER_NODE.PREV_SUM_R = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
    CONVOLVER_NODE.SUM_L = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
    CONVOLVER_NODE.SUM_R = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
    for (int i = 0; i < NUM_PARTITIONS_MAX; i++)
    {
        
        CONVOLVER_NODE.AUDIO_HISTORY[i] = fftwf_malloc(sizeof(fftwf_complex) * complex_len);
        memset(CONVOLVER_NODE.AUDIO_HISTORY[i], 0, sizeof(fftwf_complex) * complex_len);
    }

    CONVOLVER_NODE.PREV_LEFT_OUT = fftwf_malloc(sizeof(float) * padded_size);
    CONVOLVER_NODE.PREV_RIGHT_OUT = fftwf_malloc(sizeof(float) * padded_size);
    CONVOLVER_NODE.LEFT_OUT = fftwf_malloc(sizeof(float) * padded_size);
    CONVOLVER_NODE.RIGHT_OUT = fftwf_malloc(sizeof(float) * padded_size);
    CONVOLVER_NODE.ir = NULL;
    CONVOLVER_NODE.prev_ir = NULL;

    memset(CONVOLVER_NODE.MONO_FWD, 0, sizeof(fftwf_complex) * complex_len);
    memset(CONVOLVER_NODE.PREV_SUM_L, 0, sizeof(fftwf_complex) * complex_len);
    memset(CONVOLVER_NODE.PREV_SUM_R, 0, sizeof(fftwf_complex) * complex_len);
    memset(CONVOLVER_NODE.SUM_L, 0, sizeof(fftwf_complex) * complex_len);
    memset(CONVOLVER_NODE.SUM_R, 0, sizeof(fftwf_complex) * complex_len);
    memset(CONVOLVER_NODE.PREV_LEFT_OUT, 0, sizeof(float) * padded_size);
    memset(CONVOLVER_NODE.PREV_RIGHT_OUT, 0, sizeof(float) * padded_size);
    memset(CONVOLVER_NODE.LEFT_OUT, 0, sizeof(float) * padded_size);
    memset(CONVOLVER_NODE.RIGHT_OUT, 0, sizeof(float) * padded_size);

    CONVOLVER_NODE.history_head = 0;

    init_hrtf_worker_thread(&HRTF_TD);
    
    
}

void deallocate_fbo(Framebuffer * fbo)
{
    
    for (int i = 0; i < fbo->texture_count; i++)
    {

        glDeleteTextures(1, &fbo->texture[i].i);
        
    }
    for (int i = 0; i < fbo->rbo_count; i++)
    {

        glDeleteRenderbuffers(1, &fbo->renderbuffers[i].i);
        
    }
    glDeleteTextures(1, &fbo->depth_texture.i);
    glDeleteRenderbuffers(1, &fbo->depth_rbo.i);
    glDeleteFramebuffers(1, &fbo->fbo);

    memset(fbo, 0, sizeof(Framebuffer));
    
}

void init_gl_renderbuffer(GLRenderbuffer * rbo, GLint internal_format, GLsizei w, GLsizei h, bool multisampled, int samples)
{
    

    glGenRenderbuffers(1, &rbo->i);
    glBindRenderbuffer(GL_RENDERBUFFER, rbo->i);
    rbo->multisampled = multisampled;
    rbo->samples = samples;
    rbo->w = w;
    rbo->h = h;
    rbo->internal_format = internal_format;
    if (rbo->multisampled)
    {
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, internal_format, w, h); 
    }
}

void allocate_main_fbo()
{

    deallocate_fbo(&MAIN_FBO);
    deallocate_fbo(&MAIN_RESOLVE_FBO);


    MAIN_FBO.w = RENDER_WIDTH;
    MAIN_FBO.h = RENDER_HEIGHT;
    MAIN_FBO.rbo_count = 1;
    init_gl_renderbuffer
        (
            &MAIN_FBO.renderbuffers[0],
            GL_RGBA16F,
            MAIN_FBO.w,
            MAIN_FBO.h,
            GL_TRUE,
            GLOBALS.msaa_samples
        );
    init_gl_renderbuffer
        (
            &MAIN_FBO.depth_rbo,
            GL_DEPTH_COMPONENT24,
            MAIN_FBO.w,
            MAIN_FBO.h,
            GL_TRUE,
            GLOBALS.msaa_samples
        );
    
    init_fbo(&MAIN_FBO);


    MAIN_RESOLVE_FBO.w = RENDER_WIDTH;
    MAIN_RESOLVE_FBO.h = RENDER_HEIGHT;
    MAIN_RESOLVE_FBO.texture_count = 1;

    MAIN_RESOLVE_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            MAIN_RESOLVE_FBO.w,
            MAIN_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    init_fbo(&MAIN_RESOLVE_FBO);
    
}

void allocate_gbuffer_msaa()
{

    deallocate_fbo(&GBUFFER_FBO);
    deallocate_fbo(&GBUFFER_RESOLVE_FBO);
    
    GBUFFER_FBO.w = RENDER_WIDTH;
    GBUFFER_FBO.h = RENDER_HEIGHT;
    GBUFFER_FBO.rbo_count = 1;
    
    init_gl_renderbuffer
        (
            &GBUFFER_FBO.renderbuffers[GVEL],
            GL_RG32F,
            GBUFFER_FBO.w,
            GBUFFER_FBO.h,
            GL_TRUE,
            GLOBALS.msaa_samples
        );
    init_gl_renderbuffer
        (
            &GBUFFER_FBO.depth_rbo,
            GL_DEPTH_COMPONENT24,
            GBUFFER_FBO.w,
            GBUFFER_FBO.h,
            GL_TRUE,
            GLOBALS.msaa_samples
        );

    init_fbo(&GBUFFER_FBO);

    GBUFFER_RESOLVE_FBO.w = RENDER_WIDTH;
    GBUFFER_RESOLVE_FBO.h = RENDER_HEIGHT;
    GBUFFER_RESOLVE_FBO.texture_count = 1;
    GBUFFER_RESOLVE_FBO.texture[GVEL] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RG32F,
            GL_RG,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    GBUFFER_RESOLVE_FBO.depth_texture =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_DEPTH_COMPONENT24,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NO_HW
        );
    init_fbo(&GBUFFER_RESOLVE_FBO);
    
}

void allocate_gbuffer()
{
    
    deallocate_fbo(&GBUFFER_FBO);
    deallocate_fbo(&GBUFFER_RESOLVE_FBO);
    
    GBUFFER_RESOLVE_FBO.w = RENDER_WIDTH;
    GBUFFER_RESOLVE_FBO.h = RENDER_HEIGHT;
    GBUFFER_RESOLVE_FBO.texture_count = 6;
    GBUFFER_RESOLVE_FBO.texture[GNORMAL] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    GBUFFER_RESOLVE_FBO.texture[GALBEDO] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA8,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    GBUFFER_RESOLVE_FBO.texture[GEXTRA] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA8,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    GBUFFER_RESOLVE_FBO.texture[GVEL] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RG32F,
            GL_RG,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    GBUFFER_RESOLVE_FBO.texture[GPOMDEPTH] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_R32F,
            GL_RED,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    GBUFFER_RESOLVE_FBO.texture[GUV] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RG16F,
            GL_RG,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    
    GBUFFER_RESOLVE_FBO.depth_texture =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_DEPTH_COMPONENT24,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            GBUFFER_RESOLVE_FBO.w,
            GBUFFER_RESOLVE_FBO.h,
            0,
            0,
            DEPTH_MODE_NO_HW
        );

    init_fbo(&GBUFFER_RESOLVE_FBO);
}


bool
    init
    (
        SDL_Window    ** window,
        SDL_GLContext *  context
    )

{

    printf("Initializing\n");
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
        printf("SDL Init failed: %s\n", SDL_GetError());
        return false;
    }
    
    #ifdef INCLUDE_CONFIGURATOR
    static struct nk_color color_table[NK_COLOR_COUNT];
    memcpy(color_table, nk_default_color_style, sizeof(color_table));
    #endif

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 4);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);


    *window = SDL_CreateWindow("Untitled", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_OPENGL);
    SDL_SetWindowGrab(*window, SDL_TRUE);
    SDL_SetRelativeMouseMode(SDL_TRUE);
    if (!*window) {
        printf("Window creation failed: %s\n", SDL_GetError());
        return false;
    }

    *context = SDL_GL_CreateContext(*window);
    if (!*context) {
        printf("GL Context failed: %s\n", SDL_GetError());
        return false;
    }

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf("GLEW Init failed\n");
        return false;
    }

    NK_CTX = nk_sdl_init(*window);
    if (!NK_CTX)
    {
        printf("NUKLEAR INIT FAILED\n");
        return false;
    }

    glEnable(GL_MULTISAMPLE);
    SDL_GL_SetSwapInterval(0);
    HRTF_TD.ir_swap_lock = 0;

    UNDO_INIT();
    UI_INIT();
    FONTS_INIT();
    STATES_INIT();

    init_band_kernels(48000, 125.0f, 250.0f, 500.0f, 1000.0f, 2000.0f, BAND_IMPULSES);

    ma_engine_config ENGINE_CONFIG;

    ENGINE_CONFIG = ma_engine_config_init();

    ENGINE_CONFIG.noDevice = MA_TRUE; 

    ENGINE_CONFIG.channels = 2;
    ENGINE_CONFIG.sampleRate = 48000;
    ENGINE_CONFIG.periodSizeInFrames = 512;     
    ma_result result = ma_engine_init(&ENGINE_CONFIG, &AUDIO_ENGINE);
    if (result != MA_SUCCESS) {
        printf("AUDIO ENGINE INIT FAILED\n");
        return false;
    }    

    // order is important here. make sure to init bufferrs before attaching the node?

    init_sdl_audio();

    ma_uint32 input_channels[1] = {2};
    ma_uint32 output_channels[1] = {2};


    init_audio_scratch_buffers();

    load_hrtf_from_binary("./HRTF.bin");

    ma_node_config node_config = ma_node_config_init();
    
    node_config.vtable = &my_custom_node_vtable;
    node_config.pInputChannels = input_channels;
    node_config.pOutputChannels = output_channels;

    ma_result init_res = ma_node_init(ma_engine_get_node_graph(&AUDIO_ENGINE), &node_config, NULL, &CONVOLVER_NODE.base);
    if (init_res != MA_SUCCESS)
    {
        printf("INIT FAILED\n");
    }
    
    ma_result at_res = ma_node_attach_output_bus(&CONVOLVER_NODE, 0, ma_engine_get_endpoint(&AUDIO_ENGINE), 0);
    if (at_res != MA_SUCCESS)
    {
        printf("AT FAILED\n");
    }

    ma_result mut_res = ma_mutex_init(&CONVOLVER_NODE.lock);
    if (mut_res != MA_SUCCESS)
    {
        printf("AUDIO MUTEX FAILED\n");
    }
    

    if (!init_globals())
    {
        printf("GLOBALS FAILED TO INIT\n");
        return false;
        
    }
    init_action_callbacks();
    init_hover_callbacks();
    
    init_player();

    init_file_directory_registries();

    
    SDL_GL_SetSwapInterval(0);
    {struct nk_font_atlas *atlas;
        nk_sdl_font_stash_begin(&atlas);
        nk_sdl_font_stash_end();
    }

    #ifdef INCLUDE_STYLE
    #ifdef STYLE_WHITE
    set_style(ctx, THEME_WHITE);
    #elif defined(STYLE_RED)
    set_style(ctx, THEME_RED);
    #elif defined(STYLE_BLUE)
    set_style(ctx, THEME_BLUE);
    #elif defined(STYLE_DARK)
    set_style(ctx, THEME_DARK);
    #endif
    #endif

    // glEnable(GL_MULTISAMPLE);
    
    struct nk_color transparent = nk_rgba(30, 30, 30, 90);
    struct nk_color border = nk_rgba(60, 60, 60, 150);
    struct nk_color text = nk_rgba(255, 255, 255, 255);

    NK_CTX->style.window.fixed_background = nk_style_item_color(transparent);
    NK_CTX->style.window.border_color = border;
    NK_CTX->style.window.padding = nk_vec2(10, 10);
    NK_CTX->style.window.border = 1;

    NK_CTX->style.text.color = text;

    

    SDL_LockAudioDevice(SDL_AUDIO_DEVICE);
    
    AudioEmitter * emitter = init_emitter("./sounds/byzantine columns 5.mp3", true, true, 0.5);
    play_emitter(emitter, (vec3){3,5,2});
    AudioEmitter * fs_emitter = init_emitter("./sounds/footstep_01.wav", false, false, 0.5);
    FOOTSTEP_EMITTER = fs_emitter;
    
    SDL_UnlockAudioDevice(SDL_AUDIO_DEVICE);

    
    
    allocate_main_fbo();

    TEXT_FBO.w = TEXT_WIDTH;
    TEXT_FBO.h = TEXT_HEIGHT;
    TEXT_FBO.texture_count = 1;
    TEXT_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA8,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            TEXT_FBO.w,
            TEXT_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    
    init_fbo(&TEXT_FBO);
    
    glGenTextures(1, &LIGHTMAP_POS_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_POS_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &LIGHTMAP_NORM_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_NORM_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenTextures(1, &LIGHTMAP_ID_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32I, LIGHTMAP_WIDTH, LIGHTMAP_HEIGHT, 0, GL_RGB_INTEGER, GL_INT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    
    glGenFramebuffers(1, &LIGHTMAP_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, LIGHTMAP_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, LIGHTMAP_POS_TEXTURE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, LIGHTMAP_NORM_TEXTURE, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, LIGHTMAP_ID_TEXTURE, 0);

    GLenum draw_buffers[] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2};
    glDrawBuffers(3, draw_buffers);
     

    glGenTextures(1, &BRDF_LUT);
    glBindTexture(GL_TEXTURE_2D, BRDF_LUT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG16F, 512, 512, 0, GL_RG, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenFramebuffers(1, &BRDF_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, BRDF_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, BRDF_LUT, 0);


    
    glGenTextures(1, &LPV_TEXTURE);
    glBindTexture(GL_TEXTURE_3D, LPV_TEXTURE);
    glTexStorage3D(GL_TEXTURE_3D, 1, GL_RGB32F, PROBE_VOLUME_WIDTH, PROBE_VOLUME_HEIGHT, PROBE_VOLUME_DEPTH_PADDED * SH_MAX * LPV_SLOTS);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);


    glGenTextures(1, &RPROBE_CUBE_MAP_ARRAY);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, RPROBE_CUBE_MAP_ARRAY);
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, MAX_RPROBE_MIPS, GL_RGBA16F, RPROBE_WIDTH, RPROBE_HEIGHT, 6);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &RPROBE_DEPTH_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, RPROBE_DEPTH_TEXTURE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, RPROBE_WIDTH, RPROBE_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);

    glGenFramebuffers(1, &RPROBE_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, RPROBE_FBO);
    glFramebufferTexture3D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_CUBE_MAP_ARRAY, RPROBE_CUBE_MAP_ARRAY, 0, 0);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, RPROBE_DEPTH_TEXTURE, 0);


    glGenTextures(1, &TEMP_RPROBE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, TEMP_RPROBE_CUBE_MAP);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, 1, GL_RGBA16F, RPROBE_WIDTH, RPROBE_HEIGHT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &BLENDED_RPROBE_CUBE_MAP);
    glBindTexture(GL_TEXTURE_CUBE_MAP, BLENDED_RPROBE_CUBE_MAP);

    glTexStorage2D(GL_TEXTURE_CUBE_MAP, MAX_RPROBE_MIPS, GL_RGBA8, RPROBE_WIDTH, RPROBE_HEIGHT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glGenTextures(1, &RPROBE_BACK_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, RPROBE_BACK_TEXTURE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, RPROBE_WIDTH, RPROBE_HEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &RPROBE_BACK_FBO);
    glBindFramebuffer(GL_FRAMEBUFFER, RPROBE_BACK_FBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, RPROBE_BACK_TEXTURE, 0);


    


    float border_color[] = {1.0f, 1.0f, 1.0f, 1.0f};
    
    POINT_LIGHT_SHADOWS_FBO.w = DEFAULT_POINT_LIGHT_SHADOWS_SIZE;
    POINT_LIGHT_SHADOWS_FBO.h = DEFAULT_POINT_LIGHT_SHADOWS_SIZE;
    POINT_LIGHT_SHADOWS_FBO.texture_count = 0;
    POINT_LIGHT_SHADOWS_FBO.depth_texture =
        init_gl_texture
        (
            GL_TEXTURE_CUBE_MAP_ARRAY,
            GL_DEPTH_COMPONENT16,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            POINT_LIGHT_SHADOWS_FBO.w,
            POINT_LIGHT_SHADOWS_FBO.h,
            6 * MAX_POINT_LIGHT_SHADOWS_CMA,
            1,
            DEPTH_MODE_HW
        );

    init_fbo(&POINT_LIGHT_SHADOWS_FBO);

    SHADOW_MAP_FBO.w = DEFAULT_SHADOW_MAP_SIZE;
    SHADOW_MAP_FBO.h = DEFAULT_SHADOW_MAP_SIZE;
    SHADOW_MAP_FBO.texture_count = 0;
    SHADOW_MAP_FBO.rbo_count = 0;

    SHADOW_MAP_FBO.depth_texture =
        init_gl_texture
        (
            GL_TEXTURE_2D_ARRAY,
            GL_DEPTH_COMPONENT24,
            GL_DEPTH_COMPONENT,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            SHADOW_MAP_FBO.w,
            SHADOW_MAP_FBO.h,
            MAX_DIRECTIONAL_LIGHTS,
            0,
            DEPTH_MODE_HW
        );
    // we are leaving this here because i don't want to deal with the abstraction it probably doesnt fucking matter anyway
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border_color);
    init_fbo(&SHADOW_MAP_FBO);

    



    

    DEPTH_DOWNSAMPLE_FBO.w = RENDER_WIDTH / 2;
    DEPTH_DOWNSAMPLE_FBO.h = RENDER_HEIGHT / 2;
    DEPTH_DOWNSAMPLE_FBO.texture_count = 1;
    DEPTH_DOWNSAMPLE_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_R32F,
            GL_RED,
            GL_FLOAT,
            GL_NEAREST,
            GL_NEAREST,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            DEPTH_DOWNSAMPLE_FBO.w,
            DEPTH_DOWNSAMPLE_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );


    init_fbo(&DEPTH_DOWNSAMPLE_FBO);

    SSR_FBO.w = SSR_WIDTH;
    SSR_FBO.h = SSR_HEIGHT;
    SSR_FBO.texture_count = 1;
    SSR_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            SSR_FBO.w,
            SSR_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    init_fbo(&SSR_FBO);
    SSR_BLUR_FBO.w = SSR_WIDTH;
    SSR_BLUR_FBO.h = SSR_HEIGHT;
    SSR_BLUR_FBO.texture_count = 1;
    SSR_BLUR_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_BORDER,
            GL_CLAMP_TO_BORDER,
            0,
            SSR_BLUR_FBO.w,
            SSR_BLUR_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    init_fbo(&SSR_BLUR_FBO);


    TAA_FBO[0].w = RENDER_WIDTH;
    TAA_FBO[0].h = RENDER_HEIGHT;
    TAA_FBO[0].texture_count = 1;
    TAA_FBO[0].texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR_MIPMAP_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_FBO.w,
            GBUFFER_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    init_fbo(&TAA_FBO[0]);
    TAA_FBO[1].w = RENDER_WIDTH;
    TAA_FBO[1].h = RENDER_HEIGHT;
    TAA_FBO[1].texture_count = 1;
    TAA_FBO[1].texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            GBUFFER_FBO.w,
            GBUFFER_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );
    init_fbo(&TAA_FBO[1]);
    

    
    COC_FBO.w = DOF_WIDTH;
    COC_FBO.h = DOF_HEIGHT;
    COC_FBO.texture_count = 1;
    COC_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            COC_FBO.w,
            COC_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    init_fbo(&COC_FBO);

    BOKEH_FBO.w = DOF_WIDTH;
    BOKEH_FBO.h = DOF_HEIGHT;
    BOKEH_FBO.texture_count = 1;
    BOKEH_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RGBA16F,
            GL_RGBA,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            BOKEH_FBO.w,
            BOKEH_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    init_fbo(&BOKEH_FBO);

    int current_width = BLOOM_WIDTH;
    int current_height = BLOOM_HEIGHT;
    for (int i = 0; i < 5; i++)
    {

        Framebuffer * fbo = &BLOOM_PYRAMID[i];
        fbo->w = current_width;
        fbo->h = current_height;
        fbo->texture_count = 1;
        fbo->texture[0] =
            init_gl_texture
            (
                GL_TEXTURE_2D,
                GL_RGBA16F,
                GL_RGBA,
                GL_FLOAT,
                GL_LINEAR,
                GL_LINEAR,
                GL_CLAMP_TO_EDGE,
                GL_CLAMP_TO_EDGE,
                0,
                fbo->w,
                fbo->h,
                0,
                0,
                DEPTH_MODE_NONE
            );

        init_fbo(fbo);

        
        current_width /= 2;
        current_height /= 2;
        
    }


    if (EDITOR_SETTINGS.MSAA)
    {
        allocate_gbuffer_msaa();
    }
    else
    {
        
        allocate_gbuffer();
    }
    

    
    
    precompute_ssao_kernel();


    SSAO_FBO.w = SSAO_WIDTH;
    SSAO_FBO.h = SSAO_HEIGHT;
    SSAO_FBO.texture_count = 1;
    SSAO_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RED,
            GL_RED,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            SSAO_FBO.w,
            SSAO_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    init_fbo(&SSAO_FBO);

    SSAO_BLUR_FBO.w = SSAO_WIDTH;
    SSAO_BLUR_FBO.h = SSAO_HEIGHT;
    SSAO_BLUR_FBO.texture_count = 1;
    SSAO_BLUR_FBO.texture[0] =
        init_gl_texture
        (
            GL_TEXTURE_2D,
            GL_RED,
            GL_RED,
            GL_FLOAT,
            GL_LINEAR,
            GL_LINEAR,
            GL_CLAMP_TO_EDGE,
            GL_CLAMP_TO_EDGE,
            0,
            SSAO_BLUR_FBO.w,
            SSAO_BLUR_FBO.h,
            0,
            0,
            DEPTH_MODE_NONE
        );

    init_fbo(&SSAO_BLUR_FBO);

    if (EDITOR_SETTINGS.MSAA)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, MAIN_FBO.fbo);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, GBUFFER_FBO.depth_rbo.i);
        
    }
    else
    {
        
        glBindFramebuffer(GL_FRAMEBUFFER, MAIN_RESOLVE_FBO.fbo);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, GBUFFER_RESOLVE_FBO.depth_texture.i, 0);
    }
    

    float noise[16][4];
    for (int i = 0; i < 16; i++) {
        noise[i][0] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; 
        noise[i][1] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; 
        noise[i][2] = ((float)rand() / RAND_MAX) * 2.0f - 1.0f; 
        noise[i][3] = 1.0f;                                     
    }    
    glGenTextures(1, &SSAO_NOISE_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, SSAO_NOISE_TEXTURE);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGBA, GL_FLOAT, noise);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 

    


    glGenTextures(1, &SOBEL_TEXTURE[0]);
    glBindTexture(GL_TEXTURE_2D, SOBEL_TEXTURE[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &SOBEL_FBO[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, SOBEL_FBO[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SOBEL_TEXTURE[0], 0);

    glGenTextures(1, &SOBEL_TEXTURE[1]);
    glBindTexture(GL_TEXTURE_2D, SOBEL_TEXTURE[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &SOBEL_FBO[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, SOBEL_FBO[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, SOBEL_TEXTURE[1], 0);
    
    glGenTextures(1, &HISTORY_TEXTURE[0]);
    glBindTexture(GL_TEXTURE_2D, HISTORY_TEXTURE[0]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &HISTORY_FBO[0]);
    glBindFramebuffer(GL_FRAMEBUFFER, HISTORY_FBO[0]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, HISTORY_TEXTURE[0], 0);
    
    glGenTextures(1, &HISTORY_TEXTURE[1]);
    glBindTexture(GL_TEXTURE_2D, HISTORY_TEXTURE[1]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, RENDER_WIDTH, RENDER_HEIGHT, 0, GL_RGBA, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &HISTORY_FBO[1]);
    glBindFramebuffer(GL_FRAMEBUFFER, HISTORY_FBO[1]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, HISTORY_TEXTURE[1], 0);
    
    

    glGenTextures(1, &FBM_TEXTURE);
    glBindTexture(GL_TEXTURE_2D, FBM_TEXTURE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, FBM_TEXTURE_SIZE, FBM_TEXTURE_SIZE, 0, GL_RGB, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    precompute_fbm_texture();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    GLbitfield ubo_flags = GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_COHERENT_BIT;
        
    GLint ubo_alignment;
    glGetIntegerv(GL_UNIFORM_BUFFER_OFFSET_ALIGNMENT, &ubo_alignment);
    printf("SSBO\n");


    glGenBuffers(1, &CAM_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, CAM_UBO);
    cam_ubo_padded = (sizeof(CamData) + ubo_alignment - 1) / ubo_alignment * ubo_alignment;
    glBufferStorage(GL_UNIFORM_BUFFER, cam_ubo_padded * BUFFER_FRAMES, NULL, ubo_flags);
    cam_ubo_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, cam_ubo_padded * BUFFER_FRAMES, ubo_flags);    


    
    glGenBuffers(1, &RENDER_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, RENDER_UBO);
    ivec4 res = (ivec4){RENDER_WIDTH, RENDER_HEIGHT, 0, 0};
    glBufferData(GL_UNIFORM_BUFFER, sizeof(RenderData), &res, GL_DYNAMIC_DRAW);
    glBindBufferBase(GL_UNIFORM_BUFFER, 1, RENDER_UBO); 

    light_cam_ubo_padded = (sizeof(ShadowmapUBO) + ubo_alignment - 1) / ubo_alignment * ubo_alignment;
    glGenBuffers(1, &LIGHT_CAM_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, LIGHT_CAM_UBO);
    glBufferStorage(GL_UNIFORM_BUFFER, light_cam_ubo_padded * BUFFER_FRAMES, NULL, ubo_flags);
    light_cam_ubo_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, light_cam_ubo_padded * BUFFER_FRAMES, ubo_flags);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 2, LIGHT_CAM_UBO); 

    rprobe_ubo_padded = (sizeof(RProbeUBO) + ubo_alignment - 1) / ubo_alignment * ubo_alignment;
    glGenBuffers(1, &RPROBE_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, RPROBE_UBO);
    glBufferStorage(GL_UNIFORM_BUFFER, rprobe_ubo_padded * BUFFER_FRAMES, NULL, ubo_flags);
    rprobe_ubo_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, rprobe_ubo_padded * BUFFER_FRAMES, ubo_flags);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 3, RPROBE_UBO); 

    glGenBuffers(1, &SSAO_KERNEL_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, SSAO_KERNEL_UBO);
    glBufferStorage(GL_UNIFORM_BUFFER, SSAO_KERNEL_SIZE * sizeof(vec4), SSAO_KERNEL, 0);
    glBindBufferBase(GL_UNIFORM_BUFFER, 5, SSAO_KERNEL_UBO); 

    probe_volume_ubo_padded = (sizeof(ProbeVolumeUBO) + ubo_alignment - 1) / ubo_alignment * ubo_alignment;
    glGenBuffers(1, &PROBE_VOLUME_UBO);
    glBindBuffer(GL_UNIFORM_BUFFER, PROBE_VOLUME_UBO);
    glBufferStorage(GL_UNIFORM_BUFFER, probe_volume_ubo_padded * BUFFER_FRAMES, NULL, ubo_flags);
    probe_volume_ubo_ptr = (uint8_t*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, probe_volume_ubo_padded * BUFFER_FRAMES, ubo_flags);    
    glBindBufferBase(GL_UNIFORM_BUFFER, 6, PROBE_VOLUME_UBO); 

    glGenBuffers(1, &LIGHT_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, LIGHT_SSBO);
    light_ssbo_size = sizeof(LightGPU) * MAX_LIGHTS;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, light_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    light_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, light_ssbo_size * BUFFER_FRAMES, ubo_flags);    
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, LIGHT_SSBO); 

    glGenBuffers(1, &MESH_LIGHT_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MESH_LIGHT_SSBO);
    mesh_light_ssbo_size = sizeof(uint32_t) * MAX_MESH_NODES * MAX_LIGHTS_PER_MESH;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, mesh_light_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    mesh_light_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, mesh_light_ssbo_size * BUFFER_FRAMES, ubo_flags);    

    glGenBuffers(1, &MATERIAL_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MATERIAL_SSBO);
    glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(MaterialData), NULL, GL_STATIC_DRAW);
    
    glGenBuffers(1, &LIGHTMAP_SSBO);
    glGenBuffers(1, &VERTEX_SSBO);
    glGenBuffers(1, &TRI_SSBO);
    glGenBuffers(1, &MESH_SSBO);
    glGenBuffers(1, &MESH_BVH_SSBO);
    glGenBuffers(1, &SCENE_BVH_SSBO);
    glGenBuffers(1, &PROBE_SSBO);
    glGenBuffers(1, &RPROBE_SSBO);

    glGenBuffers(1, &JOINT_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, JOINT_SSBO);
    joint_ssbo_size = sizeof(mat4) * MAX_JOINT_MATRICES;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, joint_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    joint_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, joint_ssbo_size * BUFFER_FRAMES, ubo_flags);    

    glGenBuffers(1, &CLUSTER_AABB_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, CLUSTER_AABB_SSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ClusterAABBGPU) * TOTAL_CLUSTERS, NULL, 0);

    glGenBuffers(1, &GLOBAL_LIGHT_INDICES_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GLOBAL_LIGHT_INDICES_SSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(uint32_t) * TOTAL_CLUSTERS * MAX_LIGHTS_PER_CLUSTER, NULL, 0);

    glGenBuffers(1, &CLUSTER_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, CLUSTER_SSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(ClusterGPU) * TOTAL_CLUSTERS * MAX_LIGHTS_PER_CLUSTER, NULL, 0);

    glGenBuffers(1, &MESH_NODE_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, MESH_NODE_SSBO);
    mesh_node_ssbo_size = sizeof(MeshNodeSSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, mesh_node_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    mesh_node_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, mesh_node_ssbo_size * BUFFER_FRAMES, ubo_flags);    

    glGenBuffers(1, &GLOBAL_INSTANCE_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, GLOBAL_INSTANCE_SSBO);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(GlobalInstanceGPU) * 6, NULL, GL_MAP_READ_BIT);

    glGenBuffers(1, &INDIRECT_ARRAYS_COMMAND_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, INDIRECT_ARRAYS_COMMAND_BUFFER);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand) * MAX_MESH_NODES, NULL, ubo_flags);
    indirect_arrays_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawArraysIndirectCommand) * MAX_MESH_NODES, ubo_flags);    

    glGenBuffers(1, &INDIRECT_ARRAYS_COMMAND_BUFFER_OUT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, INDIRECT_ARRAYS_COMMAND_BUFFER_OUT);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawArraysIndirectCommand) * MAX_MESH_NODES, NULL, 0);
    indirect_arrays_out_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawArraysIndirectCommand) * MAX_MESH_NODES, GL_MAP_PERSISTENT_BIT);    
        
    glGenBuffers(1, &INDIRECT_ELEMENTS_COMMAND_BUFFER);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, INDIRECT_ELEMENTS_COMMAND_BUFFER);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * MAX_MESH_NODES, NULL, ubo_flags);
    indirect_elements_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawElementsIndirectCommand) * MAX_MESH_NODES, ubo_flags);    

    glGenBuffers(1, &INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, INDIRECT_ELEMENTS_COMMAND_BUFFER_OUT);
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, sizeof(DrawElementsIndirectCommand) * MAX_MESH_NODES, NULL, 0);
    indirect_elements_out_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, sizeof(DrawElementsIndirectCommand) * MAX_MESH_NODES, GL_MAP_PERSISTENT_BIT);    
    

    glGenBuffers(1, &INSTANCE_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, INSTANCE_SSBO);
    instance_ssbo_size = sizeof(TransformGPU) * MAX_INSTANCES * MAX_MESH_NODES;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, instance_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    instance_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, instance_ssbo_size * BUFFER_FRAMES, ubo_flags);    

    glGenBuffers(1, &TEXT_BOX_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, TEXT_BOX_SSBO);
    text_box_ssbo_size = sizeof(TextBoxGPU) * MAX_TEXT_BOXES;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, text_box_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    text_box_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, text_box_ssbo_size * BUFFER_FRAMES, ubo_flags);    
    
    glGenBuffers(1, &UI_SSBO);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, UI_SSBO);
    ui_ssbo_size = sizeof(UIElementGPU) * MAX_UI_ELEMENTS;
    glBufferStorage(GL_SHADER_STORAGE_BUFFER, ui_ssbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    ui_ssbo_ptr = (uint8_t*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, ui_ssbo_size * BUFFER_FRAMES, ubo_flags);    

    bind_render_ssbos();

    printf("Shaders\n");
    init_shaders();

    printf("Vertices\n");




    glGenVertexArrays(1, &MAIN_VAO);
    glBindVertexArray(MAIN_VAO);
    glGenBuffers(1, &MAIN_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, MAIN_VBO);
    glGenBuffers(1, &MAIN_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, MAIN_EBO);

    size_t main_stride = sizeof(float) * 16 + sizeof(int) * 4;
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, main_stride, (void *)(uintptr_t)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, main_stride, (void *)(uintptr_t)(sizeof(float) * 3));
    // tangent
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, 0, main_stride, (void *)(uintptr_t)(sizeof(float) * 6));
    
    // joint indices
    glEnableVertexAttribArray(3);
    glVertexAttribIPointer(3, 4, GL_INT, main_stride, (void *)(uintptr_t)(sizeof(float) * 10));
    
    // weights
    glEnableVertexAttribArray(4);
    glVertexAttribPointer(4, 4, GL_FLOAT, 0, main_stride, (void *)(uintptr_t)(sizeof(float) * 10) + sizeof(int) * 4);
    // uv
    glEnableVertexAttribArray(5);
    glVertexAttribPointer(5, 2, GL_FLOAT, 0, main_stride, (void *)(uintptr_t)(sizeof(float) * 14) + sizeof(int) * 4);

    // can bind a second vbo to use just for instance matrices
    instance_vbo_size = MAX_INSTANCES * sizeof(InstanceData);
    glGenBuffers(1, &INSTANCE_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, INSTANCE_VBO);
    glBufferStorage(GL_ARRAY_BUFFER, instance_vbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    instance_vbo_ptr = (uint8_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, instance_vbo_size * BUFFER_FRAMES, ubo_flags);    

    for (int i = 0; i < 4; i++) {

        GLuint location = 6 + i;
        glEnableVertexAttribArray(location);
    
        glVertexAttribPointer(location, 4, GL_FLOAT, GL_FALSE, sizeof(InstanceData), (void*)(i * sizeof(float) * 4));
    
        glVertexAttribDivisor(location, 1);
    }

    glEnableVertexAttribArray(10);
    glVertexAttribIPointer(10, 2, GL_INT, sizeof(InstanceData), (void *)(uintptr_t)(4 * sizeof(float) * 4));
    glVertexAttribDivisor(10, 1);
    glEnableVertexAttribArray(11);
    glVertexAttribIPointer(11, 2, GL_INT, sizeof(InstanceData), (void *)(uintptr_t)(4 * sizeof(float) * 4 + sizeof(int) * 2));
    glVertexAttribDivisor(11, 1);
    glEnableVertexAttribArray(12);
    glVertexAttribIPointer(12, 2, GL_INT, sizeof(InstanceData), (void *)(uintptr_t)(4 * sizeof(float) * 4 + sizeof(int) * 4));
    glVertexAttribDivisor(12, 1);
    glEnableVertexAttribArray(13);
    glVertexAttribIPointer(13, 1, GL_INT, sizeof(InstanceData), (void *)(uintptr_t)(4 * sizeof(float) * 4 + sizeof(int) * 6));
    glVertexAttribDivisor(13, 1);


    glGenVertexArrays(1, &LIGHTMAP_VAO);
    glBindVertexArray(LIGHTMAP_VAO);
    glGenBuffers(1, &LIGHTMAP_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, LIGHTMAP_VBO);

    size_t lightmap_stride = sizeof(float) * 10 + sizeof(int) * 2;
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, lightmap_stride, (void *)(uintptr_t)0);
    // normal
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, lightmap_stride, (void *)(uintptr_t)(sizeof(float) * 3));
    // uv
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, 0, lightmap_stride, (void *)(uintptr_t)(sizeof(float) * 6));
    // lm_uv
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 2, GL_FLOAT, 0, lightmap_stride, (void *)(uintptr_t)(sizeof(float) * 8));
    // material id
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 1, GL_INT, lightmap_stride, (void *)(uintptr_t)(sizeof(float) * 10));
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_INT, lightmap_stride, (void *)(uintptr_t)(sizeof(float) * 10 + sizeof(int)));
    
    
    glGenVertexArrays(1, &WIREFRAME_VAO);
    glBindVertexArray(WIREFRAME_VAO);
    glGenBuffers(1, &WIREFRAME_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, WIREFRAME_VBO);
    glBufferData(GL_ARRAY_BUFFER, DEBUG_LINES_MAX * 3 * sizeof(DebugVertex), NULL, GL_STREAM_DRAW);

    size_t wireframe_stride = sizeof(float) * 6;
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, wireframe_stride, (void *)(uintptr_t)0);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, 0, wireframe_stride, (void *)(uintptr_t)(sizeof(float) * 3));



    glGenVertexArrays(1, &TEXT_VAO);
    glBindVertexArray(TEXT_VAO);
    glGenBuffers(1, &TEXT_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, TEXT_VBO);

    text_vbo_size = MAX_TEXT_BOXES * MAX_TEXT_BOX_CHARACTERS * 6 * sizeof(TextVertex);
    glBufferData(GL_ARRAY_BUFFER, text_vbo_size, NULL, GL_STREAM_DRAW);
    // text_vbo_ptr = (uint8_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, text_vbo_size, ubo_flags);    

    glGenBuffers(1, &TEXT_EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TEXT_EBO);

    glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_TEXT_BOXES * MAX_TEXT_BOX_CHARACTERS * 4 * sizeof(uint32_t), NULL, GL_STREAM_DRAW);
    
    size_t text_stride = sizeof(float) * 14 + sizeof(uint32_t) * 2;
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, text_stride, (void *)(uintptr_t)0);
    // uv
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, 0, text_stride, (void *)(uintptr_t)(sizeof(float) * 2));
    // center of character
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 2, GL_FLOAT, 0, text_stride, (void *)(uintptr_t)(sizeof(float) * 4));
    // color
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, 0, text_stride, (void *)(uintptr_t)(sizeof(float) * 6));
    // fill
    glEnableVertexAttribArray(4);
    glVertexAttribIPointer(4, 4, GL_FLOAT, text_stride, (void *)(uintptr_t)(sizeof(float) * 10));
    // state
    glEnableVertexAttribArray(5);
    glVertexAttribIPointer(5, 1, GL_UNSIGNED_INT, text_stride, (void *)(uintptr_t)(sizeof(float) * 14));
    // uID
    glEnableVertexAttribArray(6);
    glVertexAttribIPointer(6, 1, GL_UNSIGNED_INT, text_stride, (void *)(uintptr_t)(sizeof(float) * 14) + sizeof(uint32_t) * 1);


    glGenVertexArrays(1, &UI_VAO);
    glBindVertexArray(UI_VAO);
    glGenBuffers(1, &UI_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, UI_VBO);

    ui_vbo_size = sizeof(vec2) * MAX_UI_ELEMENTS * 6;
    glBufferStorage(GL_ARRAY_BUFFER, ui_vbo_size, NULL, ubo_flags);

    ui_vbo_ptr = (uint8_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, ui_vbo_size, ubo_flags);    
    int ui_vbo_index = 0;
    for (int i = 0; i < MAX_UI_ELEMENTS; i++)
    {

        vec2 * uvbo = &(((vec2*)(ui_vbo_ptr))[ui_vbo_index]);
        uvbo[0] = (vec2){-1, -1};
        uvbo[1] = (vec2){-1, 1};
        uvbo[2] = (vec2){1, -1};
        uvbo[3] = (vec2){1, -1};
        uvbo[4] = (vec2){-1, 1};
        uvbo[5] = (vec2){1, 1};
        
        ui_vbo_index += 6;
    }

    size_t ui_stride = sizeof(vec2);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, 0, ui_stride, (void *)(uintptr_t)0);

    // can bind a second vbo to use just for instance matrices
    ui_instance_vbo_size = MAX_UI_ELEMENTS * sizeof(uint32_t);

    glGenBuffers(1, &UI_INSTANCE_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, UI_INSTANCE_VBO);
    
    glBufferStorage(GL_ARRAY_BUFFER, ui_instance_vbo_size * BUFFER_FRAMES, NULL, ubo_flags);
    ui_instance_vbo_ptr = (uint8_t*)glMapBufferRange(GL_ARRAY_BUFFER, 0, ui_instance_vbo_size * BUFFER_FRAMES, ubo_flags);    


    glEnableVertexAttribArray(1);
    glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(uint32_t), (void *)(uintptr_t)(0));
    glVertexAttribDivisor(1, 1);
    
    COLOR_GRADE_LUT = load_3d_lut("../engine-assets/LUTs/main_LUT.cube");

    SKYBOX = load_cubemap("../engine-assets/cubemap3");
    glGenVertexArrays(1, &SKY_VAO);
    glBindVertexArray(SKY_VAO);
    glGenBuffers(1, &SKY_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, SKY_VBO);

    const float skyboxVertices[] = {

        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
         1.0f,  1.0f, -1.0f,
         1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f,  1.0f

    };

    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 36 * 3, skyboxVertices, GL_STATIC_DRAW);

    
    // Skybox
    size_t sky_stride = sizeof(float) * 3;
    // pos
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, 0, sky_stride, (void *)(uintptr_t)0);



    glGenVertexArrays(1, &CONVOLVE_TRIANGLE_VAO);
    glBindVertexArray(CONVOLVE_TRIANGLE_VAO);
    glGenBuffers(1, &CONVOLVE_TRIANGLE_VBO);
    glBindBuffer(GL_ARRAY_BUFFER, CONVOLVE_TRIANGLE_VBO);
    float triangleVertices[] = {
        -1.0f, -1.0f,
        3.0f, -1.0f, 
        -1.0f,  3.0f 
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(triangleVertices), triangleVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 2, (void*)0);


    glGenVertexArrays(1, &FS_VAO);

    init_vertex_layouts();

    
    
    glBindFramebuffer(GL_FRAMEBUFFER, BRDF_FBO);
    glViewport(0,0,512,512);
    glClearColor(0, 0, 0, 1.0f);
    glClearDepth(1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(BRDF_SHADER.program);

    glDrawArrays(GL_TRIANGLES, 0, 3);
    
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}
