#include <stdlib.h>
#include <string.h>
#include "CollectionsPlus.h"
#include "Try.h"

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

int ListAddGeneric(ListGeneric *list, const void *value, const size_t elemSize)
{
    if(list->Count >= list->Length)
        Try(ListResizeGeneric(list, list->Length * 2 + 1, elemSize), -1);

    memcpy((char *)list->V + elemSize * list->Count, value, elemSize);
    list->Count++;
    return 0;
}

int ListInsertGeneric(ListGeneric *list, const void *value, const size_t index, const size_t elemSize)
{
    if(list->Count >= list->Length)
        Try(ListResizeGeneric(list, list->Length * 2 + 1, elemSize), -1);

    memmove(
        (char *)list->V + (index + 1) * elemSize,
        (char *)list->V + index * elemSize,
        (list->Count - index) * elemSize
    );
    memcpy(
        (char *)list->V + index * elemSize,
        value, 
        elemSize
    );

    list->Count++;
    return 0;
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
