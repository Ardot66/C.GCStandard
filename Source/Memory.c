#include "GCInternalGlobals.h"
#include "GCMemory.h"
#include "GCException.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

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
    if(ptr == NULL)
        return;

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

