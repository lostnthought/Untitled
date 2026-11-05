
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

uniform mat4 projection;
uniform mat4 view;

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
struct Transform
{
    uint mn_id;
    uint command_id;
    uvec2 directional_lights;
    uvec2 spot_lights;
    uint skin_offset;
    uint material;
    mat4 mat;
};

layout(std430, binding = 7) buffer TransformData {

    Transform trs[];

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


#define MAX_INSTANCES 1024
layout(std430, binding = 4) buffer VisibleData {

    uint visible_indices[];

};

layout(std430, binding = 11) buffer JointData {

    mat4 joint_mats[];

};
out vec3 FragWorldPos;
uniform uint uOffset;
uniform uint uDynamicPass;
out vec2 UV;
flat out uint Mat;
uniform uint uStartArr;
uniform uint uStartEle;

void main()
{
    
    vec3 pos = aPosition;

    uint off = 0;
    uint index_offset = uStartArr + gl_DrawID;
    if (uDynamicPass == 1)
    {
        
        DrawArraysIndirectCommand com = draw_arrays_commands[uOffset-1];
        off = com.instanceCount + com.baseInstance;
        index_offset = uStartEle + gl_DrawID;
        index_offset += uOffset;
    }
    uint visible_offset = index_offset * MAX_INSTANCES;
    Transform tr = trs[visible_indices[visible_offset + gl_InstanceID]];
    mat4 skinMat = 
        joint_mats[aJointIndices.x] * aWeights.x +
        joint_mats[aJointIndices.y] * aWeights.y +
        joint_mats[aJointIndices.z] * aWeights.z +
        joint_mats[aJointIndices.w] * aWeights.w;
    vec4 skinned_pos = skinMat * vec4(pos, 1.0);
    vec4 worldPos = tr.mat * skinned_pos;
    vec4 clipPos = projection * view * tr.mat * skinned_pos;
    MeshNode mn = mns[tr.mn_id];
    Mat = tr.material;
    UV = aUV;
    FragWorldPos = worldPos.xyz;
    gl_Position = clipPos; 
}
