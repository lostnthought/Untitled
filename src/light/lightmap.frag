#version 430 core

// gbuffer
layout(location = 0) out vec4 outWorldPosition;
layout(location = 1) out vec4 outWorldNormal;
layout(location = 2) out ivec3 outMaterialMeshTriID;

in vec3 WorldPos;
in vec3 WorldNormal;


// uvs are stored in pos and normal
in vec2 vUV;
in flat int MeshNode;
in flat int MaterialID;
in flat int TriID;

void main() {
    
    outWorldPosition = vec4(WorldPos, vUV.x);
    outWorldNormal = vec4((WorldNormal), vUV.y);
    outMaterialMeshTriID = ivec3(MaterialID, MeshNode, TriID);

}
