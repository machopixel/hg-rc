#include "w3t.h"
#include "../util/access.h"
#include "../unsorted_1004/w3t.h"

/* value * 15 / 65536, rounded: shrd / adc of the original. */
static int32_t Scale15(int32_t value)
{
    int64_t p = (int64_t)value * 15;

    return (int32_t)((uint32_t)(p >> 16) + (uint32_t)((p >> 15) & 1));
}

// FUNCTION: HGSIM 0x100219c8
void __cdecl Sim_100219c8(int unused, int value)
{
    int32_t scaled = Scale15(value);

    (void)unused;
    if (scaled != GS32(0x1013b990)) {
        GS32(0x1013b990) = scaled;
        Sim_1004b7d2(scaled);
    }
}

// FUNCTION: HGSIM 0x10021a10
void __cdecl Sim_10021a10(int unused, int value)
{
    (void)unused;
    GS32(0x1013b990) = Scale15(value);
    GS32(0x1013b98c) = GS32(0x1013b990);
    S32(G32(0x1015d1b8), 0x44) = GS32(0x1013b98c);
    Sim_1004b7d2(GS32(0x1013b98c));
}

// FUNCTION: HGSIM 0x10021a60
void __cdecl Sim_10021a60(int unused)
{
    (void)unused;
    G32(0x1013b990) = G32(0x1013b98c);
    Sim_1004b7d2(GS32(0x1013b98c));
}

// FUNCTION: HGSIM 0x10029fba
void __cdecl Sim_10029fba(void)
{
    int i;

    for (i = 0; i < GS32(0x1015dc64); i++) {
        uint32_t fn = U32(PTRTAB(i), 0x34);
        uint32_t arg = U32(PTRTAB(i), 0x28);

        if (fn != 0 && arg != 0)
            ((void(__cdecl *)(uint32_t))(uintptr_t)fn)(arg);
    }
}
