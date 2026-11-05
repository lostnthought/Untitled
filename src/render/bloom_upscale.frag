#version 460 core

uniform sampler2D srcTexture;
uniform float uFilterRadius; 

in vec2 vUV;
out vec4 fragColor;

void main() {

    float x = uFilterRadius;
    float y = uFilterRadius;

    vec3 a = texture(srcTexture, vec2(vUV.x - x, vUV.y + y)).rgb;
    vec3 b = texture(srcTexture, vec2(vUV.x, vUV.y + y)).rgb;
    vec3 c = texture(srcTexture, vec2(vUV.x + x, vUV.y + y)).rgb;

    vec3 d = texture(srcTexture, vec2(vUV.x - x, vUV.y)).rgb;
    vec3 e = texture(srcTexture, vec2(vUV.x, vUV.y)).rgb;
    vec3 f = texture(srcTexture, vec2(vUV.x + x, vUV.y)).rgb;

    vec3 g = texture(srcTexture, vec2(vUV.x - x, vUV.y - y)).rgb;
    vec3 h = texture(srcTexture, vec2(vUV.x, vUV.y - y)).rgb;
    vec3 i = texture(srcTexture, vec2(vUV.x + x, vUV.y - y)).rgb;

    vec3 upsample = e * 4.0;
    upsample += (b + d + f + h) * 2.0;
    upsample += (a + c + g + i);
    upsample *= 1.0 / 16.0;

    fragColor = vec4(upsample, 1.0);

}
