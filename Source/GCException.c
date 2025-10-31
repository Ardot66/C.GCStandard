#include "GCException.h"
#include "GCTime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <backtrace.h>

thread_local struct GCInternalExceptionThreadData GCInternalExceptionThreadData = {0};
static Exception FallbackException = {.IsFallbackException = 1};
static int FallbackExceptionInUse = 0;
static struct backtrace_state *BacktraceState = NULL;
    
static struct backtrace_state *GetBacktraceState()
{
    // TODO: set up error callbacks.
    if(BacktraceState == NULL)
        BacktraceState = backtrace_create_state(NULL, 1, NULL, NULL);

    return BacktraceState;
}

void GCInternalExceptionJump(GCInternalExitFunc nextExit, Exception *exception)
{
    while(nextExit != NULL)
        nextExit = (GCInternalExitFunc)nextExit(exception);

    if(GCInternalExceptionThreadData.NextBufRef == NULL)
    {
        ExceptionPrint(GCInternalExceptionThreadData.Exception);
        fprintf(stderr, "Fatal error: no try statement found to catch current exception, aborting\n");
        exit(GCInternalExceptionThreadData.Exception->Type);
    }
    
    longjmp(*GCInternalExceptionThreadData.NextBufRef, -1);
}

static int BacktraceCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)filename; (void)lineno;

    Exception *exception = data;
    if(function == NULL || exception->BacktraceFrames >= GC_INTERNAL_BACKTRACE_FRAMES)
        return 1;

    exception->Backtrace[exception->BacktraceFrames] = pc;
    exception->BacktraceFrames++;

    // Stop the backtrace at main.
    return strcmp(function, "main") == 0;
}

Exception *GCInternalExceptionCreate(int type, const char *message, uint64_t line, const char *file, const char *function)
{
    // If an exception already exists, then this exception must be occurring within an exit block, and thus must be handled
    // as a special case. Because only one exception can be handled at once, the exception that would typically be created
    // here will be printed straight away with a special warning message and the older exception will be propagated along.
    if(GCInternalExceptionThreadData.Exception)
    {
        Exception tempException = 
        {
            .Type = type,
            .Message = message,
            .Line = line,
            .File = file,
            .Function = function,
            .BacktraceFrames = 0
        };

        backtrace_full(GetBacktraceState(), 1, BacktraceCallback, NULL, &tempException);
        fprintf(
            stderr,
            "Warning: The following exception was thrown within an exit block while another exception was unwinding the stack, and as such the block terminated early, "
            "potentially skipping object destruction. Due to the design of GCException, the inciting exception will be printed immediately and not handled directly.\n"
        );
        ExceptionPrint(&tempException);

        return GCInternalExceptionThreadData.Exception;
    }

    Exception *exception = malloc(sizeof(*exception));

    if(exception == NULL)
    {
        for(uint32_t x = 0; FallbackExceptionInUse; x++)
        {
            Timespec sleep = SecsToTimespec(0.1f);
            nanosleep(&sleep, NULL);

            if(x > 50)
            {
                fprintf(stderr, "Fatal error: unable to allocate or acquire backup exception object, exiting\n");
                exit(-1);
            }
        }
    
        FallbackExceptionInUse = 1;
        exception = &FallbackException;
    }
    else
        exception->IsFallbackException = 0;

    exception->Type = type;
    exception->Message = message;
    exception->Line = line;
    exception->File = file;
    exception->Function = function;
    exception->BacktraceFrames = 0;

    backtrace_full(GetBacktraceState(), 1, BacktraceCallback, NULL, exception);

    GCInternalExceptionThreadData.Exception = exception;
    return exception;
}

static int BacktracePrintCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)pc;
    int *depth = data;
    fprintf(stderr, "%d - %s - %s() - Line %d\n", *depth, filename , function, lineno);
    return 0;
}

void ExceptionPrint(Exception *exception)
{
    fprintf(stderr, "%s: %s at %s - %s() - Line %zu\n", strerror(exception->Type), exception->Message, exception->File, exception->Function, exception->Line);

    for(int x = 0; x < exception->BacktraceFrames; x++)
    {
        int depth = exception->BacktraceFrames - x - 1;
        backtrace_pcinfo(GetBacktraceState(), exception->Backtrace[x], BacktracePrintCallback, NULL, &depth);
    }
}

void ExceptionFree(Exception *exception)
{
    if(exception == NULL)
        return;

    if(exception->IsFallbackException)
    {
        FallbackExceptionInUse = 0;
        return;
    }

    free(exception);
}