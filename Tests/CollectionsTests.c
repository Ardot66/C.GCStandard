// #define TESTING_UTILITIES_VERBOSE

#include <error.h>
#include "GCTestingUtilities.h" 
#include "GCCollections.h"
#include "GCException.h"

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
    TryEnd;

    if(exception.Type)
        PrintException(exception);
    TEST_TYPED(exception.Type, ==, 0, int, "d");

    ListFree(&list);
}

DictDefine(size_t, size_t, DictIntInt);
 
#define SIZE_BITS(type) (sizeof(type) * CHAR_BIT)
typedef size_t ExistsListInt;

static inline ExistsListInt *DictGetExistsList(const DictGeneric *dictionary, const size_t keySize, const size_t valueSize)
{
    return (ExistsListInt *)((char *)dictionary->V + (keySize + valueSize) * dictionary->Length);
}

static inline int DictGetElementExists(const DictGeneric *dictionary, const size_t index, const size_t keySize, const size_t valueSize)
{
    ExistsListInt *existsList = DictGetExistsList(dictionary, keySize, valueSize);
    return (existsList[index / SIZE_BITS(ExistsListInt)] >> (SIZE_BITS(ExistsListInt) - 1 - (index % SIZE_BITS(ExistsListInt)))) & (ExistsListInt)0x1;
}

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
            TEST(*(size_t *)DictGetValue(&dict, index), ==, x);
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
            TEST(*(size_t *)DictGetValue(&dict, index), ==, x);
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
    TestDictionary();

    PrintTestStatus(NULL);
}