#version 330 core

layout(location = 0) in vec3 pos;
layout(location = 1) in vec3 norm;
layout(location = 2) in vec2 tex;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;
layout(location = 5) in ivec4 boneIds; 
layout(location = 6) in vec4 weights;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform int MAX_BONES = 200;
const int MAX_BONE_INFLUENCE = 4;
uniform sampler2D boneMatrixImage;

out vec2 TexCoords;

mat4 getBoneMatrix(int row)
{
    return mat4(
        texelFetch(boneMatrixImage, ivec2(0, row), 0),
        texelFetch(boneMatrixImage, ivec2(1, row), 0),
        texelFetch(boneMatrixImage, ivec2(2, row), 0),
        texelFetch(boneMatrixImage, ivec2(3, row), 0));
}

void main()
{
    vec4 totalPosition = vec4(0.0f);
    for(int i = 0 ; i < MAX_BONE_INFLUENCE ; i++)
    {
        if(boneIds[i] == -1) 
            continue;
        if(boneIds[i] >=MAX_BONES) 
        {
            totalPosition = vec4(pos,1.0f);
            break;
        }
        mat4 boneMatrix = getBoneMatrix(boneIds[i]);
        vec4 localPosition = boneMatrix * vec4(pos,1.0f);
        totalPosition += localPosition * weights[i];
        vec3 localNormal = mat3(boneMatrix) * norm;
   }
	
    mat4 viewModel = view * model;
    gl_Position =  projection * viewModel * totalPosition;
	TexCoords = tex;
}
