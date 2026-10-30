#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "w2i.h"
#include "w7l.h"
#include "../unsorted_100a/w2k.h"
#include "../unsorted_100a/w3q.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5c.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "w2g.h"
#include "w9b.h"
#include "../render/w2n.h"
#include "../unsorted_100d/w8z.h"

#define GF(addr) (*SIM_PTR(float, addr))

// FUNCTION: HGSIM 0x10010fbf
float __cdecl Sim_10010fbf(const void *obj)
{
    float f = *(const float *)(BYTES(U32(obj, 0x28)) + 0x53c);
    float v;

    /* the product of two floats is exact in double, so one rounding to float as the original's fstp */
    switch (U32(obj, 0)) {
    case 3:
        v = f * GF(0x101110fc);
        break;
    case 5:
    case 8:
        v = f * GF(0x10111100);
        break;
    case 10:
        v = f * GF(0x101110f8);
        break;
    default:
        v = f * GF(0x10111104);
        break;
    }
    return v;
}

typedef void(__cdecl *W10_1SlerpFn)(const float *a, const float *b, float t, float *out);

static uint32_t W10_1Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

// FUNCTION: HGSIM 0x1001c1ab
void __cdecl Sim_1001c1ab(uint8_t *obj, int32_t t, uint8_t *anim, int32_t blend, uint8_t *state)
{
    float q1[4], q2[4];       /* the original's two rotations at ebp-0x28 and ebp-0x18 */
    float *qa, *qb;
    uint8_t *node, *rec, *anim2, *target;
    uint32_t id, two, ev, m;
    int any = 0, i;
    float x, y, z, a, b, c, d, e, f;

    if (obj == 0 || anim == 0 || state == 0 || U32(anim, 0) == 0)
        return;
    node = BYTES(U32(obj, 0x48));
    rec = BYTES(U32(obj, 0x28));
    anim2 = BYTES(U32(state, 0x10));
    two = U32(state, 0x18);
    for (i = 0; i < S32(U32(anim, 0x14), 0xc); i++) {
        uint32_t bone = U32(anim, 0x1c + i * 0xc);
        uint32_t keys = U32(anim, 0x24 + i * 0xc);

        id = U32(bone, 4);
        if ((two != 0 && (uint32_t)i == U32(U32(anim2, 0x14), 0xc)) || (U32(keys, 0) == 0 && t != 0))
            target = 0;
        else
            target = (uint8_t *)Sim_100aadaa(node, (const char *)(uintptr_t)(bone + 4));
        if (target == 0)
            continue;
        if (U32(keys, 0) != 0 && Sim_1001bb9d(t, S32(U32(anim, 0x14), 4), (const void *)(uintptr_t)keys, q1) == 0)
            qa = q1;
        else
            qa = SIM_PTR(float, 0x10131740);
        if (two != 0) {
            uint32_t keys2 = U32(anim2, 0x24 + i * 0xc);

            if (U32(keys2, 0) != 0 &&
                Sim_1001bb9d(S32(state, 4), S32(U32(anim2, 0x14), 4), (const void *)(uintptr_t)keys2, q2) == 0)
                qb = q2;
            else
                qb = SIM_PTR(float, 0x10131740);
            SIM_FN(W10_1SlerpFn, 0x100608ef)(qb, qa, (float)((double)blend / (double)*SIM_PTR(float, 0x101111ec)),
                                              qa);
        }
        if (id == G32(0x1015d190) || U32(target, 0) == 0)
            Sim_100aa227(target, qa);
        else if (id == G32(0x1015d198) && G32(0x10131738) != 0)
            Sim_100aa1de(target, qa);
        else if (id == G32(0x1015d194))
            Sim_100aa1de(target, qa);
        else if (id == G32(0x1015d19c))
            Sim_100aa1de(target, qa);
        else
            Sim_100aa190(target, (const uint32_t *)qa);
        any = 1;
    }
    if (any)
        Sim_100aa799(node);
    ev = (uint32_t)(uintptr_t)Sim_1001c075(t, S32(U32(anim, 0x14), 4), (void *)(uintptr_t)U32(anim, 0x31c));
    if (ev != 0) {
        m = U32(ev, 0xd);
        if ((m == 1 || m == 2 || m == 0) && U32(state, 0x2c) != m) {
            U32(state, 0x2c) = m;
            if (m == 0 || m == 1)
                U32(obj, 0x88) |= 8;
        }
        Sim_100a9ccf(node, (uint32_t)(uintptr_t)&x, (uint32_t)(uintptr_t)&y, (uint32_t)(uintptr_t)&z);
        rec = BYTES(U32(obj, 0x28));
        *(float *)(rec + 0x564) = x;
        if (U32(ev, 0x11) == 1 && U16(rec, 0x562) != 0)
            U16(rec, 0x562) = (uint16_t)(U16(rec, 0x562) | 0x1000);
    }
    if (!any || (U32(obj, 0) != 1 && U32(obj, 0) != 9) || U32(obj, 0x15c) != 0)
        return;
    if (U32(rec, 0x4c0) != 0 && (U16(rec, 0x562) & 0x2000) == 0)
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), (uint32_t *)&a, (uint32_t *)&b, (uint32_t *)&c);
    else
        b = 3.4e38f;     /* 0x7f7fc99e */
    if (U32(rec, 0x4c4) != 0 && (U16(rec, 0x562) & 0x4000) == 0)
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c4), (uint32_t *)&d, (uint32_t *)&e, (uint32_t *)&f);
    else
        e = 3.4e38f;
    if (!(b >= e))
        e = b;
    if (W10_1Bits(e) == 0x7f7fc99eu)
        e = 0.0f;
    *(float *)(state + 0x34) = *(float *)(obj + 0x7c) - e;
    Sim_100a9d7c(node, 0.0f, *(float *)(state + 0x34), 0.0f);
    Sim_100aa799(node);
}


