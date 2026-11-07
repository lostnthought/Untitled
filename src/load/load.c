
#include "load.h"
#include <stdio.h>
#include <string.h>
#include "load/init.h"
#include "light/lightmap.h"
#include "utilities/types.h"
#include "stb/stb_image.h"
#include "render/render.h"
#include "audio/audio.h"
#include "ui/fonts.h"
#include "editor/editor.h"





GLuint
    load_texture
    (
        const char * path,
        GLint min_filter,
        GLint mag_filter,
        GLint wrap_s,
        GLint wrap_t,
        bool sRGB
    )

{

    int w, h, channels;

    unsigned char *data = stbi_load(path, &w, &h, &channels, 0);

    if (!data) {
        printf("Failed to load image: %s\n", path);
        return 0;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);




    glBindTexture(GL_TEXTURE_2D, textureID);
    if (sRGB)
    {
        
        if (channels == 3)
        {
        
            printf("LOADING: %s %d\n", path, min_filter == GL_LINEAR_MIPMAP_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        }
        else
        {
            printf("LOADING: %s %d\n", path, min_filter == GL_LINEAR_MIPMAP_LINEAR);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB8_ALPHA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        
        }
    }
    else
    {
        if (channels == 3)
        {
    
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        }
        else if (channels == 4)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);

            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
            
            glGenerateMipmap(GL_TEXTURE_2D);


    
        }
        else if (channels == 2)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RG8, w, h, 0, GL_RG, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
            
        }
        else if (channels == 1)
        {
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, w, h, 0, GL_RED, GL_UNSIGNED_BYTE, data);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter); 
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
            float max_aniso = 0.0f;
            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &max_aniso);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, MIN(8.0f, max_aniso));
            glGenerateMipmap(GL_TEXTURE_2D);

            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
            
        }
        
    }


    stbi_image_free(data);
    return textureID;

}


GLuint
    load_cubemap
    ( const char * path )

{
    const char a[6][12] =
    {
        "/pos_x.hdr", "/neg_x.hdr", "/pos_y.hdr", "/neg_y.hdr", "/pos_z.hdr", "/neg_z.hdr"
    };
    
    GLuint cubemap = 0;
    glGenTextures(1, &cubemap);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap);
    for (int i = 0; i < 6; i++)
    {

        stbi_set_flip_vertically_on_load(0);
        char c[428];
        strcpy(c, path);
        strcat(c, a[i]);
        printf("%s\n", c);

        int w = 0, h = 0, channels = 0;

        float * data = stbi_loadf(c, &w, &h, &channels, 0);
        printf("W %d H %d C %d\n", w, h, channels);

        if (!data) {

            printf("Failed to load image: %s\n", c);
            return 0;
        }
        
        if (channels == 3)
        {
            
            glPixelStorei(GL_UNPACK_ALIGNMENT, 3); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB16F, w, h, 0, GL_RGB, GL_FLOAT, NULL);
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGB, GL_FLOAT, data);
        }
        else
        {
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4); 
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA16F, w, h, 0, GL_RGBA, GL_FLOAT, NULL);
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, 0, 0, w, h, GL_RGBA, GL_FLOAT, data);
            
        }

        stbi_image_free(data);
        
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR); 
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    
    return cubemap;

}

GLuint load_3d_lut(const char* filepath) {
    FILE* file = fopen(filepath, "r");
    if (!file) {
        printf("Failed to open LUT file: %s\n", filepath);
        return 0;
    }

    int lut_size = 0;
    char line[256];

    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "LUT_3D_SIZE", 11) == 0) {
            sscanf(line, "LUT_3D_SIZE %d", &lut_size);
            break;
        }
    }

    if (lut_size != 33) {
        printf("Warning: Expected LUT size 33, got %d\n", lut_size);
    }

    int total_elements = lut_size * lut_size * lut_size * 3;
    float* lut_data = (float*)malloc(total_elements * sizeof(float));

    int index = 0;
    while (fgets(line, sizeof(line), file)) {
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') continue;

        float r, g, b;
        if (sscanf(line, "%f %f %f", &r, &g, &b) == 3) {
            lut_data[index++] = r;
            lut_data[index++] = g;
            lut_data[index++] = b;
        }
    }
    fclose(file);

    GLuint lut_texture;
    glGenTextures(1, &lut_texture);
    glBindTexture(GL_TEXTURE_3D, lut_texture);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB16F, lut_size, lut_size, lut_size, 0, GL_RGB, GL_FLOAT, lut_data);

    free(lut_data);
    return lut_texture;
}

unsigned char * open_image(Image * image)
{
    
    int w, h, channels;

    unsigned char *data = stbi_load(image->path, &w, &h, &channels, 0);
    image->width = w;
    image->height = h;
    image->channels = channels;
    if (!data) {
        printf("Failed to load image: %s\\n", image->path);
        return NULL;
    }
    return data;
}


int add_image_to_registry(char * filename, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, bool sRGB)
{

    GLuint i = load_texture(filename, min_filter, mag_filter, wrap_s, wrap_t, sRGB);
    if (i == 0) return -1;

    GLuint64 handle = glGetTextureHandleARB(i);

    glMakeTextureHandleResidentARB(handle);

    Image * image = vector_expand(&GLOBALS.images, 1);
    image->buf = NULL;
    image->sRGB = sRGB;
    strncpy(image->path, filename, 255);    
    image->path[255] = '\0';
    image->channels = 4;
    image->i = i;
    image->handle = handle;
    return (GLOBALS.images.count - 1);
    
}


void save_ui(const char * name)
{

    char ui_name[256];
    strcpy(ui_name, ui_registry.directory);
    strcat(ui_name, name);
    strcat(ui_name, ".bin");

    FILE * f = fopen(ui_name, "wb");
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);
    fwrite(&MENUS.count, sizeof(size_t), 1, f);
    
    for (int i = 0; i < MENUS.count; i++)
    {
        UIMenu * m = &ms[i];
        fwrite(&m->uID, sizeof(uint32_t), 1, f);
        fwrite(&m->id, sizeof(char), 256, f);
        fwrite(m->elements, sizeof(UIElement), MAX_UI_ELEMENTS, f);
        fwrite(m->containers, sizeof(UIContainer), MAX_UI_CONTAINERS, f);

        for (int j = 0; j < MAX_TEXT_BOXES; j++)
        {
            
            TextBox * tb = &m->text_boxes[j];
            fwrite(tb, sizeof(TextBox), 1, f);
            fwrite(tb->text, sizeof(char), tb->text_cap, f);
           
        }

    }
}


void load_ui(const char * name)
{

    char ui_name[256];
    strcpy(ui_name, ui_registry.directory);
    strcat(ui_name, name);
    strcat(ui_name, ".bin");
    

    FILE * f = fopen(ui_name, "rb");
    if (MENUS.buffer) free(MENUS.buffer);
    init_vector(&MENUS, 0, sizeof(UIMenu));
    fread(&MENUS.count, sizeof(size_t), 1, f);
    MENUS.buffer = malloc(sizeof(UIMenu) * MENUS.count);
    MENUS.capacity = MENUS.count;
    UIMenu * ms = CAST_VEC(MENUS, UIMenu);

    
    for (int i = 0; i < MENUS.count; i++)
    {

        UIMenu * m = &ms[i];
        fread(&m->uID, sizeof(uint32_t), 1, f);
        fread(&m->id, sizeof(char), 256, f);
        fread(m->elements, sizeof(UIElement), MAX_UI_ELEMENTS, f);
        fread(m->containers, sizeof(UIContainer), MAX_UI_CONTAINERS, f);

        for (int j = 0; j < MAX_TEXT_BOXES; j++)
        {
            
            TextBox * tb = &m->text_boxes[j];
            fread(tb, sizeof(TextBox), 1, f);
            tb->text = malloc(sizeof(char) * tb->text_cap);
            fread(tb->text, sizeof(char), tb->text_cap, f);
           
        }

    }

    
}



void reallocate_rprobe_array()
{
    
    glDeleteTextures(1, &RPROBE_CUBE_MAP_ARRAY);
    glGenTextures(1, &RPROBE_CUBE_MAP_ARRAY);
    glBindTexture(GL_TEXTURE_CUBE_MAP_ARRAY, RPROBE_CUBE_MAP_ARRAY);
    glTexStorage3D(GL_TEXTURE_CUBE_MAP_ARRAY, MAX_RPROBE_MIPS, GL_RGBA16F, RPROBE_WIDTH, RPROBE_HEIGHT, GLOBALS.scene_data.reflection_probes.count * 6);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP_ARRAY, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

}

void write_mesh_node(MeshNode * mn, FILE * f)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    // fwrite(mn->name, sizeof(char), 256, f);
    fwrite(&mn->active, sizeof(bool), 1, f);
    fwrite(&mn->id, sizeof(int), 1, f);
    fwrite(&mn->parent, sizeof(int), 1, f);
    fwrite(&mn->translation, sizeof(vec3), 1, f);
    fwrite(&mn->rotation, sizeof(vec4), 1, f);
    fwrite(&mn->scale, sizeof(vec3), 1, f);
    fwrite(&mn->local_matrix, sizeof(mat4), 1, f);
    fwrite(&mn->global_matrix, sizeof(mat4), 1, f);
    fwrite(&mn->mesh, sizeof(int), 1, f);
    fwrite(&mn->asset, sizeof(int), 1, f);
    fwrite(&mn->skin, sizeof(int), 1, f);
    fwrite(&mn->child_count, sizeof(int), 1, f);
    fwrite(mn->children, sizeof(int), mn->child_count, f);

    fwrite(&mn->static_mesh, sizeof(bool), 1, f);
    fwrite(&mn->physics_mode, sizeof(bool), 1, f);

    fwrite(&mn->skip_directional_uv_lighting, sizeof(bool), 1, f);

    if (mn->mesh != -1)
    {

        Mesh * m = &meshes[mn->mesh];
        if (m->static_mesh)
        {
            
            if (mn->uv_map)
            {
                
                bool uv_map = true;
                fwrite(&uv_map, sizeof(bool), 1, f);
                fwrite(mn->uv_map, sizeof(UVMapping), m->tris.count, f);
            }
            else
            {
                bool uv_map = false;
                fwrite(&uv_map, sizeof(bool), 1, f);
            }

        }

    }
    
    if (mn->pc)
    {
        
        int id = mn->pc->id;
        fwrite(&id, sizeof(int), 1, f);
    }
    else
    {
        int zero = -1;
        fwrite(&zero, sizeof(int), 1, f);
    }
    
    if (mn->ent)
    {
        
        int id = mn->ent->id;
        fwrite(&id, sizeof(int), 1, f);
    }
    else
    {
        int zero = -1;
        fwrite(&zero, sizeof(int), 1, f);
    }
}

