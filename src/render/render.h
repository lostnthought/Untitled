#ifndef RENDER_H
#define RENDER_H

#include "core/engine_types.h"

void add_mesh_to_buffer(Mesh * m, VertexLayout * layout);

void upload_lights();
void create_and_sort_draw_commands(int * dc_cnt, DrawCommand * dcs);
void render_to_cubemap_face(Camera * rprobe_cam, GLuint render_fbo, int render_width, int render_height, bool probe_bake);
void render();

void aabb_to_vertices(DebugVertex * out, AABB * aabb, vec3 offset);
void render_mesh_aabbs_from_root(MeshNode * mn);
void add_all_meshes_to_buffer(VertexLayout * layout);
void bind_static_shader(MeshNode * mn);
void upload_material_data();
void copy_light_to_gpu_light(LightGPU * lgpu, Light * l);
void bind_baking_ssbos();
void unbind_baking_ssbos();
void bind_render_ssbos();

#endif
