#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    int vertexID;
} vs_out;

void main()
{
    vs_out.vertexID = gl_VertexID;
    gl_Position = vec4(aPos, 1.0); 
}