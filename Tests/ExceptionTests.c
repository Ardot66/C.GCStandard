#include "GCException.h"
#include <stdio.h>
#include <errno.h>
#include "GCTestingUtilities.h"
#include "Tests.h"

const char *ErrorMessage = "This is an error";
size_t ExceptionRecursions = 5, ExceptionExitsCounted = 0;
size_t throw = 0;

void ExceptionRecursionTest(size_t recursion)
{
    ExitInit();
    
    if(recursion < ExceptionRecursions)
        ExceptionRecursionTest(recursion + 1);

    if(throw)
        Throw(ErrorMessage, "more erroring");

    ExitBegin();
        int detected = 0;
        IfExitException
            detected = 1;

        if(throw == 1)
            TEST(detected, ==, 1);

        ExceptionExitsCounted++;
    ExitEnd();

    throw = 1;
}

// TODO: Set up tests for exception jumps.
void TestException()
{
    ExitInit();

    Exception *exception;
    TryBegin(exception);
        ExceptionRecursionTest(1);
    TryEnd;

    TEST(ExceptionExitsCounted, ==, ExceptionRecursions);
    TEST(exception->MessageCount, ==, 2);
    TEST(exception->Messages[0], ==, ErrorMessage);

    printf("Testing exception printing, should say: 'This is an error, more erroring, at ExceptionRecursionTest() in Tests/Test.c line [x]', followed by a backtrace\n");
    ExceptionPrint(exception);

    Exception *exception2;
    TryBegin(exception2);
        ThrowException(exception);
    TryEnd;

    TEST(exception2, ==, exception);
    ExceptionFree(exception);

    TryBegin(exception);
        ThrowIf(1 == 1);
    TryEnd;

    ExceptionFree(exception);

    ExitBegin();
    ExitEnd();

    PrintTestStatus(NULL);
    MetaTest(TestsPassed, TestsRun);
}