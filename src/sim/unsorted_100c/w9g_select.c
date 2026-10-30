#include "w9g.h"
#include "../util/access.h"
#include "../unsorted_100c/w7q.h"

// FUNCTION: HGSIM 0x100ca670
void __cdecl Sim_100ca670(void *obj, int32_t delta, uint32_t flags)
{
    int done = 0;
    uint32_t sel;           /* left unset by the original when flags is 1 and +0x118 was -1 */
    uint32_t kind, index;
    int32_t limit, n, r;

    if (S32(obj, 4) == GS32(0x1017b8c8))
        G32(0x1017e5b0) = 0;
    if (flags == 1) {
        if (S32(obj, 0x118) == -1)
            U32(obj, 0x118) = 0x100;
        else
            sel = U32(obj, 0x118);
    } else {
        sel = U32(obj, 0xf0);
    }
    kind = sel & 0xf00u;
    index = sel & 0xffu;
    if (delta == 0 || (sel & 0x1000u) != 0) {
        kind = 0x200;
        sel = kind;
        index = 0;
        delta = 1;
    } else {
        index += (uint32_t)delta;
    }
    limit = (int32_t)(G32(0x1015dc64) + G32(0x1015dc68) + G32(0x1017e5a8) + 3u);
    for (n = 0; n < limit && !done; n++) {
        switch (kind) {
        case 0x100:
            r = Sim_100cab7b(S32(obj, 4), (int)index, flags);
            if (r == -2) {
                index = 0;
                kind = 0x200;
            } else if (r == -1) {
                index = G32(0x1015dc68) - 1u;
                kind = 0x400;
            } else if (r == 1) {
                done = 1;
            } else {
                index += (uint32_t)delta;
            }
            break;
        case 0x200:
            r = W9GC_CALL_100cada2(S32(obj, 4), (int)index, flags);
            if (r == -2) {
                index = 0;
                kind = 0x400;
            } else if (r == -1) {
                index = G32(0x1017e5a8) - 1u;
                kind = 0x100;
            } else if (r == 1) {
                done = 1;
            } else {
                index += (uint32_t)delta;
            }
            break;
        case 0x400:
            r = Sim_100cb299(S32(obj, 4), (int)index, flags);
            if (r == -2) {
                index = 0;
                kind = 0x100;
            } else if (r == -1) {
                index = G32(0x1015dc64) - 1u;
                kind = 0x200;
            } else if (r == 1) {
                done = 1;
            } else {
                index += (uint32_t)delta;
            }
            break;
        default:
            index = 0;
            kind = 0x200;
            break;
        }
    }
    if (done) {
        if (kind != 0x100)
            U32(obj, 0xf0) = kind | index;
        else
            U32(obj, 0x118) = kind | index;
    } else if (kind == 0x100) {
        U32(obj, 0x118) |= 0x1000u;
    }
    if ((U32(obj, 0x118) & 0x100u) == 0 && U32(U32(obj, 0x28), 0x50c) != 0)
        U8(U32(obj, 0x54), 0x46) = 1;
}
