#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>

#include "GCCList.h"
#include "GCResult.h"
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

GCError CListResizeGeneric(CListGeneric *list, const size_t newLength, const size_t elemSize)
{
    if(newLength < list->Count)
        Throw(EINVAL, "List cannot be resized to be smaller than its count");

    CListGeneric *temp = GCRealloc(list->V, newLength * elemSize);
    if(temp == NULL)
        GotoError;

    list->V = temp;
    size_t preloopCount = (list->Length - list->Offset) * (list->Offset + list->Count > list->Length);

    memmove(
        (char *)list->V + (newLength - preloopCount) * elemSize,
        (char *)list->V + (list->Length - preloopCount) * elemSize,
        preloopCount * elemSize
    );
    if(preloopCount != 0)
        list->Offset = newLength - preloopCount;

    list->Length = newLength;

    ErrorLabel;
    return Error;
}

GCError CListInsertRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize)
{
    if(index > list->Count)
        Throw(EINVAL, "Cannot insert elements past the end of a list");

    if(list->Count + rangeCount > list->Length)
        Try(CListResizeGeneric(list, list->Length == 0 ? 16 : list->Length * 2, elemSize));

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

    ErrorLabel; 
    return Error;
}

GCError CListInsertGeneric(CListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    return CListInsertRangeGeneric(list, value, 1, index, elemSize);
}

GCError CListAddRangeGeneric(CListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize)
{
    return CListInsertRangeGeneric(list, range, rangeCount, list->Count, elemSize);
}

GCError CListAddGeneric(CListGeneric *list, const void *value, const size_t elemSize)
{
    return CListInsertRangeGeneric(list, value, 1, list->Count, elemSize);
}

GCError CListRemoveRangeGeneric(CListGeneric *list, const size_t index, const size_t rangeCount, const size_t elemSize)
{
    if(index + rangeCount > list->Count)
        Throw(EINVAL, "Cannot remove elements out of the bounds of a list");

    // Testing if displaced elements should be pushed forwards or backwards, depending on what would be more efficient.
    if(index < list->Count >> 1)
    {
        CListMoveGeneric(list, 0, rangeCount, index, elemSize);
        CListChangeOffset(list, rangeCount);
    }
    else
        CListMoveGeneric(list, index + rangeCount, index, list->Count - index - rangeCount, elemSize);

    list->Count -= rangeCount;

    ErrorLabel; 
    return Error;
}

GCError CListRemoveAtGeneric(CListGeneric *list, const size_t index, const size_t elemSize)
{
    return CListRemoveRangeGeneric(list, index, 1, elemSize);
}

void CListClearGeneric(CListGeneric *list)
{
    list->Count = 0;
    list->Offset = 0;
}
