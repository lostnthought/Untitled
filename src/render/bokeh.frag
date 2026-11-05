#version 430 core


in vec2 vUV;
out vec4 fragColor;

const vec2 hex_taps[19] = vec2[](
    vec2( 0.00,  0.00),
    vec2( 0.50,  0.86), vec2( 1.00,  0.00), vec2( 0.50, -0.86), 
    vec2(-0.50, -0.86), vec2(-1.00,  0.00), vec2(-0.50,  0.86),
    vec2( 1.00,  1.73), vec2( 2.00,  0.00), vec2( 1.00, -1.73), 
    vec2(-1.00, -1.73), vec2(-2.00,  0.00), vec2(-1.00,  1.73),
    vec2( 1.50,  0.86), vec2( 1.50, -0.86), vec2( 0.00, -1.73), 
    vec2(-1.50, -0.86), vec2(-1.50,  0.86), vec2( 0.00,  1.73)
);

uniform sampler2D uCOC;          
uniform vec2 uTexelSize;         

void main() {
    vec2 uv = vUV;
    vec4 center = texture(uCOC, uv);
    float centerCoC = center.a;
    
    const float max_blur_pixels = 20.0; 
    float radius = abs(centerCoC) * max_blur_pixels;

    vec3 total_color = vec3(0.0);
    float total_weight = 0.0;

    for (int i = 0; i < 19; i++) {

        vec2 tap_offset = hex_taps[i] * 0.5 * radius * uTexelSize;
        vec2 sampleUV = uv + tap_offset;
        
        vec4 tap = texture(uCOC, sampleUV);
        float tapCoC = tap.a;
        float sample_dist = length(hex_taps[i] * 0.5 * radius);

        float weight = 1.0;
        if (tapCoC > centerCoC) {
            float tapRadius = abs(tapCoC) * max_blur_pixels;
            weight = clamp(tapRadius - sample_dist + 1.0, 0.0, 1.0);
        }

        float r = texture(uCOC, uv + tap_offset).r;
        float g = texture(uCOC, uv + tap_offset * 2.0).g;
        float b = texture(uCOC, uv + tap_offset * 1.5).b;
        
        vec3 tap_color = vec3(r, g, b);

        total_color += tap_color * weight;
        total_weight += weight;
    }

    fragColor = vec4(total_color / max(total_weight, 0.0001), 1.0);
}
