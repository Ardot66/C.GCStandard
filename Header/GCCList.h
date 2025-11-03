#ifndef __GC_CLIST__
#define __GC_CLIST__

#include <stddef.h>
#include "GCMemory.h"

// CLists provide a generic array that will automatically handle resizing and other relatively complex
// operations that are useful for manipulating data. CLists have a signifcant advantage over lists, in
// that they perform significantly more efficiently at adding and removing items from the front of the
// list, rather than just the end. This makes them ideal for queues and other 'circular' structures.
//
// NOTE: CLists should not be accessed through their V parameter, instead use CListGet and CListSet.
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

#endif