void write_skin(GLTFSkin * skin, FILE * f)
{

    fwrite(&skin->joint_count, sizeof(int), 1, f);
    if (skin->joints) fwrite(skin->joints, sizeof(int), skin->joint_count, f);

    if (skin->final_mat) fwrite(skin->final_mat, sizeof(mat4), skin->joint_count, f);
    if (skin->inverse_bind) fwrite(skin->inverse_bind, sizeof(mat4), skin->joint_count, f);

    fwrite(&skin->ibm_accessor, sizeof(int), 1, f);

    // animation buffer not here
    fwrite(&skin->animation_indices_count, sizeof(int), 1, f);
    fwrite(skin->animation_indices, sizeof(int), skin->animation_indices_count, f);
   
    
}





void scene_save(const char * name)
{

    char scene_name[256];
    strcpy(scene_name, scene_registry.directory);
    strcat(scene_name, name);
    strcat(scene_name, ".bin");

    char lm_name[256];
    strcpy(lm_name, lightmap_registry.directory);
    strcat(lm_name, name);
    strcat(lm_name, "_lm.bin");

    char light_name[256];
    strcpy(light_name, light_registry.directory);
    strcat(light_name, name);
    strcat(light_name, "_lights.bin");

    char aportal_name[256];
    strcpy(aportal_name, aportal_registry.directory);
    strcat(aportal_name, name);
    strcat(aportal_name, "_aportals.bin");

    char rprobe_name[256];
    strcpy(rprobe_name, rprobe_registry.directory);
    strcat(rprobe_name, name);
    strcat(rprobe_name, "_rprobes.bin");

    char lprobe_name[256];
    strcpy(lprobe_name, lprobe_registry.directory);
    strcat(lprobe_name, name);
    strcat(lprobe_name, "_lprobes.bin");

    char aprobe_name[256];
    strcpy(aprobe_name, aprobe_registry.directory);
    strcat(aprobe_name, name);
    strcat(aprobe_name, "_aprobes.bin");

    char scene_apv_name[256];
    strcpy(scene_apv_name, scene_apv_registry.directory);
    strcat(scene_apv_name, name);
    strcat(scene_apv_name, "_scene_apvs.bin");


    FILE * f = fopen(scene_name, "wb");
    printf("%s\n", scene_name);
    if (!f) return;

    save_lightmap_to_binary(&GLOBALS.scene_data.lightmap_data, lm_name);

    save_aportals(aportal_name, &GLOBALS.audio_portals);

    save_aprobes(aprobe_name);

    save_rprobes(rprobe_name, &GLOBALS.scene_data.reflection_probes);
    
    save_vector(light_name, &GLOBALS.lights);

    printf("GLOBALS\n");
    fwrite(GLOBALS.scene_data.name, sizeof(char), 256, f);

    fwrite(&GLOBALS.scene_data.aabb, sizeof(AABB), 1, f);

    fwrite(&GLOBALS.scene_data.bvh_nodes.count, sizeof(size_t), 1, f);
    if (GLOBALS.scene_data.bvh_nodes.buffer) fwrite(GLOBALS.scene_data.bvh_nodes.buffer, sizeof(BVHNode), GLOBALS.scene_data.bvh_nodes.count, f);

    fwrite(&GLOBALS.scene_data.light_bvh.nodes.count, sizeof(size_t), 1, f);
    if (GLOBALS.scene_data.light_bvh.nodes.buffer) fwrite(GLOBALS.scene_data.light_bvh.nodes.buffer, sizeof(BVHNode), GLOBALS.scene_data.light_bvh.nodes.count, f);
    fwrite(&GLOBALS.scene_data.light_bvh.lights.count, sizeof(size_t), 1, f);
    if (GLOBALS.scene_data.light_bvh.lights.buffer) fwrite(GLOBALS.scene_data.light_bvh.lights.buffer, sizeof(Light), GLOBALS.scene_data.light_bvh.lights.count, f);
    if (GLOBALS.scene_data.light_bvh.indices) fwrite(GLOBALS.scene_data.light_bvh.indices, sizeof(size_t), GLOBALS.scene_data.light_bvh.lights.count, f);

    fwrite(&GLOBALS.scene_data.leaf_indices.count, sizeof(size_t), 1, f);
    if (GLOBALS.scene_data.leaf_indices.buffer) fwrite(GLOBALS.scene_data.leaf_indices.buffer, sizeof(size_t), GLOBALS.scene_data.leaf_indices.count, f);
    fwrite(&GLOBALS.scene_data.bvh_indices_count, sizeof(int), 1, f);
    if (GLOBALS.scene_data.bvh_indices) fwrite(GLOBALS.scene_data.bvh_indices, sizeof(size_t), GLOBALS.scene_data.bvh_indices_count, f);

    // fwrite(&GLOBALS.scene_data.reflection_probes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.reflection_probes.buffer) fwrite(GLOBALS.scene_data.reflection_probes.buffer, sizeof(ReflectionProbe), GLOBALS.scene_data.reflection_probes.count, f);

    // fwrite(&GLOBALS.scene_data.audio_probe_volumes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.audio_probe_volumes.buffer) fwrite(GLOBALS.scene_data.audio_probe_volumes.buffer, sizeof(ProbeVolume), GLOBALS.scene_data.audio_probe_volumes.count, f);

    // fwrite(&GLOBALS.scene_data.lighting_probe_volumes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.lighting_probe_volumes.buffer) fwrite(GLOBALS.scene_data.lighting_probe_volumes.buffer, sizeof(ProbeVolume), GLOBALS.scene_data.lighting_probe_volumes.count, f);


    // fwrite(GLOBALS.scene_data.APV_PATH, sizeof(char), 256, f);
    
    
    printf("LD\n");

    LightmapData * ld = &GLOBALS.scene_data.lightmap_data;
    UVIsland * islands = CAST_VEC(ld->islands, UVIsland);
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
    for (int i = 0; i < ld->islands.count; i++)
    {

        UVIsland * uvi = &islands[i];
        fwrite(&uvi->mesh_node, sizeof(int), 1, f);
        fwrite(&uvi->tris.count, sizeof(size_t), 1, f);
        if (uvi->tris.buffer) fwrite(uvi->tris.buffer, sizeof(size_t), uvi->tris.count, f);
        if (uvi->uvs) fwrite(uvi->uvs, sizeof(LightmapUV), uvi->tris.count, f);
        fwrite(&uvi->normal, sizeof(vec3), 1, f);
        fwrite(&uvi->rect, sizeof(LightmapRect), 1, f);

    }

    
    fwrite(&GLOBALS.world_min, sizeof(vec3), 1, f);
    fwrite(&GLOBALS.world_size, sizeof(vec3), 1, f);
    fwrite(&GLOBALS.probe_tex_size, sizeof(ivec3), 1, f);
    fwrite(&GLOBALS.probe_spacing, sizeof(float), 1, f);
    fwrite(&GLOBALS.WORLD_WIDTH, sizeof(int), 1, f);
    fwrite(&GLOBALS.WORLD_HEIGHT, sizeof(int), 1, f);
    fwrite(&GLOBALS.WORLD_DEPTH, sizeof(int), 1, f);
    fwrite(&GLOBALS.noclip, sizeof(bool), 1, f);
    // fwrite(&GLOBALS.keyboard_locked, sizeof(bool), 1, f);





    printf("IMAGES\n");

    

    // Images
    Image * images = CAST_VEC(GLOBALS.images, Image);
    fwrite(&GLOBALS.images.count, sizeof(size_t), 1, f);
    for (int i = DEFAULT_TEXTURES; i < GLOBALS.images.count; i++)
    {

        Image * image = &images[i];
        fwrite(image->path, sizeof(char), 256, f);
        fwrite(image->mime_type, sizeof(char), 256, f);
        fwrite(image->name, sizeof(char), 256, f);
        fwrite(&image->min_filter, sizeof(GLint), 1, f);
        fwrite(&image->mag_filter, sizeof(GLint), 1, f);
        fwrite(&image->wrap_s, sizeof(GLint), 1, f);
        fwrite(&image->wrap_t, sizeof(GLint), 1, f);
        fwrite(&image->sRGB, sizeof(bool), 1, f);

    }
    printf("MATERIALS\n");
    
    // Materials
    Material * mats = CAST_VEC(GLOBALS.materials, Material);
    fwrite(&GLOBALS.materials.count, sizeof(size_t), 1, f);
    for (int i = 0; i < GLOBALS.materials.count; i++)
    {

        Material * mat = &mats[i];
        fwrite(mat->name, sizeof(char), 256, f);
        fwrite(&mat->fwd_shader, sizeof(int), 1, f);
        fwrite(&mat->gbuffer_shader, sizeof(int), 1, f);

        fwrite(&mat->ka, sizeof(vec4), 1, f);
        fwrite(&mat->kd, sizeof(vec4), 1, f);
        fwrite(&mat->ks, sizeof(vec4), 1, f);
        fwrite(&mat->ke, sizeof(vec4), 1, f);

        fwrite(mat->absorption_coefficients, sizeof(float), AUDIO_NUM_BANDS, f);
        fwrite(mat->scattering_coefficients, sizeof(float), AUDIO_NUM_BANDS, f);

        fwrite(&mat->diffuse_tex, sizeof(int), 1, f);
        fwrite(&mat->normal_tex, sizeof(int), 1, f);
        fwrite(&mat->emissive_tex, sizeof(int), 1, f);
        fwrite(&mat->arm_tex, sizeof(int), 1, f);
        fwrite(&mat->ao_tex, sizeof(int), 1, f);
        fwrite(&mat->mr_tex, sizeof(int), 1, f);
        
        fwrite(&mat->normal_scale, sizeof(float), 1, f);
        fwrite(&mat->ao_strength, sizeof(float), 1, f);
        fwrite(&mat->metallic, sizeof(float), 1, f);
        fwrite(&mat->roughness, sizeof(float), 1, f);
        fwrite(&mat->alpha_mode, sizeof(int), 1, f);
        fwrite(&mat->alpha_cutoff, sizeof(float), 1, f);
        fwrite(&mat->emissive_strength, sizeof(float), 1, f);
        fwrite(&mat->double_sided, sizeof(bool), 1, f);
        
    }


    printf("MESHES\n");
    // Meshes
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    fwrite(&GLOBALS.meshes.count, sizeof(size_t), 1, f);
    for (int i = 0; i < GLOBALS.meshes.count; i++)
    {

        Mesh * m = &meshes[i];
        fwrite(m->name, sizeof(char), 256, f);
        fwrite(&m->bvh_prim, sizeof(bool), 1, f);
        fwrite(&m->asset_id, sizeof(size_t), 1, f);
        fwrite(&m->id, sizeof(size_t), 1, f);
        fwrite(&m->static_mesh, sizeof(bool), 1, f);
        fwrite(&m->aabb, sizeof(AABB), 1, f);

        fwrite(&m->submeshes.count, sizeof(size_t), 1, f);
        if (m->submeshes.buffer) fwrite(m->submeshes.buffer, sizeof(SubMesh), m->submeshes.count, f);

        fwrite(&m->hulls.count, sizeof(size_t), 1, f);
        ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);
        for (int j = 0; j < m->hulls.count; j++)
        {

            ConvexHull * h = &hulls[j];
            fwrite(&h->vertices.count, sizeof(size_t), 1, f);
            if (h->vertices.buffer) fwrite(h->vertices.buffer, sizeof(vec3), h->vertices.count, f);
            fwrite(&h->faces.count, sizeof(size_t), 1, f);
            if (h->faces.buffer) fwrite(h->faces.buffer, sizeof(MeshFace), h->faces.count, f);
            fwrite(&h->hull_faces.count, sizeof(size_t), 1, f);
            if (h->hull_faces.buffer) fwrite(h->hull_faces.buffer, sizeof(HullFace), h->hull_faces.count, f);
            fwrite(&h->centroid, sizeof(vec3), 1, f);
            fwrite(&h->aabb, sizeof(AABB), 1, f);
            
        }


        fwrite(&m->vertices.count, sizeof(size_t), 1, f);
        if (m->vertices.buffer) fwrite(m->vertices.buffer, sizeof(Point), m->vertices.count, f);
        
        fwrite(&m->tris.count, sizeof(size_t), 1, f);
        if (m->tris.buffer) fwrite(m->tris.buffer, sizeof(MeshFace), m->tris.count, f);


        TriangleAdjacency * tri_adj = m->tri_adjacency;
        for (int j = 0; j < m->tris.count; j++)
        {

            TriangleAdjacency * ta = &tri_adj[j];
            fwrite(&ta->adjacent_indices.count, sizeof(size_t), 1, f);
            if (ta->adjacent_indices.buffer) fwrite(ta->adjacent_indices.buffer, sizeof(size_t), ta->adjacent_indices.count, f);

        }

        if (m->static_mesh)
        {
            
            fwrite(&m->nodes.count, sizeof(size_t), 1, f);
            if (m->nodes.buffer) fwrite(m->nodes.buffer, sizeof(BVHNode), m->nodes.count, f);

            fwrite(&m->tri_aabbs.count, sizeof(size_t), 1, f);
            if (m->tri_aabbs.buffer) fwrite(m->tri_aabbs.buffer, sizeof(AABB), m->tri_aabbs.count, f);

            if (m->bvh_indices) fwrite(m->bvh_indices, sizeof(size_t), m->tris.count, f);

        }

    }
    printf("MESH NODES\n");

    // Mesh Nodes
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    fwrite(&GLOBALS.mesh_nodes.count, sizeof(size_t), 1, f);
    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        write_mesh_node(&mns[i], f);
    }

    printf("SKINS\n");
    // Skins
    
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    fwrite(&GLOBALS.skins.count, sizeof(size_t), 1, f);
    for (int i = 0; i < GLOBALS.skins.count; i++)
    {

        GLTFSkin * skin = &skins[i];
        write_skin(skin, f);

    }

    

    printf("ASSETS\n");
    // Assets
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
    fwrite(&GLOBALS.assets.count, sizeof(size_t), 1, f);
    for (int i = 0; i < GLOBALS.assets.count; i++)
    {

        Asset * asset = &assets[i];

        fwrite(asset->path, sizeof(char), 256, f);

        fwrite(&asset->mesh_indices.count, sizeof(size_t), 1, f);
        if (asset->mesh_indices.buffer) fwrite(asset->mesh_indices.buffer, sizeof(int), asset->mesh_indices.count, f);


        fwrite(&asset->node_count, sizeof(size_t), 1, f);
        printf("NODE COUNT: %zu\n", asset->node_count);
        for (int j = 0; j < asset->node_count; j++)
        {
            write_mesh_node(&asset->nodes[j], f);
        }
        
        fwrite(&asset->skin_count, sizeof(size_t), 1, f);
        for (int j = 0; j < asset->skin_count; j++)
        {
            write_skin(&asset->skins[j], f);
        }

        fwrite(&asset->animation_count, sizeof(size_t), 1, f);
        for (int j = 0; j < asset->animation_count; j++)
        {

            Animation * anim = &asset->animations[j];
            fwrite(anim->name, sizeof(char), 256, f);
            fwrite(&anim->asset_id, sizeof(int), 1, f);
            fwrite(&anim->sampler_count, sizeof(int), 1, f);
            for (int k = 0; k < anim->sampler_count; k++)
            {
                AnimationSampler * sampler = &anim->samplers[k];
                fwrite(&sampler->count, sizeof(int), 1, f);
                if (sampler->times) fwrite(sampler->times, sizeof(float), sampler->count, f);
                fwrite(&sampler->elem_count, sizeof(int), 1, f);
                if (sampler->values) fwrite(sampler->values, sizeof(float), sampler->elem_count, f);
                fwrite(&sampler->interpolation, sizeof(int), 1, f);
                
            }

            fwrite(&anim->channel_count, sizeof(int), 1, f);
            if (anim->channels) fwrite(anim->channels, sizeof(GLTFAnimationChannel), anim->channel_count, f);
            // fwrite(&anim->event_count, sizeof(int), 1, f);
            // fwrite(anim->events, sizeof(AnimationEvent), anim->event_count, f);

            
        }

        fwrite(&asset->asset_id, sizeof(int), 1, f);
        fwrite(&asset->aabb, sizeof(AABB), 1, f);

    }

    

    printf("ASSET INSTANCES\n");
    // Asset Instances
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++)
    {

        AssetInstance * ai = &ASSET_INSTANCES[i];
        fwrite(&ai->free, sizeof(bool), 1, f);

        if (ai->free) continue;

        fwrite(&ai->first_skin, sizeof(size_t), 1, f);
        fwrite(&ai->skin_count, sizeof(size_t), 1, f);

        fwrite(&ai->asset_id, sizeof(int), 1, f);

        fwrite(&ai->node_count, sizeof(size_t), 1, f);

        fwrite(&ai->aabb, sizeof(AABB), 1, f);
        
    }

    printf("ENTITIES\n");
    // Enitites
    for (int i = 0; i < MAX_ENTITIES; i++)
    {

        Entity * ent = &ENTITIES[i];
        fwrite(&ent->free, sizeof(bool), 1, f);
        
        if (ent->free) continue;

        fwrite(&ent->id, sizeof(size_t), 1, f);
        fwrite(&ent->mesh_node, sizeof(int), 1, f);
        fwrite(&ent->skin, sizeof(int), 1, f);
        fwrite(&ent->aabb, sizeof(AABB), 1, f);

        
        if (ent->instance)
        {
            int id = ent->instance->id;
            fwrite(&id, sizeof(int), 1, f);
        }
        else
        {
            int n1 = -1;
            fwrite(&n1, sizeof(int), 1, f);
        }

        if (ent->pc)
        {
            
            int id = ent->pc->id;
            fwrite(&id, sizeof(int), 1, f);
            
        }
        else
        {
            int n1 = -1;
            fwrite(&n1, sizeof(int), 1, f);
            
        }

        // fwrite(&ent->audio_emitter_count, sizeof(int), 1, f);
        
        
    }

    printf("PCS\n");
    // Physics Components

    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {

        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];
        fwrite(&pc->free, sizeof(bool), 1, f);
        if (pc->free) continue;


        fwrite(&pc->id, sizeof(size_t), 1, f);
        fwrite(&pc->at_rest, sizeof(bool), 1, f);
        fwrite(&pc->player_pc, sizeof(bool), 1, f);
        fwrite(&pc->mass, sizeof(float), 1, f);
        fwrite(&pc->restitution, sizeof(float), 1, f);
        fwrite(&pc->mu, sizeof(float), 1, f);

        fwrite(&pc->vel, sizeof(vec3), 1, f);
        fwrite(&pc->angular_velocity, sizeof(vec3), 1, f);
        fwrite(&pc->center_of_mass, sizeof(vec3), 1, f);
        fwrite(&pc->inverse_inertia, sizeof(mat3), 1, f);
        fwrite(&pc->inverse_world, sizeof(mat3), 1, f);


        fwrite(&pc->aabb, sizeof(AABB), 1, f);

        fwrite(&pc->cd, sizeof(CollisionData), 1, f);

        if (pc->ent)
        {
            
            int id = pc->ent->id;
            fwrite(&id, sizeof(int), 1, f);
        }
        else
        {
            
            int n1 = -1;
            fwrite(&n1, sizeof(int), 1, f);
        }

        
    }

    
    for (int i = 0; i < MAX_ANIMATION_COMPONENTS; i++)
    {

        AnimationComponent * ac = &ANIMATION_COMPONENTS[i];
        fwrite(&ac->free, sizeof(bool), 1, f);
        if (ac->free) continue;
        fwrite(&ac->track_count, sizeof(int), 1, f);

        for (int j = 0; j < ac->track_count; j++)
        {

            AnimationTrack * t = &ac->tracks[j];
            fwrite(&t->type, sizeof(AnimationTargetType), 1, f);
            fwrite(&t->loop, sizeof(bool), 1, f);
            fwrite(&t->active, sizeof(bool), 1, f);
            fwrite(&t->current_time, sizeof(float), 1, f);
            fwrite(&t->duration, sizeof(float), 1, f);
            fwrite(&t->kf_count, sizeof(int), 1, f);

            for (int k = 0; k < t->kf_count; k++)
            {

                AnimationKeyframe * kf = &t->kf[k];
                fwrite(&kf->time, sizeof(float), 1, f);
                switch(t->type)
                {
                    case ANIM_TARGET_FLOAT:
                        fwrite(&kf->val.v, sizeof(float), 1, f);
                        break;
                    case ANIM_TARGET_VEC3:
                        fwrite(&kf->val.v3, sizeof(vec3), 1, f);
                        break;
                    case ANIM_TARGET_INT:
                        fwrite(&kf->val.i, sizeof(int), 1, f);
                        break;
                    case ANIM_TARGET_BOOL:
                        break;
                    default:
                        break;
                }
               
            }

            fwrite(&t->data_type, sizeof(AnimationDataType), 1, f);
            fwrite(&t->data_index_offset, sizeof(size_t), 1, f);
            fwrite(&t->data_byte_offset, sizeof(size_t), 1, f);

        }

        if (ac->ent)
        {
            int id = ac->ent->id;
            fwrite(&id, sizeof(int), 1, f);
        }
        else
        {
            int n1 = -1;
            fwrite(&n1, sizeof(int), 1, f);
        }


    }

    printf("LIGHTS\n");
    // Lights
    // Light * lights = CAST_VEC(GLOBALS.lights, Light);
    // fwrite(&GLOBALS.lights.count, sizeof(size_t), 1, f);
    // for (int i = 0; i < GLOBALS.lights.count; i++)
    // {
    //     Light * l = &lights[i];
    //     fwrite(l, sizeof(Light), 1, f);
    // }
    

    Font * fonts = CAST_VEC(FONTS, Font);
    fwrite(&FONTS.count, sizeof(size_t), 1, f);
    for (int i = 0; i < FONTS.count; i++)
    {

        Font * font = &fonts[i];
        fwrite(font, sizeof(Font), 1, f);
        fwrite(&font->glyph_count, sizeof(size_t), 1, f);
        if (font->glyphs) fwrite(font->glyphs, sizeof(Glyph), font->glyph_count, f);
        
        
    }
    

    // TextBox * tbs = CAST_VEC(GLOBALS.text_boxes, TextBox);
    // fwrite(&GLOBALS.text_boxes.count, sizeof(size_t), 1, f);
    // for (int i = PERFORMANCE_OVERLAY_TEXT_BOX_COUNT; i < GLOBALS.text_boxes.count; i++)
    // {
    //     TextBox * tb = &tbs[i];
    //     fwrite(tb, sizeof(TextBox), 1, f);
    //     if (tb->text)
    //     {
            
    //         uint32_t len = strlen(tb->text);
    //         fwrite(&len, sizeof(uint32_t), 1, f);
    //         fwrite(tb->text, sizeof(char), len, f);
    //     }
    //     else
    //     {
    //         uint32_t len = 0;
    //         fwrite(&len, sizeof(uint32_t), 1, f);
    //     }
    // }

    // AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    // fwrite(&GLOBALS.audio_portals.count, sizeof(size_t), 1, f);
    // for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    // {

    //     AudioPortal * p = &portals[i];
    //     fwrite(&p->pos, sizeof(vec3), 1, f);
    //     fwrite(&p->adjacent_portals.count, sizeof(size_t), 1, f);
    //     if (p->adjacent_portals.buffer) fwrite(p->adjacent_portals.buffer, sizeof(int), p->adjacent_portals.count, f);
        
    // }



    

    fclose(f);
    
    
}


