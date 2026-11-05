
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

layout(std140, binding = 6) uniform ProbeVolumeData{
    OBB pvs[6];
};


layout(std430, binding = 3) buffer LightmapData {

    vec4 uvs[];

};
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

layout(std430, binding = 11) buffer JointData {

    mat4 joint_mats[];

};


out vec3 ViewPos;
out vec3 Normal;
out vec3 Tangent;
out float TangentSign;
out vec3 Pos;
out vec2 UV;
out vec2 LMUV;
out vec3 Color;
out vec4 FragPosLightSpace[4];
out mat3 TBN;
out vec3 ViewDirTanSpace;
flat out ivec2 PointLight;
flat out uvec2 DirectionalLight;
flat out uvec2 SpotLight;
flat out uint Instance;
flat out uint Mat;
flat out uint Static;

uniform uint uOffset;
uniform uint uDynamicPass;

uniform vec3 probe_origin; 
uniform float probe_spacing;
uniform ivec3 probe_grid_size;

uniform uint uStartArr;
uniform uint uStartEle;
uniform uint uLPVDepthPadded;

uniform sampler3D probe_tex;


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
    
    vec3 pos = aPosition;
    PointLight = aPointLight;
    UV = aUV;

    
    mat4 skinMat = 
        joint_mats[aJointIndices.x] * aWeights.x +
        joint_mats[aJointIndices.y] * aWeights.y +
        joint_mats[aJointIndices.z] * aWeights.z +
        joint_mats[aJointIndices.w] * aWeights.w;
    // mat4 skinMat = mat4(1.0);
    uint off = 0;
    uint index_offset = uStartArr + gl_DrawID;
    if (uDynamicPass == 1)
    {
        
        index_offset = uStartEle + gl_DrawID;
        index_offset += uOffset;

    }
    uint visible_offset = index_offset * MAX_INSTANCES;

    // if we have problems later with sorting etc it is probably due to commands inserting in a different order than we have sorting based on shaders 
    Transform tr = trs[visible_indices[visible_offset + gl_InstanceID]];

    uint offset = MAX_INSTANCES * tr.command_id;
    mat4 model = tr.mat;
    vec4 skinned_pos = skinMat * vec4(pos, 1.0);
    vec4 world_pos = model * skinned_pos;
    MeshNode mn = mns[tr.mn_id];
    uint lm_offset = mn.lm_offset;
    uint start_index = mn.first;
    uint index_count = mn.count;

    
    uint lm_idx = lm_offset + (index_count * gl_InstanceID) + gl_VertexID - start_index;
    LMUV = uvs[lm_idx].xy;

    
    Mat = tr.material;
    uint skin_offset = tr.skin_offset;
    DirectionalLight = tr.directional_lights;
    SpotLight = tr.spot_lights;
    Static = mn.static_mesh;

    

    mat3 normalMatrix = transpose(inverse(mat3(model) * mat3(skinMat)));
    Normal = normalize(normalMatrix * aNormal);

    vec3 tangent;
    float tangent_sign;
    vec3 up;
    // if (aTangent.w == 2)
    // {
        
    //     if (abs(Normal.z) > 0.9f)
    //     {
    //         up = vec3(0,0,1);
    //     }
    //     else
    //     {
    //         up = vec3(1,0,0);
    //     }
    //     tangent = normalize(cross(up, Normal));
    //     tangent_sign = 1;
    // }
    // else
    // {
        
        tangent = normalize(mat3(model) * aTangent.xyz);
        tangent_sign = sign(aTangent.w);
    // }
    
    vec3 T = normalize(tangent - dot(tangent, Normal) * Normal);
    vec3 B = cross(Normal, tangent) * tangent_sign; 

    Tangent = tangent;
    TangentSign = tangent_sign;
    TBN = mat3(T, B, Normal); 


    

    
    vec3 n = Normal;
    

    vec4 mpos = world_pos;
    Pos = mpos.xyz;

    ViewDirTanSpace = transpose(TBN) * (cam_pos.xyz - mpos.xyz);
    
    ViewPos = (view * model * skinned_pos).xyz;


    
    
    gl_Position = projection * view * model * skinned_pos;        

    
    vec3 worldPos =
        (model * vec4(pos,1)).xyz + n * 0.1f;

    for (int i = 0; i < 4; i++)
    {
        FragPosLightSpace[i] = light_proj[i] * light_view[i] * vec4(worldPos,1);    
        
    }


}
