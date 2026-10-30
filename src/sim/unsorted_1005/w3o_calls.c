#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100556c2
void __cdecl Sim_100556c2(void)
{
    int i = GS32(0x10164bc0);

    while (i-- != 0) {
        if (G32(0x1015f868u + (uint32_t)i * 4) != 0) {
            uint32_t fn = U32(G32(0x1015fd20u + (uint32_t)i * 4), 0xc);

            ((void(__cdecl *)(void *))(uintptr_t)fn)(GBYTES(0x1015f880u + (uint32_t)i * 0x74));
        }
    }
}
