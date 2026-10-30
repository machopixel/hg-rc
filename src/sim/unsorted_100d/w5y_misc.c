#include "w5y.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100db205
void __cdecl Sim_100db205(void *base)
{
    int i;

    for (i = 0; i < 0x10; i++) {
        SIM_FN(int(__cdecl *)(int, void *), 0x1005259e)(i, BYTES(base) + 8 + (uint32_t)i * 0x11u);
        G32(0x109b7d50u + (uint32_t)i * 0x6cu) = 0;
    }
}
