/**
 * @file lightmap.h
 * @brief UV Lightmap setup and baking, ReflectionProbe baking, and lighting ProbeVolume baking.
 */

#ifndef LIGHTMAP_H
#define LIGHTMAP_H


#include "core/engine_types.h"


/**
 * @brief Bakes ReflectionProbe%s for the entire scene onto cubemaps on ::RPROBE_CUBE_MAP_ARRAY.
 */
void bake_reflection_probes();

/**
 * @brief Bakes a UV Lightmap based on which UVIsland%s have UVIsland.currently_rasterizing set.
 * @param The LightmapData for the lightmap.
 */
void bake_lightmap(LightmapData * ld);

/**
 * @brief Frees current LightmapAtlas and UVIsland data and re-initializes the LightmapAtlas array and initializes the UVIsland Vector.
 * @param The LightmapData for the lightmap.
 */
void init_lightmap_data(LightmapData * ld);

/**
 * @brief Adds all current UVIsland%s to the lightmap by allocating LightmapRect%s.
 * @param The LightmapData for the lightmap.
 */
void init_uv_islands(LightmapData * ld);

/**
 * @brief Builds UVIsland%s over a MeshNode by stepping through TriangleAdjacency and stopping when normals become too different.
 * @param The MeshNode to traverse.
 */
void build_uv_islands(MeshNode * mn);

/**
 * @brief Saves a UV lightmap and relevant UVIsland and MeshNode data to binary.
 * @param The LightmapData for the lightmap.
 * @param The path to and name of the file.
 */
void save_lightmap_to_binary(LightmapData * ld, const char * name);

/**
 * @brief Loads a UV lightmap and relevant UVIsland and MeshNode data fro, binary.
 * @param The LightmapData for the lightmap.
 * @param The path to and name of the file.
 * @warning This does not currently free the current UVIsland%s on load.
 */
void load_lightmap_from_binary(LightmapData * ld, const char * name);

/**
 * @brief Loads a UV lightmap and relevant UVIsland and MeshNode data fro, binary.
 * @param The LightmapData for the lightmap.
 * @param The path to and name of the file.
 * @warning This does not currently free the current UVIsland%s on load.
 */
void bake_lighting_probe_volume(ProbeVolume * pv);

/**
 * @brief Resets current LPV%s from their slots in SceneData.LOADED_LIGHTING_PV.
 */
void reset_lighting_pvs();

/**
 * @brief Determines which LPV%s we may need to replace due to distance to new ones.
 */
void check_lighting_pv();

/**
 * @brief Regenerates lightmap texels: re-inits LightmapData, then builds UVIsland%s, then calls add_all_meshes_to_draw_buffer.
 */
void LM_regenerate_texels();

/**
 * @brief Allocates texels for MeshNode%s that don't have UVIsland%s generated yet. Used when loading a binary that does not include certain MeshNode%s.
 */
void LM_allocate_empty_mns();

#endif
