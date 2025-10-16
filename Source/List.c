#include <stdlib.h>
#include <string.h>
#include "CollectionsPlus.h"
#include "Exception.h"

void ListAutoResize (ListGeneric *list, size_t newElementsCount, size_t elemSize)
{
    if(list->Count + newElementsCount > list->Length)
    {
        size_t newLength = list->Length == 0 ? 8 : list->Length;
        while(newLength < list->Count + newElementsCount) 
            newLength *= 2;
        ListResizeGeneric(list, newLength, elemSize);
    }
}

void ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize)
{
    ListGeneric *temp = realloc(list->V, newLength * elemSize);
    Assert(temp, errno);

    list->V = temp;
    list->Length = newLength;
}

void ListRemoveRangeGeneric(ListGeneric *list, const size_t startIndex, const size_t count, const size_t elemSize)
{
    memmove(
        (char *)list->V + startIndex * elemSize, 
        (char *)list->V + (startIndex + count) * elemSize,
        (list->Count - startIndex - count) * elemSize
    );

    list->Count -= count;
}

void ListInsertRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize)
{
    Assert(index <= list->Count, EINVAL);
    ListAutoResize(list, rangeCount, elemSize);

    if(index < list->Count)
        memmove(
            (char *)list->V + (index + rangeCount) * elemSize,
            (char *)list->V + index * elemSize,
            (list->Count - index) * elemSize
        );

    memcpy(
        (char *)list->V + index * elemSize,
        range, 
        elemSize * rangeCount
    );

    list->Count += rangeCount;
}

void ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    ListInsertRangeGeneric(list, value, 1, index, elemSize);
}

void ListAddRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize)
{
    ListInsertRangeGeneric(list, range, rangeCount, list->Count, elemSize);
}

void ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize)
{
    ListInsertRangeGeneric(list, value, 1, list->Count, elemSize);
}

void ListInitGeneric(ListGeneric *list, const size_t length, const size_t elemSize)
{
    list->V = malloc(length * elemSize);
    Assert(list->V, errno);

    list->Length = length;
    list->Count = 0;
}

void ListClearGeneric(ListGeneric *list, const size_t elemSize)
{
    list->Count = 0;
}
