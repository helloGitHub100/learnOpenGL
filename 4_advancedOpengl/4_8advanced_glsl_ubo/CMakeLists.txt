cmake_minimum_required(VERSION 3.0.0)

project (4_8advanced_glsl_ubo C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.advanced_glsl.vs\")
add_compile_definitions(FRAGMENT_BLUE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.blue.fs\")
add_compile_definitions(FRAGMENT_GREEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.green.fs\")
add_compile_definitions(FRAGMENT_RED_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.red.fs\")
add_compile_definitions(FRAGMENT_YELLOW_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.yellow.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${PROJECT_NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${PROJECT_NAME} model)