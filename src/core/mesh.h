#ifndef MESH_H
#define MESH_H


#include "core/engine_types.h"

int find_mn_by_name(char name[256]);
void load_obj(const char *filename);
void build_adjacency_map(Mesh * mesh);

void deduplicate(Vector * in_verts, Vector * faces);
void incremental_hull(ConvexHull * hull);
void mesh_decomposition(Mesh * mesh);

void compute_mesh_hulls(ConvexHull * hull);

#endif
