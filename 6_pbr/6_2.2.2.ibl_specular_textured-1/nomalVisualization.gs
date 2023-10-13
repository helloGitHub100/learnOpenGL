#version 330 core
layout (points) in;
layout (line_strip, max_vertices = 2) out;

in VS_OUT {
    vec3 normal;
} gs_in[];

const float MAGNITUDE = 0.2;

uniform mat4 view;
uniform mat4 model;
uniform mat4 projection;

void GenerateLine(int index)
{
    gl_Position = projection * view * model * gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = projection * view * model * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
}