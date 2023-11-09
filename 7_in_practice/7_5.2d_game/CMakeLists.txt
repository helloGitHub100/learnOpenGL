# 获取当前文件夹中所有文件，包含后缀.h、.cpp、.vs、.fs、.tcs、.tes、.gs、.cs
file(GLOB TESTBED_SOURCE_FILES *.cpp *.h *.vs *.fs *.tcs *.tes *.gs *.cs)
# 获取当前文件夹中所有shader文件
file(GLOB SHADER_FILES *.vs *.fs *.tcs *.tes *.gs *.cs)

set(NAME 7_5.2d_game)
add_executable(${NAME} ${TESTBED_SOURCE_FILES} ${Include_Common_Dir}/AudioEngine.hpp)

# 将shader文件添加到当前工程
add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_FILES} $<TARGET_FILE_DIR:${NAME}>)