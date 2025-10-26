project("Bonsai" VERSION 0.1.0)

# Set up bonsai core library
configure_file(bonsai_config.hpp.in bonsai_config.hpp)
add_library(bonsai_core STATIC
        # Platform API
        src/platform/assert.hpp
        src/platform/logger.cpp
        src/platform/logger.hpp
        src/platform/platform.hpp
        src/platform/platform_vulkan.hpp
        src/platform/platform_win32.hpp
        src/platform/sdl/platform_sdl.cpp
        src/platform/sdl/platform_sdl.hpp
        src/platform/sdl/platform_vulkan_sdl.cpp
        src/platform/sdl/platform_win32_sdl.cpp
        # Core API
        src/core/allocator.cpp
        src/core/allocator.hpp
        src/core/die.hpp
        src/core/timer.hpp
        # RHI API
        src/rhi/rhi.hpp
        src/rhi/rhi_vulkan.cpp
        src/rhi/rhi_vulkan.hpp
        src/rhi/vulkan/vulkan_buffer.cpp
        src/rhi/vulkan/vulkan_buffer.hpp
        src/rhi/vulkan/vulkan_command_buffer.cpp
        src/rhi/vulkan/vulkan_command_buffer.hpp
        src/rhi/vulkan/vulkan_texture.cpp
        src/rhi/vulkan/vulkan_texture.hpp
        # Render Graph API
        src/render_graph/render_graph.cpp
        src/render_graph/render_graph.hpp
        # Asset API
        src/assets/asset.hpp
        src/assets/asset_cache.cpp
        src/assets/asset_cache.hpp
        src/assets/model.cpp
        src/assets/model.hpp
        src/assets/model/material.cpp
        src/assets/model/material.hpp
        src/assets/model/mesh.cpp
        src/assets/model/mesh.hpp
        # World API
        src/world/entity.cpp
        src/world/entity.hpp
        src/world/world.cpp
        src/world/world.hpp
        src/world/world_manager.cpp
        src/world/world_manager.hpp
        # Component API
        src/components/camera_component.cpp
        src/components/camera_component.hpp
        src/components/render_component.cpp
        src/components/render_component.hpp
        # Engine API
        src/engine.cpp
        src/engine.hpp
        src/rhi/vulkan/vulkan_helpers.hpp
)
target_include_directories(bonsai_core PUBLIC src/ ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_link_libraries(bonsai_core PUBLIC glm::glm nlohmann_json::nlohmann_json spdlog::spdlog tinyobjloader)
target_extended_warnings(bonsai_core)

if (BONSAI_USE_SDL)
    target_link_libraries(bonsai_core PUBLIC SDL3::SDL3)
    target_compile_definitions(bonsai_core PUBLIC BONSAI_PLATFORM_SDL=1)
endif()

if (BONSAI_USE_VULKAN)
    target_link_libraries(bonsai_core PUBLIC GPUOpen::VulkanMemoryAllocator volk::volk_headers)
    target_compile_definitions(bonsai_core PUBLIC BONSAI_USE_VULKAN=1)
endif()

# Set up bonsai executable
add_executable(bonsai
        src/main.cpp
)
target_include_directories(bonsai PRIVATE src/)
target_link_libraries(bonsai PRIVATE bonsai_core)
target_extended_warnings(bonsai)
target_track_dll_dependencies(bonsai)

# Set up bonsai unit tests
if (BONSAI_TEST)
    include(GoogleTest)
    add_executable(bonsai_tests
            tests/sanity.cpp
            tests/platform_tests.cpp
            tests/core_allocator_tests.cpp
            tests/rhi_tests.cpp
            tests/render_graph_tests.cpp
            tests/assets_tests.cpp
            tests/world_tests.cpp
    )
    target_link_libraries(bonsai_tests PRIVATE GTest::gtest_main bonsai_core)
    target_extended_warnings(bonsai_tests)
    target_track_dll_dependencies(bonsai_tests)
    if (BONSAI_PLATFORM_SURFACE_TESTS)
        target_compile_definitions(bonsai_tests PRIVATE BONSAI_PLATFORM_SURFACE_TESTS=1)
    endif()

    gtest_discover_tests(bonsai_tests)
endif()
