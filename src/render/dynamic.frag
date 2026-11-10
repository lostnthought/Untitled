
/**
 * @file dynamic.frag
 * @brief The main forward fragment shader.
 */
#extension GL_ARB_bindless_texture : require


/**
 * @brief Oriented Bounding Boxes.
 * @ingroup shader_structs_group
 */
struct OBB
{
    vec3 center;
    uint loaded;
    vec3 axis_x;
    uint pad1;
    vec3 axis_y;
    uint pad2;
    vec3 axis_z;
    uint pad3;
    vec3 extents;
    uint pad4;
    
};

/**
 * @brief GPU Lights.
 * @ingroup shader_structs_group
 */
struct Light
{
    
    vec3 pos;
    uint entity_id;

    vec3 dir;
    float outer_cos;
    
    vec3 color;
    float cos_diff;

    float intensity;
    uint light_type;
    float inner_angle;
    float outer_angle;

    float radius;
    uint layer;
    uint mesh;
    uint tri;

    float area;
    uint pad0;
    uint pad1;
    uint pad2;

};

/**
 * @brief GPU-side Material%s.
 * @ingroup shader_structs_group
 */
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

/**
 * @brief GPU-side ReflectionProbe%s.
 * @ingroup shader_structs_group
 */
struct RProbe
{
    vec3 center;
    uint pad0;
    vec3 min;
    uint pad1;
    vec3 max;
    uint pad2;
    
};

/**
 * @brief GPU-side Cluster%s for clustered forward.
 * @ingroup shader_structs_group
 */
struct Cluster
{
    
    uint offset;
    uint count;
};



layout(location = 0)out vec4 fragColor;
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

layout(std140, binding = 3) uniform RProbeData {

    uint rprobe_indices[4];
    uint rprobe_count;
    uint pad0;
    uint pad1;
    uint pad2;

};


layout(std140, binding = 6) uniform ProbeVolumeData{
    OBB pvs[6];
};



layout(std430, binding = 0) buffer LightData {

    Light lights[];

};

layout(std430, binding = 1) buffer MeshLightData {

    uint light_indices[];

};

layout(std430, binding = 2) buffer MaterialData {

    Material materials[];

};
layout(std430, binding = 3) buffer LightmapData {

    vec4 uvs[];

};
layout(std430, binding = 10) buffer RProbeSSBO {

    RProbe rprobes[10];

};

layout(std430, binding = 13) buffer GlobalLightIndexData {
    uint global_light_indices[];
};

layout(std430, binding = 14) buffer ClusterData {
    Cluster clusters[];
};

in vec3 Normal;
in vec3 Tangent;
in float TangentSign;
in vec3 Pos;
in vec3 ViewPos;
in vec2 UV;
in vec2 LMUV;
in vec3 Color;
in vec4 FragPosLightSpace[4];
in mat3 TBN;
in vec3 ViewDirTanSpace;

flat in ivec2 PointLight;
flat in uvec2 DirectionalLight;
flat in uvec2 SpotLight;

flat in uint Static;
flat in uint Mat;

uniform int material;
uniform sampler2D uLightmap;
uniform sampler2D uDirectionalmap;
uniform sampler2DArrayShadow uShadowmap;
uniform sampler2D gAlbedo;
uniform sampler2D gNormal;
uniform sampler2D gExtra;
uniform samplerCubeArray uRProbe;
uniform samplerCubeArrayShadow uPLSCMA;
uniform sampler2D uBRDFLUT;
uniform samplerCube uSkybox;
uniform sampler2D uFBM;
uniform sampler2D uSSAO;
uniform sampler2D uDepth;
uniform sampler2D uSSR;
uniform sampler2D gUV;
uniform vec3 probe_origin; 
uniform float probe_spacing;
uniform ivec3 probe_grid_size;
uniform int index_offset;
uniform int probe_bake;



uniform uint uLPVDepthPadded;
uniform sampler3D probe_tex;


uniform vec2 uScreenRes;
uniform uvec3 uGridSize;
uniform float uNear;
uniform float uFar;
uniform float uLogScale;

const float height_scale = 0.02f;



