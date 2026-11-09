/**
 * @file load.h
 * @brief Saves and loads various files that store engine data.
 */

#ifndef LOAD_H
#define LOAD_H

#include "core/engine_types.h"

/**
 * @brief Opens an image in bytes via stb.
 * @param image Pointer to Image.
 */
unsigned char * open_image(Image * image);

/**
 * @brief Loads a cubemap in OpenGL naming convention (must be HDR).
 * @param path Path to cubemap folder.
 * @returns Handle to the cubemap.
 */
GLuint load_cubemap (const char * path);

/**
 * @brief Loads a 3D LUT for postprocessing.
 * @param filePath The path to the LUT.
 * @returns Handle to the 3D texture.
 */
GLuint load_3d_lut(const char* filepath);

/**
 * @brief Loads a texture through OpenGL.
 * @param path The path and name of the texture.
 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
 * @param sRGB Indicates if we load this as sRGB.
 * @returns Handle to the texture.
 */
GLuint load_texture (const char * path, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, bool sRGB);

/**
 * @brief Loads an adds an image to Globals.images.
 * @param filename The name and path to the image.
 * @see https://registry.khronos.org/OpenGL-Refpages/gl4/html/glTexParameter.xhtml
 * @param sRGB Indicates if we load this as sRGB.
 * @returns Index of the texture in Globals.images.
 */
int add_image_to_registry(char * filename, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, bool sRGB);

/**
 * @brief Saves an entire scene into separate files.
 * @param name The name of the scene.
 * @note The scene directories and bin suffixes are prepended and appended, only the name is needed.
 */
void scene_save(const char * name);

/**
 * @brief Loads an entire scene from separate files in FileDirectoryRegistry directories.
 * @param name The name of the scene.
 * @note The scene directories and bin suffixes are prepended and appended, only the name is needed.
 */
void scene_load(const char * name);

/**
 * @brief Saves only AudioProbe%s, AudioPortal%s, and ReflectionProbe%s.
 * @param name The name of the scene.
 * @note The scene directories and bin suffixes are prepended and appended, only the name is needed.
 */
void scene_save_volumes(const char * name);

/**
 * @brief Loads only AudioProbe%s, AudioPortal%s, and ReflectionProbe%s.
 * @param name The name of the scene.
 * @note The scene directories and bin suffixes are prepended and appended, only the name is needed.
 */
void scene_load_volumes(const char * name);


/**
 * @brief Reallocates ::RPROBE_CUBE_MAP_ARRAY to new dimensions or when loading.
 */
void reallocate_rprobe_array();



/**
 * @name Saving and Loading Individual System Data 
 * @brief Individual functions for loading specific systems. Useful for not touching the rest of the scene.
 * @{
 */
void save_vector(const char * filename, Vector * vec);
void load_vector(const char * filename, Vector * vec);
void save_rprobes(const char * filename, Vector * vec);
void load_rprobes(const char * filename, Vector * vec);
void save_lprobes(const char * filename, Vector * vec);
void load_lprobes(const char * filename, Vector * vec);
void save_aportals(const char * filename, Vector * vec);
void load_aportals(const char * filename, Vector * vec);
void save_aprobes(const char * filename);
void load_aprobes(const char * filename);
/** @} */


/**
 * @name Saving and Loading UI from Binary 
 * @param name File path and name.
 * @note Only .bin appended.
 * @{
 */
void save_ui(const char * name);
void load_ui(const char * name);
/** @} */


#endif
