#include "w5c.h"
#include "../unsorted_1002/w2k.h"
#include "../util/access.h"

typedef float HG_MAY_ALIAS w5c_f32;
typedef double HG_MAY_ALIAS w5c_f64;

// FUNCTION: HGSIM 0x100c0379
int __cdecl Sim_100c0379(void *obj, float f)
{
    int result = 0;

    if (SIM_FABS_C((double)f) > (double)*SIM_PTR(w5c_f32, 0x1012bb1c)) {
        w5c_f32 *p = (w5c_f32 *)(BYTES(U32(obj, 0x54)) + 8);

        result = 1;
        /* the compare sets "equal" for a NaN too */
        if (!(*p < *SIM_PTR(w5c_f32, 0x1012bb18) || *p > *SIM_PTR(w5c_f32, 0x1012bb18)))
            U32(U32(obj, 0x54), 8) = 0x3acccccd;
    }
    return result;
}

// FUNCTION: HGSIM 0x100c312c
int __cdecl Sim_100c312c(const void *obj)
{
    if (Sim_1002dc46(obj) != 0) {
        /* the original also calls fabs and compares with 1e-7f, then stores [0x1015dd5c] over itself: both outcomes are the same */
        (void)SIM_FABS_C((double)*SIM_PTR(w5c_f32, 0x1015dd5c));
        if (*SIM_PTR(w5c_f32, 0x1015dd5c) >= *SIM_PTR(w5c_f32, 0x1012bb84))
            return 1;
    }
    return (U16(obj, 2) & 0xf0) == 0x50;
}

// FUNCTION: HGSIM 0x100cca20
char *__cdecl Sim_100cca20(int n)
{
    int major = n / 0x9f150;
    int minor = (n - major * 0x9f150) / 0x2a6c;
    int rest = n - major * 0x9f150 - minor * 0x2a6c;
    double fraction = (double)rest / *SIM_PTR(w5c_f64, 0x1012bc60);

    SIM_CALL_100fc780(SIM_PTR(char, 0x1017e5c0), SIM_PTR(char, 0x10151dc8), major, minor, fraction);
    return SIM_PTR(char, 0x1017e5c0);
}
