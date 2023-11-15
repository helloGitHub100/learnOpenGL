#version 420 core

// shader inputs
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in mat4 aInstanceMatrix;

out vec2 TexCoords;

// projection * view matrix
uniform mat4 pv;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = pv * aInstanceMatrix * vec4(position, 1.0f);
}