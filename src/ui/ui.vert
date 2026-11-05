#version 430 core


struct UIElement {
    
    uint visible;
    uint hovered;
    uint shadow;
    float shadow_dist;
    vec2 pos;
    vec2 size;
    vec2 shadow_dir;
    uint state;
    uint type;
    vec4 color;
    vec4 fill;
    vec4 padding;
    vec2 translation;
    vec2 scale;
    float radius;
    uint masked;
    uint container_id;
    uint pad0;
    mat4 model;

   
};


layout(std430, binding = 9) buffer UIData {
    
    UIElement elements[];
    
};



layout(location = 0) in vec2 aPos;
layout(location = 1) in uint aUID;

out vec2 vUV;
flat out vec4 vColor;
flat out uint vType;
out vec4 vFill;
flat out uint vState;
flat out uint vHovered;
out vec2 vShadowDir;
flat out uint vShadow;
uniform uint uShadowPass;
out vec2 vPos;
flat out uint vVisible;
flat out vec2 vSize;
flat out float vRadius;
flat out vec2 vCenter;
uniform mat4 uOrtho;
flat out vec4 vPadding;
flat out float vContRadius;
flat out vec2 vContSize;
flat out uint vContType;
flat out uint vMask;
out vec4 vScreenPos;
flat out vec4 vContPos;
flat out vec2 vContScale;

void main() {
    
    UIElement e = elements[aUID];
    
    vColor = e.color;
    vFill = e.fill;
    vHovered = e.hovered;
    vState = e.state;
    vType = e.type;
    vShadowDir = e.shadow_dir;
    vShadow = e.shadow;
    vVisible = e.visible;
    vRadius = e.radius;
    vSize = e.size;
    vCenter = e.pos;

    vec2 pos = aPos;
    vPos = pos;

    vScreenPos = e.model * vec4(pos, 0.0, 1.0);
    // minor issue here might be if container id is stale and points to garbage but we only check for -1
    if (e.container_id != -1 && e.masked == 1)
    {
        
        UIElement c = elements[e.container_id];
        vContPos = c.model * vec4(c.pos, 0.0, 1.0);
        vPadding = c.padding;
        vContRadius = c.radius;
        vContSize = c.size;
        vContType = c.type;
        vMask = 1;
        vContScale = c.scale;
    }
    else
    {
        vMask = 0;
    }


    

    gl_Position = uOrtho * e.model * vec4(pos, 0.0, 1.0);
    
}
