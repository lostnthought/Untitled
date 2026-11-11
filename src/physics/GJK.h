/**
 * @file GJK.h
 * @brief GJK and EPA collision resolution.
 */

#ifndef GJK_H
#define GJK_H

#include "core/engine_types.h"


/**
 * @brief GJK polytope construction based on A and B (which can be any CollisionType), calls EPA internally.
 * @anchor collision_anchor
 * @see epa()
 * @see support_diff()
 * @see find_next_direction()
 * @details
 * \begin{align}
 * v &= \begin{bmatrix} 0 \\\\ 0\\\\ 1 \end{bmatrix}\\\\
 * c &= 0 \\\\
 * & \text{while } c = 0 \text{:} \\\\   
 * & \quad p = S(A, B) \\\\
 * & \quad c = \text{DNE}, & \text{if } p \cdot v \leq 0 \text{ or } p \in s.\\\\
 * & \quad s \Vert p \\\\ 
 * & \quad v = %D(s, v) \\\\ 
 * & \quad c = 1, & \text{if } v = v_\perp \\\\
 * \end{align}
 * \f$\text{Where } c \text{ is the collision status and s is the simplex.}\f$
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
