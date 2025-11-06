#ifndef __GC_COMMAND_QUEUE__
#define __GC_COMMAND_QUEUE__

#include "GCCList.h"
#include "GCTime.h"
#include <pthread.h>

CListDefine(char, GCInternalCommandQueueCListChar);

// Structure designed to handle command / message queues across threads.
typedef struct CommandQueue
{
    GCInternalCommandQueueCListChar List;
    pthread_mutex_t Mutex;
    pthread_cond_t Signal;
} CommandQueue;

static const CommandQueue CommandQueueDefault = {
    .List = CListDefault, 
    .Mutex = PTHREAD_MUTEX_INITIALIZER,
    .Signal = PTHREAD_COND_INITIALIZER
};

// Pushes a new command to the end of the queue. Passed params will be copied into the queue in the process.
// Automatically locks and unlocks the command queue.
void CommandQueuePushParams(CommandQueue *queue, const uint32_t command, const size_t paramCount, const size_t *paramSizes, const void **params);

// Pushes a new command to the end of the queue. Passed params will be copied into the queue in the process.
// Automatically locks and unlocks the command queue.
void CommandQueuePush(CommandQueue *queue, const uint32_t command, const size_t paramSize, const void *params);

// Used to control the command queue's internal mutex. Should only be used around CommandQueuePop commands, as CommandQueuePush
// commands automatically handle locking and unlocking.
void CommandQueueLock(CommandQueue *queue);
void CommandQueueUnlock(CommandQueue *queue);
void CommandQueueWait(CommandQueue *queue, Timespec *time);

// Pops the oldest command off of the queue. Just obtains the command's code, returning zero on success and -1 if no commands are available.
// To get passed parameters, see CommandQueuePopExtra.
int CommandQueuePop(CommandQueue *queue, uint32_t *commandDest);

// Pops parameters passed into a command. ParamDest must be a buffer capable of containing paramSize bytes.
void CommandQueuePopParam(CommandQueue *queue, const size_t paramSize, void *paramDest);

void CommandQueueFree(CommandQueue *queue);

#endif