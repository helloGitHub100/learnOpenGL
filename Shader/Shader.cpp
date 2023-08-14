#include "Shader.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
    // 1. 从文件路径中获取顶点/片段着色器
    std::string vertexCode;
    std::string fragmentCode;
    std::ifstream vShaderFile;
    std::ifstream fShaderFile;
    // 保证ifstream对象可以抛出异常：
    vShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    fShaderFile.exceptions (std::ifstream::failbit | std::ifstream::badbit);
    try 
    {
        // 打开文件
        vShaderFile.open(vertexPath);
        fShaderFile.open(fragmentPath);
        std::stringstream vShaderStream, fShaderStream;
        // 读取文件的缓冲内容到数据流中
        vShaderStream << vShaderFile.rdbuf();
        fShaderStream << fShaderFile.rdbuf();       
        // 关闭文件处理器
        vShaderFile.close();
        fShaderFile.close();
        // 转换数据流到string
        vertexCode   = vShaderStream.str();
        fragmentCode = fShaderStream.str();     
    }
    catch(std::ifstream::failure e)
    {
        std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
    }
    const char* vShaderCode = vertexCode.c_str();
    const char* fShaderCode = fragmentCode.c_str();

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))//初始化OpenGL函数指针
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return;
    }
    // 2. 编译着色器
    unsigned int vertex, fragment;

    // 顶点着色器
    vertex = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex, 1, &vShaderCode, NULL);
    glCompileShader(vertex);
    // 打印编译错误（如果有的话）
    checkCompileErrors(vertex, "VERTEX");

    // 片段着色器也类似
    fragment = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment, 1, &fShaderCode, NULL);
    glCompileShader(fragment);
    // 打印编译错误（如果有的话）
    checkCompileErrors(fragment, "FRAGMENT");

    // 着色器程序
    ID = glCreateProgram();
    glAttachShader(ID, vertex);
    glAttachShader(ID, fragment);
    glLinkProgram(ID);
    // 打印连接错误（如果有的话）
    checkCompileErrors(ID, "PROGRAM");

    // 删除着色器，它们已经链接到我们的程序中了，已经不再需要了
    glDeleteShader(vertex);
    glDeleteShader(fragment);
}

Shader::~Shader()
{
    glDeleteProgram(ID);
}

void Shader::use() 
{ 
    glUseProgram(ID);
}

void Shader::setBool(const std::string &name, bool value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); 
}

void Shader::setInt(const std::string &name, int value) const
{
    glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}

void Shader::setFloat(const std::string &name, vector<float> values) const
{
    int loction = glGetUniformLocation(ID, name.c_str());
    if (-1 == loction)
        std::cout << "ERROR::Shader::setFloat: loction = " << loction << ", name: " << name << std::endl;
    
    size_t num = values.size();
    if (1 == num)
        glUniform1f(loction, values[0]);
    else if (2 == num)
        glUniform2f(loction, values[0], values[1]);
    else if (3 == num)
        glUniform3f(loction, values[0], values[1], values[2]);
    else if (4 == num)
        glUniform4f(loction, values[0], values[1], values[2], values[3]);
    else
        std::cout << "ERROR::Shader::setFloat: values.size() = " << num << ". It should be in the range [1,4]." << std::endl;
}

void Shader::setFloat(const std::string &name, float value)
{
    setFloat(name, vector<float>{value});
}

void Shader::setVec3(const std::string &name, glm::vec3 v)
{
    setFloat(name, vector<float>{v.x, v.y, v.z});
}

void Shader::setVec3(const std::string &name, float x, float y, float z)
{
    setFloat(name, vector<float>{x, y, z});
}

void Shader::setMatrixFloat(const std::string &name, vector<glm::mat4> transList, int dimension)
{
    int matrixNum = (int)transList.size();
    float** matrices = new float*[matrixNum]; // 分配n个指向float数组的指针
    for (int i = 0; i < matrixNum; i++) {
        matrices[i] = glm::value_ptr(transList[i]);
    }
    int loction = glGetUniformLocation(ID, name.c_str());
    if (dimension == 2)
        glUniformMatrix2fv(loction, matrixNum, GL_FALSE, matrices[0]);
    else if (dimension == 3)
        glUniformMatrix3fv(loction, matrixNum, GL_FALSE, matrices[0]);
    else if (dimension == 4)
        glUniformMatrix4fv(loction, matrixNum, GL_FALSE, matrices[0]);
    else
        std::cout << "ERROR::Shader::setMatrixFloat dimension = " << dimension << std::endl;
    delete []matrices;
}

void Shader::checkCompileErrors(unsigned int shader, std::string type)
{
    int success;
    char infoLog[1024];
    if (type != "PROGRAM")
    {
        glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
        if (!success)
        {
            glGetShaderInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
    else
    {
        glGetProgramiv(shader, GL_LINK_STATUS, &success);
        if (!success)
        {
            glGetProgramInfoLog(shader, 1024, NULL, infoLog);
            std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
        }
    }
}
