#include "GCResult.h"
#include "GCErrorInfoCache.h"
#include "GCList.h"
#include "GCMemory.h"
#include "GCInternal.h"

#include <stddef.h>
#include <backtrace.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

thread_local ErrorInfo GCInternalCurrentErrorInfo;

static void *ErrorInfoCallbackData;
static void (*ErrorInfoCallback)(ErrorInfo *errorInfo, void *data) = NULL;

void SetErrorCallback(void (*callback)(ErrorInfo *errorInfo, void *data), void *data)
{
    ErrorInfoCallback = callback;
    ErrorInfoCallbackData = data;
}

static int BacktraceCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)filename; (void)lineno;

    ErrorInfo *errorInfo = data;
    if(pc == UINTPTR_MAX || errorInfo->BacktraceFrames >= GC_INTERNAL_BACKTRACE_FRAMES)
        return 1;

    errorInfo->Backtrace[errorInfo->BacktraceFrames] = pc;
    errorInfo->BacktraceFrames++;

    // Stop the backtrace at main.
    return strcmp(function, "main") == 0;
}

int GCInternalSetErrorInfo(const char *file, const char *function, const uint64_t line, const uint64_t code, const char **messages)
{
    ErrorInfo *errorInfo = &GCInternalCurrentErrorInfo;

    errorInfo->MessageCount = 0;
    while(messages[errorInfo->MessageCount] != NULL)
        errorInfo->MessageCount++;

    errorInfo->Code = code;
    errorInfo->Line = line;
    errorInfo->File = file;
    errorInfo->Function = function;
    errorInfo->BacktraceFrames = 0;

    memcpy(errorInfo->Messages, messages, errorInfo->MessageCount * sizeof(*messages));
    backtrace_full(GCInternalGetBacktraceState(), 1, BacktraceCallback, NULL, errorInfo);

    if(ErrorInfoCallback != NULL)
        ErrorInfoCallback(errorInfo, ErrorInfoCallbackData);

    return 1;
}

static int BacktracePrintCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)pc;
    int *depth = data;
    fprintf(stderr, "%d - %s - %s() - Line %d\n", *depth, filename , function, lineno);
    return 0;
}

void ErrorInfoPrint(ErrorInfo *errorInfo)
{
    if(errorInfo == NULL)
    {
        fprintf(stderr, "Cannot print null errorInfo\n");
        return;
    }

    for(size_t x = 0; errorInfo->Messages[x] != NULL; x++)
        fprintf(stderr, "%s, ", errorInfo->Messages[x]);
    fprintf(stderr, "at %s() in %s line %zu\n", errorInfo->Function, errorInfo->File, errorInfo->Line);

    for(int x = 0; x < errorInfo->BacktraceFrames; x++)
    {
        int depth = errorInfo->BacktraceFrames - x - 1;
        backtrace_pcinfo(GCInternalGetBacktraceState(), errorInfo->Backtrace[x], BacktracePrintCallback, NULL, &depth);
    }
}

static void ErrorInfoCacheCallback(ErrorInfo *errorInfo, void *data)
{
    ErrorInfoCache *cache = data;
    pthread_mutex_lock(&cache->Mutex);
    // Don't care if it fails, there's not really anything that can be done.
    ListAdd(&cache->ErrorInfos, *errorInfo);
    pthread_mutex_unlock(&cache->Mutex);
}

ErrorInfoCache *ErrorInfoCacheCreate()
{
    ErrorInfoCache *cache = GCMalloc(sizeof(ErrorInfoCache));
    if(cache == NULL)
        GotoError;

    cache->ErrorInfos = (GCInternalListErrorInfo)ListDefault;
    cache->Mutex = PTHREAD_MUTEX_INITIALIZER;

    // This way at least a few errorInfos are guaranteed to be captured regardless of memory issues.
    Try(ListResize(&cache->ErrorInfos, sizeof(ErrorInfoCache) * 8));
    SetErrorCallback(ErrorInfoCacheCallback, cache);

    ErrorLabel;
    IfError
        return NULL;
    return cache;
}

void ErrorInfoCacheFree(ErrorInfoCache *cache)
{
    if(cache == NULL)
        return;

    ListFree(&cache->ErrorInfos);
    pthread_mutex_destroy(&cache->Mutex);
    GCFree(cache);
}

void ErrorInfoCachePrint(ErrorInfoCache *cache)
{
    for(size_t x = 0; x < cache->ErrorInfos.Count; x++)
        ErrorInfoPrint(&cache->ErrorInfos.V[x]);
}

