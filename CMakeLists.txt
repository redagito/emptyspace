cmake_minimum_required(VERSION 3.25.0)
project(EmptySpace)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set(CXX_STANDARD_REQUIRED ON)

if (MSVC)
	# Ignore warnings about missing pdb
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /ignore:4099")
	set(CMAKE_STATIC_LINKER_FLAGS "${CMAKE_STATIC_LINKER_FLAGS} /ignore:4099")
endif()

find_package(assimp REQUIRED)
find_package(glad REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)
find_package(PhysX REQUIRED)
find_package(stb REQUIRED)
find_package(fmtlog REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB_RECURSE SOURCE_FILES CONFIGURE_DEPENDS 
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.cxx
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.h
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.hpp
	${CMAKE_CURRENT_SOURCE_DIR}/source/*.glsl
)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE_FILES})

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

set_target_properties(${PROJECT_NAME} PROPERTIES 
	CXX_STANDARD 17
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE assimp::assimp
    PRIVATE glad::glad
    PRIVATE glfw
    PRIVATE glm::glm
	PRIVATE physx::physx
    PRIVATE stb::stb
    PRIVATE fmtlog::fmtlog
    PRIVATE OpenGL::GL
)

target_include_directories(${PROJECT_NAME} 
	PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/source
)

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/WX /W4>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Werror>
  # -Wextra -Wpedantic 
)

file (COPY ${CMAKE_CURRENT_SOURCE_DIR}/data DESTINATION ${CMAKE_CURRENT_BINARY_DIR})