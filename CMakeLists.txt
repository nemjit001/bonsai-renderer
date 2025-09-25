cmake_minimum_required(VERSION 3.25)
project("Bonsai Renderer")
enable_testing()

# Options
option(BONSAI_TEST "Enable unit test targets" ON)
option(BONSAI_USE_SDL "Use SDL as the platform layer backend" ON)
option(BONSAI_USE_VULKAN "Use Vulkan for rendering" ON)

# Set CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/")
include(cmake/extended_warnings.cmake)
include(cmake/target_track_dll_dependencies.cmake)

add_subdirectory(external)
add_subdirectory(bonsai)
add_subdirectory(bonsai_core)
