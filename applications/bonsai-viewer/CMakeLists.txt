project("Bonsai Viewer" VERSION 0.1.0)

# Set up application library
add_library(bonsai_viewer SHARED
        src/bonsai_viewer.cpp
        src/bonsai_viewer.hpp
)
target_include_directories(bonsai_viewer PRIVATE src PUBLIC include)
target_link_libraries(bonsai_viewer PRIVATE PUBLIC bonsai_core)
target_compile_definitions(bonsai_viewer PUBLIC BONSAI_EXPORT_SYMBOLS=1)

# Set up application binary
add_executable(bonsai_viewer_exe
        src/entrypoint.cpp
)
target_link_libraries(bonsai_viewer_exe PRIVATE bonsai_viewer)
set_target_properties(bonsai_viewer_exe PROPERTIES OUTPUT_NAME bonsai_viewer)
target_track_dll_dependencies(bonsai_viewer_exe)

if (BONSAI_BUILD_TESTS)
    include(GoogleTest)
endif()
