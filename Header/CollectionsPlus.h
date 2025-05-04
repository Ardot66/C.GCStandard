#ifndef ___COLLECTIONS_PLUS___
#define ___COLLECTIONS_PLUS___

#include <stdint.h>

#define M_ListType(type) List_ ## type
#define ListDeclare(type) typedef struct M_ListType(type) {type *V; size_t Length; size_t Count;} M_ListType(type);
#define List(type) M_ListType(type)

ListDeclare(void);
int ListResizeGeneric(List(void) *list, const size_t newLength, const size_t elemSize);
int ListAddGeneric(List(void) *list, const void *value, const size_t elemSize);
int ListInsertGeneric(List(void) *list, const void *value, const size_t index, const size_t elemSize);
void ListRemoveAtGeneric(List(void) *list, const size_t index, const size_t elemSize);
int ListInitGeneric(List(void) *list, const size_t length, const size_t elemSize);
int ListClearGeneric(List(void) *list, const size_t elemSize);

#define M_ListGenericParams(list, ...) (List(void) *)list, ## __VA_ARGS__, sizeof(*(list)->V)
#define ListAdd(list, value) ListAddGeneric(M_ListGenericParams(list, value))
#define ListInsert(list, value, index) ListInsertGeneric(M_ListGenericParams(list, value, index))
#define ListRemoveAt(list, index) ListRemoveAtGeneric(M_ListGenericParams(list, index))
#define ListInit(list, length) ListInitGeneric(M_ListGenericParams(list, length))
#define ListResize(list, length) ListResizeGeneric(M_ListGenericParams(list, length))
#define ListClear(list) ListClearGeneric(M_ListGenericParams(list))


#endif