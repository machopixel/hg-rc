#include "w8t.h"
#include "fns.h"
#include "w2e.h"
#include "w2f.h"
#include "w2g.h"
#include "w2h.h"
#include "w2k.h"
#include "w3t.h"
#include "w5a.h"
#include "w5c.h"
#include "w5x.h"
#include "w5y.h"
#include "w6d.h"
#include "w7l.h"
#include "../unsorted_1000/w3t.h"
#include "../unsorted_1002/w2l.h"
#include "../unsorted_1002/w8t.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_100b/w2e.h"
#include "../unsorted_100c/w2e.h"
#include "../util/util.h"
#include "../util/w3o.h"

#define W8T_STATE_FN(i) (SIM_PTR(W8tStateFn, 0x10130e58)[(i)])
#define W8T_KIND_FN(i) (SIM_PTR(W8tKindFn, 0x10130e98)[(i)])
#define W8T_ENTRY_FN(i) (SIM_PTR(W8tEntryFn, 0x10130eb8)[(i)])

/* The per-group block: 0x4fc3a bytes per group from 0x10473800, its current entry [0x104727a0 + 4 * group]. */
static uint8_t *W8t_GroupBlock(uint32_t group, uint32_t extra)
{
    return BYTES((uint32_t)(uintptr_t)GBYTES(0x10473800) + group * 0x4fc3au + extra);
}

// FUNCTION: HGSIM 0x10011093
void __cdecl Sim_10011093(void *obj)
{
    uint32_t index;
    uint8_t *player;

    U32(obj, 0x170) = U32(obj, 4) * 0x18u + (uint32_t)(uintptr_t)GBYTES(0x10a38a80);
    U32(obj, 0x19c) = 0;
    U16(obj, 0x1a6) = 0;
    U32(obj, 0x1d6) = 0x1000;
    U32(obj, 0x1c2) = 0;
    U32(obj, 0x24a) = 0;
    U32(obj, 0x23e) = 0;
    U32(obj, 0x242) = 0;
    FL(obj, 0x246) = (float)W8T_CALL_10010fbf(obj);
    Sim_1001150d(obj);
    index = (uint32_t)Sim_10052aaf((int)U32(obj, 8));
    if (U32(PTRTAB(index), 0x14) == 0 || U32(PTRTAB(index), 0x14) == 1)
        U32(obj, 0x1b2) = 0;
    else
        U32(obj, 0x1b2) = 1;
    if (U32(obj, 0x14) != 2 && U32(obj, 0x14) != 3)
        return;
    Sim_10016a6b(obj, 0, 0, (int16_t)Sim_10052aaf((int)U32(obj, 8)));
    player = BYTES(PTRTAB(G32(0x1017b8c8)));
    U8(obj, 0x1b6) = U8(obj, 0x188);
    if ((int8_t)U8(obj, 0x1b6) == 0)
        U8(obj, 0x1b6) = 1;
    U32(obj, 0x1d2) = U16(obj, 0x18a);
    if (U32(obj, 0x1d2) == 0)
        U32(obj, 0x1d2) = 0xfa;
    U32(obj, 0x1ca) = U16(obj, 0x18c);
    if (U32(obj, 0x1ca) == 0)
        U32(obj, 0x1ca) = 0xfa;
    U32(obj, 0x1ce) = U16(obj, 0x18e);
    if (U32(obj, 0x1ce) == 0)
        U32(obj, 0x1ce) = 0xfa;
    U8(obj, 0x1b7) = U8(obj, 0x190);
    if ((int8_t)U8(obj, 0x1b7) == 0)
        U8(obj, 0x1b7) = 1;
    U8(obj, 0x1b8) = U8(obj, 0x192);
    if ((int8_t)U8(obj, 0x1b8) == 0)
        U8(obj, 0x1b8) = 0;
    U8(obj, 0x1b9) = U8(obj, 0x194);
    if ((int8_t)U8(obj, 0x1b9) == 0)
        U8(obj, 0x1b9) = 5;
    U32(obj, 0x1ba) = U16(obj, 0x196);
    if (U32(obj, 8) == U32(player, 8))
        Sim_10020456((int)U32(obj, 0x10), (int)U32(obj, 0x1ba));
    Sim_1001139e(obj);
    U8(obj, 0x1b6) += 1;
    U32(obj, 0x1d2) *= 0x64u;
    U32(obj, 0x1ca) *= 0x64u;
    U32(obj, 0x1ce) *= 0x64u;
    Sim_100bea42(obj);
}

