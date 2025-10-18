// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "GCTestingUtilities.h" 
#include "GCCollections.h"
#include "GCException.h"

TypedefList(int, ListInt);

void TestList()
{
    ListInt list;
    Exception exception;
    TryBegin(exception);
        ListInit(&list, 0);

        for(int x = 0; x < 4; x++)
        {
            ListAdd(&list, &x);

            TEST(list.V[x], ==, x)
            TEST(list.Count, ==, x + 1)
        }

        ListRemoveAt(&list, 0);
        TEST(list.V[0], ==, 1)

        ListClear(&list);
        TEST(list.Count, ==, 0)

        for(int x = 0; x < 4; x++)
        {
            ListInsert(&list, &x, 0);

            for(int y = 0; y <= x; y++)
                TEST(list.V[y], ==, x - y)
        }
    TryEnd;

    if(exception.Type)
        PrintException(exception);
    TEST_TYPED(exception.Type, ==, 0, int, "d");

    ListFree(&list);
}

void TestCollections()
{
    TestList();

    PrintTestStatus(NULL);
}