// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "TestingUtilities.h" 
#include "CollectionsPlus.h"
#include "Exception.h"

TypedefList(int, ListInt);

int TestList()
{
    ListInt list;
    Exception exception;
    TryBegin(exception);
        ListInit(&list, 0);

        for(int x = 0; x < 4; x++)
        {
            printf("Next ref: %p\n", _NextBufRef);
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

int main(int argc, char **argv)
{
    TestList();
    TestsEnd();
}