# Configure GLM
add_subdirectory(glm EXCLUDE_FROM_ALL SYSTEM)

# Configure GoogleTest
set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
add_subdirectory(googletest EXCLUDE_FROM_ALL SYSTEM)

# Configure SDL3
add_subdirectory(SDL EXCLUDE_FROM_ALL SYSTEM)

# Configure SPDLog
add_subdirectory(spdlog EXCLUDE_FROM_ALL SYSTEM)

# Configure TinyObjLoader
add_subdirectory(tinyobjloader EXCLUDE_FROM_ALL SYSTEM)

# Configure Volk
add_subdirectory(volk EXCLUDE_FROM_ALL SYSTEM)

# Configure VMA
add_subdirectory(VMA EXCLUDE_FROM_ALL SYSTEM)
