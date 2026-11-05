
#version 430 core

uniform sampler2D uFontAtlas;

in vec2 vUV;
in vec2 vDim;
in vec4 vColor;
in vec4 vFill;
flat in uint vState;
flat in uint vVisible;
in vec2 vShadowDir;
flat in uint vShadow;
flat in uint vHovered;
out vec4 fragColor;
uniform uint uShadowPass;
flat in vec2 vCenter;
flat in vec4 vContPos;
flat in vec4 vPadding;
flat in float vContRadius;
flat in vec2 vContSize;
flat in uint vContType;
flat in uint vMask;
flat in vec2 vContScale;
in vec4 vScreenPos;

in vec2 vPos;


float SDF_circle(vec2 p, vec2 center, float radius) {
    return length(p - center) - radius;
}


float SDF_rect(vec2 p, vec2 center, vec2 size, float r) {
    vec2 q = abs(p - center) - size * 0.5 + vec2(r);
    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;
}

float median(float r, float g, float b) {
    return max(min(r, g), min(max(r, g), b));
}

void main() {
    if (vVisible == 0) discard;
    vec3 sdf = texture(uFontAtlas, vUV).rgb;

    float sd = (median(sdf.r, sdf.g, sdf.b) - 0.5);

    float screen_px_distance = 0.015;
    float alpha = smoothstep(-screen_px_distance, screen_px_distance, sd);

    if (vMask == 1)
    {
        
        float d_mask = 0.0f;

        
        vec2 tr_pos = (vScreenPos.xy - vContPos.xy) / vContScale;
        switch(vContType)
        {

            // if we have a center that needs to be different than 0,0 then pass it here
            case 0:
                d_mask = SDF_rect(tr_pos, vec2(0,0), vContSize, vContRadius);
                break;
            case 1:
                d_mask = SDF_circle(tr_pos, vec2(0,0), vContRadius);
                break;
        }
        if (d_mask + vPadding.x > 0.0f)
        {
            discard;
        }
    }
    if (uShadowPass == 1 && vShadow == 1)
    {

        float dist = length(vPos - vCenter);
        float a = clamp(alpha, 0.3, 1.0);
        fragColor = vec4(0,0,0, a);

        return;
    }
    else if (uShadowPass == 1 && vShadow == 0)
    {
        discard;

    }
    
    if (alpha < 0.1)
    {
        if (vState == 3)
        {
            fragColor = vec4(0,1,0,1);
            return;
        }
        discard;
    }
    vec3 color = vColor.rgb;

    fragColor = vec4(color.rgb, clamp(alpha, 0.3, 1.0));
}

