#version 330 core
out vec4 FragColor;
in vec3 ourColor;
in vec2 TexCoord;
in vec3 Normal;
in vec3 FragPos;
in vec3 lightPos_viewSpace;
in vec3 viewPos_viewSpace;
uniform float mixRatio;

uniform vec3 objectColor;
uniform vec3 lightColor;

void main()
{
   // ambient
    float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * lightColor;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(lightPos_viewSpace - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 diffuse = diff * lightColor;
            
   float specularStrength = 0.5;
   vec3 viewDir = normalize(viewPos_viewSpace - FragPos);
   vec3 reflectDir = reflect(-lightDir, norm);
   float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
   vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse + specular) * objectColor;
    FragColor = vec4(result, 1.0);
}