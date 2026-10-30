#include "w5c.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;
typedef double HG_MAY_ALIAS w5c_f64;

// FUNCTION: HGSIM 0x10084a17
int __cdecl Sim_10084a17(void)
{
    int i;

    for (i = 0; i < 0x320; i++) {
        double t = (double)i / *SIM_PTR(w5c_f64, 0x1012b898);
        double u = (double)i / *SIM_PTR(w5c_f64, 0x1012b898);
        double inv = *SIM_PTR(w5c_f64, 0x1012b8a0) / SIM_SQRT_C(t * u + *SIM_PTR(w5c_f64, 0x1012b8a0));

        SIM_PTR(w5c_f32, 0x10370d20)[i] = (float)inv;
        SIM_PTR(w5c_f32, 0x1036f860)[i] = (float)(((double)i / *SIM_PTR(w5c_f64, 0x1012b898)) * inv);
    }
    return 1;
}
