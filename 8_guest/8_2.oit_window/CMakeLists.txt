cmake_minimum_required(VERSION 3.0.0)

set(NAME 8_2.oit_window)#修改1：修改为对应文件名

add_compile_definitions(VERTEX_TRANSPARENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/transparent.vs\")
add_compile_definitions(FRAGMENT_TRANSPARENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/transparent.fs\")

add_compile_definitions(VERTEX_TRANSPARENT_TEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/transparentTex.vs\")
add_compile_definitions(FRAGMENT_TRANSPARENT_TEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/transparentTex.fs\")

add_compile_definitions(VERTEX_SOLID_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/solid.vs\")
add_compile_definitions(FRAGMENT_SOLID_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/solid.fs\")

add_compile_definitions(VERTEX_SOLID_TEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/solidTex.vs\")
add_compile_definitions(FRAGMENT_SOLID_TEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/solidTex.fs\")

add_compile_definitions(VERTEX_SCREEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/screen.vs\")
add_compile_definitions(FRAGMENT_SCREEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/screen.fs\")

add_compile_definitions(VERTEX_COMPOSITE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/composite.vs\")
add_compile_definitions(FRAGMENT_COMPOSITE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/composite.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)