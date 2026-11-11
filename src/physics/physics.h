/**
 * @file physics.h
 * @brief PhysicsComponent impulse resolution and collision detection and acceleration structure traversal.
 */

#ifndef PHYSICS_H
#define PHYSICS_H

#include "core/engine_types.h"


/**
 * @brief Initializes a PhysicsComponent.
 * @param mesh_node The top-most MeshNode this corresponds to.
 * @param pc Pointer to the PC to initialize.
 * @param ai_aabb The AABB over the entire AssetInstance (unused).
 * @param collision_type Indicates what to use for collision detection.
 * @param physics_type Indicates what to compute during impulse resolution.
 * @note Uses an AABB to determine collision. If the MeshNode refers to a Mesh, then it uses that AABB. Else, it uses the MeshNode AABB, which recursively includes all children MeshNode%s.
 */
void init_PC(int mesh_node, PhysicsComponent * pc, AABB ai_aabb, CollisionType collision_type, PhysicsType physics_type);

/**
 * @brief The main PhysicsComponent loop that determines velocity, solves collisions, then integrates position.
 * @anchor physics_anchor
 * @details
 * \begin{align}
 * a(P) &= \begin{cases}
 * v_{n+1} &= v_{n} - G \cdot \Delta t \\\\
 * v_{n+1} &= v_{n+1} \cdot \beta \\\\
 * \omega_{n+1} &= \omega_{n} \cdot \alpha
 * \end{cases} \\\\
 * P\prime &= a(P) = \lbrace a(x) | x \in P\rbrace \\\\
 * q(Q) &= \begin{cases} \begin{bmatrix}
1 - 2Q_y^2 - 2Q_z^2 & 2Q_xQ_y - 2Q_wQ_z & 2Q_xQ_z + 2Q_wQ_y \\\\
2Q_xQ_y + 2Q_wQ_z & 1 - 2Q_x^2 - 2Q_z^2 & 2Q_yQ_z - 2Q_wQ_x \\\\
2Q_xQ_z - 2Q_wQ_y & 2Q_yQ_z + 2Q_wQ_x & 1 - 2Q_x^2 - 2Q_y^2
\end{bmatrix} \end{cases} \\\\
 * t(P) &= \begin{cases}
 * p = p + v\Delta t \\\\
 * \theta = || \omega || \cdot \Delta t \\\\
 * \left\. \begin{cases}
 * k = \frac{\omega}{||\omega||}\\\\
 * Q = \begin{pmatrix} k_x \cdot \sin \frac{\theta}{2}, k_y \cdot \sin \frac{\theta}{2}, k_z \cdot \sin \frac{\theta}{2}, \cos \frac{\theta}{2} \end{pmatrix}\\\\
 * r = Q r\\\\
 * r = \frac{r}{||r||}\\\\
 * \end{cases}\right\rbrace, & \text{if } \theta \gt 0 \\\\
 * M = \text{TRS}(p, r, s) \\\\
 * R = q(r) \\\\
 * \mathit{IIW} = (R \cdot IIA) \cdot R \\\\  
 * \mathit{%AABB} = \mathit{%AABB} \cdot M
 * \end{cases} \\\\
 * t(P) &= \lbrace t(x) | x \in P\prime\rbrace \\\\
 * l(P) &= \begin{cases}
 * \text{for } y = 0 \text{ to } P_{\text{max}} - 1: \\\\ 
 * \quad B[P_{i}, t(P)[y]_{i}] = P_{\mathit{%AABB}} \cap t(P)[y]_{\mathit{%AABB}},  & \text{if } P_{\mathit{%AABB}} \cap t(P)[y]_{\mathit{%AABB}} \neq \emptyset \\\\
 * \end{cases} \\\\
 * & \text{Where B is the manifold array}\\\\
 * B &= \lbrace l(x) | x \in t(P)\rbrace \\\\
 * f(n, i) &= \begin{cases}
 * R = r \times n \\\\
 * I = i \cdot R  \\\\
 * T = I \times r  \\\\
 * \psi = n \cdot T \\\\
 * K = \frac{1}{m_a} + \frac{1}{m_b} + \psi_a + \psi_b \\\\
 * (R, I, T, \psi, K)
 * \end{cases} \\\\
 * H(b) &= \begin{cases}
 * p = M \begin{bmatrix} p_x \\\\ p_y \\\\ p_z \\\\ 1\end{bmatrix} \\\\ 
 * c = M \begin{bmatrix} c_x \\\\ c_y \\\\ c_z \\\\ 1\end{bmatrix} \\\\ 
 * r = p - c \\\\ 
 * V = v - \omega \times r \\\\ 
 * m_i = \frac{1}{m} \\\\
 * \phi = V_a - V_b \\\\
 * \mathcal L = \vec \phi \cdot \vec N \\\\
 * \text{when } \mathcal L \in (-\infty, 0)\text{:} \\\\
 * \quad (R, I, T, \psi, K) = f(\vec N, \mathit{IIW}) \\\\
 * \quad J = \frac{-(1 + \mathcal{R}_a) \cdot \mathcal L}{K} \\\\
 * \quad \text{Where } \mathcal{R}_a = \text{ restitution} \\\\
 * \quad B_{F}\prime = \max{B_F + J, 0} \\\\
 * \quad J = B_F\prime - B_F \\\\
 * \quad F = J \vec{N} \\\\
 * \quad v = v \pm F\frac{1}{M} \\\\
 * \quad \tau = r \times \pm F \\\\
 * \quad \omega_F = \text{IIW} \cdot \tau \\\\
 * \quad \omega = \omega + \omega_F \\\\ 
 * \quad V = V + \omega \times r \\\\
 * \quad \phi = V_a - V_b \\\\
 * \quad \mathcal L = \vec \phi \cdot \vec N \\\\
 * \quad V_t = \phi - \mathcal L \vec N \\\\
 * \quad V_l = || V_t || \\\\
 * \quad \text{when } V_l \in (0, \infty)\text{:} \\\\
 * \qquad \hat t = \frac{V_t}{V_l} \\\\
 * \qquad (R, I, T, \psi, K) = f(\hat t, \mathit{IIW}) \\\\
 * \qquad J_t = \frac{-(\phi \cdot \hat t)}{K} \\\\
 * \qquad \gamma = \mu_a B_F \\\\
 * \qquad B_T\prime = \min(\max(B_T + J_t, -\gamma), \gamma) \\\\
 * \qquad J_t = B_T\prime - B_T \\\\
 * \qquad F = J_t \hat t \\\\
 * \qquad v = v \pm F \frac{1}{M} \\\\
 * \qquad \tau = r \times F \\\\
 * \qquad \omega_F = \mathit{IIW} \cdot \tau \\\\
 * \qquad \omega \pm \omega_F \\\\
 * \end{cases} \\\\
 * & \text {Where } \pm \text{ relates even and opposite forces and where H(b) modifies both respective bodies in } P\prime \\\\
 * P\prime\prime &= \lbrace H(x) | x \in B\rbrace \\\\
 * P\prime\prime\prime &= t(P) = \lbrace t(x) | x \in P\prime\prime\rbrace \\\\
 * S(b) &= \begin{cases}
 * p = M \begin{bmatrix} p_x \\\\ p_y \\\\ p_z \\\\ 1\end{bmatrix} \\\\ 
 * d = p_a - p_b \\\\
 * s = d \cdot \vec N \\\\
 * u = \begin{cases}
 * \left. \begin{bmatrix} 0 \\\\ 0 \\\\ 1 \end{bmatrix}\right\rbrace, & \text{if } \vec{N}_z \lt 0.9\\\\
 * \left. \begin{bmatrix} 1 \\\\ 0 \\\\ 0 \end{bmatrix}\right\rbrace, & \text{otherwise}
 * \end{cases} \\\\
 * t = u \times \vec N \\\\
 * T = \frac{t}{||t||} \\\\
 * D = d \cdot T \\\\
 * B \setminus \lbrace b \rbrace,  & \text{ if } s \gt 0.01 \text{ or } D \gt 0.01
 * \end{cases} \\\\
 * B\prime &= \lbrace S(x) | x \in B\rbrace \\\\
 * \end{align}
 */
void update_physics_components();

/**
 * @brief Solves an impulse for a Player PhysicsComponent against another PhysicsComponent.
 * @param pcA The Player PhysicsComponent.
 * @param pcB The B PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void player_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);

/**
 * @brief Solves an impulse for a Player PhysicsComponent against a static Mesh.
 * @param pcA The Player PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void player_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p);

/**
 * @brief Solves an impulse for a PhysicsComponent against a static Mesh.
 * @param pcA The PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 * @param p The input CollisionPoints that describe the collision .
 */
void PC_resolve_impulse_static(PhysicsComponent * pcA, CollisionPoints * p);

/**
 * @brief Solves an impulse for a PhysicsComponent against another.
 * @param pcA The A PhysicsComponent.
 * @param pcB The B PhysicsComponent.
 * @param p The input CollisionPoints that describe the collision penetration.
 */
void PC_resolve_impulse(PhysicsComponent * pcA, PhysicsComponent * pcB, CollisionPoints * p);

/**
 * @brief Calculates the mat3 IIA for a PhysicsComponent, using AABB bounds and mass.
 * @returns The inverse inertia matrix.
 */
mat3 calc_inverse_inertia(PhysicsComponent * pc);

#endif
