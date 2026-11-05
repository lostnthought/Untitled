
#version 460
layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aTangent;
layout(location = 3) in ivec4 aJointIndices;
layout(location = 4) in vec4 aWeights;
layout(location = 5) in vec2 aUV;
layout(location = 6) in mat4 aInstanceMatrix;
layout(location = 10) in ivec2 aPointLight;
layout(location = 11) in ivec2 aDirectionalLight;
layout(location = 12) in ivec2 aSpotLight;
layout(location = 13) in int aSkinOffset;


struct DrawArraysIndirectCommand {
    uint count;
    uint instanceCount;
    uint first;
    uint baseInstance;
    uint instanceOffset; // the amount of instances pushed this frame
    uint pad0;
};


layout(std430, binding = 5) buffer DrawArraysData {

    DrawArraysIndirectCommand draw_arrays_commands[];
};
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
struct Transform
{
    uint mn_id;
    uint command_id;
    uvec2 directional_lights;
    uvec2 spot_lights;
    uint skin_offset;
    uint pad0;
    mat4 mat;
};

layout(std430, binding = 7) buffer TransformData {

    Transform trs[];

};
layout(std430, binding = 9) buffer PrevTransformData {

    Transform prev_trs[];

};
layout(std430, binding = 11) buffer JointData {

    mat4 joint_mats[];

};


out vec3 vWorldPos;
uniform uint uOffset;
uniform uint uDynamicPass;
out vec3 ViewPos;

out vec3 Normal;
out vec3 ViewDirTanSpace;
out mat3 TBN;
flat out uint Mat;
out vec2 UV;
out vec4 ClipPos;
out vec4 PrevClipPos;

void main()
{
    
    vec3 pos = aPosition;
    UV = aUV; 

    uint off = 0;
    if (uDynamicPass == 1)
    {
        
        DrawArraysIndirectCommand com = draw_arrays_commands[uOffset-1];
        off = com.instanceCount + com.baseInstance;
    }
    Transform tr = trs[off + gl_BaseInstance + gl_InstanceID];
    Transform prev_tr = prev_trs[off + gl_BaseInstance + gl_InstanceID];
    MeshNode mn = mns[tr.mn_id];
    Mat = mn.material;

    mat4 skinMat = 
        joint_mats[aJointIndices.x] * aWeights.x +
        joint_mats[aJointIndices.y] * aWeights.y +
        joint_mats[aJointIndices.z] * aWeights.z +
        joint_mats[aJointIndices.w] * aWeights.w;

    // mat4 skinMat = mat4(1.0);

    mat4 model = tr.mat;
    vec4 skinned_pos = skinMat * vec4(pos, 1.0);


    mat4 view_model = view * model;
    mat3 normalMatrix = transpose(inverse(mat3(view_model) * mat3(skinMat)));
    Normal = normalize(normalMatrix * aNormal);

    vec3 tangent;
    int tangent_sign;
    vec3 up;
    if (aTangent.w == 2)
    {
        
        if (abs(Normal.z) > 0.9f)
        {
            up = vec3(0,0,1);
        }
        else
        {
            up = vec3(1,0,0);
        }
        tangent = normalize(cross(up, Normal));
        tangent_sign = 1;
    }
    else
    {
        
        tangent = normalize(mat3(model) * aTangent.xyz);
        tangent_sign = int(aTangent.w);
    }
    
    vec3 T = (tangent - dot(tangent, aNormal) * aNormal);
    vec3 B = cross(aNormal, tangent) * tangent_sign; 
    T = normalize(vec3(view_model * vec4(T, 0.0)));
    B = normalize(vec3(view_model * vec4(B, 0.0)));
    vec3 N = normalize(vec3(view_model * vec4(aNormal, 0.0)));

    TBN = mat3(T, B, N); 

    vec3 reg_Normal = normalize(mat3(inv_view) * Normal);
    vec3 reg_T = normalize((tangent - dot(tangent, reg_Normal) * reg_Normal));
    vec3 reg_B = normalize(cross(reg_Normal, tangent) * tangent_sign);
    
    mat3 regular_TBN = mat3(reg_T, reg_B, reg_Normal);
    
    vec4 wp = model * vec4(pos, 1.0);

    ViewPos = (view * model * vec4(pos, 1.0)).xyz;
    ViewDirTanSpace = transpose(regular_TBN) * (cam_pos.xyz - wp.xyz);

    
    PrevClipPos = prev_unjittered_projection * prev_view * prev_tr.mat * skinned_pos;
    ClipPos = unjittered_projection * view * model * skinned_pos;

    
    gl_Position = projection * view * model * skinned_pos; 
}