// FUNCTION: HGSIM 0x10011681
void __cdecl Sim_10011681(void *obj)
{
    int flag;

    if (!(GS32(0x1015dc78) > S32(obj, 0x19c)))
        return;
    U32(obj, 0x19c) = G32(0x1015dc78);
    if (U32(obj, 4) == 0)
        *SIM_PTR(u16a, 0x10a38a74) = 0;
    if (S16(obj, 0x1a6) == 0xc)
        return;
    if (Sim_10017a05(obj) != 0)
        return;
    if (U32(obj, 4) == (uint32_t)Sim_10052aaf((int)U32(obj, 8)))
        W8T_CALL_10017263(U32(obj, 8));
    if (U32(obj, 0x14) == 2 || (U32(obj, 0x14) == 3 && U32(obj, 0xc) == G32(0x1017b8c8))) {
        if (Sim_100117f8(obj) == 0)
            W8T_STATE_FN(S16(obj, 0x1a6))(obj, U16(obj, 0x1a8));
        Sim_1001403a(obj, (short)U16(obj, 0x1aa));
        Sim_100125a3(obj);
        return;
    }
    if (U32(U32(obj, 0x28), 0x4f4) != 0 && S32(obj, 0x84) != -1 && Sim_10052be7(S32(obj, 0x84)) == 1)
        Sim_10015afd((int)U32(obj, 4), U32(PTRTAB(U32(obj, 0x84)), 4));
    if (G32(0x1015d114) != 0)
        Sim_1000bf73(obj, &flag);
}

// FUNCTION: HGSIM 0x100117f8
int __cdecl Sim_100117f8(void *obj)
{
    int16_t index = 0;
    int matched = 0;

    Sim_100123bf(obj);
    for (;;) {
        uint8_t *entry = BYTES(U32(U32(obj, 0x170), index * 4));
        int16_t first, second;

        if (entry == 0)
            return 0;
        index = (int16_t)(index + 1);
        first = (int16_t)Sim_10013fe1(U16(entry, 2), U16(obj, 0x1aa), 0, U16(obj, 0x1a8));
        second = (int16_t)Sim_10013fe1(U16(entry, 4), U16(obj, 0x1aa), 0, U16(obj, 0x1a8));
        if (S16(entry, 0) == S16(obj, 0x1a0) && (int32_t)first == (int32_t)U16(obj, 0x1a2)) {
            matched = 1;
            U16(obj, 0x1a0) = 0;
        }
        if (S16(entry, 0) < 8)
            first = (int16_t)W8T_KIND_FN(S16(entry, 0))(obj, (uint16_t)first, (uint16_t)second);
        if (first != 0 || matched != 0) {
            if (W8T_ENTRY_FN(U8(entry, 6))(obj, entry) != 0) {
                int16_t key = (int16_t)Sim_10013fe1(U16(entry, 9), U16(obj, 0x1aa), (uint16_t)first, U16(obj, 0x1a8));

                Sim_10015630(obj, S16(entry, 7), key, 0);
                return 1;
            }
        }
    }
}

// FUNCTION: HGSIM 0x100123bf
void __cdecl Sim_100123bf(void *obj)
{
    uint32_t members[8];
    int16_t key;
    int count, i;

    if ((U32(obj, 0x1ae) & 1) != 0)
        return;
    key = (int16_t)Sim_10013f2a(obj);
    if (key == -1)
        return;
    if ((uint32_t)U16(obj, 0x1aa) == U32(obj, 0x1d6) && S16(obj, 0x1a6) == 8)
        return;
    if ((S16(obj, 0x1a6) == 2 || S16(obj, 0x1a6) == 3) && (int32_t)U16(obj, 0x1aa) == (int32_t)key)
        return;
    if (U32(obj, 0x1dc) != 0) {
        int16_t other, near;
        uint8_t *partner = Sim_100135d4(obj, key, -3, &other, &near);

        if (partner != 0) {
            if ((U32(partner, 0x1ae) & 0x40) != 0)
                key = -1;
            else
                Sim_10015630(partner, 0, 0, 0);
        }
    }
    if (key == -1)
        return;
    Sim_10015630(obj, 2, key, 1);
    count = Sim_10016cb7((int)U32(obj, 8), members, 1);
    for (i = 0; i < count; i++) {
        uint8_t *m = BYTES(members[i]);

        if (m == 0)
            continue;
        if (U32(m, 4) == G32(0x1017b8c8) || U32(m, 4) == U32(obj, 4))
            continue;
        if (S16(m, 0x1a6) == 2 || S16(m, 0x1a6) == 3)
            continue;
        if ((int32_t)U16(m, 0x1aa) == (int32_t)key)
            continue;
        Sim_10015630(m, 2, key, 1);
    }
}

