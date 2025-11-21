project("Bonsai" VERSION 0.1.0)

add_executable(bonsai
        src/engine.cpp
        src/engine.hpp
        src/main.cpp
)
target_compile_features(bonsai PRIVATE cxx_std_17)
target_include_directories(bonsai PRIVATE src PUBLIC include)
target_link_libraries(bonsai PRIVATE PUBLIC)
target_extended_warnings(bonsai)
target_track_dll_dependencies(bonsai)
