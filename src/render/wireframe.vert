
#version 430 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aColor;

layout(std140, binding = 0) uniform CameraData {
    vec4 cam_pos;
    vec4 cam_vel;
    mat4 projection;
    mat4 unjittered_projection;
    mat4 view;
    mat4 inv_projection;
    mat4 unjittered_inv_projection;
    mat4 inv_view;
};
layout(std140, binding = 1) uniform RenderingData {
    ivec4 resolution;
};
layout(std140, binding = 2) uniform LightCameraData {
    mat4 light_proj[4];
    mat4 light_view[4];
};
layout(std140, binding = 4) uniform PrevCameraData {
    vec4 prev_cam_pos;
    vec4 prev_cam_vel;
    mat4 prev_projection;
    mat4 prev_unjittered_projection;
    mat4 prev_view;
    mat4 prev_inv_projection;
    mat4 prev_unjittered_inv_projection;
    mat4 prev_inv_view;
};
out vec3 vNormal;
out vec3 vColor;
void main()
{
    vec3 pos = aPosition;
    vColor = aColor;
    gl_Position = unjittered_projection * view * vec4(pos, 1.0);        
}
