#include "w9g.h"
#include "../util/access.h"
#include "../unsorted_100c/w7q.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"

/* One text draw through the drawer the display globals select, with [0x109b992e] set around it. */
static void DrawText(uint8_t mode, uint32_t surface, int32_t x, int32_t y, uint32_t font, const char *text)
{
    *GBYTES(0x109b992e) = mode;
    if (G32(0x101668e8) != 0)
        W9GC_CALL_100f2f01(surface, x, y, font, text, GBYTES(0x109b9920));
    else if (G32(0x1017b940) != 0)
        W9GC_CALL_100f1e10(surface, x, y, font, text, GBYTES(0x109b9920));
    else
        W9GC_CALL_100ecea6(surface, x, y, font, text, GBYTES(0x109b9920));
    *GBYTES(0x109b992e) = 9;
}

// FUNCTION: HGSIM 0x100cd70f
void __cdecl Sim_100cd70f(void *obj, int32_t *pos, uint32_t font)
{
    char buf[0x100], full[0x100];
    const char *space;
    uint8_t *base = GBYTES(0x10473800) + G32(0x1013bb40) * 0x4fc3au;
    int32_t height = W9GC_CALL_100ecce0(font);
    int single = 0;
    int32_t i, len, cut, rest, x0, y0;
    uint8_t *e;
    const char *label;
    int wrapped;

    if (G32(0x1015ec58) != 0 && U8(G32(0x1017b8c4), 0xa) == 0) {
        int32_t active = 0;
        int kind2 = 0;

        for (i = 0; i < S32(base, 0); i++) {
            e = base + 0x3a + (uint32_t)i * 0x13fu;
            active += e[0x74];
            if (e[0x74] != 0 && e[1] == 2)
                kind2 = 1;
        }
        if (kind2 && active == 1)
            single = 1;
    }
    for (i = 0; i < S32(base, 0); i++) {
        e = base + 0x3a + (uint32_t)i * 0x13fu;
        if (e[0x74] == 0 || e[0] == 5 || e[0] == 6 || U32(e, 0x75) == 0)
            continue;
        /* the first active entry that is not of kind 5 or 6 and has +0x75 set is drawn, then the function
           returns */
        pos[0] = S32(obj, 0xb4);
        wrapped = 0;
        switch ((uint32_t)e[1] - 1u) {
        case 0:
            label = (const char *)(uintptr_t)G32(0x101c4994);
            break;
        case 1:
            label = (const char *)(uintptr_t)(single ? G32(0x101c4994) : G32(0x101c498c));
            break;
        case 7:
            label = (const char *)(uintptr_t)G32(0x101c49a8);
            break;
        default:
            label = (const char *)(uintptr_t)G32(0x101c4988);
            break;
        }
        DrawText(4, U32(obj, 0xec), pos[0], pos[1], font, label);
        pos[0] += Sim_100ccb0d(label, (void *)(uintptr_t)font);
        len = (int32_t)Str_Length((const char *)e + 0xad);
        e[0xad + len] = 0;
        /* 1.2 (0x100b1c8f): the objective is copied whole into its own buffer first; it wraps only when longer
           than the width (1.0: also when equal) and always breaks at the objective's last space (1.2 tests the
           byte it has just zeroed, so the search always runs, over the whole copy) */
        Str_Copy(full, (const char *)e + 0xad);
#ifdef W9GW_MUTATE                      /* the checker's negative check only: 1.0's width test */
        if (len < GS32(0x10151d60 + G32(0x101c64f0) * 4u)) {
#else
        if (len <= GS32(0x10151d60 + G32(0x101c64f0) * 4u)) {
#endif
            DrawText(0xc, U32(obj, 0xec), pos[0], pos[1], font, full);
        } else {
            x0 = pos[0];
            y0 = pos[1];
            cut = GS32(0x10151d60 + G32(0x101c64f0) * 4u);
            space = Str_FindLast(full, ' ');
            if (space != 0)         /* deviation: 1.2 takes a cut from a NULL result (no space at all) */
                cut = (int32_t)(space - full);
            Str_CopyN(buf, full, (uint32_t)cut + 1u);
            buf[cut + 1] = 0;
            DrawText(0xc, U32(obj, 0xec), pos[0], pos[1], font, buf);
            pos[0] = x0;
            pos[1] += height;
            rest = (int32_t)Str_Length(full) - cut;
            if (rest > 0) {
                Str_CopyN(buf, full + cut + 1, (uint32_t)rest);
                buf[rest] = 0;
                DrawText(0xc, U32(obj, 0xec), pos[0], pos[1], font, buf);
            }
            pos[0] = x0;
            pos[1] = y0;
            wrapped = 1;
        }
        if (wrapped)
            pos[1] += height;
        break;      /* only the first entry that passes the filter is drawn (0x100cde69 leaves the loop) */
    }
}
