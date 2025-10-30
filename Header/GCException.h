#ifndef __GC_EXCEPTION__
#define __GC_EXCEPTION__

#include <setjmp.h>
#include <stdint.h>

enum GCInternalExceptionConstants
{
    GC_INTERNAL_BACKTRACE_FRAMES = 64
};

typedef struct Exception
{
    int Type;
    int BacktraceFrames;
    int IsFallbackException;
    uintptr_t Backtrace[GC_INTERNAL_BACKTRACE_FRAMES];
    const char *Message;

    // Line information is still kept as backtraces may fail.
    const char *File;
    const char *Function;
    uint64_t Line;
} Exception;

typedef void (* GCInternalExitFunc)(Exception *gcInternalException);

// Thread local data needs to be wrapped in a single struct, weird overlapping stuff was happening otherwise.
struct GCInternalExceptionThreadData
{
    jmp_buf *NextBufRef;
    GCInternalExitFunc NextExitFunc;
    Exception *Exception;
};

extern thread_local struct GCInternalExceptionThreadData GCInternalExceptionThreadData;

[[__noreturn__]]
void GCInternalExceptionJump(GCInternalExitFunc nextExit, Exception *exception);
Exception *GCInternalExceptionCreate(int type, const char *message, uint64_t line, const char *file, const char *function);

// Initializes the exit block, must be placed at the beginning of a scope, before any potential exceptions occur.
// If multiple exit blocks (and thus ExitInits) are placed within the same scope and intersect in any way,
// they must be nested as if the internal exit block was within its own function.
//
// The identifier parameter must only be set when multiple exit blocks exist within a single scope,
// in which case all but one exit block must have a unique identifier.
#define ExitInit(identifier) \
auto void GCInternalExit ## identifier(Exception *);\
GCInternalExitFunc GCInternalNextExit ## identifier = GCInternalExceptionThreadData.NextExitFunc; \
GCInternalExceptionThreadData.NextExitFunc = GCInternalExit ## identifier

// Marks the beginning of an exit block, which is guaranteed to run no matter what, and is ideal for cleanup code.
// Exit blocks should be placed at the end of a function, and require that ExitInit() has been called before it
// is possible that any exceptions have been thrown. It is recommended to initialize any variables that will
// be cleaned up before invoking ExitInit, so that they are always in a state where freeing them is safe.
//
// See ExitInit for the use case of the identifier parameter, which can usually be left empty.
#define ExitBegin(identifier)\
GCInternalExit ## identifier(NULL);\
void GCInternalExit ## identifier(Exception *gcInternalException) {\
    GCInternalExceptionThreadData.NextExitFunc = GCInternalNextExit ## identifier;\
do {} while (0)

// Used to check if the exit block has been triggered by an exception, which can be useful when freeing
// resources that would otherwise be returned.
#define IfExitException if (gcInternalException)

// Marks the end of an exit block.
//
// See ExitInit for the use case of the identifier parameter, which can usually be left empty.
#define ExitEnd(identifier) \
    IfExitException\
        GCInternalExceptionJump(GCInternalNextExit ## identifier, gcInternalException);\
} do {} while (0)

// Throws an exception, automatically exiting the current scope and jumping to the most recent exit block for cleanup or
// try block for handling.
#define Throw(error, message)\
do { \
    GCInternalExceptionJump(GCInternalExceptionThreadData.NextExitFunc, GCInternalExceptionCreate(error, message, __LINE__, __FILE__, __func__));\
} while (0)

// Throws an existing exception.
#define ThrowException(exception)\
do {\
    GCInternalExceptionThreadData.Exception = (exception);\
    GCInternalExceptionJump(GCInternalExceptionThreadData.NextExitFunc);\
}while (0)

// Confirms that a statement is true, throwing an exception otherwise.
#define ThrowMsgIf(statement, error, message)\
if(statement)\
    Throw(error, message)

// Confirms that a statement is true, throwing an exception otherwise.
// Always contains the error message "Assertion failed".
#define ThrowIf(statement, error) ThrowMsgIf(statement, error, "Error detected: (" #statement ")")

// Marks the beginning of a try block, in which any exceptions will be caught and placed in the passed exception variable
// for handling after the try block ends. Any non-null exception needs to be freed using ExceptionFree.
// If an exception occurs in a try block, code execution will immediately fast-forward to the end of the block.
#define TryBegin(exception)\
do {\
    exception = NULL;\
    jmp_buf GCInternalTryBuf, *GCInternalNextTryBuf = GCInternalExceptionThreadData.NextBufRef;\
    GCInternalExitFunc GCInternalTryNextExit = GCInternalExceptionThreadData.NextExitFunc;\
    GCInternalExceptionThreadData.NextExitFunc = NULL;\
    \
    if(setjmp(GCInternalTryBuf) == 0)\
        GCInternalExceptionThreadData.NextBufRef = &GCInternalTryBuf;\
    else { \
        exception = GCInternalExceptionThreadData.Exception; \
        GCInternalExceptionThreadData.Exception = NULL;\
        GCInternalExceptionThreadData.NextBufRef = GCInternalNextTryBuf; \
        GCInternalExceptionThreadData.NextExitFunc = GCInternalTryNextExit;\
        break; \
    } do{}while(0)

// Marks the end of a try block.
#define TryEnd \
    GCInternalExceptionThreadData.NextBufRef = GCInternalNextTryBuf;\
} while (0)

// Neatly prints an exception to the standard output.
void ExceptionPrint(Exception *exception);
void ExceptionFree(Exception *exception);

#endif