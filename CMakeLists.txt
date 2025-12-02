cmake_minimum_required(VERSION 3.25)
project("Bonsai Renderer")
enable_testing()

# Options
option(BONSAI_BUILD_TESTS "Enable unit test targets" ON)
option(BONSAI_USE_ASSERTIONS "Enable assertions in all build types" ON)
option(BONSAI_USE_VULKAN "Enable the Vulkan render backend for Bonsai" ON)
option(BONSAI_USE_VENDORED_DXC "Use the vendored DirectX Shader Compiler, will significantly increase build times..." OFF)

# Set CMake module path
set(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH}" "${CMAKE_SOURCE_DIR}/cmake/modules")
include(cmake/extended_warnings.cmake)
include(cmake/track_dll_dependencies.cmake)

# Configure default CMake target behaviour
set(CMAKE_C_VISIBILITY_PRESET hidden)
set(CMAKE_CXX_VISIBILITY_PRESET hidden)
set(CMAKE_VISIBILITY_INLINES_HIDDEN ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure sub projects
add_subdirectory(external)
add_subdirectory(bonsai-core)
add_subdirectory(applications)
