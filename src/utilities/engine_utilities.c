

#include "core/engine_types.h"

bool mn_inside_of_root_tree(MeshNode * mns, int check_id, int id)
{
    if (id == -1 || check_id == -1) return false;
    MeshNode * mn = &mns[id];
    if (mn->id == check_id) return true;
    for (int i = 0; i < mn->child_count; i++)
    {
        bool inside = mn_inside_of_root_tree(mns, check_id, mn->children[i]);
        if (inside == true) return true;
    }
    return false;
    
}

MeshNode * get_root_node(MeshNode * mns, MeshNode * mn)
{


    if (mn->parent != -1)
    {
        return get_root_node(mns, &mns[mn->parent]);
    }

    return mn;
    
}
