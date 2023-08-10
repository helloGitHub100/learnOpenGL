#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;
out vec3 ourColor;
out vec2 TexCoord;
out vec3 FragPos;
out vec3 Normal;
out vec3 lightPos_viewSpace;
out vec3 viewPos_viewSpace;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform vec3 lightPos;
uniform vec3 viewPos;

void main()
{
   // 注意乘法要从右向左读
   //gl_Position = projection * view * model * vec4(aPos, 1.0);
   FragPos = vec3(view * model * vec4(aPos, 1.0));
   Normal = mat3(transpose(inverse(view * model))) * aNormal;;
   gl_Position = projection * vec4(FragPos, 1.0);
   lightPos_viewSpace = vec3(view * vec4(lightPos, 1.0));
   viewPos_viewSpace = vec3(view * vec4(viewPos, 1.0));
}