#include "GCException.h"
#include "GCTime.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

thread_local struct GCInternalExceptionThreadData GCInternalExceptionThreadData = {0};
Exception GCInternalFallbackException = {.IsFallbackException = 1};
int GCInternalFallbackExceptionInUse = 0;

#ifdef __WIN32__
#include <windows.h>
#include <dbghelp.h>
void GCInternalBacktrace(Exception *exception)
{
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    exception->BacktraceFrames = CaptureStackBackTrace(1, GC_INTERNAL_BACKTRACE_FRAMES, exception->Backtrace, NULL);
}

void GCInternalPrintBacktrace(Exception *exception)
{
    HANDLE process = GetCurrentProcess();
    SYMBOL_INFO *symbol = calloc(1, sizeof(*symbol) + 256 * sizeof(char));
    symbol->MaxNameLen = 255;
    symbol->SizeOfStruct = sizeof(SYMBOL_INFO);

    for(int x = 0; x < exception->BacktraceFrames; x++)
    {
        SymFromAddr(process, (DWORD64)(exception->Backtrace[x]), 0, symbol);
        // Checking that x > 0 here to skip the first frame of the backtrace, which is always GCInternalExceptionJump.
        // For some reason SymFromAddr does not work if skipping this first frame.
        if(x > 0)
            fprintf(stderr, "%i: %s() - 0x%p\n", exception->BacktraceFrames - x - 1, symbol->Name, (void *)symbol->Address);
    }

    free(symbol);
}
#elif __GLIBC__ 
#include <execinfo.h>
void GCInternalBacktrace()
{
    if(InitBacktrace())
        return;

    GCInternalExceptionThreadData.Exception.BacktraceFrames = backtrace(GCInternalExceptionThreadData.Exception.Backtrace, BACKTRACE_FRAMES);
}

// TODO, set up backtrace printing for glibc.
#endif

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

void GCInternalExceptionCreate(int type, const char *message, uint64_t line, const char *file, const char *function)
{
    Exception *exception = malloc(sizeof(*exception));
    if(exception == NULL)
    {
        for(uint32_t x = 0; GCInternalFallbackExceptionInUse; x++)
        {
            Timespec sleep = SecsToTimespec(0.1f);
            nanosleep(&sleep, NULL);

            if(x > 50)
            {
                fprintf(stderr, "Fatal error: unable to allocate or acquire backup exception object, exiting\n");
                exit(-1);
            }
        }
    
        GCInternalFallbackExceptionInUse = 1;
        exception = &GCInternalFallbackException;
    }

    exception->Type = type;
    exception->Message = message;
    exception->Line = line;
    exception->File = file;
    exception->Function = function;
    exception->IsFallbackException = 0;

    GCInternalBacktrace(exception);
    GCInternalExceptionThreadData.Exception = exception;
}

void ExceptionPrint(Exception *exception)
{
    fprintf(stderr, "%s: %s at %s - %s() - Line %zu\n", strerror(exception->Type), exception->Message, exception->File, exception->Function, exception->Line);
    GCInternalPrintBacktrace(exception);
}

void ExceptionFree(Exception *exception)
{
    if(exception == NULL)
        return;

    if(exception->IsFallbackException)
    {
        GCInternalFallbackExceptionInUse = 0;
        return;
    }

    free(exception);
}