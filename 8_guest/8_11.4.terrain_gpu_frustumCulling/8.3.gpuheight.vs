#version 410 core
layout (location = 0) in vec3 aPos;

uniform mat4 inverseMainView;
uniform mat4 inverseProjection;
uniform vec3 cameraPos;
uniform vec2 xMinMax;
uniform vec2 zMinMax;

out vec2 TexCoord;

void main()
{
    vec4 viewPos = inverseProjection * vec4(aPos.x, -aPos.y, aPos.z, 1.0);
    viewPos /= viewPos.w;
    vec4 worldPos = inverseMainView * viewPos;
    vec3 dir = normalize(worldPos.xyz - cameraPos);
    float k = -cameraPos.y / dir.y;
    k = k < 0 ? 1000 : k;
    vec3 pos = cameraPos + k * dir;
    float textureX = (pos.x - xMinMax.x) / (xMinMax.y - xMinMax.x);
    float textureZ = (pos.z - zMinMax.x) / (zMinMax.y - zMinMax.x);

    gl_Position = vec4(pos, 1.0);
    TexCoord = vec2(textureX, textureZ);
}