void read_mesh_node(MeshNode * mn, FILE * f)
{

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    // fread(mn->name, sizeof(char), 256, f);
    fread(&mn->active, sizeof(bool), 1, f);
    fread(&mn->id, sizeof(int), 1, f);
    fread(&mn->parent, sizeof(int), 1, f);
    fread(&mn->translation, sizeof(vec3), 1, f);
    fread(&mn->rotation, sizeof(vec4), 1, f);
    fread(&mn->scale, sizeof(vec3), 1, f);
    fread(&mn->local_matrix, sizeof(mat4), 1, f);
    fread(&mn->global_matrix, sizeof(mat4), 1, f);
    fread(&mn->mesh, sizeof(int), 1, f);
    fread(&mn->asset, sizeof(int), 1, f);
    fread(&mn->skin, sizeof(int), 1, f);
    fread(&mn->child_count, sizeof(int), 1, f);
    fread(mn->children, sizeof(int), mn->child_count, f);

    fread(&mn->static_mesh, sizeof(bool), 1, f);
    fread(&mn->physics_mode, sizeof(bool), 1, f);

    fread(&mn->skip_directional_uv_lighting, sizeof(bool), 1, f);

    if (mn->mesh != -1)
    {

        Mesh * m = &meshes[mn->mesh];
        if (m->static_mesh)
        {
            
            bool uv_map = false;
            fread(&uv_map, sizeof(bool), 1, f);
            if (uv_map)
            {
                mn->uv_map = malloc(sizeof(UVMapping) * m->tris.count);
                fread(mn->uv_map, sizeof(UVMapping), m->tris.count, f);
            }
            else
            {
                mn->uv_map = NULL;
            }
        }
        else
        {
            mn->uv_map = NULL;
        }

    }
    
    int pc_id = -1;
    fread(&pc_id, sizeof(int), 1, f);
    if (pc_id != -1)
    {
        
        mn->pc = &PHYSICS_COMPONENTS[pc_id];
    }
    else
    {
        mn->pc = NULL;
    }

    int ent_id = -1;
    fread(&ent_id, sizeof(int), 1, f);

    if (ent_id != -1)
    {
        mn->ent = &ENTITIES[ent_id];
    }
    else
    {
        mn->ent = NULL;
    }

    
}

