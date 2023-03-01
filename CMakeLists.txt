
cmake_minimum_required (VERSION 3.16.0)
project (Untitled)

set (CMAKE_CXX_STANDARD 11)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/../)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)

add_subdirectory(src)