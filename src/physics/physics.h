/**
 * @file physics.h
 * @brief PhysicsComponent impulse resolution and collision detection and acceleration structure traversal.
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/engine_types.h"


/**
 * @brief Initializes a PhysicsComponent.
 * @param mesh_node The top-most MeshNode this corresponds to.
 * @param pc Pointer to the PC to initialize.
 * @param ai_aabb The AABB over the entire AssetInstance (unused).
 * @param collision_type Indicates what to use for collision detection.
 * @param physics_type Indicates what to compute during impulse resolution.
 * @note Uses an AABB to determine collision. If the MeshNode refers to a Mesh, then it uses that AABB. Else, it uses the MeshNode AABB, which recursively includes all children MeshNode%s.
 */
void init_PC(int mesh_node, PhysicsComponent * pc, AABB ai_aabb, CollisionType collision_type, PhysicsType physics_type);

/**
 * @brief The main PhysicsComponent loop that determines velocity, solves collisions, then integrates position.
 */
void update_physics_components();

/**
 * @brief Solves an impulse for a Player PhysicsComponent against another PhysicsComponent.
 * @param pcA The Player PhysicsComponent.
 * @param pcB The B PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void player_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);

/**
 * @brief Solves an impulse for a Player PhysicsComponent against a static Mesh.
 * @param pcA The Player PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void player_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p);

/**
 * @brief Solves an impulse for a PhysicsComponent against a static Mesh.
 * @param pcA The PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 * @param p The input CollisionPoints that describe the collision .
 */
void PC_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p);

/**
 * @brief Solves an impulse for a PhysicsComponent against another.
 * @param pcA The A PhysicsComponent.
 * @param pcB The B PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void PC_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);

/**
 * @brief Calculates the mat3 IIA for a PhysicsComponent, using AABB bounds and mass.
 * @returns The inverse inertia matrix.
 */
mat3 calc_inverse_inertia(PhysicsComponent * pc);

#endif
