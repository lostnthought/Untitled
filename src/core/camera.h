
/**
 * @file camera.h
 * @brief Functions for camera.
 */
#ifndef CAMERA_H
#define CAMERA_H

#include "engine_types.h"

extern Camera cam; ///< This is currently the player cam.

/**
 * @brief Makes an 4x4 orthographic matrix.
 * @param left The left dist.
 * @param right The right dist.
 * @param bottom The bottom dist.
 * @param top The top dist.
 * @param near The near plane.
 * @param far The far plane.
 * @returns The 4x4 orthographic matrix.
 */
mat4
    make_ortho
    (
        float left,
        float right,
        float bottom,
        float top,
        float near,
        float far
    );

/**
 * @brief Makes an 4x4 projection matrix.
 * @param fov_deg FOV in degrees.
 * @param aspect The aspect ratio (W/H).
 * @param near The near plane.
 * @param far The far plane.
 * @returns The 4x4 projection matrix.
 */
mat4
    make_projection
    (
        float fov_deg,
        float aspect,
        float near,
        float far
    );

/**
 * @brief Makes a 4x4 view matrix using camera vectors.
 * @param cam The camera to use.
 * @param eye The camera's "eye" position.
 * @returns The 4x4 view matrix.
 */
mat4
    make_look_at
    (
        Camera * cam,
        vec3 eye
    );

/**
 * @brief Computes camera vectors based on yaw, pitch, and roll.
 * @param cam The camera to effect.
 * @param yaw The direction left-right in radians.
 * @param pitch The direction up-down in radians.
 * @param roll The rotation in radians.
 * @param world_up The vector to cross with, normally (0,0,1).
 */
void
    compute_camera_basis
    (
        Camera * cam,
        float yaw,
        float pitch,
        float roll,
        vec3 world_up
    );

/**
 * @brief Updates a camera based on a skeleton's current neck and body position.
 * @param s The skeleton to use to determine camera position.
 * @param cam The camera to effect.
 */
void
    update_camera
    (
        Skeleton * s,
        Camera * cam
    );
    
/**
 * @brief Updates a directional light camera and locks it to texel size.
 * @param lcam Pointer to the light camera.
 * @param light Pointer to directional light.
 */
void
    update_light_camera
    (
      Camera * lcam,
      Light * light
    );

// TODO pass player instead of cam to this funciton.
/**
 * @brief Resolves collisions between camera and geometry, uses player.
 */
void
    resolve_collisions
    (
        Camera * current_cam
    );

/**
 * @brief Applies no-clip mode velocity to player with no gravity applied.
 * @param move_dir The move vectors from input.
 * @param dt The frame time / simulation step time.
 */
void
    apply_velocity
    (
        Camera * current_cam,
        vec3 move_dir,
        float dt
    );

/**
 * @brief Applies normal velocity to player with gravity and friction applied.
 * @param move_dir The move vectors from input.
 * @param dt The frame time / simulation step time.
 */
void
    apply_planet_velocity
    (
        Camera * current_cam,
        vec3 move_dir,
        float dt
    );


/**
 * @brief Moves feet, then solves a skeleton's leg IKs and updates the springs accordingly.
 * @param s The skeleton to effect.
 * @param cam The camera attached used to derive hip offsets.
 * @param vel The incoming move input raw direction.
 */
void update_skeleton(Skeleton * s, Camera * cam, vec3 vel);

// TODO pass player here
 
/**
 * @brief Calculates velocity based on leg positions and integrates them into the player. Also handles falling off ledges by initiating a drop when raycasts to ground fail above a certain margin.
 * @param left_leg Left leg IK.
 * @param right_leg Right leg IK.
 * @param intended_vel The raw input direction.
 * @param dt The frame time / simulation frame time.
 */
void update_body_from_legs(LegIK * left_leg, LegIK * right_leg, vec3 intended_vel, float dt);

/**
 * @brief Resets a skeleton to be relative to a new position.
 * @param s The skeleton to effect.
 * @param pos The target position.
 */
void reset_skeleton(Skeleton * s, vec3 pos);

bool collision(vec3 proposed, vec3 pts[3], vec3 normal, CollisionPoints * p);

#endif
