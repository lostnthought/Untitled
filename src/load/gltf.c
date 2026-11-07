
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include "core/engine_types.h"
#include "utilities/mathf.h"
#include "utilities/engine_utilities.h"
#include "utilities/types.h"
#include "load/load.h"
#include "load/init.h"
#include "nuklear/nuklear.h"
#include "nuklear/nuklear_sdl_gl3.h"
#include "cJSON/cJSON.h"
#include "utilities/utilities.h"

static int comp_count(const char *type) {
    if (!type) return 1;
    if (strcmp(type, "SCALAR") == 0) return 1;
    if (strcmp(type, "VEC2") == 0) return 2;
    if (strcmp(type, "VEC3") == 0) return 3;
    if (strcmp(type, "VEC4") == 0) return 4;
    if (strcmp(type, "MAT4") == 0) return 16;
    return 1;
}

static int comp_size(int componentType) {
    switch (componentType) {
        case 5120: 
        case 5121: 
            return 1;
        case 5122: 
        case 5123: 
            return 2;
        case 5125: 
        case 5126: 
            return 4;
        default: return 0;
    }
}

static char* load_text_file(const char *path) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long len = ftell(f); rewind(f);
    char *buf = (char*)malloc(len+1);
    fread(buf,1,len,f);
    buf[len]=0;
    fclose(f);
    return buf;
}

static unsigned char* load_bin_file(const char *path, int expectedLen) {
    FILE *f = fopen(path, "rb");
    if (!f) return NULL;
    fseek(f,0,SEEK_END);
    long len = ftell(f); rewind(f);
    if (expectedLen > 0 && len < expectedLen) {
        fclose(f); return NULL;
    }
    unsigned char *data = (unsigned char*)malloc(len);
    fread(data,1,len,f);
    fclose(f);
    return data;
}

static int str_eq(const char* a, const char* b){ return a && b && strcmp(a,b)==0; }

static int json_get_int(cJSON* obj, const char* key, int defv){
    cJSON* it = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsNumber(it) ? it->valueint : defv;
}

static const char* json_get_str(cJSON* obj, const char* key){
    cJSON* it = cJSON_GetObjectItemCaseSensitive(obj, key);
    return cJSON_IsString(it) ? it->valuestring : NULL;
}
static void json_get_vec3(cJSON *obj, const char *key, vec3 * out, const float def[3]) {
    cJSON *arr = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (cJSON_IsArray(arr) && cJSON_GetArraySize(arr) == 3) {
        cJSON *val = cJSON_GetArrayItem(arr,0);
        out->x = cJSON_IsNumber(val) ? (float)val->valuedouble : def[0];
        val = cJSON_GetArrayItem(arr,1);
        out->y = cJSON_IsNumber(val) ? (float)val->valuedouble : def[1];
        val = cJSON_GetArrayItem(arr,2);
        out->z = cJSON_IsNumber(val) ? (float)val->valuedouble : def[2];
    } else {
        memcpy(out, def, sizeof(float)*3);
    }
}
static void json_get_vec4(cJSON *obj, const char *key, vec4 * out, const float def[3]) {
    cJSON *arr = cJSON_GetObjectItemCaseSensitive(obj, key);
    if (cJSON_IsArray(arr) && cJSON_GetArraySize(arr) == 4) {
        cJSON *val = cJSON_GetArrayItem(arr,0);
        out->x = cJSON_IsNumber(val) ? (float)val->valuedouble : def[0];
        val = cJSON_GetArrayItem(arr,1);
        out->y = cJSON_IsNumber(val) ? (float)val->valuedouble : def[1];
        val = cJSON_GetArrayItem(arr,2);
        out->z = cJSON_IsNumber(val) ? (float)val->valuedouble : def[2];
        val = cJSON_GetArrayItem(arr,3);
        out->w = cJSON_IsNumber(val) ? (float)val->valuedouble : def[3];
    } else {
        memcpy(out, def, sizeof(float)*4);
    }
}


static void json_get_float_array(cJSON *parent, const char *name, float *out, int count, const float *defaults) {
    cJSON *arr = cJSON_GetObjectItemCaseSensitive(parent, name);
    if (cJSON_IsArray(arr)) {
        for (int i = 0; i < count; i++) {
            cJSON *item = cJSON_GetArrayItem(arr, i);
            out[i] = cJSON_IsNumber(item) ? (float)item->valuedouble : defaults[i];
        }
    } else {
        for (int i = 0; i < count; i++) out[i] = defaults[i];
    }
}

static int json_get_tex_index(cJSON *parent, const char *name) {
    cJSON *obj = cJSON_GetObjectItemCaseSensitive(parent, name);
    if (obj) {
        cJSON *idx = cJSON_GetObjectItemCaseSensitive(obj, "index");
        if (cJSON_IsNumber(idx)) return idx->valueint;
    }
    return -1;
}

