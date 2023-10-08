#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

out vec2 TexCoords;

void main()
{
    TexCoords = aTexCoords;
    gl_Position = projection * view * model * vec4(aPos, 1.0);
    //相机始终保持在球体中心
    /*mat4 viewTemp = mat4(mat3(view));
	vec4 posTemp = projection * viewTemp * vec4(aPos, 1.0);
    gl_Position = posTemp.xyww;*/
}