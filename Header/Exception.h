#ifndef __EXCEPTION__
#define __EXCEPTION__

#include <setjmp.h>
#include <stdint.h>

typedef struct Exception
{
    int Type;
    char *Message;
    uint64_t Line;
    const char *File;
    const char *Function;
} Exception;

// Thread local data needs to be wrapped in a single struct, weird overlapping stuff was happening otherwise.
struct _ExceptionThreadData
{
    jmp_buf *NextBufRef;
    Exception Exception;
};

extern __thread struct _ExceptionThreadData _ExceptionThreadData;

void _ExceptionJump(jmp_buf *jumpBuffer);

// Initializes the exit block, must be placed at the beginning of a scope, before any potential exceptions occur.
#define ExitInit() \
jmp_buf _buf, *_nextBuf = _ExceptionThreadData.NextBufRef;\
\
if(setjmp(_buf) == 0) \
    _ExceptionThreadData.NextBufRef = &_buf; \
else \
    goto _ExitBegin

// Marks the beginning of an exit block, which is guaranteed to run no matter what, and is ideal for cleanup code.
// Exit blocks should be placed at the end of a function, and require that ExitInit() has been called at the 
// beginning of the function.
#define ExitBegin \
_ExitBegin: \
do { do{}while(0)

// Used to check if the exit block has been triggered by an exception, which can be useful when freeing
// resources that would otherwise be passed to a calling function.
#define IfExitException if (_ExceptionThreadData.Exception.Type)

// Marks the end of an exit block.
#define ExitEnd \
    if(_ExceptionThreadData.Exception.Type) \
        _ExceptionJump(_nextBuf);\
} while (0)

// Throws an exception, automatically exiting the current scope and jumping to the most recent exit block for cleanup or
// try block for handling.
#define Throw(error, message)\
do { \
    _ExceptionThreadData.Exception = (Exception){.Type = error, .Message = message, .Line = __LINE__, .File = __FILE__, .Function = __func__};\
    _ExceptionJump(_ExceptionThreadData.NextBufRef);\
} while (0)

// Throws an existing exception.
#define ThrowException(exception) Throw((exception).Type, (exception).Message)

// Confirms that a statement is true, throwing an exception otherwise.
#define AssertMsg(statement, error, message)\
if(!(statement))\
    Throw(error, message)

// Confirms that a statement is true, throwing an exception otherwise.
// Always contains the error message "Assertion failed".
#define Assert(statement, error) AssertMsg(statement, error, "Assertion failed (" #statement ")")

// Marks the beginning of a try block, in which any exceptions will be caught and placed in the passed exception variable
// for handling after the try block ends.
// If an exception occurs in a try block, code execution will immediately fast-forward to the end of the block.
#define TryBegin(exception)\
do {\
    exception = (Exception){};\
    jmp_buf _tryBuf, *_tryNextBuf = _ExceptionThreadData.NextBufRef;\
    \
    if(setjmp(_tryBuf) == 0)\
        _ExceptionThreadData.NextBufRef = &_tryBuf;\
    else { \
        exception = _ExceptionThreadData.Exception; \
        _ExceptionThreadData.Exception = (Exception){};\
        _ExceptionThreadData.NextBufRef = _tryNextBuf; \
        break; \
    } do{}while(0)

// Marks the end of a try block.
#define TryEnd \
    _ExceptionThreadData.NextBufRef = _tryNextBuf;\
} while (0)

// Neatly prints an exception to the standard output.
void PrintException(Exception exception);

#endif