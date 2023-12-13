cmake_minimum_required(VERSION 3.0.0)

set(NAME 8_11.3.terrain_gpu_frustumCulling)#修改1：修改为对应文件名

add_compile_definitions(VERTEX_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.3.gpuheight.vs\")
add_compile_definitions(FRAGMENT_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.3.gpuheight.fs\")
add_compile_definitions(TCS_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.3.gpuheight.tcs\")
add_compile_definitions(TES_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/8.3.gpuheight.tes\")

add_compile_definitions(VERTEX_DEBUG_FRUSTUM_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/debug_frustum.vs\")
add_compile_definitions(FRAGMENT_DEBUG_FRUSTUM_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/debug_frustum.fs\")

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)