#version 330 core
out vec4 FragColor;
in vec2 TexCoords;

void main()
{
    FragColor = vec4(TexCoords, 0.0, 1.0);
}
