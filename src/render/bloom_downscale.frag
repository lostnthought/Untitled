#version 460 core

uniform sampler2D srcTexture;

in vec2 vUV;
out vec4 fragColor;

void main() {

    vec2 srcTexelSize = 1.0 / vec2(textureSize(srcTexture, 0));
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;


    vec3 a = texture(srcTexture, vec2(vUV.x - 2.0 * x, vUV.y + 2.0 * y)).rgb;
    vec3 b = texture(srcTexture, vec2(vUV.x, vUV.y + 2.0*y)).rgb;
    vec3 c = texture(srcTexture, vec2(vUV.x + 2.0 * x, vUV.y + 2.0 * y)).rgb;

    vec3 d = texture(srcTexture, vec2(vUV.x - 2.0 * x, vUV.y)).rgb;
    vec3 e = texture(srcTexture, vec2(vUV.x, vUV.y)).rgb;
    vec3 f = texture(srcTexture, vec2(vUV.x + 2.0 * x, vUV.y)).rgb;

    vec3 g = texture(srcTexture, vec2(vUV.x - 2.0 * x, vUV.y - 2.0 * y)).rgb;
    vec3 h = texture(srcTexture, vec2(vUV.x, vUV.y - 2.0*y)).rgb;
    vec3 i = texture(srcTexture, vec2(vUV.x + 2.0 * x, vUV.y - 2.0 * y)).rgb;

    vec3 j = texture(srcTexture, vec2(vUV.x - x, vUV.y + y)).rgb;
    vec3 k = texture(srcTexture, vec2(vUV.x + x, vUV.y + y)).rgb;
    vec3 l = texture(srcTexture, vec2(vUV.x - x, vUV.y - y)).rgb;
    vec3 m = texture(srcTexture, vec2(vUV.x + x, vUV.y - y)).rgb;

    vec3 downsample = e * 0.125;
    downsample += (a + c + g + i) * 0.03125;
    downsample += (b + d + f + h) * 0.0625;
    downsample += (j + k + l + m) * 0.125;

    fragColor = vec4(downsample, 1.0);
}
