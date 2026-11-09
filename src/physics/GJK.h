/**
 * @file GJK.h
 * @brief GJK and EPA collision resolution.
 */

#ifndef GJK_H
#define GJK_H

#include "core/engine_types.h"


/**
 * @brief GJK polytope construction based on A and B (which can be any CollisionType), calls EPA internally.
 * @param a_cd CollisionData for A.
 * @param b_cd CollisionData for B.
 * @param a_mat Transformation for A.
 * @param b_mat Transformation for B.
 * @param a_inv Inverse transformation for A.
 * @param b_inv Inverse transformation for B.
 * @param collision_points Pointer to the outgoing collision data.
 * @param EW The workspace to conduct EPA in.
 * @returns If collision is detected through EPA.
 */
bool GJK(CollisionData a_cd, CollisionData b_cd, mat4 a_mat, mat4 b_mat, mat4 a_inv, mat4 b_inv, CollisionPoints * collision_points, EPAWorkspace * EW);

#endif
