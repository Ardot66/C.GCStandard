#include "GCException.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

__thread struct _ExceptionThreadData _ExceptionThreadData = {0};

void _ExceptionJump(jmp_buf *jumpBuffer)
{
    if(jumpBuffer == NULL)
    {
        PrintException(_ExceptionThreadData.Exception);
        printf("Fatal error: no try statement found to catch current exception, aborting\n");
        exit(_ExceptionThreadData.Exception.Type);
    }
    
    longjmp(*jumpBuffer, -1);
}

void PrintException(Exception exception)
{
    printf("%s: %s at %s - %s() - Line %zu\n", strerror(exception.Type), exception.Message, exception.File, exception.Function, exception.Line);
}