
#version 430 core

in vec2 vUV;

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
    vec4 light_cam_pos;
    vec4 light_cam_vel;
    mat4 light_projection;
    mat4 light_unjittered_projection;
    mat4 light_view;
    mat4 light_inv_projection;
    mat4 light_unjittered_inv_projection;
    mat4 light_inv_view;
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
layout(std140, binding = 5) uniform SSAOKernel {
    vec4 kernel[16];
};

uniform sampler2D uDepth;
uniform sampler2D uNoise;
uniform ivec2 uSSAOResolution;

out float fragColor;

vec3 get_view_pos(vec2 uv)
{
    float depth = texture(uDepth, uv).r;
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    vec4 view_pos = unjittered_inv_projection * ndc;
    return view_pos.xyz / view_pos.w;
}


vec3 reconstruct_normal(vec2 uv, vec3 view_pos)
{

    vec2 uTexelSize = 1.0 / uSSAOResolution;
    vec3 view_top = get_view_pos(uv + vec2(0.0, uTexelSize.y));
    vec3 view_right = get_view_pos(uv + vec2(uTexelSize.x, 0.0));
    
    vec3 dx = view_right - view_pos;
    vec3 dy = view_top - view_pos;

    return normalize(cross(dx, dy));

}


void main()
{
    
    vec3 fragPos = get_view_pos(vUV);

    vec3 normal = reconstruct_normal(vUV, fragPos);

    vec2 noiseScale = uSSAOResolution / 4.0; 
    vec3 randomVec = texture(uNoise, vUV * noiseScale).xyz;

    vec3 tangent = normalize(randomVec - normal * dot(randomVec, normal));
    vec3 bitangent = cross(normal, tangent);
    mat3 TBN = mat3(tangent, bitangent, normal);

    float radius = 0.5;
    int samples = 16;

    
    float occlusion = 0.0f;
    for (int i = 0; i < 16; i++)
    {

        vec3 samplePos = TBN * kernel[i].xyz;
        samplePos = fragPos + samplePos * radius;

        vec4 offset = vec4(samplePos, 1.0);
        offset = unjittered_projection * offset;
        offset.xyz /= offset.w;
        offset.xyz = offset.xyz * 0.5 + 0.5;

        vec3 surface_pos = get_view_pos(offset.xy);

        float range_check = smoothstep(0.0, 1.0, radius / abs(fragPos.z - surface_pos.z));
        occlusion += (surface_pos.z >= samplePos.z + 0.01 ? 1.0 : 0.0) * range_check;
        
    }
    fragColor = float(1.0f - occlusion / samples);
}
