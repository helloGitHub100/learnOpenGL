#version 410 core

in vec3 color;
in vec2 tesTexCoord;

out vec4 FragColor;

void main()
{
    if (tesTexCoord.x > 1.0 || tesTexCoord.x < 0.0 || tesTexCoord.y > 1.0 || tesTexCoord.y < 0.0)
        discard;
    FragColor = vec4(color, 1.0);
}