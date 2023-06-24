cmake_minimum_required(VERSION 3.0.0)
project(LearnOpenGL VERSION 0.1.0)
message(STATUS "project= ${PROJECT_NAME}" )
include(CTest)
enable_testing()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})#可执行文件的输出目录
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})#动态库的输出目录
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})#静态库的输出目录

set(GLAD_SOURCE "D:/vscode/learnOpenGL/glad/src/glad.c")
set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake")
include(${CMAKE_TOOLCHAIN_FILE})
message(STATUS "CMAKE_TOOLCHAIN_FILE= ${CMAKE_TOOLCHAIN_FILE}" )
message(STATUS "CMAKE_PREFIX_PATH= ${CMAKE_PREFIX_PATH}" )
message(STATUS "glfw3_DIR= ${glfw3_DIR}" )
find_package(glfw3 CONFIG REQUIRED)
find_package(glad CONFIG REQUIRED)

link_libraries(opengl32.lib glfw glad::glad)

set(Include_Common_Dir ${CMAKE_CURRENT_SOURCE_DIR}/includes/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/Shader/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/includes/glm/)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)
set(STD_IMAGE_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/includes/stb_image.h)
set(RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources)
set(RESOURCES_TEXTURES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/resources/textures)

add_subdirectory(Shader)
add_subdirectory(1_StartedTutorial)
