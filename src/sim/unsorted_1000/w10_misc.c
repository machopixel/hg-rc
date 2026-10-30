#include "w10.h"
#include "../simmem.h"
#include "../render/w2n.h"
#include "../util/access.h"
#include "../unsorted_1009/w3o.h"
#include "fns.h"
#include "w3t.h"
#include "w6d.h"
#include "w8t.h"
#include "w2f.h"
#include "w3p.h"
#include "w3q.h"
#include "w7l.h"
#include "../unsorted_1001/w5y.h"
#include "../unsorted_1001/w6d.h"
#include "../util/util.h"
#include "../unsorted_100c/w8z.h"

#define GF(addr) (*SIM_PTR(float, addr))

static uint32_t W10_Bits(float f)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.f = f;
    return v.u;
}

static float W10_Float(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x10009fe8
float __cdecl Sim_10009fe8(float x, float y, float z, float r, void *seg)
{
    const float *s = (const float *)seg;
    float dx, dy, dz, d, e, w, t;
    double dz53, q53;

    if (!(r >= GF(0x10111020)))
        return GF(0x1011101c);
    /* x87 at 53-bit precision. A value the original keeps on the stack after `fst dword` stays a double
     * here (dz53, q53); the stored float copy is the float. */
    dx = (float)((double)x - (double)s[0]);
    dy = (float)((double)y - (double)s[1]);
    dz53 = (double)z - (double)s[2];
    dz = (float)dz53;
    d = (float)(dz53 * (double)dz + (double)dx * (double)dx + (double)dy * (double)dy - (double)r * (double)r);
    if (W10_Bits(d) & 0x80000000u)
        return GF(0x10111024);
    e = (float)((double)dx * (double)s[6] + (double)dy * (double)s[7] + (double)dz * (double)s[8]);
    if (W10_Bits(e) & 0x80000000u)
        return GF(0x1011101c);
    q53 = (double)e / (double)s[0xc];
    e = (float)q53;
    w = (float)(q53 * (double)e - (double)d);
    if (W10_Bits(w) & 0x80000000u)
        return GF(0x1011101c);
    t = (float)((double)e - (double)r);
    if (W10_Bits(t) & 0x80000000u)
        return GF(0x10111018);
    if (!((double)Sim_1009cbde(seg) > (double)t))
        return W10_Float(0x7f7fc99eu);
    return t;
}

// FUNCTION: HGSIM 0x1000f096
int32_t __cdecl Sim_1000f096(float d)
{
    int32_t v;

    if (d > GF(0x101110e0))
        return -10000;
    /* x87 at 53-bit precision: each step rounds to double, as here */
    v = Render_Ftol((double)GF(0x101110e4) * (double)d * (double)d / (double)GF(0x101110e8));
    if (v > 0)
        v = 0;
    else if (v < -10000)
        v = -10000;
    return v;
}

typedef double(__cdecl *W10FmodFn)(double, double);

// FUNCTION: HGSIM 0x1000d34d
int __cdecl Sim_1000d34d(void *obj, float a)
{
    uint8_t *o = (uint8_t *)obj;
    int flag = 0, result = 0, r = 0, ok;
    float d, w;

    if (!(S32(o, 0x1be) > GS32(0x1015dc78))) {
        r = Sim_1009c929((int)(int8_t)o[0x1b6]);
        U32(o, 0x1be) = G32(0x1015dc78) + (uint32_t)r * 0x16u;
        d = a - *(float *)(o + 0x74);
        /* the three addends stay in the 53-bit register until the fmod */
        w = (float)SIM_FN(W10FmodFn, 0x100fcb30)((double)*(float *)(o + 0x64) +
                                                     (double)*(float *)(BYTES(U32(o, 0x28)) + 0x1c) +
                                                     (double)*(float *)(o + 0xec), 360.0);
        if (d < GF(0x1011104c) && d > GF(0x101110b4) && w < GF(0x1011104c) && w > GF(0x101110b4) &&
            ((U16(o, 0x1aa) & 0xffu) == G32(0x1017b8c8) || Sim_1009c929(3) == 0)) {
            Sim_1001403a(obj, (short)U16(o, 0x1aa));
            if (r == 0) {
                if (U32(o, 0x1c2) != 0)
                    ok = Sim_1000bf73(obj, &flag);
                else
                    ok = Sim_1000bba7(obj, (uint32_t *)&flag);
                if (ok != 0 && Sim_1000a6f0(obj, 1) != 0) {
                    if (U32(o, 0x1c2) != 0)
                        U8(U32(o, 0x54), 0x28) = 1;
                    else
                        U8(U32(o, 0x54), 0x26) = 1;
                    result = 1;
                }
            }
        }
    }
    if (flag != 0 && result != 0)
        Sim_1000c172(obj);
    if (result == 0)
        r = 0;
    *(float *)(BYTES(U32(o, 0x54)) + 4) =
        (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, a, r) * (double)GF(0x10111064));
    *(float *)BYTES(U32(o, 0x54)) =
        (float)((double)-Sim_100144bb((uint32_t)(uintptr_t)obj, *(float *)(o + 0xec), r) * (double)GF(0x10111068));
    return result;
}

