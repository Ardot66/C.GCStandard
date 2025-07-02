#ifndef ___COLLECTIONS_PLUS___
#define ___COLLECTIONS_PLUS___

#include <stddef.h>

#define TypedefList(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count;} typeName

TypedefList(void, ListGeneric);

int ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize);
int ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize);
int ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize);
int ListInitGeneric(ListGeneric *list, const size_t length, const size_t elemSize);
void ListRemoveRangeGeneric(ListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize);
void ListClearGeneric(ListGeneric *list, const size_t elemSize);

#define M_ListGenericParams(list, ...) (ListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define ListAdd(list, value) ListAddGeneric(M_ListGenericParams(list, value))
#define ListInsert(list, value, index) ListInsertGeneric(M_ListGenericParams(list, value, index))
#define ListRemoveAt(list, index) ListRemoveRangeGeneric(M_ListGenericParams(list, index, 1))
#define ListRemoveRange(list, index, count) ListRemoveRangeGeneric(M_ListGenericParams(list, index, count))
#define ListInit(list, length) ListInitGeneric(M_ListGenericParams(list, length))
#define ListResize(list, length) ListResizeGeneric(M_ListGenericParams(list, length))
#define ListClear(list) ListClearGeneric(M_ListGenericParams(list))

#define ListValid(list) ((list) != NULL && (list)->V != NULL)

#define TypedefCList(type, typeName) typedef struct typeName {type *V; size_t Length; size_t Count; size_t Offset;} typeName

TypedefCList(void, CListGeneric);

#define M_CListGenericParams(list, ...) (CListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define CListGet(list, index) ((list)->V[(index + (list)->Offset) % list->Length])
#define CListSet(list, index, value) ((list)->V[(index + (list)->Offset) % list->Length] = value)

#endif 