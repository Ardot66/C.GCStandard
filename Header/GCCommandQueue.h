#ifndef __GC_COMMAND_QUEUE__
#define __GC_COMMAND_QUEUE__

#include "GCCollections.h"
#include <pthread.h>

CListDefine(char, GCInternalCommandQueueCListChar);

// Structure designed to handle command / message queues across threads.
typedef struct CommandQueue
{
    GCInternalCommandQueueCListChar List;
    pthread_mutex_t Mutex;
} CommandQueue;

static const CommandQueue CommandQueueDefault = {.List = ListDefault, .Mutex = PTHREAD_MUTEX_INITIALIZER};

typedef struct CommandHeader
{
    uint32_t Type;
    uint32_t ParamSize;
    char Params[];
} CommandHeader;

// Pushes a new command to the end of the queue. Passed params will be copied into the queue in the process.
void CommandQueuePush(CommandQueue *queue, const uint32_t command, const size_t paramSize, const void *params);
void CommandQueueLock(CommandQueue *queue);
void CommandQueueUnlock(CommandQueue *queue);

// Gets the next command in the queue. Meant to be paired with CommandQueueClearNext.
// Requires that the command queue be manually locked before use. The returned pointer is part of the queue,
// so it will no longer be valid after the queue is unlocked or CommandQueueClearNext is called.
CommandHeader *CommandQueueGetNext(CommandQueue *queue);

// Removes the next command in the queue, which is always last command obtained with CommandQueueGetNext.
// Requires that the command queue be manually locked before use.
void CommandQueueRemoveNext(CommandQueue *queue);
void CommandQueueFree(CommandQueue *queue);

#endif