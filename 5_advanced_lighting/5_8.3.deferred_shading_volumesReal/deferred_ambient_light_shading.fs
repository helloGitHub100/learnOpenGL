#version 330 core
out vec4 FragColor;

in vec2 TexCoords;

uniform sampler2D gAlbedoSpec;

void main()
{             
    // retrieve data from gbuffer
    vec3 Diffuse = texture(gAlbedoSpec, TexCoords).rgb;
    
    // then calculate lighting as usual
    vec3 lighting  = Diffuse * 0.1; // hard-coded ambient component
    FragColor = vec4(lighting, 1.0);
}
