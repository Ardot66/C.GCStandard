#include <stdio.h>
#include "GCException.h"

void TestException();
void TestCollections();
void TestCommandQueue();
void TestMemory();

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
}