#include "GCArena.h"
#include "GCTestingUtilities.h"
#include "GCMemoryDebugTools.h"
#include "Tests.h"

void PrintMemory(const void *ptr, const size_t size);
void TestArena()
{
    GCWatchHeap(16, 32);

    GCArena arena = GCArenaDefault;

    GCArenaReserve(&arena, 100);

    const size_t allocCount = 20;
    void *allocation[allocCount];
    for(size_t x = 0; x < allocCount; x++)
    {
        const size_t blockSize = 58;
        allocation[x] = GCArenaAllocate(&arena, blockSize);
        memset(allocation[x], 0, blockSize);
        if(x != 0)
            TEST(allocation[x], >, (char *)allocation[x - 1] + blockSize - 1);
    }

    GCArenaFree(&arena);

    size_t index = 0;
    TEST(GCIterateHeap(&index, NULL), ==, NULL);
    GCStopWatchingHeap();

    PrintTestStatus(NULL);
    MetaTest(TestsPassed, TestsRun);
}