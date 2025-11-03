#ifndef __GC_LIST__
#define __GC_LIST__

#include <stddef.h>
#include "GCMemory.h"

// Lists provide a generic array that will automatically handle resizing and other relatively complex
// operations that are useful for manipulating data.
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

#endif