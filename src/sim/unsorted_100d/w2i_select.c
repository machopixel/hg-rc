#include "w2i.h"
#include "../util/access.h"

#define RECORDS(block) U32(block, 0x84)
#define SELECTED(block) U32(block, 0x4fc)
#define RECORD(block, i) (RECORDS(block) + (uint32_t)(i) * 0x128u)
#define TABLE_ENTRY(n) (0x10145508u + (uint32_t)(n) * 0xd6u)

// FUNCTION: HGSIM 0x100d63fa
void __cdecl Sim_100d63fa(void *block)
{
    int i;

    for (i = 0; i < 8; i++) {
        uint32_t rec, info;

        if (G32(0x101853C0u + (uint32_t)i * 4u) == 0)
            continue;
        G32(0x101853C0u + (uint32_t)i * 4u) = 0;
        rec = RECORD(block, i);
        info = TABLE_ENTRY(U32(rec, 4));
        if (S32(rec, 8) == -1 && G32(0x1015EC58) == 0)
            continue;
        U32(rec, 8) = 2;
        U32(rec, 0x2c) = U32(GBYTES(info), 0x2c);
        U32(rec, 0x10) = 0;
        if ((U32(GBYTES(info), 0x86) & 1) != 0 && (U16(block, 0x560) & 0x80) != 0) {
            U32(rec, 0x18) = U32(U32(block, 0), 0xf0) & 0xff;
            U32(rec, 0x1c) = U32(U32(block, 0), 0xf0) & 0xf00;
        } else {
            U32(rec, 0x1c) = 0;
            U32(rec, 0x18) = 0;
        }
    }
}

// FUNCTION: HGSIM 0x100d5f40
void __cdecl Sim_100d5f40(void *block)
{
    int32_t start;
    int32_t slot;
    int found = 0;
    int i;

    if (S32(block, 0x4f8) <= 0)
        return;
    start = S32(RECORD(block, SELECTED(block)), 0xc);
    slot = (int32_t)((uint32_t)start + 1u) % 8;
    while (slot != start && !found) {
        for (i = 0; i < 8 && !found; i++) {
            int32_t n = S32(RECORD(block, i), 4);
            uint32_t rec, info;

            if (n < 0)
                continue;
            info = TABLE_ENTRY(n);
            rec = RECORD(block, i);
            if (S32(rec, 0) != -1 && S32(rec, 0xc) == slot && S32(rec, 8) != -1
                && (U32(GBYTES(info), 8) & 4) != 0) {
                found = 1;
                SELECTED(block) = (uint32_t)i;
            }
        }
        slot = (int32_t)((uint32_t)slot + 1u) % 8;
    }
}

// FUNCTION: HGSIM 0x100d63d4
void __cdecl Sim_100d63d4(void)
{
    Sim_100d5f40((void *)(uintptr_t)U32(PTRTAB(G32(0x1017B8C8)), 0x28));
}

// FUNCTION: HGSIM 0x100d5cc1
void __cdecl Sim_100d5cc1(void *block, int flag)
{
    int found = 0;
    int j;
    int32_t current, slot, t;

    if (S32(block, 0x4f8) <= 0)
        return;
    current = S32(block, 0x4fc);
    slot = S32(RECORD(block, SELECTED(block)), 0xc);
    t = current;
    for (j = 1; j < 8; j++) {
        uint32_t rec, info;

        t = (int32_t)((uint32_t)t + 1u) % 8;
        rec = RECORD(block, t);
        if (S32(rec, 4) < 0)
            continue;
        info = TABLE_ENTRY(U32(rec, 4));
        if (S32(rec, 0) != -1 && S32(rec, 0xc) == slot && S32(rec, 8) != -1 && t != current
            && (U32(GBYTES(info), 8) & 4) != 0) {
            found = 1;
            SELECTED(block) = (uint32_t)t;
            break;
        }
    }
    if (!found && flag != 0)
        Sim_100d5f40(block);
}

// FUNCTION: HGSIM 0x100d5de5
void __cdecl Sim_100d5de5(void *block)
{
    int count = 0;
    int j;

    if (S32(block, 0x4f8) <= 0)
        return;
    for (j = 0; j < 8; j++) {
        int32_t n;
        uint32_t info;

        SELECTED(block) = SELECTED(block) + 1u;
        SELECTED(block) = (uint32_t)((int32_t)SELECTED(block) % 8);
        n = S32(RECORD(block, SELECTED(block)), 4);
        if (n < 0)
            continue;
        if (S32(RECORD(block, SELECTED(block)), 8) == -1)
            continue;
        info = TABLE_ENTRY(n);
        if (G32(0x10179CCC) != 0 && U32(U32(block, 0), 4) == G32(0x1017B8C8))
            return;
        if (S32(RECORD(block, SELECTED(block)), 0) == -1)
            SELECTED(block) = 0;
        if (S32(RECORD(block, SELECTED(block)), 8) != -1 && (U32(GBYTES(info), 8) & 4) != 0)
            return;
        count++;
        if (count == 8)
            return;
    }
}

// FUNCTION: HGSIM 0x100d82f1
void __cdecl Sim_100d82f1(void)
{
}
