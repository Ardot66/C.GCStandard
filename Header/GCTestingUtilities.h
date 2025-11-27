#ifndef __GC_TESTING_UTILITIES__
#define __GC_TESTING_UTILITIES__

#include <stdio.h>
#include <stddef.h>
#include <inttypes.h>
#include <string.h>

static size_t TestsRun = 0;
static size_t TestsPassed = 0;

static inline void ResetTestStatus()
{
    TestsRun = 0;
    TestsPassed = 0;
}

static inline void PrintTestStatus(char *message)
{
    if(message == NULL)
        message = "Testing completed";

    printf("%s, %zu out of %zu tests passed.\n", message, TestsPassed, TestsRun);
}

#define TEST_VERBOSE(message, pattern, expressionString) printf(message ": %s; Values are %" pattern ", %" pattern "; at %s line %d\n", expressionString, valueA, valueB, __FILE__, __LINE__)

#ifdef TESTING_UTILITIES_VERBOSE
#define TEST_PRINT_SUCCESS(pattern, expressionString) \
TEST_VERBOSE("Test Passed", pattern, expressionString)
#else
#define TEST_PRINT_SUCCESS(pattern, expressionString)
#endif

#define TEST_BASE(a, b, expression, expressionString, type, pattern, ...)\
{\
    type volatile valueA = (type)(a);\
    type volatile valueB = (type)(b);\
    TestsRun++; \
    if(expression) \
    {\
        TEST_PRINT_SUCCESS(pattern, expressionString);\
        TestsPassed ++;\
    }\
    else\
    {\
        TEST_VERBOSE("Test Failed", pattern, expressionString);\
        __VA_ARGS__\
    }\
}

#define TEST_TYPED(a, comparer, b, type, pattern, ...) TEST_BASE(a, b, valueA comparer valueB, #a " " #comparer " " #b, type, pattern, __VA_ARGS__)

#define TEST(a, comparer, b, ...) TEST_TYPED(a, comparer, b, intptr_t, PRIdPTR, __VA_ARGS__)
#define TEST_FLOAT(a, comparer, b, ...) TEST_TYPED(a, comparer, b, double, "f", __VA_ARGS__)
#define TEST_STRING(a, comparer, b, ...) TEST_BASE(a, b, strcmp(a, b) comparer 0, "strcmp("#a ","#b") " #comparer " 0", char *, "s", __VA_ARGS__)

#endif