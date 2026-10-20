cmake_minimum_required(VERSION 3.25.0)
project(Test LANGUAGES CXX)

if(NOT TARGET EmptySpace)
	add_subdirectory(
		${CMAKE_CURRENT_SOURCE_DIR}/../EmptySpace
		${CMAKE_CURRENT_BINARY_DIR}/EmptySpace
	)
endif()

find_package(Catch2 3 REQUIRED)

file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${TEST_SOURCES})

add_executable(${PROJECT_NAME} ${TEST_SOURCES})

target_link_libraries(${PROJECT_NAME}
	PRIVATE EmptySpace Catch2::Catch2WithMain
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# Tests that load assets expect them next to the test executable.
get_filename_component(EMPTYSPCE_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../data ABSOLUTE)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		${EMPTYSPCE_DATA_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)

include(CTest)
add_test(NAME ${PROJECT_NAME} COMMAND ${PROJECT_NAME}
	WORKING_DIRECTORY $<TARGET_FILE_DIR:${PROJECT_NAME}>
)
