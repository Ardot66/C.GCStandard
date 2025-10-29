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

        CommandHeader *command = CommandQueueGetNext(&info.Queue);
        if(command != NULL)
        {
            if(command->Type == COMMAND_TYPE_SUCCESS)
            {
                successful = 1;
                TEST(command->ParamSize, ==, sizeof(CommandParams));
                int *params = (int *)command->Params;
                for(size_t x = 0; x < sizeof(CommandParams) / sizeof(*CommandParams); x++)
                    TEST(params[x], ==, CommandParams[x]);
                CommandQueueRemoveNext(&info.Queue);
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