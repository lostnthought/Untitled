
#version 330 core

in vec2 uv;
out vec4 fragColor;

uniform sampler2D scene;
// uniform sampler2D bloom;
// uniform sampler2D bloom2;

void main()
{

    // vec3 color = texture(scene, uv).rgb;
    // vec3 bcolor = texture(bloom, uv).rgb;
    // vec3 b2color = texture(bloom2, uv).rgb;
    // fragColor = texture(scene, uv) +texture(bloom, uv);
    // fragColor = vec4(texture(scene, uv).rgb + texture(bloom, uv).rgb * 0.5 + texture(bloom2, uv).rgb * 0.8, 1.0) ;
    // fragColor = vec4(b2color.rgb, 1.0);
    fragColor = vec4(texture(scene, uv).rgb, 0.0);
}
