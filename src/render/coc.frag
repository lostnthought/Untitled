
#version 430 core

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

in vec2 vUV;

out vec4 fragColor;

uniform sampler2D gDepth;
uniform sampler2D uScene;

vec3 get_view_pos_from_depth(vec2 uv, float depth, mat4 invProjMatrix) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    
    vec4 viewSpacePos = invProjMatrix * ndc;
    
    return viewSpacePos.xyz / viewSpacePos.w;
}

float linearize(float z)
{
    float near = 0.1;
    float far = 100.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

uniform float uFocalLength;   
uniform float uFStop;   
uniform float uSensorHeight;   
uniform float uMaxBlurRadius;   

struct MeshNode
{
    vec4 min;
    vec4 max;
    vec4 center;
    vec4 extents;
    uint instance_count;
    uint static_mesh;
    uint lm_offset;
    uint material;
    uint first;
    uint count;
    uint pad0;
    uint pad1;
};

#define MAX_MESH_NODES 1024
layout(std430, binding = 15) buffer MeshNodeData {

    float focal_distance;
    uint pad0;
    uint pad1;
    uint pad2;
    MeshNode mns[MAX_MESH_NODES];

};

void main() {
    float rawDepth = texture(gDepth, vUV).r;

    vec3 viewPos = get_view_pos_from_depth(vUV, rawDepth, unjittered_inv_projection);
    
    float pixelDistance = -(viewPos.z); 

    float coeff = (uFocalLength * uFocalLength) / (uFStop * (focal_distance - uFocalLength));
    float cocMeters = coeff * ((pixelDistance - focal_distance) / pixelDistance);

    float cocUV = cocMeters / uSensorHeight;

    float finalCoC = clamp(cocUV, -uMaxBlurRadius, uMaxBlurRadius);

    fragColor = vec4(textureLod(uScene, vUV, 0).rgb, finalCoC); 


}
