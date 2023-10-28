#version 330 core
in vec2 TexCoords;
out vec4 color;

uniform sampler2D text;
uniform vec3 textColor;
uniform vec3 shadowColor = vec3(0);
uniform vec2 MinMax;
uniform vec2 MinMax_shadow;
uniform vec2 uv_Offset;

void main()
{
    vec2 texelSize = 1.0 / textureSize(text, 0);
    float sdfValue = texture(text, TexCoords).r; // 从纹理中获取 SDF 值
    float sdfValue_Offset = texture(text, TexCoords + uv_Offset * texelSize).r; // 从纹理中获取 SDF 值
    float alpha = smoothstep(MinMax.x, MinMax.y, sdfValue);
    float alpha_shodow = smoothstep(MinMax_shadow.x, MinMax_shadow.y, sdfValue_Offset);
    float stepValue = step(MinMax.x, alpha);
    color = vec4(textColor * stepValue + shadowColor * (1 - stepValue), alpha + alpha_shodow);
}