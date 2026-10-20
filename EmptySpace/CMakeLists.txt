cmake_minimum_required(VERSION 3.25.0)
project(EmptySpace LANGUAGES CXX)

find_package(assimp REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(PhysX REQUIRED)
find_package(stb REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE EMPTYSPACE_SOURCES CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
)
file(GLOB_RECURSE EMPTYSPACE_HEADERS CONFIGURE_DEPENDS
	${CMAKE_CURRENT_SOURCE_DIR}/include/*.hpp
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${EMPTYSPACE_SOURCES} ${EMPTYSPACE_HEADERS})

add_library(${PROJECT_NAME} STATIC ${EMPTYSPACE_SOURCES} ${EMPTYSPACE_HEADERS})

target_include_directories(${PROJECT_NAME}
	PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
	PUBLIC glad::glad glm::glm physx::physx stb::stb OpenGL::GL glfw
	PRIVATE assimp::assimp
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

if(MSVC)
	# Ignore warnings about missing pdb in the PhysX static libraries
	target_link_options(${PROJECT_NAME} INTERFACE /ignore:4099)
endif()
