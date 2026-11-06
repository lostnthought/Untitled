/**
 * @file entity.h
 * @brief ECS creation and maintenance
 */

#ifndef ENTITY_H
#define ENTITY_H

#include <stdbool.h>
#include "core/engine_types.h"



/**
 * @brief Registry for entity states used for entity events and behaviors.
 */
extern State STATE_REGISTRY[MAX_ENTITY_STATES];
extern int STATE_COUNT; ///< The amount of states inside of STATE_REGISTRY.

/**
 * @brief TODO: Entity update loop.
 */
void update_entities();

/**
 * @brief Initializes entity states, which are a registry of function pointers used for entity events and behaviors.
 */
void STATES_INIT();

/**
 * @brief Creates an physics component for an entity
 * @param ent Pointer to the entity
 * @param collision_type What collision type is required
 * @param physics_type What physics type is required for impulse calculations
 * @returns Pointer to the physics component
 */
PhysicsComponent * create_physics_component(Entity * ent, CollisionType collision_type, PhysicsType physics_type);


/**
 * @brief Creates an animation component for an entity
 * @param ent Pointer to the entity
 * @returns Pointer to the animation component
 */
AnimationComponent * create_animation_component(Entity * ent);

/**
 * @brief Creates an entity using the free list of ENTITIES.
 * @param asset_instance The entity's asset instance in ASSET_INSTANCES
 * @param mn_id The entity's root mesh node
 * @param physics_component If this entity will have a physics component
 * @param animation_component If this entity will have an animation component
 * @returns A pointer to the entity
 */
Entity * create_entity(AssetInstance * asset_instance, int mn_id, bool physics_component, bool animation_component);

/**
 * @brief Deletes an entity by simply freeing it and its components.
 * @param ent Pointer to the entity
 */
void destroy_entity(Entity * ent);

#endif
