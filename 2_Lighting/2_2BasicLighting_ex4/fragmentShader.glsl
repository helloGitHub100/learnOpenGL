#version 330 core
out vec4 FragColor;



in vec3 result;

void main()
{
    FragColor = vec4(result, 1.0);
}