typedef double(__cdecl *W10SqrtFn)(double);
typedef double(__cdecl *W10FabsFn)(double);
#define W10_FABS(x) SIM_FN(W10FabsFn, 0x100fcb70)((double)(x))

/* One coordinate of the prediction, as the original's x87 sequence: n = trunc(t); s = n * [0x10111060] (kept in
 * the register, stored as float for the velocity update); p += ((s * [0x10111078] + v) * n); v += (float)s. */
static void W10_Predict(float t, float *v, float *p)
{
    int32_t n = Render_Ftol((double)t);
    double s = (double)n * (double)GF(0x10111060);

    *p = (float)(((s * (double)GF(0x10111078) + (double)*v) * (double)n) + (double)*p);
    *v = (float)((double)*v + (double)(float)s);
}

// FUNCTION: HGSIM 0x1000c1ea
void __cdecl Sim_1000c1ea(void *unit, float *heading, float *d, float *out)
{
    uint8_t *u = (uint8_t *)unit;
    uint32_t sel, tgt, rec, trec, slot;
    const uint8_t *kind;
    float dx, dy, dz, dot, vx, vy, vz, spd, speed, t, px, py, pz, a, b, c;

    sel = U32(u, 4) == G32(0x1017b8c8) ? U32(u, 0xf0) : U16(u, 0x1aa);
    if ((sel & 0xf00) != 0x200)
        return;
    tgt = PTRTAB(sel & 0xff);
    rec = U32(u, 0x28);
    trec = U32(tgt, 0x28);
    slot = U32(rec, 0x84) + U32(rec, 0x4fc) * 0x128u;
    kind = GBYTES(0x10145508) + U32(slot, 4) * 0xd6u;
    speed = *(const float *)(kind + 0x3c);
    dx = *(float *)(BYTES(tgt) + 0x58) - *(float *)(u + 0x58);
    dy = *(float *)(BYTES(tgt) + 0x5c) - *(float *)(u + 0x5c);
    dz = *(float *)(BYTES(tgt) + 0x60) - *(float *)(u + 0x60);
    vx = *(float *)(BYTES(trec) + 0x548);
    vy = *(float *)(BYTES(trec) + 0x54c);
    vz = *(float *)(BYTES(trec) + 0x550);
    /* x87 at 53-bit precision: products of floats are exact, sums round to double */
    dot = (float)((double)dx * (double)vx + (double)dy * (double)vy + (double)dz * (double)vz);
    if (!(W10_FABS(vx) >= (double)GF(0x10111074)) && !(W10_FABS(vy) >= (double)GF(0x10111074)) &&
        !(W10_FABS(vz) >= (double)GF(0x10111074)))
        return;
    spd = (float)SIM_FN(W10SqrtFn, 0x100fca04)((double)vx * (double)vx + (double)vy * (double)vy +
                                                (double)vz * (double)vz);
    if (W10_Bits(dot) & 0x80000000u)
        spd = -spd;
    /* fcomp: equal or unordered gives 2.0 */
    if (!(W10_FABS(spd) < (double)speed || W10_FABS(spd) > (double)speed))
        t = W10_Float(0x40000000u);
    else
        t = (float)((double)*(float *)(u + 0xd8) / ((double)speed - (double)spd));
    if (!(W10_FABS(t) >= (double)GF(0x10111074)))
        return;
    if (!((double)S32(kind, 0x5c) >= (double)t))
        return;
    if ((double)GS32(0x1015dc80) >= (double)t)     /* only when t is past [0x1015dc80] */
        return;
    px = *(float *)(BYTES(tgt) + 0x58);
    py = *(float *)(BYTES(tgt) + 0x5c);
    pz = *(float *)(BYTES(tgt) + 0x60);
    W10_Predict(t, &vx, &px);
    W10_Predict(t, &vy, &py);
    W10_Predict(t, &vz, &pz);
    out[0] = px;
    out[1] = py;
    out[2] = pz;
    px = px - *(float *)(u + 0x58);
    py = py - *(float *)(u + 0x5c);
    pz = pz - *(float *)(u + 0x60);
    Sim_100cbc6b(px, py, pz, &a, &b, &c, d);
    *heading = a - *(float *)(u + 0x68);
    if (*heading > GF(0x1011107c))
        *heading = *heading - GF(0x10111080);
    else if (!(*heading >= GF(0x10111084)))
        *heading = *heading + GF(0x10111080);
}

