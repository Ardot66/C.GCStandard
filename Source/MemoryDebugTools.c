#include "GCInternalGlobals.h"

#include "GCMemoryDebugTools.h"
#include "GCMemory.h"
#include "GCException.h"
#include "GCDictionary.h"
#include "GCAssert.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <backtrace.h>
#include <pthread.h>

typedef void (*Func)();
DictDefine(Func, GCAllocationData, DictFunctionAllocationData);

static DictFunctionAllocationData HeapDict = DictDefault;
static size_t BacktraceCount = 0, AllocationPadding = 0;
static pthread_mutex_t HeapDictMutex = PTHREAD_RECURSIVE_MUTEX_INITIALIZER;

static inline char PaddingHash(const void *ptr, const size_t index) 
{
    // This is just random gobbledigook, not scientific at all lol.
    return ((((uintptr_t)ptr * 57) >> 3) + (index << 3) * 13103) * 13;
}

static inline void FillPadding(void *ptr, size_t size)
{
    for(size_t x = 0; x < AllocationPadding * 2 + size; x++)
    {
        if(x == AllocationPadding)
            x += size;
        ((char *)ptr)[x] = PaddingHash(ptr, x);
    }
}

static int PrintAllocationPCInfoCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
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

static void PrintAllocation(const void *ptr, GCAllocationData data)
{
    printf("   %p (%zu bytes) allocated at:\n", ptr, data.Size);
    backtrace_pcinfo(GCInternalGetBacktraceState(), data.PC, PrintAllocationPCInfoCallback, NULL, NULL);
    if(data.ExtraPCs != NULL)
        for(size_t x = 0; x < data.ExtraPCs->Count; x++)
            backtrace_pcinfo(GCInternalGetBacktraceState(), data.ExtraPCs->PCs[x], PrintAllocationPCInfoCallback, NULL, NULL);
}

static inline void CheckPaddingContaminatedPrintBlockInfo(size_t size)
{
    printf("0: Beginning of main memory block\n");
    printf("%zd: End of main memory block\n", (ssize_t)size - 1);
}

static inline void CheckPaddingContaminated(const void *ptr)
{
    pthread_mutex_lock(&HeapDictMutex);
    ssize_t index = DictIndexOf(&HeapDict, ptr, DictDefaultFunctions);
    Assert(index != -1);
    size_t size = DictGetValue(&HeapDict, index).Size;
    void *unpaddedPtr = (char *)ptr - AllocationPadding;
    bool contaminated = 0;
    for(size_t x = 0; x < AllocationPadding * 2 + size; x++)
    {
        if(x == AllocationPadding)
        {
            x += size;
            if(contaminated)
                CheckPaddingContaminatedPrintBlockInfo(size);
        }

        char hash = PaddingHash(unpaddedPtr, x);
        char padding = ((char *)unpaddedPtr)[x];
        if(padding != hash)
        {
            if(!contaminated)
            {
                printf(
                    "Warning: out of bounds memory access detected, printing padding:\n"
                    "[Byte relative to allocation]: [Should be] - [Was set to]\n"
                );
                if(x >= AllocationPadding)
                    CheckPaddingContaminatedPrintBlockInfo(size);
            }
            
            printf("%zd: %02X - %02X\n", (ssize_t)x - AllocationPadding, (unsigned char)hash, (unsigned char)padding);
            contaminated = true;
        }
    }

    if(contaminated)
        PrintAllocation(ptr, DictGetValue(&HeapDict, index));
    pthread_mutex_unlock(&HeapDictMutex);
}

static int WatchHeapBacktraceCallback(void *data, uintptr_t pc, const char *filename, int lineno, const char *function)
{
    (void)lineno;
    if(pc == UINTPTR_MAX)
        return 1;

    if(filename != NULL && strcmp("Memory.c", filename) == 0)
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
    return function == NULL ? 0 : strcmp("main", function) == 0;
}

static void WatchHeapMallocCallback(void *ptr, const size_t size)
{
    GCAllocationData data = 
    {
        .Size = size, 
        .PC = (uintptr_t)NULL, 
        .ExtraPCs = BacktraceCount == 0 ? NULL : GCCalloc(sizeof(GCAllocationExtraPCData) + sizeof(uintptr_t) * BacktraceCount)
    };
    backtrace_full(GCInternalGetBacktraceState(), 2, WatchHeapBacktraceCallback, NULL, &data);

    pthread_mutex_lock(&HeapDictMutex);
    ExitInit();
    DictAdd(&HeapDict, ptr, data, DictDefaultFunctions);
    ExitBegin();
        IfExitException
            GCFree(data.ExtraPCs);
        pthread_mutex_unlock(&HeapDictMutex);
    ExitEnd();
}

