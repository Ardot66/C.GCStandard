#ifndef __GC_MATH__
#define __GC_MATH__

#define min(a, b) ((a <= b) * a + (b < a) * b)
#define max(a, b) ((a >= b) * a + (b > a) * b)
#define clamp(a, min, max) (((a >= min) & (a <= max)) * a + (a < min) * min + (a > max) * max)

#endif