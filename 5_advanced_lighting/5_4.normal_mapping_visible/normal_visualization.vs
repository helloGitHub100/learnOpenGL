#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 normal;
} vs_out;

uniform mat4 view;
uniform mat4 model;
uniform float dotMax = 0.9;
uniform bool enableTBN = true;

uniform sampler2D normalMap;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vec3 T = normalize(normalMatrix * aTangent);
    vec3 N = normalize(normalMatrix * aNormal);
    // re-orthogonalize T with respect to N
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    
    mat3 TBN = mat3(T, B, N);
    vec3 normal = texture(normalMap, aTexCoords).xyz;
    if(enableTBN)
        vs_out.normal = abs(dot(vec3(0.0, 0.0, 1.0), normal)) < dotMax ? normalize(TBN * texture(normalMap, aTexCoords).xyz) : vec3(0.0);
    else
        vs_out.normal = normalize(normalMatrix * texture(normalMap, aTexCoords).xyz);
    //vs_out.normal = normalize(TBN * vec3(0.0, 0.0, 1.0));//检测TBN
    //vs_out.normal = normalize(TBN * vec3(1.0, 0.0, 0.0));//检测TBN
    gl_Position = view * model * vec4(aPos, 1.0); 
}