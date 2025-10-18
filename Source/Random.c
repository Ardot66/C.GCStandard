#include <time.h>
#include "GCRandom.h"

RandomState GlobalRandomState = {0};

static void GlobalRandomInit()
{
    if(GlobalRandomState.State[0] == 0)
		GlobalRandomState = RandomStateFromTime();
}

RandomState RandomStateFromTime()
{
	struct timespec timespec;
	timespec_get(&timespec, 0);
	
    RandomState randomState =
    {
        .State[0] = timespec.tv_sec,
        .State[1] = timespec.tv_nsec
    };

    return randomState;
}

uint64_t GlobalRandom()
{
	GlobalRandomInit();
    return Random(&GlobalRandomState);
}

uint64_t Random(RandomState *state)
{
	uint64_t x = state->State[0];
	uint64_t const y = state->State[1];
	state->State[0] = y;
	x ^= x << 23; // shift & xor
	x ^= x >> 17; // shift & xor
	x ^= y; // xor
	state->State[1] = x + y;
	return (size_t)x;
}

double GlobalRandomFloat()
{
	GlobalRandomInit();
	return RandomFloat(&GlobalRandomState);
}

// Returns a random float between zero and one
double RandomFloat(RandomState *state)
{
    return (double)Random(state) / (double)UINT64_MAX;
}