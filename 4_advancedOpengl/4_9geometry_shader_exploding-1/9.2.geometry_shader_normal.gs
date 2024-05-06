#version 330 core
layout (triangles) in;
layout (triangle_strip, max_vertices = 3) out;

in VS_OUT {
    vec2 texCoords;
    vec4 position;
} gs_in[];

out vec2 TexCoords;

uniform float time;
uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

vec4 explode(vec4 position, vec3 normal)
{
    float magnitude = 2.0;
    vec3 direction = normal * ((sin(time) + 1.0) * 0.5) * magnitude;
    return position + vec4(direction, 0.0);
}

vec3 GetNormal()
{
    vec3 pos0 = vec3(gs_in[0].position);
    vec3 pos1 = vec3(gs_in[1].position);
    vec3 pos2 = vec3(gs_in[2].position);

    vec3 a = pos0 - pos1;
    vec3 b = pos2 - pos1;
    return normalize(cross(b, a));
}

void main() {
    mat4 mvp = projection * view * model;
    vec3 normal = GetNormal();
    vec4 pos = gs_in[0].position;
    gl_Position = mvp * explode(pos, normal);
    TexCoords = gs_in[0].texCoords;
    EmitVertex();

    pos = gs_in[1].position;
    gl_Position = mvp * explode(pos, normal);
    TexCoords = gs_in[1].texCoords;
    EmitVertex();

    pos = gs_in[2].position;
    gl_Position = mvp * explode(pos, normal);
    TexCoords = gs_in[2].texCoords;
    EmitVertex();
    EndPrimitive();
}