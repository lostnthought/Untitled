#ifndef ENGINE_UTILITIES_H
#define ENGINE_UTILITIES_H

#include "core/engine_types.h"
#include "utilities/mathf.h"
#include "utilities/utilities.h"

bool mn_inside_of_root_tree(MeshNode * mns, int check_id, int id);
static inline mat4 compose_trs_from_mesh_node(MeshNode * mn) {
    
    mat4 T = mat4_translation(mn->translation);
    mat4 R = mat4_from_quat(mn->rotation);
    mat4 S = mat4_scale(mn->scale);

    return m4_mul(T, m4_mul(R, S));
}

static inline void PV_step_size(ProbeVolume * pv, vec3 * step_x, vec3 * step_y, vec3 * step_z)
{
    
    vec3 fwd; vec3 right; vec3 up;
    OBB_vectors(pv->obb.rot, &fwd, &right, &up);
    
    float dx = (pv->obb.extents.x * 2) / (pv->resolution.x-1);
    float dy = (pv->obb.extents.y * 2) / (pv->resolution.y-1);
    float dz = (pv->obb.extents.z * 2) / (pv->resolution.z-1);

    *step_x = vec3_scale(fwd, dx);
    *step_y = vec3_scale(right, dy);
    *step_z = vec3_scale(up, dz);
    
}
MeshNode * get_root_node(MeshNode * mns, MeshNode * mn);


#endif
