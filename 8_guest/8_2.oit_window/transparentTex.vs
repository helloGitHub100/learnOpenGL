#version 420 core

// shader inputs
layout (location = 0) in vec3 position;
layout (location = 1) in vec2 aTexCoords;

out vec2 TexCoords;

// model * view * projection matrix
uniform mat4 mvp;

void main()
{
	TexCoords = aTexCoords;
	gl_Position = mvp * vec4(position, 1.0f);
}