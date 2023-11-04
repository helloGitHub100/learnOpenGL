cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_3.2.1.point_shadows)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.2.1.point_shadows_depth.vs\")
add_compile_definitions(GEOMETRY_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.2.1.point_shadows_depth.gs\")
add_compile_definitions(FRAGMENT_DEPTH_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.2.1.point_shadows_depth.fs\")
add_compile_definitions(VERTEX_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.2.1.point_shadows.vs\")
add_compile_definitions(FRAGMENT_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/3.2.1.point_shadows.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)