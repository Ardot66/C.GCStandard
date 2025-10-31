#include "GCCommandQueue.h"
#include "GCTime.h"
#include "GCException.h"
#include "GCTestingUtilities.h"

enum CommandType
{
    COMMAND_TYPE_SUCCESS = 13123
};

const int CommandParams[] = 
{
    51243,
    232,
    -1515755
};

typedef struct ThreadInfo
{
    CommandQueue Queue;
} ThreadInfo;

void *CommandQueueThread(void *params)
{
    ThreadInfo *threadInfo = params;

    Exception *exception;
    TryBegin(exception);
    CommandQueuePush(&threadInfo->Queue, COMMAND_TYPE_SUCCESS, sizeof(CommandParams), CommandParams);
    TryEnd;

    TEST(exception, ==, NULL, ExceptionPrint(exception); exit(exception->Type););
    ExceptionFree(exception);

    pthread_exit(NULL);
    return NULL;
}

void TestCommandQueue()
{
    Exception *exception;
    TryBegin(exception);

    ThreadInfo info =
    {
        .Queue = CommandQueueDefault
    };

    int successful = 0;

    pthread_t thread;
    pthread_create(&thread, NULL, CommandQueueThread, &info);

    Timespec startTime = TimespecGet(TIME_UTC);
    while(1)
    {
        Timespec now = TimespecGet(TIME_UTC);
        if(TimespecToSecs(TimespecDiff(now, startTime)) > 1.0f)
            break;

        CommandQueueLock(&info.Queue);

        uint32_t command;
        if(!CommandQueuePop(&info.Queue, &command))
        {
            if(command == COMMAND_TYPE_SUCCESS)
            {
                successful = 1;
                int paramsDest[sizeof(CommandParams) / sizeof(*CommandParams)];
                CommandQueuePopParam(&info.Queue, sizeof(CommandParams), paramsDest);
                for(size_t x = 0; x < sizeof(CommandParams) / sizeof(*CommandParams); x++)
                    TEST(paramsDest[x], ==, CommandParams[x]);
            }
            else 
                break;
        }

        CommandQueueUnlock(&info.Queue);
    }

    void *res;
    pthread_join(thread, &res);

    TEST(successful, ==, 1);
    TryEnd;

    TEST(exception, ==, NULL, ExceptionPrint(exception););
    ExceptionFree(exception);

    PrintTestStatus(NULL);
}