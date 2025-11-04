#include <stdlib.h>
#include <sys/types.h>
#include <string.h>

#include "GCCList.h"
#include "GCException.h"
#include "GCMemory.h"

static inline void *CListIndex(CListGeneric *list, const size_t index, const size_t elemSize)
{
    return (char *)list->V + ((index + list->Offset) % list->Length) * elemSize;
}

static inline void CListChangeOffset(CListGeneric *list, const ssize_t amount)
{
    ssize_t offset = ((ssize_t)list->Offset + amount) % list->Length;
    if(offset < 0)
        offset += list->Length;

    list->Offset = offset;
}

static inline void CListMoveGeneric(CListGeneric *list, const size_t startIndex, const size_t endIndex, const size_t count, const size_t elemSize)
{
    int direction = startIndex < endIndex;
    ssize_t directionMultiplier = !direction - direction;

    // Offset added when copying in the reverse direction.
    ssize_t add = direction * (count - 1);

    for(size_t x = 0; x < count; x++)
        memcpy(CListIndex(list, endIndex + add + x * directionMultiplier, elemSize), CListIndex(list, startIndex + add + x * directionMultiplier, elemSize), elemSize);
}

void CListResizeGeneric(CListGeneric *list, const size_t newLength, const size_t elemSize)
{
    ThrowIf(newLength < list->Count);

    CListGeneric *temp = GCRealloc(list->V, newLength * elemSize);

    list->V = temp;
    size_t preloopCount = (list->Length - list->Offset) * (list->Offset + list->Count > list->Length);

    memmove(
        (char *)list->V + (newLength - preloopCount) * elemSize,
        (char *)list->V + (list->Length - preloopCount) * elemSize,
        preloopCount * elemSize
    );

    list->Length = newLength;
}

void CListInsertRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize)
{
    ThrowIf(index > list->Count);

    if(list->Count + rangeCount > list->Length)
        CListResizeGeneric(list, list->Length == 0 ? 16 : list->Length * 2 + 1, elemSize);

    // Testing if displaced elements should be pushed forwards or backwards, depending on what would be more efficient.
    if(index + (rangeCount >> 1) < list->Count >> 1)
    {
        CListChangeOffset(list, -(ssize_t)rangeCount);
        CListMoveGeneric(list, rangeCount, 0, index, elemSize);
    }
    else
        CListMoveGeneric(list, index, index + rangeCount, list->Count - index, elemSize);

    if(range)
        for(size_t x = 0; x < rangeCount; x++)
            memcpy(CListIndex(list, index + x, elemSize), (char *)range + x * elemSize, elemSize);
    list->Count += rangeCount;
}

void CListInsertGeneric(CListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    CListInsertRangeGeneric(list, value, 1, index, elemSize);
}

void CListAddRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize)
{
    CListInsertRangeGeneric(list, range, rangeCount, list->Count, elemSize);
}

void CListAddGeneric(CListGeneric *list, const void *value, const size_t elemSize)
{
    CListInsertRangeGeneric(list, value, 1, list->Count, elemSize);
}

void CListRemoveRangeGeneric(CListGeneric *list, const size_t index, const size_t rangeCount, const size_t elemSize)
{
    ThrowIf(index + rangeCount > list->Count);

    // Testing if displaced elements should be pushed forwards or backwards, depending on what would be more efficient.
    if(index < list->Count >> 1)
    {
        CListMoveGeneric(list, 0, rangeCount, index, elemSize);
        CListChangeOffset(list, rangeCount);
    }
    else
        CListMoveGeneric(list, index + rangeCount, index, list->Count - index - rangeCount, elemSize);

    list->Count -= rangeCount;
}

void CListRemoveAtGeneric(CListGeneric *list, const size_t index, const size_t elemSize)
{
    CListRemoveRangeGeneric(list, index, 1, elemSize);
}

void CListClearGeneric(CListGeneric *list)
{
    list->Count = 0;
    list->Offset = 0;
}
