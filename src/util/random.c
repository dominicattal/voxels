#include "random.h"

u64 randu(u64 seed)
{
    // https://stackoverflow.com/questions/167735/fast-pseudo-random-number-generator-for-procedural-content
    seed = (seed ^ 61) ^ (seed >> 16);
    seed = seed + (seed << 3);
    seed = seed ^ (seed >> 4);
    seed = seed * 0x27d4eb2d;
    seed = seed ^ (seed >> 15);
    return seed;
}

f32 randf(u64 seed)
{
    u32 a = randu(seed);
    return (f32)(a) / (1LL<<32);
}