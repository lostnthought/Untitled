#version 430 core

#extension GL_ARB_bindless_texture : require
struct Material
{
    
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;
    vec4 emissive;
    float absorption_coefficients[6];
    uint pad0;
    uint pad1;
    float scattering_coefficients[6];
    uint pad2;
    uint pad3;

    sampler2D albedo_tex;
    sampler2D normal_tex;
    sampler2D arm_tex;
    sampler2D emissive_tex;
    
    float metallic;
    float roughness;
    float ao_strength;
    float normal_scale;
    uint alpha_mode;
    float alpha_cutoff;
    uint double_sided;
    float emissive_strength;

};


layout(std430, binding = 2) buffer MaterialData {

    Material materials[];

};

in vec3 FragWorldPos;

uniform vec3 uLightPos;
uniform float uFar;

flat in uint vMat;
in vec2 vUV;
void main()
{


    float lightDistance = length(FragWorldPos - uLightPos);


    Material mat = materials[vMat];
    vec4 albedo = texture(mat.albedo_tex, vUV) * mat.diffuse;
    if (albedo.a < 0.8f) discard;
    
    gl_FragDepth = lightDistance / uFar;
}
