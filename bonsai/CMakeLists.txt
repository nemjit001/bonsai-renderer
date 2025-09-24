project("Bonsai" VERSION 0.1.0)

# Set up Bonsai target
add_executable(Bonsai
        src/main.cpp
        src/logger.cpp
        src/logger.hpp
        src/platform.hpp
        src/platform_sdl.cpp
)
target_compile_features(Bonsai PRIVATE cxx_std_17)
target_link_libraries(Bonsai PRIVATE SDL3::SDL3 spdlog::spdlog)

# Copy DLL dependencies to target binary dir
add_custom_command(TARGET Bonsai POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:Bonsai> $<TARGET_FILE_DIR:Bonsai>
    COMMAND_EXPAND_LISTS
)
