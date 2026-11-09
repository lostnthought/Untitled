
#include "load/init.h"
#include "utilities/types.h"
#include "core/shaders.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char * read_file(const char *filename) {

    FILE * f = fopen(filename, "rb");
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);
    char * buffer = (char *)malloc(size + 1);
    if (!buffer) return NULL;
    size_t bytes_read = fread(buffer, 1, size, f);
    buffer[bytes_read] = '\0';
    fclose(f);

    size_t write_idx = 0;
    for (size_t i = 0; i < bytes_read; i++) {
        if (buffer[i] != '\r') {
            buffer[write_idx++] = buffer[i];
        }
    }
    buffer[write_idx] = '\0';    
    return buffer;

}

GLuint create_compute_shader(const char *filename) {

    const char *src = read_file(filename);
    if (!src) {
        fprintf(stderr, "Failed to read shader file: %s\n", filename);
        return 0;
    }

    GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
    GLint src_len = (GLint)strlen(src);
    glShaderSource(shader, 1, &src, &src_len);
    glCompileShader(shader);
    free((void*)src); // don't forget to free it after compile

    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (!status) {
        char log[2048];
        glGetShaderInfoLog(shader, 2048, NULL, log);
        fprintf(stderr, "Compute Shader Error %s: %s\n", filename, log);
        exit(0);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, shader);
    glLinkProgram(program);

    GLint status2;
    glGetProgramiv(program, GL_LINK_STATUS, &status2);
    if (!status2) {
        char log[2048];
        glGetProgramInfoLog(program, 2048, NULL, log);
        fprintf(stderr, "Compute Shader LINKER Error %s: %s\n", filename, log);
        exit(0);
        return 0;
    }

    glDeleteShader(shader);
    return program;
}


GLuint compile_shader(GLenum type, const char *filename) {
    const char *source = read_file(filename);
    if (!source) return 0;

    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    free((void *)source);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("\\nShader compile error (%s):\\n%s\\n", filename, log);
        exit(0);
        return 0;
    } else {
        glGetShaderInfoLog(shader, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader compile warning (%s):\\n%s\\n", filename, log);
        }
    }

    return shader;
}

GLuint compile_gbuffer_frag_shader(GLenum type, const char *filename, int i) {
    const char *source = read_file(filename);
    if (!source) return 0;

    GLuint shader = glCreateShader(type);

    const char * version = "#version 460\n";
    char macro[256];
    switch(i)
    {
        case DVIEW_DEFAULT:
            sprintf(macro, "\n");
            break;
        case DVIEW_ALBEDO:
            sprintf(macro, "#define ALBEDO\n");
            break;
        case DVIEW_ARM:
            sprintf(macro, "#define ARM\n");
            break;
        case DVIEW_EMISSIVE:
            sprintf(macro, "#define EMISSIVE\n");
            break;
        case DVIEW_AO:
            sprintf(macro, "#define AO\n");
            break;
        case DVIEW_UV:
            sprintf(macro, "#define UV\n");
            break;
        case DVIEW_POM_VDIR:
            sprintf(macro, "#define POM_VDIR\n");
            break;
        case DVIEW_CLUSTERS:
            sprintf(macro, "#define CLUSTERS\n");
            break;
        case DVIEW_NORMAL:
            sprintf(macro, "#define NORMAL\n");
            break;
        case DVIEW_ORIGINAL_NORMAL:
            sprintf(macro, "#define ORIGINAL_NORMAL\n");
            break;
        case DVIEW_DIRECT:
            sprintf(macro, "#define DIRECT\n");
            break;
        case DVIEW_INDIRECT:
            sprintf(macro, "#define INDIRECT\n");
            break;
        case DVIEW_LM_IRRADIANCE:
            sprintf(macro, "#define LM_IRRADIANCE\n");
            break;
        case DVIEW_LM_DIRECTIONAL:
            sprintf(macro, "#define LM_DIRECTIONAL\n");
            break;
        case DVIEW_RPROBE:
            sprintf(macro, "#define RPROBE\n");
            break;
        case DVIEW_SPECULAR:
            sprintf(macro, "#define SPECULAR\n");
            break;
        case DVIEW_MAX:
            sprintf(macro, "#define POM\n");
            break;
        default:
            break;
    }
    const char * sources[3] =
    {
        version, macro, source
    };

    glShaderSource(shader, 3, sources, NULL);
    
    glCompileShader(shader);
    free((void *)source);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("\\nShader compile error (%s):\\n%s\\n", filename, log);
        exit(0);
        return 0;
    } else {
        glGetShaderInfoLog(shader, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader compile warning (%s):\\n%s\\n", filename, log);
        }
    }

    return shader;
}

GLuint compile_uber_frag_shader(GLenum type, const char *filename, int i) {
    const char *source = read_file(filename);
    if (!source) return 0;

    GLuint shader = glCreateShader(type);

    const char * version = "#version 430 core\n";
    char macro[256];
    switch(i)
    {
        case DVIEW_DEFAULT:
            sprintf(macro, "\n");
            break;
        case DVIEW_ALBEDO:
            sprintf(macro, "#define ALBEDO\n");
            break;
        case DVIEW_ARM:
            sprintf(macro, "#define ARM\n");
            break;
        case DVIEW_EMISSIVE:
            sprintf(macro, "#define EMISSIVE\n");
            break;
        case DVIEW_AO:
            sprintf(macro, "#define AO\n");
            break;
        case DVIEW_UV:
            sprintf(macro, "#define DVIEW_UV\n");
            break;
        case DVIEW_POM_VDIR:
            sprintf(macro, "#define POM_VDIR\n");
            break;
        case DVIEW_CLUSTERS:
            sprintf(macro, "#define CLUSTERS\n");
            break;
        case DVIEW_NORMAL:
            sprintf(macro, "#define NORMAL\n");
            break;
        case DVIEW_ORIGINAL_NORMAL:
            sprintf(macro, "#define ORIGINAL_NORMAL\n");
            break;
        case DVIEW_DIRECT:
            sprintf(macro, "#define DIRECT\n");
            break;
        case DVIEW_INDIRECT:
            sprintf(macro, "#define INDIRECT\n");
            break;
        case DVIEW_LM_IRRADIANCE:
            sprintf(macro, "#define LM_IRRADIANCE\n");
            break;
        case DVIEW_LM_DIRECTIONAL:
            sprintf(macro, "#define LM_DIRECTIONAL\n");
            break;
        case DVIEW_RPROBE:
            sprintf(macro, "#define RPROBE\n");
            break;
        case DVIEW_SPECULAR:
            sprintf(macro, "#define SPECULAR\n");
            break;
        case DVIEW_MAX:
            sprintf(macro, "#define POM\n");
            break;
        default:
            break;
    }
    const char * sources[3] =
    {
        version, macro, source
    };

    glShaderSource(shader, 3, sources, NULL);
    
    glCompileShader(shader);
    free((void *)source);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("\\nShader compile error %s (%s):\\n%s\\n", macro, filename, log);
        exit(0);
        return 0;
    } else {
        glGetShaderInfoLog(shader, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader compile warning %s (%s):\\n%s\\n", macro, filename, log);
        }
    }

    return shader;
}

