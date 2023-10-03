#version 330 core
layout (triangles) in;
layout (points, max_vertices = 6) out;

in VS_OUT {
    vec3 normal;
    vec2 TexCoords;
} gs_in[];

out vec2 TexCoords;

const float MAGNITUDE = 0.2;

uniform mat4 projection;

void GenerateLine(int index)
{
    TexCoords = gs_in[index].TexCoords;
    gl_Position = projection * gl_in[index].gl_Position;
    EmitVertex();
    //gl_Position = projection * (gl_in[index].gl_Position + vec4(gs_in[index].normal, 0.0) * MAGNITUDE);
    //EmitVertex();
    EndPrimitive();
}

void main()
{
    GenerateLine(0); // first vertex normal
    GenerateLine(1); // second vertex normal
    GenerateLine(2); // third vertex normal
}