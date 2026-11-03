#include "w3t.h"
#include "fns.h"
#include "../util/w3t.h"
#include "../render/w2n.h"
#include "../unsorted_100f/w2e.h"

/* Live brightness (a deliberate addition, DECISIONS.md round 25): set by the hook layer in the game, 0 in the checker. */
int g_simLiveBrightness;

/* The cache entry of the mission palette (name 0x10185620, type [0x1014ced4]): Sim_100baf3f's lookup without its use
   count and time stamp update. 0 when it is not cached. */
static uint32_t W3t_PaletteEntry(void)
{
    const uint32_t *key = SIM_PTR(const uint32_t, 0x10185620);
    uint32_t table = SIM_FN(uint32_t(__cdecl *)(void *, uint32_t, void *), 0x100bda9f)(
        (void *)(uintptr_t)G32(0x1017d604), G32(0x1014ced4), SIM_PTR(void, 0x100b971d));
    uint32_t entry;

    if (table == 0 || S32(table, 0) == 0)
        return 0;
    for (entry = G32(U32(table, 0x10) + (uint32_t)((int32_t)(key[0] ^ key[1]) % S32(table, 0)) * 4); entry != 0;
         entry = U32(entry, 0x1c)) {
        if (Str_CompareN((const char *)(uintptr_t)entry, (const char *)key, 8) == 0)
            return entry;
    }
    return 0;
}

// FUNCTION: HGSIM 0x1004b7d2
void __cdecl Sim_1004b7d2(int value)
{
    uint32_t saved = G32(0x1013b98c);

    G32(0x1013b98c) = (uint32_t)value;
    /* The original sets only the 2D palette here ([0x1015e688] + 0x18 with [0x109b8c00]); the 3D texture palette is
       built at mission start (Sim_100d3000), so its brightness needed a restart. With the renderer ([0x1017b940]) that
       slot is 0x1008792c, which rewrites the 2D colour table 0x10156dcc from [0x109b8c00], a palette only the GDI /
       DirectDraw / DISPDIB drivers fill: in renderer modes it is empty, and the menus lost their colours (an original
       bug). In the game (g_simLiveBrightness) both are rebuilt as mission start does instead: Sim_100d3000 with this
       gamma row sets the texture palette and, through the driver's +0x14, the 2D colour table from the mission
       palette (DECISIONS.md, round 25). The original's non-renderer test is gone with its software side
       (round 42). */
    if (g_simLiveBrightness != 0) {
        uint32_t entry = W3t_PaletteEntry(), count = 0, stampLo = 0, stampHi = 0;

        if (entry != 0) {
            count = U32(entry, 8);
            stampLo = U32(entry, 0x10);
            stampHi = U32(entry, 0x14);
        }
        SIM_FN(void(__cdecl *)(void *), 0x100d3000)(0);
        /* its load of the mission palette is a cache hit: use count +1 and a new time stamp, put back */
        if (entry != 0) {
            U32(entry, 8) = count;
            U32(entry, 0x10) = stampLo;
            U32(entry, 0x14) = stampHi;
        }
    } else {
        ((void(__cdecl *)(void *))(uintptr_t)U32(G32(0x1015e688), 0x18))(GBYTES(0x109b8c00));
    }
    G32(0x1013b98c) = saved;
}

// FUNCTION: HGSIM 0x1004bc65
float __cdecl Sim_1004bc65(int a, int b)
{
    double q = (double)a / (double)b;
    float r = (float)q;

    if (q > (double)1.26f)
        return 0.25f;
    if ((double)r > 1.2)
        return 0.5f;
    if ((double)r > 1.15)
        return 0.6f;
    if ((double)r > 1.1)
        return 0.75f;
    return 1.0f;
}

// FUNCTION: HGSIM 0x1004bec6
int __cdecl Sim_1004bec6(int a, int b)
{
    float r = (float)((double)a / (double)b);

    if (a == b)
        return -3;
    if ((double)r >= 0.75)
        return -2;
    if ((double)r >= 0.5)
        return -1;
    return 0;
}

/* The selected entry of a 0x98-byte table. */
#define ENTRY(base, field) ((uint32_t)(base) + (uint32_t)(field) * 0x98u)

// FUNCTION: HGSIM 0x1004bfbd
int __cdecl Sim_1004bfbd(int index, int word)
{
    uint32_t e1 = ENTRY(0x10124eb8, Sim_1004bf79((unsigned)word));
    uint32_t e2 = ENTRY(0x10124eb8, Sim_1004bf87(word));
    uint32_t e3 = ENTRY(0x10124eb8, Sim_1004bf98(word));
    uint32_t sum = U32(GBYTES(0x101116b2u), (uint32_t)index * 0xaau) + U32(GBYTES(e1), 0) + U32(GBYTES(e2), 0)
                   + U32(GBYTES(e3), 0);
    float f = (float)(int32_t)sum;
    float f1, f2;

    f1 = (float)((double)f * (double)GFL32(e1 + 4));
    f2 = (float)((double)f1 * (double)GFL32(e2 + 4));
    return Render_Ftol((double)f2 * (double)GFL32(e3 + 4));
}

// FUNCTION: HGSIM 0x1004c0e5
int __cdecl Sim_1004c0e5(int index, int word, int ratio)
{
    uint32_t sum = U32(GBYTES(0x101116bau), (uint32_t)index * 0xaau)
                   + U32(GBYTES(ENTRY(0x10124ec4, Sim_1004bf79((unsigned)word))), 0)
                   + U32(GBYTES(ENTRY(0x10124ec4, Sim_1004bf87(word))), 0)
                   + U32(GBYTES(ENTRY(0x10124ec4, Sim_1004bf98(word))), 0);

    return (int)((uint32_t)Sim_1004bec6(ratio, S32(GBYTES(0x101116aeu), (uint32_t)index * 0xaau)) + sum);
}
