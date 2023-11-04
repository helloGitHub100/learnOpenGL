cmake_minimum_required(VERSION 3.0.0)

set(NAME 7_2.text_rendering-1)#修改1：修改为对应文件名

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/text.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/text.fs\")
add_compile_definitions(FRAGMENT_SDF_SOFT_EDGES_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/softEdges.fs\")
add_compile_definitions(FRAGMENT_SDF_HARD_EDGES_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/hardEdges.fs\")
add_compile_definitions(FRAGMENT_SDF_SHADOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/shadow.fs\")
add_compile_definitions(FRAGMENT_SDF_OUTLINE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/outline.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)