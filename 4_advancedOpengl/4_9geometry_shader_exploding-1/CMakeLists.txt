cmake_minimum_required(VERSION 3.0.0)

project (4_9geometry_shader_exploding-1 C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.2.geometry_shader.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.2.geometry_shader.fs\")
add_compile_definitions(GEOMETRY_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.2.geometry_shader.gs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${PROJECT_NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${PROJECT_NAME} model)