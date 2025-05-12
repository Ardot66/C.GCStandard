#ifndef ___COLLECTIONS_PLUS___
#define ___COLLECTIONS_PLUS___

#include <stdint.h>

#define List(type, typeName) struct List ## typeName {type *V; size_t Length; size_t Count;}

typedef List(void, void) ListGeneric;

int ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize);
int ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize);
int ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize);
void ListRemoveAtGeneric(ListGeneric *list, const size_t index, const size_t elemSize);
int ListInitGeneric(ListGeneric *list, const size_t length, const size_t elemSize);
void ListClearGeneric(ListGeneric *list, const size_t elemSize);

#define M_ListGenericParams(list, ...) (ListGeneric *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define ListAdd(list, value) ListAddGeneric(M_ListGenericParams(list, value))
#define ListInsert(list, value, index) ListInsertGeneric(M_ListGenericParams(list, value, index))
#define ListRemoveAt(list, index) ListRemoveAtGeneric(M_ListGenericParams(list, index))
#define ListInit(list, length) ListInitGeneric(M_ListGenericParams(list, length))
#define ListResize(list, length) ListResizeGeneric(M_ListGenericParams(list, length))
#define ListClear(list) ListClearGeneric(M_ListGenericParams(list))

#define CList(type) CList_ ## type ## _ ## CollectionsUID
#define CListDeclare(type) typedef struct CList(type) {type *V; size_t Length; size_t Count; size_t Offset;} CList(type);

CListDeclare(void);


#define M_CListGenericParams(list, ...) (CListVoid *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define CListGet(list, index) ((list)->V[(index + (list)->Offset) % list->Length])
#define CListSet(list, index, value) ((list)->V[(index + (list)->Offset) % list->Length] = value)

#endif