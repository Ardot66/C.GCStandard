#include <stdlib.h>
#include <string.h>
#include "CollectionsPlus.h"

ListDeclare(int);

int ListResizeGeneric(List(void) *list, const size_t newLength, const size_t elemSize)
{
    List(void) *temp = realloc(list->V, newLength * elemSize);

    if(temp == NULL)
        return errno;

    list->V = temp;
    list->Length = newLength;

    return 0;
}

void ListRemoveAtGeneric(List(void) *list, const size_t index, const size_t elemSize)
{
    memmove(
        (char *)list->V + index * elemSize, 
        (char *)list->V + (index + 1) * elemSize,
        (list->Count - index - 1) * elemSize
    );

    list->Count--;
}

int ListAddGeneric(List(void) *list, const void *value, const size_t elemSize)
{
    if(list->Count >= list->Length)
    {   
        int err;
        if(err = ListResizeGeneric(list, list->Length * 2 + 1, elemSize)) return err;
    }

    memcpy((char *)list->V + elemSize * list->Count, value, elemSize);
    list->Count++;
}

int ListInsertGeneric(List(void) *list, const void *value, const size_t index, const size_t elemSize)
{
    if(list->Count >= list->Length)
    {   
        int err;
        if(err = ListResizeGeneric(list, list->Length * 2 + 1, elemSize)) return err;
    }

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
}

int ListInitGeneric(List(void) *list, const size_t length, const size_t elemSize)
{
    list->V = malloc(length * elemSize);

    if(list->V == NULL)
        return errno;

    list->Length = length;
    list->Count = 0;
}

int ListClearGeneric(List(void) *list, const size_t elemSize)
{
    list->Count = 0;
}
