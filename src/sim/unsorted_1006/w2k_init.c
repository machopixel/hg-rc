#include "w2k.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1006271a
void __cdecl Sim_1006271a(void)
{
    int i;

    G32(0x10165BE0) = 0;
    G32(0x10165BE4) = 0;
    for (i = 0; i < 10; i++) {
        G32(0x10165BE8 + i * 0x18) = 0;
        G32(0x10165BF8 + i * 0x18) = 0;
        G32(0x10165BEC + i * 0x18) = 0;
    }
}
