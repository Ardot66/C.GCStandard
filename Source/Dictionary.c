#include "GCCollections.h"
#include "GCException.h"
#include "GCMemory.h"
#include <stdlib.h>
#include <string.h>

#define SIZE_BITS(type) (sizeof(type) * CHAR_BIT)
typedef size_t ExistsListInt;

const double DictResizeFraction = 0.5f;
const size_t DictInitialLength = 16;

static inline size_t DictGetExistsListCount(const size_t length)
{
    return length / SIZE_BITS(ExistsListInt) + (length % SIZE_BITS(ExistsListInt) != 0);
}

static inline ExistsListInt *DictGetExistsList(const DictGeneric *dictionary, const size_t keySize, const size_t valueSize)
{
    return (ExistsListInt *)((char *)dictionary->V + (keySize + valueSize) * dictionary->Length);
}

static inline int DictGetElementExists(const DictGeneric *dictionary, const size_t index, const size_t keySize, const size_t valueSize)
{
    ExistsListInt *existsList = DictGetExistsList(dictionary, keySize, valueSize);
    return (existsList[index / SIZE_BITS(ExistsListInt)] >> (SIZE_BITS(ExistsListInt) - 1 - (index % SIZE_BITS(ExistsListInt)))) & (ExistsListInt)0x1;
}

static inline void DictSetElementExists(const DictGeneric *dictionary, const size_t index, const int exists, const size_t keySize, const size_t valueSize)
{
    ExistsListInt *existsList = DictGetExistsList(dictionary, keySize, valueSize);

    size_t numIndex = index / SIZE_BITS(ExistsListInt);
    size_t flipBit = (ExistsListInt)0x1 << (SIZE_BITS(ExistsListInt) - 1 - (index % SIZE_BITS(ExistsListInt)));

    existsList[numIndex] = exists * (existsList[numIndex] | flipBit) + !exists * (existsList[numIndex] & ~flipBit);
}

static inline size_t DictGetSize(const size_t length, const size_t keySize, const size_t valueSize)
{
    return length * (keySize + valueSize) + DictGetExistsListCount(length) * sizeof(ExistsListInt);
}

ssize_t DictIndexOfGeneric(const DictGeneric*dictionary, const void *key, const DictFunctions functions, const size_t keySize, const size_t valueSize)
{
    // Avoid modulo by zero exception.
    if(dictionary->Length == 0)
        return -1;

    uint64_t hash = functions.Hash(keySize, key);

    for(size_t x = 0, checkIndex = hash % dictionary->Length; x < dictionary->Length; x++, checkIndex = (checkIndex + 1) % dictionary->Length)
    {
        if(!DictGetElementExists(dictionary, checkIndex, keySize, valueSize))
            break;
        
        void *curKey = DictGetKeyGeneric(dictionary, checkIndex, keySize, valueSize);

        if(functions.Equate(keySize, key, curKey))
            continue;

        return checkIndex;
    }

    return -1;
}

void DictFreeGeneric(DictGeneric *dictionary)
{
    GCFree(dictionary->V);
}

void DictResizeGeneric(DictGeneric *dictionary, const size_t newLength, const DictFunctions functions, const size_t keySize, const size_t valueSize)
{
    DictGeneric newDictionary = DictDefault;
    ExitInit();

    newDictionary.Count = 0;
    newDictionary.Length = newLength;
    newDictionary.V = GCMalloc(DictGetSize(newLength, keySize, valueSize));

    memset(DictGetExistsList(&newDictionary, keySize, valueSize), 0, DictGetExistsListCount(newDictionary.Length) * sizeof(ExistsListInt));

    for(size_t index = 0; DictIterateGeneric(dictionary, &index, keySize, valueSize); index++)
    {   
        void *key = DictGetKeyGeneric(dictionary, index, keySize, valueSize);
        void *value = DictGetValueGeneric(dictionary, index, keySize, valueSize);

        DictAddGeneric(&newDictionary, key, value, functions, keySize, valueSize);
    }

    ExitBegin();
    IfExitException
        GCFree(newDictionary.V);
    else
        GCFree(dictionary->V);
    ExitEnd();

    *dictionary = newDictionary;
}

