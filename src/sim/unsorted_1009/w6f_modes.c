#include "w6f.h"
#include "w2f.h"
#include "w2g.h"
#include "w5y.h"
#include "../util/access.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1002/w5a.h"
#include "../view/w2d.h"

#define ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, a))

// FUNCTION: HGSIM 0x10098721
int32_t __cdecl Sim_10098721(int32_t unused1, int32_t unused2)
{
    (void)unused1;
    (void)unused2;
    if (G32(0x1015d1b8) != 0) {
        W6F_CALL_100988fc(1, U32(G32(0x1015d1b8), 0x14));
        W6F_CALL_100989a1(1, U32(G32(0x1015d1b8), 0x18));
        Sim_10098a18(1, (int)U32(G32(0x1015d1b8), 0x1c));
        W6F_CALL_100b5bcb(1, U32(G32(0x1015d1b8), 0x24));
        Sim_10098826(1, U32(G32(0x1015d1b8), 0x28));
        Sim_1009885e(1, (int)U32(G32(0x1015d1b8), 0x2c));
        Sim_10098896(1, (int)U32(G32(0x1015d1b8), 0x30));
    }
    return 1;
}

/* Starts the sound named by the string the dword at `field` of the record [0x1014c4d8] points to, if any. */
static void PlayModeSound(uint32_t field)
{
    const char *name = (const char *)(uintptr_t)U32(G32(0x1014c4d8), field);

    if (*name != 0)
        Sim_1000ebfb(name, 0, 0, 5, 0x50);
}

// FUNCTION: HGSIM 0x100999fc
int32_t __cdecl Sim_100999fc(void)
{
    int32_t changed = 0;

    if (GS32(0x1015d1ac) <= 2 && G32(0x1015d098) == 4 && G32(0x1017cc54) != 0) {
        G32(0x10144fa4) = G32(0x1014bef0);
        G32(0x10144fa8) = G32(0x1014bef8);
        G32(0x1017cc54) = 0;
    }
    if (G32(0x1015d1ac) == 4 && (G32(0x10179cd4) != 0 || (S16(PTRTAB(G32(0x1017b8c8)), 0x1c) & 6) != 0))
        G32(0x1015d1cc) = 5;
    if (G32(0x1015d1ac) == G32(0x1015d1cc))
        return changed;
    changed = 1;
    switch (G32(0x1015d1cc)) {
    case 0:
        G32(0x1015d098) = G32(0x1015d1ac);
        G32(0x1015d1ac) = 0;
        break;
    case 1:
        G32(0x1015d098) = G32(0x1015d1ac);
        G32(0x1015d1ac) = 1;
        break;
    case 2:
        G32(0x1015d098) = G32(0x1015d1ac);
        G32(0x1015d1ac) = 2;
        break;
    case 3:
        G32(0x10144fa0) = 0;
        G32(0x1015d1cc) = 4;
        G32(0x1014beec) = G32(0x1014cb3c);
        G32(0x1014cb3c) = ADDR(0x10003210);
        PlayModeSound(0xc);
        Sim_100203cf(0x27, -1);
        break;
    case 4:
        G32(0x1015d098) = G32(0x1015d1ac);
        G32(0x1015d1ac) = 4;
        if (G32(0x1017cc54) == 0) {
            G32(0x1014bef0) = G32(0x10144fa4);
            G32(0x10144fa4) = 0;
            G32(0x1014bef8) = G32(0x10144fa8);
            G32(0x10144fa8) = 0;
            G32(0x1014bef4) = G32(0x10144fa0);
            G32(0x1017cc54) = 1;
        }
        break;
    case 5:
        G32(0x10144fa0) = 1;
        G32(0x1014cb3c) = G32(0x1014beec);
        G32(0x1015d1cc) = G32(0x1015d098);
        PlayModeSound(0x10);
        break;
    default:
        break;
    }
    return changed;
}

/* The original copies with `rep movsd`: 5 dwords, upwards. */
static void Copy5(uint32_t dst, uint32_t src)
{
    int i;

    for (i = 0; i < 5; i++)
        U32(dst, 4 * i) = U32(src, 4 * i);
}

// FUNCTION: HGSIM 0x1009b8d8
int32_t __cdecl Sim_1009b8d8(int32_t flip, uint32_t *view, const uint32_t *pair, int32_t apply)
{
    uint32_t cur = U32(view, 0);
    int32_t found = 1;
    uint32_t block = (uint32_t)(uintptr_t)Sim_1008f747(flip, pair);

    if (block == 0) {
        found = 0;
        if (apply != 0)
            block = flip == 0 ? U32(U32(pair, 4), 8) : U32(U32(pair, 4), 4);
    }
    if (block != 0) {
        Copy5(U32(view, 4), cur);
        Copy5(cur, block);
        W6F_CALL_10099c59();
        Copy5(cur, U32(view, 4));
        if (apply != 0) {
            Copy5(ADDR(0x109e3e00), block);
            G32(0x109e3dac) = 1;
        }
    }
    return found;
}
