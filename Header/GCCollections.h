#ifndef ___GC_COLLECTIONS___
#define ___GC_COLLECTIONS___

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>

#include "GCMemory.h"

#define ListDefine(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count;} typeName

ListDefine(void, ListGeneric);
#define ListDefault {.V = NULL, .Length = 0, .Count = 0}

void ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize);
void ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize);
void ListAddRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize);
void ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize);
void ListInsertRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize);
void ListRemoveRangeGeneric(ListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize);
void ListClearGeneric(ListGeneric *list);

#define M_ListGenericParams(list, ...) (ListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define ListAdd(list, value) do {ListAddGeneric(M_ListGenericParams(list, NULL)); (list)->V[(list)->Count - 1] = (value);} while (0)
#define ListAddRange(list, range, rangeCount) ListAddRangeGeneric(M_ListGenericParams(list, range, rangeCount))
#define ListInsert(list, value, index) do {ListInsertGeneric(M_ListGenericParams(list, NULL, index)); (list)->V[index] = (value);} while (0)
#define ListInsertRange(list, range, rangeCount, index) ListInsertRangeGeneric(M_ListGenericParams(list, range, rangeCount, index))
#define ListRemoveAt(list, index) ListRemoveRangeGeneric(M_ListGenericParams(list, index, 1))
#define ListRemoveRange(list, index, count) ListRemoveRangeGeneric(M_ListGenericParams(list, index, count))
#define ListResize(list, length) ListResizeGeneric(M_ListGenericParams(list, length))
#define ListClear(list) ListClearGeneric((ListGeneric *)list)
#define ListFree(list) GCFree((list)->V);

#define ListValid(list) ((list) != NULL && (list)->V != NULL)

#define CListDefine(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count; size_t Offset;} typeName

CListDefine(void, CListGeneric);
#define CListDefault {.V = NULL, .Length = 0, .Count = 0, .Offset = 0}

#define CListGet(list, index) ((list)->V[((index) + (list)->Offset) % (list)->Length])
#define CListSet(list, index, value) ((list)->V[((index) + (list)->Offset) % (list)->Length] = (value))

void CListResizeGeneric(CListGeneric *list, const size_t newLength, const size_t elemSize);
void CListAddGeneric(CListGeneric *list, const void *value, const size_t elemSize);
void CListAddRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize);
void CListInsertGeneric(CListGeneric *list, const void *value, const size_t index, const size_t elemSize);
void CListInsertRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize);
void CListRemoveAtGeneric(CListGeneric *list, const size_t index, const size_t elemSize);
void CListRemoveRangeGeneric(CListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize);
void CListClearGeneric(CListGeneric *list);

#define M_CListGenericParams(list, ...) (CListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define CListAdd(list, value) do {CListAddGeneric(M_CListGenericParams(list, NULL)); CListSet(list, (list)->Count - 1, value);} while (0)
#define CListAddRange(list, range, rangeCount) CListAddRangeGeneric(M_CListGenericParams(list, range, rangeCount))
#define CListInsert(list, value, index) do {CListInsertGeneric(M_CListGenericParams(list, NULL, index)); CListSet(list, index, value);} while (0)
#define CListInsertRange(list, range, rangeCount, index) CListInsertRangeGeneric(M_CListGenericParams(list, range, rangeCount, index))
#define CListRemoveAt(list, index) CListRemoveRangeGeneric(M_CListGenericParams(list, index, 1))
#define CListRemoveRange(list, index, count) CListRemoveRangeGeneric(M_CListGenericParams(list, index, count))
#define CListResize(list, length) CListResizeGeneric(M_CListGenericParams(list, length))
#define CListClear(list) CListClearGeneric((CListGeneric *)list)
#define CListFree(list) GCFree((list)->V);

typedef struct DictFunctions
{
    uint64_t (*Hash)(const size_t keySize, const void *key);
    int (*Equate)(const size_t size, const void *a, const void *b);
} DictFunctions;

#define DictDefine(keyType, valueType, typeName) \
typedef struct typeName { union { struct {void *V; size_t Length; size_t Count; }; keyType *_Key; valueType *_Value;};} typeName

DictDefine(void, void, DictGeneric);

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
#define DictDefaultFunctions {.Hash = DictDefaultHash, .Equate = DictDefaultEquate}

size_t DictDefaultStringHash(const size_t keySize, const void *key);
int DictDefaultStringEquate(const size_t keySize, const void *keyA, const void *keyB);
#define DictDefaultStringFunctions {.Hash = DictDefaultStringHash, .Equate = DictDefaultStringEquate}

#define M_DictGenericParams(dict, ...) (DictGeneric *)dict, ## __VA_ARGS__, sizeof(*(dict)->_Key), sizeof(*(dict)->_Value)
#define DictGetKey(dict, index) (typeof((dict)->_Key))DictGetKeyGeneric(M_DictGenericParams(dict, index))
#define DictGetValue(dict, index) (typeof((dict)->_Value))DictGetValueGeneric(M_DictGenericParams(dict, index))
#define DictIndexOf(dict, key, functions) DictIndexOfGeneric(M_DictGenericParams(dict, &(key), functions))
#define DictFree(dict) DictFreeGeneric((DictGeneric *)dict)
#define DictResize(dict, newLength, functions) DictResizeGeneric(M_DictGenericParams(dict, newLength, functions))
// This requires that key be a variable that can have its address taken.
#define DictAdd(dict, key, value, functions) DictAddGeneric(M_DictGenericParams(dict, &(key), &(value), functions))
#define DictRemove(dict, index, functions) DictRemoveGeneric(M_DictGenericParams(dict, index, functions))
#define DictIterate(dict, index) DictIterateGeneric(M_DictGenericParams(dict, index))

#endif 