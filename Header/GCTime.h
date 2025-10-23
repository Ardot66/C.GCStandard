#ifndef __GC_TIME__
#define __GC_TIME__

#include <time.h>

enum Timespans
{
    NSEC_PER_SEC = 1000000000
};

typedef struct timespec Timespec;

static inline Timespec TimespecGet(int base)
{
    Timespec timespec;
    timespec_get(&timespec, base);
    return timespec;
}

Timespec TimespecAdd(const Timespec t1, const Timespec t2);
Timespec TimespecDiff(const Timespec t1, const Timespec t2);
double TimespecToSecs(const Timespec time);
Timespec SecsToTimespec(double secs);

#endif