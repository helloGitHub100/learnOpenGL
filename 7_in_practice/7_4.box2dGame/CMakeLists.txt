set (TESTBED_SOURCE_FILES
	draw.cpp
	draw.h
	main.cpp
	settings.h
	settings.cpp
	test.cpp
	test.h
	tests/random_collision.cpp
	tests/random_collision1.cpp
)

set(NAME 7_4.box2dGame)

add_executable(${NAME} ${TESTBED_SOURCE_FILES})
target_include_directories(${NAME} PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(${NAME} PUBLIC sajson)
set_target_properties(${NAME} PROPERTIES
	CXX_STANDARD 11
    CXX_STANDARD_REQUIRED YES
    CXX_EXTENSIONS NO
)

# message(STATUS "runtime = ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
# message(STATUS "binary = ${CMAKE_CURRENT_BINARY_DIR}")

# Copy font files, etc
add_custom_command(
        TARGET ${NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/data/
                ${CMAKE_CURRENT_BINARY_DIR}/data/)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TESTBED_SOURCE_FILES})
