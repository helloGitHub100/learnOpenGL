cmake_minimum_required(VERSION 3.0.0)

set(NAME 1_4Shader-1Ex1)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/vertexShader.glsl\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/fragmentShader.glsl\")

add_executable(${NAME} main.cpp)

target_link_libraries(${NAME} Shader)