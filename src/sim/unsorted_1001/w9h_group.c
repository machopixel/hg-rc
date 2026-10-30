#include "w9h.h"
#include "w2f.h"
#include "w2h.h"
#include "w5b.h"
#include "w5c.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1002/w6d.h"
#include "../unsorted_1004/fns.h"

/* The group entries: 0x10473800 + index * 0x4fc3a is a block of 0x13f-byte entries after a 0x3a-byte header. */
#define W9H_ENTRY(index, sel) \
    (GBYTES(0x10473800) + (uint32_t)(index) * 0x4fc3au + (uint32_t)(sel) * 0x13fu + 0x3au)

// FUNCTION: HGSIM 0x10017263
void __cdecl Sim_10017263(int index)
{
    uint32_t list[8];
    uint8_t *entry;
    uint32_t rec, target, flags, bits;
    int32_t idx, sel, n, i, j, need, f;
    int16_t kind, mode;
    uint16_t word;

    idx = Sim_10052aaf(index);
    if (idx == -1)
        return;
    rec = PTRTAB(idx);
    if (S16(rec, 0x1a0) != 0) {
        Sim_10016d83(index, (const uint16_t *)(BYTES(rec) + 0x1a0));
        U16(rec, 0x1a0) = 0;
    }
    if (idx == GS32(0x1017b8c8))
        return;
    if (S32(rec, 0x14) == 0 || S32(rec, 0x14) == 1)
        return;
    sel = GS32(0x104727a0 + (uint32_t)index * 4);
    if (sel == -1)
        return;
    entry = W9H_ENTRY(index, sel);
    flags = U32(entry, 2);
    if (flags == 0 || (flags & 0xe10) != 0)
        return;
    n = Sim_10016cb7(index, list, 0);
    if (n == 0)
        return;
    f = (int32_t)flags;
    word = 0;
    for (i = 0; i < (int32_t)U8(entry, 0xee) && word == 0; i++) {
        need = S32(entry, 0x79);
        if (need == 0)
            need = n;
        word = U16(entry, 0xef + i * 2);
        if (Sim_100152fc((unsigned)index, word, f) != 0) {
            word = 0;
            continue;
        }
        for (j = 0; j < n && need != 0; j++) {
            rec = list[j];
            if (U16(rec, 0x1aa) != word)
                continue;
            kind = S16(rec, 0x1a6);
            if (f <= 4) {
                if (f == 4) {
                    if (kind == 7)
                        need--;
                } else if (f > 0 && f <= 2) {
                    if (kind == 2 || kind == 3)
                        need--;
                }
            } else if (f <= 0x20) {
                if (f == 0x20) {
                    if (kind == 8)
                        need--;
                } else if (f == 8) {
                    if (kind == 2)
                        need--;
                }
            } else if (f <= 0x200) {
                if ((f == 0x200 || f == 0x100) && kind == 8)
                    need--;
            } else if (f <= 0x1000) {
                if (f == 0x1000)
                    need--;
                else if (f == 0x400 && kind == 8)
                    need--;
            } else if (f == 0x2000) {
                if (kind == 4)
                    need--;
            } else if (f == 0x4000) {
                if (kind == 2 || kind == 3)
                    need--;
            }
        }
        if (need == 0)
            word = 0;
    }
    if (word == 0)
        return;
    target = 0;
    for (i = 0; i < n; i++) {
        rec = list[i];
        if (S16(rec, 0x1a6) != 0) {
            if (S16(rec, 0x1a6) != 5)
                continue;
            if (Sim_100c3668(BYTES(U32(rec, 0x28))) != 0)
                continue;
        }
        if (rec == PTRTAB(idx))
            continue;
        target = rec;
        break;
    }
    if (target == 0) {
        if (S16(PTRTAB(idx), 0x1a6) != 0)
            return;
        if (Sim_100c3668(BYTES(U32(PTRTAB(idx), 0x28))) != 0)
            return;
        target = PTRTAB(idx);
    }
    for (i = 0; i < 16; i++) {
        if ((flags & (1u << i)) != 0) {
            int16_t other = (int16_t)Sim_10052aaf(index);

            Sim_10016a6b(BYTES(target), 2, (int16_t)(i + 1), other);
            break;
        }
    }
    bits = 0x40 | (uint32_t)Sim_10016a4a(S32(entry, 0x7d));
    mode = -1;
    if (f <= 4) {
        if (f == 4)
            mode = 7;
        else if (f > 0 && f <= 2)
            mode = 2;
    } else if (f <= 0x20) {
        if (f == 0x20)
            mode = 8;
        else if (f == 8)
            mode = 2;
    } else if (f <= 0x200) {
        if (f == 0x200 || f == 0x100)
            mode = 8;
    } else if (f <= 0x1000) {
        if (f == 0x1000) {
            mode = 0;
            Sim_10025f6b(BYTES(target));
        } else if (f == 0x400) {
            mode = 8;
        }
    } else if (f == 0x2000) {
        mode = 4;
    } else if (f == 0x4000) {
        mode = 2;
    }
    if (mode != -1) {
        Sim_1001555c(BYTES(target), mode, (int16_t)word);
        U32(target, 0x1ae) |= bits;
    }
}
