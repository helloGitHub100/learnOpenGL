set (TESTBED_SOURCE_FILES
	draw.cpp
	draw.h
	main.cpp
	settings.h
	settings.cpp
	test.cpp
	test.h
	tests/add_pair.cpp
	tests/apply_force.cpp
	tests/body_types.cpp
	tests/box_stack.cpp
	tests/breakable.cpp
	tests/bridge.cpp
	tests/bullet_test.cpp
	tests/cantilever.cpp
	tests/car.cpp
	tests/chain.cpp
	tests/chain_problem.cpp
	tests/character_collision.cpp
	tests/circle_stack.cpp
	tests/collision_filtering.cpp
	tests/collision_processing.cpp
	tests/compound_shapes.cpp
	tests/confined.cpp
	tests/continuous_test.cpp
	tests/convex_hull.cpp
	tests/conveyor_belt.cpp
	tests/distance_joint.cpp
	tests/distance_test.cpp
	tests/dominos.cpp
	tests/dump_loader.cpp
	tests/dynamic_tree.cpp
	tests/edge_shapes.cpp
	tests/edge_test.cpp
	tests/friction.cpp
	tests/gear_joint.cpp
	tests/heavy1.cpp
	tests/heavy2.cpp
	tests/mobile_balanced.cpp
	tests/mobile_unbalanced.cpp
	tests/motor_joint.cpp
	tests/pinball.cpp
	tests/platformer.cpp
	tests/polygon_collision.cpp
	tests/polygon_shapes.cpp
	tests/prismatic_joint.cpp
	tests/pulley_joint.cpp
	tests/pyramid.cpp
	tests/ray_cast.cpp
	tests/restitution.cpp
	tests/revolute_joint.cpp
	tests/rope.cpp
	tests/sensor.cpp
	tests/shape_cast.cpp
	tests/shape_editing.cpp
	tests/skier.cpp
	tests/slider_crank_1.cpp
	tests/slider_crank_2.cpp
	tests/theo_jansen.cpp
	tests/tiles.cpp
	tests/time_of_impact.cpp
	tests/tumbler.cpp
	tests/web.cpp
	tests/wheel_joint.cpp
	tests/wrecking_ball.cpp
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
