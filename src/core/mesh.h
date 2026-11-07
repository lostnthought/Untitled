
/**
 * @file mesh.h
 * @brief Mesh functions that modify and initialize Mesh data.
 */

#ifndef MESH_H
#define MESH_H


#include "core/engine_types.h"



/**
 * @brief Finds a Mesh inside of Globals.meshes via string.
 * @param name The name to compare.
 * @returns The index of the mesh.
 * @warning Returns -1 on failure.
 */
int find_mn_by_name(char name[256]);
/**
 * @brief Builds a map of TriangleAdjacency over adjacent MeshFace%s on the Mesh.
 * @param mesh The Mesh to build over.
 */
void build_adjacency_map(Mesh * mesh);

/**
 * @brief Deduplicated vertices.
 * @param mesh The Mesh to build over.
 */
void deduplicate(Vector * in_verts, Vector * faces);
void incremental_hull(ConvexHull * hull);
void mesh_decomposition(Mesh * mesh);

void compute_mesh_hulls(ConvexHull * hull);

#endif
