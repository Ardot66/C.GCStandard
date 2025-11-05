#ifndef __GC_ARENA__
#define __GC_ARENA__

#include <stddef.h>

typedef struct GCArenaBlock GCArenaBlock;
struct GCArenaBlock
{
    GCArenaBlock *Next;
    size_t Size;
    char Data[];
};

typedef struct GCArena
{
    GCArenaBlock *ActiveBlock;
    size_t Used;
} GCArena;

#define GCArenaDefault {.ActiveBlock = NULL, .Used = 0}

void *GCArenaAllocate(GCArena *arena, const size_t size);
void GCArenaReserve(GCArena *arena, const size_t size);
void GCArenaFree(GCArena *arena);

#endif