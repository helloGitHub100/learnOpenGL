#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;

out vec2 TexCoords;
out vec3 WorldPos;
out vec3 WorldPosOriginal;
out vec3 Normal;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform mat3 normalMatrix;
uniform float heightScale = 0.1;
uniform sampler2D heightMap;

void main()
{
    TexCoords = aTexCoords;
    float height = texture(heightMap, TexCoords).r;
    vec3 posNormal   = normalize(aPos);
    vec3 aPosOffset = aPos + posNormal * height * heightScale;
    WorldPos = vec3(model * vec4(aPosOffset, 1.0));
    WorldPosOriginal = vec3(model * vec4(aPos, 1.0));
    Normal = normalMatrix * aNormal;   

    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}