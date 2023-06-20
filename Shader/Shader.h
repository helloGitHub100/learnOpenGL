#ifndef SHADER_H
#define SHADER_H

#ifndef BUILD_STATIC
# if defined(MY_SHADER_LIB)
#  define MY_SHADER_EXPORT __declspec(dllexport)
# else
#  define MY_SHADER_EXPORT __declspec(dllimport)
# endif
#else
# define MY_SHADER_EXPORT
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

using namespace std;

enum GLSL_SAMPLE_TYPE
{
    GLSL_SAMPLE_2D,
    GLSL_SAMPLE_3D
};

class MY_SHADER_EXPORT Shader
{
public:
    // 程序ID
    unsigned int ID;

    // 构造器读取并构建着色器
    Shader(const char* vertexPath, const char* fragmentPath);
    ~Shader();
    // 使用/激活程序
    void use();
    // uniform工具函数
    void setBool(const std::string &name, bool value) const;  
    void setInt(const std::string &name, int value) const;   
    void setFloat(const std::string &name, vector<float> values) const;
    void setMatrixFloat(const std::string &name, vector<glm::mat4>  transList, int dimension);

    void checkCompileErrors(unsigned int shader, std::string type);
};

#endif