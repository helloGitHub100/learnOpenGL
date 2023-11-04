cmake_minimum_required(VERSION 3.0.0)

set(NAME Shader)

add_compile_definitions(MY_SHADER_LIB)

add_library(${NAME} SHARED Shader.h Shader.cpp)