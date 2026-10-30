#include "w13.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../unsorted_1001/w2e.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_100b/w10.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100b/w9d.h"
#include "../unsorted_100c/w12.h"
#include "../unsorted_100c/w2l.h"
#include "../unsorted_100c/w5b.h"
#include "../unsorted_100c/w5c.h"
#include "../unsorted_100c/w5y.h"
#include "../unsorted_100c/w6g.h"
#include "../unsorted_100c/w6h.h"
#include "../unsorted_100c/w7p.h"
#include "../unsorted_100c/w8z.h"
#include "../unsorted_100c/w9c.h"

/* The answer of a step that reports 1 (done), -1 (failed) or 0 (going on) into the result word +0x22c. */
static int W13_Result(uint8_t *unit, int r)
{
    if (r == -1) {
        U16(unit, 0x22c) = 0xffff;
        return 1;
    }
    if (r == 1) {
        U16(unit, 0x22c) = 1;
        return 1;
    }
    U16(unit, 0x22c) = 0;
    return 0;
}

/* As W13_Result for steps 22 / 23: 1 also copies the entry +0x23a to +0x22e; any other non-zero answer fails. */
static int W13_EntryResult(uint8_t *unit, int r)
{
    if (r == 0)
        return 0;
    if (r == 1) {
        U16(unit, 0x22c) = 1;
        U32(unit, 0x22e) = U32(unit, 0x23a);
    } else {
        U16(unit, 0x22c) = 0xffff;
    }
    return 1;
}

/* Step 1: move to the formation slot +0x218 of the leader (unit table entry of the word +0x1aa). */
static int W13_SlotMove(uint8_t *unit, int16_t key)
{
    uint8_t *lead;

    if (S32(unit, 0x204) > GS32(0x1015dc78))
        return 1;
    Sim_1001570f(unit);
    if (Sim_1001403a(unit, S16(unit, 0x1aa)) != 1)
        return 1;
    lead = BYTES(PTRTAB(U16(unit, 0x1aa) & 0xff));
    if (S32(unit, 0x218) != 0)
        lead[0x270 + S32(unit, 0x218) / 2] -= 1;
    U32(unit, 0x204) = G32(0x1015dc78) + 0x21fu;
    if (S32(unit, 0x21c) == -1) {
        S32(unit, 0x218) = (int16_t)Sim_100c3d16(unit);
        U32(unit, 0x21c) = 0;
    }
    Sim_100c00c7(unit, U16(unit, 0x1aa), S16(unit, 0x218), 0x466a6000u); /* 15000.0f */
    lead[0x270 + S32(unit, 0x218) / 2] += 1;
    Sim_100c07c5(unit, key);
    return 0;
}