vec2 parallax_mapping(sampler2D normal_map, vec2 texCoords, vec3 viewDir, out float final_depth)
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
    vec2 P = viewDir.xy *height_scale;
    vec2 deltaTexCoords = vec2(P.x, -P.y) / numLayers;
    
    vec2 currentTexCoords = texCoords;
    float freq = 10;
    float currentDepthMapValue = textureGrad(normal_map, currentTexCoords, dx, dy).a;
    
    int safety = 100;
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

vec3 get_parallax_corrected_vector(vec3 worldPos, vec3 viewDir, vec3 normal, vec3 boxMin, vec3 boxMax, vec3 probeCenter) {

    vec3 reflectionDir = reflect(-viewDir, normal);

    vec3 firstPlaneIntersect = (boxMax - worldPos) / reflectionDir;
    vec3 secondPlaneIntersect = (boxMin - worldPos) / reflectionDir;

    vec3 furthestPlane = max(firstPlaneIntersect, secondPlaneIntersect);

    float distance = min(min(furthestPlane.x, furthestPlane.y), furthestPlane.z);

    vec3 intersectPosition = worldPos + reflectionDir * distance;

    vec3 correctedReflDir = intersectPosition - probeCenter;

    return correctedReflDir;
}



float shadow_calculation(uint layer, vec4 pos, float diff)
{
    vec3 proj_coords = pos.xyz / pos.w;

    proj_coords = proj_coords * 0.5 + 0.5;
    if (proj_coords.z > 1.0f) return 0;

    float bias = max(0.005f * (diff), 0.0005f);


    float current_depth = proj_coords.z;

    float shadow = texture(uShadowmap, vec4(proj_coords.xy, layer, current_depth));    

    
    return shadow;

}


const vec3 DIELECTRIC_F0 = vec3(0.04);
const float PI = 3.14159265359;

float distribution_GGX(float NdotH, float roughness)
{
    float a = roughness * roughness;
    float a2 = a * a;
    float d = (NdotH * a2 - NdotH) * NdotH + 1.0;
    return a2 / (PI * d * d + 0.00001);
}

/**
 * @brief Computes Schlick's approximation for Fresnel reflectance.
 * \f{eqnarray*}
 * F(\theta) &=& F_0 + (1 - F_0)(1 - \cos\theta)^5 \\
 * \text{where } \cos\theta &=& \max(0, \omega_h \cdot v)
 * \f}
 */ 
vec3 fresnel_schlick(float CosTheta, vec3 F0)
{
    float fc = 1.0 - CosTheta;
    float fc2 = fc * fc;
    float fc5 = fc2 * fc2 * fc;
    return F0 + (1.0 - F0) * fc5;
}

vec3 fresnel_schlick_roughness(float cosTheta, vec3 albedo, float roughness, float metallic)
{

    vec3 F0 = mix(DIELECTRIC_F0, albedo, metallic);
    
    vec3 F = fresnel_schlick(cosTheta, F0);
    return F;
}

float visibility(float NdotV, float NdotL, float roughness)
{
    float k = roughness * 0.5;
    float gV = NdotV * (1.0 - k) + k;
    float gL = NdotL * (1.0 - k) + k;
    return 0.25 / (gV * gL + 0.00001);
}

vec3 calculate_light(vec3 N, vec3 V, vec3 L, vec3 albedo,  float roughness, float metallic, vec3 light_color)
{
    vec3 H = normalize(V + L); 
    
    float NdotL = max(dot(N, L), 0.0);
    float NdotV = max(dot(N, V), 0.0);
    float NdotH = max(dot(N, H), 0.0);
    float LdotH = max(dot(L, H), 0.0);

    if (NdotL <= 0.0) return vec3(0.0); 

    vec3 F0 = mix(DIELECTRIC_F0, albedo, metallic);

    float D = distribution_GGX(NdotH, roughness);
    vec3  F = fresnel_schlick(LdotH, F0);
    float vis = visibility(NdotV, NdotL, roughness);

    vec3 specular = D * F * vis;

    vec3 kS = F;                
    vec3 kD = vec3(1.0) - kS;   
    kD *= 1.0 - metallic;       

    vec3 diffuse = kD * albedo / PI;

    return (diffuse + specular) * light_color * NdotL;
}



