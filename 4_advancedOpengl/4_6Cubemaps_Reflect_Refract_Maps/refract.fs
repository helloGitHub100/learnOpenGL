#version 330 core
out vec4 FragColor;

in vec3 Normal;
in vec3 Position;
in vec2 TexCoords;

uniform vec3 cameraPos;
uniform samplerCube skybox;
uniform sampler2D texture_diffuse1;
uniform float refractRatio;
uniform bool bDiffuse;
uniform bool bReflectMaps;

void main()
{    
    float ratio = 1.00 / refractRatio;
    vec3 I = normalize(Position - cameraPos);
    vec3 R = refract(I, normalize(Normal), ratio);
    vec4 diffuse = bDiffuse ? texture(texture_diffuse1, TexCoords) : vec4(0.0);
    FragColor = diffuse + vec4(texture(skybox, R).rgb, 1.0);
}