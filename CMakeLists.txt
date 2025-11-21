cmake_minimum_required(VERSION 3.25)
project("Bonsai Renderer")
enable_testing()

# Options
option(BONSAI_BUILD_TESTS "Enable unit test targets" ON)

# Set CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")
include(cmake/extended_warnings.cmake)
include(cmake/track_dll_dependencies.cmake)

# Configure default CMake shared library behaviour
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)

# Configure sub projects
add_subdirectory(external)
add_subdirectory(bonsai)
add_subdirectory(applications)
