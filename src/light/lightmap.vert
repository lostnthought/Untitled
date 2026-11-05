
#version 430 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aUV;
layout(location = 3) in vec2 aLightmapUV;
layout(location = 4) in int aMaterialID;
layout(location = 5) in int aTriID;


struct Mesh
{
    uint mesh_node_id;
    uint mesh_id;
    uint node_count;
    uint node_offset;
    uint vertex_offset;
    uint vertex_count;
    uint tri_offset;
    uint tri_count;
    uint bvh_index;
    uint is_static;
    uint pad0;
    uint pad1;

    mat4 global_matrix;
    mat4 inverse_global_matrix;

};

layout(std430, binding = 6) buffer MeshData {

    Mesh meshes[];

};


uniform int uMeshNode;

out vec3 WorldPos;
out vec3 WorldNormal;
out vec2 vUV;
out flat int TriID;

out flat int MeshNode;
out flat int MaterialID;


void main() {

    WorldPos = (meshes[uMeshNode].global_matrix * vec4(aPos, 1.0)).xyz;
    WorldNormal = mat3(transpose(meshes[uMeshNode].inverse_global_matrix)) * aNormal;

    vUV = aUV;
    vec2 clipSpaceUV = aLightmapUV * 2.0 - 1.0;

    MeshNode = uMeshNode;
    MaterialID = aMaterialID;
    
    TriID = aTriID;
    gl_Position = vec4(clipSpaceUV, 0.0, 1.0);
}
