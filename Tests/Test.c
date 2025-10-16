#include "Exception.h"
#include <stdio.h>
#include <errno.h>

void ErrorFunction()
{
    Throw(EINVAL, "This is an error");
}

void MyFunction(int recursion)
{
    if(recursion > 5)
        ErrorFunction();

    ExitInit();

    Exception exception;
    MyFunction(recursion + 1);

    printf("Error: %d\n", exception.Type);

    ExitBegin;
        IfExitException
            printf("Exit exception successful\n");

        printf("Exiting %d\n", recursion);
    ExitEnd;
}

int main()
{
    printf("You should see stack exiting messages from 0 to 5, an invalid argument error, a no memory assertion failure, followed by a error about no try statement having been found.\n");
    
    ExitInit();

    Exception exception;
    TryBegin(exception);
        MyFunction(0);
    TryEnd;

    PrintException(exception);

    TryBegin(exception);
        Assert(1 == 0, ENOMEM);
    TryEnd;

    PrintException(exception);
    Assert(1 == 0, EINVAL);

    ExitBegin;
    ExitEnd;
}