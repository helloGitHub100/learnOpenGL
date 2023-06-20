cmake_minimum_required(VERSION 3.0.0)

project (1_4Shader C CXX)#修改1：修改为对应文件名
message(STATUS "project= ${PROJECT_NAME}" )

add_executable(${PROJECT_NAME} main.cpp)