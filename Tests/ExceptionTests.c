#include "GCException.h"
#include <stdio.h>
#include <errno.h>
#include "GCTestingUtilities.h"

const char *ErrorMessage = "This is an error";
size_t ExceptionRecursions = 5, ExceptionExitsCounted = 0;

void ExceptionRecursionTest(size_t recursion)
{
    if(recursion > ExceptionRecursions)
        Throw(EINVAL, ErrorMessage);

    ExitInit();

    ExceptionRecursionTest(recursion + 1);

    ExitBegin;
        int detected = 0;
        IfExitException
            detected = 1;
        TEST(detected, ==, 1);

        ExceptionExitsCounted++;
    ExitEnd;
}

void TestException()
{
    ExitInit();

    Exception exception;
    TryBegin(exception);
        ExceptionRecursionTest(1);
    TryEnd;

    TEST(ExceptionExitsCounted, ==, ExceptionRecursions);
    TEST(exception.Type, ==, EINVAL);
    TEST(exception.Message, ==, ErrorMessage);

    printf("Testing exception printing, should say: 'Invalid argument: This is an error at Tests/Test.c - ExceptionRecursionTest() - Line 12'\n");
    PrintException(exception);

    TryBegin(exception);
        Assert(1 == 0, ENOMEM);
    TryEnd;

    TEST(exception.Type, ==, ENOMEM)

    ExitBegin;
    ExitEnd;

    PrintTestStatus(NULL);
}