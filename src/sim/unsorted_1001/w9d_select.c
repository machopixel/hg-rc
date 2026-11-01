#include "w9d.h"
#include "w2e.h"
#include "w2f.h"
#include "w2h.h"
#include "w3s.h"
#include "w5c.h"
#include "../unsorted_1004/fns.h"
#include "../util/access.h"
#include "../util/util.h"

/* The record passes when its dword +0x14 is 2, or 3 and its +0xc is the current player [0x1017b8c8]. */
static int W9d_Selectable(uint32_t rec)
{
    if (U32(rec, 0x14) == 2)
        return 1;
    return U32(rec, 0x14) == 3 && U32(rec, 0xc) == G32(0x1017b8c8);
}

/* The group's record is kept out when [0x1015d120 + index * 4] is set and PTRTAB(idx) +0x14 is 0 or 1. */
static int W9d_Held(int index, int16_t idx)
{
    uint32_t p = SIM_PTR(u32a, 0x109e8b40)[(int32_t)idx];

    if (U32(p, 0x14) != 0 && U32(p, 0x14) != 1)
        return 0;
    return G32(0x1015d120 + (uint32_t)index * 4) != 0;
}

static void W9d_Apply(uint32_t rec, int16_t idx, uint32_t mode, uint16_t sel, uint32_t flags)
{
    if (S32(rec, 4) == (int32_t)idx && mode == 5)
        Sim_1001555c((void *)(uintptr_t)rec, 0, 0);
    else
        Sim_1001555c((void *)(uintptr_t)rec, (int16_t)mode, (int16_t)sel);
    U32(rec, 0x1ae) = U32(rec, 0x1ae) & 0xfffffffcu;
    U32(rec, 0x1ae) = U32(rec, 0x1ae) | flags;
    Sim_10017852((void *)(uintptr_t)rec);
}

// FUNCTION: HGSIM 0x10016632
void __cdecl Sim_10016632(int index)
{
    uint16_t sel = 0;
    int16_t idx;
    uint32_t list[8]; /* the original's 0x20-byte list; Sim_10016cb7 fills entries 0..count-1 and maybe 4 */
    int count;
    uint8_t *table;
    int32_t entry;
    int32_t off;
    uint16_t kind;
    uint32_t mode;
    uint32_t flags;
    int bit;
    int i;

    idx = (int16_t)Sim_10052aaf(index);
    if (idx == -1) {
        idx = (int16_t)Sim_10012843(index);
        if (idx == -1)
            return;
    }
    count = Sim_10016cb7(index, list, 1);
    if (count == 0)
        return;
    table = GBYTES(0x10473800) + (uint32_t)index * 0x4fc3au;
    entry = (int32_t)SIM_PTR(u32a, 0x104727a0)[index];
    off = (int32_t)((uint32_t)entry * 0x13fu); /* imul: wraps, -1 addresses the bytes before the table */
    if (entry != -1)
        kind = U16(table, off + 0x3c);
    else
        kind = 0x200;
    if (kind == 0x400) {
        mode = 10;
    } else if (kind == 0x800 || kind == 0x10) {
        mode = 11;
    } else {
        sel = (uint16_t)(idx | 0x200);
        mode = 5;
    }
    /* the original reads the entry's dword +0xb7 even when the selection is -1 (entry -1 of the table) */
    flags = 0x40 | (uint32_t)Sim_10016a4a((int)U32(table, off + 0xb7));
    for (bit = 0; bit < 16; bit++) {
        if ((kind & (1u << bit)) != 0)
            break;
    }
    for (i = 0; i < count; i++) {
        uint32_t rec = list[i];

        if (!W9d_Selectable(rec))
            continue;
        Sim_10016a6b((void *)(uintptr_t)rec, 2, (int16_t)(bit + 1), idx);
        if (G32(0x1015ec58) != 0) {
            /* 1.2 (0x1000f7d8): the network game's own list entry ([0x1014c99c]) is never held back */
#ifdef W9DN_MUTATE /* the checker's negative check only: 1.0's test */
            if (W9d_Held(index, idx))
#else
            if (W9d_Held(index, idx) && GS32(0x1014c99c) != i)
#endif
                continue;
        } else if ((int32_t)idx == GS32(0x1017b8c8) && G32(0x1015d120 + (uint32_t)index * 4) != 0) {
            continue;
        }
        W9d_Apply(rec, idx, mode, sel, flags);
    }
    if (G32(0x109b7d48 + (uint32_t)index * 0x6cu) != 0) {
        uint32_t rec = list[4];

        if (W9d_Selectable(rec)) {
            Sim_10016a6b((void *)(uintptr_t)rec, 2, (int16_t)(bit + 1), idx);
            if (!W9d_Held(index, idx))
                W9d_Apply(rec, idx, mode, sel, flags);
        }
    }
    if (G32(0x1015ec58) != 0) {
        int k = Sim_10052aaf(index);
        uint32_t p = SIM_PTR(u32a, 0x109e8b40)[k];

        if ((U32(p, 0x14) == 0 || U32(p, 0x14) == 1) && kind != 0x10)
            G32(0x1015d120 + (uint32_t)index * 4) = 1;
    } else {
        if (Sim_10052aaf(index) == GS32(0x1017b8c8) && kind != 0x10)
            G32(0x1015d120 + (uint32_t)index * 4) = 1;
    }
}