float normal_filtering(vec3 N, float roughness) {
    vec3 dNdx = dFdx(N);
    vec3 dNdy = dFdy(N);
    
    float variance = dot(dNdx, dNdx) + dot(dNdy, dNdy);
    
    float kernel_roughness = min(variance * 0.5, 0.5); 
    
    float alpha = roughness * roughness;
    float filtered_alpha = clamp(alpha + kernel_roughness, 0.0, 1.0);
    
    return sqrt(filtered_alpha); 
}



vec3 palette(in float t, in vec3 a, in vec3 b, in vec3 c, in vec3 d) {
    return a + b * cos(6.28318 * (c * t + d));
}

float hash(vec3 p) {
    p = fract(p * 0.3183099 + vec3(0.1));
    p *= 17.0;
    return fract(p.x * p.y * p.z * (p.x + p.y + p.z));
}

float compute_probe_weight(vec3 world_pos, RProbe rp) {

    vec3 dist_min = world_pos - rp.min;
    vec3 dist_max = rp.max - world_pos;
    
    vec3 min_dist = min(dist_min, dist_max);
    float closest_wall_dist = min(min(min_dist.x, min_dist.y), min_dist.z);

    if (closest_wall_dist <= 0.0f) {
        return 0.0f;
    }

    vec3 fade_margin = vec3(5, 5, 5);

    float margin = min(min(fade_margin.x, fade_margin.y), fade_margin.z);
    float weight = closest_wall_dist / margin;

    if (weight > 1.0f) weight = 1.0f;
    if (weight < 0.0f) weight = 0.0f;


    return weight;
}


vec3 sample_weighted_rprobes(vec3 pos, vec3 view_dir, vec3 normal, float roughness_lod)
{

    float weights[4];
    float sum_weights = 0.0f;
    for (int i = 0; i < rprobe_count; i++)
    {
        RProbe rp = rprobes[rprobe_indices[i]];
        weights[i] = clamp(compute_probe_weight(pos, rp), 0.0, 1.0);        

        sum_weights += weights[i];
    }
    vec3 sky_color = vec3(1,0,0);
    vec3 color = vec3(0.0,0.0,0.0);
    for (int i = 0; i < rprobe_count; i++)
    {
    
        float w_norm = weights[i] / sum_weights;
        w_norm = clamp(w_norm, 0.0, 1.0);
        RProbe rp = rprobes[rprobe_indices[i]];
        vec3 corrected = get_parallax_corrected_vector(pos, view_dir, normal, rp.min, rp.max, rp.center);
        corrected = vec3(corrected.x, corrected.z, -corrected.y);
        color += textureLod(uRProbe, vec4(corrected, rprobe_indices[i]), roughness_lod).rgb * w_norm;
    }


    return color;

    
}



float calculate_attenuation(float distance, float radius, float intensity) {
    float attenuation = intensity / (distance  + 1.0);
    
    float factor = distance / (radius * radius);
    float window = clamp(1.0 - factor * factor * factor * factor, 0.0, 1.0);
    window = window * window; 
    
    return attenuation * window;
}

uint get_cluster_index() {
    float depth = max(-ViewPos.z, uNear); 
    
    uint z_tile = uint(max(0.0, log2(depth / uNear) * uLogScale));
    z_tile = min(z_tile, uGridSize.z - 1); 

    uint x_tile = uint(gl_FragCoord.x / (uScreenRes.x / float(uGridSize.x)));
    uint y_tile = uint(gl_FragCoord.y / (uScreenRes.y / float(uGridSize.y)));

    x_tile = clamp(x_tile, 0u, uGridSize.x - 1u);
    y_tile = clamp(y_tile, 0u, uGridSize.y - 1u);

    uint cluster_index = x_tile + (y_tile * uGridSize.x) + (z_tile * uGridSize.x * uGridSize.y);

    return cluster_index;
}


float calculate_specular_occlusion(float NdotV, float ao, float roughness) 
{
    return clamp(pow(NdotV + ao, exp2(-16.0 * roughness - 1.0)) - 1.0 + ao, 0.0, 1.0);
}



float linearize(float z)
{
    float near = 0.1;
    float far = 100.0;
    return (2.0 * near) / (far + near - z * (far - near));
}

/**
 * @brief Converts a position to local OBB space and returns the coord as if it was a texture from \f$0 \rightarrow 1\f$.
 * \begin{align}
 * p &= \mathrm{Position} \\\\
 * c &= \mathrm{Center} \\\\
 * l &= \mathrm{Length} \\\\
 * \Delta &= p - c \\\\
 * n &= \mathbf{M^r}\mathbf{\Delta} \\\\
 * &= (\frac{n}{l}) * 0.5
 * \end{align}
 */ 
