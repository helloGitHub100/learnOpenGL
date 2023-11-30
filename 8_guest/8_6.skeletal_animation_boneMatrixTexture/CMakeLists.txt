cmake_minimum_required(VERSION 3.0.0)

set(NAME 8_6.skeletal_animation_boneMatrixTexture)#修改1：修改为对应文件名

add_compile_definitions(VERTEX_ANIMATION_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/anim_model.vs\")
add_compile_definitions(FRAGMENT_ANIMATION_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/anim_model.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)