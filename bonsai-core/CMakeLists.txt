project("Bonsai Core" VERSION 0.1.0)

add_library(bonsai_core STATIC
        # Public sources
        include/bonsai/core/dylib_loader.hpp
        include/bonsai/core/platform.hpp
        include/bonsai/application.hpp
        include/bonsai/bonsai_api.hpp
        include/bonsai/engine.hpp
        include/bonsai/entrypoint.hpp
        # Private sources
        src/core/dylib_loader_unix.cpp
        src/core/dylib_loader_win32.cpp
        src/core/platform_sdl.cpp
        src/application.cpp
        src/engine.cpp
)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_include_directories(bonsai_core PRIVATE src PUBLIC include)
target_link_libraries(bonsai_core PRIVATE SDL3::SDL3 PUBLIC spdlog::spdlog)
target_extended_warnings(bonsai_core)

if (UNIX)
    target_link_libraries(bonsai_core PRIVATE dl)
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
