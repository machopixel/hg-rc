#include "w9g.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100f/w2g.h"

/* A member may take the key: not in state 2 or 3, Sim_100c3668 says 0 on its block, and dword +0x1b2 is set. */
static int Eligible(uint8_t *m)
{
    if (S16(m, 0x1a6) == 2 || S16(m, 0x1a6) == 3)
        return 0;
    if (Sim_100c3668((void *)(uintptr_t)U32(m, 0x28)) != 0)
        return 0;
    return U32(m, 0x1b2) != 0;
}

// FUNCTION: HGSIM 0x10016de7
int __cdecl Sim_10016de7(int kind, uint32_t key)
{
    uint32_t members[9];
    uint8_t *group;
    uint8_t *m;
    int32_t idx, n, i, bestId, bestDist;
    uint32_t ref = 0;

    if ((key & 0xffffu & 0xf000u) != 0)
        return 0;
    idx = Sim_10052aaf(kind);
    if (idx == -1)
        return 0;
    n = Sim_10016cb7(kind, members, 1);
    if (n == 0)
        return 0;
    if ((key & 0xffffu & 0xf00u) == 0x200u)
        ref = U16(PTRTAB(key & 0xffffu & 0xffu), 0x1ac);
    if (S32(GBYTES(0x10473800) + (uint32_t)kind * 0x4fc3au + GS32(0x104727a0 + (uint32_t)kind * 4u) * 0x13fu,
            0x3c) == 4) {
        int found = 0;
        uint8_t *entry;
        int32_t limit;

        group = GBYTES(0x109b7d40) + (uint32_t)kind * 0x6cu;
        entry = GBYTES(0x10473800) + (uint32_t)kind * 0x4fc3au + GS32(0x104727a0 + (uint32_t)kind * 4u) * 0x13fu +
                0x3a;
        limit = S32(entry, 0x79);
        if (limit == 0)
            limit = n;
        for (i = 0; i < n && limit != 0; i++) {
            m = BYTES(members[i]);
            if (Eligible(m)) {
                Sim_10015630(m, 2, (int16_t)key, 1);
                found = 1;
            }
        }
        if (U32(group, 8) != 0) {
            m = BYTES(members[4]);
            if (Eligible(m)) {
                Sim_10015630(m, 2, (int16_t)key, 1);
                found = 1;
            }
        }
        return found;
    }
    group = GBYTES(0x109b7d40) + (uint32_t)kind * 0x6cu;
    bestId = -1;
    bestDist = 0x7fff;
    for (i = 0; i < n; i++) {
        int32_t d;

        m = BYTES(members[i]);
        if (S32(m, 4) == idx || (U32(m, 0x1ae) & 3) != 0 || !Eligible(m))
            continue;
        d = (int32_t)((uint32_t)U16(m, 0x1ac) - ref);
        if (Math_AbsInt_100fd100(d) < bestDist) {
            bestDist = Math_AbsInt_100fd100(d);
            bestId = S32(m, 4);
        }
    }
    if (U32(group, 8) != 0) {
        m = BYTES(members[4]);
        if (S32(m, 4) != idx && (U32(m, 0x1ae) & 3) == 0 && Eligible(m)) {
            int32_t d = (int32_t)((uint32_t)U16(m, 0x1ac) - ref);

            if (Math_AbsInt_100fd100(d) < bestDist) {
                bestDist = Math_AbsInt_100fd100(d);
                bestId = S32(m, 4);
            }
        }
    }
    if (bestId != -1) {
        m = BYTES(PTRTAB(bestId));
    } else {
        m = BYTES(PTRTAB(idx));
        if ((U32(m, 0x1ae) & 3) != 0 || !Eligible(m))
            m = 0;
    }
    if (m == 0)
        return 0;
    Sim_10015630(m, 2, (int16_t)key, 1);
    U32(m, 0x1ae) |= 0x20;
    U32(m, 0x1ae) &= ~0x40u;
    return 1;
}
