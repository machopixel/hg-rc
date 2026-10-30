#include "w3p.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x1000b01f
int __cdecl Sim_1000b01f(float a, float b, float c, float d)
{
    return X87_LessOrUnordered(-(double)b, (double)a) && X87_LessOrUnordered((double)a, (double)b) &&
           X87_LessOrUnordered(-(double)d, (double)c) && X87_LessOrUnordered((double)c, (double)d);
}

// FUNCTION: HGSIM 0x1000e6c5
void __cdecl Sim_1000e6c5(void)
{
}
