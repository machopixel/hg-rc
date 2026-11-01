#include "../fx/w32.h"
#include "w10.h"
#include "w3o.h"
#include "w6e.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w7o.h"
#include "../unsorted_100a/w8y.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1009/w8x.h"
#include "w8w.h"
#include "w9b.h"
#include "w9c.h"
#include "../unsorted_100f/w8z.h"
#include "../render/w2n.h"
#include "../render/w31.h"
#include "../unsorted_1000/w8t.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1009/w6f.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100a/w5a.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1003/w6e.h"
#include "../unsorted_1009/w2j.h"

typedef int(__cdecl *W10Fn100687a3)(uint32_t box, float x, float y, float z, uint32_t radius, float *outX,
                                    float *outY, float *outZ);

// FUNCTION: HGSIM 0x100688a3
int __cdecl Sim_100688a3(const uint8_t *obj, const uint32_t *ent, float x, float y, float z, float *outX,
                         float *outY, float *outZ)
{
    float p[3], q[3];
    union {
        float f;
        uint32_t u;
    } r[3];
    uint32_t node;

    if (ent == 0 || obj == 0)
        return 0;
    node = U32(*ent, 0x48);
    Sim_100a9cfb((void *)(uintptr_t)node, (uint32_t *)&p[0], (uint32_t *)&p[1], (uint32_t *)&p[2]);
    Sim_100a9ca3((void *)(uintptr_t)node, (uint32_t)(uintptr_t)&q[0], (uint32_t)(uintptr_t)&q[1],
                 (uint32_t)(uintptr_t)&q[2]);
    Sim_10068e1d(x, y, z, -p[0], -p[1], -p[2], &r[0].f, &r[1].f, &r[2].f);
    Sim_10068d76(r[0].f, r[1].f, r[2].f, -q[0], -q[1], -q[2], &r[0].u, &r[1].u, &r[2].u);
    return SIM_FN(W10Fn100687a3, 0x100687a3)(*ent + 0x284, r[0].f, r[1].f, r[2].f, U32(obj, 0x53c), outX, outY,
                                             outZ);
}


/* the unit counts as here: the own unit, a single player game, or the own player's kind-3 unit */
static int W10_6Local(uint32_t unit)
{
    return U32(unit, 4) == G32(0x1017b8c8) || G32(0x1015ec58) == 0 ||
           (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8));
}

/* hides the part on the unit's node: Sim_100ab054 for a hard hit (no flag, not a kind-9 unit), else Sim_100aafe4 */
static void W10_6Hide(uint32_t unit, int32_t part, int flag, int any_kind)
{
    if (flag == 0 && (any_kind || U32(unit, 0) != 9))
        Sim_100ab054((void *)(uintptr_t)U32(unit, 0x48), (uint32_t)part);
    else
        Sim_100aafe4((void *)(uintptr_t)U32(unit, 0x48), (uint32_t)part);
}

