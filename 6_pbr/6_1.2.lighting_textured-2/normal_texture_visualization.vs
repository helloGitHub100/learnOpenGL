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
uniform bool bTBN = true;
uniform sampler2D normalMap;
uniform sampler2D heightMap;

void main()
{
    vec3 posNormal   = normalize(aPos);
    float height = texture(heightMap, aTexCoords).r;
    vec3 aPosOffset = aPos + posNormal * height * heightScale;
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 B = normalize(normalMatrix * aBitangent);
    vec3 N = normalize(normalMatrix * aNormal);
    mat3 TBN = mat3(T, B, N);
    vec3 rgb_normal = texture(normalMap, aTexCoords).xyz;
    vec3 normal = normalize(rgb_normal * 2.0 - 1.0);
    vs_out.normal = bTBN ? TBN * normal : normalMatrix * normal;
    //vs_out.normal = normalize(TBN * vec3(0.0, 0.0, 1.0));//检测TBN
    //vs_out.normal = normalize(TBN * vec3(1.0, 0.0, 0.0));//检测TBN
    gl_Position = view * model * vec4(aPosOffset, 1.0); 
}