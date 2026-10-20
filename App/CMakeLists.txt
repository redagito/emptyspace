cmake_minimum_required(VERSION 3.25.0)
project(App LANGUAGES CXX)

if(NOT TARGET EmptySpace)
	add_subdirectory(
		${CMAKE_CURRENT_SOURCE_DIR}/../EmptySpace
		${CMAKE_CURRENT_BINARY_DIR}/EmptySpace
	)
endif()

find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)

add_executable(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/source/main.cpp)

target_link_libraries(${PROJECT_NAME}
	PRIVATE EmptySpace glad::glad glfw
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

# The application loads its assets from "data" relative to the working
# directory, so the asset tree has to sit next to the executable.
get_filename_component(EMPTYSPCE_DATA_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../data ABSOLUTE)
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
	COMMAND ${CMAKE_COMMAND} -E copy_directory_if_different
		${EMPTYSPCE_DATA_DIR} $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
)
