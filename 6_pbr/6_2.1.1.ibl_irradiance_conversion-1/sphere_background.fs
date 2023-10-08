#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

uniform sampler2D equirectangularMap;

void main()
{		
    vec2 TexCoordsTemp = vec2(TexCoords.x, 1.0 - TexCoords.y);
    vec3 envColor = texture(equirectangularMap, TexCoordsTemp).rgb;
    
    // HDR tonemap and gamma correct
    envColor = envColor / (envColor + vec3(1.0));
    envColor = pow(envColor, vec3(1.0/2.2)); 
    
    FragColor = vec4(envColor, 1.0);
}
