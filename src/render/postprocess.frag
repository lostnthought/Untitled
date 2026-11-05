
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
uniform sampler2D uScene;
uniform sampler2D uHistory;
uniform sampler2D uFBM;

uniform sampler2D uDPP;
uniform sampler2D uText;
uniform sampler2D uSSR;
uniform sampler2D uCurrentSSR;
uniform sampler2D gVel;
uniform sampler2D uBokeh;
uniform sampler2D uCOC;
uniform sampler2D uBloom;
uniform sampler2D uLM;
uniform sampler3D uLUT;
uniform uint uMBLUR;
uniform uint uDOF;
uniform float uPhysicsFramerate;
uniform float uRenderFramerate;
uniform uint uWireframeToggled;


layout(std140, binding = 1) uniform RenderingData {
    ivec4 resolution;
};

uniform sampler3D uColorLUT;

vec3 applyLUT(vec3 color) {
    float lutSize = 33.0;
    vec3 scale = vec3((lutSize - 1.0) / lutSize);
    vec3 offset = vec3(1.0 / (2.0 * lutSize));
    
    vec3 lutUVW = color * scale + offset;
    
    return texture(uLUT, lutUVW).rgb;
}


float calculateExposure(vec3 avgColor){
    float lum = dot(avgColor, vec3(0.2126, 0.7152, 0.0722));
    
    lum = max(lum, 0.0001f); 

    float middleGray = 0.18f;
    float acesBoost = 0.01f;
    
    float minExp = 0.1f;
    float maxExp = 2.0f;

    float targetExposure = (middleGray / lum);

    return clamp(targetExposure, minExp, maxExp);
}

vec3 ACESFilm(vec3 x) {
    float a = 2.51f;
    float b = 0.03f;
    float c = 2.43f;
    float d = 0.59f;
    float e = 0.14f;
    return clamp((x*(a*x+b))/(x*(c*x+d)+e), 0.0, 1.0);
}

vec3 vignette(vec3 color, float u, float v){
  vec2 nvuv = vUV - 0.5;
  float nvdist = length(nvuv);
  float vignette = smoothstep(u, v, nvdist);
  color.rgb *= vignette;

  return color;
}


const mat3 srgb_to_ap1 = mat3
(
    0.613097, 0.070194, 0.020585, 
    0.339523, 0.916394, 0.109572, 
    0.047379, 0.013412, 0.869843  
);

const mat3 ap1_to_srgb = mat3
(
    1.704859, -0.130258, -0.024003, 
   -0.621714,  1.140733, -0.128963, 
   -0.083145, -0.010475,  1.152966  
);
vec3 linear_to_acescct(vec3 linearRGB)
{

    vec3 ap1 = srgb_to_ap1 * linearRGB;

    vec3 log_low = 10.5402377416545 * ap1 + 0.0729055341958355;
    vec3 log_high = (log2(max(ap1, vec3(1e-6))) + 9.72) / 17.52;

    vec3 use_low = vec3(lessThanEqual(ap1, vec3(0.0078125)));
    
    return mix(log_high, log_low, step(ap1, vec3(0.0078125)));
}

vec3 acescct_to_linear(vec3 graded)
{

    vec3 linear_t = (graded - 0.0729055341958355) / 10.5402377416545;
    vec3 log_curve= exp2(graded * 17.52 - 9.72);

    vec3 use_low = vec3(lessThanEqual(graded, vec3(0.155228)));
    vec3 ap1 = mix(log_curve, linear_t, step(graded, vec3(0.155251141552511)));
    // vec3 ap1 = mix(log_curve, linear_t, use_low);
    return ap1_to_srgb * ap1;
}







float linear_to_acescct_channel(float x)
{
    if (x <= 0.0078125)
    {
        return (10.5402377416545 * x + 0.0729055341958355);
    }
    else
    {
        return ((log2(x) + 9.72) / 17.52);
    }
}
float acescct_to_linear_channel(float x)
{
    if (x <= 0.155228)
    {
        return (x - 0.072905534195) / 10.5402377416;
    }
    else
    {
        return pow(2.0, x * 17.52 - 9.72);
    }
}

vec3 processLUT(vec3 color)
{
    

    vec3 aces_cct = linear_to_acescct(color);

    vec3 graded = applyLUT(aces_cct);



    return acescct_to_linear(graded);
    
}

    





