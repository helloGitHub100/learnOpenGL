#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;

out VS_OUT {
    vec3 normal;
} vs_out;

void main()
{
    vs_out.normal = vec3(vec4(aNormal, 0.0));
    gl_Position = vec4(aPos, 1.0); 
}