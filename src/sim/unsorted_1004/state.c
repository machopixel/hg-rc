#include "fns.h"
#include "../simmem.h"

// FUNCTION: HGSIM 0x100448cd
void __cdecl Sim_100448cd(int mode)
{
    if (*SIM_PTR(int, 0x10179cd4) != 0) {
        if (*SIM_PTR(int, 0x1013aee0) == -1)
            *SIM_PTR(int, 0x1013aee0) = 1;
        mode = *SIM_PTR(int, 0x1013aee0);
    }
    *SIM_PTR(int, 0x1015ef04) = mode;
}

// FUNCTION: HGSIM 0x100448ff
int __cdecl Sim_100448ff(void)
{
    return *SIM_PTR(int, 0x1015ef04);
}

// FUNCTION: HGSIM 0x10044992
int __cdecl Sim_10044992(const uint32_t *src, uint32_t *dst)
{
    /* volatile keeps the original's word-by-word order if the two overlap */
    const volatile uint32_t *s = src;
    volatile uint32_t *d = dst;

    if (dst == 0 || src == 0)
        return 0;
    d[0] = s[0];
    d[1] = s[1];
    d[2] = s[2];
    d[3] = s[3];
    d[4] = s[4];
    d[5] = s[5];
    d[6] = 1;
    return 1;
}
