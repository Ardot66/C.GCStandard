#ifndef __GC_MEMORY_DEBUG_TOOLS__
#define __GC_MEMORY_DEBUG_TOOLS__

typedef struct GCAllocationExtraPCData
{
    size_t Count;
    uintptr_t PCs[];
} GCAllocationExtraPCData;

typedef struct GCAllocationData
{
    size_t Size;
    uintptr_t PC;
    GCAllocationExtraPCData *ExtraPCs;
} GCAllocationData;

// Adds allocation callbacks that track any subsequent allocations, allowing for the heap to be examined
// for memory leaks and other debugging purposes. To stop watching the heap, call GCStopWatchingHeap.
// - BacktraceCount specifies the number of stack frames to be stored with each allocation.
// - AllocationPadding specifies if extra memory should be added as padding around allocations
//   that is watched in order to detect writing memory out of bounds.
//
// NOTE: Only tracks allocations made with GC functions, not the standard malloc, calloc, and realloc.
// NOTE: If allocationPadding is nonzero, this function will set its own custom allocators. As a
// result, any pointers allocated while the heap is being watched cannot be freed after 
// GCStopWatchingHeap is called.
void GCWatchHeap(size_t backtraceCount, size_t allocationPadding);

// Stops watching the heap.
void GCStopWatchingHeap();


// Iterates through the heap one pointer at a time. The passed index must be incremented by one after each call.
void *GCIterateHeap(size_t *index, GCAllocationData *data);

// Prints the current status of the heap for debugging.
void GCPrintHeap();
void GCPrintHeapPadding();

// Neatly prints a span of memory.
void GCPrintMemory(const void *ptr, const size_t size);
void GCPrintBacktrace();
void GCCheckMemoryPadding(const void *ptr);

#endif