#ifndef CAMERA_H
#define CAMERA_H

#include "engine_types.h"

extern Camera cam;

vec3 project_point_plane(vec3 point, vec3 plane_point, vec3 normal);
vec3 closest_point_triangle(vec3 p, vec3 a, vec3 b, vec3 c);
bool collision(vec3 proposed, vec3 pts[3], vec3 normal, CollisionPoints * p);
vec3 closest_point_segment(vec3 p, vec3 a, vec3 b);

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

mat4
    make_projection
    (
        float fov_deg,
        float aspect,
        float near,
        float far
    );

void
    update_camera
    (
        Skeleton * s,
        Camera * cam
    );
    
void
    resolve_collisions
    (
        Camera * current_cam
    );

void
    apply_velocity
    (
        Camera * current_cam,
        vec3 move_dir,
        float dt
    );

mat4
    make_look_at
    (
        Camera * cam,
        vec3 eye
    );

void
    apply_planet_velocity
    (
        Camera * current_cam,
        vec3 move_dir,
        float dt
    );

void
    compute_camera_basis
    (
        Camera * cam,
        float yaw,
        float pitch,
        float roll,
        vec3 world_up
    );

void
    update_light_camera
    (
      Camera * lcam,
      Light * light
    );
    
void
    look_at_selected_body ();




void move_foot(Skeleton * s, Camera * cam, vec3 vel);
void update_body_from_legs(LegIK *left_leg, LegIK *right_leg, vec3 intended_vel, float dt);
void reset_skeleton(Skeleton * s, vec3 pos);

#endif
