#version 420 core

// shader inputs
layout (location = 0) in vec3 position;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;

// mvp matrix
uniform mat4 mvp;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = mvp * vec4(position, 1.0f);
}