// FUNCTION: HGSIM 0x1001305e
uint16_t __cdecl Sim_1001305e(void *obj, int16_t key, int16_t sel)
{
    int16_t result;

    if (sel != 0)
        return Sim_10013297(obj, key, sel);
    result = (int16_t)Sim_10013297(obj, key,
                                   (int16_t)Render_Ftol((double)Sim_10014061(key) / (double)GFL(0x10111108)));
    if (result == 0 || (key & 0x100) == 0)
        return (uint16_t)result;
    if ((int32_t)key == S32(obj, 0x1d6))
        Sim_10017852(obj);
    if (S16(obj, 0x1a6) != 7) {
        Sim_100159cf(obj, key);
        return (uint16_t)result;
    }
    if ((U16(obj, 0x1aa) & 0x400) != 0 || (U16(obj, 0x1aa) & 0x100) != 0) {
        Sim_100159cf(obj, key);
        if ((key & 0x100) != 0)
            U16(obj, 0x1a8) = U16(obj, 0x118);
        else
            U16(obj, 0x1a8) = U16(obj, 0xf0);
        if ((U16(obj, 0x1a8) & 0x1000) != 0) {
            Sim_100159cf(obj, 0x100);
            U16(obj, 0x1a8) = U16(obj, 0x118);
        }
    } else if ((U16(obj, 0x1aa) & 0x200) != 0) {
        int32_t i;

        U16(obj, 0x1a8) = U16(obj, 0x1aa);
        Sim_1001570f(obj);
        Sim_100157ca(obj);
        U32(obj, 0x21c) += 1;
        S32(obj, 0x21c) = S32(obj, 0x21c) % 4;
        for (i = 0; i < S32(obj, 0x21c); i++)
            Sim_100159cf(obj, (int16_t)U16(obj, 0xf0));
    }
    return 0;
}

// FUNCTION: HGSIM 0x10013297
uint16_t __cdecl Sim_10013297(void *obj, int16_t key, int16_t sel)
{
    int16_t result = 0, best = -1, cur = -1;
    int32_t limit = sel == -3 ? 100000000 : (int32_t)sel * 100;
    int32_t dist = limit;

    for (;;) {
        cur = Sim_10015018(obj, key, cur);
        if (cur == -1)
            break;
        if (Sim_100152fc(U32(obj, 8), (uint16_t)cur, 2) == 0 && Sim_1001403a(obj, cur) != 0 &&
            dist > Render_Ftol(FL(obj, 0xd4))) {
            dist = Render_Ftol(FL(obj, 0xd4));
            best = cur;
        }
    }
    if (dist < limit && best != -1)
        result = best;
    return (uint16_t)result;
}

// FUNCTION: HGSIM 0x10013372
uint16_t __cdecl Sim_10013372(void *obj, int16_t key, int16_t sel)
{
    int16_t result = 0, best = -1, cur = -1;
    int32_t limit, dist;

    if (sel == -3)
        limit = 0;
    else if (sel == -4)
        limit = S32(obj, 0x1d2);
    else if (sel == -2)
        limit = S32(obj, 0x1ce);
    else if (sel == 0)
        limit = Render_Ftol((double)Sim_10014061(key));
    else
        limit = (int32_t)sel * 100;
    dist = limit;
    for (;;) {
        cur = Sim_10015018(obj, key, cur);
        if (cur == -1)
            break;
        if (Sim_100152fc(U32(obj, 8), (uint16_t)cur, 2) == 0 && Sim_1001403a(obj, cur) != 0 &&
            dist < Render_Ftol(FL(obj, 0xd8))) {
            dist = Render_Ftol(FL(obj, 0xd8));
            best = cur;
        }
    }
    if (dist > limit && best != -1)
        result = best;
    return (uint16_t)result;
}