GLuint compile_postprocess_shader(GLenum type, const char *filename, uint32_t PP_OPTIONS, bool is_dview, PostprocessDView dview) {
    const char *source = read_file(filename);
    if (!source) return 0;

    GLuint shader = glCreateShader(type);

    const char * version = "#version 430 core\n";
    char macro[10000];
    macro[0] = '\0';
    if (is_dview)
    {
        switch(dview)
        {
            case PPDVIEW_DEPTH:
                strcat(macro, "#define DEPTH\n");
                break;
            case PPDVIEW_VEL:
                strcat(macro, "#define VEL\n");
                break;
            case PPDVIEW_BOKEH:
                strcat(macro, "#define BOKEH\n");
                break;
            case PPDVIEW_LM:
                strcat(macro, "#define LM\n");
                break;
            default:
                break;
        }
    }
    else
    {
        if (PP_OPTIONS & (1 << PP_MBLUR))
        {
            strcat(macro, "#define MBLUR\n");
        }
        if (PP_OPTIONS & (1 << PP_DOF))
        {
            strcat(macro, "#define DOF\n");
        }
        if (PP_OPTIONS & (1 << PP_BLOOM))
        {
            strcat(macro, "#define BLOOM\n");
        }
        if (PP_OPTIONS & (1 << PP_NOISE))
        {
            strcat(macro, "#define NOISE\n");
        }
        if (PP_OPTIONS & (1 << PP_ACES))
        {
            strcat(macro, "#define ACES\n");
        }
        if (PP_OPTIONS & (1 << PP_LUT))
        {
            strcat(macro, "#define LUT\n");
        }
        
    }
    const char * sources[3] =
    {
        version, macro, source
    };

    glShaderSource(shader, 3, sources, NULL);
    
    glCompileShader(shader);
    free((void *)source);

    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetShaderInfoLog(shader, 512, NULL, log);
        printf("\\nShader compile error %s (%s):\\n%s\\n", macro, filename, log);
        exit(0);
        return 0;
    } else {
        glGetShaderInfoLog(shader, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader compile warning %s (%s):\\n%s\\n", macro, filename, log);
        }
    }

    return shader;
}

GLuint create_shader_program(const char * vertex_path, const char * fragment_path) {
    GLuint vertex_shader = compile_shader(GL_VERTEX_SHADER, vertex_path);
    GLuint fragment_shader = compile_shader(GL_FRAGMENT_SHADER, fragment_path);
    if (!vertex_shader || !fragment_shader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertex_shader);
    glAttachShader(program, fragment_shader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Shader link error:\\n%s\\n", log);
        exit(0);
        return 0;
    } else {
        glGetProgramInfoLog(program, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader link warning:\\n%s\\n", log);
        }
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);

    return program;
}

GLuint create_geometry_shader_program(const char *vertexPath, const char * geometryPath, const char *fragmentPath) {
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexPath);
    GLuint geometryShader = compile_shader(GL_GEOMETRY_SHADER, geometryPath);
    GLuint fragmentShader = compile_shader(GL_FRAGMENT_SHADER, fragmentPath);
    if (!vertexShader || !geometryShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, geometryShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Shader link error:\\n%s\\n", log);
        exit(0);
        return 0;
    } else {
        glGetProgramInfoLog(program, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader link warning:\\n%s\\n", log);
        }
}

    glDeleteShader(vertexShader);
    glDeleteShader(geometryShader);
    glDeleteShader(fragmentShader);

    return program;
}


GLuint create_gbuffer_program(const char *vertexPath, const char *fragmentPath, int i) {
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShader = compile_gbuffer_frag_shader(GL_FRAGMENT_SHADER, fragmentPath, i);
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Shader link error:\\n%s\\n", log);
        exit(0);
        return 0;
    } else {
        glGetProgramInfoLog(program, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader link warning:\\n%s\\n", log);
        }
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


GLuint create_ubershader_program(const char *vertexPath, const char * geometryPath, const char *fragmentPath, int i) {
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShader = compile_uber_frag_shader(GL_FRAGMENT_SHADER, fragmentPath, i);
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Shader link error:\\n%s\\n", log);
        exit(0);
        return 0;
    } else {
        glGetProgramInfoLog(program, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader link warning:\\n%s\\n", log);
        }
}

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}


GLuint create_postprocess_program(const char *vertexPath, const char *fragmentPath, int i, bool is_dview, PostprocessDView dview) {
    GLuint vertexShader = compile_shader(GL_VERTEX_SHADER, vertexPath);
    GLuint fragmentShader = compile_postprocess_shader(GL_FRAGMENT_SHADER, fragmentPath, i, is_dview, dview);
    if (!vertexShader || !fragmentShader) return 0;

    GLuint program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);

    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    char log[512];
    if (!success) {
        
        glGetProgramInfoLog(program, 512, NULL, log);
        printf("Shader link error:\\n%s\\n", log);
        exit(0);
        return 0;
    } else {
        glGetProgramInfoLog(program, 512, NULL, log);
        if (strlen(log) > 0) {
            printf("Shader link warning:\\n%s\\n", log);
        }
}

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    return program;
}

