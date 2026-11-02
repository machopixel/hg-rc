#include "w8z.h"
#include "w2f.h"
#include "w5z.h"
#include "w7p.h"
#include "w7q.h"
#include "w5a.h"
#include "w5b.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1001/w3t.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_100d/w8z.h"
#include "../util/util.h"
#include "../unsorted_1000/w24.h"
#include "../libm/hg_math.h"

// FUNCTION: HGSIM 0x100cbc6b
void __cdecl Sim_100cbc6b(float dx, float dy, float dz, float *a, float *b, float *c, float *d)
{
    float angle = W8Z_CALL_1008486e(dx, dz);
    double sizeX = hg_fabs((double)dx);
    float level;
    float div;
    float up;
    float vert;

    if (!(hg_fabs((double)dz) >= sizeX)) {
        div = W8Z_CALL_10084766(angle);
        if (!(hg_fabs((double)div) >= (double)W8Z_GF(0x1012bc54)))
            level = 0.0f;
        else
            level = (float)((double)dx / (double)div);
    } else {
        div = W8Z_CALL_10084766((float)((double)angle + (double)W8Z_GF(0x1012bc58)));
        if (!(hg_fabs((double)div) >= (double)W8Z_GF(0x1012bc54)))
            level = 0.0f;
        else
            level = (float)((double)dz / (double)div);
    }
    up = W8Z_CALL_1008486e(dy, level);
    div = W8Z_CALL_10084766((float)((double)up + (double)W8Z_GF(0x1012bc58)));
    if (!(hg_fabs((double)div) >= (double)W8Z_GF(0x1012bc54)))
        vert = 0.0f;
    else
        vert = (float)((double)level / (double)div);
    *a = angle;
    *c = level;
    *b = vert;
    *d = up;
}

