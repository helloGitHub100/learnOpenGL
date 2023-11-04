cmake_minimum_required(VERSION 3.0.0)

set(NAME 4_10.3.asteroids_instanced)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.3.asteroids.vs\")
add_compile_definitions(VERTEX_PLANET_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.3.planet.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/10.3.asteroids.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)