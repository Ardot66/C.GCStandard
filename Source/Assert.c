#include "GCInternalGlobals.h"
#include "GCAssert.h"
#include <stdlib.h>
#include <stdio.h>
#include <backtrace.h>
#include <string.h>

static int BacktraceCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)data; (void)pc;
    if(function == NULL || filename == NULL)
        return 1;

    fprintf(stderr, "   %s() in %s line %d\n", function, filename, lineno);
    return strcmp(function, "main") == 0;
}

[[__noreturn__]]
void GCInternalAssertExit(const char *statementString, const char *file, const char *function, const uint64_t line)
{
    fprintf(stderr, "Assertion failed: (%s) at %s() in %s line %llu\n", statementString, function, file, line);
    backtrace_full(GCInternalGetBacktraceState(), 1, BacktraceCallback, NULL, NULL);
    exit(EXIT_FAILURE);
}