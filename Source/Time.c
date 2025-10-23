#include "GCTime.h"
#include <math.h>

static inline Timespec TimespecNormalize(time_t tv_sec, long tv_nsec)
{
    Timespec timespec;
    timespec.tv_sec = (tv_nsec >= 0 ? tv_nsec : tv_nsec - (NSEC_PER_SEC - 1)) / NSEC_PER_SEC;
    timespec.tv_nsec = tv_nsec - timespec.tv_sec * NSEC_PER_SEC;
    timespec.tv_sec += tv_sec;
    return timespec;
}

Timespec TimespecAdd(const Timespec t1, const Timespec t2)
{
    return TimespecNormalize(t1.tv_sec + t2.tv_sec, t1.tv_nsec + t2.tv_nsec);
}

Timespec TimespecDiff(const Timespec t1, const Timespec t2)
{
    return TimespecNormalize(t1.tv_sec - t2.tv_sec, t1.tv_nsec - t2.tv_nsec);
}

double TimespecToSecs(const Timespec timespec)
{
    double time = (double)timespec.tv_sec;
    time += (double)timespec.tv_nsec / (double)NSEC_PER_SEC;
    return time;
}

Timespec SecsToTimespec(double secs)
{
    Timespec timespec;
    timespec.tv_sec = (time_t)secs;
    timespec.tv_nsec = (long)((secs - floor(secs)) * (double)NSEC_PER_SEC);
    return timespec;
}