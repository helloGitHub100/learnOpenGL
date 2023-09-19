#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 alightPos;
layout (location = 2) in vec3 aColor;
layout (location = 3) in mat4 aInstanceMatrix;

uniform mat4 projection;
uniform mat4 view;

out vec3 lightPos;
out vec3 Color;

void main()
{
    lightPos = alightPos;
    Color = aColor;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
}