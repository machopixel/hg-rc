#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "w7l.h"
#include "../unsorted_1006/w5a.h"
#include "../unsorted_1006/w9d.h"
#include "../unsorted_1006/w9g.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w5y.h"
#include "../unsorted_1009/w7o.h"
#include "w2e.h"
#include "w3q.h"
#include "../unsorted_1005/w2i.h"
#include "../unsorted_1006/w5b.h"
#include "../unsorted_1007/w8w.h"
#include "../unsorted_1009/w3t.h"
#include "../unsorted_1006/w10.h"
#include "w6d.h"
#include "../render/w2n.h"
#include "../libm/hg_math.h"
#include "../crt/hg_winheap.h"

/* The original's fcomp tests: "not below" fails for an unordered compare, "equal" holds for it. */
#define W10_2NOT_BELOW(a, b) ((a) >= (b))
#define W10_2ABOVE(a, b) ((a) > (b))
#define W10_2EQUAL(a, b) (!((a) < (b) || (a) > (b)))
#define VX(v) (*(const float *)((v) + 0x10))
#define VY(v) (*(const float *)((v) + 0x14))

typedef union {
    float f;
    uint32_t u;
} W10_2Word;

// FUNCTION: HGSIM 0x1002f021
int __cdecl Sim_1002f021(const uint8_t *poly, const uint8_t *verts, float x, float y)
{
    int32_t n = U16(poly, 2);
    const uint8_t *idx, *cur, *prev;
    uint32_t sides = 0;
    int left = 0, right = 0;
    int32_t i;
    W10_2Word t;

    if (n < 3)
        return 0;
    idx = poly + U32(poly, 4);
    for (i = n; i-- != 0;) {
        cur = verts + idx[i] * 0x58u;
        sides |= W10_2ABOVE(VX(cur), x) ? 2u : 1u;
        sides |= W10_2ABOVE(VY(cur), y) ? 8u : 4u;
        if (sides == 0xf)
            break;
    }
    if (sides != 0xf)
        return 0;
    prev = verts + idx[0] * 0x58u;
    for (i = n; i-- != 0; prev = cur) {
        cur = verts + idx[i] * 0x58u;
        if (!W10_2NOT_BELOW(VY(cur), y) && !W10_2NOT_BELOW(VY(prev), y))
            continue;
        if (W10_2ABOVE(VY(cur), y) && W10_2ABOVE(VY(prev), y))
            continue;
        if (W10_2EQUAL(VY(cur), y) && W10_2EQUAL(VY(prev), y)) {
            if (!W10_2NOT_BELOW(VX(cur), x) && !W10_2NOT_BELOW(VX(prev), x))
                return 0;
            if (W10_2ABOVE(VX(cur), x) && W10_2ABOVE(VX(prev), x))
                return 0;
            return 1;
        }
        if (W10_2EQUAL(VY(cur), y))
            continue;
        if (!W10_2NOT_BELOW(VX(cur), x) && !W10_2NOT_BELOW(VX(prev), x)) {
            if (left)
                return 0;
            if (right)
                return 1;
            left = 1;
            continue;
        }
        if (W10_2ABOVE(VX(cur), x) && W10_2ABOVE(VX(prev), x)) {
            if (right)
                return 0;
            if (left)
                return 1;
            right = 1;
            continue;
        }
        /* x87 at 53-bit precision: each step rounds to double, the fstp to float */
        t.f = (float)(((double)VX(prev) - (double)VX(cur)) * ((double)y - (double)VY(cur)) /
                          ((double)VY(prev) - (double)VY(cur)) +
                      ((double)VX(cur) - (double)x));
        if (t.u & 0x80000000u) {
            if (left)
                return 0;
            if (right)
                return 1;
            left = 1;
            continue;
        }
        if (!W10_2ABOVE(t.f, *SIM_PTR(float, 0x101112d4)))
            return 1;
        if (right)
            return 0;
        if (left)
            return 1;
        right = 1;
    }
    return 0;
}

typedef void(__cdecl *W10_2CameraFn)(void *camera);
typedef void(__cdecl *W10_2PanelFn)(uint32_t panel);
typedef double(__cdecl *W10_2FmodFn)(double, double);

#define GF(addr) (*SIM_PTR(float, addr))
#define W10_2FMOD hg_fmod

