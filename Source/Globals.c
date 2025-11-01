#include "GCInternalGlobals.h"
#include <backtrace.h>

thread_local struct GCInternalThreadData GCInternalThreadData = {0};
static struct backtrace_state *BacktraceState = NULL;
    
struct backtrace_state *GCInternalGetBacktraceState()
{
    // TODO: set up error callbacks.
    if(BacktraceState == NULL)
        BacktraceState = backtrace_create_state(NULL, 1, NULL, NULL);

    return BacktraceState;
}
