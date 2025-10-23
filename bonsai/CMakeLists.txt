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
        src/core/die.hpp
        src/core/timer.hpp
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
        # Rendering API
        src/rendering/render_backend.hpp
        src/rendering/render_world.hpp
        src/rendering/renderer.hpp
        src/rendering/vulkan/render_backend_vulkan.cpp
        src/rendering/vulkan/render_backend_vulkan.hpp
        src/rendering/vulkan/render_world_vulkan.cpp
        src/rendering/vulkan/renderer_vulkan.cpp
        src/rendering/vulkan/use_vma.cpp
        src/rendering/vulkan/use_volk.cpp
        # Engine API
        src/engine.cpp
        src/engine.hpp
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
target_compile_features(bonsai PRIVATE cxx_std_17)
target_link_libraries(bonsai PRIVATE bonsai_core)
target_extended_warnings(bonsai)
target_track_dll_dependencies(bonsai)

# Set up bonsai unit tests
if (BONSAI_TEST)
    include(GoogleTest)
    add_executable(bonsai_tests
            tests/sanity.cpp
            tests/platform_tests.cpp
            tests/world_tests.cpp
            tests/renderer_tests.cpp
            tests/assets_tests.cpp
    )
    target_compile_features(bonsai_tests PRIVATE cxx_std_17)
    target_link_libraries(bonsai_tests PRIVATE GTest::gtest_main bonsai_core)
    target_extended_warnings(bonsai_tests)
    target_track_dll_dependencies(bonsai_tests)

    gtest_discover_tests(bonsai_tests)
endif()
