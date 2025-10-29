#include "GCException.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

thread_local struct GCInternalExceptionThreadData GCInternalExceptionThreadData = {0};

enum Constants
{
    BACKTRACE_FRAMES = 64
};

static int InitBacktrace()
{
    void ***backtraceDest = &GCInternalExceptionThreadData.Exception.Backtrace;
    if(*backtraceDest == NULL)
    {
        *backtraceDest = malloc(sizeof(void *) * BACKTRACE_FRAMES);
        if(*backtraceDest == NULL)
            return -1;
    }

    return 0;
}

#ifdef __WIN32__
#include <windows.h>
#include <dbghelp.h>
void GCInternalBacktrace()
{
    if(InitBacktrace()) 
        return;
    
    HANDLE process = GetCurrentProcess();
    SymInitialize(process, NULL, TRUE);
    GCInternalExceptionThreadData.Exception.BacktraceFrames = CaptureStackBackTrace(1, BACKTRACE_FRAMES, GCInternalExceptionThreadData.Exception.Backtrace, NULL);
}

void GCInternalPrintBacktrace(Exception *exception)
{
    if(exception->Backtrace == NULL)
    {
        fprintf(stderr, "Unable to print backtrace");
        return;
    }

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
        PrintException(GCInternalExceptionThreadData.Exception);
        fprintf(stderr, "Fatal error: no try statement found to catch current exception, aborting\n");
        exit(GCInternalExceptionThreadData.Exception.Type);
    }
    
    longjmp(*GCInternalExceptionThreadData.NextBufRef, -1);
}

void GCInternalSetException(int type, const char *message, uint64_t line, const char *file, const char *function)
{
    Exception *exception = &GCInternalExceptionThreadData.Exception;
    exception->Type = type;
    exception->Message = message;
    exception->Line = line;
    exception->File = file;
    exception->Function = function;

    GCInternalBacktrace();
}

void PrintException(Exception exception)
{
    fprintf(stderr, "%s: %s at %s - %s() - Line %zu\n", strerror(exception.Type), exception.Message, exception.File, exception.Function, exception.Line);
    GCInternalPrintBacktrace(&exception);
}