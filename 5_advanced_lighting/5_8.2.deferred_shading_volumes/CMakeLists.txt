cmake_minimum_required(VERSION 3.0.0)

set(NAME 5_8.2.deferred_shading_volumes)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_DEFERRED_LIGHT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.deferred_light_box.vs\")
add_compile_definitions(FRAGMENT_DEFERRED_LIGHT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.deferred_light_box.fs\")
add_compile_definitions(VERTEX_DEFERRED_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.deferred_shading.vs\")
add_compile_definitions(FRAGMENT_DEFERRED_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.deferred_shading.fs\")
#add_compile_definitions(VERTEX_FBO_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.fbo_debug.vs\")
#add_compile_definitions(FRAGMENT_FBO_DEBUG_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.fbo_debug.fs\")
add_compile_definitions(VERTEX_G_BUFFER_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.g_buffer.vs\")
add_compile_definitions(FRAGMENT_G_BUFFER_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.2.g_buffer.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)