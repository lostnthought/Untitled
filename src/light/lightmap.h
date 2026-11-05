#ifndef LIGHTMAP_H
#define LIGHTMAP_H


#include "core/engine_types.h"


void bake_reflection_probes();
void bake_lightmap(LightmapData * ld);
void generate_uv_lightmap(LightmapData * ld);
void init_lightmap_data(LightmapData * ld);
void init_uv_islands(LightmapData * ld);
void build_adjacency_map(Mesh * mesh);
void build_uv_islands(MeshNode * mn);
void init_mesh_lightmap_data(Mesh * mesh);
void load_lightmap_image(LightmapData * ld, const char * name);
void save_lightmap_to_binary(LightmapData * ld, const char * name);
void load_lightmap_from_binary(LightmapData * ld, const char * name);
void bake_lighting_probe_volume(ProbeVolume * pv);
void reset_lighting_pvs();
void check_lighting_pv();

void LM_regenerate_texels();
void LM_allocate_empty_mns();

#endif
