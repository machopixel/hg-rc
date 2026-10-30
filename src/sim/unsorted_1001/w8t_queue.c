#include "w8t.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1000/w8t.h"
#include "../unsorted_1007/w2e.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w2j.h"
#include "../util/access.h"

/* The word of table 0x109f1520 (8 bytes per entry) at byte offset off of entry k. */
#define W8T_SOUND_WORD(k, off) (*SIM_PTR(s16a, 0x109f1520u + (uint32_t)(k) * 8u + (off)))

// FUNCTION: HGSIM 0x1001f790
int __cdecl Sim_1001f790(uint8_t *a, uint8_t *b, int32_t prio)
{
    uint8_t *slot = 0;
    int i;

    if (G32(0x1015d530) != 0)
        return 0;
    if (a == 0)
        return 0;
    if (U32(a, 0x14) != 0)
        U8(a, 0) = 0;
    if (U32(a, 0x10) != 0 && U8(U32(a, 0x10), 0) == 0)
        U32(a, 0x10) = 0;
    if (b != 0) {
        if (U32(b, 0x14) != 0) {
            b = 0;
        } else if (U32(b, 0x10) != 0 && U8(U32(b, 0x10), 0) == 0) {
            U32(b, 0x10) = 0;
        }
    }
    for (i = 0; i < 8 && slot == 0; i++) {
        if (G32(0x1015d1d0 + i * 0x6b) == 0)
            slot = GBYTES(0x1015d1d0 + i * 0x6b);
    }
    if (slot == 0)
        return 0;
    Mem_Fill(slot, 0, 0x6b);
    U32(slot, 0) = 1;
    Str_Copy((char *)slot + 4, (const char *)a);
    U32(slot, 0x24) = U32(a, 0x14);
    if (b != 0) {
        Str_Copy((char *)slot + 0x14, (const char *)b);
        U32(slot, 0x28) = U32(b, 0x14);
    }
    if (U32(a, 0x10) != 0)
        Str_NCat((char *)slot + 0x34, (const char *)(uintptr_t)U32(a, 0x10), 0x33);
    if (b != 0 && U32(b, 0x10) != 0) {
        Str_NCat((char *)slot + 0x34, (const char *)GBYTES(0x10134250), 0x33 - Str_Length((const char *)slot + 0x34));
        Str_NCat((char *)slot + 0x34, (const char *)(uintptr_t)U32(b, 0x10),
                 0x33 - Str_Length((const char *)slot + 0x34));
    }
    if (U8(slot, 0x34) != 0)
        Str_NCat((char *)slot + 0x34, (const char *)GBYTES(0x10134254), 0x33 - Str_Length((const char *)slot + 0x34));
    if (prio == -1 && U8(slot, 4) != 0)
        S32(slot, 0x30) = (int32_t)W8T_SOUND_WORD(Sim_1000df9c((const char *)slot + 4), 0);
    else
        S32(slot, 0x30) = prio;
    if (G32(0x1015d528) != 0) {
        uint8_t *prev = BYTES(G32(0x1015d528));
        uint8_t *cur = BYTES(U32(prev, 0x67));
        int32_t same = 0;

        while (cur != 0) {
            if (U32(cur, 0x24) != 0) {
                if (Str_Compare((const char *)cur + 4, (const char *)slot + 4) == 0)
                    return 0;
            } else {
                if (Str_Compare((const char *)cur + 4, (const char *)slot + 4) == 0 &&
                    Str_Compare((const char *)cur + 0x14, (const char *)slot + 0x14) == 0) {
                    int k = Sim_1000df9c((const char *)slot + 4);

                    if (W8T_SOUND_WORD(k, 2) == -1)
                        return 0;
                    same++;
                    if (!(same < (int32_t)W8T_SOUND_WORD(k, 2)))
                        return 0;
                }
                if (S32(slot, 0x30) > S32(cur, 0x30)) {
                    U32(prev, 0x67) = (uint32_t)(uintptr_t)slot;
                    U32(slot, 0x67) = (uint32_t)(uintptr_t)cur;
                    break;
                }
            }
            prev = cur;
            cur = BYTES(U32(cur, 0x67));
        }
        U32(prev, 0x67) = (uint32_t)(uintptr_t)slot;
    } else if (Sim_1001fc18(slot) >= 0) {
        G32(0x1015d528) = (uint32_t)(uintptr_t)slot;
    } else {
        U32(slot, 0) = 0;
        slot = 0;
    }
    return slot != 0;
}

// FUNCTION: HGSIM 0x1001fc18
int32_t __cdecl Sim_1001fc18(uint8_t *slot)
{
    const char *first, *second;
    int32_t result = -1;

    if (G32(0x1015d0a4) == 0 || (G32(0x101321c0) & 2) == 0) {
        if (U8(slot, 0x34) != 0) {
            Sim_1007df6c((const char *)slot + 0x34, 0, 0x712, 0x32);
            U32(slot, 0x2c) = G32(0x1015dc78) + 0x712;
            return 1;
        }
        return 0;
    }
    first = (const char *)slot + 4;
    second = (const char *)slot + 0x14;
    U32(slot, 0x2c) = 0;
    if (first[0] == 0 && second[0] != 0) {
        first = second;
        second = 0;
    }
    if (first[0] == 0) {
        if (U32(slot, 0x24) == 0)
            return 0;
        first = 0;
    }
    /* second is null here when only it had a name: the original reads through it (and faults) all the same */
    if (*(const volatile char *)second != 0) {
        if (W8T_CALL_100ba7d5(second, G32(0x1014ceb4), 0) != 0) {
            if (W8T_CALL_100ba7d5(first, G32(0x1014ceb4), 0) == 0) {
                Sim_100bb2b0((const uint32_t *)second, (const char *)(uintptr_t)G32(0x1014ceb4));
            } else {
                int k2 = Sim_1000df9c(second);
                int k1 = Sim_1000df9c(first);

                result = Sim_1000e737(0, 0, k1, 0, 0, (int32_t)G32(0x101321b8), 0, 0x5622, k2, 0x150, 1, 0);
            }
        }
    } else if (first != 0) {
        int k = Sim_1000df9c(first);

        result = Sim_1000e737(0, 0, k, 0, 0, (int32_t)G32(0x101321b8), 0, 0x2b11, -1, 0x150, 1, 0);
    } else if (U8(slot, 0x34) != 0) {
        Sim_1007df6c((const char *)slot + 0x34, 0, 0x712, 0x32);
    }
    if (result >= 0)
        G32(0x1015d52c) = 1;
    return result;
}
