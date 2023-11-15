#version 420 core

// shader outputs
layout (location = 0) out vec4 frag;

in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

void main()
{
	frag = texture(texture_diffuse1, TexCoords);
}