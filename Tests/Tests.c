// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "TestingUtilities.h" 
#include "CollectionsPlus.h"
#include "Try.h"

ListDeclare(int, int);

int TestList()
{
    List(int) list;
    ListInit(&list, 0);

    for(int x = 0; x < 4; x++)
    {
        ListAdd(&list, &x);
        TEST(list.V[x], ==, x, d)
        TEST(list.Count, ==, x + 1, zu)
    }

    ListRemoveAt(&list, 0);
    TEST(list.V[0], ==, 1, d)

    ListClear(&list);
    TEST(list.Count, ==, 0, zu)

    for(int x = 0; x < 4; x++)
    {
        ListInsert(&list, &x, 0);

        for(int y = 0; y <= x; y++)
            TEST(list.V[y], ==, x - y, d)
    }
}

int main(int argc, char **argv)
{
    TestList();
    TestsEnd();
}