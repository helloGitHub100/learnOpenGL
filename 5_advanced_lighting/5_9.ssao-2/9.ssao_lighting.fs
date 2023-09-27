#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedo;
uniform sampler2D ssao;

struct Light {
    vec3 Position;
};
uniform float Linear;
uniform float Quadratic;
uniform vec3 lightColor;
const int NR_LIGHTS = 500;
uniform int currentLightNumber = 0;
uniform Light lights[NR_LIGHTS];

uniform float textureScale = 1.0;
uniform vec3 viewPos;
uniform float ambientRatio = 0.3;

void main()
{             
    // retrieve data from gbuffer
    float offset = (1.0 - textureScale) / 2;
    vec2 TexCoordsTemp = TexCoords * vec2(textureScale) + vec2(offset);
    vec3 FragPos = texture(gPosition, TexCoordsTemp).rgb;
    vec3 Normal = texture(gNormal, TexCoordsTemp).rgb;
    vec3 Diffuse = texture(gAlbedo, TexCoordsTemp).rgb;
    float Specular = texture(gAlbedo, TexCoordsTemp).a;
    float AmbientOcclusion = texture(ssao, TexCoordsTemp).r;
    
    // then calculate lighting as usual
    vec3 ambient = vec3(ambientRatio * Diffuse * AmbientOcclusion);
    vec3 lighting  = ambient; 
    vec3 viewDir  = normalize(viewPos - FragPos);
    for(int i = 0; i < currentLightNumber; ++i)
    {
        // diffuse
        vec3 lightDir = normalize(lights[i].Position - FragPos);
        vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * lightColor;
        // specular
        vec3 halfwayDir = normalize(lightDir + viewDir);  
        float spec = pow(max(dot(Normal, halfwayDir), 0.0), 8.0);
        vec3 specular = lightColor * spec * Specular;
        // attenuation
        float distance = length(lights[i].Position - FragPos);
        float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);
        diffuse *= attenuation;
        specular *= attenuation;
        lighting += diffuse + specular;
    }
    FragColor = vec4(lighting, 1.0);
}
