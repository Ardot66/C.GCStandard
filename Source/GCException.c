#include "GCException.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

thread_local struct GCInternalExceptionThreadData GCInternalExceptionThreadData = {0};

void GCInternalExceptionJump(GCInternalExitFunc nextExit)
{
    if(nextExit != NULL)
        nextExit();

    if(GCInternalExceptionThreadData.NextBufRef == NULL)
    {
        PrintException(GCInternalExceptionThreadData.Exception);
        printf("Fatal error: no try statement found to catch current exception, aborting\n");
        exit(GCInternalExceptionThreadData.Exception.Type);
    }
    
    longjmp(*GCInternalExceptionThreadData.NextBufRef, -1);
}

void PrintException(Exception exception)
{
    printf("%s: %s at %s - %s() - Line %zu\n", strerror(exception.Type), exception.Message, exception.File, exception.Function, exception.Line);
}