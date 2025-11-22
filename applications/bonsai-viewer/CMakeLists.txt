project("Bonsai Viewer" VERSION 0.1.0)

# Set up application library
add_library(bonsai_viewer_lib SHARED
        src/bonsai_viewer.cpp
        src/bonsai_viewer.hpp
)
target_include_directories(bonsai_viewer_lib PRIVATE src PUBLIC include)
target_link_libraries(bonsai_viewer_lib PRIVATE PUBLIC bonsai_core)
target_compile_definitions(bonsai_viewer_lib PUBLIC BONSAI_EXPORT_SYMBOLS=1)

# Set up application binary
add_executable(bonsai_viewer
        src/entrypoint.cpp
)
target_link_libraries(bonsai_viewer PRIVATE bonsai_viewer_lib)
target_track_dll_dependencies(bonsai_viewer)

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
endif()