vec3 pos_to_pv_local(OBB obb, vec3 pos)
{
    
    vec3 local_pos = pos - obb.center;
    mat3 rot = mat3(obb.axis_x, obb.axis_y, obb.axis_z);
    vec3 unrotated = inverse(rot) * local_pos;
    vec3 norm = unrotated / (obb.extents * 2.0f);
    return norm + 0.5f;
}

#define LPV_SLOTS 6
#define SH_MAX 9
void main()
{

    vec2 texelSize = 1.0 / uScreenRes.xy;
    vec2 screenUV = gl_FragCoord.xy * texelSize;    
    uint cluster_index = get_cluster_index();    
    float final_depth;
    Material mat = materials[Mat];
    vec3 viewDir = normalize(ViewDirTanSpace);
    #ifdef POM_VDIR
        fragColor = vec4(viewDir, 1.0);
        return;
    #endif
    vec2 uv = UV;
    #ifdef POM
        uv = parallax_mapping(mat.normal_tex, UV, viewDir, final_depth);
    #endif

    #ifdef DVIEW_UV
        fragColor = vec4(uv.x, uv.y, 0.0, 1.0);
        return;
    #endif

    vec4 albedo = texture(mat.albedo_tex, uv) * mat.diffuse;
    #ifdef ALBEDO
        fragColor = albedo;
        return;
    #endif

    vec3 arm = texture(mat.arm_tex, uv).rgb;
    #ifdef ARM
        fragColor = vec4(arm, 1.0);
        return;
    #endif

    vec4 map_normal = texture(mat.normal_tex, uv) * 2.0 - 1.0;
    map_normal.xyz = normalize(map_normal.xyz);
    vec3 N = normalize(TBN * map_normal.xyz);
    #ifdef NORMAL
        fragColor = vec4(N, 1.0);
        return;
    #endif

    vec3 emissive = texture(mat.emissive_tex, uv).rgb;
    #ifdef EMISSIVE
        fragColor = vec4(emissive * mat.emissive.rgb * 3.0, 1.0);
        return;
    #endif
    vec3 original_normal = normalize(Normal);
    #ifdef ORIGINAL_NORMAL
        fragColor = vec4(original_normal, 1.0);
        return;
    #endif

    
    vec3 sun_dir = vec3(-0.5, 0.2, 0.9);
    vec3 L = normalize(sun_dir);

    vec3 view_dir = normalize(cam_pos.xyz - Pos);

    float cosTheta = max(dot(N, view_dir), 0.0);
    
    float ao = arm.r * texture(uSSAO, screenUV).r;
    vec4 SSR = texture(uSSR, screenUV);
    
    float roughness = arm.g * mat.roughness;
    float metallic = arm.b * mat.metallic;
    vec3 acc = vec3(0,0,0);
    vec3 ddir = vec3(0,0,1);

    uint directional_light_offset = DirectionalLight.x;
    uint directional_light_count = DirectionalLight.y;
    uint spot_light_offset = SpotLight.x;
    uint spot_light_count = SpotLight.y;

    Cluster cluster = clusters[cluster_index];
    float depth = max(-ViewPos.z, uNear); 
    
    uint z_tile = uint(max(0.0, log2(depth / uNear) * uLogScale));
    z_tile = min(z_tile, uGridSize.z - 1); 

    uint x_tile = uint(gl_FragCoord.x / (uScreenRes.x / float(uGridSize.x)));
    uint y_tile = uint(gl_FragCoord.y / (uScreenRes.y / float(uGridSize.y)));

    x_tile = clamp(x_tile, 0u, uGridSize.x - 1u);
    y_tile = clamp(y_tile, 0u, uGridSize.y - 1u);

    #ifdef CLUSTERS
        fragColor = vec4(float(x_tile) / uGridSize.x, float(y_tile) / uGridSize.y, depth * (float(z_tile) / uGridSize.z), 1.0);
        return;
    #endif
    
    for (int i = 0; i < cluster.count; i++)
    {
        uint index = global_light_indices[cluster.offset + i];
        Light l = lights[index];
        vec3 diff = l.pos - Pos;
        vec3 dir = normalize(diff);

        float nbias = max(0.1 * (1.0 - dot(original_normal, dir)), 0.05);        
        float d2 = dot(diff, diff);


        vec3 lighting = calculate_light(N, view_dir, dir, albedo.rgb, roughness, metallic, l.color);
        lighting *= calculate_attenuation(d2, l.radius, l.intensity);

        vec3 opp = (Pos + original_normal * nbias) - l.pos;
        float current_depth = length(opp) / 100;

        opp = vec3(opp.x, opp.z, -opp.y);
        float bias = max(0.005 * (1.0 - dot(N, dir)), 0.0005);        

        float s = texture(uPLSCMA, vec4(opp, index), current_depth - bias).r;

        lighting *= s;


        
        acc += lighting;
        
    }
    for (int i = 0; i < directional_light_count; i++)
    {
        Light l = lights[light_indices[directional_light_offset + i]];
        vec3 lighting = calculate_light(N, view_dir, l.dir, albedo.rgb, roughness, metallic, l.color);
        
        float shadow = shadow_calculation(l.layer, FragPosLightSpace[l.layer], 0);
        lighting *= l.intensity;
        lighting *= shadow;
        acc += lighting;
    }
    for (int i = 0; i < spot_light_count; i++)
    {

        Light l = lights[light_indices[spot_light_offset + i]];
        vec3 diff = l.pos - Pos;
        vec3 dir = normalize(diff);
        float d2 = dot(diff, diff);


        vec3 lighting = calculate_light(N, view_dir, dir, albedo.rgb, roughness, metallic, l.color);
        float theta = dot(-dir, l.dir);

        float intensity = clamp((theta - l.outer_cos) * l.cos_diff, 0.0f, 1.0f);
        lighting /= max(d2, 0.001);
        lighting *= intensity;
        lighting *= l.intensity;
        acc += lighting;
    }
    vec3 direct_diffuse = albedo.rgb * acc;
    #ifdef DIRECT
        fragColor = vec4(direct_diffuse, albedo.a);
        return;
    #endif

    vec3 irradiance = vec3(0,0,0);
    float ff;
    if (Static == 0)
    {
        vec3 tex_local = vec3(0.0);
        float closest = 10000000;

        uint closest_pv = 0;
        for (int i = 0; i < 6; i++)
        {

            if (pvs[i].loaded == 0) continue;
            vec3 local = pos_to_pv_local(pvs[i], Pos);
            vec3 clamped_local = clamp(local, vec3(0.0), vec3(1.0));
            float dist = length(clamped_local - local);
            if (dist < closest)
            {
                closest_pv = i;
                closest = dist;
                tex_local = local;
            }
            if (dist <= 0.0f)
            {
                break;
            }
        }
    
        // pad out to dilate into empty space on top and bottom z to avoid leaking
        float full_z = uLPVDepthPadded * SH_MAX * LPV_SLOTS;
        float grid_size_div = uLPVDepthPadded / full_z;
        float z_offset = (uLPVDepthPadded * closest_pv * SH_MAX) / full_z;
        float local_z = ((tex_local.z + 1.0 / float(uLPVDepthPadded)) / (1.0 + 2.0 / float(uLPVDepthPadded))) / SH_MAX / LPV_SLOTS + z_offset;
        vec3 coord0 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 0);
        vec3 coord1 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 1);
        vec3 coord2 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 2);
        vec3 coord3 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 3);
        vec3 coord4 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 4);
        vec3 coord5 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 5);
        vec3 coord6 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 6);
        vec3 coord7 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 7);
        vec3 coord8 = vec3(tex_local.x, tex_local.y, local_z + grid_size_div * 8);
        vec3 p0 = texture(probe_tex, coord0).rgb;
        vec3 p1 = texture(probe_tex, coord1).rgb;
        vec3 p2 = texture(probe_tex, coord2).rgb;
        vec3 p3 = texture(probe_tex, coord3).rgb;
        vec3 p4 = texture(probe_tex, coord4).rgb;
        vec3 p5 = texture(probe_tex, coord5).rgb;
        vec3 p6 = texture(probe_tex, coord6).rgb;
        vec3 p7 = texture(probe_tex, coord7).rgb;
        vec3 p8 = texture(probe_tex, coord8).rgb;
    
    
        vec3 pc0 = p0 * 0.282095;
        vec3 pc1 = p1 * 0.488603 * N.y;
        vec3 pc2 = p2 * 0.488603 * N.z;
        vec3 pc3 = p3 * 0.488603 * N.x;
        vec3 pc4 = p4 * 1.092548 * N.x * N.y;
        vec3 pc5 = p5 * 1.092548 * N.y * N.z;
        vec3 pc6 = p6 * 0.315392 * (3 * (N.z * N.z) - 1);
        vec3 pc7 = p7 * 1.092548 * (N.x * N.z);
        vec3 pc8 = p8 * 0.546274 * (N.x * N.x - N.y * N.y);


        irradiance = max(pc0 + pc1 + pc2 + pc3 + pc4 + pc5 + pc6 + pc7 + pc8, vec3(0.0));
    }
    else
    {

        vec4 lm = texture(uLightmap, LMUV);
        vec4 dir_data = texture(uDirectionalmap, LMUV);
        
        float focus_factor = dir_data.a;

        vec3 dominant_dir = normalize(dir_data.xyz * 2.0f - 1.0f);

        float ndotl = max(0.0, dot(N, dominant_dir));

        float lam = dot(N, dominant_dir) * 0.5f + 0.5f;

        ff = (lam / max(focus_factor, 0.00001f));
        #ifdef LM_DIRECTIONAL
            fragColor = vec4(ff, ff, ff, 1.0f);
            return;
        #endif
        // ff = focus_factor;
        // ff = mix(1.0, ndotl, focus_factor);
        // ff = mix(1.0, lam, focus_factor);
        // fragColor = vec4(dominant_dir, 1.0);
        // return;
        // irradiance = lm.rgb * ff;
        irradiance = lm.rgb;
        // irradiance = lm.rgb;

        ao *= lm.a;

    }
    #ifdef AO
        fragColor = vec4(ao, ao, ao, 1.0f);
        return;
    #endif

    #ifdef LM_IRRADIANCE
        fragColor = vec4(irradiance, 1.0f);
        return;
    #endif

    const float PI = 3.14159265359;    
    float filtered_roughness = normal_filtering(N, roughness);    

    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo.rgb, metallic);
    
    vec3 kS = fresnel_schlick_roughness(cosTheta, albedo.rgb, filtered_roughness, metallic);
    vec3 kD_indirect = (vec3(1.0) - kS) * (1.0 - metallic);
    vec3 indirect_diffuse = kD_indirect * irradiance * albedo.rgb;
    #ifdef INDIRECT
        fragColor = vec4(indirect_diffuse, 1.0f);
        return;
    #endif

    const float MAX_REFLECTION_LOD = 4.0f;

    float normal_blend = filtered_roughness;
    vec3 parallax_N = normalize(mix(original_normal, N, clamp(normal_blend + 0.1, 0.2, 1.0)));

    float temp_lod = 0;


    vec3 rprobe_color = vec3(0.0);
    vec2 brdf = texture(uBRDFLUT, vec2(cosTheta, filtered_roughness)).rg;
    vec3 specular = vec3(0,0,0);
    float spec_occ = calculate_specular_occlusion(cosTheta, ao, filtered_roughness);
    if (probe_bake == 0)
    {
        rprobe_color = sample_weighted_rprobes(Pos, view_dir, N, filtered_roughness * MAX_REFLECTION_LOD);
        #ifdef RPROBE
            fragColor = vec4(rprobe_color, 1.0f);
            return;
        #endif
        
        specular = rprobe_color * (F0 * brdf.x + brdf.y);
        #ifdef SPECULAR
            fragColor = vec4(specular, 1.0f);
            return;
        #endif
    }
    else
    {
        rprobe_color = vec3(0,0,0);


        
    }
    vec3 final_indirect = (indirect_diffuse + specular) * ao; 

    vec3 ambient = vec3(0.005, 0.005, 0.005);

    vec3 ambient_light = ambient * albedo.rgb;
    float z_scale = z_tile / 24.0f;
    vec3 tiles = vec3(x_tile / 16.0f, y_tile /9.0f, depth / uFar);

    

    
    fragColor = vec4(direct_diffuse + final_indirect + ambient_light + mat.emissive.rgb * emissive * mat.emissive_strength, albedo.a);
    // fragColor = vec4(1,1,1,1);

}