// FUNCTION: HGSIM 0x100cb4fb
int __cdecl Sim_100cb4fb(void *unit)
{
    int invalid = 1;
    int current = 0;
    uint32_t index;
    uint32_t kind;
    uint32_t other;
    uint32_t target;
    uint32_t bit;
    int i, picked;

    if (U32(unit, 4) == G32(0x1017b8c8))
        current = 1;
    index = U32(unit, 0xf0) & 0xff;
    kind = U32(unit, 0xf0) & 0xf00;
    switch (kind) {
    case 0x200:
        picked = W8Z_CALL_100cada2(S32(unit, 4), (int)index, 0);
        /* the values this call left where 0x1000aa9e reads them (DECISIONS.md, round 24) */
        if (g_w24TargetSet != 0) {
            g_w24AimDz = g_w24TargetDy;
            g_w24AimDy = g_w24TargetAngle;
        }
        if (picked >= 0) {
            other = PTRTAB(index);
            if (Sim_10052be7(S32(other, 4)) != Sim_10052be7(S32(unit, 4))) {
                for (i = 0; i < 3; i++) {
                    if (S32(other, 0x20c + 4 * i) == -1) {
                        U32(other, 0x20c + 4 * i) = U32(unit, 4);
                        break;
                    }
                }
            }
            invalid = 0;
        }
        break;
    case 0x400:
        if (Sim_100cb299(S32(unit, 4), (int)index, 0) >= 0)
            invalid = 0;
        break;
    case 0x100:
        if (Sim_100cab7b(S32(unit, 4), (int)index, 0) >= 0) {
            invalid = 0;
            target = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x109a8640u + index * 0x54u);
            if (!(W8Z_FL(unit, 0xd4) >= W8Z_FL(target, 0x10)) && S32(U32(unit, 0x28), 0x50c) != 1 &&
                U32(unit, 4) == G32(0x1017b8c8)) {
                if ((S16(target, 0x24) & 0x20) == 0) {
                    U16(target, 0x24) = (uint16_t)(U16(target, 0x24) | 0x20u);
                    U16(target, 0x26) = (uint16_t)(U16(target, 0x26) | (uint16_t)(1u << (U32(unit, 8) & 31u)));
                    Sim_1000ebfb((const char *)SIM_PTR(char, 0x10151d40), 0xfffff63cu, 0, 5, 0x50);
                }
                Sim_100cbdb8((uint32_t)(uintptr_t)unit, 1, 0);
            }
        }
        break;
    }
    if (invalid) {
        U32(unit, 0xf0) |= 0x1000;
        return 0;
    }
    if (current)
        G32(0x1017e5ac) = 0;
    if (kind == 0x200) {
        other = PTRTAB(index);
        bit = 1u << (U32(unit, 8) & 31u);
        if (((uint32_t)(int32_t)S16(other, 0x1e) & bit) == 0) {
            if ((double)W8Z_GF(0x1012bc50) + (double)W8Z_FL(U32(other, 0x28), 0x53c) > (double)W8Z_FL(unit, 0xd4)) {
                if (current)
                    G32(0x1017e5ac) = 1;
                U16(other, 0x1c) = (uint16_t)(U16(other, 0x1c) | 0x20u);
                U16(other, 0x1e) = (uint16_t)(U16(other, 0x1e) | (uint16_t)bit);
            }
        } else if (current) {
            G32(0x1017e5ac) = 3;
        }
    } else if (kind == 0x400) {
        target = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x109e9ba0u + index * 0x50u);
        bit = 1u << (U32(unit, 8) & 31u);
        if (((uint32_t)(int32_t)S16(target, 6) & bit) == 0) {
            if ((double)S32(target, 0x14) + (double)W8Z_GF(0x1012bc50) > (double)W8Z_FL(unit, 0xd4)) {
                if (current)
                    G32(0x1017e5ac) = 1;
                U16(target, 0) = (uint16_t)(U16(target, 0) | 0x20u);
                U16(target, 6) = (uint16_t)(U16(target, 6) | (uint16_t)bit);
            }
        } else if (current) {
            G32(0x1017e5ac) = 3;
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x100cbaf8
void __cdecl Sim_100cbaf8(void)
{
    uint32_t choice = 0xffffffffu;
    int wasSet = 0;
    uint32_t unit = PTRTAB(G32(0x1017b8c8));
    uint32_t old = U32(unit, 0xf0);
    uint32_t record;

    if (S32(U32(unit, 0x28), 0x50c) == 1)
        wasSet = 1;
    record = G32(0x1017e678);
    if (record != 0) {
        if ((U16(record, 2) & 0x100) != 0) {
            if (W8Z_CALL_100cada2(GS32(0x1017b8c8),
                                  U16(record, 0x24), 0) >= 0)
                choice = U16(record, 0x24) | 0x200u;
        } else if ((U16(record, 2) & 0x200) != 0) {
            if (Sim_100cb299(GS32(0x1017b8c8), U16(record, 0x24), 0) >= 0)
                choice = U16(record, 0x24) | 0x400u;
        }
        if (choice != 0xffffffffu && choice != U32(unit, 0xf0)) {
            U32(unit, 0xf0) = choice;
            if ((U32(unit, 0x118) & 0x100) == 0 && U32(U32(unit, 0x28), 0x50c) != 0)
                U8(U32(unit, 0x54), 0x46) = 1;
            if (Sim_100cb4fb((void *)(uintptr_t)unit) == 0) {
                U32(unit, 0xf0) = old;
                if (wasSet) {
                    U8(U32(unit, 0x54), 0x46) = 0;
                    U32(U32(unit, 0x28), 0x50c) = 1;
                }
            }
        }
    }
    G32(0x1017e5b0) = 1;
}

// FUNCTION: HGSIM 0x100c398d
void __cdecl Sim_100c398d(void *rec, void *pp)
{
    uint32_t sel = 0;
    uint32_t target = 0;
    uint32_t unit = U32(pp, 0);
    uint32_t kind;
    uint32_t index;
    float outA, outB;
    float x, y, z;
    float limitA, limitB;
    float angleA, angleB;
    float key;
    int16_t dir;
    int16_t side;
    int16_t i;
    uint8_t seg[0x38];
    uint32_t hit;

    if (((U32(unit, 0x26c) >> 2) & 1u) == 0 || Sim_1009c929(3) == 0)
        return;
    if ((U32((uintptr_t)SIM_PTR(uint8_t, 0x1014558e) + (uint32_t)S32(rec, 4) * 0xd6u, 0) & 1u) == 0)
        return;
    unit = U32(pp, 0);
    if (U32(unit, 0x14) == 2 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))
        sel = U32(unit, 0xf0);
    else if ((U16(unit, 0x1aa) & 0x200) != 0)
        sel = U16(unit, 0x1aa);
    else
        sel = U32(unit, 0xf0);
    kind = sel & 0xf00;
    index = sel & 0xff;
    if (index == 0 || kind != 0x200 || S32(PTRTAB(index), 0x1fc) == 4)
        return;
    if (U32(unit, 0x14) == 2 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))
        target = PTRTAB(index);
    if (target == 0 && W8Z_CALL_10064824(pp, &outA, &outB) != 0) {
        x = W8Z_FL(PTRTAB(index), 0x58);
        y = W8Z_FL(PTRTAB(index), 0x5c);
        z = W8Z_FL(PTRTAB(index), 0x60);
        if (Sim_100d2669(&x, &y, &z) != 0) {
            limitA = W8z_Float(0x40400000);
            limitB = W8z_Float(0x41700000);
            angleA = W8Z_CALL_100fcb30((double)W8Z_FL(pp, 0x1c) + (double)W8Z_FL(U32(pp, 0), 0xec), 360.0);
            angleB = W8Z_CALL_100fcb30((double)Sim_10015266((void *)(uintptr_t)U32(pp, 0)), 360.0);
            if (!(angleA >= limitA) && !(-(double)limitA >= (double)angleA) && !(angleB >= limitB) &&
                !(-(double)limitB >= (double)angleB))
                target = PTRTAB(index);
        }
    }
    if (target == 0)
        return;
    if (Sim_100c3892((void *)(uintptr_t)target, 0x41) == 0)
        return;
    if (Sim_1009c929(3) != 0) {
        key = Sim_100c34b7((void *)(uintptr_t)target, S16(U32(pp, 0), 4));
        dir = (int16_t)W8z_Ftol((double)key / (double)W8Z_GF(0x1012bbb8));
        side = Sim_1009c929(2) != 0 ? -1 : 1;
        dir = (int16_t)(uint16_t)((uint32_t)(((int32_t)dir + 1) % 4) << 2);
        for (i = 0; i < 2; i++) {
            Sim_100c31a6((void *)(uintptr_t)target, seg, side, dir, W8z_Float(0x459c4000), 0);
            if (Sim_1002dda9(seg, &hit, S32(target, 4)) == 0) {
                U32(target, 0x1fc) = 0xb;
                U32(target, 0x218) = (uint32_t)(int32_t)dir;
                break;
            }
            side = (int16_t)-side;
        }
    }
    if (S32(target, 0x1fc) == -1 && ((U32(target, 0x26c) >> 1) & 1u) != 0)
        U32(target, 0x1fc) = 4;
}

