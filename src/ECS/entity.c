
#include "ECS/entity.h"
#include "core/engine_types.h"
#include "physics/physics.h"
#include "utilities/types.h"
#include "load/init.h"
#include <stdio.h>
#include "utilities/mathf.h"
#include "utilities/utilities.h"
#include "core/mesh.h"
#include "ECS/entity_states.h"


State STATE_REGISTRY[MAX_ENTITY_STATES];
int STATE_COUNT = 0;


static State * find_state_by_name(const char * name);
static State * find_default_state(const char * name);

typedef void (*func_ptr_t)(void*);

void STATES_INIT()
{

    sprintf(STATE_REGISTRY[STATE_COUNT].name, "chair_default");
    STATE_REGISTRY[STATE_COUNT].fn[ESTATE_PLAYER_INTERACT] = ESTATE_chair_interact;
    STATE_COUNT++;
    // sprintf(STATE_REGISTRY[STATE_COUNT].name, "chair_combat");
    // STATE_REGISTRY[STATE_COUNT].fn[ESTATE_COMBAT] = ESTATE_chair_combat;
    // STATE_COUNT++;

    
}

static State * find_state_by_name(const char * name)
{

    for (int i = 0; i < STATE_COUNT; i++)
    {
        if (strcmp(STATE_REGISTRY[i].name, name) == 0)
        {
            return &STATE_REGISTRY[i];
        }
    }

    return NULL;
}


static State * find_default_state(const char * name)
{

    char default_name[256];

    sprintf(default_name, "%s_default", name);
    
    State * s = find_state_by_name(default_name);
    if (!s)
    {
        printf("NO DEFAULT STATE FOUND: %s\n", name);
    }
    
    return s;
}


// TODO entity loop
void update_entities()
{
    
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (ENTITIES[i].free) continue;
        Entity * ent = &ENTITIES[i];


        if (ent->pc)
        {
            

            
        }
        
    }

}
PhysicsComponent * create_physics_component(Entity * ent, CollisionType collision_type, PhysicsType physics_type)
{
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);

    PhysicsComponent * pc = NULL;
    for (int i = 0; i < MAX_PHYSICS_COMPONENTS; i++)
    {
        if (PHYSICS_COMPONENTS[i].free)
        {
            pc = &PHYSICS_COMPONENTS[i];
            PHYSICS_COMPONENTS[i].free = false;
            PHYSICS_COMPONENTS[i].ent = ent;

            break;

        }
    }
    if (!pc)
    {

        printf("NO AC SLOT FOUND\n");
        ent->pc = NULL;
    }
    else
    {
        ent->pc = pc;
        mns[ent->mesh_node].pc = pc;
        init_PC(ent->mesh_node, pc, ent->aabb, collision_type, physics_type);
    }
    
    return pc;
}

AnimationComponent * create_animation_component(Entity * ent)
{

    AnimationComponent * ac = NULL;
    for (int i = 0; i < MAX_ANIMATION_COMPONENTS; i++)
    {
        if (ANIMATION_COMPONENTS[i].free)
        {
            ac = &ANIMATION_COMPONENTS[i];
            ANIMATION_COMPONENTS[i].free = false;
            ANIMATION_COMPONENTS[i].ent = ent;

            break;

        }
    }
    if (!ac)
    {

        printf("NO AC SLOT FOUND\n");
        ent->ac = NULL;
    }
    else
    {
        ent->ac = ac;
    }
    
    return ac;
}



Entity * create_entity(AssetInstance * asset_instance, int mn_id, bool physics_component, bool animation_component)
{

    bool found_entity = false;
    Entity * ent = NULL;
    for (int i = 0; i < MAX_ENTITIES; i++)
    {
        if (ENTITIES[i].free)
        {

            found_entity = true;
            ent = &ENTITIES[i];
            ENTITIES[i].free = false;
            break;

        }
    }
    if (!ent)
    {
        printf("NO ENT SLOT FOUND\n");
        return NULL;
    }

    ent->mesh_node = mn_id;
    ent->state = NULL;
    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    Mesh * meshes = CAST_VEC(GLOBALS.meshes, Mesh);
    ent->free = false;
    if (asset_instance && mn_id != -1)
    {
        MeshNode * mn = &mns[mn_id];
        ent->state = find_default_state(mn->name);
        ent->mesh_node = mn_id;
        mn->ent = ent;
        ent->skin = mns[ent->mesh_node].skin;
        ent->instance = asset_instance;
        if (mn->mesh != -1)
        {
            Mesh * m = &meshes[mn->mesh];
            ent->aabb = m->aabb;
        }
        else
        {
            ent->aabb = asset_instance->aabb;
        }
        if (physics_component)
        {
            create_physics_component(ent, mn->collision_type, mn->physics_mode);
        }
        else
        {
            ent->pc = NULL;
        
        }
        
    }
    if (animation_component)
    {
        create_animation_component(ent);
        
    }
    else
    {
        ent->ac = NULL;
    }

    return ent;
   
}


void destroy_entity(Entity * ent)
{

    ent->free = true;
    if (ent->pc)
    {
        ent->pc->free = true;
    }
    if (ent->ac)
    {
        ent->ac->free = true;
    }

    
}
    