/* The original subtracts in the 53-bit register, tests that value against +-180 and wraps the stored float. */
static float W10_2Wrap(double d53)
{
    float f = (float)d53;

    if (d53 > (double)GF(0x101112b0))
        f = f - GF(0x101112b4);
    else if (!(d53 >= (double)GF(0x101112b8)))
        f = f + GF(0x101112b4);
    return f;
}

static void W10_2Panels(uint32_t off)
{
    int i;

    for (i = 0; i < 0x1e; i++) {
        uint32_t p = SIM_PTR(u32a, 0x109e88e0)[i];

        if (U32(p, off) != 0)
            ((W10_2PanelFn)(uintptr_t)U32(SIM_PTR(u32a, 0x109e88e0)[i], off))(SIM_PTR(u32a, 0x109e88e0)[i]);
    }
}

// FUNCTION: HGSIM 0x1002c1f9
void __cdecl Sim_1002c1f9(uint8_t *obj)
{
    uint32_t unit = U32(obj, 0);
    float d, e, q, len;
    int replay;

    if (U32(unit, 4) != G32(0x1017b8c8))
        return;
    Sim_1006561a();
    if (G32(0x10144fa0) == 0)
        return;
    if (G32(0x1017e618) != 0)
        SIM_FN(W10_2CameraFn, 0x100d1f90)((void *)(uintptr_t)G32(0x1014cae0));
    G32(0x109e89d8) = U32(obj, 0x4f0);
    if (G32(0x109e89d8) != 2 && U32(G32(0x109e8920), 0x13c) != 0)
        ((W10_2PanelFn)(uintptr_t)U32(G32(0x109e8920), 0x13c))(G32(0x109e8920));
    if (G32(0x109e89d8) == 5 || G32(0x109e89d8) == 4)
        return;
    if (G32(0x10144fa0) == 0)
        return;
    GF(0x109e88bc) = (float)W10_2FMOD(W10_2FMOD((double)*(float *)(BYTES(unit) + 0x68), 360.0) +
                                          *SIM_PTR(double, 0x101112a8), 360.0);
    GF(0x109e88b8) = (float)W10_2FMOD((double)*(float *)(obj + 0xc), 360.0);
    q = (float)W10_2FMOD((double)*(float *)(obj + 0x1c) + (double)*(float *)(BYTES(unit) + 0xec), 360.0);
    d = W10_2Wrap((double)*(float *)(BYTES(unit) + 0xe8) - (double)GF(0x109e88bc));
    e = W10_2Wrap((double)d - (double)GF(0x109e88b8));
    len = (float)Sim_10084a94(*(float *)(BYTES(unit) + 0xdc) - *(float *)(BYTES(unit) + 0x58),
                              *(float *)(BYTES(unit) + 0xe0) - *(float *)(BYTES(unit) + 0x5c),
                              *(float *)(BYTES(unit) + 0xe4) - *(float *)(BYTES(unit) + 0x60));
    if (G32(0x1015dd10) != 0) {
        if (G32(0x109e89d8) == 2)
            Sim_10069f68(obj);
        G32(0x1015dd10) = 0;
    }
    if (G32(0x1015f1d8) != 0 && G32(0x109e89d8) != 3 && (U16(obj, 0x560) & 4) != 0 && (U16(obj, 0x560) & 8) == 0)
        U16(obj, 0x560) = (uint16_t)(U16(obj, 0x560) | 8);
    G32(0x1015f1d8) = 0;
    Sim_1002c76d(obj);
    replay = U8(G32(0x1017b8c4), 9) != 0 || G32(0x1015ec58) == 0;
    switch (G32(0x109e89d8)) {
    case 2: {
        union {
            float f;
            uint32_t u;
        } a1, a2, a3, a4, a5, a6;

        if (replay)
            Sim_100999bc();
        W10_2Panels(0x13c);
        a1.f = GF(0x109e88bc);
        a2.f = GF(0x109e88b8);
        a3.f = d;
        a4.f = e;
        a5.f = q;
        a6.f = len;
        Sim_100622f0(obj, a1.u, a2.u, a3.u, a4.u, a5.u, a6.u);
        break;
    }
    case 1:
        if (replay)
            Sim_1009b99b();
        W10_2Panels(0x138);
        break;
    default:
        if (replay)
            Sim_1009bc94();
        W10_2Panels(0x140);
        break;
    }
}

typedef void(__cdecl *W10_2PanelArgFn)(uint32_t panel, uint32_t arg);
typedef void(__cdecl *W10_2ReadIntFn)(void *table, int32_t *in, int32_t *out);