#define W10_1ST(i) GBYTES(0x10a1d140u + (uint32_t)(i) * 0x38u)
/* the frame step: q = trunc(x / [0x101111a8] + [0x101111f0]), at least 1 while the frame time [0x1015dc80] is not 0 */
static int32_t W10_1Step(double x)
{
    int32_t q = Render_Ftol(x / *SIM_PTR(float, 0x101111a8) + *SIM_PTR(float, 0x101111f0));

    if (q <= 0 && G32(0x1015dc80) != 0)
        q = 1;
    return q;
}

// FUNCTION: HGSIM 0x1001c693
void __cdecl Sim_1001c693(void)
{
    uint8_t *unit, *st, *rec, *anim, *node;
    const char *name;
    float x, y, z, r;
    int32_t i, q, period, half, sum;
    uint32_t speed, kind;

    if (G32(0x1015ec58) != 0 && (G32(0x1017b94c) & 1) == 0)
        return;
    for (i = 0; i < GS32(0x1015dc64); i++) {
        unit = BYTES(PTRTAB(i));
        st = W10_1ST(i);
        U32(st, 0x30) = 0;
        node = BYTES(U32(unit, 0x48));
        if (U32(unit, 0) != 1 && U32(unit, 0) != 9 && U32(unit, 0) != 5)
            continue;
        name = (const char *)BYTES(U32(node, 0x90));
        if (name[0] == 'V' && name[1] == '1' && name[2] == '0')
            continue;
        rec = BYTES(U32(unit, 0x28));
        G32(0x10131738) = !((U32(rec, 0x4f0) == 4 && U32(U32(rec, 0), 0x80) != 0) || (U16(rec, 0x560) & 0x100) != 0);
        if (U32(unit, 4) != G32(0x1017b8c8) && Sim_1001c618(U32(unit, 0x90)) == 0 && Sim_1001c618(U32(unit, 0x8c)) == 0) {
            /* not shown when its projected circle lies off the screen (25 pixels margin) or in front of the near plane */
            x = *(float *)(unit + 0x58);
            y = *(float *)(unit + 0x5c);
            z = *(float *)(unit + 0x60);
            Sim_100d2669(&x, &y, &z);
            r = (float)((double)*(float *)BYTES(G32(0x1014cae0) + 0x94) * *(float *)(rec + 0x53c) / z);
            if (!((double)(GS32(0x10185bec) + 0x19) >= (double)x - r))
                continue;
            if ((double)(GS32(0x10185bd0) - 0x19) > (double)x + r)
                continue;
            if (!((double)(GS32(0x10185be0) + 0x19) >= (double)y - r))
                continue;
            if ((double)(GS32(0x10185bf0) - 0x19) > (double)y + r)
                continue;
            if ((double)z - r > (double)*SIM_PTR(float, 0x10185bf8))
                continue;
        }
        speed = U32(unit, 0x94);
        if (U32(unit, 0x90) != U32(unit, 0x8c)) {
            anim = Anim_RecordOfId((int)U32(unit, 0x90));
            if (anim != 0) {
                if (U32(st, 0x14) != 0 && U32(U32(st, 0x14), 0) != 0) {
                    if (U32(anim, 4) != 0) {
                        U32(st, 0x10) = U32(st, 0x14);
                        U32(st, 0x18) = 1;
                        U32(st, 4) = U32(st, 8);
                        U32(st, 0xc) = 0;
                        U32(st, 8) = 0;
                        U32(st, 0x14) = U32(anim, 0);
                        U32(st, 0x1c) = U32(unit, 0x8c);
                        U32(unit, 0x8c) = U32(unit, 0x90);
                    }
                } else if (U32(anim, 4) != 0) {
                    U32(st, 0x14) = U32(anim, 0);
                    U32(unit, 0x8c) = U32(unit, 0x90);
                } else {
                    U32(unit, 0x90) = U32(unit, 0x8c);
                }
            }
        }
        if (U32(st, 0x14) == 0 || U32(U32(st, 0x14), 0) == 0)
            continue;
        if (U32(st, 0x18) != 0) {
            /* blending into the new animation over 10 steps */
            Sim_1001c1ab(unit, S32(st, 0xc), BYTES(U32(st, 0x14)), S32(st, 8), st);
            q = W10_1Step((double)(int32_t)(G32(0x1015dc80) * 0x1eu));
            U32(st, 8) += (uint32_t)q;
            if (S32(st, 8) >= 0xa) {
                U32(st, 0x18) = 0;
                U32(st, 8) = U32(st, 0xc);
            }
            continue;
        }
        Sim_1001c1ab(unit, S32(st, 8), BYTES(U32(st, 0x14)), 0, st);
        anim = BYTES(U32(U32(st, 0x14), 0x14));
        if (speed == 1)
            q = W10_1Step((double)S32(anim, 8) * *SIM_PTR(float, 0x101111f4) * (double)GS32(0x1015dc80));
        else if (speed == 3)
            q = W10_1Step((double)S32(anim, 8) * *SIM_PTR(float, 0x101111f8) * (double)GS32(0x1015dc80));
        else
            q = W10_1Step((double)(int32_t)(U32(anim, 8) * G32(0x1015dc80)));
        period = S32(anim, 4);
        half = period / 2;
        sum = (int32_t)(U32(st, 8) + (uint32_t)q);
        kind = U32(unit, 0x8c) & 0xff;
        if (kind == 3 || kind == 1 || kind == 2) {
            if (sum > half) {
                U32(st, 0x30) = 1;
                U32(st, 8) = (uint32_t)half;
            } else {
                U32(st, 8) = (uint32_t)(sum % period);
            }
        } else if (kind == 5 || kind == 4) {
            if ((U16(rec, 0x562) & 0x200) != 0) {
                if ((U16(rec, 0x560) & 0x20) == 0) {
                    U32(st, 8) = (uint32_t)(sum % period);
                    if (S32(st, 8) <= half)
                        U32(st, 8) = (uint32_t)period;
                    if (S32(st, 8) == period) {
                        U16(rec, 0x562) &= 0xfffb;
                        U16(rec, 0x562) &= 0xfbff;
                        U16(rec, 0x562) &= 0xfdff;
                    }
                } else {
                    U16(rec, 0x562) &= 0xfdff;
                    U32(rec, 0xc15) = 1;
                }
            } else if (sum >= half) {
                U32(st, 0x30) = 1;
                U32(st, 8) = (uint32_t)half;
                U16(rec, 0x562) |= 8;
            } else {
                U32(st, 8) = (uint32_t)(sum % period);
            }
        } else if (kind == 0xd || kind == 0x11 || kind == 8 || kind == 0x19 || kind == 0x15) {
            if (sum >= period) {
                U16(rec, 0x562) &= 0xf7ff;
                U32(unit, 0x90) = U32(st, 0x1c);
            } else {
                U32(st, 8) = (uint32_t)(sum % period);
            }
        } else if (kind == 0x18) {
            if (sum >= period)
                U16(rec, 0x560) |= 0x10;
            else
                U32(st, 8) = (uint32_t)(sum % period);
        } else {
            U32(st, 8) = (uint32_t)(sum % period);
        }
    }
}