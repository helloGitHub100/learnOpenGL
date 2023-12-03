cmake_minimum_required(VERSION 3.0.0)

set(NAME 8_8.csm)#修改1：修改为对应文件名

add_compile_definitions(VERTEX_DEBUG_CASCADE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.debug_cascade.vs\")
add_compile_definitions(FRAGMENT_DEBUG_CASCADE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.debug_cascade.fs\")

add_compile_definitions(VERTEX_DEBUG_QUAD_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.debug_quad.vs\")
add_compile_definitions(FRAGMENT_DEBUG_QUAD_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.debug_quad_depth.fs\")

add_compile_definitions(VERTEX_SHADOW_MAPING_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.shadow_mapping_depth.vs\")
add_compile_definitions(GEOMETRY_SHADOW_MAPING_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.shadow_mapping_depth.gs\")
add_compile_definitions(FRAGMENT_SHADOW_MAPING_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.shadow_mapping_depth.fs\")

add_compile_definitions(VERTEX_SHADOW_MAPING_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.shadow_mapping.vs\")
add_compile_definitions(FRAGMENT_SHADOW_MAPING_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.shadow_mapping.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)