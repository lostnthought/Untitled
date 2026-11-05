#ifndef GJK_H
#define GJK_H

#include "core/engine_types.h"

bool GJK(CollisionData a_cd, CollisionData b_cd, mat4 a_mat, mat4 b_mat, mat4 a_inv, mat4 b_inv, CollisionPoints * collision_points, EPAWorkspace * EW);
#endif
