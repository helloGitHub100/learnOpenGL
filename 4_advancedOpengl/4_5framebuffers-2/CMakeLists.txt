cmake_minimum_required(VERSION 3.0.0)

project (4_5framebuffers-2 C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.vs\")
add_compile_definitions(VERTEX_SCREEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers_screen.vs\")
add_compile_definitions(VERTEX_SCREEN_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers.fs\")
add_compile_definitions(FRAGMENT_SCREEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/5.1.framebuffers_screen.fs\")
add_compile_definitions(FRAGMENT_SCREEN_INVERSE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen_Inverse.fs\")
add_compile_definitions(FRAGMENT_SCREEN_GRAYSCALE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen_Grayscale.fs\")
add_compile_definitions(FRAGMENT_SCREEN_SHARPEN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen_Sharpen.fs\")
add_compile_definitions(FRAGMENT_SCREEN_BLUR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen_Blur.fs\")
add_compile_definitions(FRAGMENT_SCREEN_EDGE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/framebuffers_screen_EdgeDetection.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${PROJECT_NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${PROJECT_NAME} model)