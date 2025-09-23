cmake_minimum_required(VERSION 3.25)
project("Bonsai Renderer")
enable_testing()

# Set CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")

add_subdirectory(external)
add_subdirectory(bonsai)
