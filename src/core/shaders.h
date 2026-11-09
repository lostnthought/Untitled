/**
 * @file shaders.h
 * @brief Initialization for ShaderProgram%s.
 */
#ifndef SHADERS_H
#define SHADERS_H
#include "core/engine_types.h"


/**
 * @brief Reads a file into raw bytes.
 * @param filename The file to read.
 * @returns The byte buffer.
 * @warning Must be freed.
 */
char * read_file(const char *filename);

/**
 * @brief Creates a compute shader.
 * @param filename The path to file.
 * @returns The handle for the shader.
 */
GLuint create_compute_shader(const char * filename);

/**
 * @brief Compiles a non-compute shader.
 * @param type The type of shader to compile.
 * @param filename The path to the shader.
 * @returns The handle for the shader.
 */
GLuint compile_shader(GLenum type, const char * filename);


/**
 * @brief Compiles and links vert and frag pairs.
 * @param vertex_path The path to the vert shader.
 * @param fragment_path The path to the frag shader.
 * @returns The handle to the linked shader.
 */
GLuint create_shader_program(const char * vertex_path, const char * fragment_path);


/**
 * @brief Deletes and then recompiles and links all engine shaders.
 */
void SHADERS_INIT();

#endif
