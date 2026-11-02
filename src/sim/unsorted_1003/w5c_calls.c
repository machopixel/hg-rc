#include "w5c.h"
#include "w3r.h"
#include "../unsorted_100a/w4u.h"
#include "../util/access.h"
#include "../unsorted_1004/w3p.h"
#include "../../platform/music.h"
#include "../crt/hg_winheap.h"

// FUNCTION: HGSIM 0x1003a052
int __cdecl Sim_1003a052(void)
{
    uint32_t i;

#if !defined(W5C_MUTATE12) /* W5C_MUTATE12: the checker's negative check only */
    Music_Close();      /* 1.2 0x1002e0aa: frees its music stream [0x10131310] first (Music_Close: none open is fine) */
#endif
    if (G32(0x109e369c) != 0)
        SIM_CALL_1003a548(SIM_PTR(char, 0x109e369c));
    if (G32(0x1015d0a4) == 0)
        return 0;
    for (i = 0; i < U32(G32(0x1015d0a4), 4); i++) {
        if (U32(G32(0x1015d0a4), 0x54 + 4 * i) != 0)
            U32(G32(0x1015d0a4), 0x54 + 4 * i) = 0;
    }
    Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)G32(0x1015d0a4));
    G32(0x1015d0a4) = 0;
    return 1;
}

// FUNCTION: HGSIM 0x10041247
void __cdecl Sim_10041247(const uint8_t *msg)
{
    uint32_t rec = PTRTAB(msg[2]);

#ifndef NETG_MUTATE /* the checker's negative check only: no guard */
    if (g_w34NetOver != 0)          /* 1.2 (0x10034eef): ignored once the game is over */
        return;
#endif

    if (rec == 0)
        return;
    if (U32(rec, 4) == G32(0x1017b8c8))
        return;
    if (U32(rec, 0x14) == 3 && U32(rec, 0xc) == G32(0x1017b8c8))
        return;
    SIM_CALL_10038913(U32(rec, 0x28), U32(msg, 3), 0);
    SIM_CALL_100aa799(U32(rec, 0x48));
}

// FUNCTION: HGSIM 0x100354ff
void __cdecl Sim_100354ff(void)
{
    SIM_CALL_1008a5e3();
    Sim_100a87f0();
    if (G32(0x109e3d2c) != 0)
        ((void(__cdecl *)(void))(uintptr_t)G32(0x109e3d2c))();
    if (G32(0x1017b940) == 0 && G32(0x1015e534) != 0 && G32(0x109e3e00) != 0) {
        SIM_CALL_100eb070(GBYTES(0x109e3e00), 0xf0);
        if (G32(0x1017b920) != 0)
            ((void(__cdecl *)(void))(uintptr_t)U32(G32(0x1015e68c), 0x18))();
    }
    if (G32(0x1015e534) != 0)
        Hg_HeapFree(G32(0x1017b8f8), 1, (void *)(uintptr_t)G32(0x1015e534));
    G32(0x1015e538) = 0;
    Sim_10037c6c();
}
