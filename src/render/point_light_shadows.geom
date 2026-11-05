
#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices=18) out;



uniform mat4 vp[6];
uniform uint uLayerOffset;

out vec3 FragWorldPos;
flat out uint vMat;
out vec2 vUV;
in VS_OUT
{
    uint Mat;
    vec2 UV;
    
} gs_in[];

void main()
{
    
    for (int i = 0; i < 6; ++i)
    {
        gl_Layer = int(uLayerOffset) + i;
        for (int j = 0; j < 3; ++j)
        {
            vec4 pos = gl_in[j].gl_Position;
            gl_Position = vp[i] * pos;
            FragWorldPos = pos.xyz;
            vUV = gs_in[0].UV;
            vMat = gs_in[0].Mat;
            EmitVertex();
            
        }
        
        EndPrimitive();
    }

    
}


