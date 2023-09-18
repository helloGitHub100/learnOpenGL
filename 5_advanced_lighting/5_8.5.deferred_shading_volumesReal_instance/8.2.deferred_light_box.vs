#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aLightColor;
layout (location = 4) in mat4 aInstanceMatrix;

uniform mat4 projection;
uniform mat4 view;
out vec3 lightColor;

void main()
{
    lightColor = aLightColor;
    gl_Position = projection * view * aInstanceMatrix * vec4(aPos, 1.0);
}