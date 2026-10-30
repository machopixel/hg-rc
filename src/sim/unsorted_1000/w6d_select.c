#include "w6d.h"
#include "fns.h"
#include "w2k.h"
#include "w2l.h"
#include "w3t.h"
#include "w5b.h"
#include "w5z.h"
#include "../unsorted_100d/w2i.h"
#include "../util/util.h"
#include "../util/w3p.h"
#include "../util/access.h"

#define ENTRY_OF_KIND(kind) (0x10145508u + (uint32_t)(kind) * 0xd6u)
#define RECORD_OF(block, i) (U32(block, 0x84) + (uint32_t)(i) * 0x128u)

// FUNCTION: HGSIM 0x1000bba7
int __cdecl Sim_1000bba7(void *obj, uint32_t *flag)
{
    uint32_t block = U32(obj, 0x28);
    int32_t saved = -1;
    int32_t best_score = 0;
    int32_t best = -1;
    uint32_t best_78 = 0;
    int32_t best_20 = 0;
    int sel = -1;
    int kind = 0;
    uint32_t entry = 0;
    uint32_t rec = 0;
    int result;
    int not_ready, own_kind, special, uses_slot, bit_400;
    int i;

    *flag = 0;
    saved = S32(block, 0x4fc);
    not_ready = Sim_1000c5f8((uint32_t)(uintptr_t)obj);
    own_kind = Sim_1000c627(obj);
    special = Sim_1000c813((void *)(uintptr_t)block, saved);
    uses_slot = Sim_1000c680(obj);
    bit_400 = (int)(U32(obj, 0xf0) & 0x400);
    for (i = 0; i < S32(block, 0x4f8); i++) {
        int32_t score = 0;

        if (Sim_1000b9f1((void *)(uintptr_t)block, &sel, &kind, &entry, &rec) != 0) {
            int32_t v20 = S32(rec, 0x20);
            uint32_t v78;

            if (best_20 != 0) {
                if (v20 == -1)
                    score += 2;
                else if (v20 > best_20)
                    score += 2;
            } else {
                score += 1;
            }
            if (not_ready == Sim_1000c749((void *)(uintptr_t)block, sel, 0x40))
                score += 1;
            if (own_kind == Sim_1000c7b4((uint32_t)kind))
                score += 1;
            v78 = U32(entry, 0x78);
            if (best_78 != 0) {
                if (own_kind != 0) {
                    if (v78 < best_78)
                        score += 5;
                } else if (v78 > best_78) {
                    score += 3;
                }
            } else {
                score += 1;
            }
            if (Sim_1000c749((void *)(uintptr_t)block, sel, 1) != 0)
                score += 2;
            if (Sim_1000c813((void *)(uintptr_t)block, sel) == 0) {
                if (special != 0)
                    score += 2;
                if (uses_slot != 0)
                    score += 1;
                if (bit_400 != 0)
                    score += 4;
            }
            if (score >= best_score) {
                best_20 = v20;
                best_score = score;
                best_78 = v78;
                best = sel;
            }
        }
        Sim_100d5cc1((void *)(uintptr_t)block, 1);
        if (S32(block, 0x4fc) == saved)
            break;
    }
    if (best != -1) {
        int32_t best_kind = S32(RECORD_OF(block, best), 4);
        uint32_t value = U32(obj, 0x54);

        if (best_kind == 0x16)
            F32(value, 0) = (float)((double)F32(value, 0) + (double)GF32(0x1011106c));
        else if (best_kind == 0x14)
            F32(value, 0) = (float)((double)F32(value, 0) + (double)GF32(0x10111070));
        else if (best_kind == 0x10)
            F32(value, 0) = (float)((double)F32(value, 0) + (double)GF32(0x10111060));
        /* the entry of the last record Sim_1000b9f1 accepted, which need not be the selected one */
        if ((U32(entry, 0x86) & 1) != 0)
            *flag = 1;
    }
    if (best == -1) {
        S32(block, 0x4fc) = saved;
        result = 0;
    } else {
        S32(block, 0x4fc) = best;
        result = 1;
    }
    if ((U32(GBYTES(ENTRY_OF_KIND(U32(RECORD_OF(block, U32(block, 0x4fc)), 4))), 0x86) & 1) != 0)
        *flag = 1;
    return result;
}

// FUNCTION: HGSIM 0x1000ceb7
int __cdecl Sim_1000ceb7(void *obj)
{
    uint32_t block;
    uint32_t rec;
    int kind;
    uint32_t entry;
    int own_kind;
    int state;

    if (obj == 0)
        return 0;
    block = U32(obj, 0x28);
    rec = RECORD_OF(block, U32(block, 0x4fc));
    kind = S32(rec, 4);
    entry = ENTRY_OF_KIND(kind);
    own_kind = Sim_1000c627(obj);
    state = Sim_1000a9bf(obj, SIM_PTR(uint8_t, entry));
    if (state == 3 || state == 0)
        return 1;
    if (own_kind != Sim_1000c7b4((uint32_t)kind))
        return 1;
    /* the original then compares the entry's dword +0x10 with 0 and with 1 so that it always ends here */
    if (S32(rec, 0x20) <= 0)
        return 1;
    if (Sim_1000ce13(obj, (const void *)(uintptr_t)U32(block, 0x84), (const void *)(uintptr_t)rec,
                     S32(block, 0x4f8)) != 0)
        return 1;
    return 0;
}