void read_skin(GLTFSkin * skin, FILE * f)
{

    fread(&skin->joint_count, sizeof(int), 1, f);

    if (skin->joint_count > 0)
    {
        skin->joints = malloc(sizeof(int) * skin->joint_count);
        skin->final_mat = malloc(sizeof(mat4) * skin->joint_count);
        skin->inverse_bind = malloc(sizeof(mat4) * skin->joint_count);

        fread(skin->joints, sizeof(int), skin->joint_count, f);
        fread(skin->final_mat, sizeof(mat4), skin->joint_count, f);
        fread(skin->inverse_bind, sizeof(mat4), skin->joint_count, f);

    }

    fread(&skin->ibm_accessor, sizeof(int), 1, f);

    // animation buffer not here
    fread(&skin->animation_indices_count, sizeof(int), 1, f);
    fread(skin->animation_indices, sizeof(int), skin->animation_indices_count, f);
   
    
}


void read_vector(Vector * v, size_t byte_size, FILE * f)
{
    if (v->buffer)
    {
        free(v->buffer);
        v->buffer = NULL;
        init_vector(v, 0, byte_size);
    }

    fread(&v->count, sizeof(size_t), 1, f);
    
    if (v->count > 0)
    {
        v->buffer = malloc(byte_size * v->count);
        v->capacity = v->count;
        fread(v->buffer, byte_size, v->count, f);
    }
    else
    {
        init_vector(v, 0, byte_size);
    }
    
}

void read_alloc_vector(Vector * v, size_t byte_size, FILE * f)
{
    
    if (v->buffer)
    {
        free(v->buffer);
        v->buffer = NULL;
        init_vector(v, 0, byte_size);
    }

    fread(&v->count, sizeof(size_t), 1, f);

    if (v->count > 0)
    {
        v->buffer = malloc(byte_size * v->count);
        v->capacity = v->count;
    }
    else
    {
        init_vector(v, 0, byte_size);
    }
}


void free_mesh_node(MeshNode * mn)
{
    
    if (mn->uv_map) free(mn->uv_map);
}

void free_skin(GLTFSkin * skin)
{
    if (skin->joints) free(skin->joints);
    if (skin->final_mat) free(skin->final_mat);
    if (skin->inverse_bind) free(skin->inverse_bind);
    
}

