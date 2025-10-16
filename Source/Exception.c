#include "Exception.h"

__thread jmp_buf *_NextBufRef = NULL;
__thread Exception _Exception = {};

void PrintException(Exception exception)
{
    printf("%s: %s at %s - %s() - Line %zu\n", strerror(exception.Type), exception.Message, exception.File, exception.Function, exception.Line);
}