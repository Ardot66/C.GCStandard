#include <stdlib.h>
#include <string.h>
#include "CollectionsPlus.h"
#include "Try.h"

int ListAutoResize (ListGeneric *list, size_t newElementsCount, size_t elemSize)
{
    if(list->Count + newElementsCount > list->Length)
    {
        size_t newLength = list->Length == 0 ? 8 : list->Length;
        while(newLength < list->Count + newElementsCount) 
            newLength *= 2;
        Try(ListResizeGeneric(list, newLength, elemSize), -1);
    }

    return 0;
}

int ListResizeGeneric(ListGeneric *list, const size_t newLength, const size_t elemSize)
{
    ListGeneric *temp;
    Try((temp = realloc(list->V, newLength * elemSize)) == NULL, -1);

    list->V = temp;
    list->Length = newLength;

    return 0;
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

int ListInsertRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t index, const size_t elemSize)
{
    Assert(index <= list->Count, EINVAL, -1);
    Try(ListAutoResize(list, rangeCount, elemSize), -1);

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
    return 0;
}

int ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, value, 1, index, elemSize);
}

int ListAddRangeGeneric(ListGeneric *list, const void *range, const size_t rangeCount, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, range, rangeCount, list->Count, elemSize);
}

int ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize)
{
    return ListInsertRangeGeneric(list, value, 1, list->Count, elemSize);
}

int ListInitGeneric(ListGeneric *list, const size_t length, const size_t elemSize)
{
    Try((list->V = malloc(length * elemSize)) == NULL, -1);

    list->Length = length;
    list->Count = 0;
    return 0;
}

void ListClearGeneric(ListGeneric *list, const size_t elemSize)
{
    list->Count = 0;
}