// FUNCTION: HGSIM 0x100135d4
void *__cdecl Sim_100135d4(void *obj, int16_t key, int16_t sel, int16_t *other, int16_t *near)
{
    uint32_t members[8];
    uint8_t *found = 0;
    int32_t best = 100000000;
    int16_t cur = -1;
    int count;
    int32_t entry;
    uint16_t kind;

    *near = 0;
    *other = 0;
    count = Sim_10016cb7((int)U32(obj, 8), members, 1);
    if (count == 0)
        return 0;
    entry = GS32(0x104727a0 + U32(obj, 8) * 4u);
    if (entry == -1)
        return 0;
    kind = U16(W8t_GroupBlock(U32(obj, 8), (uint32_t)entry * 0x13fu), 0x3c);
    for (;;) {
        int pick;

        cur = Sim_10015018(obj, key, cur);
        if (cur == -1)
            break;
        pick = 0;
        if (Sim_1001403a(obj, cur) == 0)
            continue;
        if (Render_Ftol(FL(obj, 0xd4)) < best) {
            best = Render_Ftol(FL(obj, 0xd4));
            *near = cur;
            pick = 1;
            if ((cur & 0x200) == 0)
                *other = cur;
        }
        if ((cur & 0x200) != 0) {
            int16_t j;

            for (j = 0; j < count; j = (int16_t)(j + 1)) {
                uint8_t *m = BYTES(members[j]);

                if (U32(obj, 4) == U32(m, 4))
                    continue;
                if ((S16(m, 0x1c) & 0x16) != 0)
                    continue;
                if ((U16(m, 0x1aa) & 0xf000) != 0)
                    continue;
                if ((int32_t)U16(m, 0x1aa) == (int32_t)cur && (S16(m, 0x1a6) == 3 || S16(m, 0x1a6) == 2) &&
                    pick != 0 && kind != 4) {
                    found = m;
                    pick = 0;
                    break;
                }
            }
            if (pick != 0)
                *other = cur;
        }
    }
    if (*other != 0 && (int16_t)Sim_10013297(obj, *other, sel) == 0)
        *other = 0;
    if (*near != 0 && (int16_t)Sim_10013297(obj, *near, sel) == 0)
        *near = 0;
    return found;
}

// FUNCTION: HGSIM 0x10013848
uint16_t __cdecl Sim_10013848(void *obj, int16_t key, int16_t sel)
{
    int16_t other, near;

    if ((U32(obj, 0x1ae) & 3) != 0)
        return 0;
    if (sel == -2)
        sel = (int16_t)(S32(obj, 0x1ce) / 100);
    else if (sel == -1)
        sel = (int16_t)(S32(obj, 0x1ca) / 100);
    else if (sel == 0)
        sel = (int16_t)Render_Ftol((double)Sim_10014061(key) / (double)GFL(0x10111108));
    Sim_100135d4(obj, key, sel, &other, &near);
    if (U32(obj, 0x1dc) != 0)
        return (uint16_t)other;
    return (uint16_t)near;
}