size_t DictAddGeneric(DictGeneric *dictionary, const void *key, const void *value, const DictFunctions functions, const size_t keySize, const size_t valueSize)
{
    if(dictionary->Length == 0 || (double)dictionary->Count / (double)dictionary->Length > DictResizeFraction)
        DictResizeGeneric(dictionary, dictionary->Length == 0 ? DictInitialLength : dictionary->Length * 2, functions, keySize, valueSize);

    uint64_t hash = functions.Hash(keySize, key);

    for(size_t x = 0, checkIndex = hash % dictionary->Length; x < dictionary->Length; x++, checkIndex = (checkIndex + 1) % dictionary->Length)
    {
        if(DictGetElementExists(dictionary, checkIndex, keySize, valueSize))
            continue;

        DictSetElementExists(dictionary, checkIndex, 1, keySize, valueSize);

        void *keyDest = DictGetKeyGeneric(dictionary, checkIndex, keySize, valueSize);
        memcpy(keyDest, key, keySize);

        if(value != NULL)
        {
            void *valueDest = DictGetValueGeneric(dictionary, checkIndex, keySize, valueSize);
            memcpy(valueDest, value, valueSize);
        }

        dictionary->Count += 1;
        return checkIndex;
    }

    Throw("Failed to add value to dictionary (this shouldn't happen)");
    return 0;
}

void DictRemoveGeneric(DictGeneric *dictionary, const size_t index, const DictFunctions function, const size_t keySize, const size_t valueSize)
{
    size_t curIndex = index;
    size_t nextIndex = index + 1;
    void *curKey = DictGetKeyGeneric(dictionary, curIndex, keySize, valueSize);
    void *nextKey = DictGetKeyGeneric(dictionary, nextIndex % dictionary->Length, keySize, valueSize);
    uint64_t nextHash = function.Hash(keySize, nextKey) % dictionary->Length;

    for(size_t x = 0; x < dictionary->Length; x++)
    {
        if(!DictGetElementExists(dictionary, nextIndex % dictionary->Length, keySize, valueSize))
            break;

        if(nextHash <= curIndex)
        {
            memcpy(curKey, DictGetKeyGeneric(dictionary, nextIndex % dictionary->Length, keySize, valueSize), keySize + valueSize);

            curIndex = nextIndex;
            curKey = nextKey;
        }

        nextIndex++;
        nextKey = DictGetKeyGeneric(dictionary, nextIndex % dictionary->Length, keySize, valueSize); 
        nextHash = function.Hash(keySize, nextKey) % dictionary->Length;
    }

    DictSetElementExists(dictionary, curIndex % dictionary->Length, 0, keySize, valueSize);
    dictionary->Count -= 1;
}

int DictIterateGeneric(const DictGeneric *dictionary, size_t *index, const size_t keySize, const size_t valueSize)
{
    for(; *index < dictionary->Length; *index += 1)
    {
        if(!DictGetElementExists(dictionary, *index, keySize, valueSize))
            continue;

        return 1;
    }

    return 0;
}

size_t DictDefaultHash(const size_t keySize, const void *key)
{
    size_t hash = 5381;

    for(size_t x = 0; x < keySize; x++)
    {
        size_t keyByte = (size_t)(((char *)key)[x]);
        hash = ((hash << 5) + hash) + keyByte;
    }

    return hash;
}

int DictDefaultEquate(const size_t keySize, const void *keyA, const void *keyB)
{
    int equal = 0;

    for(size_t x = 0; x < keySize; x++)
        equal |= ((char *)keyA)[x] != ((char *)keyB)[x];
    
    return equal;
}

size_t DictDefaultStringHash(const size_t keySize, const void *key)
{
    (void)keySize;
    return DictDefaultHash(strlen(key), *(char **)key);
}

int DictDefaultStringEquate(const size_t keySize, const void *keyA, const void *keyB)
{
    (void)keySize;
    return strcmp(*(char **)keyA, *(char **)keyB);
}