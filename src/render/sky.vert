
#version 460

layout(location = 0) in vec3 aPosition;

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


out vec3 vPos;
void main()
{
    

    float c = cos(1.0f);
    
    float s = sin(1.0f);


    mat3 rotation = mat3(
        vec3(1, 0, 0),
        vec3(0, 0, 1),
        vec3(0, -1, 0)
    );
    
    vec3 pos = aPosition;
    vPos = pos;
    pos = rotation * pos;
    vec4 p = projection * view * vec4(pos, 0.0); 

    gl_Position = p.xyww;
    
}
