#pragma once
#ifndef BONSAI_RENDERER_ASSERT_HPP
#define BONSAI_RENDERER_ASSERT_HPP

void bonsai_assertion_report(char const* file, int line, char const* expr);

// Insert platform dependent inline assembly to replicate debug break behaviour
#if __x86_64__
    #define BONSAI_PLATFORM_DEBUG_TRAP()    __asm volatile "int3"
#else
    #define BONSAI_PLATFORM_DEBUG_TRAP()
#endif

#if _MSC_VER
    #define BONSAI_DEBUG_BREAK()    __debugbreak()
#elif __GNUC__ && __has_builtin(__builtin_debugtrap)
    #define BONSAI_DEBUG_BREAK()    __builtin_debugtrap()
#else
    #define BONSAI_DEBUG_BREAK()    BONSAI_PLATFORM_DEBUG_TRAP()
#endif

#if BONSAI_USE_ASSERTIONS
    #define BONSAI_ASSERT(expr) ((!!(expr)) || (bonsai_assertion_report(__FILE__, __LINE__, #expr), BONSAI_DEBUG_BREAK(), 0))
#else
    #define BONSAI_ASSERT(expr)
#endif

#endif //BONSAI_RENDERER_ASSERT_HPP