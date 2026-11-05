#ifndef SHADERS_H
#define SHADERS_H
#include "core/engine_types.h"



char *read_file(const char *filename);
GLuint create_compute_shader(const char *filename);
GLuint compile_shader(GLenum type, const char *filename);
GLuint create_shader_program(const char *vertexPath, const char *fragmentPath);
void init_shaders();

#endif
