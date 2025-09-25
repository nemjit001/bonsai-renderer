project("Bonsai" VERSION 0.1.0)

# Set up bonsai executable
configure_file(bonsai_config.hpp.in bonsai_config.hpp)
add_executable(bonsai
        src/engine.cpp
        src/engine.hpp
        src/main.cpp
)
target_include_directories(bonsai PRIVATE src/ ${CMAKE_CURRENT_BINARY_DIR})
target_compile_features(bonsai PRIVATE cxx_std_17)
target_link_libraries(bonsai PRIVATE bonsai_core)
target_extended_warnings(bonsai)
target_track_dll_dependencies(bonsai)