void scene_load(const char * name)
{

    char scene_name[256];
    strcpy(scene_name, scene_registry.directory);
    strcat(scene_name, name);
    strcat(scene_name, ".bin");

    char lm_name[256];
    strcpy(lm_name, lightmap_registry.directory);
    strcat(lm_name, name);
    strcat(lm_name, "_lm.bin");

    char light_name[256];
    strcpy(light_name, light_registry.directory);
    strcat(light_name, name);
    strcat(light_name, "_lights.bin");

    char aportal_name[256];
    strcpy(aportal_name, aportal_registry.directory);
    strcat(aportal_name, name);
    strcat(aportal_name, "_aportals.bin");

    char rprobe_name[256];
    strcpy(rprobe_name, rprobe_registry.directory);
    strcat(rprobe_name, name);
    strcat(rprobe_name, "_rprobes.bin");

    char lprobe_name[256];
    strcpy(lprobe_name, lprobe_registry.directory);
    strcat(lprobe_name, name);
    strcat(lprobe_name, "_lprobes.bin");

    char aprobe_name[256];
    strcpy(aprobe_name, aprobe_registry.directory);
    strcat(aprobe_name, name);
    strcat(aprobe_name, "_aprobes.bin");

    char scene_apv_name[256];
    strcpy(scene_apv_name, scene_apv_registry.directory);
    strcat(scene_apv_name, name);
    strcat(scene_apv_name, "_scene_apvs.bin");

    
    FILE * f = fopen(scene_name, "rb");

    printf("%s\n", scene_name);
    if (!f) return;

    load_lightmap_from_binary(&GLOBALS.scene_data.lightmap_data, lm_name);
    
    load_aportals(aportal_name, &GLOBALS.audio_portals);

    load_aprobes(aprobe_name);

    load_rprobes(rprobe_name, &GLOBALS.scene_data.reflection_probes);

    load_vector(light_name, &GLOBALS.lights);
    
    printf("GLOBALS\n");
    fread(GLOBALS.scene_data.name, sizeof(char), 256, f);

    fread(&GLOBALS.scene_data.aabb, sizeof(AABB), 1, f);

    read_vector(&GLOBALS.scene_data.bvh_nodes, sizeof(BVHNode), f);

    // fread(&GLOBALS.scene_data.bvh_nodes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.bvh_nodes.count > 0)
    // {
    //     GLOBALS.scene_data.bvh_nodes.buffer = malloc(sizeof(BVHNode) * GLOBALS.scene_data.bvh_nodes.count);
    //     GLOBALS.scene_data.bvh_nodes.capacity = GLOBALS.scene_data.bvh_nodes.count;
    //     fread(GLOBALS.scene_data.bvh_nodes.buffer, sizeof(BVHNode), GLOBALS.scene_data.bvh_nodes.count, f);
    // }
    // else
    // {
    //     init_vector(&GLOBALS.scene_data.bvh_nodes, 0, sizeof(BVHNode));
    // }

    // read_vector(&GLOBALS.scene_data.bvh_nodes, sizeof(BVHNode), f);
    read_vector(&GLOBALS.scene_data.light_bvh.nodes, sizeof(BVHNode), f);
    read_vector(&GLOBALS.scene_data.light_bvh.lights, sizeof(Light), f);
    if (GLOBALS.scene_data.light_bvh.lights.count > 0)
    {
        GLOBALS.scene_data.light_bvh.indices = malloc(sizeof(size_t) * GLOBALS.scene_data.light_bvh.lights.count);
        fread(GLOBALS.scene_data.light_bvh.indices, sizeof(size_t), GLOBALS.scene_data.light_bvh.lights.count, f);
    }
    read_vector(&GLOBALS.scene_data.leaf_indices, sizeof(size_t), f);
    // fread(&GLOBALS.scene_data.leaf_indices.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.leaf_indices.count > 0)
    // {
        
    //     GLOBALS.scene_data.leaf_indices.buffer = malloc(sizeof(size_t) * GLOBALS.scene_data.leaf_indices.count);
    //     GLOBALS.scene_data.leaf_indices.capacity = GLOBALS.scene_data.leaf_indices.count;
    //     fread(GLOBALS.scene_data.leaf_indices.buffer, sizeof(size_t), GLOBALS.scene_data.leaf_indices.count, f);
    // }
    // else
    // {
        
    //     init_vector(&GLOBALS.scene_data.leaf_indices, 0, sizeof(size_t));
    // }
    if (GLOBALS.scene_data.bvh_indices) free(GLOBALS.scene_data.bvh_indices);
    fread(&GLOBALS.scene_data.bvh_indices_count, sizeof(int), 1, f);
    if (GLOBALS.scene_data.bvh_indices_count > 0)
    {
        
        GLOBALS.scene_data.bvh_indices = malloc(sizeof(size_t) * GLOBALS.scene_data.bvh_indices_count);

        fread(GLOBALS.scene_data.bvh_indices, sizeof(size_t), GLOBALS.scene_data.bvh_indices_count, f);
    }
    else
    {
        
        GLOBALS.scene_data.bvh_indices = NULL;
    }
    
    // read_vector(&GLOBALS.scene_data.reflection_probes, sizeof(ReflectionProbe), f);
    // read_vector(&GLOBALS.scene_data.audio_probe_volumes, sizeof(ProbeVolume), f);
    // read_vector(&GLOBALS.scene_data.lighting_probe_volumes, sizeof(ProbeVolume), f);

    // reset_audio_pvs();
    // fread(GLOBALS.scene_data.APV_PATH, sizeof(char), 256, f);
    // if (GLOBALS.scene_data.APV_FILE)
    // {
    //     fclose(GLOBALS.scene_data.APV_FILE);
    //     GLOBALS.scene_data.APV_FILE = NULL;
    // }
    // GLOBALS.scene_data.APV_FILE = fopen(GLOBALS.scene_data.APV_PATH, "rb+");
    
    // fread(&GLOBALS.scene_data.reflection_probes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.scene_data.reflection_probes.count > 0)
    // {
        
    //     GLOBALS.scene_data.reflection_probes.buffer = malloc(sizeof(ReflectionProbe) * GLOBALS.scene_data.reflection_probes.count);
    //     GLOBALS.scene_data.reflection_probes.capacity = GLOBALS.scene_data.reflection_probes.count;
    //     fread(GLOBALS.scene_data.reflection_probes.buffer, sizeof(ReflectionProbe), GLOBALS.scene_data.reflection_probes.count, f);
    // }
    // else
    // {
        
    //     init_vector(&GLOBALS.scene_data.reflection_probes, 0, sizeof(size_t));
    // }

    printf("LD\n");

    LightmapData * ld = &GLOBALS.scene_data.lightmap_data;
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
        fread(&uvi->mesh_node, sizeof(int), 1, f);
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


    }

    
    fread(&GLOBALS.world_min, sizeof(vec3), 1, f);
    fread(&GLOBALS.world_size, sizeof(vec3), 1, f);
    fread(&GLOBALS.probe_tex_size, sizeof(ivec3), 1, f);
    fread(&GLOBALS.probe_spacing, sizeof(float), 1, f);
    fread(&GLOBALS.WORLD_WIDTH, sizeof(int), 1, f);
    fread(&GLOBALS.WORLD_HEIGHT, sizeof(int), 1, f);
    fread(&GLOBALS.WORLD_DEPTH, sizeof(int), 1, f);
    fread(&GLOBALS.noclip, sizeof(bool), 1, f);
    // fread(&GLOBALS.keyboard_locked, sizeof(bool), 1, f);




    

    printf("IMAGES\n");

    

    // Images
    size_t old_count = GLOBALS.images.count;
    Image * images = CAST_VEC(GLOBALS.images, Image);
    for (int i = DEFAULT_TEXTURES; i < GLOBALS.images.count; i++)
    {

        Image * image = &images[i];
        if (image->buf)
        {
            free(image->buf);
            image->buf = NULL;
        }
        if (image->i)
        {
            glDeleteTextures(1, &image->i);
        }
    }

    fread(&GLOBALS.images.count, sizeof(size_t), 1, f);
    if (GLOBALS.images.count > 0)
    {


        if (old_count < GLOBALS.images.count)
        {
            
            GLOBALS.images.capacity = GLOBALS.images.count;
            GLOBALS.images.buffer = realloc(GLOBALS.images.buffer, sizeof(Image) * GLOBALS.images.count);
        }

    }
    else
    {
        
        init_vector(&GLOBALS.images, 0, sizeof(Image));
    }
    images = CAST_VEC(GLOBALS.images, Image);
    for (int i = DEFAULT_TEXTURES; i < GLOBALS.images.count; i++)
    {

        Image * image = &images[i];
        fread(image->path, sizeof(char), 256, f);
        fread(image->mime_type, sizeof(char), 256, f);
        fread(image->name, sizeof(char), 256, f);
        fread(&image->min_filter, sizeof(GLint), 1, f);
        fread(&image->mag_filter, sizeof(GLint), 1, f);
        fread(&image->wrap_s, sizeof(GLint), 1, f);
        fread(&image->wrap_t, sizeof(GLint), 1, f);
        fread(&image->sRGB, sizeof(bool), 1, f);
        GLuint i = load_texture(image->path, image->min_filter, image->mag_filter, image->wrap_s, image->wrap_t, image->sRGB);

        // GLuint64 handle = 0;
        GLuint64 handle = glGetTextureHandleARB(i);

        glMakeTextureHandleResidentARB(handle);
        image->handle = handle;
        image->i = i;
        image->channels = 4;
        


    }
    printf("MATERIALS\n");
    if (GLOBALS.materials.buffer)
    {
        free(GLOBALS.materials.buffer);
        init_vector(&GLOBALS.materials, 0, sizeof(Material));
        
    }
    
    // Materials
    fread(&GLOBALS.materials.count, sizeof(size_t), 1, f);
    if (GLOBALS.materials.count > 0)
    {
        

        GLOBALS.materials.buffer = malloc(sizeof(Material) * GLOBALS.materials.count);
        GLOBALS.materials.capacity = GLOBALS.materials.count;

        // fread(GLOBALS.materials.buffer, sizeof(Material), GLOBALS.materials.count, f);
    }
    else
    {
        init_vector(&GLOBALS.materials, 0, sizeof(Material));
    }
    Material * mats = CAST_VEC(GLOBALS.materials, Material);
    for (int i = 0; i < GLOBALS.materials.count; i++)
    {

        Material * mat = &mats[i];
        fread(mat->name, sizeof(char), 256, f);
        fread(&mat->fwd_shader, sizeof(int), 1, f);
        fread(&mat->gbuffer_shader, sizeof(int), 1, f);

        fread(&mat->ka, sizeof(vec4), 1, f);
        fread(&mat->kd, sizeof(vec4), 1, f);
        fread(&mat->ks, sizeof(vec4), 1, f);
        fread(&mat->ke, sizeof(vec4), 1, f);


        fread(mat->absorption_coefficients, sizeof(float), AUDIO_NUM_BANDS, f);
        fread(mat->scattering_coefficients, sizeof(float), AUDIO_NUM_BANDS, f);
        
        fread(&mat->diffuse_tex, sizeof(int), 1, f);
        fread(&mat->normal_tex, sizeof(int), 1, f);
        fread(&mat->emissive_tex, sizeof(int), 1, f);
        fread(&mat->arm_tex, sizeof(int), 1, f);
        fread(&mat->ao_tex, sizeof(int), 1, f);
        fread(&mat->mr_tex, sizeof(int), 1, f);
        
        fread(&mat->normal_scale, sizeof(float), 1, f);
        fread(&mat->ao_strength, sizeof(float), 1, f);
        fread(&mat->metallic, sizeof(float), 1, f);
        fread(&mat->roughness, sizeof(float), 1, f);
        fread(&mat->alpha_mode, sizeof(int), 1, f);
        fread(&mat->alpha_cutoff, sizeof(float), 1, f);
        fread(&mat->emissive_strength, sizeof(float), 1, f);
        fread(&mat->double_sided, sizeof(bool), 1, f);
        
    }


    printf("MESHES\n");
    // Meshes
    if (GLOBALS.meshes.buffer)
    {
        Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
        for (int i = 0; i < GLOBALS.meshes.count; i++)
        {

            Mesh * m = &meshes[i];
            if (m->submeshes.buffer) free(m->submeshes.buffer);
            if (m->hulls.buffer) free(m->hulls.buffer);
            if (m->nodes.buffer) free(m->nodes.buffer);
            if (m->vertices.buffer) free(m->vertices.buffer);
            if (m->tris.buffer) free(m->tris.buffer);
            if (m->tri_aabbs.buffer) free(m->tri_aabbs.buffer);
            if (m->tri_adjacency)
            {
                for (int j = 0; j < m->tris.count; j++)
                {
                    if (m->tri_adjacency[j].adjacent_indices.buffer)
                    {
                        free(m->tri_adjacency[j].adjacent_indices.buffer);
                    }
                }
                free(m->tri_adjacency);
                
            }
            if (m->bvh_indices)
            {
                free(m->bvh_indices);
            }
        }
    
        free(GLOBALS.meshes.buffer);
        init_vector(&GLOBALS.meshes, 0, sizeof(Mesh));
        
    }
    fread(&GLOBALS.meshes.count, sizeof(size_t), 1, f);
    if (GLOBALS.meshes.count > 0)
    {
        

        GLOBALS.meshes.buffer = malloc(sizeof(Mesh) * GLOBALS.meshes.count);
        GLOBALS.meshes.capacity = GLOBALS.meshes.count;

        // fread(GLOBALS.meshes.buffer, sizeof(Mesh), GLOBALS.meshes.count, f);
    }
    else
    {
        
        init_vector(&GLOBALS.meshes, 0, sizeof(Mesh));
    }
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    for (int i = 0; i < GLOBALS.meshes.count; i++)
    {

        Mesh * m = &meshes[i];
        fread(m->name, sizeof(char), 256, f);
        fread(&m->bvh_prim, sizeof(bool), 1, f);
        fread(&m->asset_id, sizeof(size_t), 1, f);
        fread(&m->id, sizeof(size_t), 1, f);
        fread(&m->static_mesh, sizeof(bool), 1, f);
        fread(&m->aabb, sizeof(AABB), 1, f);

        // fread(&m->submeshes.count, sizeof(size_t), 1, f);
        m->submeshes.buffer = NULL;
        read_vector(&m->submeshes, sizeof(SubMesh), f);
        // if (m->submeshes.count > 0)
        // {
        
        //     m->submeshes.buffer = malloc(sizeof(SubMesh) * m->submeshes.count);
        //     m->submeshes.capacity = m->submeshes.count;

        //     fread(m->submeshes.buffer, sizeof(SubMesh), m->submeshes.count, f);
        // }
        // else
        // {
        
        //     init_vector(&m->submeshes, 0, sizeof(SubMesh));
        // }


        fread(&m->hulls.count, sizeof(size_t), 1, f);
        if (m->hulls.count> 0)
        {
    

            m->hulls.buffer = malloc(sizeof(ConvexHull) * m->hulls.count);
            m->hulls.capacity = m->hulls.count;

            // fread(m->hulls.buffer, sizeof(ConvexHull), m->hulls.count, f);
        }
        else
        {
    
            init_vector(&m->hulls, 0, sizeof(ConvexHull));
        }
        ConvexHull * hulls = CAST_VEC(m->hulls, ConvexHull);
        for (int j = 0; j < m->hulls.count; j++)
        {

            ConvexHull * h = &hulls[j];
            h->vertices.buffer = NULL;
            read_vector(&h->vertices, sizeof(vec3), f);
            // fread(&h->vertices.count, sizeof(size_t), 1, f);
            // if (h->vertices.count > 0)
            // {
    
            //     h->vertices.buffer = malloc(sizeof(vec3) * h->vertices.count);
            //     h->vertices.capacity = h->vertices.count;

            //     fread(h->vertices.buffer, sizeof(vec3), h->vertices.count, f);
            // }
            // else
            // {
    
            //     init_vector(&h->vertices, 0, sizeof(vec3));
            // }
            h->faces.buffer = NULL;
            read_vector(&h->faces, sizeof(MeshFace), f);
            // fread(&h->faces.count, sizeof(size_t), 1, f);
            // if (h->faces.count > 0)
            // {
        
            //     h->faces.buffer = malloc(sizeof(MeshFace) * h->faces.count);
            //     h->faces.capacity = h->faces.count;

            //     fread(h->faces.buffer, sizeof(MeshFace), h->faces.count, f);
            // }
            // else
            // {
        
            //     init_vector(&h->faces, 0, sizeof(MeshFace));
            // }
            h->hull_faces.buffer = NULL;
            read_vector(&h->hull_faces, sizeof(HullFace), f);
            // fread(&h->hull_faces.count, sizeof(size_t), 1, f);
            // if (h->hull_faces.count > 0)
            // {
        
            //     h->hull_faces.buffer = malloc(sizeof(HullFace) * h->hull_faces.count);
            //     h->hull_faces.capacity = h->hull_faces.count;

            //     fread(h->hull_faces.buffer, sizeof(HullFace), h->hull_faces.count, f);
            // }
            // else
            // {
        
            //     init_vector(&h->hull_faces, 0, sizeof(HullFace));
            // }
            fread(&h->centroid, sizeof(vec3), 1, f);
            fread(&h->aabb, sizeof(AABB), 1, f);
            
        }


        m->vertices.buffer = NULL;
        read_vector(&m->vertices, sizeof(Point), f);
        // fread(&m->vertices.count, sizeof(size_t), 1, f);
        // if (m->vertices.count > 0)
        // {
    
        //     m->vertices.buffer = malloc(sizeof(Point) * m->vertices.count);
        //     m->vertices.capacity = m->vertices.count;

        //     fread(m->vertices.buffer, sizeof(Point), m->vertices.count, f);
        // }
        // else
        // {
    
        //     init_vector(&m->vertices, 0, sizeof(Point));
        // }
        
        m->tris.buffer = NULL;
        read_vector(&m->tris, sizeof(MeshFace), f);
        // fread(&m->tris.count, sizeof(size_t), 1, f);
        // if (m->tris.count > 0)
        // {
    
        //     m->tris.buffer = malloc(sizeof(MeshFace) * m->tris.count);
        //     m->tris.capacity = m->tris.count;

        //     fread(m->tris.buffer, sizeof(MeshFace), m->tris.count, f);
        // }
        // else
        // {
    
        //     init_vector(&m->tris, 0, sizeof(MeshFace));
        // }


        m->tri_adjacency = malloc(sizeof(TriangleAdjacency) * m->tris.count);
        TriangleAdjacency * tri_adj = m->tri_adjacency;
        for (int j = 0; j < m->tris.count; j++)
        {

            TriangleAdjacency * ta = &tri_adj[j];
            ta->adjacent_indices.buffer = NULL;
            read_vector(&ta->adjacent_indices, sizeof(size_t), f);
            // fread(&ta->adjacent_indices.count, sizeof(size_t), 1, f);
            // if (ta->adjacent_indices.count > 0)
            // {
    
            //     ta->adjacent_indices.buffer = malloc(sizeof(size_t) * ta->adjacent_indices.count);
            //     ta->adjacent_indices.capacity = ta->adjacent_indices.count;

            //     fread(ta->adjacent_indices.buffer, sizeof(size_t), ta->adjacent_indices.count, f);
            // }
            // else
            // {
    
            //     init_vector(&ta->adjacent_indices, 0, sizeof(size_t));
            // }

        }

        if (m->static_mesh)
        {
            
            m->nodes.buffer = NULL;
            read_vector(&m->nodes, sizeof(BVHNode), f);

            m->tri_aabbs.buffer = NULL;
            read_vector(&m->tri_aabbs, sizeof(AABB), f);

            m->bvh_indices = malloc(sizeof(size_t) * m->tris.count);

            fread(m->bvh_indices, sizeof(size_t), m->tris.count, f);

        }
        else
        {
            init_vector(&m->nodes, 0, sizeof(BVHNode));
            init_vector(&m->tri_aabbs, 0, sizeof(AABB));
            m->bvh_indices = NULL;
        }

    }
    printf("MESH NODES\n");

    if (GLOBALS.mesh_nodes.buffer)
    {

        MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
        for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
        {
            MeshNode * mn = &mns[i];
            if (mn->uv_map) free(mn->uv_map);
            mn->uv_map = NULL;
        }
        
        free(GLOBALS.mesh_nodes.buffer);
        init_vector(&GLOBALS.mesh_nodes, 0, sizeof(MeshNode));
        
    }
    // Mesh Nodes
    fread(&GLOBALS.mesh_nodes.count, sizeof(size_t), 1, f);
    if (GLOBALS.mesh_nodes.count> 0)
    {
        

        GLOBALS.mesh_nodes.buffer = malloc(sizeof(MeshNode) * GLOBALS.mesh_nodes.count);
        GLOBALS.mesh_nodes.capacity = GLOBALS.mesh_nodes.count;

    }
    else
    {
        
        init_vector(&GLOBALS.mesh_nodes, 0, sizeof(MeshNode));
    }
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    for (int i = 0; i < GLOBALS.mesh_nodes.count; i++)
    {
        read_mesh_node(&mns[i], f);
    }

    printf("SKINS\n");
    // Skins
    
    if (GLOBALS.skins.buffer)
    {

        GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
        for (int i = 0; i < GLOBALS.skins.count; i++)
        {

            GLTFSkin * skin = &skins[i];
            if (skin->joints) free(skin->joints);
            if (skin->final_mat) free(skin->final_mat);
            if (skin->inverse_bind) free(skin->inverse_bind);

        }

        
        free(GLOBALS.skins.buffer);
        init_vector(&GLOBALS.skins, 0, sizeof(GLTFSkin));
        
    }
    fread(&GLOBALS.skins.count, sizeof(size_t), 1, f);
    if (GLOBALS.skins.count> 0)
    {
        

        GLOBALS.skins.buffer = malloc(sizeof(GLTFSkin) * GLOBALS.skins.count);
        GLOBALS.skins.capacity = GLOBALS.skins.count;

    }
    else
    {
        
        init_vector(&GLOBALS.skins, 0, sizeof(GLTFSkin));
    }
    GLTFSkin * skins = CAST_VEC(GLOBALS.skins, GLTFSkin);
    for (int i = 0; i < GLOBALS.skins.count; i++)
    {

        GLTFSkin * skin = &skins[i];
        read_skin(skin, f);

    }

    

    printf("ASSETS\n");
    // Assets
    if (GLOBALS.assets.buffer)
    {

        Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
        for (int i = 0; i < GLOBALS.assets.count; i++)
        {
            Asset * asset = &assets[i];
            for (int j = 0; j < asset->node_count; j++)
            {
                free_mesh_node(&asset->nodes[j]);
            }
            if (asset->nodes) free(asset->nodes);
            for (int j = 0; j < asset->skin_count; j++)
            {
                free_skin(&asset->skins[j]);
            }
            if (asset->skins) free(asset->skins);
            
            for (int j = 0; j < asset->animation_count; j++)
            {
                Animation * anim = &asset->animations[j];
                for (int k = 0; k < anim->sampler_count; k++)
                {
                    AnimationSampler * s = &anim->samplers[k];
                    if (s->times) free(s->times);
                    if (s->values) free(s->values);
                    
                }
                if (anim->samplers) free(anim->samplers);

                if (anim->channels) free(anim->channels);
                
            }

            if (asset->animations) free(asset->animations);
        }

        free(GLOBALS.assets.buffer);
        init_vector(&GLOBALS.assets, 0, sizeof(Asset));
        
    }
    fread(&GLOBALS.assets.count, sizeof(size_t), 1, f);
    if (GLOBALS.assets.count> 0)
    {
        

        GLOBALS.assets.buffer = malloc(sizeof(Asset) * GLOBALS.assets.count);
        GLOBALS.assets.capacity = GLOBALS.assets.count;

        // fread(GLOBALS.assets.buffer, sizeof(Asset), GLOBALS.assets.count, f);
    }
    else
    {
        
        init_vector(&GLOBALS.assets, 0, sizeof(Asset));
    }
    Asset * assets = CAST_VEC(GLOBALS.assets, Asset);
    for (int i = 0; i < GLOBALS.assets.count; i++)
    {

        Asset * asset = &assets[i];

        fread(asset->path, sizeof(char), 256, f);

        fread(&asset->mesh_indices.count, sizeof(size_t), 1, f);
        if (asset->mesh_indices.count > 0)
        {

            asset->mesh_indices.buffer = malloc(sizeof(int) * asset->mesh_indices.count);
            asset->mesh_indices.capacity = asset->mesh_indices.count;

            fread(asset->mesh_indices.buffer, sizeof(int), asset->mesh_indices.count, f);
        }
        else
        {

            init_vector(&asset->mesh_indices, 0, sizeof(int));
        }


        fread(&asset->node_count, sizeof(size_t), 1, f);
        printf("NODE COUNT: %zu\n", asset->node_count);
        if (asset->node_count > 0)
        {
            
            asset->nodes = malloc(sizeof(MeshNode) * asset->node_count);
            for (int j = 0; j < asset->node_count; j++)
            {
                read_mesh_node(&asset->nodes[j], f);
            }
        }
        else
        {
            asset->nodes = NULL;
        }
        
        fread(&asset->skin_count, sizeof(size_t), 1, f);
        if (asset->skin_count > 0)
        {
            
            asset->skins = malloc(sizeof(GLTFSkin) * asset->skin_count);
            for (int j = 0; j < asset->skin_count; j++)
            {
                read_skin(&asset->skins[j], f);
            }
        }
        else
        {
            asset->skins = NULL;
            
        }

        fread(&asset->animation_count, sizeof(size_t), 1, f);
        if (asset->animation_count > 0)
        {
            asset->animations = malloc(sizeof(Animation) * asset->animation_count);
            for (int j = 0; j < asset->animation_count; j++)
            {

                Animation * anim = &asset->animations[j];
                fread(anim->name, sizeof(char), 256, f);
                fread(&anim->asset_id, sizeof(int), 1, f);
                fread(&anim->sampler_count, sizeof(int), 1, f);
                for (int k = 0; k < anim->sampler_count; k++)
                {
                    AnimationSampler * sampler = &anim->samplers[k];
                    fread(&sampler->count, sizeof(int), 1, f);
                    if (sampler->count > 0)
                    {
                    
                        sampler->times = malloc(sizeof(float) * sampler->count);

                        fread(sampler->times, sizeof(float), sampler->count, f);
                    }
                    else
                    {
                        sampler->times = NULL;
                    }
                    fread(&sampler->elem_count, sizeof(int), 1, f);
                    if (sampler->elem_count > 0)
                    {
                    
                        sampler->values = malloc(sizeof(float) * sampler->elem_count);

                        fread(sampler->values, sizeof(float), sampler->elem_count, f);
                    }
                    else
                    {
                        sampler->values  = NULL;
                    }
                    fread(&sampler->interpolation, sizeof(int), 1, f);
                
                }

                fread(&anim->channel_count, sizeof(int), 1, f);
                if (anim->channel_count> 0)
                {
                
                    anim->channels= malloc(sizeof(GLTFAnimationChannel) * anim->channel_count);

                    fread(anim->channels, sizeof(GLTFAnimationChannel), anim->channel_count, f);
                }
                else
                {
                    anim->channels= NULL;
                }

                // fwrite(&anim->event_count, sizeof(int), 1, f);
                // fwrite(anim->events, sizeof(AnimationEvent), anim->event_count, f);

            
            }
            
        }
        else
        {
            asset->animations = NULL;
        }

        fread(&asset->asset_id, sizeof(int), 1, f);
        fread(&asset->aabb, sizeof(AABB), 1, f);

    }

    

    printf("ASSET INSTANCES\n");
    // Asset Instances
    for (int i = 0; i < MAX_ASSET_INSTANCES; i++)
    {

        AssetInstance * ai = &ASSET_INSTANCES[i];
        fread(&ai->free, sizeof(bool), 1, f);

        if (ai->free) continue;

        fread(&ai->first_skin, sizeof(size_t), 1, f);
        fread(&ai->skin_count, sizeof(size_t), 1, f);

        fread(&ai->asset_id, sizeof(int), 1, f);

        fread(&ai->node_count, sizeof(size_t), 1, f);

        fread(&ai->aabb, sizeof(AABB), 1, f);
        
    }

    printf("ENTITIES\n");
    // Enitites
    for (int i = 0; i < MAX_ENTITIES; i++)
    {

        Entity * ent = &ENTITIES[i];
        fread(&ent->free, sizeof(bool), 1, f);
        
        if (ent->free) continue;

        fread(&ent->id, sizeof(size_t), 1, f);
        fread(&ent->mesh_node, sizeof(int), 1, f);
        fread(&ent->skin, sizeof(int), 1, f);
        fread(&ent->aabb, sizeof(AABB), 1, f);

        
        int instance_id = -1;
        fread(&instance_id, sizeof(int), 1, f);
        if (instance_id != -1)
        {
            ent->instance = &ASSET_INSTANCES[instance_id];
        }
        else
        {
            ent->instance = NULL;
        }
        int pc_id = -1;
        fread(&pc_id, sizeof(int), 1, f);
        if (pc_id != -1)
        {
            ent->pc = &PHYSICS_COMPONENTS[pc_id];
        }
        else
        {
            ent->pc = NULL;
        }
    }

    printf("PCS\n");
    // Physics Components

    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {

        PhysicsComponent * pc = &PHYSICS_COMPONENTS[i];
        fread(&pc->free, sizeof(bool), 1, f);
        if (pc->free) continue;


        fread(&pc->id, sizeof(size_t), 1, f);
        fread(&pc->at_rest, sizeof(bool), 1, f);
        fread(&pc->player_pc, sizeof(bool), 1, f);
        fread(&pc->mass, sizeof(float), 1, f);
        fread(&pc->restitution, sizeof(float), 1, f);
        fread(&pc->mu, sizeof(float), 1, f);

        fread(&pc->vel, sizeof(vec3), 1, f);
        fread(&pc->angular_velocity, sizeof(vec3), 1, f);
        fread(&pc->center_of_mass, sizeof(vec3), 1, f);
        fread(&pc->inverse_inertia, sizeof(mat3), 1, f);
        fread(&pc->inverse_world, sizeof(mat3), 1, f);


        fread(&pc->aabb, sizeof(AABB), 1, f);

        fread(&pc->cd, sizeof(CollisionData), 1, f);

        int ent_id = -1;
        fread(&ent_id, sizeof(int), 1, f);
        if (ent_id != -1)
        {
            pc->ent = &ENTITIES[ent_id];
        }
        else
        {
            pc->ent = NULL;
        }

        
    }

    for (int i = 0; i < MAX_ANIMATION_COMPONENTS; i++)
    {

        AnimationComponent * ac = &ANIMATION_COMPONENTS[i];
        fread(&ac->free, sizeof(bool), 1, f);
        if (ac->free) continue;
        fread(&ac->track_count, sizeof(int), 1, f);

        for (int j = 0; j < ac->track_count; j++)
        {

            AnimationTrack * t = &ac->tracks[j];
            fread(&t->type, sizeof(AnimationTargetType), 1, f);
            fread(&t->loop, sizeof(bool), 1, f);
            fread(&t->active, sizeof(bool), 1, f);
            fread(&t->current_time, sizeof(float), 1, f);
            fread(&t->duration, sizeof(float), 1, f);
            fread(&t->kf_count, sizeof(int), 1, f);

            for (int k = 0; k < t->kf_count; k++)
            {

                AnimationKeyframe * kf = &t->kf[k];
                fread(&kf->time, sizeof(float), 1, f);
                switch(t->type)
                {
                    case ANIM_TARGET_FLOAT:
                        fread(&kf->val.v, sizeof(float), 1, f);
                        break;
                    case ANIM_TARGET_VEC3:
                        fread(&kf->val.v3, sizeof(vec3), 1, f);
                        break;
                    case ANIM_TARGET_INT:
                        fread(&kf->val.i, sizeof(int), 1, f);
                        break;
                    case ANIM_TARGET_BOOL:
                        break;
                    default:
                        break;
                }
               
            }

            fread(&t->data_type, sizeof(AnimationDataType), 1, f);
            fread(&t->data_index_offset, sizeof(size_t), 1, f);
            fread(&t->data_byte_offset, sizeof(size_t), 1, f);
            // NOTE leaving this at null, come back to it when necessary
            t->data = NULL;
            t->completion_data = NULL;

        }

        int ent_id = -1;
        fread(&ent_id, sizeof(int), 1, f);
        if (ent_id != -1)
        {
            ac->ent = &ENTITIES[ent_id];
        }
        else
        {
            ac->ent = NULL;
        }


    }
    
    printf("LIGHTS\n");
    // Lights
    
    // if (GLOBALS.lights.buffer)
    // {
    //     free(GLOBALS.lights.buffer);
    //     init_vector(&GLOBALS.lights, 0, sizeof(Asset));
        
    // }
    // fread(&GLOBALS.lights.count, sizeof(size_t), 1, f);
    // if (GLOBALS.lights.count> 0)
    // {
        

    //     GLOBALS.lights.buffer = malloc(sizeof(Light) * GLOBALS.lights.count);
    //     GLOBALS.lights.capacity = GLOBALS.lights.count;

    //     // fread(GLOBALS.lights.buffer, sizeof(Light), GLOBALS.lights.count, f);
    // }
    // else
    // {
        
    //     init_vector(&GLOBALS.lights, 0, sizeof(Light));
    // }
    // Light * lights = CAST_VEC(GLOBALS.lights, Light);
    // for (int i = 0; i < GLOBALS.lights.count; i++)
    // {
    //     Light * l = &lights[i];
    //     fread(l, sizeof(Light), 1, f);
    //     l->sm_dirty = true;
    // }

    read_alloc_vector(&FONTS, sizeof(Font), f);
    Font * fonts = CAST_VEC(FONTS, Font);
    for (int i = 0; i < FONTS.count; i++)
    {

        Font * font = &fonts[i];
        fread(font, sizeof(Font), 1, f);
        fread(&font->glyph_count, sizeof(size_t), 1, f);
        if (font->glyph_count > 0)
        {
            font->glyphs = malloc(sizeof(Glyph) * font->glyph_count);
            fread(font->glyphs, sizeof(Glyph), font->glyph_count, f);
        }
        font->texture = font->image_id;
        
        
    }

    
    
    // fread(&GLOBALS.text_boxes.count, sizeof(size_t), 1, f);
    // if (GLOBALS.text_boxes.count> 0)
    // {
        

    //     GLOBALS.text_boxes.buffer = malloc(sizeof(TextBox) * GLOBALS.text_boxes.count);
    //     GLOBALS.text_boxes.capacity = GLOBALS.text_boxes.count;

    // }
    // else
    // {
        
    //     init_vector(&GLOBALS.text_boxes, 0, sizeof(TextBox));
    // }
    // TextBox * tbs = CAST_VEC(GLOBALS.text_boxes, TextBox);
    // for (int i = PERFORMANCE_OVERLAY_TEXT_BOX_COUNT; i < GLOBALS.text_boxes.count; i++)
    // {
    //     TextBox * tb = &tbs[i];
    //     fread(tb, sizeof(TextBox), 1, f);
    //     uint32_t len = 0;
    //     fread(&len, sizeof(uint32_t), 1, f);
    //     if (len > 0)
    //     {
    //         tb->text = malloc(sizeof(char) * len);
    //         fread(tb->text, sizeof(char), len, f);
            
    //     }
    //     else
    //     {
    //         tb->text = NULL;
    //     }
    // }


    // read_alloc_vector(&GLOBALS.audio_portals, sizeof(AudioPortal), f);
    // AudioPortal * portals = CAST_VEC(GLOBALS.audio_portals, AudioPortal);
    // for (int i = 0; i < GLOBALS.audio_portals.count; i++)
    // {
    //     AudioPortal * p = &portals[i];
    //     fread(&p->pos, sizeof(vec3), 1, f);
    //     p->adjacent_portals.buffer = NULL;
    //     p->adjacent_portals.count = 0;
    //     read_vector(&p->adjacent_portals, sizeof(int), f);
    // }



    
    fclose(f);
    
    VertexLayout * vl = CAST_VEC(GLOBALS.vertex_layouts, VertexLayout);
    upload_material_data();
    upload_lights();
    GPU_upload_scene_data();
    add_all_meshes_to_buffer(vl);
    printf("FINISHED\n");
    create_performance_overlay();
    add_all_text_boxes_to_draw_buffer();
    
    reallocate_rprobe_array();

    draw_dirty = true;
}

