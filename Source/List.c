#include <stdlib.h>
#include <string.h>
#include "GCList.h"
#include "GCMemory.h"
#include "GCResult.h"

GCError ListAutoResize (ListGeneric *list, size_t newElementsCount, size_t elemSize)
{
    if(list->Count + newElementsCount > list->Length)
    {
        size_t newLength = list->Length == 0 ? 8 : list->Length;
        while(newLength < list->Count + newElementsCount) 
            newLength *= 2;
        return ListResizeGeneric(list, newLength, elemSize);
    }

    ErrorLabel; 
    return Error;
}

GCError ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize)
{
    if(newLength < list->Count)
        Throw(EINVAL, "List cannot be resized to be smaller than its count");

    ListGeneric *temp = GCRealloc(list->V, newLength * elemSize);
    if(temp == NULL)
        GotoError;

    list->V = temp;
    list->Length = newLength;

    ErrorLabel; 
    return Error;
}

GCError ListRemoveRangeGeneric(ListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize)
{
    if(startIndex + count > list->Count)
        Throw(EINVAL, "Cannot remove elements out of the bounds of a list");
    memmove(
        (char *)list->V + startIndex * elemSize, 
        (char *)list->V + (startIndex + count) * elemSize,
        (list->Count - startIndex - count) * elemSize
    );

    list->Count -= count;
    
    ErrorLabel; 
    return Error;
}

GCError ListInsertRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize)
{
    if(index > list->Count)
        Throw(EINVAL, "Cannot insert elements past the end of a list");
    Try(ListAutoResize(list, rangeCount, elemSize));

    if(index < list->Count)
        memmove(
            (char *)list->V + (index + rangeCount) * elemSize,
            (char *)list->V + index * elemSize,
            (list->Count - index) * elemSize
        );

    if(range)
        memcpy(
            (char *)list->V + index * elemSize,
            range, 
            elemSize * rangeCount
        );

    list->Count += rangeCount;

    ErrorLabel; 
    return Error;
}

GCError ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, value, 1, index, elemSize);
}

GCError ListAddRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, range, rangeCount, list->Count, elemSize);
}

GCError ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, value, 1, list->Count, elemSize);
}

void ListClearGeneric(ListGeneric *list)
{
    list->Count = 0;
}