// FUNCTION: HGSIM 0x1001457f
void __cdecl Sim_1001457f(void *obj)
{
    int16_t keep = 0;

    U8(U32(obj, 0x54), 0x26) = 0;
    U8(U32(obj, 0x54), 0x28) = 0;
    switch (S16(obj, 0x1a6)) {
    case 8: {
        uint32_t group = U32(obj, 8);
        uint8_t *rec = W8t_GroupBlock(group, G32(0x104727a0 + group * 4u) * 0x13fu + 0x3a);

        if (S32(rec, 2) == 0x100 && S32(rec, 0x6f) > 0 && S32(obj, 0) == 1) {
            U8(U32(obj, 0x54), 0x56) = 0;
            U32(obj, 0x224) = 0;
            U32(obj, 0x228) = 0;
        }
        break;
    }
    case 7:
        if (Sim_100152fc(U32(obj, 8), U16(obj, 0x1aa), 4) != 0 && (U32(obj, 0x1ae) & 0x10) != 0)
            Sim_10020256(7, (int32_t)U32(obj, 0x10));
        Sim_1001570f(obj);
        break;
    case 5:
        Sim_1001570f(obj);
        break;
    case 3:
        W8T_CALL_100bfa33(obj);
        U32(U32(obj, 0x54), 0xc) = 0;
        U32(U32(obj, 0x54), 0) = 0;
        U32(U32(obj, 0x54), 4) = 0;
        /* fall through: the original continues with the code of state 6 */
    case 6:
        if (Sim_100152fc(U32(obj, 8), U16(obj, 0x1aa), 8) != 0 && (U32(obj, 0x1ae) & 0x10) != 0)
            Sim_10020256(7, (int32_t)U32(obj, 0x10));
        break;
    case 2:
        if (Sim_100152fc(U32(obj, 8), U16(obj, 0x1aa), 2) == 0)
            keep = (int16_t)U16(obj, 0x1ae);
        else if ((U32(obj, 0x1ae) & 0x10) != 0)
            Sim_10020256(7, (int32_t)U32(obj, 0x10));
        break;
    case 13:
        Sim_100ca417(U32(obj, 4), U16(obj, 0x1aa));
        break;
    case 15:
        if ((U16(obj, 0x1a8) & 0x100) != 0) {
            Sim_100ca417(U32(obj, 4), U16(obj, 0x1a8));
            U16(obj, 0x1aa) = 0;
            U16(obj, 0x1a8) = 0;
        }
        U32(obj, 0x228) = 0;
        break;
    default:
        break;
    }
    U32(obj, 0x1ae) &= ~0x10u;
    U32(obj, 0x1ae) |= (uint32_t)(int32_t)keep;
}

// FUNCTION: HGSIM 0x10015630
void __cdecl Sim_10015630(void *obj, int16_t sel, int16_t key, int flag)
{
    if (sel == -1)
        return;
    if (Sim_100155b7(obj, sel) == 0)
        return;
    if (flag != 0)
        Sim_10013e3c(obj, 0);
    else
        Sim_10013e20(obj, 0);
    Sim_1001457f(obj);
    if (sel == 5 && key == 0x2201 && U32(obj, 4) == (uint32_t)Sim_10052aaf((int)U32(obj, 8))) {
        sel = 0;
        key = 0;
    }
    U16(obj, 0x1a8) = (uint16_t)Sim_10012a72(obj, key);
    U16(obj, 0x1aa) = U16(obj, 0x1a8);
    W8T_CALL_1001484e(obj, (uint16_t)sel);
    Sim_100119a7(obj);
}

// FUNCTION: HGSIM 0x10015d30
uint16_t __cdecl Sim_10015d30(void *obj, int16_t key, int16_t sel, int16_t other)
{
    int16_t cur = -1;
    int16_t skipped = 0;
    int is4200 = key == 0x4200;
    int allowed = 1;

    for (;;) {
        uint8_t *t;
        int kind4;

        cur = Sim_10015018(obj, key, cur);
        if (cur == -1)
            break;
        if ((cur & 0x200) == 0)
            continue;
        if (other == cur) {
            skipped = (int16_t)(skipped + 1);
            continue;
        }
        t = BYTES(PTRTAB(cur & 0xff));
        kind4 = U32(t, 0x10) == 4;
        if (sel == 2 || sel == 5) {
            if (is4200 != 0 && kind4 != 0)
                allowed = 0;
        } else if (kind4 != 0) {
            allowed = 0;
        }
        if (U32(t, 4) == G32(0x1017b8c8) || allowed == 0)
            continue;
        if (S16(t, 0x1a6) == 0xc || Sim_100155b7(t, sel) == 0) {
            skipped = (int16_t)(skipped + 1);
            continue;
        }
        if (sel == 3) {
            U32(t, 0x1b2) = U32(t, 0x1b2) == 0;
            U32(t, 0x1ae) &= ~3u;
            continue;
        }
        U32(t, 0x1ae) = 0;
        if (sel == 7)
            Sim_10015630(t, sel, other, 0);
        else
            Sim_10015630(t, sel, other, 1);
        if (sel == 2) {
            U32(t, 0x1ae) |= 0x11;
        } else {
            if (sel == 5) {
                U32(t, 0x1b2) = 0;
                U32(t, 0x1ae) |= 0x11;
            }
            U32(t, 0x1ae) |= 0x12;
        }
    }
    return (uint16_t)skipped;
}
