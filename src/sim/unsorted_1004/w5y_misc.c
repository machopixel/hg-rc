#include "w5y.h"
#include "../util/access.h"
#include "w3p.h"

// FUNCTION: HGSIM 0x10041a16
void __cdecl Sim_10041a16(void)
{
#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x10035801): ignored once the game is over */
        return;
#endif
    SIM_FN(void(__cdecl *)(void), 0x1007c42f)();
    if (G32(0x1015f1fc) == 0) {
        G32(0x1015f1fc) = 1;
        G32(0x1015f1e4) = 0;
        G32(0x1015f1d0) = 0;
    }
}

// FUNCTION: HGSIM 0x10044069
void __cdecl Sim_10044069(void)
{
    G32(0x1015ed20) = 0;
    G32(0x1015ed38) = 0;
    if (GS32(0x1013aec4) > 0) {
        G32(0x1015ee98) = 1;
        SIM_FN(void(__cdecl *)(int), 0x1004409f)(0);
    }
}

// FUNCTION: HGSIM 0x10048720
int __cdecl Sim_10048720(short value)
{
    if (((int)value & 0xff00) != 0x700)
        return 0;
    SIM_FN(void *(__cdecl *)(void *, const void *, uint32_t), 0x100fdf00)(GBYTES(0x109b9480), GBYTES(0x109b9481), 0x3e);
    U8(GBYTES(0x109b94be), 0) = (uint8_t)value;
    return 1;
}

// FUNCTION: HGSIM 0x1004edb6
void __cdecl Sim_1004edb6(void)
{
    G32(0x1013b9a8) = 3;
    SIM_FN(void(__cdecl *)(void), 0x1004e838)();
}
