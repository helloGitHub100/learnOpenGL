cmake_minimum_required(VERSION 3.0.0)

set(NAME 6_2.1.2.ibl_irradiance)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.background.vs\")
add_compile_definitions(FRAGMENT_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.background.fs\")

add_compile_definitions(VERTEX_PBR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.pbr.vs\")
add_compile_definitions(FRAGMENT_PBR_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.pbr.fs\")

add_compile_definitions(VERTEX_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.cubemap.vs\")
add_compile_definitions(FRAGMENT_EQUIRE_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.equirectangular_to_cubemap.fs\")
add_compile_definitions(FRAGMENT_IRRADIANCE_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.1.2.irradiance_convolution.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)