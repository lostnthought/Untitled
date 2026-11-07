#ifndef LOAD_H
#define LOAD_H

#include "core/engine_types.h"

unsigned char * open_image(Image * image);
GLuint load_cubemap (const char * path);

GLuint load_3d_lut(const char* filepath);
GLuint load_texture (const char * path, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, bool sRGB);
int add_image_to_registry(char * filename, GLint min_filter, GLint mag_filter, GLint wrap_s, GLint wrap_t, bool sRGB);

void scene_save(const char * name);
void scene_load(const char * name);
void scene_save_volumes(const char * name);
void scene_load_volumes(const char * name);


void reallocate_rprobe_array();
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
void save_ui(const char * name);
void load_ui(const char * name);


#endif
