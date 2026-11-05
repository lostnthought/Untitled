#version 460

out vec4 fragColor;



in vec3 vPos;
uniform samplerCube uCubemap;
const int size = 128;
void main()
{


    
    vec3 pos = normalize(vPos);
    vec4 color = texture(uCubemap, pos);
    fragColor = vec4(color.rgb, 1);

}
