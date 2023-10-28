#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec2 MinMax;

void main()
{
    float sdfValue = texture(text, TexCoords).r; // 从纹理中获取 SDF 值
    //sdfValue *= smoothstep(MinMax.x, MinMax.y, sdfValue);
    sdfValue = smoothstep(MinMax.x, MinMax.y, sdfValue);
    color = vec4(textColor, sdfValue);
}