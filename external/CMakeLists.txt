# GLM
add_subdirectory("glm" EXCLUDE_FROM_ALL SYSTEM)

# GoogleTest
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
add_subdirectory("googletest" EXCLUDE_FROM_ALL SYSTEM)

# SDL3
add_subdirectory("SDL" EXCLUDE_FROM_ALL SYSTEM)

# SPDLog
add_subdirectory("spdlog" EXCLUDE_FROM_ALL SYSTEM)

# Volk
add_subdirectory("volk" EXCLUDE_FROM_ALL SYSTEM)

# VMA
add_subdirectory("VMA" EXCLUDE_FROM_ALL SYSTEM)
