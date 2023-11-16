#version 420 core

// shader outputs
layout (location = 0) out vec4 frag;

in vec2 TexCoords;
uniform sampler2D texture_diffuse1;

void main()
{
	vec4 color = texture(texture_diffuse1, TexCoords);
	// 如果allpha小于1，则不绘制该片元
	if(color.a < 1)
		discard;
	frag = color;
}