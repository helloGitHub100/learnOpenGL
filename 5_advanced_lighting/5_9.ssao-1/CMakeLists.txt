cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_9.ssao-1)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_SSAO_GEOMETRY_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao_geometry.vs\")
add_compile_definitions(FRAGMENT_SSAO_GEOMETRY_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao_geometry.fs\")
add_compile_definitions(FRAGMENT_SSAO_BLUR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao_blur.fs\")
add_compile_definitions(FRAGMENT_SSAO_LIGHTING_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao_lighting.fs\")
add_compile_definitions(VERTEX_SSAO_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao.vs\")
add_compile_definitions(FRAGMENT_SSAO_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/9.ssao.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)