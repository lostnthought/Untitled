
#include "entity_states.h"

#include "utilities/engine_utilities.h"
#include "utilities/utilities.h"
#include "entity.h"

void ESTATE_chair_interact(void * data)
{

    Entity * ent = (Entity*)data;

    MeshNode * mns = CAST_VEC(GLOBALS.mesh_nodes, MeshNode);
    MeshNode * mn = &mns[ent->mesh_node];
    mn->translation.z += 0.2;
    mn->global_matrix = compose_trs_from_mesh_node(mn);
    
    
}


void ESTATE_chair_combat(void * data)
{

    Entity * ent = (Entity*)data;

    
}


