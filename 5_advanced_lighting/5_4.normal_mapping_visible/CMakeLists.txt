cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_4.normal_mapping_visible)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/4.normal_mapping.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/4.normal_mapping.fs\")

add_compile_definitions(VERTEX_NORMAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/normal_visualization.vs\")
add_compile_definitions(FRAGMENT_NORMAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/normal_visualization.fs\")
add_compile_definitions(GEOMETRY_NORMAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/normal_visualization.gs\")

add_compile_definitions(VERTEX_TBN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/tbn_visualization.vs\")
add_compile_definitions(GEOMETRY_TBN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/tbn_visualization.gs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)