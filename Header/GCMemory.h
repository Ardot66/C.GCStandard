#ifndef __GC_MEMORY__
#define __GC_MEMORY__

#include <stdint.h>

// Allows overriding the default GCMalloc allocator. Set to NULL to restore default.
void GCSetCustomAllocator(void *(* allocator)(const size_t size));

// Allows overriding the default GCRealloc reallocator. Set to NULL to restore default.
void GCSetCustomReallocator(void *(*customReallocator)(void *oldPtr, const size_t size));

// Allows overriding the default GCFree deallocator. Set to NULL to restore default.
void GCSetCustomDeallocator(void (* deallocator)(void *ptr));


// Allows specifying a callback that runs every time GCMalloc is called. Such callbacks are relatively
// slow and should only really be used for debugging. Set to NULL to restore to default.
void GCSetMallocCallback(void (*callback)(void *ptr, const size_t size));

// Allows specifying a callback that runs every time GCRealloc is called. Such callbacks are relatively
// slow and should only really be used for debugging. Set to NULL to restore to default.
void GCSetReallocCallback(void (*reallocCallback)(void *oldPtr, void *ptr, const size_t size));

// Allows specifying a callback that runs every time GCFree is called. Such callbacks are relatively
// slow and should only really be used for debugging. Set to NULL to restore to default.
void GCSetFreeCallback(void (*callback)(void *ptr));


// Works the same as malloc, but allows for custom allocators and callbacks to be set.
// Automatically throws exceptions if the allocator returns NULL.
void *GCMalloc(const size_t size);

// Same as GCMalloc but will return NULL rather than throwing an exception.
void *GCMallocNoExcept(const size_t size);


// Same as GCMalloc but clears the allocated memory to be all zero.
void *GCCalloc(const size_t size);

// Same as GCMallocNoExcept but clears the allocated memory to be all zero.
void *GCCallocNoExcept(const size_t size);


// Works the same as realloc, but allows for custom reallocators and callbacks to be set.
// Automatically throws exceptions if the reallocator returns NULL.
// NOTE: Only triggers the realloc callback on reallocations, calling the malloc callback instead
// if the passed oldPtr is NULL.
void *GCRealloc(void *oldPtr, const size_t size);

// Same as GCRealloc but will return NULL rather than throwing an exception.
void *GCReallocNoExcept(void *oldPtr, const size_t size);

// Works the same as free, but allows for custom deallocators and callbacks to be set.
void GCFree(void *ptr);

#endif