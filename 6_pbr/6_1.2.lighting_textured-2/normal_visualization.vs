#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;

uniform float heightScale = 0.1;
uniform sampler2D heightMap;

void main()
{
    vec3 posNormal   = normalize(aPos);
    float height = texture(heightMap, aTexCoords).r;
    vec3 aPosOffset = aPos + posNormal * height * heightScale;
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vs_out.normal = vec3(vec4(normalMatrix * aNormal, 0.0));
    gl_Position = view * model * vec4(aPosOffset, 1.0); 
}