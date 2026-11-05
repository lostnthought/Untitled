
#version 430 core
uniform sampler2D uScene;

in vec2 vUV;
out vec4 fragColor;

void main() {
    
    vec4 sc = texture(uScene, vUV);
    float brightness = max(max(sc.r, sc.g), sc.b);
    float threshold = 1.0;
    float knee = 0.2;


    float rq = clamp(brightness - threshold + knee, 0.0, 2.0 * knee);
    rq = (rq * rq) / (4.0 * knee);

    float intensity = max(rq, brightness - threshold);

    float multiplier = intensity / max(brightness, 0.0001); 

    fragColor = vec4(sc.rgb * multiplier, 1.0);
    
    
}