void scene_save_volumes(const char * name)
{

    char scene_name[256];
    strcpy(scene_name, scene_registry.directory);
    strcat(scene_name, name);
    strcat(scene_name, ".bin");

    char lm_name[256];
    strcpy(lm_name, lightmap_registry.directory);
    strcat(lm_name, name);
    strcat(lm_name, "_lm.bin");

    char light_name[256];
    strcpy(light_name, light_registry.directory);
    strcat(light_name, name);
    strcat(light_name, "_lights.bin");

    char aportal_name[256];
    strcpy(aportal_name, aportal_registry.directory);
    strcat(aportal_name, name);
    strcat(aportal_name, "_aportals.bin");

    char rprobe_name[256];
    strcpy(rprobe_name, rprobe_registry.directory);
    strcat(rprobe_name, name);
    strcat(rprobe_name, "_rprobes.bin");

    char lprobe_name[256];
    strcpy(lprobe_name, lprobe_registry.directory);
    strcat(lprobe_name, name);
    strcat(lprobe_name, "_lprobes.bin");

    char aprobe_name[256];
    strcpy(aprobe_name, aprobe_registry.directory);
    strcat(aprobe_name, name);
    strcat(aprobe_name, "_aprobes.bin");

    char scene_apv_name[256];
    strcpy(scene_apv_name, scene_apv_registry.directory);
    strcat(scene_apv_name, name);
    strcat(scene_apv_name, "_scene_apvs.bin");

    save_aportals(aportal_name, &GLOBALS.audio_portals);

    save_aprobes(aprobe_name);

    save_rprobes(rprobe_name, &GLOBALS.scene_data.reflection_probes);

    save_vector(light_name, &GLOBALS.lights);

}

