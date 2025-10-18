#ifndef ___GC_COLLECTIONS___
#define ___GC_COLLECTIONS___

#include <stddef.h>
#include <stdlib.h>

#define TypedefList(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count;} typeName

TypedefList(void, ListGeneric);
static const ListGeneric ListDefault = 
{
    .V = NULL,
    .Length = 0,
    .Count = 0
};

void ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize);
void ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize);
void ListAddRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize);
void ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize);
void ListInsertRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize);
void ListInitGeneric(ListGeneric *list, const size_t length, const size_t elemSize);
void ListRemoveRangeGeneric(ListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize);
void ListClearGeneric(ListGeneric *list);

#define M_ListGenericParams(list, ...) (ListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define ListAdd(list, value) ListAddGeneric(M_ListGenericParams(list, value))
#define ListAddRange(list, range, rangeCount) ListAddRangeGeneric(M_ListGenericParams(list, range, rangeCount))
#define ListInsert(list, value, index) ListInsertGeneric(M_ListGenericParams(list, value, index))
#define ListInsertRange(list, range, rangeCount, index) ListInsertRangeGeneric(M_ListGenericParams(list, range, rangeCount, index))
#define ListRemoveAt(list, index) ListRemoveRangeGeneric(M_ListGenericParams(list, index, 1))
#define ListRemoveRange(list, index, count) ListRemoveRangeGeneric(M_ListGenericParams(list, index, count))
#define ListInit(list, length) ListInitGeneric(M_ListGenericParams(list, length))
#define ListResize(list, length) ListResizeGeneric(M_ListGenericParams(list, length))
#define ListClear(list) ListClearGeneric((ListGeneric *)list)
#define ListFree(list) free((list)->V);

#define ListValid(list) ((list) != NULL && (list)->V != NULL)

#define TypedefCList(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count; size_t Offset;} typeName

TypedefCList(void, CListGeneric);

#define M_CListGenericParams(list, ...) (CListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define CListGet(list, index) ((list)->V[(index + (list)->Offset) % list->Length])
#define CListSet(list, index, value) ((list)->V[(index + (list)->Offset) % list->Length] = value)

#endif 