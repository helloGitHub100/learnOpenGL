#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 256) out;

in VS_OUT {
    int vertexID;
} gs_in[];

const float MAGNITUDE = 0.2;
uniform sampler2D importanceSampleMap;
uniform int SAMPLE_COUNT = 1024;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void GenerateLine(int index)
{
    int maxNum = gs_in[index].vertexID * 128 + 128;
    for(int i = 0 + gs_in[index].vertexID * 128; i < maxNum; ++i)
    {
        vec2 TexCoords = vec2(float(i) / float(SAMPLE_COUNT), 0.5);
        vec3 light = normalize(texture(importanceSampleMap, TexCoords).xyz);
        gl_Position = projection * view * model * gl_in[index].gl_Position;
        EmitVertex();
        gl_Position = projection * view * model * (gl_in[index].gl_Position + vec4(light, 0.0) * MAGNITUDE);
        EmitVertex();
        EndPrimitive();
    }
}

void main()
{
    GenerateLine(0); // first vertex normal
}