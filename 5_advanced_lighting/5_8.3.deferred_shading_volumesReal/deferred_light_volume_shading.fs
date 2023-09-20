#version 330 core
out vec4 FragColor;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;

uniform vec3 lightPos;
uniform vec3 Color;
    
uniform float Linear;
uniform float Quadratic;
uniform vec3 viewPos;
uniform vec2 gScreenSize = vec2(800, 600);

void main()
{             
    // retrieve data from gbuffer
    vec2 TexCoords = gl_FragCoord.xy / gScreenSize;
    vec3 FragPos = texture(gPosition, TexCoords).rgb;
    vec3 Normal = texture(gNormal, TexCoords).rgb;
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    float Specular = texture(gAlbedoSpec, TexCoords).a;
    
    // then calculate lighting as usual
    vec3 viewDir  = normalize(viewPos - FragPos);
    // diffuse
    vec3 lightDir = normalize(lightPos - FragPos);
    vec3 diffuse = max(dot(Normal, lightDir), 0.0) * Diffuse * Color;
    // specular
    vec3 halfwayDir = normalize(lightDir + viewDir);  
    float spec = pow(max(dot(Normal, halfwayDir), 0.0), 16.0);
    vec3 specular = Color * spec * Specular;
    // attenuation
    float distance = length(lightPos - FragPos);
    float attenuation = 1.0 / (1.0 + Linear * distance + Quadratic * distance * distance);
    diffuse *= attenuation;
    specular *= attenuation;
    FragColor = vec4(diffuse + specular, 1.0);
}