// FUNCTION: HGSIM 0x1000b587
int __cdecl Sim_1000b587(void *obj, float a)
{
    uint8_t *o = (uint8_t *)obj;
    uint8_t *rec = BYTES(U32(o, 0x28));
    int flag = 0, result = 0, ok = 0, okw, k = 0;
    int r = 0;
    float f1 = 10.0f, f2 = 10.0f;      /* 0x41200000 */
    const float base = 0.0f;
    float d = 0.0f, w = 0.0f, chk = 0.0f;
    uint32_t entry = 0;
    int twin = U32(o, 0) == 1 || U32(o, 0) == 9;

    if (!(S32(o, 0x1be) > GS32(0x1015dc78))) {
        r = Sim_1009c929((int)(int8_t)o[0x1b6]);
        U32(o, 0x1be) = G32(0x1015dc78) + (uint32_t)r * 0xbu;
        if (Sim_1000ceb7(obj) != 0) {
            if (U32(o, 0x1c2) != 0)
                okw = Sim_1000bf73(obj, &flag);
            else
                okw = Sim_1000bba7(obj, (uint32_t *)&flag);
            if (okw != 0)
                ok = 1;
            else
                ok = Sim_1000b9f1(rec, 0, 0, &entry, 0);
        } else {
            if (Sim_1000c749(rec, S32(rec, 0x4fc), 1) != 0)
                flag = 1;
            ok = Sim_1000b9f1(rec, 0, 0, &entry, 0);
        }
        k = Sim_1000c749(rec, S32(rec, 0x4fc), 0x40);
        if (k == 1) {
            uint32_t slot = U32(rec, 0x84) + U32(rec, 0x4fc) * 0x128u;

            Sim_1000ca63(S32(slot, 4), *(float *)(o + 0xd4), &chk);
            *(float *)(o + 0x1c6) = chk;
        } else {
            U32(o, 0x1c6) = 0;
        }
        /* x87 at 53-bit precision: each step rounds to double, the fstp to float */
        if (twin)
            d = (float)(((double)a - (double)*(float *)(o + 0x74)) / (double)GF(0x1011105c));
        else
            d = a - *(float *)(o + 0x74);
        if (k == 0)
            w = (float)SIM_FN(W10FmodFn, 0x100fcb30)((double)*(float *)(o + 0x64) + (double)*(float *)(rec + 0x1c) +
                                                         (double)*(float *)(o + 0xec) - (double)base, 360.0);
        else
            w = (float)SIM_FN(W10FmodFn, 0x100fcb30)((double)*(float *)(o + 0x64) + (double)*(float *)(rec + 0x1c) +
                                                         (double)*(float *)(o + 0xec) + (double)*(float *)(o + 0x1c6),
                                                     360.0);
        Sim_1000ae6e(rec, &f1, &f2);
        if (ok != 0 && Sim_1000b01f(d, f1, w, f2) != 0 && r == 0) {
            if (Sim_1000a6f0(obj, 1) != 0) {
                if (U32(o, 0x1c2) != 0)
                    U8(U32(o, 0x54), 0x28) = 1;
                else
                    U8(U32(o, 0x54), 0x26) = 1;
                result = 1;
            }
            if (Sim_1000c6e3(rec, S32(rec, 0x4fc), 0x45) != 0) {
                U8(U32(o, 0x54), 0x26) = 1;
                result = 1;
            }
        }
    }
    if (flag != 0 && result != 0)
        Sim_1000c172(obj);
    if (result == 0)
        r = 0;
    if (twin) {
        *(float *)(BYTES(U32(o, 0x54)) + 4) =
            (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, a, r) * (double)GF(0x10111064) /
                    (double)GF(0x10111058));
        if (k == 0)
            *(float *)BYTES(U32(o, 0x54)) =
                (float)((double)-Sim_100144bb((uint32_t)(uintptr_t)obj, *(float *)(o + 0xec) - base, r) *
                        (double)GF(0x10111068));
        else
            *(float *)BYTES(U32(o, 0x54)) =
                (float)((double)-Sim_100144bb((uint32_t)(uintptr_t)obj, *(float *)(o + 0xec) + *(float *)(o + 0x1c6), r) *
                        (double)GF(0x10111068));
    } else {
        *(float *)(BYTES(U32(o, 0x54)) + 4) =
            (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, a, r) * (double)GF(0x10111064));
        *(float *)BYTES(U32(o, 0x54)) =
            (float)((double)-Sim_100144bb((uint32_t)(uintptr_t)obj, *(float *)(o + 0xec), r) * (double)GF(0x10111068));
    }
    if (result != 0) {
        if (entry != 0)
            U32(rec, 0x4dc) = G32(0x1015dc78) + U32(entry, 0x54);
        else
            U32(rec, 0x4dc) = G32(0x1015dc78) + 0xbu;
    }
    return result;
}

