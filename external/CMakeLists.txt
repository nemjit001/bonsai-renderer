set(gtest_force_shared_crt OFF CACHE BOOL "" FORCE)
add_subdirectory("googletest" EXCLUDE_FROM_ALL SYSTEM)

add_subdirectory("SDL" EXCLUDE_FROM_ALL SYSTEM)

add_subdirectory("spdlog" EXCLUDE_FROM_ALL SYSTEM)

add_subdirectory("volk" EXCLUDE_FROM_ALL SYSTEM)
