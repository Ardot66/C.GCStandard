// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "GCTestingUtilities.h" 
#include "GCList.h"
#include "GCDictionary.h"
#include "GCCList.h"
#include "Tests.h"
#include "GCResult.h"

#define countof(list) (sizeof(list) / sizeof(*list))

ListDefine(int, ListInt);

void TestList()
{
    ListInt list = ListDefault;
    for(int x = 0; x < 4; x++)
    {
        Try(ListAdd(&list, x));

        TEST(list.V[x], ==, x)
        TEST(list.Count, ==, x + 1)
    }

    Try(ListRemoveAt(&list, 0));
    TEST(list.V[0], ==, 1)

    ListClear(&list);
    TEST(list.Count, ==, 0)

    for(int x = 0; x < 4; x++)
    {
        Try(ListInsert(&list, x, 0));

        for(int y = 0; y <= x; y++)
            TEST(list.V[y], ==, x - y)
    }

    int rangeToAdd[] = {1, 5, 3, 71};
    ListClear(&list);
    Try(ListAddRange(&list, rangeToAdd, 4));
    TEST(list.Count, ==, 4);

    for(int x = 0; x < 4; x++)
        TEST(list.V[x], ==, rangeToAdd[x]);

    ListFree(&list);

    ErrorLabel;
    IfError
    {
        ErrorInfoPrintCurrent();
        TEST(1, ==, 0);
    }
    return;
}

CListDefine(int, CListInt);

void TestCList()
{
    CListInt list = CListDefault;

    int insertIndices[] = {0, 0, 0, 2, 4};
    int finalResult[] = {2, 1, 3, 0, 4};

    for(size_t x = 0; x < countof(insertIndices); x++)
        Try(CListInsert(&list, x, insertIndices[x]));

    TEST(list.Count, ==, 5);
    for(size_t x = 0; x < countof(insertIndices); x++)
        TEST(CListGet(&list, x), ==, finalResult[x]);

    int removeIndices[] = {1, 3};
    int removeResult[] = {2, 3, 0};
    for(size_t x = 0; x < countof(removeIndices); x++)
        Try(CListRemoveAt(&list, removeIndices[x]));

    TEST(list.Count, ==, 3);
    for(size_t x = 0; x < countof(removeResult); x++)
        TEST(CListGet(&list, x), ==, removeResult[x]);

    int insertRange[] = {1, 2, 3, 4};
    int insertRangeResult[] = {2, 1, 2, 3, 4, 3, 0};
    Try(CListInsertRange(&list, insertRange, countof(insertRange), 1));

    TEST(list.Count, ==, countof(insertRangeResult));
    for(size_t x = 0; x < countof(insertRangeResult); x++)
        TEST(CListGet(&list, x), ==, insertRangeResult[x]);

    const size_t stressTest = 1000;
    CListClear(&list);
    for(size_t x = 0; x < stressTest; x++)
    {
        size_t value = ((x * 50 + 23) << 2) * 43;
        Try(CListAdd(&list, value));
    }
    for(size_t x = 0; x < stressTest; x++)
    {
        size_t value = ((x * 50 + 23) << 2) * 43;
        TEST(CListGet(&list, x), ==, value);
    }

    ErrorLabel;
    IfError
    {
        ErrorInfoPrint(ErrorInfoGetCurrent());
        TEST(1, ==, 0);
    }

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

    const size_t elements = 100, removeElements = 50;
    for(size_t x = 0; x < elements; x++)
    {
        size_t key = DictKey(x);
        Try(DictAdd(&dict, key, x, NULL, functions));
    }

    for(size_t x = 0; x < elements; x++)
    {
        size_t key = DictKey(x);
        ssize_t index = DictIndexOf(&dict, key, functions);

        TEST(index, !=, -1);
        TEST(DictGetValue(&dict, index), ==, x);
    }

    for(size_t x = 0; x < removeElements; x++)
    {
        size_t key = DictKey(x);
        size_t index = DictIndexOf(&dict, key, functions);
        DictRemove(&dict, index, functions);
    }
    
    for(size_t x = removeElements; x < elements; x++)
    {
        size_t key = DictKey(x);
        ssize_t index = DictIndexOf(&dict, key, functions);

        TEST(index, !=, -1);
        TEST(DictGetValue(&dict, index), ==, x);
    }

    ErrorLabel;
    IfError
    {
        ErrorInfoPrint(ErrorInfoGetCurrent());
        TEST(1, ==, 0);
    }

    DictFree(&dict);
}

void TestCollections()
{
    TestList();
    TestCList();
    TestDictionary();

    PrintTestStatus(NULL);
    MetaTest(TestsPassed, TestsRun);
}