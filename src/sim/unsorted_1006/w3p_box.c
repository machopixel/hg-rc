#include "w3p.h"
#include "../util/w3p.h"

// FUNCTION: HGSIM 0x100686ea
float __cdecl Math_ClampFloat_100686ea(float value, float lo, float hi)
{
    if (X87_LessOrUnordered((double)value, (double)lo))
        return lo;
    if (!X87_LessEqualOrUnordered((double)value, (double)hi))
        return hi;
    return value;
}

// FUNCTION: HGSIM 0x10068716
int __cdecl Sim_10068716(const float *box, float x, float y, float z, float *outX, float *outY, float *outZ)
{
    if (box == 0 || outX == 0 || outY == 0 || outZ == 0)
        return 0;
    *outX = Math_ClampFloat_100686ea(x, box[0], box[1]);
    *outY = Math_ClampFloat_100686ea(y, box[2], box[3]);
    *outZ = Math_ClampFloat_100686ea(z, box[4], box[5]);
    return 1;
}

// FUNCTION: HGSIM 0x10068b10
int __cdecl Sim_10068b10(const float *box, float x, float y, float z)
{
    float cx, cy, cz;

    if (box == 0)
        return 0;
    cx = Math_ClampFloat_100686ea(x, box[0], box[1]);
    cy = Math_ClampFloat_100686ea(y, box[2], box[3]);
    cz = Math_ClampFloat_100686ea(z, box[4], box[5]);
    return X87_EqualOrUnordered((double)cx, (double)x) && X87_EqualOrUnordered((double)cy, (double)y) &&
           X87_EqualOrUnordered((double)cz, (double)z);
}