// FUNCTION: HGSIM 0x100be0f0
void __cdecl Sim_100be0f0(uint8_t *unit, int16_t key)
{
    int done = 0;
    uint32_t rec = U32(unit, 0x28);
    int16_t state;

    G32(0x1017e2a8) = 0;
    G32(0x1017e2ac) = 0;
    G32(0x1017e2b0) = 0;
    if (S16(unit, 0x1f8) == -1) {
        U16(unit, 0x1f8) = (uint16_t)Sim_100bf186(unit);
        if (S16(unit, 0x1f8) != -1)
            Sim_100bf28e(unit);
        return;
    }
    switch (S16(unit, 0x1f8)) {
    case 0:
        Sim_100c264a(unit);
        Sim_100c0670(unit, key);
        break;
    case 1:
        done = W13_SlotMove(unit, key);
        break;
    case 2:
        Sim_100c264a(unit);
        done = Sim_100c0965(unit, key) != 0;
        break;
    case 3:
        Sim_100c264a(unit);
        done = Sim_100c0965(unit, key) != 0;
        if (Sim_100c2ae5(unit) == 0)
            FL(U32(unit, 0x54), 0xc) =
                (float)((double)(int32_t)(U32(unit, 0x218) * 0x1c2u) + (double)FL(U32(unit, 0x54), 0xc));
        if (S32(unit, 0x204) <= GS32(0x1015dc78)) {
            U32(unit, 0x218) = 0u - U32(unit, 0x218);
            U32(unit, 0x204) = G32(0x1015dc78) + 0x21fu;
        }
        break;
    case 4:
        done = Sim_100c1c67(unit, key) != 0;
        break;
    case 5:
        if (Sim_100c0dd7(unit, key) != 0 || S32(U32(unit, 0x28), 0x510) <= 0)
            done = 1;
        break;
    case 6:
        done = Sim_100c0a2d(unit, key) != 0;
        break;
    case 7:
        Sim_100c264a(unit);
        done = Sim_100c0b1d(unit, key) != 0;
        break;
    case 8:
        Sim_100c264a(unit);
        done = Sim_100c0ac4(unit, key) != 0;
        break;
    case 9:
        done = Sim_100c0bdd(unit, key) != 0;
        break;
    case 10:
        Sim_100c0fc4(unit, key);
        break;
    case 11:
        done = Sim_100c0d9d(unit, key) != 0;
        break;
    case 12:
        Sim_100c264a(unit);
        done = Sim_100c1049(unit, key) != 0;
        break;
    case 13:
        Sim_100c1169(unit, key);
        break;
    case 14: {
        int r = Sim_100c12d7(unit);

        if (r == -1) {
            U16(unit, 0x22c) = 0xffff;
            U32(unit, 0x22e) = 0xffffffffu;
            U32(unit, 0x23a) = 0xffffffffu;
            done = 1;
        } else if (r == 1) {
            U16(unit, 0x22c) = 1;
            U32(unit, 0x22e) = U32(unit, 0x23a);
            done = 1;
        } else {
            U16(unit, 0x22c) = 0;
        }
        S32(unit, 0x1fc) = Sim_100c4f6c();
        break;
    }
    case 16:
        done = W13_Result(unit, Sim_100c17b7(unit));
        break;
    case 17:
        done = W13_Result(unit, Sim_100c18c8(unit));
        break;
    case 19:
        done = Sim_100c2279(unit, key) != 0;
        break;
    case 20:
        done = Sim_100c1e69(unit, key) != 0;
        break;
    case 21:
        done = Sim_100c209c(unit, key) != 0;
        break;
    case 22:
        done = W13_EntryResult(unit, Sim_100c145b(unit, key));
        break;
    case 23:
        done = W13_EntryResult(unit, Sim_100c1609(unit, key));
        break;
    case 24:
        Sim_100c03d8(unit);
        break;
    case 25:
        Sim_100c0423(unit);
        break;
    case 26:
        done = Sim_100c0471(unit);
        break;
    case 27:
        Sim_100c0558(unit);
        break;
    default:
        break;
    }
    if (S32(unit, 0) == 1 && Sim_100c3eb5(BYTES(rec)) != 0) {
        G32(0x1017e2a8) = 1;
        done = 1;
    }
    if (S32(unit, 0x200) != 0 && S32(unit, 0x200) <= GS32(0x1015dc78)) {
        G32(0x1017e2ac) = 1;
        done = 1;
    }
    if (S32(unit, 0x1fc) != -1) {
        G32(0x1017e2b0) = 1;
        done = 1;
    } else if (S32(unit, 0) == 1 && Sim_1009c929(0x61) < 3) {
        state = S16(unit, 0x1f8);
        if (state != 0x18 && state != 0x19 && state != 0 && state != 0x10 && state != 0x11 && state != 0x1a &&
            state != 0x1b) {
            int32_t pick;

            done = 1;
            pick = Sim_1009c929(0x40) >> 4;
            GS32(0x1017e2a4) = pick;
            S32(unit, 0x1fc) = *SIM_PTR(int16_t, 0x10151b88 + pick * 2);
        }
    }
    if (done)
        Sim_100bfa33(unit);
}
