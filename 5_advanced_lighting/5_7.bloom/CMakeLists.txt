cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_7.bloom)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_BLOOM_FINAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.bloom_final.vs\")
add_compile_definitions(FRAGMENT_BLOOM_FINAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.bloom_final.fs\")
add_compile_definitions(VERTEX_BLOOM_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.bloom.vs\")
add_compile_definitions(FRAGMENT_BLOOM_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.bloom.fs\")
add_compile_definitions(VERTEX_BLUR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.blur.vs\")
add_compile_definitions(FRAGMENT_BLUR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.blur.fs\")

add_compile_definitions(FRAGMENT_LIGHT_BOX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/7.light_box.fs\")
add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)