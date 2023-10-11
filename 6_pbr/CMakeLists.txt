cmake_minimum_required(VERSION 3.0.0)

add_custom_target(6_pbr)

add_subdirectory(6_1.1.lighting)
add_subdirectory(6_1.2.lighting_textured)
add_subdirectory(6_1.2.lighting_textured-1)
add_subdirectory(6_1.2.lighting_textured-2)
add_subdirectory(6_2.1.1.ibl_irradiance_conversion)
add_subdirectory(6_2.1.1.ibl_irradiance_conversion-1)
add_subdirectory(6_2.1.2.ibl_irradiance)
add_subdirectory(6_2.1.2.ibl_irradiance-1)
add_subdirectory(6_2.2.1.ibl_specular)
add_subdirectory(6_2.2.2.ibl_specular_textured)