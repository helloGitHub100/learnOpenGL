cmake_minimum_required(VERSION 3.0.0)

project (1_7CoorSystem-3 C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/vertexShader.glsl\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/fragmentShader.glsl\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)
add_compile_definitions(RESOURCES_TEXTURES_DIR=\"${RESOURCES_TEXTURES_DIR}\")

add_executable(${PROJECT_NAME} main.cpp ${STD_IMAGE_HEADER})

target_link_libraries(${PROJECT_NAME} Shader)