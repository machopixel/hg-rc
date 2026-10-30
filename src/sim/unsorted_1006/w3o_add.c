#include "w3o.h"

// FUNCTION: HGSIM 0x10068e1d
void __cdecl Sim_10068e1d(float ax, float ay, float az, float bx, float by, float bz, float *outX, float *outY, float *outZ)
{
    if (outX != 0 && outY != 0 && outZ != 0) {
        *outX = (float)((double)ax + bx);
        *outY = (float)((double)ay + by);
        *outZ = (float)((double)az + bz);
    }
}