uniform float uTime;
out vec4 fragColor;

const int samples = 16;


uniform float uFocalLength;   
uniform float uFStop;          
uniform float uSensorHeight; 
uniform float uDOFMaxBlurRadius;
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

vec3 get_view_pos_from_depth(vec2 uv, float depth, mat4 invProjMatrix) {
    vec4 ndc = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
    
    vec4 viewSpacePos = invProjMatrix * ndc;
    
    return viewSpacePos.xyz / viewSpacePos.w;
}

void main()
{
    vec2 uv = vUV;
    vec4 color = vec4(0.0, 0.0, 0.0, 1.0);

    vec4 sc = texture(uScene, uv);
    vec4 text = texture(uText, uv);

    #ifdef DEPTH
        float s = texture(uDPP, uv).r;
        fragColor = vec4(s, s, s, 1.0);
        return;
    #endif
        
    #ifdef VEL
        fragColor = vec4(texture(gVel, uv).rg, 0.0, 1.0);
        return;
    #endif

    #ifdef BOKEH
        vec4 bokeh = texture(uBokeh, uv);
        fragColor = vec4(bokeh.rgb, 1.0);
        return;
    #endif
    #ifdef LM
        vec4 lm = texture(uLM, uv);
        fragColor = vec4(lm.rgb, 1.0);
        return;
    #endif


    
    float framerate_scale = uRenderFramerate / uPhysicsFramerate;
    #ifdef MBLUR
        
        vec2 velocity = texture(gVel, uv).rg;

        velocity *= 0.7; 
        
        vec2 texelStep = velocity / float(samples - 1);

        vec2 currentTexCoords = uv - (velocity * 0.7);

        for (int i = 1; i < samples; ++i) {
            currentTexCoords += texelStep;
            sc += textureLod(uSSR, currentTexCoords, 0);
        }

        sc /= samples;
    #endif

    #ifdef DOF
        
        float rawDepth = texture(uDPP, uv).r;

        vec3 viewPos = get_view_pos_from_depth(uv, rawDepth, unjittered_inv_projection);
        float pixelDist = -viewPos.z; 

        float coeff = (uFocalLength * uFocalLength) / (uFStop * (focal_distance - uFocalLength));
        float cocMeters = coeff * ((pixelDist - focal_distance) / pixelDist);
        float cocUV = cocMeters / uSensorHeight;    


        float finalCoC = clamp(cocUV, -uDOFMaxBlurRadius, uDOFMaxBlurRadius);

        float blendFactor = abs(finalCoC) / uDOFMaxBlurRadius;

        blendFactor = smoothstep(0.05, 0.9, blendFactor);    

        vec4 bokeh = texture(uBokeh, uv);

        sc.rgb = mix(sc.rgb, bokeh.rgb, blendFactor);

    #endif


        
        color.rgb = mix(sc.rgb, text.rgb, text.a);


    #ifdef BLOOM
        vec4 bloom = texture(uBloom, uv);
        vec3 halationSample = texture(uBloom, uv + vec2(0.003, 0.0)).rgb;
        vec3 halationColor = vec3(halationSample.r * 1.5, halationSample.g * 0.2, 0.0);
    
        color.rgb += bloom.rgb * 0.3f; 
    #endif
    vec2 st = gl_FragCoord.xy / resolution.xy;

    #ifdef NOISE

        float noise = fract(sin(dot(st.xy + vec2(uTime * 0.001, 0), vec2(12.9898, 78.233))) * 43758.5453123);
        vec3 nc = color.rgb * noise;
        color.rgb = mix(color.rgb, nc, 0.25);

    #endif


    
    
    int lastLOD = textureQueryLevels(uScene) - 1;
    vec3 avgColor = textureLod(uScene, vec2(0.5, 0.5), lastLOD).rgb;

    float autoExposure = calculateExposure(avgColor);

    vec3 aces = color.rgb;

    #ifdef ACES
        aces = ACESFilm(color.rgb);
    #endif
    #ifdef LUT
        aces = processLUT(max(color.rgb, 0.00001));
    #endif
    float gamma = 2.2;

    aces.rgb = pow(aces, vec3(1.0/gamma));
    fragColor = vec4(aces.rgb, 1.0);
}
