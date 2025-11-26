#include "GCCommandQueue.h"
#include "GCTime.h"
#include "GCResult.h"
#include "GCTestingUtilities.h"
#include "Tests.h"
#include <stdlib.h>

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
    Try(CommandQueuePush(&threadInfo->Queue, COMMAND_TYPE_SUCCESS, sizeof(CommandParams), CommandParams));

    ErrorLabel;
    IfError
    {
        ErrorInfoPrint(ErrorInfoGetCurrent());
        TEST(1, ==, 0);
    }

    return NULL;
}

void TestCommandQueue()
{
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
        switch (CommandQueuePop(&info.Queue, &command))
        {
            ErrorCase;
            case GC_RESULT_SUCCESS:
            {
                if(command == COMMAND_TYPE_SUCCESS)
                {
                    successful = 1;
                    int paramsDest[sizeof(CommandParams) / sizeof(*CommandParams)];
                    Try(CommandQueuePopParam(&info.Queue, sizeof(CommandParams), paramsDest));
                    for(size_t x = 0; x < sizeof(CommandParams) / sizeof(*CommandParams); x++)
                        TEST(paramsDest[x], ==, CommandParams[x]);
                }
                else 
                    goto BreakWhile;
            }
            default: break;
        }

        CommandQueueUnlock(&info.Queue);
    }

    BreakWhile:

    void *res;
    pthread_join(thread, &res);

    TEST(successful, ==, 1);

    ErrorLabel;
    IfError
    {
        ErrorInfoPrint(ErrorInfoGetCurrent());
        TEST(1, ==, 0);
    }

    PrintTestStatus(NULL);
    MetaTest(TestsPassed, TestsRun);
}