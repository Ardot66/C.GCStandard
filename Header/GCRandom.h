#ifndef __GC_RANDOM__
#define __GC_RANDOM__

#include <stdint.h>

#define RANDOM_STATE_STATES 2

typedef struct RandomState
{
    uint64_t State[RANDOM_STATE_STATES];
} RandomState;

extern RandomState GlobalRandomState; 

RandomState RandomStateFromTime();

uint64_t GlobalRandom();
uint64_t Random(RandomState *state);
double GlobalRandomFloat();
double RandomFloat(RandomState *state);

#endif