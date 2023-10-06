#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 3) in vec3 aTangent;

out VS_OUT {
    vec3 normal;
    vec3 B;
    vec3 T;
} vs_out;

uniform mat4 view;
uniform mat4 model;

void main()
{
    mat3 normalMatrix = transpose(inverse(mat3(view * model)));
    vs_out.T = normalize(normalMatrix * aTangent);
    vs_out.normal = normalize(normalMatrix * aNormal);
    // re-orthogonalize T with respect to N
    vs_out.T = normalize(vs_out.T - dot(vs_out.T, vs_out.normal) * vs_out.normal);
    vs_out.B = cross(vs_out.normal, vs_out.T);
    
    //mat3 TBN = transpose(mat3(T, B, N));
    gl_Position = view * model * vec4(aPos, 1.0); 
}