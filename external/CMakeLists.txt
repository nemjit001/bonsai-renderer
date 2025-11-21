# Set up GoogleTest
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
add_subdirectory(GoogleTest SYSTEM EXCLUDE_FROM_ALL)

# Set up SDL
add_subdirectory(SDL SYSTEM EXCLUDE_FROM_ALL)
