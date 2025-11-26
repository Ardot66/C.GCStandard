#include "GCArena.h"
#include "GCMemory.h"

static inline GCError GCArenaAddBlock(GCArena *arena, const size_t size)
{
    size_t blockSize = arena->ActiveBlock != NULL ? arena->ActiveBlock->Size << 1 : 64;
    while(blockSize < size)
        blockSize <<= 1;

    GCArenaBlock *block = GCMalloc(sizeof(*block) * blockSize);
    if(block == NULL)
        GotoError;
    block->Size = blockSize;
    block->Next = arena->ActiveBlock;
    arena->ActiveBlock = block;
    arena->Used = 0;

    ErrorLabel;
    return Error;
}

void *GCArenaAllocate(GCArena *arena, const size_t size)
{
    if(arena->ActiveBlock == NULL)
        Try(GCArenaAddBlock(arena, size));

    if(arena->ActiveBlock->Size - arena->Used < size)
        Try(GCArenaAddBlock(arena, size));
    
    void *ptr = arena->ActiveBlock->Data + arena->Used;
    arena->Used += size;

    ErrorLabel;
    IfError
        return NULL;
    return ptr;
}

GCError GCArenaReserve(GCArena *arena, const size_t size)
{
    if(arena->ActiveBlock != NULL && size < arena->ActiveBlock->Size - arena->Used)
        return GC_SUCCESS;
    return GCArenaAddBlock(arena, size);
}

void GCArenaFree(GCArena *arena)
{
    if(arena == NULL)
        return;

    GCArenaBlock *block = arena->ActiveBlock;
    while(block != NULL)
    {
        GCArenaBlock *nextBlock = block->Next;
        GCFree(block);
        block = nextBlock;
    }
}