/* the original's fcomp test "equal": also holds for an unordered compare */
#define W10_EQUAL(a, b) (!((a) < (b) || (a) > (b)))

// FUNCTION: HGSIM 0x1000b070
int __cdecl Sim_1000b070(uint8_t *obj, float heading)
{
    uint8_t *rec = BYTES(U32(obj, 0x28));
    uint32_t flag = 0, entry = 0, w;
    int fired = 0, shoot = 0, kind = 0, hit;
    int32_t r = 0;
    float v28 = 0.0f, v3c = 0.0f, dist, out[3], v30, v8, v40, arg;
    double base;

    if (S32(obj, 0x1be) <= GS32(0x1015dc78)) {
        r = Sim_1009c929((int8_t)U8(obj, 0x1b6));
        U32(obj, 0x1be) = G32(0x1015dc78) + (uint32_t)r * 11u;
        if (Sim_1000ceb7(obj) != 0) {
            if (U32(obj, 0x1c2) != 0)
                hit = Sim_1000bf73(obj, (int *)&flag);
            else
                hit = Sim_1000bba7(obj, &flag);
            if (hit != 0)
                shoot = 1;
            else
                shoot = Sim_1000b9f1(rec, 0, 0, &entry, 0);
        } else {
            if (Sim_1000c749(rec, S32(rec, 0x4fc), 1) != 0)
                flag = 1;
            shoot = Sim_1000b9f1(rec, 0, 0, &entry, 0);
        }
        kind = Sim_1000c749(rec, S32(rec, 0x4fc), 0x40);
        if (kind == 1) {
            w = U32(rec, 0x84) + U32(rec, 0x4fc) * 0x128u;
            Sim_1000ca63(S32(w, 4), *(float *)(obj + 0xd4), &v3c);
            U32(obj, 0x1c6) = W10_Bits(v3c);
        } else {
            U32(obj, 0x1c6) = 0;
        }
        Sim_1000c1ea(obj, &heading, &dist, out);
        if (U32(obj, 0) == 1 || U32(obj, 0) == 9)
            v30 = (float)(((double)heading - *(float *)(obj + 0x74)) / GF(0x1011105c));
        else
            v30 = (float)((double)heading - *(float *)(obj + 0x74));
        base = (double)*(float *)(obj + 0x64) + *(float *)(BYTES(U32(obj, 0x28)) + 0x1c) + *(float *)(obj + 0xec);
        if (kind == 0)
            v8 = (float)SIM_FN(W10FmodFn, 0x100fcb30)(base - v28, 360.0);
        else
            v8 = (float)SIM_FN(W10FmodFn, 0x100fcb30)(base + *(float *)(obj + 0x1c6), 360.0);
        v40 = U32(rec, 0x4fc) == U32(rec, 0x5fb) ? 5.0f : 10.0f;
        if (shoot != 0 && Sim_1000b01f(v30, 1.0f, v8, v40) != 0) {
            Sim_1001403a(obj, (short)U16(obj, 0x1aa));
            if (r == 0) {
                if (Sim_1000a6f0(obj, 1) != 0) {
                    if (U32(obj, 0x1c2) != 0)
                        U8(U32(obj, 0x54), 0x28) = 1;
                    else
                        U8(U32(obj, 0x54), 0x26) = 1;
                    fired = 1;
                }
                if (Sim_1000c6e3(rec, S32(rec, 0x4fc), 0x45) != 0) {
                    U8(U32(obj, 0x54), 0x26) = 1;
                    fired = 1;
                }
            }
        }
    }
    if (flag != 0 && fired)
        Sim_1000c172(obj);
    if ((U16(obj, 0x1aa) & 0xf00) == 0x200 &&
        !W10_EQUAL(*(float *)(BYTES(U32(PTRTAB(U16(obj, 0x1aa) & 0xff), 0x54)) + 8), GF(0x10111060)))
        r = 0;
    if (!fired)
        r = 0;
    *(float *)(BYTES(U32(obj, 0x54)) + 4) = (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, heading, r) *
                                                     GF(0x10111064));
    if (U32(obj, 0) == 1 || U32(obj, 0) == 9) {
        *(float *)(BYTES(U32(obj, 0x54)) + 4) =
            (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, heading, r) * GF(0x10111064) / GF(0x10111058));
        if (kind == 0)
            arg = (float)((double)*(float *)(obj + 0xec) - v28);
        else
            arg = (float)((double)*(float *)(obj + 0xec) + *(float *)(obj + 0x1c6));
        *(float *)BYTES(U32(obj, 0x54)) =
            (float)(-(double)Sim_100144bb((uint32_t)(uintptr_t)obj, arg, r) * GF(0x10111068));
    } else {
        *(float *)(BYTES(U32(obj, 0x54)) + 4) = (float)((double)Sim_1001448d((uint32_t)(uintptr_t)obj, heading, r) *
                                                         GF(0x10111064));
        *(float *)BYTES(U32(obj, 0x54)) =
            (float)(-(double)Sim_100144bb((uint32_t)(uintptr_t)obj, *(float *)(obj + 0xec), r) * GF(0x10111068));
    }
    if (fired) {
        if (entry != 0)
            U32(rec, 0x4dc) = G32(0x1015dc78) + U32(entry, 0x54);
        else
            U32(rec, 0x4dc) = G32(0x1015dc78) + 11u;
    }
    return fired;
}