#include "Exception.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

__thread jmp_buf *_NextBufRef = NULL;
__thread Exception _Exception = {};

void _ExceptionJump(jmp_buf *jumpBuffer)
{
    if(jumpBuffer == NULL)
    {
        PrintException(_Exception);
        printf("Fatal error: no try statement found to catch current exception, aborting\n");
        exit(_Exception.Type);
    }
    
    longjmp(*jumpBuffer, -1);
}

void PrintException(Exception exception)
{
    printf("%s: %s at %s - %s() - Line %zu\n", strerror(exception.Type), exception.Message, exception.File, exception.Function, exception.Line);
}