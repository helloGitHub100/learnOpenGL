cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_6.hdr)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_HDR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.hdr.vs\")
add_compile_definitions(FRAGMENT_HDR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.hdr.fs\")
add_compile_definitions(VERTEX_LIGHTING_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.lighting.vs\")
add_compile_definitions(FRAGMENT_LIGHTING_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.lighting.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)