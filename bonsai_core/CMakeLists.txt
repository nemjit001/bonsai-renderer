project("Bonsai Core" VERSION 0.1.0)

set(BONSAI_PUBLIC_SOURCES
        include/core/die.hpp
        include/core/logger.hpp
        include/platform/platform.hpp
        include/platform/platform_vulkan.hpp
)
set(BONSAI_PRIVATE_SOURCES
        src/core/logger.cpp
        src/platform/sdl/platform_sdl.cpp
        src/platform/sdl/platform_sdl.hpp
        src/platform/sdl/platform_vulkan_sdl.cpp
)

# Set up bonsai core library
add_library(bonsai_core STATIC
        ${BONSAI_PUBLIC_SOURCES}
        ${BONSAI_PRIVATE_SOURCES}
)
target_include_directories(bonsai_core PUBLIC include/ PRIVATE src/)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_link_libraries(bonsai_core PUBLIC spdlog::spdlog)
target_extended_warnings(bonsai_core)

if (BONSAI_USE_SDL)
    target_link_libraries(bonsai_core PRIVATE SDL3::SDL3)
    target_compile_definitions(bonsai_core PRIVATE BONSAI_PLATFORM_SDL=1)
endif()

if (BONSAI_USE_VULKAN)
    target_link_libraries(bonsai_core PRIVATE volk::volk_headers)
    target_compile_definitions(bonsai_core PRIVATE BONSAI_USE_VULKAN=1)
endif()

# Set up bonsai core unit tests
if (BONSAI_TEST)
    include(GoogleTest)
    add_executable(bonsai_core_tests
            tests/sanity.cpp
            tests/platform_tests.cpp
    )
    target_compile_features(bonsai_core_tests PRIVATE cxx_std_17)
    target_link_libraries(bonsai_core_tests PRIVATE GTest::gtest_main bonsai_core)
    target_extended_warnings(bonsai_core_tests)
    target_track_dll_dependencies(bonsai_core_tests)

    gtest_discover_tests(bonsai_core_tests)
endif()