#define W10_2PANEL(i) G32(0x109e88e0 + 4 * (i))
#define W10_2PANEL_CALL(i, off, arg) \
    ((W10_2PanelArgFn)(uintptr_t)U32(W10_2PANEL(i), off))(W10_2PANEL(i), (uint32_t)(arg))
#define W10_2ADDR(addr) ((uint32_t)(uintptr_t)SIM_PTR(void, addr))

/* The four routines of a panel (+0x13c, +0x144, +0x138, +0x140), its kind +0xc and the mode of its +0x11c. */
static void W10_2PanelSet(int i, uint32_t r13c, uint32_t r144, uint32_t r138, uint32_t r140, uint32_t kind,
                          uint32_t mode)
{
    U32(W10_2PANEL(i), 0x13c) = r13c;
    U32(W10_2PANEL(i), 0x144) = r144;
    U32(W10_2PANEL(i), 0x138) = r138;
    U32(W10_2PANEL(i), 0x140) = r140;
    U32(W10_2PANEL(i), 0xc) = kind;
    W10_2PANEL_CALL(i, 0x11c, mode);
}

// FUNCTION: HGSIM 0x1002bca0
void __cdecl Sim_1002bca0(void)
{
    uint32_t widgets = U32(G32(0x109e8b40 + 4 * G32(0x1017b8c8)), 0x28);
    int32_t v, scaled;
    int i;

    for (i = 0; i < 30; i++)
        G32(0x109e8960 + 4 * i) = 1;
    G32(0x109e89b0) = 0;
    G32(0x109e89b4) = 0;
    G32(0x109e89b8) = 0;
    G32(0x109e89bc) = 0;
    G32(0x109e89c8) = 0;
    G32(0x109e8968) = 0;
    if (U8(G32(0x1017b8c4), 9) == 0 && G32(0x1015ec58) != 0)
        G32(0x109e898c) = 0;
    for (i = 0; i < 30; i++) {
        W10_2PANEL(i) = (uint32_t)(uintptr_t)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0x148);
        Sim_10028810((void *)(uintptr_t)W10_2PANEL(i));
        W10_2PANEL_CALL(i, 0x10c, G32(0x101381d0 + 4 * i));
        W10_2PANEL_CALL(i, 0x118, 0x109e8660 + 0x14 * i);
        W10_2PANEL_CALL(i, 0x120, 0x109e8240 + 8 * i);
        W10_2PANEL_CALL(i, 0x124, G32(0x10138158 + 4 * i));
        W10_2PANEL_CALL(i, 0x11c, 0);
        if (G32(0x109e8960 + 4 * i) != 0)
            ((W10_2PanelFn)(uintptr_t)U32(W10_2PANEL(i), 0x130))(W10_2PANEL(i));
        else
            ((W10_2PanelFn)(uintptr_t)U32(W10_2PANEL(i), 0x134))(W10_2PANEL(i));
    }
    Sim_1002b68f((const void *)(uintptr_t)widgets);
    W10_2PanelSet(18, W10_2ADDR(0x10003265), W10_2ADDR(0x10002c7a), W10_2ADDR(0x10001640), W10_2ADDR(0x100023ce), 6,
                  9);
    W10_2PanelSet(1, W10_2ADDR(0x10003265), W10_2ADDR(0x10002c7a), W10_2ADDR(0x10001640), W10_2ADDR(0x100023ce), 3, 1);
    U32(W10_2PANEL(11), 0x13c) = W10_2ADDR(0x10001956);
    U32(W10_2PANEL(11), 0x144) = W10_2ADDR(0x10001550);
    U32(W10_2PANEL(12), 0x13c) = W10_2ADDR(0x10002e37);
    U32(W10_2PANEL(11), 0x138) = W10_2ADDR(0x10002c02);
    U32(W10_2PANEL(11), 0x140) = W10_2ADDR(0x10002329);
    U32(W10_2PANEL(11), 0xc) = 7;
    W10_2PANEL_CALL(11, 0x11c, 8);
    U32(W10_2PANEL(13), 0x13c) = W10_2ADDR(0x10002d88);
    U32(W10_2PANEL(14), 0x13c) = W10_2ADDR(0x1000246e);
    U32(W10_2PANEL(16), 0x13c) = W10_2ADDR(0x1000128f);
    U32(W10_2PANEL(17), 0x13c) = W10_2ADDR(0x100023b5);
    U32(W10_2PANEL(19), 0x13c) = W10_2ADDR(0x100029e6);
    U32(W10_2PANEL(25), 0x13c) = W10_2ADDR(0x10001e15);
    Sim_10050c5b();
    G32(0x1015ef1c) = widgets + 0x524;
    G32(0x1015ef20) = widgets + 0xc;
    Sim_10077e34();
    Sim_10062a3a();
    if (U8(G32(0x1017b8c4), 9) != 0 || G32(0x1015ec58) == 0)
        Sim_10099928();
    G32(0x1015dc2c) = (uint32_t)(uintptr_t)Hg_HeapAlloc(G32(0x1017b8f8), 1, 0xfe01);
    G32(0x1015dc38) = 0x51f;
    v = (int32_t)G32(0x1015dc38);
    SIM_FN(W10_2ReadIntFn, 0x1008cdc9)(SIM_PTR(void, 0x109e3e00), &v, &v);
    G32(0x1015dc38) = (uint32_t)v;
    /* (x << 16, sign-extended and shifted 16 more) / 0x320000, the original's idiv */
    scaled = (int32_t)(G32(0x109e3e1c) << 16);
    G32(0x1017e5e8) = (uint32_t)(int32_t)(((int64_t)scaled * 65536) / 0x320000);
}

