
#version 430 core


struct TextBox {
    
    uint visible;
    uint hovered;
    uint shadow;
    float shadow_dist;
    uint container_id;
    uint mask;
    vec2 shadow_dir;
    uint state;
    uint pad0;
    uint pad1;
    uint pad2;
    vec4 color;
    vec4 fill;
    mat4 model;
    
};


layout(std430, binding = 8) buffer TextBoxData{
    
    TextBox boxes[];
    
};

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



layout(location = 0) in vec2 aPosition;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec2 aCenter;
layout(location = 3) in vec4 aColor;
layout(location = 4) in vec4 aFill;
layout(location = 5) in uint aState;
layout(location = 6) in uint aUID;

out vec2 vUV;
out vec4 vColor;
out vec4 vFill;
flat out uint vState;
flat out uint vHovered;
out vec2 vShadowDir;
flat out uint vShadow;
uniform uint uShadowPass;
flat out vec2 vCenter;
out vec2 vPos;
flat out uint vVisible;
out vec4 vScreenPos;
flat out vec4 vContPos;
flat out vec2 vContScale;
flat out vec2 vContSize;
flat out uint vContType;
flat out uint vMask;
flat out vec4 vPadding;
flat out float vContRadius;

uniform mat4 uOrtho;

void main() {
    TextBox b = boxes[aUID];
    vColor = aColor;
    vState = b.state;
    vFill = aFill;
    vHovered = b.hovered;
    vShadowDir = b.shadow_dir;
    vShadow = b.shadow;
    vCenter = aCenter;

    vVisible = b.visible;
    vec2 pos = aPosition;
    vec2 uv = vec2(aUV.xy);
    vUV = clamp(uv, 0.0, 1.0);
    vPos = pos;

    vScreenPos = b.model * vec4(pos, 0.0, 1.0);
    // minor issue here might be if container id is stale and points to garbage but we only check for -1
    if (b.container_id != -1 && b.mask == 1)
    {
        
        UIElement c = elements[b.container_id];
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

    if (uShadowPass == 1)
    {
        pos += b.shadow_dir * b.shadow_dist;
    }
    gl_Position = uOrtho * b.model * vec4(pos, 0.0, 1.0);
}

