#ifndef ___GC_COLLECTIONS___
#define ___GC_COLLECTIONS___

#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>
#include "GCResult.h"

// Dictionaries provide an efficient way to compare large sets of data and quickly look up values.
// They can be used to both relate keys and values, or just store keys by passing a type of size
// zero in the valueType parameter of this macro. No standard zero length type is provided, as
// such types are unfortunately nonstandard.
//
// With the errorInfo of DictAdd, all dictionary functions work with indices obtained using
// DictIndexOf. Such indices are only valid until any call that adds or removes from the dictionary,
// at which point use of the any previously obtained index is undefined behaviour.
#define DictDefine(keyType, valueType, typeName) typedef struct typeName { union { struct {void *V; size_t Length; size_t Count; }; keyType *_Key; valueType *_Value;};} typeName

DictDefine(void, void, DictGeneric);

typedef struct DictFunctions
{
    uint64_t (*Hash)(const size_t keySize, const void *key);
    int (*Equate)(const size_t size, const void *a, const void *b);
} DictFunctions;

static inline void *DictGetKeyGeneric(const DictGeneric *dictionary, const size_t index, const size_t keySize, const size_t valueSize)
{
    return ((char *)dictionary->V) + index * (keySize + valueSize);
}

static inline void *DictGetValueGeneric(const DictGeneric *dictionary, const size_t index, const size_t keySize, const size_t valueSize)
{
    return ((char *)dictionary->V) + index * (keySize + valueSize) + keySize;
}

#define DictDefault {.V = NULL, .Length = 0, .Count = 0}

ssize_t DictIndexOfGeneric(const DictGeneric*dictionary, const void *key, DictFunctions functions, const size_t keySize, const size_t valueSize);
void DictFreeGeneric(DictGeneric *dictionary);
GCError DictResizeGeneric(DictGeneric *dictionary, const size_t newLength, DictFunctions functions, const size_t keySize, const size_t valueSize);
GCError DictAddGeneric(DictGeneric *dictionary, const void *key, const void *value, size_t *indexDest, DictFunctions functions, const size_t keySize, const size_t valueSize);
void DictRemoveGeneric(DictGeneric *dictionary, const size_t index, DictFunctions functions, const size_t keySize, const size_t valueSize);
GCResult DictIterateGeneric(const DictGeneric *dictionary, size_t *index, const size_t keySize, const size_t valueSize);

size_t DictDefaultHash(const size_t keySize, const void *key);
int DictDefaultEquate(const size_t keySize, const void *keyA, const void *keyB);
#define DictDefaultFunctions (DictFunctions){.Hash = DictDefaultHash, .Equate = DictDefaultEquate}

size_t DictDefaultStringHash(const size_t keySize, const void *key);
int DictDefaultStringEquate(const size_t keySize, const void *keyA, const void *keyB);
#define DictDefaultStringFunctions (DictFunctions){.Hash = DictDefaultStringHash, .Equate = DictDefaultStringEquate}

#define M_DictGenericParams(dict, ...) (DictGeneric *)dict, ## __VA_ARGS__, sizeof(*(dict)->_Key), sizeof(*(dict)->_Value)
#define DictGetKey(dict, index) (*(typeof((dict)->_Key))DictGetKeyGeneric(M_DictGenericParams(dict, index)))
#define DictGetValue(dict, index) (*(typeof((dict)->_Value))DictGetValueGeneric(M_DictGenericParams(dict, index)))
#define DictIndexOf(dict, key, functions) DictIndexOfGeneric(M_DictGenericParams(dict, &(key), functions))
#define DictFree(dict) DictFreeGeneric((DictGeneric *)dict)
#define DictResize(dict, newLength, functions) DictResizeGeneric(M_DictGenericParams(dict, newLength, functions))
#define DictAdd(dict, key, value, indexDest, functions) DictAddGeneric(M_DictGenericParams(dict, &(key), &(value), indexDest, functions))
#define DictRemove(dict, index, functions) DictRemoveGeneric(M_DictGenericParams(dict, index, functions))
// Does not throw any errors, returns GC_RESULT_FAILURE after reaching the end of the dictionary.
#define DictIterate(dict, index) DictIterateGeneric(M_DictGenericParams(dict, index))

#endif 