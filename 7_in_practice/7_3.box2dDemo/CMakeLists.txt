cmake_minimum_required(VERSION 3.0.0)

set(NAME 7_3.box2dDemo)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)