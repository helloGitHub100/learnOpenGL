# 获取当前文件夹中所有文件，包含后缀.h、.cpp、.vs、.fs、.tcs、.tes、.gs、.cs
file(GLOB TESTBED_SOURCE_FILES *.cpp *.h *.vs *.fs *.tcs *.tes *.gs *.cs)
# 获取当前文件夹中所有shader文件
file(GLOB SHADER_FILES *.vs *.fs *.tcs *.tes *.gs *.cs)

set(NAME 7_5.2d_game)
link_directories(${NAME} ${CMAKE_SOURCE_DIR}/lib)
add_executable(${NAME} ${TESTBED_SOURCE_FILES})

#message("CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${NAME} PUBLIC irrKlang)

# 获取“根目录/dlls”下的所有dll文件
set(dllFolders ${CMAKE_SOURCE_DIR}/dlls)
file(GLOB DLLS_FILES ${dllFolders}/*.dll)
#message("DLLS_FILES: ${DLLS_FILES}")
# 将dll文件添加到当前工程
add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DLLS_FILES} $<TARGET_FILE_DIR:${NAME}>)
# 将shader文件添加到当前工程
add_custom_command(TARGET ${NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_FILES} $<TARGET_FILE_DIR:${NAME}>)