#ifndef __GC_EXCEPTION__
#define __GC_EXCEPTION__

#include <setjmp.h>
#include <stdint.h>

typedef struct Exception
{
    int Type;
    const char *Message;
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

[[__noreturn__]]
void _ExceptionJump(jmp_buf *jumpBuffer);

// Initializes the exit block, must be placed at the beginning of a scope, before any potential exceptions occur.
#define ExitInit() \
jmp_buf _buf, *_nextBuf = _ExceptionThreadData.NextBufRef;\
\
if(setjmp(_buf) == 0) \
    _ExceptionThreadData.NextBufRef = &_buf; \
else \
    goto _ExitBegin

// Used to check if the exit block has been triggered by an exception, which can be useful when freeing
// resources that would otherwise be passed to a calling function.
#define IfExitException if (_ExceptionThreadData.Exception.Type)

// Can be used to split up exit blocks into sections to allow memory to be freed throughout a function
// rather than just at the end. Must be placed at the end of a exit block section, and must be paired
// with a respective ExitJumpEnd with the same identifier.
//
// As with a standard exit block, a split exit block is guaranteed to execute regardless of whether
// or not an exception is thrown.
//
// Note: Exit blocks sections can execute multiple times in cases where a section has executed normally,
// but an exception occurs before the final section has been reached. For such sections, ensure that
// double frees are avoided by setting freed pointers to null.
#define ExitJumpBegin(index) \
IfExitException\
    goto _ExitBegin ## index;\
} while (0)

// Used with ExitJumpBegin to split exit blocks into sections. Must be placed at the start of a exit
// block section. See ExitJumpBegin for more info.
#define ExitJumpEnd(index)\
_ExitBegin ## index: \
do { do{}while(0)

// Marks the beginning of an exit block, which is guaranteed to run no matter what, and is ideal for cleanup code.
// Exit blocks should be placed at the end of a function, and require that ExitInit() has been called at the 
// beginning of the function.
#define ExitBegin ExitJumpEnd()

// Marks the end of an exit block.
#define ExitEnd \
IfExitException\
    _ExceptionJump(_nextBuf);\
else\
    _ExceptionThreadData.NextBufRef = _nextBuf;\
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
#define ThrowMsgIf(statement, error, message)\
if(statement)\
    Throw(error, message)

// Confirms that a statement is true, throwing an exception otherwise.
// Always contains the error message "Assertion failed".
#define ThrowIf(statement, error) ThrowMsgIf(statement, error, "Error detected: (" #statement ")")

// Marks the beginning of a try block, in which any exceptions will be caught and placed in the passed exception variable
// for handling after the try block ends.
// If an exception occurs in a try block, code execution will immediately fast-forward to the end of the block.
#define TryBegin(exception)\
do {\
    exception = (Exception){0};\
    jmp_buf _tryBuf, *_tryNextBuf = _ExceptionThreadData.NextBufRef;\
    \
    if(setjmp(_tryBuf) == 0)\
        _ExceptionThreadData.NextBufRef = &_tryBuf;\
    else { \
        exception = _ExceptionThreadData.Exception; \
        _ExceptionThreadData.Exception = (Exception){0};\
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