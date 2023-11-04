cmake_minimum_required(VERSION 3.0.0)

set(NAME 4_11.2.anti_aliasing_offscreen)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/11.2.anti_aliasing.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/11.2.anti_aliasing.fs\")
add_compile_definitions(VERTEX_POST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/11.2.aa_post.vs\")
add_compile_definitions(FRAGMENT_POST_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/11.2.aa_post.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)