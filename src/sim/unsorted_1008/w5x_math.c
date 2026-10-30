#include "w5x.h"

// FUNCTION: HGSIM 0x10084b57
double __cdecl Sim_10084b57(float a, float b)
{
    double x = a;
    double y = b;

    return SIM_CALL_100fca04(x * x + y * y);
}
