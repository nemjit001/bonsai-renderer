project("Bonsai" VERSION 0.1.0)

# Set up bonsai core library
add_library(bonsai_core STATIC
        src/engine.cpp
        src/engine.hpp
        src/core/die.hpp
        src/core/logger.cpp
        src/core/logger.hpp
        src/platform/platform.hpp
        src/platform/platform_vulkan.hpp
        src/platform/sdl/platform_sdl.cpp
        src/platform/sdl/platform_sdl.hpp
        src/platform/sdl/platform_vulkan_sdl.cpp
)
target_include_directories(bonsai_core PUBLIC src/)
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

# Set up bonsai executable
configure_file(bonsai_config.hpp.in bonsai_config.hpp)
add_executable(bonsai
        src/main.cpp
)
target_include_directories(bonsai PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(bonsai PRIVATE cxx_std_17)
target_link_libraries(bonsai PRIVATE bonsai_core)
target_extended_warnings(bonsai)

# Copy DLL dependencies to target binary dir
add_custom_command(TARGET bonsai POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:bonsai> $<TARGET_FILE_DIR:bonsai>
    COMMAND_EXPAND_LISTS
)

# Set up bonsai unit tests
if (BONSAI_TEST)
    include(GoogleTest)
    add_executable(bonsai_tests
            tests/sanity.cpp
            tests/platform_tests.cpp
    )
    target_compile_features(bonsai_tests PRIVATE cxx_std_17)
    target_link_libraries(bonsai_tests PRIVATE GTest::gtest_main bonsai_core)
    target_extended_warnings(bonsai_tests)

    gtest_discover_tests(bonsai_tests)
endif()
