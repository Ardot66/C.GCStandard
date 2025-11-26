#include "GCMemory.h"
#include "GCMemoryDebugTools.h"
#include "GCResult.h"
#include "GCTestingUtilities.h"
#include "Tests.h"
#include <stdio.h>

void TestMemory()
{
    GCWatchHeap(0, 5);

    const size_t allocateCount = 5;
    void *allocatedPtrs[allocateCount];
    for(size_t x = 0; x < allocateCount; x++)
    {
        allocatedPtrs[x] = GCMalloc(64 * x);
        if(allocatedPtrs[x] == NULL)
            GotoError;
    }

    for(size_t x = 0; x < allocateCount; x++)
    {
        if(x % 2 == 0)
            continue;
        allocatedPtrs[x] = GCRealloc(allocatedPtrs[x], 8 + x);
        if(allocatedPtrs[x] == NULL)
            GotoError;
    }

    ((char **)allocatedPtrs)[1][11] = 10;
    ((char **)allocatedPtrs)[1][-2] = 10;

    memset(allocatedPtrs[2], 0, 64 * 2);

    void *iterPtr;
    for(size_t x = 0; (iterPtr = GCIterateHeap(&x, NULL)) != NULL; x++)
    {
        bool found = false;
        for(size_t y = 0; y < allocateCount; y++)    
        {
            if(iterPtr == allocatedPtrs[y])
            {
                found = true;
                break;
            }
        }
        TEST(found, ==, true);
    }

    GCPrintHeap();

    for(size_t x = 0; x < allocateCount; x++)
        GCFree(allocatedPtrs[x]);

    size_t index = 0;
    TEST(GCIterateHeap(&index, NULL), ==, NULL);

    GCStopWatchingHeap();

    ErrorLabel;
    IfError
    {
        ErrorInfoPrint(ErrorInfoGetCurrent());
        TEST(1, ==, 0);
    }

    PrintTestStatus(NULL);
    MetaTest(TestsPassed, TestsRun);
}
