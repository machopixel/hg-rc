#include "w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10060656
void __cdecl Sim_10060656(uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint32_t a5, uint32_t a6, uint32_t a7)
{
    SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t, uint32_t), 0x10060220)(
        a1, a2, a3, a4, a5, a6, a7, 0);
}

// FUNCTION: HGSIM 0x10061b78
int __cdecl Sim_10061b78(uint32_t n)
{
    int v;

    if (n > 0xf)
        v = 0x16a;
    else
        v = (int)(int64_t)(((double)n / (double)15.0f) * (double)362.0f);
    return SIM_FN(int(__cdecl *)(int, int, int), 0x100d3510)(0x11, v, 1);
}
