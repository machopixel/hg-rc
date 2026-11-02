#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../unsorted_1002/w2j.h"
#include "../crt/hg_winheap.h"
#include "../unsorted_100f/w8z.h"
#include "../../platform/force.h"
#include "../../platform/plat.h"

/* Force feedback: FFORCE.DLL's three functions are force.h's now (DECISIONS.md, "Force feedback kept for rumble"); with
 * no device the sim takes the original's path for a missing FFORCE.DLL. */

// FUNCTION: HGSIM 0x10045d70
int __cdecl Sim_10045d70(uint32_t a, uint32_t b)
{
    if (!Force_Present()) {
        Plat_DebugText((const char *)GBYTES(0x1013afe0));     /* "Error loading TS_SFRCE.dll" */
        return 0;
    }
    return Force_Init(a, b);
}

// FUNCTION: HGSIM 0x10045db9
int __cdecl Sim_10045db9(void)
{
    int result;
    char text[0x64];

    if (G32(0x1015ef3c) == 0)
        return 0;
    if (Force_Present()) {
        result = Force_Effect(GBYTES(0x1013af00));
        if (result < 0) {
            Crt_SPrintf(text, (const char *)GBYTES(0x1013affc), result);
            Plat_DebugText(text);
        }
    } else {
        result = (int)0x8004050f;
    }
    return result;
}

// FUNCTION: HGSIM 0x100481dd
void __cdecl Sim_100481dd(int index)
{
    if (index >= 0) {
        Sim_1004821c(index);
        return;
    }
    for (index = 0; index < 7; index++)
        Sim_1004821c(index);
}

// FUNCTION: HGSIM 0x1004821c
void __cdecl Sim_1004821c(int index)
{
    uint32_t slot = 0x109b94e0u + (uint32_t)index * 4u;
    uint32_t rec = G32(slot);

    if (rec != 0) {
        if (U32(rec, 0x18) != 0)
            Sim_1002828e((int)(U32(rec, 0x14) + G32(0x101c64f0)), (const void *)(uintptr_t)G32(0x1014cec0));
        Hg_HeapFree(G32(0x1017b8f8), 1, BYTES(rec));
        G32(slot) = 0;
    }
}
