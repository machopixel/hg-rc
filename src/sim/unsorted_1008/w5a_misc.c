#include "w5a.h"
#include "../util/access.h"
#include "../view/view.h"
#include "../unsorted_1000/w2i.h"

// FUNCTION: HGSIM 0x1008349a
void __cdecl Sim_1008349a(const float *m)
{
    int i;

    for (i = 0; i < 3; i++)
        W5A_CALL_100348d8(SIM_PTR(char, 0x1014aafc), (double)m[i], (double)m[i + 3], (double)m[i + 6],
                          (double)m[i + 9]);
}

#include "w26_trig_tables.inc"

/*
 * Round 26: the tables are the original's own results, captured on the game machine (tools/round26/trigcap.c runs
 * 0x100846e0 natively: x87 fsin / fpatan, rounded once from 64-bit mantissas to float) and copied here, instead of
 * computing them again (DECISIONS.md, round 26).
 */
// FUNCTION: HGSIM 0x100846e0
int __cdecl Trig_InitTables(void)
{
    int i;

    for (i = 0; i < 0x200; i++) {
#ifdef W26T_MUTATE /* the checker's negative check only */
        SIM_PTR(u32a, 0x103719a0)[i] = kTrigSine[i] ^ (uint32_t)(i == 0x80);
#else
        SIM_PTR(u32a, 0x103719a0)[i] = kTrigSine[i];
#endif
        SIM_PTR(u32a, 0x103721c0)[i] = kTrigAtan[i];
    }
    G32(0x103721a0) = kTrigSineEnd;
    G32(0x103729c0) = kTrigAtanEnd;
    return 1;
}

// FUNCTION: HGSIM 0x1008799c
int __cdecl Sim_1008799c(const uint8_t *rgb)
{
    int i;
    uint32_t r, g, b;

    for (i = 0; i < 0x100; i++) {
        r = rgb[i * 3];
        g = rgb[i * 3 + 1];
        b = rgb[i * 3 + 2];
        W5A_CALL_100f01cc(i, ((r >> 1) << 11) | (g << 5) | (b >> 1));
    }
    return 0;
}

// FUNCTION: HGSIM 0x10088c1c
int __cdecl Sim_10088c1c(uint32_t out)
{
    uint32_t node;
    int count = 0;
    volatile int unusedCount;

    W5A_CALL_100fd7e0(out, SIM_PTR(char, 0x1014b8f0));
    for (node = G32(0x1017b9a0); node != 0; node = U32(node, 0x18))
        count++;
    unusedCount = count;
    count = 0;
    W5A_CALL_100fd7e0(out, SIM_PTR(char, 0x1014b904));
    for (node = G32(0x1017b998); node != 0; node = U32(node, 0x10))
        count++;
    return count;
}

// FUNCTION: HGSIM 0x1008fce3
void __cdecl Sim_1008fce3(uint32_t flags)
{
    int i;
    uint32_t node;

    if (flags & 1)
        W5A_CALL_100a0def(G32(0x1014cae0));
    Sim_10087c86();
    for (i = 0; (node = SIM_PTR(u32a, 0x1036f2a0)[i]) != 0; i++)
        W5A_CALL_100a4f03(node);
    Sim_100071f6();
}
