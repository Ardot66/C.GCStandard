#include "GCCommandQueue.h"
#include "GCException.h"
#include <errno.h>

#include "GCAssert.h"
#include <stdio.h>

typedef struct CommandHeader
{
    uint32_t Type;
} CommandHeader;

void PrintCommandQueue(CommandQueue *queue)
{
    for(size_t x = 0; x < queue->List.Count; x++)
        printf("%02X ", (unsigned char)CListGet(&queue->List, x));
    printf("\n");
}

void CommandQueuePushParams(CommandQueue *queue, const uint32_t command, const size_t paramCount, const size_t *paramSizes, const void **params)
{
    pthread_mutex_lock(&queue->Mutex);
    ExitInit();

    CommandHeader header =
    {
        .Type = command,
    };

    const size_t startIndex = queue->List.Count;
    CListAddRange(&queue->List, &header, sizeof(header));
    for(size_t x = 0; x < paramCount; x++)
        CListAddRange(&queue->List, params[x], paramSizes[x]);
    // printf("Pushed: "); PrintCommandQueue(queue);

    ExitBegin();
        // This handles the possibility that the command queue is corrupted if any of the CListAdd calls fail.
        IfExitException
            CListRemoveRange(&queue->List, startIndex, queue->List.Count - startIndex);
        pthread_mutex_unlock(&queue->Mutex);
    ExitEnd();
}

void CommandQueuePush(CommandQueue *queue, const uint32_t command, const size_t paramSize, const void *param)
{
    CommandQueuePushParams(queue, command, 1, &paramSize, &param);
}

void CommandQueueLock(CommandQueue *queue)
{
    pthread_mutex_lock(&queue->Mutex);
}

void CommandQueueUnlock(CommandQueue *queue)
{
    pthread_mutex_unlock(&queue->Mutex);
}

// Pop commands cannot fail in dangerous ways, so it is fine to have them split up like so for convenience.
int CommandQueuePop(CommandQueue *queue, uint32_t *commandDest)
{
    if(queue->List.Count == 0)
        return -1;
    ThrowIf(queue->List.Count < sizeof *commandDest, "Not enough data on command queue to pop command code");

    for(size_t x = 0; x < sizeof(uint32_t); x++)
        ((char *)commandDest)[x] = CListGet(&queue->List, x);
    // printf("Popped: (%u)", *commandDest); PrintCommandQueue(queue);
    CListRemoveRange(&queue->List, 0, sizeof(*commandDest));

    return 0;
}

void CommandQueuePopParam(CommandQueue *queue, const size_t paramSize, void *paramDest)
{
    ThrowIf(queue->List.Count < paramSize, "Not enough data on command queue to pop param");

    for(size_t x = 0; x < paramSize; x++)
        ((char *)paramDest)[x] = CListGet(&queue->List, x);
    CListRemoveRange(&queue->List, 0, paramSize);
}

void CommandQueueFree(CommandQueue *queue)
{
    CListFree(&queue->List);
    pthread_mutex_destroy(&queue->Mutex);
}