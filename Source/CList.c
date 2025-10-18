#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include "CollectionsPlus.h"
#include "Try.h"

// static inline void *CListIndex(CListGeneric *list, const size_t index, const size_t elemSize)
// {
//     return (char *)list->V + ((index + list->Offset) % list->Length);
// }

// static inline void CListChangeOffset(CListGeneric *list, const ssize_t amount)
// {
//     ssize_t offset = ((ssize_t)list->Offset + amount) % list->Length;
//     if(offset < 0)
//         offset += list->Length;

//     list->Offset = offset;
// }

// void CListMoveGeneric(CListGeneric *list, const size_t startIndex, const size_t endIndex, const size_t amount, const size_t elemSize)
// {
//     int direction = startIndex < endIndex;
//     ssize_t mult = !direction - direction;
//     ssize_t add = direction * (amount - 1);

//     for(ssize_t x = 0; x < amount; x++)
//         memcpy(CListIndex(list, endIndex + add + x * mult, elemSize), CListIndex(list, startIndex + add + x * mult, elemSize), elemSize);
// }

// int CListResizeGeneric(CListGeneric *list, const size_t newLength, const size_t elemSize)
// {
//     CListGeneric *temp = realloc(list->V, newLength * elemSize);

//     if(temp == NULL)
//         return errno;

//     list->V = temp;
//     size_t preloopCount = (list->Length - list->Offset);// * (list->Offset + list->Count > list->Length);

//     memmove(
//         (char *)list->V + (newLength - preloopCount) * elemSize,
//         (char *)list->V + (list->Length - preloopCount) * elemSize,
//         preloopCount * elemSize
//     );

//     list->Length = newLength;
//     return 0;
// }

// int CListAddGeneric(CListGeneric *list, const void *value, const size_t elemSize)
// {
//     if(list->Count >= list->Length)
//     {
//         int err;
//         if(err = CListResizeGeneric(list, list->Length * 2 + 1, elemSize)) return err;
//     }

//     memcpy(CListIndex(list, list->Count, elemSize), value, elemSize);
//     return 0;
// }

// int CListInsertGeneric(CListGeneric *list, const void *value, const size_t index, const size_t elemSize)
// {
//     if(list->Count >= list->Length)
//     {
//         int err;
//         if(err = CListResizeGeneric(list, list->Length * 2 + 1, elemSize)) return err;
//     }

//     if(index < list->Count >> 1)
//     {
//         CListChangeOffset(list, -1);
//         CListMoveGeneric(list, 1, 0, index, elemSize);
//     }
//     else
//         CListMoveGeneric(list, index, index + 1, list->Count - index, elemSize);

//     memcpy(CListIndex(list, index, elemSize), value, elemSize);
//     list->Count++;
//     return 0;
// }

// void CListRemoveAtGeneric(CListGeneric *list, const size_t index, const size_t elemSize)
// {
//     if(index < list->Count >> 1)
//     {
//         CListMoveGeneric(list, 0, 1, index, elemSize);
//         CListChangeOffset(list, 1);
//     }
//     else
//         CListMoveGeneric(list, index + 1, index, list->Count - index - 1, elemSize);
// }

// int CListInitGeneric(CListGeneric *list, const size_t length, const size_t elemSize)
// {
//     list->V = malloc(length * elemSize);

//     if(list->V == NULL)
//         return errno;

//     list->Length = length;
//     list->Count = 0;
//     list->Offset = 0;
//     return 0;
// }

