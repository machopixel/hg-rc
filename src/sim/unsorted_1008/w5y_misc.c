#include "w5y.h"
#include "../unsorted_1001/w5y.h"
#include "../util/access.h"
#include "../libm/hg_math.h"

// FUNCTION: HGSIM 0x1008b8de
void __cdecl Sim_1008b8de(void)
{
    if (G32(0x1015ecd0) == 0) {
        SIM_FN(void(__cdecl *)(int, int), 0x100e46d0)(0x80, 1);
        Sim_1001f4ba();
    }
}

// FUNCTION: HGSIM 0x1008e211
int __cdecl Sim_1008e211(float a, float b, float *out)
{
    /* compared at full precision; the original compares st(0) as the callee left it, before any rounding to float */
    double root = hg_fabs((double)a);
    int small = !(root >= (double)1.0e-7f);

    if (!small)
        *out = (float)((double)b / (double)a);
    return small;
}
