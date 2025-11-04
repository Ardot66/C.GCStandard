#include "GCInternalGlobals.h"
#include "GCMemory.h"
#include "GCException.h"
#include "GCCollections.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <backtrace.h>
#include <pthread.h>

// Custom functions for overriding the default malloc and free functions.
static void *(*CustomAllocator)(const size_t size) = NULL;
static void *(*CustomReallocator)(void *oldPtr, const size_t size) = NULL;
static void (*CustomDeallocator)(void *ptr) = NULL;

// Custom functions for adding additional behaviours to memory allocation.
static void (*MallocCallback)(void *ptr, const size_t size) = NULL;
static void (*ReallocCallback)(void *oldPtr, void *ptr, const size_t size) = NULL;
static void (*FreeCallback)(void *ptr) = NULL;

static const char *CallbackErrorMessage = "Exception occurred within a GCMemory callback, printing immediately:\n";

void GCSetCustomAllocator(void *(* allocator)(const size_t size))
{
    CustomAllocator = allocator;
}

void GCSetCustomReallocator(void *(*customReallocator)(void *oldPtr, const size_t size))
{
    CustomReallocator = customReallocator;
}

void GCSetCustomDeallocator(void (* deallocator)(void *ptr))
{
    CustomDeallocator = deallocator;
}

void GCSetMallocCallback(void (*callback)(void *ptr, const size_t size))
{
    MallocCallback = callback;
}

void GCSetReallocCallback(void (*reallocCallback)(void *oldPtr, void *ptr, const size_t size))
{
    ReallocCallback = reallocCallback;
}

void GCSetFreeCallback(void (*callback)(void *ptr))
{
    FreeCallback = callback;
}

typedef void (*Func)();
DictDefine(Func, GCAllocationData, DictFunctionAllocationData);
static DictFunctionAllocationData HeapDict = DictDefault;
static size_t BacktraceCount = 0;
static const DictFunctions HeapDictFunctions = DictDefaultFunctions;
static pthread_mutex_t HeapDictMutex = PTHREAD_MUTEX_INITIALIZER;

static int GCWatchHeapBacktraceCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)lineno;
    if(filename == NULL && function == NULL && lineno == 0)
        return 1;

    if(strcmp(__FILE__, filename) == 0)
        return 0;

    GCAllocationData *allocData = data;
    if(allocData->PC == (uintptr_t)NULL)
    {
        allocData->PC = pc;
        return 0;
    }

    if(allocData->ExtraPCs == NULL || allocData->ExtraPCs->Count >= BacktraceCount)
        return 1;

    allocData->ExtraPCs->PCs[allocData->ExtraPCs->Count++] = pc;
    return strcmp("main", function) == 0;
}

static void GCWatchHeapMallocCallback(void *ptr, const size_t size)
{
    GCAllocationData data = 
    {
        .Size = size, 
        .PC = (uintptr_t)NULL, 
        .ExtraPCs = BacktraceCount == 0 ? NULL : GCCalloc(sizeof(GCAllocationExtraPCData) + sizeof(uintptr_t) * BacktraceCount)
    };
    backtrace_full(GCInternalGetBacktraceState(), 2, GCWatchHeapBacktraceCallback, NULL, &data);

    pthread_mutex_lock(&HeapDictMutex);
    ExitInit();
    DictAdd(&HeapDict, ptr, data, HeapDictFunctions);
    ExitBegin();
        IfExitException
            GCFree(data.ExtraPCs);
        pthread_mutex_unlock(&HeapDictMutex);
    ExitEnd();
}

static void GCWatchHeapReallocCallback(void *oldPtr, void *ptr, const size_t size)
{
    pthread_mutex_lock(&HeapDictMutex);
    ExitInit();
    ssize_t oldIndex = DictIndexOf(&HeapDict, oldPtr, HeapDictFunctions);
    ThrowIf(oldIndex == -1); // This should never happen, but it's best not to fail silently if it does.

    GCAllocationData data = *DictGetValue(&HeapDict, oldIndex);
    data.Size = size;
    DictRemove(&HeapDict, oldIndex, HeapDictFunctions);
    DictAdd(&HeapDict, ptr, data, HeapDictFunctions);

    ExitBegin();
        pthread_mutex_unlock(&HeapDictMutex);
    ExitEnd();
}

static void GCWatchHeapFreeCallback(void *ptr)
{
    pthread_mutex_lock(&HeapDictMutex);
    ssize_t index = DictIndexOf(&HeapDict, ptr, HeapDictFunctions);
    if(index == -1)
    {
        pthread_mutex_unlock(&HeapDictMutex);
        printf("Warning: index == -1 in GCWatchHeapFreeCallback\n");
        return;
    }
    GCFree((DictGetValue(&HeapDict, index))->ExtraPCs);
    DictRemove(&HeapDict, index, HeapDictFunctions);
    pthread_mutex_unlock(&HeapDictMutex);
}

void GCWatchHeap(size_t backtraceCount)
{
    // Initializing backtrace state here to avoid a potential race condition in multithreaded programs.
    GCInternalGetBacktraceState();

    GCSetMallocCallback(GCWatchHeapMallocCallback);
    GCSetReallocCallback(GCWatchHeapReallocCallback);
    GCSetFreeCallback(GCWatchHeapFreeCallback);
    BacktraceCount = backtraceCount;
}

