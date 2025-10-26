#include "GCAssert.h"
#include <stdio.h>
#include <stdlib.h>

[[__noreturn__]]
void GCInternalAssertExit(const char *statementString, const char *file, const char *function, const uint64_t line)
{
    fprintf(stderr, "Assertion failed: (%s) at %s - %s() - line %llu\n", statementString, file, function, line);
    exit(EXIT_FAILURE);
}