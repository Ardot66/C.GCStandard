#ifndef __GC_EXCEPTION__
#define __GC_EXCEPTION__

#include "GCInternalGlobals.h"

enum GCInternalExceptionConstants
{
    GC_INTERNAL_BACKTRACE_FRAMES = 64,
    GC_INTERNAL_MESSAGE_COUNT = 4
};

typedef struct Exception
{
    int BacktraceFrames;
    int IsFallbackException;
    uintptr_t Backtrace[GC_INTERNAL_BACKTRACE_FRAMES];
    size_t MessageCount;
    const char *Messages[GC_INTERNAL_MESSAGE_COUNT];

    // Line information is still kept as backtraces may fail.
    const char *File;
    const char *Function;
    uint64_t Line;
} Exception;

[[__noreturn__]]
void GCInternalExceptionJump(GCInternalExitFunc nextExit, Exception *exception);
Exception *GCInternalExceptionCreate(uint64_t line, const char *file, const char *function, const size_t messageCount, const char **messages);

// Initializes the exit block, must be placed at the beginning of a scope, before any potential exceptions occur.
// If multiple exit blocks (and thus ExitInits) are placed within the same scope and intersect in any way,
// they must be nested as if the internal exit block was within its own function.
//
// The identifier parameter must only be set when multiple exit blocks exist within a single scope,
// in which case all but one exit block must have a unique identifier.
#define ExitInit(identifier) \
auto GCInternalExitFuncBox GCInternalExit ## identifier(Exception *);\
GCInternalExitFunc GCInternalNextExit ## identifier = GCInternalThreadData.NextExitFunc; \
GCInternalThreadData.NextExitFunc = GCInternalExit ## identifier

// Marks the beginning of an exit block, which is guaranteed to run no matter what, and is ideal for cleanup code.
// Exit blocks should be placed at the end of a function, and require that ExitInit() has been called before it
// is possible that any exceptions have been thrown. It is recommended to initialize any variables that will
// be cleaned up before invoking ExitInit, so that they are always in a state where freeing them is safe.
//
// See ExitInit for the use case of the identifier parameter, which can usually be left empty.
#define ExitBegin(identifier)\
GCInternalExit ## identifier(NULL);\
GCInternalExitFuncBox GCInternalExit ## identifier(Exception *gcInternalException) {\
    (void)gcInternalException;\
    GCInternalThreadData.NextExitFunc = GCInternalNextExit ## identifier;\
do {} while (0)

// Used to check if the exit block has been triggered by an exception, which can be useful when freeing
// resources that would otherwise be returned.
#define IfExitException if (gcInternalException)

// Marks the end of an exit block.
//
// See ExitInit for the use case of the identifier parameter, which can usually be left empty.
#define ExitEnd(identifier) \
    return (GCInternalExitFuncBox)GCInternalNextExit ## identifier;\
} do {} while (0)

// Throws an exception, automatically exiting the current scope and jumping to the most recent exit block for cleanup or
// try block for handling.
#define Throw(message, ...)\
do { \
    const char *messages[] = {message, ## __VA_ARGS__};\
    GCInternalExceptionJump(GCInternalThreadData.NextExitFunc, GCInternalExceptionCreate(__LINE__, __FILE__, __func__, sizeof(messages) / sizeof(*messages), messages));\
} while (0)

// Throws an existing exception.
#define ThrowException(exception)\
do {\
    GCInternalThreadData.Exception = (exception);\
    GCInternalExceptionJump(GCInternalThreadData.NextExitFunc, GCInternalThreadData.Exception);\
}while (0)

// Throws an exception if the statement evaluates to be true.
#define ThrowIf(statement, message, ...)\
if(statement)\
    Throw(message, ## __VA_ARGS__)

// Marks the beginning of a try block, in which any exceptions will be caught and placed in the passed exception variable
// for handling after the try block ends. Any non-null exception needs to be freed using ExceptionFree.
// If an exception occurs in a try block, code execution will immediately fast-forward to the end of the block.
#define TryBegin(exception)\
do {\
    exception = NULL;\
    jmp_buf GCInternalTryBuf, *GCInternalNextTryBuf = GCInternalThreadData.NextBufRef;\
    GCInternalExitFunc GCInternalTryNextExit = GCInternalThreadData.NextExitFunc;\
    GCInternalThreadData.NextExitFunc = NULL;\
    \
    if(setjmp(GCInternalTryBuf) == 0)\
        GCInternalThreadData.NextBufRef = &GCInternalTryBuf;\
    else { \
        exception = GCInternalThreadData.Exception; \
        GCInternalThreadData.Exception = NULL;\
        GCInternalThreadData.NextBufRef = GCInternalNextTryBuf; \
        GCInternalThreadData.NextExitFunc = GCInternalTryNextExit;\
        break; \
    } do{}while(0)

// Marks the end of a try block.
#define TryEnd \
    GCInternalThreadData.NextBufRef = GCInternalNextTryBuf;\
} while (0)

// Neatly prints an exception to the standard output.
void ExceptionPrint(Exception *exception);
void ExceptionFree(Exception *exception);

#endif