void init_dynamic_shader()
{

    for (int i = 0; i < UBERSHADER_VERSIONS; i++)
    {
        
        ShaderProgram * dynamic_shader = &DYNAMIC_SHADER[i];
        sprintf(dynamic_shader->vert_path, "../src/render/dynamic.vert");
        sprintf(dynamic_shader->geom_path, "../src/render/dynamic.geom");
        sprintf(dynamic_shader->frag_path, "../src/render/dynamic.frag");


        dynamic_shader->program = create_ubershader_program(dynamic_shader->vert_path, dynamic_shader->geom_path, dynamic_shader->frag_path, i);
    }


    sprintf(DYNAMIC_SHADER[DVIEW_DEFAULT].name, "default");
    sprintf(DYNAMIC_SHADER[DVIEW_ALBEDO].name, "albedo");
    sprintf(DYNAMIC_SHADER[DVIEW_ARM].name, "arm");
    sprintf(DYNAMIC_SHADER[DVIEW_EMISSIVE].name, "em");
    sprintf(DYNAMIC_SHADER[DVIEW_AO].name, "ao");
    sprintf(DYNAMIC_SHADER[DVIEW_UV].name, "uv");
    sprintf(DYNAMIC_SHADER[DVIEW_POM_VDIR].name, "pom_vdir");
    sprintf(DYNAMIC_SHADER[DVIEW_CLUSTERS].name, "clusters");
    sprintf(DYNAMIC_SHADER[DVIEW_NORMAL].name, "normal");
    sprintf(DYNAMIC_SHADER[DVIEW_ORIGINAL_NORMAL].name, "original_normal");
    sprintf(DYNAMIC_SHADER[DVIEW_DIRECT].name, "direct");
    sprintf(DYNAMIC_SHADER[DVIEW_INDIRECT].name, "indirect");
    sprintf(DYNAMIC_SHADER[DVIEW_LM_IRRADIANCE].name, "lm_irradiance");
    sprintf(DYNAMIC_SHADER[DVIEW_LM_DIRECTIONAL].name, "lm_directional");
    sprintf(DYNAMIC_SHADER[DVIEW_RPROBE].name, "rprobe");
    sprintf(DYNAMIC_SHADER[DVIEW_SPECULAR].name, "specular");
    sprintf(DYNAMIC_SHADER[DVIEW_MAX].name, "POM");


    
    for (int i = 0; i < UBERSHADER_VERSIONS; i++)
    {

        ShaderProgram * dynamic_shader = &DYNAMIC_SHADER[i];
        dynamic_shader->uniforms[0].location = glGetUniformLocation(dynamic_shader->program, "uLightmap");
        sprintf(dynamic_shader->uniforms[0].name, "uLightmap");
        dynamic_shader->uniforms[1].location = glGetUniformLocation(dynamic_shader->program, "use_probe");
        sprintf(dynamic_shader->uniforms[1].name, "use_probe");
        dynamic_shader->uniforms[2].location = glGetUniformLocation(dynamic_shader->program, "index_offset");
        sprintf(dynamic_shader->uniforms[2].name, "lm_offset");
        dynamic_shader->uniforms[3].location = glGetUniformLocation(dynamic_shader->program, "material");
        sprintf(dynamic_shader->uniforms[3].name, "material");
        dynamic_shader->uniforms[4].location = glGetUniformLocation(dynamic_shader->program, "uShadowmap");
        sprintf(dynamic_shader->uniforms[4].name, "uShadowmap");
        dynamic_shader->uniforms[5].location = glGetUniformLocation(dynamic_shader->program, "probe_tex");
        sprintf(dynamic_shader->uniforms[5].name, "probe_tex");
        dynamic_shader->uniforms[6].location = glGetUniformLocation(dynamic_shader->program, "uSkybox");
        sprintf(dynamic_shader->uniforms[6].name, "uSkybox");
        dynamic_shader->uniforms[7].location = glGetUniformLocation(dynamic_shader->program, "uBRDFLUT");
        sprintf(dynamic_shader->uniforms[7].name, "uBRDFLUT");
        dynamic_shader->uniforms[8].location = glGetUniformLocation(dynamic_shader->program, "uTime");
        sprintf(dynamic_shader->uniforms[8].name, "uTime");
        dynamic_shader->uniforms[9].location = glGetUniformLocation(dynamic_shader->program, "uRProbe");
        sprintf(dynamic_shader->uniforms[9].name, "uRProbe");
        dynamic_shader->uniforms[10].location = glGetUniformLocation(dynamic_shader->program, "uFBM");
        sprintf(dynamic_shader->uniforms[10].name, "uFBM");
        dynamic_shader->uniforms[11].location = glGetUniformLocation(dynamic_shader->program, "probe_bake");
        sprintf(dynamic_shader->uniforms[11].name, "probe_bake");
        dynamic_shader->uniforms[12].location = glGetUniformLocation(dynamic_shader->program, "index_offset");
        sprintf(dynamic_shader->uniforms[12].name, "index_count");
        dynamic_shader->uniforms[13].location = glGetUniformLocation(dynamic_shader->program, "index_offset");
        sprintf(dynamic_shader->uniforms[13].name, "start_index");
        dynamic_shader->uniforms[14].location = glGetUniformLocation(dynamic_shader->program, "uDirectionalmap");
        sprintf(dynamic_shader->uniforms[14].name, "uDirectionalmap");
        dynamic_shader->uniforms[15].location = glGetUniformLocation(dynamic_shader->program, "probe_spacing");
        sprintf(dynamic_shader->uniforms[15].name, "probe_spacing");
        dynamic_shader->uniforms[16].location = glGetUniformLocation(dynamic_shader->program, "probe_origin");
        sprintf(dynamic_shader->uniforms[16].name, "probe_origin");
        dynamic_shader->uniforms[17].location = glGetUniformLocation(dynamic_shader->program, "probe_grid_size");
        sprintf(dynamic_shader->uniforms[17].name, "probe_grid_size");
        dynamic_shader->uniforms[18].location = glGetUniformLocation(dynamic_shader->program, "uDiffuse");
        sprintf(dynamic_shader->uniforms[18].name, "gAlbedo");
        dynamic_shader->uniforms[19].location = glGetUniformLocation(dynamic_shader->program, "uNormal");
        sprintf(dynamic_shader->uniforms[19].name, "uNormal");
        dynamic_shader->uniforms[20].location = glGetUniformLocation(dynamic_shader->program, "uARM");
        sprintf(dynamic_shader->uniforms[20].name, "uARM");
        dynamic_shader->uniforms[21].location = glGetUniformLocation(dynamic_shader->program, "uEmissive");
        sprintf(dynamic_shader->uniforms[21].name, "uEmissive");
        dynamic_shader->uniforms[22].location = glGetUniformLocation(dynamic_shader->program, "uSSAO");
        sprintf(dynamic_shader->uniforms[22].name, "uSSAO");
        dynamic_shader->uniforms[23].location = glGetUniformLocation(dynamic_shader->program, "uLPVDepthPadded");
        sprintf(dynamic_shader->uniforms[23].name, "uLPVDepthPadded");
    }

}

void init_shadow_map_shader()
{
    sprintf(SHADOW_MAP_SHADER.vert_path, "../src/render/shadow.vert");
    sprintf(SHADOW_MAP_SHADER.frag_path, "../src/render/shadow.frag");

    SHADOW_MAP_SHADER.program = create_shader_program(SHADOW_MAP_SHADER.vert_path, SHADOW_MAP_SHADER.frag_path);


    sprintf(SHADOW_MAP_SHADER.uniforms[0].name, "projection");
    SHADOW_MAP_SHADER.uniforms[0].location = glGetUniformLocation(SHADOW_MAP_SHADER.program, "projection");
    sprintf(SHADOW_MAP_SHADER.uniforms[1].name, "view");
    SHADOW_MAP_SHADER.uniforms[1].location = glGetUniformLocation(SHADOW_MAP_SHADER.program, "view");

    
    sprintf(POINT_LIGHT_SHADOW_SHADER.vert_path, "../src/render/point_light_shadows.vert");
    sprintf(POINT_LIGHT_SHADOW_SHADER.geom_path, "../src/render/point_light_shadows.geom");
    sprintf(POINT_LIGHT_SHADOW_SHADER.frag_path, "../src/render/point_light_shadows.frag");
    // POINT_LIGHT_SHADOW_SHADER.program = create_shader_program(POINT_LIGHT_SHADOW_SHADER.vert_path, POINT_LIGHT_SHADOW_SHADER.frag_path);
    POINT_LIGHT_SHADOW_SHADER.program = create_geometry_shader_program(POINT_LIGHT_SHADOW_SHADER.vert_path, POINT_LIGHT_SHADOW_SHADER.geom_path, POINT_LIGHT_SHADOW_SHADER.frag_path);

    sprintf(POINT_LIGHT_SHADOW_SHADER.uniforms[0].name, "vp");
    POINT_LIGHT_SHADOW_SHADER.uniforms[0].location = glGetUniformLocation(POINT_LIGHT_SHADOW_SHADER.program, "vp");
    sprintf(POINT_LIGHT_SHADOW_SHADER.uniforms[1].name, "uLightPos");
    POINT_LIGHT_SHADOW_SHADER.uniforms[1].location = glGetUniformLocation(POINT_LIGHT_SHADOW_SHADER.program, "uLightPos");
    sprintf(POINT_LIGHT_SHADOW_SHADER.uniforms[2].name, "uFar");
    POINT_LIGHT_SHADOW_SHADER.uniforms[2].location = glGetUniformLocation(POINT_LIGHT_SHADOW_SHADER.program, "uFar");
    sprintf(POINT_LIGHT_SHADOW_SHADER.uniforms[3].name, "uLayerOffset");
    POINT_LIGHT_SHADOW_SHADER.uniforms[3].location = glGetUniformLocation(POINT_LIGHT_SHADOW_SHADER.program, "uLayerOffset");
}

