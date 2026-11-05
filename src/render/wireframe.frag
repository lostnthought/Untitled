
#version 430 core

layout(location = 0)out vec4 fragColor;
in vec3 vColor;
uniform vec3 color;
void main()
{


    
    // fragColor = vec4(1, 1, 1, 0);
    // fragColor = vec4(vec3(1, vColor.r, 1), 1.0);
    fragColor = vec4(vColor, 1.0);
}
