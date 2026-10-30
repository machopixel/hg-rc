#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10084a94
double __cdecl Sim_10084a94(float x, float y, float z)
{
    float big = Float_OfBits(Bits_OfFloat(x) & 0x7fffffffu);
    float mid = Float_OfBits(Bits_OfFloat(z) & 0x7fffffffu);
    float small = Float_OfBits(Bits_OfFloat(y) & 0x7fffffffu);
    float t;

    if (!(big >= mid)) {
        t = big;
        big = mid;
        mid = t;
    }
    if (!(big >= small)) {
        t = big;
        big = small;
        small = t;
    }
    return ((double)big * GFL(0x1012b8a8) + mid + small) / GFL(0x1012b8a8);
}

// FUNCTION: HGSIM 0x10084c95
void __cdecl Sim_10084c95(float *x, float *y, float *z)
{
    double length = Sim_10084a94(*x, *y, *z);
    float stored = (float)length;

    if (length > GFL(0x1012b870)) {
        *x = (float)((double)*x / stored);
        *y = (float)((double)*y / stored);
        *z = (float)((double)*z / stored);
    }
}
