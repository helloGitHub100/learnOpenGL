cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_3.1.1.shadow_mapping_depth)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.1.debug_quad.vs\")
add_compile_definitions(FRAGMENT_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.1.debug_quad_depth.fs\")
add_compile_definitions(VERTEX_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.1.shadow_mapping_depth.vs\")
add_compile_definitions(FRAGMENT_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.1.shadow_mapping_depth.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)