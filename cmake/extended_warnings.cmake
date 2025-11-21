function(target_extended_warnings TARGET_NAME)
    # Enable extended warnings, assuming non-msvc compilers use gcc-like flags
    if (MSVC)
        target_compile_options(${TARGET_NAME} PUBLIC /W4 /permissive-)
    else()
        target_compile_options(${TARGET_NAME} PUBLIC -Wall -Wextra -Wpedantic -Wunused -Wshadow)
    endif()
endfunction()
