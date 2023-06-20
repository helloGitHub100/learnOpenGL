cmake_minimum_required(VERSION 3.0.0)

project (Shader C CXX)

add_compile_definitions(MY_SHADER_LIB)

add_library(${PROJECT_NAME} SHARED Shader.h Shader.cpp)