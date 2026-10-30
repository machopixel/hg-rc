#include "w3q.h"

// FUNCTION: HGSIM 0x1006112e
void __cdecl Quat_Multiply(const float *a, const float *b, float *out)
{
    float dot = (float)((double)a[0] * (double)b[0] + (double)a[1] * (double)b[1] + (double)a[2] * (double)b[2]);
    float w = (float)((double)a[3] * (double)b[3] - (double)dot);
    float x = (float)((double)a[3] * (double)b[0] + (double)b[3] * (double)a[0] + (double)a[1] * (double)b[2] -
                      (double)a[2] * (double)b[1]);
    float y = (float)((double)a[3] * (double)b[1] + (double)b[3] * (double)a[1] + (double)a[2] * (double)b[0] -
                      (double)a[0] * (double)b[2]);
    float z = (float)((double)a[3] * (double)b[2] + (double)b[3] * (double)a[2] + (double)a[0] * (double)b[1] -
                      (double)a[1] * (double)b[0]);

    out[3] = w;
    out[0] = x;
    out[1] = y;
    out[2] = z;
}
