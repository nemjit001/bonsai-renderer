project("Bonsai Core" VERSION 0.1.0)

configure_file(bonsai_config.hpp.in bonsai_config.hpp)
add_library(bonsai_core STATIC
        # Public sources
        include/bonsai/core/assert.hpp
        include/bonsai/core/dylib_loader.hpp
        include/bonsai/core/fatal_exit.hpp
        include/bonsai/core/logger.hpp
        include/bonsai/core/platform.hpp
        include/bonsai/render_backend/render_backend.hpp
        include/bonsai/systems/renderer.hpp
        include/bonsai/application.hpp
        include/bonsai/bonsai_export.hpp
        include/bonsai/engine_api.hpp
        include/bonsai/engine.hpp
        include/bonsai/entrypoint.hpp
        # Private sources
        src/core/assert.cpp
        src/core/dylib_loader_unix.cpp
        src/core/dylib_loader_win32.cpp
        src/core/logger.cpp
        src/core/platform_sdl.cpp
        src/render_backend/render_backend.cpp
        src/render_backend/shader_compiler.cpp
        src/render_backend/shader_compiler.hpp
        src/systems/renderer.cpp
        src/application.cpp
        src/engine_api.cpp
        src/engine.cpp
)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_include_directories(bonsai_core PUBLIC include PRIVATE src ${CMAKE_CURRENT_BINARY_DIR})
target_link_libraries(bonsai_core PUBLIC imgui spdlog::spdlog PRIVATE dxcompiler imgui_sdl3 SDL3::SDL3)
target_compile_definitions(bonsai_core PUBLIC BONSAI_EXPORT_SYMBOLS=1)
target_extended_warnings(bonsai_core)

if (BONSAI_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    message(STATUS "Bonsai: Enabled Vulkan backend")

    # TODO(nemjit001): Make this a separate library, will make engine/renderer split more obvious
    target_sources(bonsai_core PRIVATE
            src/render_backend/vulkan/enum_conversion.hpp
            src/render_backend/vulkan/enum_conversion.cpp
            src/render_backend/vulkan/spirv_reflector.cpp
            src/render_backend/vulkan/spirv_reflector.hpp
            src/render_backend/vulkan/vk_check.hpp
            src/render_backend/vulkan/vulkan_buffer.cpp
            src/render_backend/vulkan/vulkan_buffer.hpp
            src/render_backend/vulkan/vulkan_render_commands.cpp
            src/render_backend/vulkan/vulkan_render_commands.hpp
            src/render_backend/vulkan/vulkan_shader_pipeline.cpp
            src/render_backend/vulkan/vulkan_shader_pipeline.hpp
            src/render_backend/vulkan/vulkan_texture.cpp
            src/render_backend/vulkan/vulkan_texture.hpp
            src/render_backend/vulkan_render_backend.cpp
            src/render_backend/vulkan_render_backend.hpp
    )
    target_link_libraries(bonsai_core PUBLIC Vulkan::Headers PRIVATE GPUOpen::VulkanMemoryAllocator imgui_vulkan spirv-reflect-static volk::volk_headers)
    target_compile_definitions(bonsai_core PUBLIC BONSAI_USE_VULKAN=1)
endif()

if (BONSAI_USE_ASSERTIONS)
    target_compile_definitions(bonsai_core PUBLIC BONSAI_USE_ASSERTIONS=1)
endif()

if (NOT WIN32)
    target_link_libraries(bonsai_core PRIVATE ${CMAKE_DL_LIBS})
endif()

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
    add_executable(bonsai_core_tests
            tests/sanity.cpp
            tests/test_shader_compilation.cpp
    )
    target_include_directories(bonsai_core_tests PUBLIC include PRIVATE src tests)
    target_link_libraries(bonsai_core_tests PRIVATE GTest::gtest_main bonsai_core)
    target_track_dll_dependencies(bonsai_core_tests)

    # Link internally used libraries for testing
    if (BONSAI_USE_VULKAN)
        target_link_libraries(bonsai_core_tests PRIVATE dxcompiler spirv-reflect-static)
        target_compile_definitions(bonsai_core_tests PUBLIC BONSAI_USE_VULKAN=1)
    endif()

    gtest_discover_tests(bonsai_core_tests)
endif()
