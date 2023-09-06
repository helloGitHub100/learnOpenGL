cmake_minimum_required(VERSION 3.0.0)

project (5_3.1.2.shadow_mapping_base C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.debug_quad.vs\")
add_compile_definitions(FRAGMENT_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.debug_quad_depth.fs\")
add_compile_definitions(VERTEX_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.shadow_mapping_depth.vs\")
add_compile_definitions(FRAGMENT_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.shadow_mapping_depth.fs\")
add_compile_definitions(VERTEX_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.shadow_mapping.vs\")
add_compile_definitions(FRAGMENT_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.1.2.shadow_mapping.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${PROJECT_NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${PROJECT_NAME} model)