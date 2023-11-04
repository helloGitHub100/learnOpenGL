cmake_minimum_required(VERSION 3.0.0)

set(NAME 6_2.2.2.ibl_specular_textured-2)#修改1：修改为对应文件名


add_compile_definitions(VERTEX_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.background.vs\")
add_compile_definitions(FRAGMENT_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.background.fs\")

add_compile_definitions(VERTEX_PBR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.pbr.vs\")
add_compile_definitions(FRAGMENT_PBR_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.pbr.fs\")

add_compile_definitions(FRAGMENT_PBR_GUN_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/pbr_gun.fs\")
add_compile_definitions(FRAGMENT_PBR_WINE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/pbr_wine.fs\")

add_compile_definitions(VERTEX_SPHERE_TEXCOORDS_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/sphereTexcoords.vs\")
add_compile_definitions(FRAGMENT_SPHERE_TEXCOORDS_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/sphereTexcoords.fs\")

add_compile_definitions(VERTEX_BRDF_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.brdf.vs\")
add_compile_definitions(FRAGMENT_BRDF_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.brdf.fs\")

add_compile_definitions(VERTEX_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.cubemap.vs\")
add_compile_definitions(FRAGMENT_EQUIRE_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.equirectangular_to_cubemap.fs\")
add_compile_definitions(FRAGMENT_IRRADIANCE_CUBE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.irradiance_convolution.fs\")

add_compile_definitions(FRAGMENT_PREFILTER_BACKGROUND_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/2.2.2.prefilter.fs\")

add_compile_definitions(VERTEX_IMPORTANCE_VISUAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/importanceSampleVisualization.vs\")
add_compile_definitions(FRAGMENT_IMPORTANCE_VISUAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/importanceSampleVisualization.fs\")
add_compile_definitions(GEOMETRY_IMPORTANCE_VISUAL_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/importanceSampleVisualization.gs\")

add_compile_definitions(VERTEX_IMPORTANCE_LINE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/line.vs\")
add_compile_definitions(FRAGMENT_IMPORTANCE_LINE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/line.fs\")
add_compile_definitions(FRAGMENT_IMPORTANCE_LINE_SAMPLE_FILE=\"${CMAKE_CURRENT_SOURCE_DIR}/lineSample.fs\")

add_compile_definitions(STB_IMAGE_IMPLEMENTATION)

add_executable(${NAME} main.cpp ${Include_Common_Dir}/camera.h ${STD_IMAGE_HEADER} ${Include_Common_Dir}/shader_m.h)

target_link_libraries(${NAME} model)