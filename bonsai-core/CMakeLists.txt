project("Bonsai Core" VERSION 0.1.0)

add_library(bonsai_core STATIC
        # Public sources
        include/bonsai/core/assert.hpp
        include/bonsai/core/dylib_loader.hpp
        include/bonsai/core/fatal_exit.hpp
        include/bonsai/core/logger.hpp
        include/bonsai/core/platform.hpp
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
        src/application.cpp
        src/engine.cpp
)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_include_directories(bonsai_core PUBLIC include PRIVATE src)
target_link_libraries(bonsai_core PUBLIC imgui spdlog::spdlog PRIVATE imgui_sdl3 SDL3::SDL3)
target_compile_definitions(bonsai_core PUBLIC BONSAI_EXPORT_SYMBOLS=1)
target_extended_warnings(bonsai_core)

if (BONSAI_USE_VULKAN)
    find_package(Vulkan REQUIRED)
    message(STATUS "Bonsai: Enabled Vulkan backend")

    target_sources(bonsai_core PUBLIC
            src/render_backend/vulkan/vk_check.hpp
            src/render_backend/vulkan_render_backend.cpp
            src/render_backend/vulkan_render_backend.hpp
    )
    target_link_libraries(bonsai_core PUBLIC Vulkan::Headers PRIVATE imgui_vulkan volk::volk_headers)
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
    )
    target_link_libraries(bonsai_core_tests PRIVATE GTest::gtest_main bonsai_core)
    target_track_dll_dependencies(bonsai_core_tests)

    gtest_discover_tests(bonsai_core_tests)
endif()
