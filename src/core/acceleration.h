#ifndef ACCELERATION_H
#define ACCELERATION_H
#include "engine_types.h"

bool ray_intersects_aabb(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir);
bool ray_intersects_aabb_with_point(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir, vec3 * intersection_point);
bool ray_intersects_obb_with_point(OBB obb, vec3 origin, vec3 dir, vec3 * intersection_point);

bool ray_traverse_mesh_bvh_lighting(Mesh * m, BVHNode * nodes, BVHNode * node, vec3 origin, vec3 inv_pos, vec3 dir, vec3 mod_dir, vec3 inv_dir, RayHit * hit, size_t mesh, size_t tri, bool check_normal);
bool ray_traverse_scene_bvh_lighting(SceneData * sd, BVHNode * nodes, vec3 origin, vec3 inv_pos, vec3 dir, vec3 mod_dir, vec3 inv_dir, RayHit * hit, size_t mesh, size_t tri, bool check_normal);
RayHit raycast_scene(vec3 start, vec3 dir, float initial_t, bool check_normal);
bool traverse_collision_bvh(Mesh * m, MeshNode * mn, BVHNode * nodes, BVHNode * node, vec3 start, vec3 inv_s, vec3 proposed, vec3 inv_p, vec3 dir, vec3 td, vec3 inv_dir, CollisionPoints * p);
void init_tri_bvh(ConvexHull * hull);
void init_scene_bvh();
void init_mesh_tri_bvh(Mesh * mesh);
void build_light_bvh();

#endif