void scene_load_volumes(const char * name)
{

    char scene_name[256];
    strcpy(scene_name, scene_registry.directory);
    strcat(scene_name, name);
    strcat(scene_name, ".bin");

    char lm_name[256];
    strcpy(lm_name, lightmap_registry.directory);
    strcat(lm_name, name);
    strcat(lm_name, "_lm.bin");

    char light_name[256];
    strcpy(light_name, light_registry.directory);
    strcat(light_name, name);
    strcat(light_name, "_lights.bin");

    char aportal_name[256];
    strcpy(aportal_name, aportal_registry.directory);
    strcat(aportal_name, name);
    strcat(aportal_name, "_aportals.bin");

    char rprobe_name[256];
    strcpy(rprobe_name, rprobe_registry.directory);
    strcat(rprobe_name, name);
    strcat(rprobe_name, "_rprobes.bin");

    char lprobe_name[256];
    strcpy(lprobe_name, lprobe_registry.directory);
    strcat(lprobe_name, name);
    strcat(lprobe_name, "_lprobes.bin");

    char aprobe_name[256];
    strcpy(aprobe_name, aprobe_registry.directory);
    strcat(aprobe_name, name);
    strcat(aprobe_name, "_aprobes.bin");

    char scene_apv_name[256];
    strcpy(scene_apv_name, scene_apv_registry.directory);
    strcat(scene_apv_name, name);
    strcat(scene_apv_name, "_scene_apvs.bin");

    
    load_aportals(aportal_name, &GLOBALS.audio_portals);

    load_aprobes(aprobe_name);

    load_rprobes(rprobe_name, &GLOBALS.scene_data.reflection_probes);

    load_vector(light_name, &GLOBALS.lights);

    upload_material_data();
    upload_lights();
    GPU_upload_scene_data();

    Light * ls = CAST_VEC(GLOBALS.lights, Light);
    for (int i = 0; i < GLOBALS.lights.count; i++)
    {
        Light * l = &ls[i];
        printf("%d", l->light_type);
    }

    reallocate_rprobe_array();
}
void save_vector(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&vec->count, sizeof(size_t), 1, f);
    if (vec->buffer) fwrite(vec->buffer, vec->type_size, vec->count, f);

    
    if (f) fclose(f);
}

