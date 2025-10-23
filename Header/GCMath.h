#ifndef __GC_MATH__
#define __GC_MATH__

// Prefixing with GC here because conflicts with other headers are very likely otherwise
#define GCMin(a, b) ((a <= b) * a + (b < a) * b)
#define GCMax(a, b) ((a >= b) * a + (b > a) * b)
#define GCClamp(a, min, max) (((a >= min) & (a <= max)) * a + (a < min) * min + (a > max) * max)

#endif