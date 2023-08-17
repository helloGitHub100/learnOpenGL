cmake_minimum_required(VERSION 3.0.0)

project (model C CXX)

add_library(${PROJECT_NAME} STATIC mesh.h mesh.cpp model.h model.cpp ../codeConvert.h ../codeConvert.cpp)

#target_link_libraries(${PROJECT_NAME} Shader)