static void WatchHeapReallocCallback(void *oldPtr, void *ptr, const size_t size)
{
    pthread_mutex_lock(&HeapDictMutex);
    ExitInit();
    ssize_t oldIndex = DictIndexOf(&HeapDict, oldPtr, DictDefaultFunctions);
    Assert(oldIndex != -1);

    GCAllocationData data = DictGetValue(&HeapDict, oldIndex);
    data.Size = size;
    DictRemove(&HeapDict, oldIndex, DictDefaultFunctions);
    DictAdd(&HeapDict, ptr, data, DictDefaultFunctions);

    ExitBegin();
        pthread_mutex_unlock(&HeapDictMutex);
    ExitEnd();
}

static void WatchHeapFreeCallback(void *ptr)
{
    pthread_mutex_lock(&HeapDictMutex);
    ssize_t index = DictIndexOf(&HeapDict, ptr, DictDefaultFunctions);
    Assert(index != -1);
    GCFree((DictGetValue(&HeapDict, index)).ExtraPCs);
    DictRemove(&HeapDict, index, DictDefaultFunctions);
    pthread_mutex_unlock(&HeapDictMutex);
}

static void *WatchHeapCustomAllocator(const size_t size)
{
    if(GCInternalThreadData.HeapCallbackActive)
        return malloc(size);

    void *ptr = malloc(size + AllocationPadding * 2);
    if(ptr == NULL)
        return NULL;

    FillPadding(ptr, size);
    return (char *)ptr + AllocationPadding;
}

static void *WatchHeapCustomReallocator(void *oldPtr, const size_t size)
{
    if(!GCInternalThreadData.HeapCallbackActive)
        CheckPaddingContaminated((char *)oldPtr);
    else 
        return realloc(oldPtr, size);

    void *ptr = realloc((char *)oldPtr - AllocationPadding, size + AllocationPadding * 2);
    if(ptr == NULL)
        return NULL;

    FillPadding(ptr, size);
    return (char *)ptr + AllocationPadding;
}

static void WatchHeapCustomDeallocator(void *ptr)
{
    if(!GCInternalThreadData.HeapCallbackActive)
        CheckPaddingContaminated((char *)ptr);
    else
        return free(ptr);

    free((char *)ptr - AllocationPadding);

    // Calling this here because otherwise the dictionary entry is removed too early.
    if(!GCInternalThreadData.HeapCallbackActive)
    {
        GCInternalThreadData.HeapCallbackActive = true;
        WatchHeapFreeCallback(ptr);
        GCInternalThreadData.HeapCallbackActive = false;
    }
}

void GCWatchHeap(size_t backtraceCount, size_t allocationPadding)
{
    // Initializing backtrace state here to avoid a potential race condition in multithreaded programs.
    GCInternalGetBacktraceState();

    if(allocationPadding > 0)
    {
        GCSetCustomAllocator(WatchHeapCustomAllocator);
        GCSetCustomReallocator(WatchHeapCustomReallocator);
        GCSetCustomDeallocator(WatchHeapCustomDeallocator);
    }
    else
        GCSetFreeCallback(WatchHeapFreeCallback);

    GCSetMallocCallback(WatchHeapMallocCallback);
    GCSetReallocCallback(WatchHeapReallocCallback);
    BacktraceCount = backtraceCount;
    AllocationPadding = allocationPadding;
}

void GCStopWatchingHeap()
{
    if(AllocationPadding > 0)
    {
        GCSetCustomAllocator(NULL);
        GCSetCustomReallocator(NULL);
        GCSetCustomDeallocator(NULL);
    }
    else
        GCSetFreeCallback(NULL);

    GCSetMallocCallback(NULL);
    GCSetReallocCallback(WatchHeapReallocCallback);

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
            *data = DictGetValue(&HeapDict, *index);

        pthread_mutex_unlock(&HeapDictMutex);
        return *DictGetKey(&HeapDict, *index);
    }
    pthread_mutex_unlock(&HeapDictMutex);

    return NULL;
}

void GCPrintHeap()
{
    printf("Printing heap contents:\n");

    void *ptr;
    GCAllocationData data;
    for(size_t index = 0; (ptr = GCIterateHeap(&index, &data)) != NULL; index++)
        PrintAllocation(ptr, data);
}

void GCPrintMemory(const void *ptr, const size_t size)
{
    for(size_t x = 0; x < size; x++)
    {
        if(x % 8 == 0)
            printf("\n%04zu: ", x);
        printf("%02X ", (unsigned char)((char *)ptr)[x]);
    }
    printf("\n");
}