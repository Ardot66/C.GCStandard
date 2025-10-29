#include "GCCommandQueue.h"
#include "GCException.h"

void CommandQueuePush(CommandQueue *queue, const uint32_t command, const size_t paramSize, const void *params)
{
    pthread_mutex_lock(&queue->Mutex);
    ExitInit();

    CommandHeader header =
    {
        .Type = command,
        .ParamSize = paramSize
    };

    CListAddRange(&queue->List, &header, sizeof(header));
    CListAddRange(&queue->List, params, paramSize);

    ExitBegin();
    pthread_mutex_unlock(&queue->Mutex);
    ExitEnd();
}

void CommandQueueLock(CommandQueue *queue)
{
    pthread_mutex_lock(&queue->Mutex);
}

void CommandQueueUnlock(CommandQueue *queue)
{
    pthread_mutex_unlock(&queue->Mutex);
}

CommandHeader *CommandQueueGetNext(CommandQueue *queue)
{
    if(queue->List.Count == 0)
        return NULL;
    
    return (CommandHeader *)&CListGet(&queue->List, 0);
}

void CommandQueueRemoveNext(CommandQueue *queue)
{
    CommandHeader *header = (CommandHeader *)&CListGet(&queue->List, 0);
    CListRemoveRange(&queue->List, 0, sizeof(*header) + header->ParamSize);  
}

void CommandQueueFree(CommandQueue *queue)
{
    CListFree(&queue->List);
    pthread_mutex_destroy(&queue->Mutex);
}