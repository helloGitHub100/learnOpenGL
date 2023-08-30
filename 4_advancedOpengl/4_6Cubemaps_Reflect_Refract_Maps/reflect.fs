#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform sampler2D texture_diffuse1;
uniform sampler2D texture_height1;
uniform samplerCube skybox;
uniform bool bDiffuse;
uniform bool bReflectMaps;

void main()
{    
    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    vec4 diffuse = bDiffuse ? texture(texture_diffuse1, TexCoords) : vec4(0.0);
    vec3 reflectMap = bReflectMaps ? vec3(texture(texture_height1, TexCoords)) : vec3(1.0);
    FragColor = diffuse + vec4(texture(skybox, R).rgb * reflectMap, 1.0);
}