void GCStopWatchingHeap()
{
    GCSetMallocCallback(NULL);
    GCSetFreeCallback(NULL);

    void *ptr;
    GCAllocationData data;
    for(size_t x = 0; (ptr = GCIterateHeap(&x, &data)) != NULL; x++)
        GCFree(data.ExtraPCs);

    // Not locking the dictionary here because there really should not be stuff going on
    // in other threads when this function is called anyways.
    DictFree(&HeapDict); 
    pthread_mutex_destroy(&HeapDictMutex);
    HeapDict = (DictFunctionAllocationData)DictDefault;
    HeapDictMutex = PTHREAD_MUTEX_INITIALIZER;
}

void *GCIterateHeap(size_t *index, GCAllocationData *data)
{
    pthread_mutex_lock(&HeapDictMutex);
    while(DictIterate(&HeapDict, index))
    {
        if(data != NULL)
            *data = *DictGetValue(&HeapDict, *index);

        pthread_mutex_unlock(&HeapDictMutex);
        return *DictGetKey(&HeapDict, *index);
    }
    pthread_mutex_unlock(&HeapDictMutex);

    return NULL;
}

static int GCPrintHeapPCInfoCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)data;
    (void)pc;
    if(filename == NULL)
        filename = "file not found";
    if(function == NULL)
        function = "function not found";

    printf("      %s - %s() - line %d\n", filename, function, lineno);
    return 0;
}

void GCPrintHeap()
{
    printf("Printing heap contents:\n");

    void *ptr;
    GCAllocationData data;
    for(size_t index = 0; (ptr = GCIterateHeap(&index, &data)) != NULL; index++)
    {
        printf("   %p (%zu bytes) allocated at:\n", ptr, data.Size);
        backtrace_pcinfo(GCInternalGetBacktraceState(), data.PC, GCPrintHeapPCInfoCallback, NULL, NULL);
        if(data.ExtraPCs != NULL)
            for(size_t x = 0; x < data.ExtraPCs->Count; x++)
                backtrace_pcinfo(GCInternalGetBacktraceState(), data.ExtraPCs->PCs[x], GCPrintHeapPCInfoCallback, NULL, NULL);
    }
}

static inline void CallbackBegin()
{
    GCInternalThreadData.HeapCallbackActive = true;
}

static inline void CallbackEnd(Exception *exception)
{
    GCInternalThreadData.HeapCallbackActive = false;

    if(exception)
    {
        fprintf(stderr, CallbackErrorMessage);
        ExceptionPrint(exception);
    }
    ExceptionFree(exception);
}


static inline void *GCMallocInternal(const size_t size)
{
    void *ptr;
    if(CustomAllocator != NULL)
        ptr = CustomAllocator(size);
    else
        ptr = malloc(size);

    if(MallocCallback != NULL && ptr != NULL && !GCInternalThreadData.HeapCallbackActive)
    {
        // NOTE, the use of try blocks here is quite expensive, and a better solution may be possible.
        CallbackBegin();
        Exception *exception;
        TryBegin(exception);
            MallocCallback(ptr, size);
        TryEnd;
        CallbackEnd(exception);
    }
        
    return ptr;
}

void *GCMalloc(const size_t size)
{
    void *ptr = GCMallocInternal(size);
    ThrowIf(ptr == NULL, strerror(errno));
    return ptr;
}

void *GCMallocNoExcept(const size_t size)
{
    return GCMallocInternal(size);
}

void *GCCalloc(const size_t size)
{
    void *ptr = GCMallocInternal(size);
    ThrowIf(ptr == NULL, strerror(errno));
    memset(ptr, 0, size);
    return ptr;
}

void *GCCallocNoExcept(const size_t size)
{
    void *ptr = GCMallocInternal(size);
    memset(ptr, 0, size);
    return ptr;
}

static inline void *GCReallocInternal(void *oldPtr, const size_t size)
{
    void *ptr;
    // The malloc callback should handle this case
    if(oldPtr == NULL)
        return GCMallocInternal(size);
    else if(CustomReallocator != NULL)
        ptr = CustomReallocator(oldPtr, size);
    else
        ptr = realloc(oldPtr, size);

    if(ReallocCallback != NULL && ptr != NULL && !GCInternalThreadData.HeapCallbackActive)
    {
        CallbackBegin();
        Exception *exception;
        TryBegin(exception);
            ReallocCallback(oldPtr, ptr, size);
        TryEnd;
        CallbackEnd(exception);
    }
    
    return ptr;
}

void *GCRealloc(void *oldPtr, const size_t size)
{
    void *ptr = GCReallocInternal(oldPtr, size);
    ThrowIf(ptr == NULL, strerror(errno));
    return ptr;
}

void *GCReallocNoExcept(void *oldPtr, const size_t size)
{
    return GCReallocInternal(oldPtr, size);
}

void GCFree(void *ptr)
{
    if(FreeCallback != NULL && ptr != NULL && !GCInternalThreadData.HeapCallbackActive)
    {
        CallbackBegin();
        Exception *exception;
        TryBegin(exception);
            FreeCallback(ptr);
        TryEnd;
        CallbackEnd(exception);
    }

    if(CustomDeallocator != NULL)
        CustomDeallocator(ptr);
    else
        free(ptr);
}

