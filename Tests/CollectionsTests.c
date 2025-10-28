// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "GCTestingUtilities.h" 
#include "GCCollections.h"
#include "GCException.h"

#define countof(list) (sizeof(list) / sizeof(*list))

ListDefine(int, ListInt);

void TestList()
{
    ListInt list = ListDefault;
    Exception exception;
    TryBegin(exception);
        for(int x = 0; x < 4; x++)
        {
            ListAdd(&list, x);

            TEST(list.V[x], ==, x)
            TEST(list.Count, ==, x + 1)
        }

        ListRemoveAt(&list, 0);
        TEST(list.V[0], ==, 1)

        ListClear(&list);
        TEST(list.Count, ==, 0)

        for(int x = 0; x < 4; x++)
        {
            ListInsert(&list, x, 0);

            for(int y = 0; y <= x; y++)
                TEST(list.V[y], ==, x - y)
        }

        int rangeToAdd[] = {1, 5, 3, 71};
        ListClear(&list);
        ListAddRange(&list, rangeToAdd, 4);
        TEST(list.Count, ==, 4);

        for(int x = 0; x < 4; x++)
            TEST(list.V[x], ==, rangeToAdd[x]);
    TryEnd;

    if(exception.Type)
        PrintException(exception);
    TEST_TYPED(exception.Type, ==, 0, int, "d");

    ListFree(&list);
}

CListDefine(int, CListInt);

void TestCList()
{
    CListInt list = CListDefault;
    Exception exception;
    TryBegin(exception);
        int insertIndices[] = {0, 0, 0, 2, 4};
        int finalResult[] = {2, 1, 3, 0, 4};

        for(size_t x = 0; x < countof(insertIndices); x++)
            CListInsert(&list, x, insertIndices[x]);

        TEST(list.Count, ==, 5);
        for(size_t x = 0; x < countof(insertIndices); x++)
            TEST(CListGet(&list, x), ==, finalResult[x]);

        int removeIndices[] = {1, 3};
        int removeResult[] = {2, 3, 0};
        for(size_t x = 0; x < countof(removeIndices); x++)
            CListRemoveAt(&list, removeIndices[x]);

        TEST(list.Count, ==, 3);
        for(size_t x = 0; x < countof(removeResult); x++)
            TEST(CListGet(&list, x), ==, removeResult[x]);

        int insertRange[] = {1, 2, 3, 4};
        int insertRangeResult[] = {2, 1, 2, 3, 4, 3, 0};
        CListInsertRange(&list, insertRange, countof(insertRange), 1);

        TEST(list.Count, ==, countof(insertRangeResult));
        for(size_t x = 0; x < countof(insertRangeResult); x++)
            TEST(CListGet(&list, x), ==, insertRangeResult[x]);
    TryEnd;

    if(exception.Type)
        PrintException(exception);
    TEST_TYPED(exception.Type, ==, 0, int, "d");

    CListFree(&list);
}

DictDefine(size_t, size_t, DictIntInt);
 
size_t DictKey(size_t index)
{
    size_t keyInit = index * 1023 + 23;
    return DictDefaultHash(sizeof(size_t), &keyInit);
}

void TestDictionary()
{
    DictIntInt dict = DictDefault;
    DictFunctions functions = DictDefaultFunctions;

    Exception exception;
    TryBegin(exception);
        const size_t elements = 20, removeElements = 10;
        for(size_t x = 0; x < elements; x++)
        {
            size_t key = DictKey(x);
            DictAdd(&dict, key, x, functions);
        }

        for(size_t x = 0; x < elements; x++)
        {
            size_t key = DictKey(x);
            ssize_t index = DictIndexOf(&dict, key, functions);

            TEST(index, !=, -1);
            TEST(*DictGetValue(&dict, index), ==, x);
        }

        for(size_t x = 0; x < removeElements; x++)
        {
            size_t key = DictKey(x);
            size_t index = DictIndexOf(&dict, key, functions);
            DictRemove(&dict, index, functions);
        }
        
        for(size_t x = 0; x < elements; x++)
        {
            size_t key = DictKey(x);
            ssize_t index = DictIndexOf(&dict, key, functions);

            if(x < removeElements && index == -1)
                continue;

            TEST(index, !=, -1);
            TEST(*DictGetValue(&dict, index), ==, x);
        }

    TryEnd;

    if(exception.Type)
        PrintException(exception);
    TEST_TYPED(exception.Type, ==, 0, int, "d");

    DictFree(&dict);
}

void TestCollections()
{
    TestList();
    TestCList();
    TestDictionary();

    PrintTestStatus(NULL);
}