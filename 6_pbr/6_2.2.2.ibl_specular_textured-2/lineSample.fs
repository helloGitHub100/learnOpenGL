#version 330 core
out vec4 FragColor;

in float lineTexCoords;

uniform sampler2D importanceSampleMap;

void main()
{		
    vec3 light = texture(importanceSampleMap, vec2(lineTexCoords, 0.5)).rgb;

    FragColor = vec4(light, 1.0);
}
