
#extension GL_ARB_bindless_texture : require
layout(location = 0)out vec2 gVel;
layout(location = 1)out vec4 gNormal;
layout(location = 2)out float gPomDepth;
layout(location = 3)out vec4 gAlbedo;
layout(location = 4)out vec4 gExtra;
layout(location = 5)out vec2 gUV;

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
    // int albedo_tex;
    // uint pad0;
    // int normal_tex;
    // uint pad1;
    // int arm_tex;
    // uint pad2;
    // int emissive_tex;
    // uint pad3;
    
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



in vec3 Normal;
in mat3 TBN;
flat in uint Mat;
in vec2 UV;

in vec3 ViewPos;
in vec4 ClipPos;
in vec4 PrevClipPos;
in vec3 ViewDirTanSpace;
const float height_scale = 0.02f;
uniform sampler2D uAlbedo;
uniform sampler2D uARM;
uniform sampler2D uNormal;
// uniform vec2 uJitterOffset;

vec2 ParallaxMapping(sampler2D normal_map, vec2 texCoords, vec3 viewDir, out float final_depth)
{

    vec2 dx = dFdx(texCoords);
    vec2 dy = dFdy(texCoords);
    const float minLayers = 2.0;
    const float maxLayers = 16.0;

    float tx_width = float(textureSize(normal_map, 0).x);

    vec2 dx2 = max(dx * dx * tx_width * tx_width, dy * dy * tx_width * tx_width);
    float mip_level = 0.5 * log2(max(dx2.x, dx2.y));

    float fade = clamp(1.0 - (mip_level / 3.0), 0.0, 1.0);
    

    float current_max_layers = mix(minLayers, maxLayers, fade);
    
    
    float numLayers = mix(current_max_layers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), viewDir)));
    
    float layerDepth = 1.0 / numLayers;
    float currentLayerDepth = 0.0;

    
    
    float uv_scale = length(fwidth(texCoords));
    // vec2 P = viewDir.xy / max(viewDir.z, 0.001) * height_scale;
    vec2 P = viewDir.xy *height_scale;
    vec2 deltaTexCoords = vec2(P.x, -P.y) / numLayers;
    
    vec2 currentTexCoords = texCoords;
    float freq = 10;
    float currentDepthMapValue = textureGrad(normal_map, currentTexCoords, dx, dy).a;
    
    int safety = 20;
    int cur = 0;
    while(currentLayerDepth < currentDepthMapValue && safety > cur)
    {
        currentTexCoords -= deltaTexCoords;


        currentDepthMapValue = textureGrad(normal_map, currentTexCoords, dx, dy).a;
        currentLayerDepth += layerDepth;
        cur++;
    }
    
    vec2 prevTexCoords = currentTexCoords + deltaTexCoords;
    float afterDepth  = currentLayerDepth - currentDepthMapValue;
    float current_depth = textureGrad(normal_map, currentTexCoords, dx, dy).a;
    float prev_depth = textureGrad(normal_map, prevTexCoords, dx, dy).a;
    float beforeDepth = (prev_depth) - currentLayerDepth + layerDepth;
    
    float weight = afterDepth / (afterDepth + beforeDepth);
    vec2 finalTexCoords = mix(currentTexCoords, prevTexCoords, weight);
    final_depth = mix(current_depth, prev_depth, weight);
    
    return finalTexCoords;
}
// in vec3 vWorldPos;
void main()
{

    Material mat = materials[Mat];
    vec3 viewDir = normalize(ViewDirTanSpace);
    vec3 viewDirViewSpace = normalize(ViewPos.xyz);
    float final_depth = 0.0;
    vec2 uv = UV;
    // #ifdef POM
    //     uv = ParallaxMapping(mat.normal_tex, UV, viewDir, final_depth);
    // #endif

    vec3 displaced_view_pos = ViewPos.xyz + (viewDirViewSpace * (final_depth * height_scale / ViewDirTanSpace.z));

    vec4 clip_space_pos = projection * vec4(displaced_view_pos, 1.0);

    float ndc_depth = clip_space_pos.z / clip_space_pos.w;

    float ndc_screen_depth = ndc_depth * 0.5 + 0.5;

    gPomDepth = ndc_screen_depth;

    gUV = uv;
    // gl_FragDepth= ndc_screen_depth;

    
    // vec2 uv = UV;
    gAlbedo = texture(mat.albedo_tex, uv) * mat.diffuse;
    // gAlbedo = texture(uAlbedo, uv) * mat.diffuse;
    vec3 map_normal = texture(mat.normal_tex, uv).rgb * 2.0 - 1.0;
    // vec3 map_normal = texture(uNormal, uv).rgb * 2.0 - 1.0;
    vec3 N = normalize(TBN * map_normal);
    // vec3 N = Normal;

    // vec3 ARM = texture(uARM, uv).rgb;
    vec3 ARM = texture(mat.arm_tex, uv).rgb;
    gNormal = vec4(N, 1.0);
    gExtra = vec4(ARM.r, ARM.g * mat.roughness, ARM.b * mat.metallic, mat.emissive_strength);
    // gExtra = vec4(ARM.r * mat.ao_strength, ARM.g * mat.roughness,0, mat.emissive_strength);
    // gExtra = vec4(ARM.r * mat.ao_strength, 0.0, 0.7, mat.emissive_strength);
    // gExtra = vec3(ARM.r, , mat.emissive_strength);

    vec2 current_ndc = ClipPos.xy / ClipPos.w;
    vec2 previous_ndc = PrevClipPos.xy / PrevClipPos.w;

    vec2 current_uv = current_ndc * 0.5 + 0.5;
    vec2 previous_uv = previous_ndc * 0.5 + 0.5;

    gVel = (current_uv - previous_uv);    
    // gVel = (current_uv - previous_uv);    
    // gVel = vec2(1,1);    
}
