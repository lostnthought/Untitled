#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/engine_types.h"
void init_PC(int mesh_node, PhysicsComponent * pc, AABB ai_aabb, CollisionType collision_type, PhysicsType physics_type);


void update_physics_components();

void player_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);
void player_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p);
void PC_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p, int contact);
void PC_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);

mat3 calc_inverse_inertia(PhysicsComponent * pc);

#endif
