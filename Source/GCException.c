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

void GCInternalExceptionJump(GCInternalExitFunc nextExit)
{
    if(nextExit != NULL)
        nextExit();

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
    if(exception->BacktraceFrames >= GC_INTERNAL_BACKTRACE_FRAMES)
        return 1;

    exception->Backtrace[exception->BacktraceFrames] = pc;
    exception->BacktraceFrames++;

    // Stop the backtrace at main.
    return strcmp(function, "main") == 0;
}

void GCInternalExceptionCreate(int type, const char *message, uint64_t line, const char *file, const char *function)
{
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