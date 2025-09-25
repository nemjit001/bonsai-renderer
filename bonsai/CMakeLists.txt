project("Bonsai" VERSION 0.1.0)

# Set up bonsai core library
add_library(bonsai_core STATIC
        src/logger.cpp
        src/logger.hpp
        src/platform.hpp
        src/platform_vulkan.hpp
        src/platform/platform_sdl.cpp
        src/platform/platform_sdl.hpp
        src/platform/platform_vulkan_sdl.cpp
)
target_include_directories(bonsai_core PUBLIC src/)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_link_libraries(bonsai_core PUBLIC SDL3::SDL3 spdlog::spdlog volk::volk_headers)
target_extended_warnings(bonsai_core)

# Set up bonsai executable
add_executable(bonsai
        src/main.cpp
)
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
    )
    target_compile_features(bonsai_tests PRIVATE cxx_std_17)
    target_link_libraries(bonsai_tests PRIVATE GTest::gtest_main bonsai_core)
    target_extended_warnings(bonsai_tests)

    gtest_discover_tests(bonsai_tests)
endif()
