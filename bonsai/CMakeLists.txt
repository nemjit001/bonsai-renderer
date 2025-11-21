project("Bonsai" VERSION 0.1.0)

add_library(bonsai STATIC
        include/bonsai/application.hpp
        include/bonsai/bonsai_api.hpp
        include/bonsai/engine.hpp
        include/bonsai/entrypoint.hpp
        src/application.cpp
        src/engine.cpp
)
target_compile_features(bonsai PUBLIC cxx_std_17)
target_include_directories(bonsai PRIVATE src PUBLIC include)
target_link_libraries(bonsai PRIVATE PUBLIC)
target_extended_warnings(bonsai)

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
    add_executable(bonsai_tests
            tests/sanity.cpp
    )
    target_link_libraries(bonsai_tests PRIVATE GTest::gtest_main bonsai)
    target_track_dll_dependencies(bonsai_tests)

    gtest_discover_tests(bonsai_tests)
endif()
