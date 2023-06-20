cmake_minimum_required(VERSION 3.0.0)

project (1_4Shader-1 C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/shader.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/shader.fs\")

add_executable(${PROJECT_NAME} main.cpp)

target_link_libraries(${PROJECT_NAME} Shader)