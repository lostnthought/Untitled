
#version 430 core



in vec2 vUV;
flat in vec4 vColor;
flat in uint vType;
in vec4 vFill;
flat in uint vState;
flat in uint vHovered;
in vec2 vShadowDir;
flat in uint vShadow;
uniform uint uShadowPass;
in vec2 vPos;
flat in uint vVisible;
flat in float vRadius;
flat in vec2 vCenter;
flat in vec2 vSize;
out vec4 fragColor;
flat in vec4 vContPos;
flat in vec4 vPadding;
flat in float vContRadius;
flat in vec2 vContSize;
flat in uint vContType;
flat in uint vMask;
flat in vec2 vContScale;
in vec4 vScreenPos;
in vec2 vDist;



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

    float d = 0.0f;
    float d2 = 0.0f;

    switch(vType)
    {

        case 0:
            d = SDF_rect(vPos, vCenter, vSize, vRadius);
            d2 = SDF_rect(vPos, vCenter, vSize * 2, vRadius);
            break;
        case 1:
            d = SDF_circle(vPos, vCenter, vRadius);
            d2 = SDF_circle(vPos, vCenter, 2.0f);
            break;
    }

    if (vMask == 1)
    {
        
        float d_mask = 0.0f;

        
        vec2 tr_pos = (vScreenPos.xy - vContPos.xy) / vContScale;
        switch(vContType)
        {

            // if we have a center that needs to be different than 0,0 then pass it here
            case 0:
                d_mask = SDF_rect(tr_pos, vCenter, vContSize, vContRadius);
                break;
            case 1:
                d_mask = SDF_circle(tr_pos, vCenter, vContRadius);
                break;
        }
        if (d_mask + vPadding.x > 0.0f)
        {
            discard;
        }
    }

    if (d > 0.0f) discard;
    if (vState == 3)
    {
        if (d > -0.1f)
        {
            
            fragColor = vec4(0,1,0,1.0);
            return;
        }
    }
    d = clamp(d, -1, 0);
    d += 1;
    d2 = clamp(d2, -1, 0);
    d2 += 1;
    
    vec3 color = mix(vColor.rgb, vec3(1, 1, 1), (d-d2) * (d-d2) * (d-d2));
    
    fragColor = vec4(vColor.rgb, 1.0);
}