void init_wireframe_shader()
{
    sprintf(WIREFRAME_SHADER.vert_path, "../src/render/wireframe.vert");
    sprintf(WIREFRAME_SHADER.frag_path, "../src/render/wireframe.frag");

    WIREFRAME_SHADER.program = create_shader_program(WIREFRAME_SHADER.vert_path, WIREFRAME_SHADER.frag_path);

}


void init_postprocess_shader()
{


    uint32_t perm_count = 1 << PP_MAX;
    uint32_t * permutations = calloc(perm_count, sizeof(uint32_t));
    for (int i = 0; i < perm_count; i++)
    {
        int count = 0;
        for (int j = perm_count-1; j >= 0; j--)
        {
            permutations[i] |= ((i >> j) & 1) << j;
            count++;
        }
    }

    PP_SHADER_VERSIONS = perm_count;

    for (int i = 0; i < perm_count; i++)
    {
        
        sprintf(POSTPROCESS_SHADER[i].vert_path, "../src/render/postprocess.vert");
        sprintf(POSTPROCESS_SHADER[i].frag_path, "../src/render/postprocess.frag");
        POSTPROCESS_SHADER[i].program = create_postprocess_program(POSTPROCESS_SHADER[i].vert_path, POSTPROCESS_SHADER[i].frag_path, permutations[i], false, 0);
        POSTPROCESS_SHADER[i].perm = permutations[i];
    }
    for (int i = perm_count; i < perm_count + PPDVIEW_MAX - 1; i++)
    {
        
        sprintf(POSTPROCESS_SHADER[i].vert_path, "../src/render/postprocess.vert");
        sprintf(POSTPROCESS_SHADER[i].frag_path, "../src/render/postprocess.frag");
        POSTPROCESS_SHADER[i].program = create_postprocess_program(POSTPROCESS_SHADER[i].vert_path, POSTPROCESS_SHADER[i].frag_path, 0, true, i - perm_count + 1);
    }
    
    for (int i = 0; i < perm_count + PPDVIEW_MAX - 1; i++)
    {
        sprintf(POSTPROCESS_SHADER[i].uniforms[0].name, "uScene");
        POSTPROCESS_SHADER[i].uniforms[0].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uScene");
        sprintf(POSTPROCESS_SHADER[i].uniforms[1].name, "uHistory");
        POSTPROCESS_SHADER[i].uniforms[1].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uHistory");
        sprintf(POSTPROCESS_SHADER[i].uniforms[2].name, "uTime");
        POSTPROCESS_SHADER[i].uniforms[2].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uTime");
        sprintf(POSTPROCESS_SHADER[i].uniforms[3].name, "uMBLUR");
        POSTPROCESS_SHADER[i].uniforms[3].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uMBLUR");
        sprintf(POSTPROCESS_SHADER[i].uniforms[4].name, "uTAA");
        POSTPROCESS_SHADER[i].uniforms[4].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uTAA");
        sprintf(POSTPROCESS_SHADER[i].uniforms[5].name, "uFBM");
        POSTPROCESS_SHADER[i].uniforms[5].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uFBM");
        sprintf(POSTPROCESS_SHADER[i].uniforms[6].name, "uDPP");
        POSTPROCESS_SHADER[i].uniforms[6].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uDPP");
        sprintf(POSTPROCESS_SHADER[i].uniforms[7].name, "uText");
        POSTPROCESS_SHADER[i].uniforms[7].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uText");
        sprintf(POSTPROCESS_SHADER[i].uniforms[8].name, "uSSR");
        POSTPROCESS_SHADER[i].uniforms[8].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uSSR");
        sprintf(POSTPROCESS_SHADER[i].uniforms[9].name, "uCurrentSSR");
        POSTPROCESS_SHADER[i].uniforms[9].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uCurrentSSR");
        sprintf(POSTPROCESS_SHADER[i].uniforms[10].name, "gVel");
        POSTPROCESS_SHADER[i].uniforms[10].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "gVel");
        sprintf(POSTPROCESS_SHADER[i].uniforms[11].name, "uBokeh");
        POSTPROCESS_SHADER[i].uniforms[11].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uBokeh");
        sprintf(POSTPROCESS_SHADER[i].uniforms[12].name, "uCOC");
        POSTPROCESS_SHADER[i].uniforms[12].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uCOC");
        sprintf(POSTPROCESS_SHADER[i].uniforms[13].name, "uDOF");
        POSTPROCESS_SHADER[i].uniforms[13].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uDOF");
        sprintf(POSTPROCESS_SHADER[i].uniforms[14].name, "uBloom");
        POSTPROCESS_SHADER[i].uniforms[14].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uBloom");
        sprintf(POSTPROCESS_SHADER[i].uniforms[15].name, "uLUT");
        POSTPROCESS_SHADER[i].uniforms[15].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uLUT");
        sprintf(POSTPROCESS_SHADER[i].uniforms[16].name, "uFocalLength");
        POSTPROCESS_SHADER[i].uniforms[16].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uFocalLength");
        sprintf(POSTPROCESS_SHADER[i].uniforms[17].name, "uFStop");
        POSTPROCESS_SHADER[i].uniforms[17].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uFStop");
        sprintf(POSTPROCESS_SHADER[i].uniforms[18].name, "uSensorHeight");
        POSTPROCESS_SHADER[i].uniforms[18].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uSensorHeight");
        sprintf(POSTPROCESS_SHADER[i].uniforms[19].name, "uDOFMaxBlurRadius");
        POSTPROCESS_SHADER[i].uniforms[19].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uDOFMaxBlurRadius");
        sprintf(POSTPROCESS_SHADER[i].uniforms[20].name, "uLM");
        POSTPROCESS_SHADER[i].uniforms[20].location = glGetUniformLocation(POSTPROCESS_SHADER[i].program, "uLM");
        
    }

    


    free(permutations);

}
void init_brdf_shader()
{

    sprintf(BRDF_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BRDF_SHADER.frag_path, "../src/utilities/brdf.frag");

    BRDF_SHADER.program = create_shader_program(BRDF_SHADER.vert_path, BRDF_SHADER.frag_path);


    BRDF_SHADER.uniforms[0].location = glGetUniformLocation(BRDF_SHADER.program, "uScene");
    sprintf(BRDF_SHADER.uniforms[0].name, "uScene");

}
void init_sky_shader()
{

    sprintf(SKY_SHADER.vert_path, "../src/render/sky.vert");
    sprintf(SKY_SHADER.frag_path, "../src/render/sky.frag");

    SKY_SHADER.program = create_shader_program(SKY_SHADER.vert_path, SKY_SHADER.frag_path);
    SKY_SHADER.uniforms[0].location = glGetUniformLocation(SKY_SHADER.program, "uCubemap");
    sprintf(SKY_SHADER.uniforms[0].name, "uCubemap");

}
void init_lightmap_shader()
{

    sprintf(LIGHTMAP_SHADER.vert_path, "../src/light/lightmap.vert");
    sprintf(LIGHTMAP_SHADER.frag_path, "../src/light/lightmap.frag");

    LIGHTMAP_SHADER.program = create_shader_program(LIGHTMAP_SHADER.vert_path, LIGHTMAP_SHADER.frag_path);

}
void init_text_shader()
{

    sprintf(TEXT_SHADER.vert_path, "../src/ui/text.vert");
    sprintf(TEXT_SHADER.frag_path, "../src/ui/text.frag");

    TEXT_SHADER.program = create_shader_program(TEXT_SHADER.vert_path, TEXT_SHADER.frag_path);

    sprintf(TEXT_SHADER.uniforms[0].name, "uOrtho");
    TEXT_SHADER.uniforms[0].location = glGetUniformLocation(TEXT_SHADER.program, "uOrtho");
    sprintf(TEXT_SHADER.uniforms[1].name, "uFontAtlas");
    TEXT_SHADER.uniforms[1].location = glGetUniformLocation(TEXT_SHADER.program, "uFontAtlas");
    sprintf(TEXT_SHADER.uniforms[2].name, "uShadowPass");
    TEXT_SHADER.uniforms[2].location = glGetUniformLocation(TEXT_SHADER.program, "uShadowPass");

}
void init_ui_shader()
{

    sprintf(UI_SHADER.vert_path, "../src/ui/ui.vert");
    sprintf(UI_SHADER.frag_path, "../src/ui/ui.frag");

    UI_SHADER.program = create_shader_program(UI_SHADER.vert_path, UI_SHADER.frag_path);

    sprintf(UI_SHADER.uniforms[0].name, "uOrtho");
    UI_SHADER.uniforms[0].location = glGetUniformLocation(UI_SHADER.program, "uOrtho");
    sprintf(UI_SHADER.uniforms[1].name, "uShadowPass");
    UI_SHADER.uniforms[1].location = glGetUniformLocation(UI_SHADER.program, "uShadowPass");

}
void init_SSR_shader()
{

    // sprintf(SSR_SHADER.vert_path, "./postprocess.vert");
    // sprintf(SSR_SHADER.frag_path, "./ssr.frag");

    // SSR_SHADER.program = create_shader_program(SSR_SHADER.vert_path, SSR_SHADER.frag_path);
    // sprintf(SSR_SHADER.uniforms[0].name, "gNormal");
    // SSR_SHADER.uniforms[0].location = glGetUniformLocation(SSR_SHADER.program, "gNormal");
    // sprintf(SSR_SHADER.uniforms[1].name, "gDepth");
    // SSR_SHADER.uniforms[1].location = glGetUniformLocation(SSR_SHADER.program, "gDepth");
    // sprintf(SSR_SHADER.uniforms[2].name, "gAlbedo");
    // SSR_SHADER.uniforms[2].location = glGetUniformLocation(SSR_SHADER.program, "gAlbedo");
    // sprintf(SSR_SHADER.uniforms[3].name, "gExtra");
    // SSR_SHADER.uniforms[3].location = glGetUniformLocation(SSR_SHADER.program, "gExtra");
    // sprintf(SSR_SHADER.uniforms[4].name, "uScene");
    // SSR_SHADER.uniforms[4].location = glGetUniformLocation(SSR_SHADER.program, "uScene");
    // sprintf(SSR_SHADER.uniforms[5].name, "uTime");
    // SSR_SHADER.uniforms[5].location = glGetUniformLocation(SSR_SHADER.program, "uTime");

    // sprintf(SSR_BLUR_SHADER.vert_path, "./postprocess.vert");
    // sprintf(SSR_BLUR_SHADER.frag_path, "./ssr_blur.frag");

    // SSR_BLUR_SHADER.program = create_shader_program(SSR_BLUR_SHADER.vert_path, SSR_BLUR_SHADER.frag_path);

    // sprintf(SSR_BLUR_SHADER.uniforms[0].name, "uSSRInput");
    // SSR_BLUR_SHADER.uniforms[0].location = glGetUniformLocation(SSR_BLUR_SHADER.program, "uSSRInput");
    // sprintf(SSR_BLUR_SHADER.uniforms[1].name, "gDepth");
    // SSR_BLUR_SHADER.uniforms[1].location = glGetUniformLocation(SSR_BLUR_SHADER.program, "gDepth");
    // sprintf(SSR_BLUR_SHADER.uniforms[2].name, "gNormal");
    // SSR_BLUR_SHADER.uniforms[2].location = glGetUniformLocation(SSR_BLUR_SHADER.program, "gNormal");
    // sprintf(SSR_BLUR_SHADER.uniforms[3].name, "gExtra");
    // SSR_BLUR_SHADER.uniforms[3].location = glGetUniformLocation(SSR_BLUR_SHADER.program, "gExtra");

    // sprintf(SSR_HISTORY_SHADER.vert_path, "./postprocess.vert");
    // sprintf(SSR_HISTORY_SHADER.frag_path, "./ssr_history.frag");
    // SSR_HISTORY_SHADER.program = create_shader_program(SSR_HISTORY_SHADER.vert_path, SSR_HISTORY_SHADER.frag_path);

    // sprintf(SSR_HISTORY_SHADER.uniforms[0].name, "uSSR");
    // SSR_HISTORY_SHADER.uniforms[0].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "uSSR");
    // sprintf(SSR_HISTORY_SHADER.uniforms[1].name, "uScene");
    // SSR_HISTORY_SHADER.uniforms[1].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "uScene");
    // sprintf(SSR_HISTORY_SHADER.uniforms[2].name, "uHistorySSR");
    // SSR_HISTORY_SHADER.uniforms[2].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "uHistorySSR");
    // sprintf(SSR_HISTORY_SHADER.uniforms[3].name, "gVel");
    // SSR_HISTORY_SHADER.uniforms[3].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "gVel");

    // sprintf(SSR_HISTORY_SHADER.uniforms[4].name, "gDepth");
    // SSR_HISTORY_SHADER.uniforms[4].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "gDepth");
    // sprintf(SSR_HISTORY_SHADER.uniforms[5].name, "uJitterOffset");
    // SSR_HISTORY_SHADER.uniforms[5].location = glGetUniformLocation(SSR_HISTORY_SHADER.program, "uJitterOffset");

}
void init_lightmap_bounce_compute_shader()
{


    LIGHTMAP_BOUNCE_SHADER = create_compute_shader("../src/light/lightmap_bounce.comp");
    LIGHTMAP_DENOISE_SHADER = create_compute_shader("../src/light/lightmap_denoise.comp");
    LIGHTMAP_DILATE_SHADER = create_compute_shader("../src/light/lightmap_dilate.comp");
    PROBE_BAKE_SHADER = create_compute_shader("../src/light/probe_bake.comp");
    PROBE_UNPACK_SHADER = create_compute_shader("../src/light/probe_unpack.comp");
    DIRECTIONAL_MAP_NORMALIZE_SHADER = create_compute_shader("../src/light/directional_map_normalize.comp");

}
void init_cull_clusers_compute_shader()
{
    
    INIT_CLUSTER_AABBS_SHADER = create_compute_shader("../src/render/init_cluster_aabbs.comp");


    CULL_CLUSTERS_SHADER.program = create_compute_shader("../src/render/cull_clusters.comp");
    sprintf(CULL_CLUSTERS_SHADER.uniforms[0].name, "uGridSize");
    CULL_CLUSTERS_SHADER.uniforms[0].location = glGetUniformLocation(CULL_CLUSTERS_SHADER.program, "uGridSize");
    sprintf(CULL_CLUSTERS_SHADER.uniforms[1].name, "uNear");
    CULL_CLUSTERS_SHADER.uniforms[1].location = glGetUniformLocation(CULL_CLUSTERS_SHADER.program, "uNear");
    sprintf(CULL_CLUSTERS_SHADER.uniforms[2].name, "uFar");
    CULL_CLUSTERS_SHADER.uniforms[2].location = glGetUniformLocation(CULL_CLUSTERS_SHADER.program, "uFar");
    sprintf(CULL_CLUSTERS_SHADER.uniforms[3].name, "uMaxLightsPerCluster");
    CULL_CLUSTERS_SHADER.uniforms[3].location = glGetUniformLocation(CULL_CLUSTERS_SHADER.program, "uMaxLightsPerCluster");
    sprintf(CULL_CLUSTERS_SHADER.uniforms[4].name, "uLightCount");
    CULL_CLUSTERS_SHADER.uniforms[4].location = glGetUniformLocation(CULL_CLUSTERS_SHADER.program, "uLightCount");

}
void init_cull_draw_calls_compute_shader()
{
    
    sprintf(CULL_DRAW_CALLS_SHADER.vert_path, "../src/render/cull_draw_calls.comp");
    CULL_DRAW_CALLS_SHADER.program = create_compute_shader("../src/render/cull_draw_calls.comp");

    sprintf(CULL_DRAW_CALLS_SHADER.uniforms[0].name, "uInstanceCount");
    CULL_DRAW_CALLS_SHADER.uniforms[0].location = glGetUniformLocation(CULL_DRAW_CALLS_SHADER.program, "uInstanceCount");
    sprintf(CULL_DRAW_CALLS_SHADER.uniforms[1].name, "planes");
    CULL_DRAW_CALLS_SHADER.uniforms[1].location = glGetUniformLocation(CULL_DRAW_CALLS_SHADER.program, "planes");
    sprintf(CULL_DRAW_CALLS_SHADER.uniforms[2].name, "uArrayOffset");
    CULL_DRAW_CALLS_SHADER.uniforms[2].location = glGetUniformLocation(CULL_DRAW_CALLS_SHADER.program, "uArrayOffset");

    
    sprintf(INDIRECT_PREFIX_SHADER.vert_path, "../src/render/indirect_prefix.comp");
    INDIRECT_PREFIX_SHADER.program = create_compute_shader("../src/render/indirect_prefix.comp");

    sprintf(INDIRECT_PREFIX_SHADER.uniforms[0].name, "n");
    INDIRECT_PREFIX_SHADER.uniforms[0].location = glGetUniformLocation(INDIRECT_PREFIX_SHADER.program, "n");
    
    sprintf(ELEMENTS_PREFIX_SHADER.vert_path, "../src/render/elements_prefix.comp");
    ELEMENTS_PREFIX_SHADER.program = create_compute_shader("../src/render/elements_prefix.comp");
    sprintf(ELEMENTS_PREFIX_SHADER.uniforms[0].name, "n");
    ELEMENTS_PREFIX_SHADER.uniforms[0].location = glGetUniformLocation(ELEMENTS_PREFIX_SHADER.program, "n");


    

    sprintf(CULL_POINT_LIGHT_SHADOWS_SHADER.vert_path, "../src/render/cull_point_light_shadows.comp");
    CULL_POINT_LIGHT_SHADOWS_SHADER.program = create_compute_shader("../src/render/cull_point_light_shadows.comp");

    sprintf(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[0].name, "uLightPos");
    CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[0].location = glGetUniformLocation(CULL_POINT_LIGHT_SHADOWS_SHADER.program, "uLightPos");
    sprintf(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[1].name, "uLightRadius");
    CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[1].location = glGetUniformLocation(CULL_POINT_LIGHT_SHADOWS_SHADER.program, "uLightRadius");
    sprintf(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[2].name, "uInstanceCount");
    CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[2].location = glGetUniformLocation(CULL_POINT_LIGHT_SHADOWS_SHADER.program, "uInstanceCount");
    
    sprintf(CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[3].name, "uArrayOffset");
    CULL_POINT_LIGHT_SHADOWS_SHADER.uniforms[3].location = glGetUniformLocation(CULL_POINT_LIGHT_SHADOWS_SHADER.program, "uArrayOffset");
    
    

}
// void init_rprobe_shader()
// {
//     sprintf(GGX_RPROBE_SHADER.vert_path, "./ggx_rprobe.vert");
//     sprintf(GGX_RPROBE_SHADER.frag_path, "./ggx_rprobe.frag");

