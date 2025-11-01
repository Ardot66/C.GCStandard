#ifndef __GC_INTERNAL_THREAD_LOCAL_GLOBALS__
#define __GC_INTERNAL_THREAD_LOCAL_GLOBALS__

#include <setjmp.h>
#include <stdint.h>

typedef struct Exception Exception;
typedef void (* GCInternalExitFuncBox)();
typedef GCInternalExitFuncBox (* GCInternalExitFunc)(Exception *gcInternalException);

// Thread local data needs to be wrapped in a single struct, weird overlapping stuff was happening otherwise.
struct GCInternalThreadData
{
    jmp_buf *NextBufRef;
    GCInternalExitFunc NextExitFunc;
    Exception *Exception;
    bool HeapCallbackActive;
};

extern thread_local struct GCInternalThreadData GCInternalThreadData;

#endif