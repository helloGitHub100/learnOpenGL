cmake_minimum_required(VERSION 3.0.0)

set(NAME model)

add_library(${NAME} STATIC mesh.h mesh.cpp model.h model.cpp ../codeConvert.h ../codeConvert.cpp)

#target_link_libraries(${NAME} Shader)