// FUNCTION: HGSIM 0x1006ad98
void __cdecl Sim_1006ad98(int32_t killer, uint8_t *b, int32_t part, int32_t flag)
{
    uint32_t unit = U32(b, 0);
    int mine = U32(unit, 4) == G32(0x1017b8c8);
    uint8_t *pp;
    float x, y, z;
    int i;

    if (part == 0)
        return;
    /* 1.2 (0x1005ab68, round 32): in a network game, the local player destroying part 2 of a kind-4 unit sets off 16
       shockwaves at the unit's two nodes and scores it, without marking the part */
    if (g_w31Tex && U32(unit, 0) == 4 && G32(0x1015ec58) != 0 && part == 2 && (uint32_t)killer == G32(0x1017b8c8)) {
        Sim_100a9cfb((void *)(uintptr_t)U32(b, 0x4c0), (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
        for (i = 0; i < 6; i++)
            Sim_10092f9d(x, y, z, 0.0f, 0.0f, 0.0f, S32(b, 0xbdb));
        Sim_100a9cfb((void *)(uintptr_t)U32(b, 0x4c4), (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
#ifdef W10N_MUTATE /* the checker's negative check only: one shockwave fewer */
        for (i = 0; i < 9; i++)
#else
        for (i = 0; i < 10; i++)
#endif
            Sim_10092f9d(x, y, z, 0.0f, 0.0f, 0.0f, S32(b, 0xbdb));
        Score_RecordKill(killer, b);
        return;
    }
    pp = BYTES(U32(b, 0x88) + (uint32_t)part * 0x3cu - 0x3cu);
    if ((S16(pp, 0x36) & 0x2000) != 0)
        return;
    U32(pp, 8) = 0;
    U32(pp, 0) = 0;
    U16(pp, 0x36) |= 0x2000;
    switch (part) {
    case 1:
        if (U32(unit, 0) == 1 || U32(unit, 0) == 9) {
            if (W10_6Local(unit)) {
                Score_RecordKill(killer, b);
            } else {
                U32(pp, 8) = 0x3f800000;
                U16(pp, 0x36) &= 0xdfff;
            }
        } else {
            W10_6Hide(unit, part, flag, 1);
        }
        return;
    case 2:
    case 7:
        Sim_1006ad98(killer, b, 3, 0);
        Sim_1006ad98(killer, b, 4, 0);
        Sim_1006ad98(killer, b, 1, 0);
        if (U32(unit, 0) != 1 && U32(unit, 0) != 9 && W10_6Local(unit))
            Score_RecordKill(killer, b);
        Sim_100a9cfb((void *)(uintptr_t)U32(b, 0x4c0), (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
        for (i = 0; i < 6; i++)
            Sim_10092f9d(x, y, z, 0.0f, 0.0f, 0.0f, S32(b, 0xbdb));
        Sim_100a9cfb((void *)(uintptr_t)U32(b, 0x4c4), (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
        for (i = 0; i < 10; i++)
            Sim_10092f9d(x, y, z, 0.0f, 0.0f, 0.0f, S32(b, 0xbdb));
        break;
    case 3:
    case 4:
        W10_6Hide(unit, part, flag, 0);
        return;
    case 5:
    case 6:
        if ((U16(b, 0x560) & 0x20) != 0) {
            if (U32(unit, 0) == 1)
                Sim_1006abf6((uint32_t)killer, b, 2);
            return;
        }
        W10_6Hide(unit, part, flag, 0);
        U32(b, 0x500) = 0;
        U16(b, 0x560) |= 0x20;
        if (U32(unit, 0) == 1 || U32(unit, 0) == 9) {
            if (Sim_1009c929(2) != 0) {
                U32(b, 0xc19) = 1;
                U32(b, 0xc1d) = 0;
                U16(b, 0x562) |= 0x400;
            } else {
                U32(b, 0xc19) = 2;
                U32(b, 0xc1d) = 0;
                U16(b, 0x562) |= 4;
            }
            *(float *)(b + 0x570) = (float)((double)(int32_t)Sim_1009c969() * *SIM_PTR(float, 0x1012b670) /
                                            *SIM_PTR(float, 0x1012b674));
        }
        if (U32(unit, 0) == 1) {
            U32(b, 0xc39) += 0xe24;
            U16(b, 0x562) |= part == 5 ? 0x4000 : 0x2000;
        }
        if (mine && S32(b, 0x510) >= 0)
            Sim_100203cf(0x2b, 3);
        U32(b, 0x510) = 0xffffffffu;
        U32(b, 0x514) = 0;
        U32(b, 0x540) = 0;
        U32(b, 0x518) = 0;
        return;
    case 9:
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
        W10_6Hide(unit, part, flag, 1);
        return;
    default:     /* 8 and any other part */
        break;
    }
    W10_6Hide(unit, part, flag, 0);
    if (flag == 0 && U32(b, 0x4f0) != 4 && U32(b, 0x4f0) != 5 && W10_6Local(unit))
        Score_RecordKill(killer, b);
}

typedef void(__cdecl *W10_6SlotFn)(uint32_t flags, uint32_t on);

#define W10_6CELL(b, part, k) U32(b, 0x617u + (uint32_t)(part) * 0x50u + (uint32_t)(k) * 4u)

/* the throttle factor +0x500 scaled, a negative result (or -0) becoming 0 */
static void W10_6Scale500(uint8_t *b, uint32_t factor_addr)
{
    *(float *)(b + 0x500) = (float)((double)*(float *)(b + 0x500) * *SIM_PTR(float, factor_addr));
    if ((U32(b, 0x500) & 0x80000000u) != 0)
        U32(b, 0x500) = 0;
}

// FUNCTION: HGSIM 0x1006b52f
void __cdecl Sim_1006b52f(int32_t killer, uint8_t *b, int32_t part, int32_t k, int32_t flag)
{
    uint32_t unit = U32(b, 0);
    int mine = U32(unit, 4) == G32(0x1017b8c8);
    int act;        /* the own unit, not a quiet hit: the cockpit message */
    int32_t code;

    if (U32(b, 0x88) + (uint32_t)part * 0x3cu - 0x3cu == 0)
        return;
    if (U32(unit, 0) != 1)
        return;
    if (mine)
        *SIM_PTR(u16a, 0x10165516) += 1;
    code = (int32_t)W10_6CELL(b, part, k);
    act = flag == 0 && mine;
    switch (code) {
    case 0:
        if (k < S32(b, 0xb67u + (uint32_t)part * 4u))
            Sim_1006b52f(killer, b, part, k + 1, 0);
        break;
    case 0x1bbc:
        W10_6CELL(b, part, k) = 0;
        break;
    case 0x1b8a:
        if (mine)
            *SIM_PTR(u16a, 0x10165518) += 1;
        if (mine || G32(0x1015ec58) == 0 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))
            Sim_1006abf6((uint32_t)killer, b, part);
        break;
    case 0x1bee:
        if (act && (U32(b, 0xbd3) & 0x200) == 0 && (U32(b, 0xbd3) & 0x400) == 0 && U32(b, 0x4f0) == 2) {
            U32(b, 0xc4d) = 1;
            if (U32(b, 0xbaf) != 0) {
                Sim_100203cf(1, 3);
                U32(b, 0xbaf) = 0;
                /* 1.2 (0x1005b5c6): the fog state as (1, 0x200000), 1.0's code 2 (1.0 passed (0x200000, 1), a code
                   its renderers reject) */
                if (G32(0x1017d108) != 0)
#ifdef W10F_MUTATE /* the checker's negative check only: 1.0's call */
                    (*SIM_PTR(W10_6SlotFn, 0x109e3d40))(0x200000, 1);
#else
                    (*SIM_PTR(W10_6SlotFn, 0x109e3d40))(2, 0x200000);
#endif
                G32(0x1014cb1c) = 0;
            }
            if (U32(b, 0xbb3) != 0) {
                Sim_100203cf(0x10, 3);
                if (G32(0x1015d1ac) == 4)
                    Sim_1009b362();
                U32(b, 0xbb3) = 0;
            }
        }
        break;
    case 0x1c20:
        if (act && (U32(b, 0xbd3) & 0x400) == 0 && U32(b, 0x4f0) == 2) {
            U32(b, 0xc4d) = 2;
            if (U32(b, 0xbab) != 0) {
                Sim_100203cf(0, 3);
                U32(b, 0xbab) = 0;
                Sim_1003cbb3(0, 0);
            }
            if (U32(b, 0xc35) != 0) {
                Sim_100203cf(2, 3);
                U32(b, 0xc35) = 0;
            }
        }
        break;
    case 0x1c52:
        if (act && U32(b, 0x4f0) == 2) {
            U32(b, 0xc4d) = 3;
            Sim_100203cf(5, 3);
        }
        break;
    case 0x1c84:
        if ((U32(b, 0xbd3) & 0x200) != 0)
            break;
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(6, 2);
        W10_6Scale500(b, 0x1012b678);
        W10_6CELL(b, G32(0x10138d70 + (uint32_t)part * 4u), k) = 0;
        U32(b, 0xc39) += 0x712;
        break;
    case 0x1cb6:
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(6, 2);
        W10_6Scale500(b, 0x1012b67c);
        U32(b, 0xc39) += 0x712;
        break;
    case 0x1ce8:
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(7, -1);
        U32(b, 0x500) = 0;
        U32(b, 0xc39) += 0x712;
        break;
    case 0x1d1a:
        if (act) {
            if (U32(b, 0xbc3) != 0)
                return;
            if (U32(b, 0x4f0) == 2)
                Sim_100203cf(4, 2);
        }
        U32(b, 0xbe3) >>= 1;     /* (the original's "below 0" test of the unsigned result can never hold) */
        break;
    case 0x1d4c:
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(4, 3);
        U32(b, 0xbe3) = 0;
        break;
    case 0x1d7e:
        if ((U32(b, 0xbd3) & 0x200) != 0)
            break;
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(8, 2);
        if (U32(b, 0xbd7) != 0)
            U32(b, 0xbd7) -= 1;
        break;
    case 0x1db0:
        if ((U32(b, 0xbd3) & 0x200) != 0)
            break;
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(9, 2);
        W10_6Scale500(b, 0x1012b678);
        break;
    case 0x1de2:
        if ((U32(b, 0xbd3) & 0x200) != 0)
            break;
        if (act) {
            if (U32(b, 0xbbb) != 0) {
                U32(b, 0xbbb) = 0;
                return;
            }
            if (U32(b, 0x4f0) == 2)
                Sim_100203cf(0x28, 2);
        }
        W10_6Scale500(b, 0x1012b678);
        if (U32(b, 0xbd7) != 0)
            U32(b, 0xbd7) -= 1;
        break;
    case 0x1e14:
        if (act) {
            if (U32(b, 0xbc7) != 0)
                return;
            if (U32(b, 0x4f0) == 2)
                Sim_100203cf(0x29, -1);
        }
        U32(b, 0xc39) += 0x712;
        break;
    case 0x1e46:
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(0xc, 2);
        if (U32(U32(b, 0), 0) == 1 || U32(U32(b, 0), 0) == 9)
            U32(b, 0xc15) = U8(U32(U32(b, 0), 0x54), 0x59) != 0 ? 2 : 1;
        break;
    case 0x1e78:
        if (act && U32(b, 0x4f0) == 2)
            Sim_100203cf(0xd, 2);
        break;
    case 0x2328:
        if (flag == 0 && U32(b, 0x4f0) == 2)
            Sim_1006bf33((uint32_t)killer, part, b);
        break;
    case 0x1f40:
        if (U32(b, 0xbb7) != 0)
            return;
        if (act) {
            G32(0x1016551e) = 1;
            if (U32(b, 0x4f0) == 2) {
                Sim_100203cf(0xf, 2);
                G32(0x1015dd14) = 1;
            }
        }
        U32(b, 0xc39) += 0x712;
        break;
    case 0x1eaa:
        if (flag != 0 || (U16(b, 0x560) & 0x20) != 0 || U32(b, 0x4f0) != 2)
            break;
        if (U32(b, 0xbbf) != 0)
            return;
        if (mine)
            Sim_100203cf(0x2a, -1);
        else
            Sim_10015630((void *)(uintptr_t)U32(b, 0), 0xa, 0, 1);
        U32(b, 0xc3d) = 0x21f;
        break;
    default:
        break;
    }
    W10_6CELL(b, part, k) = 0;
}

typedef void(__cdecl *W10_6LogFn)(const char *format, ...);

#define W10_6F(p, off) (*(float *)(BYTES(p) + (off)))
#define W10_6C(addr) (*SIM_PTR(float, addr))

/* the hit's sound away from the own unit: Sim_1000eb12 at the unit's offset from the camera, scaled by [0x1012b680] */
static void W10_6HitSound(uint32_t unit, const char *name)
{
    uint32_t cam = G32(0x1014cae0);

    Sim_1000eb12((float)(((double)W10_6F(cam, 0) - W10_6F(unit, 0x58)) * W10_6C(0x1012b680)),
                 (float)(((double)W10_6F(cam, 4) - W10_6F(unit, 0x5c)) * W10_6C(0x1012b680)),
                 (float)(((double)W10_6F(cam, 8) - W10_6F(unit, 0x60)) * W10_6C(0x1012b680)), name,
                 (int)G32(0x1015ef0c), 0xa, unit);
}

/* the sparks at the part's node (not for kind-10 objects): Sim_100939a3 with the shot's velocity */
static void W10_6Sparks(uint32_t unit, int32_t part, const uint8_t *shot, int32_t type)
{
    float x, y, z;
    void *node = Sim_100aacd5((void *)(uintptr_t)U32(unit, 0x48), (uint32_t)part);

    if (node == 0 || U32(unit, 0) == 0xa)
        return;
    Sim_100a9cfb(node, (uint32_t *)&x, (uint32_t *)&y, (uint32_t *)&z);
    /* 1.2 (0x1005bfca, round 32): the sparks particles, also given the shot's ground kind +0x8e */
    if (W32_PARTICLES()) {
        Sim12_1007d342(x, y, z, W10_6F(shot, 0x18), W10_6F(shot, 0x1c), W10_6F(shot, 0x20), type,
                       (int32_t)U32(shot, 0x8e));
        return;
    }
    Sim_100939a3(x, y, z, W10_6F(shot, 0x18), W10_6F(shot, 0x1c), W10_6F(shot, 0x20), type);
}

// FUNCTION: HGSIM 0x1006c161
void __cdecl Sim_1006c161(int32_t killer, uint8_t *b, int32_t flag, uint8_t *shot, int32_t part)
{
    static const int32_t spread[5] = {-5, -1, 0, 1, 5};      /* the original's ebp-0x24..-0x14 */
    uint32_t unit = U32(b, 0);
    int mine = U32(unit, 4) == G32(0x1017b8c8);
    int hit = 0, i;
    int32_t dmg, sel, r, level = 0, armor;
    uint8_t *pp;

    if (mine && U8(G32(0x1017b8c4), 1) != 0)
        return;
    if (G32(0x10179ce0) == 0)
        return;
    if (G32(0x1015ec58) != 0 && !(U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)) && !mine)
        return;
    if (shot == 0 || part < 1 || part > 0x11)
        return;
    if ((part == 6 || part == 5) && U32(unit, 0) != 1 && U32(unit, 0) != 9)
        return;
    pp = BYTES(U32(b, 0x88) + (uint32_t)part * 0x3cu - 0x3cu);
    armor = S32(pp, 0x38);
    if ((U32(b, 0xbcf) & 0x10) != 0)
        U32(shot, 0x70) += 2;
    if (U16(shot, 0x7c) == 3 || U16(shot, 0x7c) == 4)
        dmg = Render_Ftol((double)U32(shot, 0x70) - (double)W10_6F(pp, 0) / W10_6C(0x1012b680));
    else
        dmg = Render_Ftol((double)U32(shot, 0x70) - (double)W10_6F(pp, 0));
    if (dmg > 0) {
        dmg = Render_Ftol((double)dmg * W10_6F(shot, 0x74));
        if (!(W10_6F(pp, 0) >= W10_6C(0x1012b684))) {
            U32(pp, 0) = 0;
            W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) - (double)dmg);
            U16(pp, 0x36) |= 0x4000;
            hit = 1;
        } else {
            switch (U16(shot, 0x7c)) {
            case 1:
                W10_6F(pp, 0) = (float)((double)W10_6F(pp, 0) - (double)dmg);
                U16(pp, 0x36) |= 0x8000;
                break;
            case 2:
                W10_6F(pp, 0) = (float)((double)W10_6F(pp, 0) - (double)dmg / W10_6C(0x1012b680));
                W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) - (double)dmg / W10_6C(0x1012b680));
                U16(pp, 0x36) |= 0x8000;
                U16(pp, 0x36) |= 0x4000;
                hit = 1;
                break;
            case 3:
            case 4:
                W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) - (double)dmg);
                U16(pp, 0x36) |= 0x4000;
                hit = 1;
                break;
            default:
                break;
            }
            if (!(W10_6F(pp, 0) >= W10_6C(0x1012b684))) {
                W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) + W10_6F(pp, 0));
                U32(pp, 0) = 0;
            }
        }
        if (flag != 0)
            W10_6Sparks(unit, part, shot, 2);
        if ((S16(unit, 0x1c) & 0x2000) != 0) {
            if (mine)
                Sim_1000ebfb((const char *)(uintptr_t)U32(shot, 0x86), 0, 0, 5, 0x50);
            else
                W10_6HitSound(unit, (const char *)(uintptr_t)U32(shot, 0x86));
        }
    } else {
        if (!(W10_6F(pp, 0) >= W10_6C(0x1012b684))) {
            U32(pp, 0) = 0;
            W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) - (double)U32(shot, 0x78));
            U16(pp, 0x36) |= 0x4000;
            hit = 1;
        } else {
            W10_6F(pp, 0) = (float)((double)W10_6F(pp, 0) - (double)U32(shot, 0x78));
            if (!(W10_6F(pp, 0) >= W10_6C(0x1012b684))) {
                W10_6F(pp, 8) = (float)((double)W10_6F(pp, 8) + W10_6F(pp, 0));
                U32(pp, 0) = 0;
                U16(pp, 0x36) |= 0x4000;
                hit = 1;
            }
            U16(pp, 0x36) |= 0x8000;
        }
        if (flag != 0) {
            if ((S16(unit, 0x1c) & 0x2000) != 0) {
                if (mine)
                    Sim_1000ebfb(Sim_1009c929(2) != 0 ? SIM_PTR(const char, 0x10145320) : SIM_PTR(const char, 0x1014532c),
                                 0xfffff830u, 0, 5, 0x50);
                else
                    W10_6HitSound(unit, SIM_PTR(const char, 0x10145338));
            }
            W10_6Sparks(unit, part, shot, 0);
        }
    }
    if (!(Crt_Fabs((double)hit) < (double)W10_6C(0x1012b684))) {
        if (!(W10_6F(pp, 8) > *SIM_PTR(double, 0x1012b688)) && (S16(pp, 0x36) & 0x2000) == 0) {
            g_simDestroyShot = shot;        /* for the damage log only (w8w.h) */
            Sim_1006abf6((uint32_t)killer, b, part);
            g_simDestroyShot = 0;
            return;
        }
        /* two system hits, the cell picked by chance and the damage */
        for (i = 0; i < 2; i++) {
            r = Sim_1009c929(100);
            sel = r < 5 ? spread[0] : r < 0x19 ? spread[1] : r < 0x4b ? spread[2] : r < 0x5f ? spread[3] : spread[4];
            sel += dmg / 0x32;
            if (sel >= S32(b, 0xb67u + (uint32_t)part * 4u))
                sel = S32(b, 0xb67u + (uint32_t)part * 4u);
            if (sel >= 0)
                Sim_1006b52f(killer, b, part, sel, 0);
        }
    }
    SIM_FN(W10_6LogFn, 0x10083495)(SIM_PTR(const char, 0x10145344), (double)W10_6F(pp, 0), (double)W10_6F(pp, 8),
                                   U32(shot, 0x70), dmg, part, U32(U32(b, 0), 4));
    if (armor != 0)
        level = 0xf - Render_Ftol(((double)W10_6F(pp, 8) * W10_6C(0x1012b67c) +
                                   (double)W10_6F(pp, 0) * W10_6C(0x1012b67c)) * W10_6C(0x1012b690) / (double)armor);
    Sim_100aae7b((void *)(uintptr_t)U32(U32(b, 0), 0x48), level, (uint32_t)part);
}