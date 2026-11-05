#version 330 core
out float FragColor;
in vec2 vUV;

uniform sampler2D uSSAOInput;
uniform sampler2D uDepth;

void main() {
    vec2 texelSize = 1.0 / vec2(textureSize(uSSAOInput, 0));


    float result = 0.0;
    float weightSum = 0.0;

    float centerDepth = texture(uDepth, vUV).r;

    for (int x = -2; x <= 2; ++x) {
        for (int y = -2; y <= 2; ++y) {

            vec2 offset = vec2(float(x), float(y)) * texelSize;
            vec2 sampleUV = vUV + offset;
    
            float sampleAO = texture(uSSAOInput, sampleUV).r;
            float sampleDepth = texture(uDepth, sampleUV).r;
    
            float depthDiff = abs(centerDepth - sampleDepth);
    
            float weight = exp(-depthDiff * 1000.0);
    
            result += sampleAO * weight;
            weightSum += weight;
        }
    }

    FragColor = result / weightSum;    
    
}
