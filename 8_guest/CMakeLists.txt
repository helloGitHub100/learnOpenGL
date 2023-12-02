cmake_minimum_required(VERSION 3.0.0)

add_custom_target(8_guest)

add_subdirectory(8_1.oit)
add_subdirectory(8_2.oit_window)
add_subdirectory(8_3.oit_instanced)
add_subdirectory(8_4.skeletal_animation)
add_subdirectory(8_5.skeletal_animation)
add_subdirectory(8_6.skeletal_animation_boneMatrixTexture)
add_subdirectory(8_7.hello_triangle_dsa)