void load_vector(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "rb");
    if (!f) return;
    
    read_vector(vec, vec->type_size, f);

    
    if (f) fclose(f);
    
}



void save_rprobes(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&vec->count, sizeof(size_t), 1, f);
    if (vec->buffer) fwrite(vec->buffer, vec->type_size, vec->count, f);

    
    if (f) fclose(f);
}

void load_rprobes(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "rb");
    if (!f) return;
    
    read_vector(vec, vec->type_size, f);
    reallocate_rprobe_array();

    
    if (f) fclose(f);
    
}

void save_lprobes(const char * filename, Vector * vec)
{

    FILE * f = fopen(filename, "wb");
    if (!f) return;

    fwrite(GLOBALS.scene_data.LPV_PATH, sizeof(char), 256, f);
    fwrite(&vec->count, sizeof(size_t), 1, f);
    if (vec->buffer) fwrite(vec->buffer, vec->type_size, vec->count, f);
    
    if (f) fclose(f);

}

void load_lprobes(const char * filename, Vector * vec)
{

    FILE * f = fopen(filename, "rb");
    if (!f) return;
    
    fread(GLOBALS.scene_data.LPV_PATH, sizeof(char), 256, f);

    if (GLOBALS.scene_data.LPV_FILE)
    {
        fclose(GLOBALS.scene_data.LPV_FILE);
        GLOBALS.scene_data.LPV_FILE = NULL;
    }
    GLOBALS.scene_data.LPV_FILE = fopen(GLOBALS.scene_data.LPV_PATH, "rb+");
    if (!GLOBALS.scene_data.LPV_FILE)
    {
        GLOBALS.scene_data.LPV_FILE = fopen(GLOBALS.scene_data.LPV_PATH, "wb+");
    }
    reset_lighting_pvs();
    read_vector(vec, vec->type_size, f);
    
    if (f) fclose(f);
    
}

void save_aportals(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "wb");
    if (!f) return;

    fwrite(&vec->count, sizeof(size_t), 1, f);

    AudioPortal * portals = CAST_VEC(*vec, AudioPortal);

    for (int i = 0; i < vec->count; i++)
    {

        AudioPortal * p = &portals[i];

        fwrite(&p->pos, sizeof(vec3), 1, f);
        fwrite(&p->adjacent_portals.count, sizeof(size_t), 1, f);
        if (p->adjacent_portals.buffer) fwrite(p->adjacent_portals.buffer, sizeof(int), p->adjacent_portals.count, f);
        
    }

    
    // if (vec->buffer) fwrite(vec->buffer, vec->type_size, vec->count, f);

    
    if (f) fclose(f);
}

void load_aportals(const char * filename, Vector * vec)
{
    FILE * f = fopen(filename, "rb");
    if (!f) return;

    size_t count = 0;
    fread(&count, sizeof(size_t), 1, f);
    if (count > 0)
    {
        if (vec->buffer) free(vec->buffer);
        init_vector(vec, 0, vec->type_size);
        vec->buffer = calloc(count, sizeof(AudioPortal));
        vec->count = count;
        vec->capacity = 0;

        AudioPortal * portals = CAST_VEC(*vec, AudioPortal);
        for (int i = 0; i < vec->count; i++)
        {

            AudioPortal * p = &portals[i];
            fread(&p->pos, sizeof(vec3), 1, f);
            init_vector(&p->adjacent_portals, 0, sizeof(int));
            fread(&p->adjacent_portals.count, sizeof(size_t), 1, f);
            if (p->adjacent_portals.count > 0)
            {
                p->adjacent_portals.buffer = malloc(sizeof(int) * p->adjacent_portals.count);
                p->adjacent_portals.capacity = p->adjacent_portals.count;
                fread(p->adjacent_portals.buffer, sizeof(int), p->adjacent_portals.count, f);
            }

            // init_vector(&p->adjacent_portals, 0, sizeof(int));
            // read_vector(&p->adjacent_portals, sizeof(int), f);
            // int * adj = CAST_VEC(p->adjacent_portals, int);
            // for (int j = 0; j < p->adjacent_portals.count; j++)
            // {
            //     printf("ADJ: %d\n", adj[j]);
            // }
        
        }
    }

    
    if (f) fclose(f);
}

void save_aprobes(const char * filename)
{

    FILE * f = fopen(filename, "wb");

    fwrite(GLOBALS.scene_data.APV_PATH, sizeof(char), 256, f);
    fwrite(&GLOBALS.scene_data.audio_probe_volumes.count, sizeof(size_t), 1, f);
    if (GLOBALS.scene_data.audio_probe_volumes.buffer) fwrite(GLOBALS.scene_data.audio_probe_volumes.buffer, sizeof(ProbeVolume), GLOBALS.scene_data.audio_probe_volumes.count, f);

    
    // fwrite(&GLOBALS.aprobe_size, sizeof(ivec3), 1, f);
    // fwrite(&GLOBALS.aprobe_start, sizeof(vec3), 1, f);
    // fwrite(&GLOBALS.aprobe_spacing, sizeof(float), 1, f);
    // fwrite(GLOBALS.audio_probes, sizeof(AudioProbe), GLOBALS.aprobe_size.x * GLOBALS.aprobe_size.y * GLOBALS.aprobe_size.z, f);
    
    
    if (f) fclose(f);
    
}

void load_aprobes(const char * filename)
{
    
    FILE * f = fopen(filename, "rb");

    fread(GLOBALS.scene_data.APV_PATH, sizeof(char), 256, f);

    if (GLOBALS.scene_data.APV_FILE)
    {
        fclose(GLOBALS.scene_data.APV_FILE);
        GLOBALS.scene_data.APV_FILE = NULL;
    }
    GLOBALS.scene_data.APV_FILE = fopen(GLOBALS.scene_data.APV_PATH, "rb+");
    if (!GLOBALS.scene_data.APV_FILE)
    {
        GLOBALS.scene_data.APV_FILE = fopen(GLOBALS.scene_data.APV_PATH, "wb+");
    }
    reset_audio_pvs();

    read_vector(&GLOBALS.scene_data.audio_probe_volumes, sizeof(ProbeVolume), f);
    
    // ivec3 potential_size = {0};
    // fread(&potential_size, sizeof(ivec3), 1, f);
    // int aprobe_count = potential_size.x * potential_size.y * potential_size.z;
    // // int aprobe_count = GLOBALS.aprobe_size.x * GLOBALS.aprobe_size.y * GLOBALS.aprobe_size.z;
    // if (aprobe_count > 0)
    // {
    //     GLOBALS.aprobe_size = potential_size;
    //     fread(&GLOBALS.aprobe_start, sizeof(vec3), 1, f);
    //     fread(&GLOBALS.aprobe_spacing, sizeof(float), 1, f);

    //     if (GLOBALS.audio_probes) free(GLOBALS.audio_probes);
    //     GLOBALS.audio_probes = NULL;


    //     GLOBALS.audio_probes = malloc(sizeof(AudioProbe) * aprobe_count);
    //     fread(GLOBALS.audio_probes, sizeof(AudioProbe), aprobe_count, f);
    // }
    
    if (f) fclose(f);

}

