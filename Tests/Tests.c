#include <stdio.h>
#include "GCException.h"

void TestException();
void TestCollections();
void TestCommandQueue();

int main()
{
    printf("\nTesting exception\n---\n");
    TestException();

    printf("\nTesting collections\n---\n");
    TestCollections();

    printf("\nTesting command queue\n--\n");
    TestCommandQueue();
}