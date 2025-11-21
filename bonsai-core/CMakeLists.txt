project("Bonsai Core" VERSION 0.1.0)

add_library(bonsai_core STATIC
        include/bonsai/application.hpp
        include/bonsai/bonsai_api.hpp
        src/application.cpp
)
target_compile_features(bonsai_core PUBLIC cxx_std_17)
target_include_directories(bonsai_core PRIVATE src PUBLIC include)
target_link_libraries(bonsai_core PRIVATE PUBLIC)
target_extended_warnings(bonsai_core)

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
    add_executable(bonsai_core_tests
            tests/sanity.cpp
    )
    target_link_libraries(bonsai_core_tests PRIVATE GTest::gtest_main bonsai_core)
    # target_track_dll_dependencies(bonsai_core_tests)

    gtest_discover_tests(bonsai_core_tests)
endif()
