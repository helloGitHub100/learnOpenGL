#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec2 outsideMinMax;

void main()
{
    float sdfValue = texture(text, TexCoords).r; // 从纹理中获取 SDF 值
    float sdfValue_outsideMin = step(outsideMinMax.x, sdfValue);
    float sdfValue_outsideMax = step(outsideMinMax.y, sdfValue);
    float sdfValue_outside = sdfValue_outsideMin - sdfValue_outsideMax;
    color = vec4(textColor, sdfValue_outside);
}