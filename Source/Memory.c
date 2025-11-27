#include "GCMemory.h"
#include "GCResult.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

// Custom functions for overriding the default malloc and free functions.
static void *(*CustomAllocator)(const size_t size) = NULL;
static void *(*CustomReallocator)(void *oldPtr, const size_t size) = NULL;
static void (*CustomDeallocator)(void *ptr) = NULL;

// Custom functions for adding additional behaviours to memory allocation.
static GCError (*MallocCallback)(void *ptr, const size_t size) = NULL;
static GCError (*ReallocCallback)(void *oldPtr, void *ptr, const size_t size) = NULL;
static void (*FreeCallback)(void *ptr) = NULL;

thread_local bool HeapCallbackActive = false;

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

void GCSetMallocCallback(GCError (*callback)(void *ptr, const size_t size))
{
    MallocCallback = callback;
}

void GCSetReallocCallback(GCError (*reallocCallback)(void *oldPtr, void *ptr, const size_t size))
{
    ReallocCallback = reallocCallback;
}

void GCSetFreeCallback(void (*callback)(void *ptr))
{
    FreeCallback = callback;
}

static inline void CallbackBegin()
{
    HeapCallbackActive = true;
}

static inline void CallbackEnd()
{
    HeapCallbackActive = false;
}

void *GCMalloc(const size_t size)
{
    void *ptr;
    if(CustomAllocator != NULL)
    {
        ptr = CustomAllocator(size);
        if(ptr == NULL)
            GotoError;
    }
    else
    {
        ptr = malloc(size);
        if(!ptr)
            Throw(errno, "Failed to allocate memory");
    }

    if(MallocCallback != NULL && ptr != NULL && !HeapCallbackActive)
    {
        CallbackBegin();
        Try(MallocCallback(ptr, size));
        CallbackEnd();
    }
        
    ErrorLabel;
    IfError
        return NULL;
    return ptr;
}

void *GCCalloc(const size_t size)
{
    void *ptr = GCMalloc(size);
    if(!ptr)
        return NULL;
    memset(ptr, 0, size);
    return ptr;
}

void *GCRealloc(void *oldPtr, const size_t size)
{
    void *ptr;
    // The malloc callback should handle this case
    if(oldPtr == NULL)
        return GCMalloc(size);
    else if(CustomReallocator != NULL)
    {
        ptr = CustomReallocator(oldPtr, size);
        if(ptr == NULL)
            GotoError;
    }
    else
    {
        ptr = realloc(oldPtr, size);
        if(!ptr)
            Throw(errno, "Failed to reallocate memory");
    }

    if(ReallocCallback != NULL && ptr != NULL && !HeapCallbackActive)
    {
        CallbackBegin();
        Try(ReallocCallback(oldPtr, ptr, size));
        CallbackEnd();
    }
    
    ErrorLabel;
    IfError
        return NULL;
    return ptr;
}

void GCFree(void *ptr)
{
    if(ptr == NULL)
        return;

    if(FreeCallback != NULL && ptr != NULL && !HeapCallbackActive)
    {
        CallbackBegin();
        FreeCallback(ptr);
        CallbackEnd();
    }

    if(CustomDeallocator != NULL)
        CustomDeallocator(ptr);
    else
        free(ptr);
}

