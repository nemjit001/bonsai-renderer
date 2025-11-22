#include "bonsai/core/assert.hpp"

#include <cstdio>

void bonsai_assertion_report(char const* file, int line, char const* expr)
{
    fprintf(stderr, "Assertion failed: %s:%d \"%s\"\n", file, line, expr);
}
