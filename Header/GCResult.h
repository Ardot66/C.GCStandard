#ifndef __GC_RESULT__
#define __GC_RESULT__

#include <stddef.h>
#include <stdint.h>

enum GCInternalErrorInfoConstants
{
    GC_INTERNAL_BACKTRACE_FRAMES = 16,
    GC_INTERNAL_MESSAGE_COUNT = 4
};

// Struct containing detailed information about errors.
typedef struct ErrorInfo
{
    uint64_t Code;
    int BacktraceFrames;
    uintptr_t Backtrace[GC_INTERNAL_BACKTRACE_FRAMES];
    size_t MessageCount;
    const char *Messages[GC_INTERNAL_MESSAGE_COUNT];

    // This information is still kept as backtraces may fail
    const char *File;
    const char *Function;
    uint64_t Line;
} ErrorInfo;

extern thread_local ErrorInfo GCInternalCurrentErrorInfo;

int GCInternalSetErrorInfo(const char *file, const char *function, const uint64_t line, const uint64_t code, const char **messages);

// Defines a label that will be jumped to if any errors occur throughout a function. Automatically provides
// the Error variable, which will be either GC_SUCCESS or GC_FAILURE depending on whether an error occurred.
// Also provides the IfError macro, which does as the name implies.
#define ErrorLabel \
GCError gcInternalRetval = GC_SUCCESS;\
if(0) GCInternalErrorLabel: gcInternalRetval = GC_ERROR;\
if(0) goto GCInternalErrorLabel // This avoids an annoying unused label warning

// Jumps to the error label, setting the Error variable to GC_ERROR in the process.
#define GotoError goto GCInternalErrorLabel

// Acts as an if statement that checks if Error != GC_SUCCESS.
#define IfError if(gcInternalRetval != GC_SUCCESS)

// See ErrorLabel.
#define Error gcInternalRetval

// Convenience macro for use in cases where a function's result is being passed through a switch statement.
// Automatically handles the GC_ERROR case like a Try statement would.
#define ErrorCase case GC_ERROR: goto GCInternalErrorLabel

// Convenience macro that jumps to the error label if the contained statement produces a non-zero value.
// Should only really be used to wrap conditionals like myVar == NULL or functions that return GCError.
#define Try(statement) if(statement) goto GCInternalErrorLabel

// Throws an error, jumping to the error label in the process. Should only be used for more serious errors,
// not in cases where a function is returning an expected value.
#define Throw(code, ...) do { GCInternalSetErrorInfo(__FILE__, __func__, __LINE__, code, (const char *[]){__VA_ARGS__ "", NULL}); goto GCInternalErrorLabel; } while (0)

// This type should be used if a function can only succeed or cause a serious error.
typedef enum GCError
{
    GC_ERROR = -1,
    GC_SUCCESS = 0
} GCError;

// This type should be used for functions that must return some indication of status along with the potential
// for causing a serious error.
typedef enum GCResult
{
    // Indicates that a function succeeded
    GC_RESULT_SUCCESS = GC_SUCCESS,
    // Indicates that a function failed, but differs from GC_RESULT_ERROR in that failure should still result in
    // a valid state.
    GC_RESULT_FAILURE,
    GC_RESULT_ERROR = GC_ERROR,
} GCResult;

void SetErrorCallback(void (*callback)(ErrorInfo *errorInfo, void *data), void *data);
void ErrorInfoPrint(ErrorInfo *errorInfo);

static inline ErrorInfo *ErrorInfoGetCurrent() {return &GCInternalCurrentErrorInfo;}
static inline void ErrorInfoPrintCurrent() {ErrorInfoPrint(&GCInternalCurrentErrorInfo);}

#endif