//     GGX_RPROBE_SHADER.program = create_shader_program(GGX_RPROBE_SHADER.vert_path, GGX_RPROBE_SHADER.frag_path);
// }

void init_audio_shader()
{
    AUDIO_PROBE_BAKE_SHADER = create_compute_shader("../src/audio/audio_probe_bake.comp");
    
}

void init_depth_pre_pass_shader()
{

    sprintf(DEPTH_PRE_PASS_SHADER[0].name, "default");
    // sprintf(DEPTH_PRE_PASS_SHADER[1].name, "POM");

    for (int i = 0; i < GBUFFER_VERSIONS; i++)
    {
        sprintf(DEPTH_PRE_PASS_SHADER[i].vert_path, "../src/render/depth_pre_pass.vert");
        sprintf(DEPTH_PRE_PASS_SHADER[i].frag_path, "../src/render/depth_pre_pass.frag");
        DEPTH_PRE_PASS_SHADER[i].program = create_gbuffer_program(DEPTH_PRE_PASS_SHADER[i].vert_path, DEPTH_PRE_PASS_SHADER[i].frag_path, i);
    }
    sprintf(DEPTH_PRE_PASS_SHADER_MSAA[0].name, "default");
    // sprintf(DEPTH_PRE_PASS_SHADER_MSAA[1].name, "POM");

    for (int i = 0; i < GBUFFER_VERSIONS; i++)
    {
        sprintf(DEPTH_PRE_PASS_SHADER_MSAA[i].vert_path, "../src/render/depth_pre_pass_no_gbuffer.vert");
        sprintf(DEPTH_PRE_PASS_SHADER_MSAA[i].frag_path, "../src/render/depth_pre_pass_no_gbuffer.frag");
        DEPTH_PRE_PASS_SHADER_MSAA[i].program = create_gbuffer_program(DEPTH_PRE_PASS_SHADER_MSAA[i].vert_path, DEPTH_PRE_PASS_SHADER_MSAA[i].frag_path, i);
    }
        

    sprintf(DEPTH_DOWNSAMPLE_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(DEPTH_DOWNSAMPLE_SHADER.frag_path, "../src/render/depth_downsample.frag");


    DEPTH_DOWNSAMPLE_SHADER.program = create_shader_program(DEPTH_DOWNSAMPLE_SHADER.vert_path, DEPTH_DOWNSAMPLE_SHADER.frag_path);
    sprintf(DEPTH_DOWNSAMPLE_SHADER.uniforms[0].name, "uDepth");
    DEPTH_DOWNSAMPLE_SHADER.uniforms[0].location = glGetUniformLocation(DEPTH_DOWNSAMPLE_SHADER.program, "uDepth");
}
void init_ssao_shader()
{

    sprintf(SSAO_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(SSAO_SHADER.frag_path, "../src/render/ssao.frag");

    SSAO_SHADER.program = create_shader_program(SSAO_SHADER.vert_path, SSAO_SHADER.frag_path);

    sprintf(SSAO_SHADER.uniforms[0].name, "uDepth");
    SSAO_SHADER.uniforms[0].location = glGetUniformLocation(SSAO_SHADER.program, "uDepth");
    sprintf(SSAO_SHADER.uniforms[1].name, "gNormal");
    SSAO_SHADER.uniforms[1].location = glGetUniformLocation(SSAO_SHADER.program, "gNormal");
    sprintf(SSAO_SHADER.uniforms[2].name, "uNoise");
    SSAO_SHADER.uniforms[2].location = glGetUniformLocation(SSAO_SHADER.program, "uNoise");
    sprintf(SSAO_SHADER.uniforms[3].name, "uSSAOResolution");
    SSAO_SHADER.uniforms[3].location = glGetUniformLocation(SSAO_SHADER.program, "uSSAOResolution");


    sprintf(SSAO_BLUR_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(SSAO_BLUR_SHADER.frag_path, "../src/render/ssao_blur.frag");

    SSAO_BLUR_SHADER.program = create_shader_program(SSAO_BLUR_SHADER.vert_path, SSAO_BLUR_SHADER.frag_path);
    sprintf(SSAO_BLUR_SHADER.uniforms[0].name, "uSSAOInput");
    SSAO_BLUR_SHADER.uniforms[0].location = glGetUniformLocation(SSAO_BLUR_SHADER.program, "uSSAOInput");
    sprintf(SSAO_BLUR_SHADER.uniforms[1].name, "uDepth");
    SSAO_BLUR_SHADER.uniforms[1].location = glGetUniformLocation(SSAO_BLUR_SHADER.program, "uDepth");

}

void init_dof_shader()
{

    sprintf(COC_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(COC_SHADER.frag_path, "../src/render/coc.frag");

    COC_SHADER.program = create_shader_program(COC_SHADER.vert_path, COC_SHADER.frag_path);


    sprintf(COC_SHADER.uniforms[0].name, "gDepth");
    COC_SHADER.uniforms[0].location = glGetUniformLocation(COC_SHADER.program, "gDepth");

    sprintf(COC_SHADER.uniforms[1].name, "uScene");
    COC_SHADER.uniforms[1].location = glGetUniformLocation(COC_SHADER.program, "uScene");

    sprintf(COC_SHADER.uniforms[2].name, "uFocalLength");
    COC_SHADER.uniforms[2].location = glGetUniformLocation(COC_SHADER.program, "uFocalLength");

    sprintf(COC_SHADER.uniforms[3].name, "uFStop");
    COC_SHADER.uniforms[3].location = glGetUniformLocation(COC_SHADER.program, "uFStop");

    sprintf(COC_SHADER.uniforms[4].name, "uSensorHeight");
    COC_SHADER.uniforms[4].location = glGetUniformLocation(COC_SHADER.program, "uSensorHeight");

    sprintf(COC_SHADER.uniforms[5].name, "uMaxBlurRadius");
    COC_SHADER.uniforms[5].location = glGetUniformLocation(COC_SHADER.program, "uMaxBlurRadius");

    sprintf(BOKEH_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BOKEH_SHADER.frag_path, "../src/render/bokeh.frag");

    BOKEH_SHADER.program = create_shader_program(BOKEH_SHADER.vert_path, BOKEH_SHADER.frag_path);


    sprintf(BOKEH_SHADER.uniforms[0].name, "uCOC");
    BOKEH_SHADER.uniforms[0].location = glGetUniformLocation(BOKEH_SHADER.program, "uCOC");

    sprintf(BOKEH_SHADER.uniforms[1].name, "uTexelSize");
    BOKEH_SHADER.uniforms[1].location = glGetUniformLocation(BOKEH_SHADER.program, "uTexelSize");





    sprintf(FOCAL_DISTANCE_SHADER.vert_path, "../src/render/focal_distance.comp");
    FOCAL_DISTANCE_SHADER.program = create_compute_shader("../src/render/focal_distance.comp");

    sprintf(FOCAL_DISTANCE_SHADER.uniforms[0].name, "gDepth");
    FOCAL_DISTANCE_SHADER.uniforms[0].location = glGetUniformLocation(FOCAL_DISTANCE_SHADER.program, "gDepth");
    sprintf(FOCAL_DISTANCE_SHADER.uniforms[1].name, "uTexelSize");
    FOCAL_DISTANCE_SHADER.uniforms[1].location = glGetUniformLocation(FOCAL_DISTANCE_SHADER.program, "uTexelSize");
    sprintf(FOCAL_DISTANCE_SHADER.uniforms[2].name, "uResolution");
    FOCAL_DISTANCE_SHADER.uniforms[2].location = glGetUniformLocation(FOCAL_DISTANCE_SHADER.program, "uResolution");
    sprintf(FOCAL_DISTANCE_SHADER.uniforms[3].name, "uDT");
    FOCAL_DISTANCE_SHADER.uniforms[3].location = glGetUniformLocation(FOCAL_DISTANCE_SHADER.program, "uDT");


    
}

void init_bloom_shader()
{

    sprintf(BLOOM_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BLOOM_SHADER.frag_path, "../src/render/bloom.frag");

    BLOOM_SHADER.program = create_shader_program(BLOOM_SHADER.vert_path, BLOOM_SHADER.frag_path);


    sprintf(BLOOM_SHADER.uniforms[0].name, "uScene");
    BLOOM_SHADER.uniforms[0].location = glGetUniformLocation(BLOOM_SHADER.program, "uScene");

    sprintf(BLOOM_BLUR_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BLOOM_BLUR_SHADER.frag_path, "../src/render/bloom_blur.frag");

    BLOOM_BLUR_SHADER.program = create_shader_program(BLOOM_BLUR_SHADER.vert_path, BLOOM_BLUR_SHADER.frag_path);

    sprintf(BLOOM_BLUR_SHADER.uniforms[0].name, "uBloom");
    BLOOM_BLUR_SHADER.uniforms[0].location = glGetUniformLocation(BLOOM_BLUR_SHADER.program, "uBloom");
    sprintf(BLOOM_BLUR_SHADER.uniforms[1].name, "uDirection");
    BLOOM_BLUR_SHADER.uniforms[1].location = glGetUniformLocation(BLOOM_BLUR_SHADER.program, "uDirection");
    sprintf(BLOOM_BLUR_SHADER.uniforms[2].name, "uTexel");
    BLOOM_BLUR_SHADER.uniforms[2].location = glGetUniformLocation(BLOOM_BLUR_SHADER.program, "uTexel");

    sprintf(BLOOM_DOWNSCALE_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BLOOM_DOWNSCALE_SHADER.frag_path, "../src/render/bloom_downscale.frag");

    BLOOM_DOWNSCALE_SHADER.program = create_shader_program(BLOOM_DOWNSCALE_SHADER.vert_path, BLOOM_DOWNSCALE_SHADER.frag_path);
    sprintf(BLOOM_DOWNSCALE_SHADER.uniforms[0].name, "srcTexture");
    BLOOM_DOWNSCALE_SHADER.uniforms[0].location = glGetUniformLocation(BLOOM_DOWNSCALE_SHADER.program, "srcTexture");


    sprintf(BLOOM_UPSCALE_SHADER.vert_path, "../src/render/postprocess.vert");
    sprintf(BLOOM_UPSCALE_SHADER.frag_path, "../src/render/bloom_upscale.frag");

    BLOOM_UPSCALE_SHADER.program = create_shader_program(BLOOM_UPSCALE_SHADER.vert_path, BLOOM_UPSCALE_SHADER.frag_path);

    sprintf(BLOOM_DOWNSCALE_SHADER.uniforms[0].name, "srcTexture");
    BLOOM_DOWNSCALE_SHADER.uniforms[0].location = glGetUniformLocation(BLOOM_DOWNSCALE_SHADER.program, "srcTexture");
    sprintf(BLOOM_UPSCALE_SHADER.uniforms[1].name, "uFilterRadius");
    BLOOM_UPSCALE_SHADER.uniforms[1].location = glGetUniformLocation(BLOOM_UPSCALE_SHADER.program, "uFilterRadius");

}


void SHADERS_INIT()
{

    glDeleteShader(LIGHTMAP_BOUNCE_SHADER);
    glDeleteShader(LIGHTMAP_DENOISE_SHADER);
    glDeleteShader(LIGHTMAP_DILATE_SHADER);
    glDeleteShader(PROBE_BAKE_SHADER);
    glDeleteShader(PROBE_UNPACK_SHADER);
    glDeleteShader(DIRECTIONAL_MAP_NORMALIZE_SHADER);
    for (int i = 0; i < UBERSHADER_VERSIONS; i++)
    {
        glDeleteShader(DYNAMIC_SHADER[i].program);
    }
    glDeleteShader(SHADOW_MAP_SHADER.program);
    glDeleteShader(WIREFRAME_SHADER.program);
    for (int i = 0; i < PP_SHADER_VERSIONS; i++)
    {
        glDeleteShader(POSTPROCESS_SHADER[i].program);
    }
    glDeleteShader(SKY_SHADER.program);
    glDeleteShader(LIGHTMAP_SHADER.program);
    glDeleteShader(BRDF_SHADER.program);
    for (int i = 0; i < GBUFFER_VERSIONS; i++)
    {
        glDeleteShader(DEPTH_PRE_PASS_SHADER[i].program);
    }
    glDeleteShader(POINT_LIGHT_SHADOW_SHADER.program);
    glDeleteShader(TEXT_SHADER.program);
    glDeleteShader(UI_SHADER.program);
    glDeleteShader(SSR_SHADER.program);
    glDeleteShader(SSR_BLUR_SHADER.program);
    glDeleteShader(SSR_HISTORY_SHADER.program);
    glDeleteShader(SSAO_SHADER.program);
    glDeleteShader(SSAO_BLUR_SHADER.program);
    glDeleteShader(COC_SHADER.program);
    glDeleteShader(BOKEH_SHADER.program);
    glDeleteShader(FOCAL_DISTANCE_SHADER.program);
    glDeleteShader(CULL_POINT_LIGHT_SHADOWS_SHADER.program);
    glDeleteShader(INDIRECT_PREFIX_SHADER.program);
    glDeleteShader(ELEMENTS_PREFIX_SHADER.program);
    glDeleteShader(CULL_DRAW_CALLS_SHADER.program);
    glDeleteShader(CULL_CLUSTERS_SHADER.program);
    glDeleteShader(BLOOM_SHADER.program);
    glDeleteShader(BLOOM_BLUR_SHADER.program);
    glDeleteShader(BLOOM_DOWNSCALE_SHADER.program);
    glDeleteShader(BLOOM_UPSCALE_SHADER.program);
    glDeleteShader(AUDIO_PROBE_BAKE_SHADER);
    glDeleteShader(INIT_CLUSTER_AABBS_SHADER);

    
    init_dynamic_shader();
    init_shadow_map_shader();
    init_wireframe_shader();

    init_postprocess_shader();
    init_lightmap_shader();
    init_text_shader();
    init_ui_shader();

    init_lightmap_bounce_compute_shader();

    init_sky_shader();
    init_brdf_shader();
    // init_rprobe_shader();
    init_depth_pre_pass_shader();
    init_ssao_shader();
    init_cull_clusers_compute_shader();
    init_cull_draw_calls_compute_shader();
    init_SSR_shader();
    init_dof_shader();
    init_bloom_shader();
    init_audio_shader();

}
