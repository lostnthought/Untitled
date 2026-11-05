#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include "core/engine_types.h"


extern State STATE_REGISTRY[MAX_ENTITY_STATES];
extern int STATE_COUNT;

void update_entities();
void STATES_INIT();


PhysicsComponent * create_physics_component(Entity * ent, CollisionType collision_type, PhysicsType physics_type);
AnimationComponent * create_animation_component(Entity * ent);

Entity * create_entity(AssetInstance * asset_instance, int mn_id, bool physics_component, bool animation_component);
void destroy_entity(Entity * ent);

#endif
