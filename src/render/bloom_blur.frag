
#version 430 core

in vec2 vUV;
out vec4 fragColor;

uniform sampler2D uBloom;
uniform vec2 uDirection; 
uniform vec2 uTexel;

const float weights[5] = float[](0.227027f, 0.1945946f, 0.1216216f, 0.054054f, 0.016216f);
void main() {
    vec3 result = vec3(0.0);

    for (int i = -4; i <= 4; ++i) {
        vec2 uv = vUV + uDirection * uTexel * float(i);
        result += texture(uBloom, uv).rgb * weights[abs(i)];
    }

    fragColor = vec4(result, 1.0);
}
