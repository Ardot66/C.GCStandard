#include <stdio.h>
#include "GCException.h"
#include "Tests.h"
size_t MetaTestsRun = 0, MetaTestsPassed = 0;

void TestException();
void TestCollections();
void TestCommandQueue();
void TestMemory();
void TestArena();

int main()
{
    printf("\nTesting exception\n---\n");
    TestException();

    printf("\nTesting collections\n---\n");
    TestCollections();

    printf("\nTesting command queue\n--\n");
    TestCommandQueue();

    printf("\nTesting memory\n--\n");
    TestMemory();

    printf("\nTesting arena\n--\n");
    TestArena();

    printf("\nFinal results: %zu out of %zu test bundles passed\n", MetaTestsPassed, MetaTestsRun);
}