void parse_gltf_materials(cJSON *root, GLTFDoc *doc) {
    cJSON *materials = cJSON_GetObjectItemCaseSensitive(root, "materials");
    if (!materials) return;

    doc->material_count = cJSON_GetArraySize(materials);
    doc->materials = calloc(doc->material_count, sizeof(GLTFMaterial));

    printf("MAT COUNT: %d\n", doc->material_count);
    for (int i = 0; i < doc->material_count; i++) {
        cJSON *mat = cJSON_GetArrayItem(materials, i);
        GLTFMaterial *dst = &doc->materials[i];

        dst->base_color_factor[0] = 1.0f; dst->base_color_factor[1] = 1.0f;
        dst->base_color_factor[2] = 1.0f; dst->base_color_factor[3] = 1.0f;
        dst->metallic_factor = 1.0f;
        dst->roughness_factor = 1.0f;
        dst->emissive_factor[0] = 0.0f; dst->emissive_factor[1] = 0.0f; dst->emissive_factor[2] = 0.0f;
        
        dst->base_color_tex = -1;
        dst->metallic_roughness_tex = -1;
        dst->normal_tex = -1;
        dst->occlusion_tex = -1;
        dst->emissive_tex          = -1;
        
        dst->normal_scale = 1.0f;
        dst->occlusion_strength  = 1.0f;
        dst->alpha_cutoff = 0.5f;
        dst->alpha_mode  = ALPHA_OPAQUE;
        dst->double_sided = false;

        cJSON *name = cJSON_GetObjectItemCaseSensitive(mat, "name");
        if (cJSON_IsString(name)) strncpy(dst->name, name->valuestring, sizeof(dst->name) - 1);

        cJSON *pbr = cJSON_GetObjectItemCaseSensitive(mat, "pbrMetallicRoughness");
        if (pbr) {
            float defColor[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            json_get_float_array(pbr, "baseColorFactor", dst->base_color_factor, 4, defColor);

            cJSON *mf = cJSON_GetObjectItemCaseSensitive(pbr, "metallicFactor");
            if (cJSON_IsNumber(mf)) dst->metallic_factor = (float)mf->valuedouble;

            cJSON *rf = cJSON_GetObjectItemCaseSensitive(pbr, "roughnessFactor");
            if (cJSON_IsNumber(rf)) dst->roughness_factor = (float)rf->valuedouble;

            dst->base_color_tex         = json_get_tex_index(pbr, "baseColorTexture");
            dst->metallic_roughness_tex = json_get_tex_index(pbr, "metallicRoughnessTexture");
        }

        dst->normal_tex = json_get_tex_index(mat, "normalTexture");
        dst->occlusion_tex = json_get_tex_index(mat, "occlusionTexture");
        dst->emissive_tex = json_get_tex_index(mat, "emissiveTexture");

        cJSON *normObj = cJSON_GetObjectItemCaseSensitive(mat, "normalTexture");
        if (normObj) {
            cJSON *scale = cJSON_GetObjectItemCaseSensitive(normObj, "scale");
            if (cJSON_IsNumber(scale)) dst->normal_scale = (float)scale->valuedouble;
        }

        cJSON *occObj = cJSON_GetObjectItemCaseSensitive(mat, "occlusionTexture");
        if (occObj) {
            cJSON *strength = cJSON_GetObjectItemCaseSensitive(occObj, "strength");
            if (cJSON_IsNumber(strength)) dst->occlusion_strength = (float)strength->valuedouble;
        }

        float defEmissive[3] = {0.0f, 0.0f, 0.0f};
        json_get_float_array(mat, "emissiveFactor", dst->emissive_factor, 3, defEmissive);

        cJSON *alphaMode = cJSON_GetObjectItemCaseSensitive(mat, "alphaMode");
        if (cJSON_IsString(alphaMode)) {
            if (strcmp(alphaMode->valuestring, "MASK") == 0)  dst->alpha_mode = ALPHA_MASK;
            if (strcmp(alphaMode->valuestring, "BLEND") == 0) dst->alpha_mode = ALPHA_BLEND;
        }

        cJSON *cutoff = cJSON_GetObjectItemCaseSensitive(mat, "alphaCutoff");
        if (cJSON_IsNumber(cutoff)) dst->alpha_cutoff = (float)cutoff->valuedouble;

        cJSON *doubleSided = cJSON_GetObjectItemCaseSensitive(mat, "doubleSided");
        if (cJSON_IsBool(doubleSided)) dst->double_sided = cJSON_IsTrue(doubleSided);

        cJSON *extensions= cJSON_GetObjectItemCaseSensitive(mat, "extensions");


        dst->emissive_strength = 1;
        if (extensions)
        {
            cJSON * emissive_strength_ext = cJSON_GetObjectItemCaseSensitive(extensions, "KHR_materials_emissive_strength");
            if (emissive_strength_ext)
            {
                
                cJSON * emissive_strength = cJSON_GetObjectItemCaseSensitive(emissive_strength_ext, "emissiveStrength");
                if (cJSON_IsNumber(emissive_strength))
                {
                
                    dst->emissive_strength = (float)emissive_strength->valuedouble;
                }
            }
        }
        cJSON *extras= cJSON_GetObjectItemCaseSensitive(mat, "extras");
        dst->fwd_shader = 0;
        dst->gbuffer_shader = 0;
        
        for (int i = 0; i < AUDIO_NUM_BANDS; i++)
        {
            
            dst->absorption_coefficients[i] = 0.5f;
            dst->scattering_coefficients[i] = 0.5f;
        }
        dst->override_transparent = false;
        
        if (extras)
        {

            cJSON * fwd_shader = cJSON_GetObjectItemCaseSensitive(extras, "fwd_shader");
            if (cJSON_IsString(fwd_shader))
            {
                for (int i = 0; i < UBERSHADER_VERSIONS; i++)
                {
                    
                    if (strcmp(fwd_shader->valuestring, DYNAMIC_SHADER[i].name) == 0)
                    {
                        dst->fwd_shader = i;
                        break;
                    }
                }
            }

            cJSON * gbuffer_shader = cJSON_GetObjectItemCaseSensitive(extras, "gbuffer_shader");
            if (cJSON_IsString(gbuffer_shader))
            {
                for (int i = 0; i < GBUFFER_VERSIONS; i++)
                {
                    
                    if (strcmp(gbuffer_shader->valuestring, DEPTH_PRE_PASS_SHADER[i].name) == 0)
                    {
                        dst->gbuffer_shader = i;
                        break;
                    }
                }
            }

            cJSON * absorption_coefficients = cJSON_GetObjectItemCaseSensitive(extras, "absorption_coefficients");
            if (cJSON_IsArray(absorption_coefficients))
            {

                for (int j = 0; j < AUDIO_NUM_BANDS; j++)
                {
                    cJSON * c0 = cJSON_GetArrayItem(absorption_coefficients, j);

                    if (cJSON_IsNumber(c0))
                    {
                        dst->absorption_coefficients[j] = c0->valuedouble;
                    }
                    
                }
                
            }
            cJSON * scattering_coefficients = cJSON_GetObjectItemCaseSensitive(extras, "scattering_coefficients");
            if (cJSON_IsArray(scattering_coefficients))
            {
                for (int j = 0; j < AUDIO_NUM_BANDS; j++)
                {
                    cJSON * c0 = cJSON_GetArrayItem(scattering_coefficients, j);

                    if (cJSON_IsNumber(c0))
                    {
                        dst->scattering_coefficients[j] = c0->valuedouble;
                    }
                    
                }
                
            }
            cJSON * override = cJSON_GetObjectItemCaseSensitive(extras, "override_transparent");
            if (cJSON_IsBool(override))
            {
                dst->override_transparent = override->valueint;
                
            }
            
        }
    }
}
static int parse_gltf_json(const char *gltfPath, const char * prefix, GLTFDoc *doc) {
    memset(doc,0,sizeof(*doc));

    char *json = load_text_file(gltfPath);
    if (!json) {
        printf("EARLY RETURN LOAD\n");
        return 0;
    }

    cJSON *root = cJSON_Parse(json);
    if (!root) { free(json); printf("NO ROOT\n"); return 0; }

    cJSON *buffers = cJSON_GetObjectItemCaseSensitive(root, "buffers");
    doc->bufferCount = cJSON_GetArraySize(buffers);
    doc->buffers = (GLTFBuffer*)calloc(doc->bufferCount, sizeof(GLTFBuffer));
    for (int i=0;i<doc->bufferCount;i++){
        cJSON *b = cJSON_GetArrayItem(buffers,i);
        const char *uri = json_get_str(b,"uri");
        int bytelen = json_get_int(b,"byteLength",0);
        doc->buffers[i].byteLength = bytelen;
        if (uri) {
            doc->buffers[i].uri = strdup(uri);
        } else {
            doc->buffers[i].uri = NULL;
            doc->buffers[i].data = NULL;
        }
    }

    cJSON *views = cJSON_GetObjectItemCaseSensitive(root, "bufferViews");
    doc->viewCount = cJSON_GetArraySize(views);
    doc->views = (GLTFBufferView*)calloc(doc->viewCount,sizeof(GLTFBufferView));
    for (int i=0;i<doc->viewCount;i++){
        cJSON *v = cJSON_GetArrayItem(views,i);
        doc->views[i].buffer     = json_get_int(v,"buffer",0);
        doc->views[i].byteOffset = json_get_int(v,"byteOffset",0);
        doc->views[i].byteLength = json_get_int(v,"byteLength",0);
        doc->views[i].byteStride = json_get_int(v,"byteStride",0); 
    }

    cJSON *accs = cJSON_GetObjectItemCaseSensitive(root, "accessors");
    doc->accessorCount = cJSON_GetArraySize(accs);
    doc->accessors = (GLTFAccessor*)calloc(doc->accessorCount,sizeof(GLTFAccessor));
    for (int i=0;i<doc->accessorCount;i++){
        cJSON *a = cJSON_GetArrayItem(accs,i);
        doc->accessors[i].bufferView    = json_get_int(a,"bufferView",-1);
        doc->accessors[i].byteOffset    = json_get_int(a,"byteOffset",0);
        doc->accessors[i].componentType = json_get_int(a,"componentType",0);
        doc->accessors[i].count         = json_get_int(a,"count",0);

        const char *ty = json_get_str(a,"type");
        if (ty) strncpy(doc->accessors[i].type, ty, sizeof(doc->accessors[i].type)-1);
        else strcpy(doc->accessors[i].type, "SCALAR");
    }

    cJSON *meshes = cJSON_GetObjectItemCaseSensitive(root, "meshes");
    doc->meshCount = cJSON_GetArraySize(meshes);
    doc->meshes = (GLTFMesh*)calloc(doc->meshCount,sizeof(GLTFMesh));
    for (int mi=0; mi<doc->meshCount; mi++){
        cJSON *m = cJSON_GetArrayItem(meshes, mi);
        cJSON *prims = cJSON_GetObjectItemCaseSensitive(m, "primitives");
        int pc = cJSON_GetArraySize(prims);
        doc->meshes[mi].primCount = pc;
        doc->meshes[mi].prims = (GLTFPrimitive*)calloc(pc,sizeof(GLTFPrimitive));

        for (int pi=0; pi<pc; pi++){
            cJSON *p = cJSON_GetArrayItem(prims, pi);
            GLTFPrimitive *dst = &doc->meshes[mi].prims[pi];
            dst->position = dst->normal = dst->texcoord0 = dst->joints0 = dst->weights0 = -1;
            dst->indices  = -1;
            dst->material = json_get_int(p,"material",-1);

            cJSON *attrs = cJSON_GetObjectItemCaseSensitive(p,"attributes");
            if (attrs) {
                cJSON *POS = cJSON_GetObjectItemCaseSensitive(attrs,"POSITION");
                cJSON *NOR = cJSON_GetObjectItemCaseSensitive(attrs,"NORMAL");
                cJSON *TAN = cJSON_GetObjectItemCaseSensitive(attrs,"TANGENT");
                cJSON *UV0 = cJSON_GetObjectItemCaseSensitive(attrs,"TEXCOORD_0");
                cJSON *J0  = cJSON_GetObjectItemCaseSensitive(attrs,"JOINTS_0");
                cJSON *W0  = cJSON_GetObjectItemCaseSensitive(attrs,"WEIGHTS_0");
                if (cJSON_IsNumber(POS)) dst->position = POS->valueint;
                if (cJSON_IsNumber(NOR)) dst->normal   = NOR->valueint;
                if (cJSON_IsNumber(TAN)) dst->tangent  = TAN->valueint;
                if (cJSON_IsNumber(UV0)) dst->texcoord0= UV0->valueint;
                if (cJSON_IsNumber(J0))  dst->joints0  = J0->valueint;
                if (cJSON_IsNumber(W0))  dst->weights0 = W0->valueint;
            }
            cJSON *IND = cJSON_GetObjectItemCaseSensitive(p,"indices");
            if (cJSON_IsNumber(IND)) dst->indices = IND->valueint;
        }
    }

    parse_gltf_materials(root, doc);

    cJSON *textures = cJSON_GetObjectItemCaseSensitive(root,"textures");
    doc->texture_count = cJSON_GetArraySize(textures);
    doc->textures = calloc(doc->texture_count, sizeof(GLTFTexture));
    for (int i = 0; i < doc->texture_count; i++)
    {
        cJSON * n = cJSON_GetArrayItem(textures, i);

        GLTFTexture * tex = &doc->textures[i];
        
        cJSON *sampler = cJSON_GetObjectItemCaseSensitive(n,"sampler");
        if (cJSON_IsNumber(sampler)) {
            tex->sampler = sampler->valueint; 
        }
        
        cJSON *src = cJSON_GetObjectItemCaseSensitive(n,"source");
        if (cJSON_IsNumber(src)) {
            tex->source = src->valueint; 
        }
    }


    cJSON *images = cJSON_GetObjectItemCaseSensitive(root,"images");
    doc->image_count = cJSON_GetArraySize(images);
    doc->images = calloc(doc->image_count, sizeof(GLTFImage));
    for (int i = 0; i < doc->image_count; i++)
    {

        cJSON * n = cJSON_GetArrayItem(images, i);
        GLTFImage * img = &doc->images[i];

        cJSON *mime_type = cJSON_GetObjectItemCaseSensitive(n,"mimeType");
        if (cJSON_IsString(mime_type))
        {
            strncpy(img->mime_type, mime_type->valuestring, 254);
            img->mime_type[255] = '\0';
        }
        cJSON *name = cJSON_GetObjectItemCaseSensitive(n,"name");
        if (cJSON_IsString(name))
        {
            strncpy(img->name, name->valuestring, 254);
            img->name[255] = '\0';
        }
        cJSON *uri = cJSON_GetObjectItemCaseSensitive(n,"uri");
        if (cJSON_IsString(uri))
        {
            strncpy(img->uri, prefix, 254);
            strncat(img->uri, uri->valuestring, 254);
            // strncpy(img->uri, uri->valuestring, 254);
            // img->uri[255] = '\0';
        }
        cJSON *buffer_view= cJSON_GetObjectItemCaseSensitive(n,"bufferView");
        if (cJSON_IsNumber(buffer_view)) {
            img->buffer_view = buffer_view->valueint; 
        }
    }

    
    cJSON *samplers = cJSON_GetObjectItemCaseSensitive(root,"samplers");
    doc->sampler_count = cJSON_GetArraySize(samplers);
    doc->samplers = calloc(doc->sampler_count, sizeof(GLTFSampler));
    for (int i = 0; i < doc->sampler_count; i++)
    {
        cJSON * n = cJSON_GetArrayItem(samplers, i);

        GLTFSampler * sampler = &doc->samplers[i];


        cJSON *magFilter = cJSON_GetObjectItemCaseSensitive(n,"magFilter");
        if (cJSON_IsNumber(magFilter)) {
            sampler->mag_filter = magFilter->valueint; 
        }
        cJSON *minFilter = cJSON_GetObjectItemCaseSensitive(n,"minFilter");
        if (cJSON_IsNumber(minFilter)) {
            sampler->min_filter = minFilter->valueint; 
        }
        cJSON *wrapS = cJSON_GetObjectItemCaseSensitive(n,"wrapS");
        if (cJSON_IsNumber(wrapS)) {
            sampler->wrap_s = wrapS->valueint; 
        }

        cJSON *wrapT = cJSON_GetObjectItemCaseSensitive(n,"wrapT");
        if (cJSON_IsNumber(wrapT)) {
            sampler->wrap_t = wrapT->valueint; 
        }
        cJSON *name = cJSON_GetObjectItemCaseSensitive(n,"name");
        if (cJSON_IsString(name))
        {
            strncpy(sampler->name, name->valuestring, 254);
            sampler->name[255] = '\0';
        }

    }

    
    cJSON *nodes = cJSON_GetObjectItemCaseSensitive(root,"nodes");
    // check to see if extras exists for static / dynamic
    doc->node_count = cJSON_GetArraySize(nodes);
    doc->nodes = calloc(doc->node_count, sizeof(MeshNode));

    for (int i=0; i<doc->node_count; i++) {
        cJSON *n = cJSON_GetArrayItem(nodes,i);
        MeshNode *dst = &doc->nodes[i];

        dst->parent = -1;
        dst->mesh = -1;
        dst->skin = -1;

        cJSON *name = cJSON_GetObjectItemCaseSensitive(n,"name");
        if (cJSON_IsString(name)) strncpy(dst->name, name->valuestring, sizeof(dst->name)-1);

        cJSON *mesh = cJSON_GetObjectItemCaseSensitive(n,"mesh");
        if (cJSON_IsNumber(mesh)) dst->mesh = mesh->valueint;

        cJSON *skin = cJSON_GetObjectItemCaseSensitive(n,"skin");
        if (cJSON_IsNumber(skin)) dst->skin = skin->valueint;

        json_get_vec3(n,"translation", &dst->translation, (float[3]){0,0,0});
        json_get_vec4(n,"rotation", &dst->rotation, (float[4]){0,0,0,1});
        json_get_vec3(n,"scale", &dst->scale, (float[3]){1,1,1});

        cJSON *children = cJSON_GetObjectItemCaseSensitive(n,"children");
        if (children) {
            int cc = cJSON_GetArraySize(children);
            dst->child_count = cc;
            for (int ci=0; ci<cc; ci++) {
                dst->children[ci] = cJSON_GetArrayItem(children,ci)->valueint;
                doc->nodes[dst->children[ci]].parent = i;
            }
        }

        cJSON *extras = cJSON_GetObjectItemCaseSensitive(n,"extras");
        if (extras){
            // check for static or dynamic
            
            cJSON * mesh_mode = cJSON_GetObjectItemCaseSensitive(extras, "mesh_mode");
            if (mesh_mode)
            {
                if (strcmp(mesh_mode->valuestring, "dynamic") == 0)
                {
                    dst->static_mesh = false;
                    
                }
                else
                {
                    dst->static_mesh = true;
                    
                }
                
            }
            else
            {
                dst->static_mesh = true;
            }
            cJSON * physics_mode= cJSON_GetObjectItemCaseSensitive(extras, "physics_type");
            if (physics_mode)
            {
                dst->physics_mode = physics_mode->valueint;
                if (physics_mode->valueint >= 1)
                {
                    dst->static_mesh = false;
                }
                else
                {
                    dst->physics_mode = 0;
                    dst->static_mesh = true;
                }
            }
            else
            {
                dst->physics_mode = false;
                dst->static_mesh = true;
            }
            cJSON * collision_type = cJSON_GetObjectItemCaseSensitive(extras, "collision_type");
            if (collision_type)
            {
                dst->collision_type = collision_type->valueint;
            }
            else
            {
                dst->collision_type = dst->physics_mode >= 1 ? COL_HULL : COL_TRIANGLE;
            }
            

        } else {
            dst->static_mesh = true;
            dst->physics_mode = false;
            dst->collision_type = COL_TRIANGLE;
        }
    }

    cJSON *skins = cJSON_GetObjectItemCaseSensitive(root,"skins");
    doc->skin_count = cJSON_GetArraySize(skins);
    doc->skins = calloc(doc->skin_count, sizeof(GLTFSkin));

    for (int i=0; i<doc->skin_count; i++) {
        cJSON *s = cJSON_GetArrayItem(skins,i);
        GLTFSkin *dst = &doc->skins[i];

        cJSON *joints = cJSON_GetObjectItemCaseSensitive(s,"joints");
        int jc = cJSON_GetArraySize(joints);
        dst->joint_count = jc;
        dst->joints = calloc(jc,sizeof(int));
        for (int ji=0; ji<jc; ji++)
            dst->joints[ji] = cJSON_GetArrayItem(joints,ji)->valueint;

        cJSON *ibm = cJSON_GetObjectItemCaseSensitive(s,"inverseBindMatrices");
        if (cJSON_IsNumber(ibm)) {
            dst->ibm_accessor = ibm->valueint; 
        }
        cJSON * name = cJSON_GetObjectItemCaseSensitive(s, "name");
        if (cJSON_IsString(name)){
            strncpy(dst->name, name->valuestring, 62);
            dst->name[63] = '\0';
        } else {
            sprintf(dst->name, "default");
        }
        
    }

    cJSON *animations = cJSON_GetObjectItemCaseSensitive(root, "animations");
    doc->animation_count = cJSON_GetArraySize(animations);
    doc->animations = calloc(doc->animation_count, sizeof(GLTFAnimation));

    for (int ai = 0; ai < doc->animation_count; ai++) {
        cJSON *a = cJSON_GetArrayItem(animations, ai);
        GLTFAnimation *dst = &doc->animations[ai];

        cJSON *name = cJSON_GetObjectItemCaseSensitive(a, "name");
        if (cJSON_IsString(name)) strncpy(dst->name, name->valuestring, sizeof(dst->name)-1);

        cJSON *samplers = cJSON_GetObjectItemCaseSensitive(a, "samplers");
        dst->sampler_count = cJSON_GetArraySize(samplers);
        dst->samplers = calloc(dst->sampler_count, sizeof(GLTFAnimationSampler));
        for (int si=0; si < dst->sampler_count; si++) {
            cJSON *s = cJSON_GetArrayItem(samplers, si);
            dst->samplers[si].input  = json_get_int(s, "input", -1);
            dst->samplers[si].output = json_get_int(s, "output", -1);

            const char *interp = json_get_str(s, "interpolation");
            if (!interp || strcmp(interp,"LINEAR")==0) dst->samplers[si].interpolation = 0;
            else if (strcmp(interp,"STEP")==0) dst->samplers[si].interpolation = 1;
            else if (strcmp(interp,"CUBICSPLINE")==0) dst->samplers[si].interpolation = 2;
        }

        cJSON *channels = cJSON_GetObjectItemCaseSensitive(a, "channels");
        dst->channel_count = cJSON_GetArraySize(channels);
        dst->channels = calloc(dst->channel_count, sizeof(GLTFAnimationChannel));
        for (int ci=0; ci < dst->channel_count; ci++) {
            cJSON *c = cJSON_GetArrayItem(channels, ci);
            dst->channels[ci].sampler = json_get_int(c, "sampler", -1);

            cJSON *target = cJSON_GetObjectItemCaseSensitive(c, "target");
            if (target) {
                dst->channels[ci].target_node = json_get_int(target,"node",-1);

                const char *path = json_get_str(target,"path");
                if      (strcmp(path,"translation")==0) dst->channels[ci].path = TARGET_TRANSLATION;
                else if (strcmp(path,"rotation")==0)    dst->channels[ci].path = TARGET_ROTATION;
                else if (strcmp(path,"scale")==0)       dst->channels[ci].path = TARGET_SCALE;
                else if (strcmp(path,"weights")==0)     dst->channels[ci].path = TARGET_WEIGHTS;
            }
        }
    }






    
    
    char dir[512]; strcpy(dir, gltfPath);
    for (int i=(int)strlen(dir)-1; i>=0; --i){ if (dir[i]=='/'||dir[i]=='\\'){ dir[i+1]='\0'; break; } }

    for (int bi=0; bi<doc->bufferCount; bi++){
        if (!doc->buffers[bi].uri) { /* TODO: handle embedded base64 */ continue; }
        char full[1024]; snprintf(full, sizeof(full), "%s%s", dir, doc->buffers[bi].uri);
        // doc->buffers[bi].data = load_bin_file(full, doc->buffers[bi].byteLength);
        // doc->buffers[bi].data = load_bin_file(doc->buffers[bi].uri, doc->buffers[bi].byteLength);
        doc->buffers[bi].data = load_bin_file(full, doc->buffers[bi].byteLength);
        if (!doc->buffers[bi].data) { printf("NO DATA AT %s\n", full); cJSON_Delete(root); free(json); return 0; }
    }

    cJSON_Delete(root);
    free(json);
    return 1;
}
static unsigned char* accessor_base_ptr(GLTFDoc *doc, const GLTFAccessor *acc) {
    if (acc->bufferView < 0) return NULL;
    const GLTFBufferView *v = &doc->views[acc->bufferView];
    const GLTFBuffer *b     = &doc->buffers[v->buffer];
    size_t offset = (size_t)v->byteOffset + (size_t)acc->byteOffset;
    return b->data + offset;
}

static const unsigned char *accessor_elem_ptr(const GLTFDoc *doc, const GLTFAccessor *acc, int index) {
    if (acc->bufferView < 0) return NULL;
    const GLTFBufferView *view = &doc->views[acc->bufferView];
    const GLTFBuffer     *buf  = &doc->buffers[view->buffer];
    size_t base_off = (size_t)(view->byteOffset) + (size_t)(acc->byteOffset);
    int comps = comp_count(acc->type);
    int csize = comp_size(acc->componentType);
    int tight = comps * csize;
    int stride = view->byteStride ? view->byteStride : tight;
    return buf->data + base_off + (size_t)index * stride;
}

static void fetch_vec3f(const GLTFDoc *doc, const GLTFAccessor *acc, int idx, float out[3]) {
    const unsigned char *p = accessor_elem_ptr(doc, acc, idx);
    if (!p) { out[0]=out[1]=out[2]=0.0f; return; }

    int comp = acc->componentType;
    int comps = comp_count(acc->type);

    if (comp == 5126 && comps >= 3) {
        const float *f = (const float*)p;
        out[0] = f[0];
        out[1] = f[1];
        out[2] = f[2];
        return;
    }

    if (comp == 5123 && comps >= 3) {
        const unsigned short *s = (const unsigned short*)p;
        bool normalized = false; 
        if (normalized) {
            out[0] = s[0] / 65535.0f;
            out[1] = s[1] / 65535.0f;
            out[2] = s[2] / 65535.0f;
        } else {
            out[0] = (float)s[0];
            out[1] = (float)s[1];
            out[2] = (float)s[2];
        }
        return;
    }

    if (comp == 5121 && comps >= 3) {
        const unsigned char *c = (const unsigned char*)p;
        bool normalized = false;
        if (normalized) {
            out[0] = c[0] / 255.0f;
            out[1] = c[1] / 255.0f;
            out[2] = c[2] / 255.0f;
        } else {
            out[0] = (float)c[0];
            out[1] = (float)c[1];
            out[2] = (float)c[2];
        }
        return;
    }

    if (comp == 5122 && comps >= 3) {
        const short *s = (const short*)p;
        out[0] = (float)s[0];
        out[1] = (float)s[1];
        out[2] = (float)s[2];
        return;
    }
    if (comp == 5125 && comps >= 3) { 
        const unsigned int *u = (const unsigned int*)p;
        out[0] = (float)u[0];
        out[1] = (float)u[1];
        out[2] = (float)u[2];
        return;
    }

    out[0]=out[1]=out[2]=0.0f;
}
static void fetch_vec4(const GLTFDoc *doc, const GLTFAccessor *acc, int idx, vec4 * out) {
    const unsigned char *p = accessor_elem_ptr(doc, acc, idx);
    if (!p) { out->x=out->y=out->z=out->w=0.0f; return; }

    int comp = acc->componentType;
    int comps = comp_count(acc->type);

    if (comp == 5126 && comps >= 3) {
        const float *f = (const float*)p;
        out->x = f[0];
        out->y = f[1];
        out->z = f[2];
        out->w = f[3];
        return;
    }

    if (comp == 5123 && comps >= 3) {
        const unsigned short *s = (const unsigned short*)p;
        bool normalized = false; 
        if (normalized) {
            out->x = s[0] / 65535.0f;
            out->y = s[1] / 65535.0f;
            out->z = s[2] / 65535.0f;
            out->w = s[3] / 65535.0f;
        } else {
            out->x = (float)s[0];
            out->y = (float)s[1];
            out->z = (float)s[2];
            out->w = (float)s[3];
        }
        return;
    }

    if (comp == 5121 && comps >= 3) {
        const unsigned char *c = (const unsigned char*)p;
        bool normalized = false;
        if (normalized) {
            out->x = c[0] / 255.0f;
            out->y = c[1] / 255.0f;
            out->z = c[2] / 255.0f;
            out->w = c[3] / 255.0f;
        } else {
            out->x = (float)c[0];
            out->y = (float)c[1];
            out->z = (float)c[2];
            out->w = (float)c[3];
        }
        return;
    }

    if (comp == 5122 && comps >= 3) {
        const short *s = (const short*)p;
        out->x = (float)s[0];
        out->y = (float)s[1];
        out->z = (float)s[2];
        out->w = (float)s[3];
        return;
    }
    if (comp == 5125 && comps >= 3) { 
        const unsigned int *u = (const unsigned int*)p;
        out->x = (float)u[0];
        out->y = (float)u[1];
        out->z = (float)u[2];
        out->w = (float)u[3];
        return;
    }

    out->x=out->y=out->z=out->w=0.0f;
}
static void fetch_vec2f(GLTFDoc *doc, const GLTFAccessor *acc, int idx, float out[2]) {
    const unsigned char* p = (const unsigned char*)accessor_elem_ptr(doc, acc, idx);
    if (acc->componentType == 5126) {
        const float *f = (const float*)p;
        out[0]=f[0]; out[1]=f[1];
    } else {
        out[0]=out[1]=0.0f;
    }
}
void fetch_joints4_from_accessor(const GLTFDoc *doc, const GLTFAccessor *acc, int vertexIndex, int out_joints[4]) {
    out_joints[0]=out_joints[1]=out_joints[2]=out_joints[3] = 0;

    if (!acc) return;
    int elems = comp_count(acc->type);
    if (elems < 1) return;

    GLTFBufferView *view = &doc->views[acc->bufferView];
    uint8_t *base = accessor_base_ptr(doc, acc);

    size_t comp_sz = comp_size(acc->componentType);
    size_t elem_sz = comp_sz * elems;
    size_t stride = (view->byteStride && view->byteStride > 0) ? view->byteStride : elem_sz;

    uint8_t *src = base + (size_t)vertexIndex * stride;
    for (int k = 0; k < elems && k < 4; ++k) {
        if (acc->componentType == 5121) { 
            out_joints[k] = *(uint8_t *)(src + k * comp_sz);
        } else if (acc->componentType == 5123) { 
            out_joints[k] = *(uint16_t *)(src + k * comp_sz);
        } else if (acc->componentType == 5125) { 
            out_joints[k] = *(uint32_t *)(src + k * comp_sz);
        }
    }
}

void fetch_weights4_from_accessor(const GLTFDoc *doc, const GLTFAccessor *acc, int vertexIndex, float out_weights[4]) {
    out_weights[0]=out_weights[1]=out_weights[2]=out_weights[3]=0.0f;
    if (!acc) return;
    int elems = comp_count(acc->type);
    if (elems < 1) return;

    GLTFBufferView *view = &doc->views[acc->bufferView];
    uint8_t *base = accessor_base_ptr(doc, acc);

    size_t comp_sz = comp_size(acc->componentType);
    size_t elem_sz = comp_sz * elems;
    size_t stride = (view->byteStride && view->byteStride > 0) ? view->byteStride : elem_sz;

    uint8_t *src = base + (size_t)vertexIndex * stride;

    for (int k = 0; k < elems && k < 4; ++k) {
        if (acc->componentType == 5126) { 
            float v;
            memcpy(&v, src + k*4, 4);
            out_weights[k] = v;
        } else if (acc->componentType == 5121) { 
            uint8_t v = src[k];
            out_weights[k] = (float)v / 255.0f;
        } else if (acc->componentType == 5123) { 
            uint16_t v; memcpy(&v, src + k*2, 2);
            out_weights[k] = (float)v / 65535.0f;
        } else {
            out_weights[k] = 0.0f;
        }
    }
    float s = out_weights[0] + out_weights[1] + out_weights[2] + out_weights[3];
    if (s > 0.0f && fabsf(s - 1.0f) > 1e-6f) {
        out_weights[0] /= s; out_weights[1] /= s; out_weights[2] /= s; out_weights[3] /= s;
    }
}

static unsigned int fetch_index(GLTFDoc *doc, const GLTFAccessor *acc, int idx) {
    const unsigned char* p = (const unsigned char*)accessor_elem_ptr(doc, acc, idx);
    switch (acc->componentType) {
        case 5125: return *(const unsigned int*)p;        
        case 5123: return *(const unsigned short*)p;      
        case 5121: return *(const unsigned char*)p;       
        default:   return 0;
    }
}


mat4 gltf_to_engine_basis() {
    return (mat4){
        1, 0,  0, 0,
        0, 0, -1, 0,
        0, 1,  0, 0,
        0, 0,  0, 1
    };
}
static void compute_node_globals_recursive(GLTFDoc *doc, int idx, const mat4 *parent_global) {
    MeshNode *node = &doc->nodes[idx];
    mat4 local = compose_trs_from_mesh_node(node);

    if (parent_global == NULL) {
        mat4 basis = gltf_to_engine_basis();
        node->local_matrix = local;
        node->global_matrix = local;
    } else {
        node->local_matrix = local;
        node->global_matrix = m4_mul(*parent_global, local);
    }

    for (int i = 0; i < node->child_count; ++i) {
        compute_node_globals_recursive(doc, node->children[i], &node->global_matrix);
    }
}

static void compute_all_node_globals(GLTFDoc *doc) {
    for (int i = 0; i < doc->node_count; ++i) {
        if (doc->nodes[i].parent == -1) {
            compute_node_globals_recursive(doc, i, NULL);
        }
    }
}
static void bbox_of_points(vec3 *pts, int n, vec3 *minp, vec3 *maxp) {
    vec3 mn = { INFINITY, INFINITY, INFINITY}, mx = { -INFINITY, -INFINITY, -INFINITY};
    for (int i=0;i<n;i++){
        mn.x = fminf(mn.x, pts[i].x); mx.x = fmaxf(mx.x, pts[i].x);
        mn.y = fminf(mn.y, pts[i].y); mx.y = fmaxf(mx.y, pts[i].y);
        mn.z = fminf(mn.z, pts[i].z); mx.z = fmaxf(mx.z, pts[i].z);
    }
    *minp = mn; *maxp = mx;
}
static float extent(vec3 a, vec3 b){ return fmaxf(0.0f, b.x-a.x) + 0.0f; } 

static vec3 apply_mapping_to_point(const vec3 p, const int perm[3], const int sign[3]) {
    float src[3] = { p.x, p.y, p.z };
    vec3 out;
    out.x = sign[0] * src[perm[0]];
    out.y = sign[1] * src[perm[1]];
    out.z = sign[2] * src[perm[2]];
    return out;
}

static const int perms[6][3] = {
    {0,1,2}, {0,2,1}, {1,0,2}, {1,2,0}, {2,0,1}, {2,1,0}
};

int detect_best_axis_mapping(GLTFDoc *doc, GLTFAccessor *posAcc, MeshNode *node, int out_perm[3], int out_sign[3]) {
    int count = posAcc->count;
    int maxSample = 1500;
    int step = 1 + (count / maxSample);
    int n = 0;
    vec3 *pts = malloc(sizeof(vec3) * ((count+step-1)/step));
    for (int i=0; i<count; i+=step) {
        float tmp[3]; fetch_vec3f(doc, posAcc, i, tmp);
        vec3 w = mat4_mul_point(node->global_matrix, (vec3){tmp[0],tmp[1],tmp[2]});
        pts[n++] = w;
    }

    float bestScore = 1e30f;
    int best_p = 0, best_s = 0;
    for (int pi=0; pi<6; ++pi) {
        for (int s=0; s<8; ++s) {
            int sign[3] = { (s&1)?-1:1, (s&2)?-1:1, (s&4)?-1:1 };
            
            vec3 mn = { INFINITY, INFINITY, INFINITY}, mx = { -INFINITY, -INFINITY, -INFINITY};
            for (int k=0; k<n; ++k) {
                vec3 q = apply_mapping_to_point(pts[k], perms[pi], sign);
                if (q.x < mn.x) mn.x = q.x; if (q.x > mx.x) mx.x = q.x;
                if (q.y < mn.y) mn.y = q.y; if (q.y > mx.y) mx.y = q.y;
                if (q.z < mn.z) mn.z = q.z; if (q.z > mx.z) mx.z = q.z;
            }
            float extX = mx.x - mn.x;
            
            float extY = mx.y - mn.y;
            float extZ = mx.z - mn.z;
            if (extX < 0.001f || extY < 0.001f) continue;
            float ratio = fabsf(extX - extY) / fmaxf(extX, extY);
            float zpen = extZ / fmaxf(1.0f, fmaxf(extX, extY)); 
            float score = ratio + 0.4f * zpen;
            if (score < bestScore) { bestScore = score; best_p = pi; best_s = s; }
        }
    }

    free(pts);
    out_perm[0] = perms[best_p][0]; out_perm[1] = perms[best_p][1]; out_perm[2] = perms[best_p][2];
    out_sign[0] = (best_s & 1) ? -1 : 1;
    out_sign[1] = (best_s & 2) ? -1 : 1;
    out_sign[2] = (best_s & 4) ? -1 : 1;
    return 1;
}

static float *expand_accessor(const GLTFAccessor *acc, int *out_elem_count, GLTFDoc * doc) {
    
    int elem_count = 1;
    if (strcmp(acc->type, "SCALAR") == 0) {
        elem_count = 1;
    } else if (strcmp(acc->type, "VEC2") == 0){
        
        elem_count = 2;
    } else if (strcmp(acc->type, "VEC3") == 0){
        
        elem_count = 3;
    } else if (strcmp(acc->type, "VEC4") == 0){
        
        elem_count = 4;
    } else if (strcmp(acc->type, "MAT3") == 0){
        
        elem_count = 9;
    } else if (strcmp(acc->type, "MAT4") == 0){
        
        elem_count = 16;
    }
    if (out_elem_count) *out_elem_count = elem_count;

    GLTFBufferView *view = &doc->views[acc->bufferView];
    GLTFBuffer *buf  = &doc->buffers[view->buffer];

    unsigned char *src = buf->data + view->byteOffset + acc->byteOffset;

    float *out = malloc(sizeof(float) * acc->count * elem_count);

    for (int i = 0; i < acc->count; i++) {
        memcpy(&out[i * elem_count],
               src + i * (view->byteStride ? view->byteStride : elem_count * sizeof(float)),
               elem_count * sizeof(float));
    }
    return out;
}

static AnimationSampler expand_sampler(const GLTFAccessor *input_acc, const GLTFAccessor *output_acc, int interpolation, GLTFDoc * doc) {
    AnimationSampler s;
    s.count = input_acc->count;
    s.times = expand_accessor(input_acc, NULL, doc);
    s.values = expand_accessor(output_acc, &s.elem_count, doc);
    s.interpolation = interpolation;
    return s;
}

static Animation build_animation(const char *name, GLTFAnimation * animation, GLTFDoc * doc) {
    Animation anim;

    char * dot = strchr(name, '.');
    if (dot != NULL){
        *dot = '\0';
    }
    strncpy(anim.name, name, 63);
    anim.name[63] = '\0';

    anim.sampler_count = animation->sampler_count;
    anim.samplers = malloc(sizeof(AnimationSampler) * animation->sampler_count);
    for (int i = 0; i < animation->sampler_count; i++) {
        anim.samplers[i] = expand_sampler(&doc->accessors[animation->samplers[i].input], &doc->accessors[animation->samplers[i].output], animation->samplers[i].interpolation, doc);
    }

    anim.channel_count = animation->channel_count;
    anim.channels = malloc(sizeof(GLTFAnimationChannel) * animation->channel_count);
    memcpy(anim.channels, animation->channels, sizeof(GLTFAnimationChannel) * animation->channel_count);

    return anim;
}
void remap_joints_to_skin(const GLTFSkin *skin, const int raw[4], int out[4]) {
    for (int k = 0; k < 4; ++k) {
        int nodeIdx = raw[k];
        int found = -1;
        for (int j = 0; j < skin->joint_count; ++j) {
            if (skin->joints[j] == nodeIdx) { found = j; break; }
        }
        out[k] = (found >= 0) ? found : 0;
    }
}
vec3 map_x_z_neg_y(vec3 p){ return (vec3){ p.x,  p.z, p.y }; }
vec3 map_x_negz_y(vec3 p){ return (vec3){ p.x, -p.z,  p.y }; }




int load_gltf_texture(GLTFTexture * texture, GLTFSampler * samplers, GLTFImage * gltf_images, bool normal)
{

    int idx = -1;

    if (texture->source != -1)
    {

        GLTFImage * dimage = &gltf_images[texture->source];
        GLint min_filter;
        GLint mag_filter;
        GLint wrap_s;
        GLint wrap_t;

        if (texture->sampler != -1)
        {
            GLTFSampler * dsampler = &samplers[texture->sampler];
            min_filter = dsampler->min_filter;
            mag_filter = dsampler->mag_filter;
            wrap_s = dsampler->wrap_s;
            wrap_t = dsampler->wrap_t;
        }
        else
        {

            min_filter = GL_LINEAR_MIPMAP_LINEAR;
            mag_filter = GL_LINEAR;
            wrap_s = GL_REPEAT;
            wrap_t = GL_REPEAT;
            
        }

        idx = add_image_to_registry(dimage->uri, min_filter, mag_filter, wrap_s, wrap_t, (normal) ? 0 : 1);
        if (idx == -1) printf("AAAA\n");
        Image * images = CAST_VEC(GLOBALS.images, Image);
        Image * img = &images[idx];
        strncpy(img->name, dimage->name, 254);
        printf("LOADING: %s\n", img->name);
        img->name[255] = '\0';
        strncpy(img->mime_type, dimage->mime_type, 254);
        img->mime_type[255] = '\0';
        strncpy(img->path, dimage->uri, 254);
        img->path[255] = '\0';
        img->min_filter = min_filter;
        img->mag_filter = mag_filter;
        img->wrap_s = wrap_s;
        img->wrap_t = wrap_t;
        img->buf = NULL;
        
        
    }
            
    return idx;


    
}


void apply_static_mesh_to_children(MeshNode * mns, Mesh * meshes, MeshNode * mn, bool static_mesh)
{
    mn->static_mesh = static_mesh;
    if (mn->mesh != -1)
    {
        meshes[mn->mesh].static_mesh = static_mesh;
    }
    for (int i = 0; i < mn->child_count; i++)
    {
        apply_static_mesh_to_children(mns, meshes, &mns[mn->children[i]], static_mesh);
    }

}

AABB determine_aabb_of_children(MeshNode * mns, Mesh * meshes, MeshNode * mn)
{

    AABB aabb;
    aabb.min_x = INFINITY;
    aabb.min_y = INFINITY;
    aabb.min_z = INFINITY;
    aabb.max_x = -INFINITY;
    aabb.max_y = -INFINITY;
    aabb.max_z = -INFINITY;

    if (mn->mesh != -1)
    {
        Mesh * m = &meshes[mn->mesh];
        mat4 inv;
        m4_inverse((float*)&inv, (float*)&mn->global_matrix);
        AABB current = m->aabb;

        if (current.min_x < aabb.min_x) aabb.min_x = current.min_x;
        if (current.min_y < aabb.min_y) aabb.min_y = current.min_y;
        if (current.min_z < aabb.min_z) aabb.min_z = current.min_z;
        if (current.max_x > aabb.max_x) aabb.max_x = current.max_x;
        if (current.max_y > aabb.max_y) aabb.max_y = current.max_y;
        if (current.max_z > aabb.max_z) aabb.max_z = current.max_z;
    }
    
    for (int i = 0; i < mn->child_count; i++)
    {
        AABB ch = determine_aabb_of_children(mns, meshes, &mns[mn->children[i]]);
        if (ch.min_x < aabb.min_x) aabb.min_x = ch.min_x;
        if (ch.min_y < aabb.min_y) aabb.min_y = ch.min_y;
        if (ch.min_z < aabb.min_z) aabb.min_z = ch.min_z;
        if (ch.max_x > aabb.max_x) aabb.max_x = ch.max_x;
        if (ch.max_y > aabb.max_y) aabb.max_y = ch.max_y;
        if (ch.max_z > aabb.max_z) aabb.max_z = ch.max_z;
    }

    return aabb;
    
}


MeshNode * find_root_node_armature(MeshNode * mns, MeshNode * mn)
{

    if (strcmp(mn->name, "Root") == 0)
    {

        return mn;
        
    }

    if (mn->parent != -1)
    {
        
        return find_root_node_armature(mns, &mns[mn->parent]);
    }

    return NULL;
    
}



void load_gltf(const char *gltfPath)
{
    GLTFDoc doc = {0};
    char prefix[256];
    strcpy(prefix, gltfPath);
    char * last = strrchr(prefix, '/');
    if (last != NULL)
    {
        last[1] = '\0';
    }
    if (!parse_gltf_json(gltfPath, prefix, &doc)) {
        return;
    }

    compute_all_node_globals(&doc);
    int * mesh_indices = NULL; size_t mesh_indices_count = 0; size_t mesh_indices_capacity = 0;
    int * node_indices = NULL; size_t node_indices_count = 0; size_t node_indices_capacity = 0;
    Asset * asset = vector_expand(&GLOBALS.assets, 1);
    init_vector(&asset->mesh_indices, 0, sizeof(int));

    


    asset->aabb.min_x = INFINITY;
    asset->aabb.min_y = INFINITY;
    asset->aabb.min_z = INFINITY;
    asset->aabb.max_x = -INFINITY;
    asset->aabb.max_y = -INFINITY;
    asset->aabb.max_z = -INFINITY;
    size_t start_material_count = GLOBALS.materials.count;
    GLTFMaterial * mats = doc.materials;
    GLTFTexture * textures = doc.textures;
    GLTFSampler * samplers = doc.samplers;
    GLTFImage * images = doc.images;
    
    for (int i = 0; i < doc.material_count; i++)
    {
        GLTFMaterial * material = &mats[i];
        Material * mat = vector_expand(&GLOBALS.materials, 1);
        mat->diffuse_tex = -1;
        mat->arm_tex = -1;
        mat->mr_tex = -1;
        mat->ao_tex = -1;
        mat->normal_tex = -1;
        mat->fwd_shader = mats[i].fwd_shader;
        mat->gbuffer_shader = mats[i].gbuffer_shader;
        mat->override_arm = false;
        
        if (material->base_color_tex != -1)
        {

            GLTFTexture * diffuse = &textures[material->base_color_tex];
            int d = load_gltf_texture(diffuse, samplers, images, false);
            mat->diffuse_tex = d;
           
        }
        else
        {
            mat->diffuse_tex = -1;
        }
        mat->kd = (vec4){
                    material->base_color_factor[0],
                    material->base_color_factor[1],
                    material->base_color_factor[2],
                    material->base_color_factor[3]
                };
        if (material->normal_tex != -1)
        {

            GLTFTexture * normal = &textures[material->normal_tex];
            int d = load_gltf_texture(normal, samplers, images, true);
            mat->normal_tex = d;
           
        }
        else
        {
            mat->normal_tex = -1;
        }
        mat->normal_scale = material->normal_scale;

        mat->arm_tex = -1;
        // arm
        if (material->metallic_roughness_tex == material->occlusion_tex)
        {
            if (material->metallic_roughness_tex != -1)
            {
                GLTFTexture * arm = &textures[material->metallic_roughness_tex];
                int a = load_gltf_texture(arm, samplers, images, true);
                mat->arm_tex = a;
            }
        }
        else
        {

            if (material->occlusion_tex != -1)
            {
                
                GLTFTexture * occ = &textures[material->occlusion_tex];
                int o = load_gltf_texture(occ, samplers, images, true);
                mat->arm_tex = o;
            }
            
            if (material->metallic_roughness_tex != -1)
            {
                GLTFTexture * mr = &textures[material->metallic_roughness_tex];
                int mr_tex = load_gltf_texture(mr, samplers, images, true);
                mat->arm_tex = mr_tex;
            }

        }
        if (mat->arm_tex == -1)
        {
            mat->arm_tex = DEFAULT_ARM;
        }

        mat->ao_strength = material->occlusion_strength;
        mat->metallic = material->metallic_factor;
        mat->roughness = material->roughness_factor;
        if (material->emissive_tex != -1)
        {

            GLTFTexture * emissive = &textures[material->emissive_tex];
            int d = load_gltf_texture(emissive, samplers, images, false);
            mat->emissive_tex = d;
           
        }
        else
        {
            mat->emissive_tex = -1;
        }

        mat->ke = (vec4){
            material->emissive_factor[0],
            material->emissive_factor[1],
            material->emissive_factor[2],
            0.0
        };
        mat->ka = (vec4){0,0,0,0};
        mat->ks = (vec4){0,0,0,0};

                
        strncpy(mat->name, material->name, 62);
        mat->name[63] = '\0';
        mat->alpha_mode = material->alpha_mode;
        mat->alpha_cutoff = material->alpha_cutoff;
        mat->double_sided = material->double_sided;
        memcpy(mat->absorption_coefficients, material->absorption_coefficients, sizeof(float) * AUDIO_NUM_BANDS);
        memcpy(mat->scattering_coefficients, material->scattering_coefficients, sizeof(float) * AUDIO_NUM_BANDS);
        
        mat->emissive_strength = material->emissive_strength;
        mat->override_transparent = material->override_transparent;
        
    }

    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    size_t starting_mesh = GLOBALS.meshes.count;
    int root = 0;
    Vector roots;
    init_vector(&roots, 0, sizeof(size_t));
    for (int ni = 0; ni < doc.node_count; ++ni) {
        MeshNode *node = &doc.nodes[ni];
        if (node->parent == -1) {
            // root node, set root for the asset
            root = ni;
            size_t * r = vector_expand(&roots, 1);
            *r = ni;
        }
        if (node->mesh < 0 || node->mesh >= doc.meshCount) continue;   

        
        bool valid = true;
        for (int i = 0; i < mesh_indices_count; i++) {
            if (node->mesh == mesh_indices[i]) valid = false;
        }
        if (!valid) continue;

        GLTFMesh *mesh = &doc.meshes[node->mesh];

        if (mesh_indices_count >= mesh_indices_capacity){
            mesh_indices_capacity = (mesh_indices_capacity + 2) * 2;
            mesh_indices = realloc(mesh_indices, sizeof(int) * mesh_indices_capacity);
        }
        mesh_indices[mesh_indices_count++] = node->mesh;


        Mesh * m = vector_expand(&GLOBALS.meshes, 1);
        m->static_mesh = node->static_mesh;
        int * new_mesh = vector_expand(&asset->mesh_indices, 1);
        *new_mesh = GLOBALS.meshes.count-1;
        m->id = GLOBALS.meshes.count-1;
        m->asset_id = GLOBALS.assets.count-1;
        init_vector(&m->tris, 0, sizeof(MeshFace));
        init_vector(&m->vertices, 0, sizeof(Point));
        init_vector(&m->submeshes, 0, sizeof(SubMesh));
        strcpy(m->name, node->name);
        node->mesh = GLOBALS.meshes.count-1;        
        m->aabb.min_x = INFINITY;
        m->aabb.min_y = INFINITY;
        m->aabb.min_z = INFINITY;
        m->aabb.max_x = -INFINITY;
        m->aabb.max_y = -INFINITY;
        m->aabb.max_z = -INFINITY;
        size_t current_mesh = GLOBALS.meshes.count-1;
        node->mesh = current_mesh;




        for (int p = 0; p < mesh->primCount; ++p) {
            GLTFPrimitive *prim = &mesh->prims[p];
            if (prim->position < 0 || prim->indices < 0) continue;

            GLTFAccessor *aPos = &doc.accessors[prim->position];
            GLTFAccessor *aIdx = &doc.accessors[prim->indices];
            GLTFAccessor *aNrm = (prim->normal >= 0) ? &doc.accessors[prim->normal] : NULL;
            GLTFAccessor *aTan = (prim->tangent >= 0) ? &doc.accessors[prim->tangent] : NULL;
            GLTFAccessor *aUV  = (prim->texcoord0 >= 0) ? &doc.accessors[prim->texcoord0] : NULL;
            GLTFAccessor *accJ = (prim->joints0 >= 0) ? &doc.accessors[prim->joints0] : NULL;
            GLTFAccessor *accW = (prim->weights0 >= 0) ? &doc.accessors[prim->weights0] : NULL;
            GLTFSkin *skin = (node->skin >= 0) ? &doc.skins[node->skin] : NULL;
            SubMesh * sm = vector_expand(&m->submeshes, 1);

            if (prim->material == -1)
            {
                sm->material = 0;
            }
            else
            {
                sm->material = prim->material + start_material_count;    
            }
            sm->lm_offset = 0;
            sm->draw_offset = 0;
            sm->first_vertex = m->vertices.count;

            int vertex_count = aPos->count;
            // loop through and emit vertices
            for (int i = 0; i < vertex_count; i++)
            {
                
                float p0[3];
                fetch_vec3f(&doc, aPos, i, p0);
                vec3 V0 = (vec3){p0[0],p0[1],p0[2]};

                vec3 N0 = {0,0,1};
                vec4 T0 = {1,0,0,1};

                float uv0f[2]={0,0};
                if (aUV)
                {
                    fetch_vec2f(&doc, aUV, i, uv0f);
                }

                vec2 UV = {uv0f[0],uv0f[1]};
                int rawJ0[4] = {0}, rawJ1[4] = {0}, rawJ2[4] = {0};
                float W0[4] = {1}, W1[4] ={1}, W2[4] = {1};
                int J0[4] = {0}, J1[4] = {0}, J2[4] = {0};

                if (prim->joints0 >= 0 && prim->weights0 >= 0 && node->skin >= 0)
                {

                    V0 = (vec3){p0[0],p0[1],p0[2]};
                    fetch_joints4_from_accessor(&doc, accJ, i, rawJ0);
                    fetch_weights4_from_accessor(&doc, accW, i, W0);
                    remap_joints_to_skin(skin, rawJ0, J0);
                    rawJ0[0] += 1;
                    rawJ0[1] += 1;
                    rawJ0[2] += 1;
                    rawJ0[3] += 1;

                    if (aNrm) {

                        float nn0[3]; fetch_vec3f(&doc, aNrm, i, nn0);
                        N0 = (vec3){nn0[0], nn0[1], nn0[2]};
                        N0 = vec3_normalize(N0);

                    }
                    if (aTan)
                    {

                        vec4 tt0; fetch_vec4(&doc, aTan, i, &tt0);
                        T0 = vec3_to_vec4(vec3_normalize(vec4_to_vec3(tt0)));
                        T0.w = tt0.w;
                        
                    }

                }
                else
                {
                    
                    rawJ0[0] = 0;
                    rawJ0[1] = 0;
                    rawJ0[2] = 0;
                    rawJ0[3] = 0;
                    W0[0] = 1;
                    W0[1] = 0;
                    W0[2] = 0;
                    W0[3] = 0;
                    
                    if (aNrm)
                    {

                        float nn0[3]; fetch_vec3f(&doc, aNrm, i, nn0);
                        vec3 n0 = {nn0[0], nn0[1], nn0[2]};

                        N0 = vec3_normalize(n0);

                    }
                    if (aTan)
                    {

                        vec4 tt0; fetch_vec4(&doc, aTan, i, &tt0);

                        T0 = vec3_to_vec4(vec3_normalize(vec4_to_vec3(tt0)));
                        T0.w = tt0.w;
                        
                    }
                }
                

                Point * pt = vector_expand(&m->vertices, 1);
                mat4 inv;
                m4_inverse((float*)&inv, (float*)&node->global_matrix);
                pt->pos = V0;
                if (V0.x < m->aabb.min_x) m->aabb.min_x = V0.x;
                if (V0.y < m->aabb.min_y) m->aabb.min_y = V0.y;
                if (V0.z < m->aabb.min_z) m->aabb.min_z = V0.z;
                if (V0.x > m->aabb.max_x) m->aabb.max_x = V0.x;
                if (V0.y > m->aabb.max_y) m->aabb.max_y = V0.y;
                if (V0.z > m->aabb.max_z) m->aabb.max_z = V0.z;
                pt->normal = N0;
                pt->tangent = T0;
                pt->uv = UV;
                pt->joints = (ivec4){rawJ0[0], rawJ0[1], rawJ0[2], rawJ0[3]};
                pt->weights = (vec4){W0[0], W0[1], W0[2], W0[3]};

            }
            sm->vertex_count = vertex_count;
            


            Point * pts = CAST_VEC(m->vertices, Point);


            int sm_cnt = 0;
            sm->first_index = m->tris.count * 3;
            // loop through and emit indices
            int indexCount = aIdx->count;
            for (int i = 0; i < indexCount; i += 3) {

                unsigned int i0 = fetch_index(&doc, aIdx, i+0);
                unsigned int i1 = fetch_index(&doc, aIdx, i+1);
                unsigned int i2 = fetch_index(&doc, aIdx, i+2);

                MeshFace * tri = vector_expand(&m->tris, 1);
                tri->indices[0] = i0 + sm->first_vertex;
                tri->indices[1] = i1 + sm->first_vertex;
                tri->indices[2] = i2 + sm->first_vertex;
                tri->material_id = sm->material;
                tri->normal = compute_tri_normal(pts[tri->indices[0]].pos, pts[tri->indices[1]].pos, pts[tri->indices[2]].pos);
                sm_cnt += 3;

                
            }
            sm->index_count = sm_cnt;
        }
        calculate_aabb_extents(&m->aabb);
    }


    strcpy(asset->path, gltfPath);
    asset->asset_id = GLOBALS.assets.count-1;
    MeshNode * root_node = &doc.nodes[root];


    meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    size_t * root_nodes = CAST_VEC(roots, size_t);
    for (int i = 0; i < roots.count; i++)
    {
        MeshNode * root = &doc.nodes[root_nodes[i]];
        if (root->mesh != -1)
        {
            meshes[root->mesh].static_mesh = root->static_mesh;
        }

        for (int j = 0; j < root->child_count; j++)
        {
            
            apply_static_mesh_to_children(doc.nodes, meshes, &doc.nodes[root->children[j]], root->static_mesh);

        }
    }

    for (int i = starting_mesh; i < GLOBALS.meshes.count; i++)
    {
        
        Mesh * m = &meshes[i];
        init_mesh(m);
        if (asset->aabb.min_x > m->aabb.min_x) asset->aabb.min_x = m->aabb.min_x;
        if (asset->aabb.min_y > m->aabb.min_y) asset->aabb.min_y = m->aabb.min_y;
        if (asset->aabb.min_z > m->aabb.min_z) asset->aabb.min_z = m->aabb.min_z;
        if (asset->aabb.max_x < m->aabb.max_x) asset->aabb.max_x = m->aabb.max_x;
        if (asset->aabb.max_y < m->aabb.max_y) asset->aabb.max_y = m->aabb.max_y;
        if (asset->aabb.max_z < m->aabb.max_z) asset->aabb.max_z = m->aabb.max_z;
    }
    calculate_aabb_extents(&asset->aabb);
    
    asset->animation_count = doc.animation_count;
    if (doc.animation_count > 0) {
        
        asset->animations = calloc(asset->animation_count, sizeof(Animation));
        for (int i = 0; i < doc.animation_count; i++) {
        
            asset->animations[i].samplers = NULL;
            asset->animations[i].channels = NULL;
            asset->animations[i] = build_animation(doc.animations[i].name, &doc.animations[i], &doc);
            asset->animations[i].asset_id = GLOBALS.assets.count-1;
        }
    }
    else
    {
        asset->animations = NULL;
    }

    asset->skin_count = doc.skin_count;
    if (doc.skin_count > 0)
    {
        for (int i = 0; i < doc.skin_count; i++) {

            GLTFSkin * skin = &doc.skins[i];

        
            GLTFAccessor *acc = &doc.accessors[skin->ibm_accessor];
            GLTFBufferView *view = &doc.views[acc->bufferView];

            uint8_t *base = doc.buffers[view->buffer].data;

            uint8_t *data = base + view->byteOffset + acc->byteOffset;

            size_t stride = view->byteStride ? view->byteStride : sizeof(float) * 16;

            skin->inverse_bind = malloc(sizeof(mat4) * skin->joint_count);

            for (int j = 0; j < skin->joint_count; j++) {
                memcpy(&skin->inverse_bind[j], data + j * stride, sizeof(mat4));
            }
        }
        asset->skins = calloc(asset->skin_count, sizeof(GLTFSkin));
        for (int i = 0; i < doc.skin_count; i++) {
            asset->skins[i].joint_count = doc.skins[i].joint_count;
            if (doc.skins[i].joint_count > 0)
            {
                
                asset->skins[i].joints = malloc(sizeof(int) * asset->skins[i].joint_count);
                memcpy(asset->skins[i].joints, doc.skins[i].joints, sizeof(int) * asset->skins[i].joint_count);
                asset->skins[i].inverse_bind = malloc(sizeof(mat4) * asset->skins[i].joint_count);
                memcpy(asset->skins[i].inverse_bind, doc.skins[i].inverse_bind, sizeof(mat4) * asset->skins[i].joint_count);
                asset->skins[i].final_mat = calloc(doc.skins[i].joint_count, sizeof(mat4));
            }
            else
            {
                asset->skins[i].joints = NULL;
                asset->skins[i].inverse_bind = NULL;
                asset->skins[i].final_mat = NULL;
            }
            strcpy(asset->skins[i].name, doc.skins[i].name);
            GLTFSkin * skin = &asset->skins[i];
            printf("SKIN: %s\n", skin->name);

            for (int a = 0; a < asset->animation_count; a++) {

                Animation * anim = &asset->animations[a];
                for (int ch = 0; ch < anim->channel_count; ch++){

                    GLTFAnimationChannel * channel = &anim->channels[ch];
                    bool match_found = false;
                    for (int j = 0; j < skin->joint_count; j++){
                    
                        if (channel->target_node == skin->joints[j]){
                        
                            if (skin->animation_indices_count < MAX_ANIMATION_INDICES) {
                                skin->animation_indices[skin->animation_indices_count++] = a;
                                match_found = true;
                                break;
                            } else {
                                printf("MAX ANIMATIONS REACHED\n");
                                break;
                            }

                        }
                        if (match_found) break;
                    }
                    if (match_found) break;
                }
            }
            char idle_anim_name[128];
            sprintf(idle_anim_name, "%s_Idle", asset->skins[i].name);

        }

        
    }
    else
    {
        asset->skins = NULL;
    }
    asset->node_count = doc.node_count;
    if (asset->node_count > 0)
    {
        
        asset->nodes = calloc(asset->node_count, sizeof(MeshNode));
        for (int i = 0; i < doc.node_count; i++) {
            asset->nodes[i] = doc.nodes[i];
            asset->nodes[i].id = i;
            asset->nodes[i].mesh= doc.nodes[i].mesh;
            asset->nodes[i].uv_map = NULL;
            asset->nodes[i].ent = NULL;
        
            asset->nodes[i].pc = NULL;

            asset->nodes[i].aabb = determine_aabb_of_children(doc.nodes, meshes, &doc.nodes[i]);
            calculate_aabb_extents(&asset->nodes[i].aabb);
            
            strcpy(asset->nodes[i].name, doc.nodes[i].name);
            asset->nodes[i].child_count = doc.nodes[i].child_count;
            if (doc.nodes[i].child_count > 0){

                memcpy(asset->nodes[i].children, doc.nodes[i].children, sizeof(int) * doc.nodes[i].child_count);
            
            } else {
                asset->nodes[i].child_count = 0;
            }
            strcpy(asset->nodes[i].name, doc.nodes[i].name);
            
        }

    }
    else
    {
        asset->nodes = NULL;
    }

        
    for (int i = 0; i < doc.bufferCount; i++)
    {
        if (doc.buffers[i].data) free(doc.buffers[i].data);
        if (doc.buffers[i].uri) free(doc.buffers[i].uri);
    }
    if (doc.buffers) free(doc.buffers);
    for (int i = 0; i < doc.meshCount; i++)
    {
        if (doc.meshes[i].prims) free(doc.meshes[i].prims);
    }
    if (doc.meshes) free(doc.meshes);
    if (doc.accessors) free(doc.accessors);
    if (doc.materials) free(doc.materials);
    if (doc.nodes) free(doc.nodes);
    if (doc.views) free(doc.views);
    for (int i = 0; i < doc.skin_count; i++)
    {
        if (doc.skins[i].final_mat) free(doc.skins[i].final_mat);
        if (doc.skins[i].inverse_bind) free(doc.skins[i].inverse_bind);
        if (doc.skins[i].joints) free(doc.skins[i].joints);
    }
    if (doc.skins) free(doc.skins);
    if (doc.animations) {
        for (int i = 0; i < doc.animation_count; i++){
            
            if (doc.animations[i].channels) free(doc.animations[i].channels);
            if (doc.animations[i].samplers) free(doc.animations[i].samplers);

        }
        free(doc.animations);
    }
    if (mesh_indices) free(mesh_indices);
    if (node_indices) free(node_indices);
    if (roots.buffer) free(roots.buffer);
}
