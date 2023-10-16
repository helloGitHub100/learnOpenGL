#version 330 core
layout (location = 0) in vec3 aPos;

out float lineTexCoords;

void main()
{
    lineTexCoords = aPos.x * 0.5 + 0.5;
    gl_Position =  vec4(aPos, 1.0);
}