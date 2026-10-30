#include "w3r.h"
#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1002986c
void __cdecl Sim_1002986c(void)
{
    int i, k;

    for (i = 0; i < 0x80; i++) {
        uint32_t entry = 0x109f09e0u + (uint32_t)i * 0x14u;
        uint32_t value = G32(entry + 4);

        if (value != 0 && G32(entry + 0x10) != 0)
            ((void(__cdecl *)(uint32_t))(uintptr_t)G32(entry + 0x10))(value);
        for (k = 0; k < 5; k++)
            G32(entry + (uint32_t)k * 4) = G32(0x1015dc48u + (uint32_t)k * 4);
    }
    for (i = 0; i < 0x80; i++)
        Sim_1002990f(i);
}