// FUNCTION: HGSIM 0x100255de
void __cdecl Sim_100255de(int32_t killer, float x, float y, float z, float radius, float rate)
{
    uint8_t shot[0x156];      /* the original's shot record at ebp-0x164 (0x156 bytes, as the shots 0x109a3f80) */
    int32_t i = GS32(0x1015dc64), part;
    uint32_t unit;
    uint8_t *rec;
    float dx, dy, dz, r;

    while (i-- != 0) {
        unit = PTRTAB(i);
        if (G32(0x1015ecd0) != 0 && !(U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)) &&
            (uint32_t)i != G32(0x1017b8c8))
            continue;
        rec = BYTES(U32(unit, 0x28));
        if ((U16(rec, 0x560) & 0x100) != 0)
            continue;
        dx = (float)((double)*(float *)BYTES(unit + 0x58) - x);
        dy = (float)((double)*(float *)BYTES(unit + 0x5c) - y);
        dz = (float)((double)*(float *)BYTES(unit + 0x60) - z);
        r = (float)((double)*(float *)(rec + 0x53c) + radius);
        if (!(Sim_10084a94(dx, dy, dz) >= (double)r)) {
            /* a blast: parts 2..6 take the frame's damage, part 1 a quarter of it */
            U32(shot, 0x86) = 0;
            U32(shot, 0x70) = (uint32_t)Render_Ftol((double)GS32(0x1015dc80) * rate);
            U32(shot, 0x74) = 0x3dcccccd;
            U32(shot, 0x78) = 0;
            U16(shot, 0x7c) = 1;
            for (part = 2; part <= 6; part++)
                Sim_1006c161(killer, rec, 0, shot, part);
            U32(shot, 0x70) >>= 2;
            Sim_1006c161(killer, rec, 0, shot, 1);
        }
    }
}

// FUNCTION: HGSIM 0x10026057
void __cdecl Sim_10026057(void)
{
    double r;

    if (GS32(0x1015dc0c) <= 0)
        return;
    /* the blast grows by 200 per frame time unit up to [0x1015dbd0] */
    r = (double)(int32_t)(G32(0x1015dc80) * 0xc8u) + *SIM_PTR(float, 0x1015dbf4);
    *SIM_PTR(float, 0x1015dbf4) = (float)r;
    if (r > (double)*SIM_PTR(float, 0x1015dbd0))
        G32(0x1015dbf4) = G32(0x1015dbd0);
    Sim_100255de(-2, *SIM_PTR(float, 0x1015dbf8), *SIM_PTR(float, 0x1015dbfc), *SIM_PTR(float, 0x1015dc00),
                 *SIM_PTR(float, 0x1015dbf4), 0.0625f);
    Sim_100257ec(0xfffffffeu, *SIM_PTR(float, 0x1015dbf8), *SIM_PTR(float, 0x1015dbfc), *SIM_PTR(float, 0x1015dc00),
                 *SIM_PTR(float, 0x1015dbf4), 0.0625f);
    G32(0x1015dc0c) -= G32(0x1015dc80);
}