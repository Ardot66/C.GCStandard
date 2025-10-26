#ifndef __GC_ASSERT__
#define __GC_ASSERT__

#include <stdint.h>

[[__noreturn__]]
void GCInternalAssertExit(const char *statementString, const char *file, const char *function, const uint64_t line);

#ifdef GC_NO_ASSERT
    #define Assert(statement)
#else
    #define Assert(statement) if(!(statement)) GCInternalAssertExit(#statement, __FILE__, __func__, __LINE__)
#endif

#endif