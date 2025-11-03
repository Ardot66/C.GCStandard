#ifndef ___GC_COLLECTIONS___
#define ___GC_COLLECTIONS___

#include <stddef.h>
#include <stdint.h>

// Dictionaries provide an efficient way to compare large sets of data and quickly look up values.
// They can be used to both relate keys and values, or just store keys by passing DictNoValue into 
// the valueType of this macro. 
//
// With the exception of DictAdd, all dictionary functions work with indices obtained using
// DictIndexOf. Such indices are only valid until any call that adds or removes from the dictionary,
// at which point use of the any previously obtained index is undefined behaviour.
#define DictDefine(keyType, valueType, typeName) typedef struct typeName { union { struct {void *V; size_t Length; size_t Count; }; keyType *_Key; valueType *_Value;};} typeName

DictDefine(void, void, DictGeneric);
typedef struct {} DictNoValue;

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
void DictResizeGeneric(DictGeneric *dictionary, const size_t newLength, DictFunctions functions, const size_t keySize, const size_t valueSize);
size_t DictAddGeneric(DictGeneric *dictionary, const void *key, const void *value, DictFunctions functions, const size_t keySize, const size_t valueSize);
void DictRemoveGeneric(DictGeneric *dictionary, const size_t index, DictFunctions functions, const size_t keySize, const size_t valueSize);
int DictIterateGeneric(const DictGeneric *dictionary, size_t *index, const size_t keySize, const size_t valueSize);

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
// This requires that key be a variable that can have its address taken.
#define DictAdd(dict, key, value, functions) DictAddGeneric(M_DictGenericParams(dict, &(key), &(value), functions))
#define DictRemove(dict, index, functions) DictRemoveGeneric(M_DictGenericParams(dict, index, functions))
#define DictIterate(dict, index) DictIterateGeneric(M_DictGenericParams(dict, index))

#endif 