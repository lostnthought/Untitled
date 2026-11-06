/**
 * @file acceleration.h
 * @brief Acceleration structure generation and how we traverse them.
 */

#ifndef ACCELERATION_H
#define ACCELERATION_H
#include "engine_types.h"

/**
 * @brief Checks if a ray intersects an aabb.
 * @param aabb The aabb to check.
 * @param origin The origin of the ray.
 * @param dir The dir of the ray.
 * @param inv_dir 1.0f/dir of the ray.
 * @returns If we intersected the aabb.
 */
bool ray_intersects_aabb(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir);


/**
 * @brief Checks if a ray intersects an aabb and outputs an intersection point.
 * @param aabb The aabb to check.
 * @param origin The origin of the ray.
 * @param dir The dir of the ray.
 * @param inv_dir 1.0f/dir of the ray.
 * @param intersection_point A pointer to where we will place the intersection point.
 * @pre intersection_point must not be NULL.
 * @returns If we intersected the aabb.
 */
bool ray_intersects_aabb_with_point(AABB aabb, vec3 origin, vec3 dir, vec3 inv_dir, vec3 * intersection_point);

/**
 * @brief Checks if a ray intersects an obb and outputs an intersection point.
 * @param obb The obb to check.
 * @param origin The origin of the ray.
 * @param dir The dir of the ray.
 * @param intersection_point A pointer to where we will place the intersection point.
 * @pre intersection_point must not be NULL.
 * @returns If we intersected the obb.
 */
bool ray_intersects_obb_with_point(OBB obb, vec3 origin, vec3 dir, vec3 * intersection_point);

/**
 * @brief Raycasts through the scene and finds closest intersection to meshes.
 * @param start The ray's starting position.
 * @param dir The dir of the ray.
 * @param initial_t The starting T to begin evaluating, usually set at INFINITY.
 * @param check_normal To check backfaces or not.
 * @returns A RayHit with hit information.
 */
RayHit raycast_scene(vec3 start, vec3 dir, float initial_t, bool check_normal);

// TODO make this non-recursive
/**
 * @brief Traverses the mesh bvh to find collisions based on point vs aabb within a certain epsilon for player collisions.
 * @param m The mesh we are traversing.
 * @param mn The mesh node we are traversing.
 * @param nodes The bvh nodes we are traversing.
 * @param node The current node.
 * @param proposed The position we are checking.
 * @param inv_pos The position transformed into mesh node local space using inv matrix.
 * @param p The array of collision points to append to.
 * @returns If it collides with a mesh.
 */
bool traverse_collision_bvh(Mesh * m, MeshNode * mn, BVHNode * nodes, BVHNode * node, vec3 proposed, vec3 inv_p, CollisionPoints * p);

/**
 * @brief Initializes or updates the pre-existing scene bvh based on all mesh nodes with valid meshes.
 */
void init_scene_bvh();

/**
 * @brief Initializes the bvh for a mesh's triangles. Only performed on static meshes.
 */
void init_mesh_tri_bvh(Mesh * mesh);

/**
 * @brief Initializes a bvh for all current lights and emissive triangles in the scene, and sets the flux up for importance sampling.
 */
void build_light_bvh();

#endif
