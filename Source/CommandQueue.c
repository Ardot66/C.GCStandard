#include "GCCommandQueue.h"
#include "GCResult.h"
#include <errno.h>

#include "GCAssert.h"
#include <stdio.h>

typedef struct CommandHeader
{
    uint32_t Type;
} CommandHeader;

void CommandQueueLock(CommandQueue *queue)
{
    pthread_mutex_lock(&queue->Mutex);
}

void CommandQueueUnlock(CommandQueue *queue)
{
    pthread_cond_broadcast(&queue->Signal);
    pthread_mutex_unlock(&queue->Mutex);
}

void CommandQueueWait(CommandQueue *queue, Timespec *time)
{
    if(queue->List.Count > 0)
        return;
        
    pthread_mutex_lock(&queue->Mutex);
    if(time)
        pthread_cond_timedwait(&queue->Signal, &queue->Mutex, time);
    else
        pthread_cond_wait(&queue->Signal, &queue->Mutex);
    pthread_mutex_unlock(&queue->Mutex);
}

GCError CommandQueuePushParams(CommandQueue *queue, const uint32_t command, const size_t paramCount, const size_t *paramSizes, const void **params)
{
    CommandQueueLock(queue);

    CommandHeader header =
    {
        .Type = command,
    };

    const size_t startIndex = queue->List.Count;
    Try(CListAddRange(&queue->List, &header, sizeof(header)));
    for(size_t x = 0; x < paramCount; x++)
        Try(CListAddRange(&queue->List, params[x], paramSizes[x]));

    ErrorLabel;
    // This handles the possibility that the command queue is corrupted if any of the CListAdd calls fail.
    IfError
        CListRemoveRange(&queue->List, startIndex, queue->List.Count - startIndex);

    CommandQueueUnlock(queue);
    return Error;
}

GCError CommandQueuePush(CommandQueue *queue, const uint32_t command, const size_t paramSize, const void *param)
{
    return CommandQueuePushParams(queue, command, 1, &paramSize, &param);
}

// Pop commands cannot fail in dangerous ways, so it is fine to have them split up like so for convenience.
GCResult CommandQueuePop(CommandQueue *queue, uint32_t *commandDest)
{
    if(queue->List.Count == 0)
        return GC_RESULT_FAILURE;
    if(queue->List.Count < sizeof *commandDest)
        Throw(EINVAL, "Not enough data on command queue to pop command code");

    for(size_t x = 0; x < sizeof(uint32_t); x++)
        ((char *)commandDest)[x] = CListGet(&queue->List, x);
    Try(CListRemoveRange(&queue->List, 0, sizeof(*commandDest)));

    ErrorLabel;
    return (GCResult)Error;
}

GCError CommandQueuePopParam(CommandQueue *queue, const size_t paramSize, void *paramDest)
{
    if(queue->List.Count < paramSize)
        Throw(EINVAL, "Not enough data on command queue to pop param");

    for(size_t x = 0; x < paramSize; x++)
        ((char *)paramDest)[x] = CListGet(&queue->List, x);
    Try(CListRemoveRange(&queue->List, 0, paramSize));

    ErrorLabel;
    return Error;
}

void CommandQueueFree(CommandQueue *queue)
{
    CListFree(&queue->List);
    pthread_mutex_destroy(&queue->Mutex);
}