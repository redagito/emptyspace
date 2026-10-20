cmake_minimum_required(VERSION 3.25.0)
project(EmptySpace LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT App)

enable_testing()

add_subdirectory(EmptySpace)
add_subdirectory(App)
add_subdirectory(Test)