// FUNCTION: HGSIM 0x100c9572
double __cdecl Sim_100c9572(void *obj, int kind, int who)
{
    uint32_t block = U32(obj, 0x28);
    uint32_t record = U32(block, 0x84) + (uint32_t)S32(block, 0x4fc) * 0x128u;
    uint32_t type = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x10145508) + (uint32_t)S32(record, 4) * 0xd6u;
    uint32_t aim;
    uint32_t ax, ay, az;
    uint32_t tx, ty, tz;
    float px, py, pz;
    float vx, vy, vz;
    float g;
    float height;
    float neg;
    float t;
    double root;
    float outA, outB, outC, outD;

    U32(obj, 0x50) = U32(block, 0x498 + 4 * U32(record, 0x11c));
    if (obj == 0 || U32(obj, 0x50) == 0)
        return (double)W8Z_GF(0x1012bc2c);
    if (kind == 0x200) {
        ay = U32(PTRTAB(who), 0x5c);
    } else {
        aim = Sim_100cba68();
        if (aim == 0)
            return (double)W8Z_GF(0x1012bc2c);
        Sim_100a9cfb((void *)(uintptr_t)aim, &ax, &ay, &az);
    }
    Sim_100d6ea7(obj, &px, &py, &pz, (void *)(uintptr_t)record);
    Sim_10084c95(&px, &py, &pz);
    if (S32(type, 0x10) == 0x45)
        py = (float)((double)py + W8Z_GD(0x1012bc00));
    vx = (float)((double)W8Z_FL(type, 0x3c) * (double)px);
    vy = (float)((double)W8Z_FL(type, 0x3c) * (double)py);
    vz = (float)((double)W8Z_FL(type, 0x3c) * (double)pz);
    g = (float)(-(double)W8Z_GF(0x1013abc0) * (double)S32(type, 0x38) * (double)W8Z_GF(0x1012bbf8));
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x50), &tx, &ty, &tz);
    height = (float)((double)W8z_Float(ty) - (double)W8z_Float(ay));
    neg = (float)-(double)vy;
    root = W8Z_CALL_100fca04((double)vy * (double)vy - (double)W8Z_GF(0x1012bbf4) * (double)g * (double)height);
    t = (float)(((double)neg - root) / ((double)W8Z_GF(0x1012bc30) * (double)g));
    if ((W8z_Bits(t) & 0x80000000u) != 0) {
        neg = (float)-(double)vy;
        root = W8Z_CALL_100fca04((double)vy * (double)vy - (double)W8Z_GF(0x1012bbf4) * (double)g * (double)height);
        t = (float)((root + (double)neg) / ((double)W8Z_GF(0x1012bc30) * (double)g));
    }
    Sim_100cbc6b((float)((double)vx * (double)t), (float)((double)vy * (double)t), (float)((double)vz * (double)t),
                 &outA, &outB, &outC, &outD);
    return (double)outB / (double)W8Z_GF(0x1012bc20);
}
