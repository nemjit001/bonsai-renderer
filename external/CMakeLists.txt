# Set up DirectXShaderCompiler
include(DirectXShaderCompiler/cmake/caches/PredefinedParams.cmake)
set(LLVM_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(HLSL_INCLUDE_TESTS OFF CACHE BOOL "" FORCE)
set(SPIRV_BUILD_TESTS OFF CACHE BOOL "" FORCE)
add_subdirectory(DirectXShaderCompiler SYSTEM EXCLUDE_FROM_ALL)

# Set up GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(GoogleTest SYSTEM EXCLUDE_FROM_ALL)

# Set up ImGui libs
add_library(imgui STATIC
        imgui/imgui.cpp
        imgui/imgui.h
        imgui/imgui_demo.cpp
        imgui/imgui_draw.cpp
        imgui/imgui_internal.h
        imgui/imgui_tables.cpp
        imgui/imgui_widgets.cpp
        imgui/imstb_rectpack.h
        imgui/imstb_textedit.h
        imgui/imstb_truetype.h
)
target_include_directories(imgui PUBLIC imgui)

add_library(imgui_sdl3 STATIC imgui/backends/imgui_impl_sdl3.cpp imgui/backends/imgui_impl_sdl3.h)
target_link_libraries(imgui_sdl3 PUBLIC imgui SDL3::SDL3)

add_library(imgui_vulkan STATIC imgui/backends/imgui_impl_vulkan.cpp imgui/backends/imgui_impl_vulkan.h)
target_link_libraries(imgui_vulkan PUBLIC imgui volk::volk_headers)
target_compile_definitions(imgui_vulkan PUBLIC IMGUI_IMPL_VULKAN_USE_VOLK)

# Set up SPD Log
set(SPDLOG_WCHAR_SUPPORT ON CACHE BOOL "" FORCE)
add_subdirectory(spdlog SYSTEM EXCLUDE_FROM_ALL)

# Set up SDL
add_subdirectory(SDL SYSTEM EXCLUDE_FROM_ALL)

# Set up VMA
add_subdirectory(VMA SYSTEM EXCLUDE_FROM_ALL)

# Set up Volk
add_subdirectory(volk SYSTEM EXCLUDE_FROM_ALL)
