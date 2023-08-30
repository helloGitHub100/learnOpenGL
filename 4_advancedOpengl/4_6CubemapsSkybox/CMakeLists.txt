cmake_minimum_required(VERSION 3.0.0)

project (4_6CubemapsSkybox C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.1.cubemaps.vs\")
add_compile_definitions(VERTEX_SKYBOX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.1.skybox.vs\")
add_compile_definitions(FRAGMENT_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.1.cubemaps.fs\")
add_compile_definitions(FRAGMENT_SKYBOX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/6.1.skybox.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${PROJECT_NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${PROJECT_NAME} model)