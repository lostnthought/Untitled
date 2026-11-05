#version 430 core
uniform sampler2D uDepth;

in vec2 vUV;
out float outDepth;

void main() {
    vec4 depths = textureGather(uDepth, vUV, 0);
    
    outDepth = max(max(depths.x, depths.y), max(depths.z, depths.w));
}
