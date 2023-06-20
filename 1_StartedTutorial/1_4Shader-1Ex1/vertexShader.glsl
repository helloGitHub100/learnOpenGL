#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
out vec3 ourColor;
uniform vec3 posOffset;
out vec3 ourPos;

void main()
{
   gl_Position = vec4(aPos.x + posOffset.x, -aPos.y + posOffset.y, aPos.z + posOffset.z, 1.0);
   ourColor = aPos;
}