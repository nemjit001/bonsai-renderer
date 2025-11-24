project("Bonsai Viewer" VERSION 0.1.0)

# Set up application library
add_library(bonsai_viewer SHARED
        src/bonsai_viewer.cpp
        src/bonsai_viewer.hpp
)
target_include_directories(bonsai_viewer PUBLIC include PRIVATE src)
target_link_libraries(bonsai_viewer PUBLIC bonsai_core)

# Set up application binary
add_executable(bonsai_viewer_app
        src/entrypoint.cpp
)
target_link_libraries(bonsai_viewer_app PRIVATE bonsai_viewer)
target_track_dll_dependencies(bonsai_viewer_app)

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
endif()
