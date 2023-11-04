cmake_minimum_required(VERSION 3.0.0)

set(NAME 6_2.1.1.ibl_irradiance_conversion-1)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.background.vs\")
add_compile_definitions(FRAGMENT_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.background.fs\")

add_compile_definitions(VERTEX_SPHERE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/sphere_background.vs\")
add_compile_definitions(FRAGMENT_SPHERE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/sphere_background.fs\")

add_compile_definitions(VERTEX_PBR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.pbr.vs\")
add_compile_definitions(FRAGMENT_PBR_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.pbr.fs\")

add_compile_definitions(VERTEX_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.cubemap.vs\")
add_compile_definitions(FRAGMENT_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.1.equirectangular_to_cubemap.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)