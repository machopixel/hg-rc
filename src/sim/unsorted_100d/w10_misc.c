#include "w10.h"
#include "../fx/w32.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1005/w8v.h"
#include "../unsorted_100b/w6g.h"
#include "fns.h"
#include "../util/util.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w2g.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../render/w2n.h"
#include "../unsorted_100f/w8z.h"
#include "w3q.h"
#include "w5c.h"
#include "w8z.h"
#include "../unsorted_1000/w7l.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w6f.h"
#include "../libm/hg_math.h"

typedef void(__cdecl *W10dMethodFn)(void *pane, const char *text);
typedef void *(__cdecl *W10dLoadFn)(void *name, void *key, uint32_t zero);
typedef void(__cdecl *W10dTextFn)(uint32_t target, uint32_t x, uint32_t y, void *font, const char *text,
                                   void *colours);

// FUNCTION: HGSIM 0x100d0198
void __cdecl Sim_100d0198(uint8_t *pane)
{
    void *font;
    uint32_t pos, x, y, target;
    const char *text;

    if (S16(pane, 4) == 0)
        return;
    ((W10dMethodFn)(uintptr_t)U32(pane, 0x114))(pane, SIM_PTR(const char, 0x10151e74));
    font = SIM_FN(W10dLoadFn, 0x100ba7d5)((void *)(uintptr_t)G32(0x109b9900), (void *)(uintptr_t)G32(0x1014cec4), 0);
    if (font == 0)
        return;
    text = (const char *)pane + 0x10;
    pos = U32(G32(0x109e8934), 0xf0);
    y = U32(pos, 4);
    x = U32(pos, 0);
    target = U32(pane, 0xec);
    if (G32(0x101668e8) != 0)
        SIM_FN(W10dTextFn, 0x100f2f01)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10dTextFn, 0x100f1e10)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
    Sim_10051532(pane);
}

#define GF(addr) (*SIM_PTR(float, addr))
#define SIM_ADDR(addr) ((uint32_t)(uintptr_t)SIM_PTR(void, addr))

static float W10dAsFloat(uint32_t u)
{
    union {
        float f;
        uint32_t u;
    } v;

    v.u = u;
    return v.f;
}

// FUNCTION: HGSIM 0x100d965e
void __cdecl Sim_100d965e(uint32_t x, uint32_t y, uint32_t z, float vx, float vy, float vz, void *owner)
{
    const float *cam = (const float *)(uintptr_t)G32(0x1014cae0);
    uint32_t *slot;
    uint8_t *obj;
    uint32_t rec;
    float sx, sy, sz, px, py, pz;
    int i;

    if ((double)Sim_10084a94(W10dAsFloat(x) - cam[0], W10dAsFloat(y) - cam[1], W10dAsFloat(z) - cam[2]) >
        (double)cam[0x10])
        return;
    slot = (uint32_t *)SlotPool_Take(owner);
    if (slot == 0)
        return;
    obj = Sim_10090c7d(SIM_ADDR(0x100034e0), SIM_ADDR(0x100034b8), (uint32_t)(uintptr_t)slot, 0);
    if (obj == 0) {
        SlotPool_Release(owner, (int)slot[0]);
        return;
    }
    slot[1] = U32(obj, 8);
    *(float *)&slot[5] = (float)(Sim_1009c929(0x1f4) - 0xfa);
    slot[6] = 0;
    *(float *)&slot[7] = (float)(Sim_1009c929(0x1f4) - 0xfa);
    U16(obj, 0x10) = 1;
    rec = U32(U32(obj, W32_REC_OBJ), 0x8c);
    *(float *)(BYTES(rec) + 0x50) = (float)Sim_10084a94(vx, vy, vz);
    U32(rec, 0x44) = x;
    U32(rec, 0x48) = y;
    U32(rec, 0x4c) = z;
    U32(rec, 0x38) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x40) = 0;
    *(float *)(obj + W32_REC_VEL) = vx;
    *(float *)(obj + W32_REC_VEL + 4) = vy;
    *(float *)(obj + W32_REC_VEL + 8) = vz;
    sx = vx / GF(0x1012bd6c);
    sy = vy / GF(0x1012bd6c);
    sz = vz / GF(0x1012bd6c);
    px = sx;
    py = sy;
    pz = sz;
    Sim_100aa39f((void *)(uintptr_t)U32(obj, W32_REC_OBJ));
    Sim_100a9d47((void *)(uintptr_t)U32(obj, W32_REC_OBJ), x, y, z);
    Sim_100aa799((void *)(uintptr_t)U32(obj, W32_REC_OBJ));
    U32(obj, W32_REC_TIME) = G32(0x1015dc78);
    /* 1.2 (0x100beafe): 16 points of 0x32 bytes in its particle layout (1.0: 32 of 0x2a) */
#ifdef W10W_MUTATE /* the checker's negative check only: 1.0's 32 points */
    for (i = 0; i < 0x20; i++) {
#else
    for (i = 0; i < W32_REC_NPT; i++) {
#endif
        uint8_t *p = obj + 0x12 + i * W32_REC_PT;

        U16(p, 0x18) = 1;
        px = px + sx;
        py = py + sy;
        pz = pz + sz;
        /* x87 at 53-bit precision: int + float - float rounds to double, then to float */
        *(float *)p = (float)((double)Sim_1009c929(10) + (double)px - (double)GF(0x1012bd70));
        *(float *)(p + 4) = (float)((double)Sim_1009c929(0x32) + (double)py - (double)GF(0x1012bd74));
        *(float *)(p + 8) = (float)((double)Sim_1009c929(10) + (double)pz - (double)GF(0x1012bd70));
    }
}

typedef int32_t(__cdecl *W10dHeightFn)(void *font);

static void W10dText(uint32_t target, uint32_t x, uint32_t y, void *font, const char *text)
{
    if (G32(0x101668e8) != 0)
        SIM_FN(W10dTextFn, 0x100f2f01)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
    else
        SIM_FN(W10dTextFn, 0x100f1e10)(target, x, y, font, text, SIM_PTR(void, 0x109b9920));
}

// FUNCTION: HGSIM 0x100d02bf
void __cdecl Sim_100d02bf(uint8_t *pane)
{
    uint32_t rec = U32(PTRTAB(G32(0x1017b8c8)), 0x28);
    char text[0x40];     /* the original's 0x40-byte buffer at ebp-0x44 */
    W10dMethodFn method;
    void *font;
    int32_t dist;
    uint32_t pos, y;

    if (S16(pane, 4) == 0)
        return;
    font = SIM_FN(W10dLoadFn, 0x100ba7d5)((void *)(uintptr_t)G32(0x109b9900), (void *)(uintptr_t)G32(0x1014cec4), 0);
    if (font == 0)
        return;
    dist = Render_Ftol(((double)*(float *)(BYTES(U32(rec, 0)) + 0x5c) - (double)*(float *)(BYTES(rec) + 0x520)) *
                       (double)GF(0x1012bc88));
    method = (W10dMethodFn)(uintptr_t)U32(pane, 0x114);
    method(pane, (const char *)(uintptr_t)G32(0x101c4c1c));
    pos = U32(G32(0x109e8938), 0xf0);
    y = (uint32_t)SIM_FN(W10dHeightFn, 0x100ecce0)(font);
    y += U32(pos, 4);
    y += (uint32_t)SIM_FN(W10dHeightFn, 0x100ecce0)(font);
    W10dText(U32(pane, 0xec), U32(U32(G32(0x109e8938), 0xf0), 0), y, font, (const char *)pane + 0x10);
    Crt_SPrintf(text, SIM_PTR(const char, 0x10151e7c), (double)dist / *SIM_PTR(double, 0x1012bc90));   /* "%.2f m" */
    pos = U32(G32(0x109e8938), 0xf0);
    y = (uint32_t)SIM_FN(W10dHeightFn, 0x100ecce0)(font);
    y += U32(pos, 4);
    y += (uint32_t)SIM_FN(W10dHeightFn, 0x100ecce0)(font);
    y += (uint32_t)SIM_FN(W10dHeightFn, 0x100ecce0)(font);
    W10dText(U32(pane, 0xec), U32(U32(G32(0x109e8938), 0xf0), 0), y, font, text);
    if (S32(U32(PTRTAB(G32(0x1017b8c8)), 0x28), 0x510) < 0)
        return;
    method = (W10dMethodFn)(uintptr_t)U32(pane, 0x114);
    method(pane, (const char *)(uintptr_t)G32(0x101c4c28));
    W10dText(U32(pane, 0xec), U32(U32(G32(0x109e8938), 0xf0), 0), U32(U32(G32(0x109e8938), 0xf0), 4), font,
             (const char *)pane + 0x10);
    Sim_100bb2b0((const uint32_t *)(uintptr_t)G32(0x109b9900), (const char *)(uintptr_t)G32(0x1014cec4));
    Sim_100517dc(pane);
}


#define W10D_SHOT(i) (0x109a3f80u + (uint32_t)(i) * 0x156u)

// FUNCTION: HGSIM 0x100d4f71
int __cdecl Sim_100d4f71(uint8_t *obj, const uint8_t *weapon, uint8_t *slot)
{
    uint8_t seg[0x44];        /* the original's 0x44-byte segment at ebp-0x58 (Sim_100d6f16 fills +0..+0x37) */
    float a, b, c;            /* Sim_100a9cfb's three results (unused) */
    int ret = 1, mine, found = 0;
    uint8_t *rec, *shot = 0;
    uint32_t self = G32(0x1017b8c8);
    int32_t i;

    (void)*(volatile uint32_t *)&U32(PTRTAB(self), 0x28);      /* read by the original, not used */
    if (obj == 0)
        return 0;
    rec = BYTES(U32(obj, 0x28));
    if (U32(rec, 0xc3d) != 0)
        return 0;
    mine = U32(U32(rec, 0), 4) == self;
    if (mine) {
        *SIM_PTR(u16a, 0x101653f3) += 1;
#ifndef W10F_MUTATE                     /* W10F_MUTATE: the checker's negative check only */
        if (G32(0x1015ef3c) != 0)       /* 1.2 (0x100bafae): its force-feedback request for the own shot */
            Sim_100382fb(U32(weapon, 0xd2), 0, 0, 0);
#endif
    }
    if (S32(slot, 0x20) != -1 && !(U8(G32(0x1017b8c4), 0) != 0 && U32(obj, 4) == self) &&
        (G32(0x1015ec58) == 0 || (U32(obj, 0x14) == 3 && U32(obj, 0xc) == self) || U32(obj, 4) == self)) {
        if (S32(slot, 0x20) == -2) {
            U32(slot, 8) = 0xffffffffu;
            return 1;
        }
        U32(slot, 0x20) -= 1;
        U16(U32(slot, 0x110), 2) -= 1;
        if (S16(U32(slot, 0x110), 2) <= 0) {
            U32(slot, 0x108) += 1;
            if (U32(slot, 0x108) == U32(slot, 0x38))
                U32(slot, 0x108) -= 1;
            else
                U32(slot, 0x110) += 0x24;
        }
        if (S32(slot, 0x20) <= 0) {
            U32(slot, 0x20) = 0xfffffffeu;
            U32(slot, 8) = 0xffffffffu;
            ret = 0;
        }
    }
    if (U32(PTRTAB(U32(obj, 4)), 8) == U32(PTRTAB(self), 8))
        *SIM_PTR(u16a, 0x1016540c) += 1;
    for (i = 0; !found && i < 20; i++) {
        shot = GBYTES(W10D_SHOT(i));
        if (U32(shot, 0x48) == 0)
            found = 1;
    }
    if (!found)
        return ret;
    /* the original's further tests for the case "mine" can never hold there */
    if (!mine)
        Sim_100d6d6a(obj);
    U32(shot, 0x54) = (uint32_t)(uintptr_t)slot;
    U32(shot, 0x86) = (uint32_t)(uintptr_t)weapon + 0xc2u;
    U32(shot, 4) = U32(weapon, 8);
    U32(shot, 0x30) = U32(slot, 0x10);
    U32(shot, 0x34) = U32(weapon, 0x5c) - U32(slot, 0x10);
    U32(shot, 8) = U32(weapon, 0x10);
    U32(shot, 0x58) = U32(weapon, 0x40);
    U32(shot, 0x5c) = U32(weapon, 0x48);
    U32(shot, 0x48) = 1;
    U32(shot, 0x4c) = U16(weapon, 0x76);
    U32(shot, 0x40) = U32(slot, 0x18);
    U32(shot, 0x44) = U32(slot, 0x1c);
    U32(shot, 0x50) = U32(obj, 4);
    U32(shot, 0x3c) = U32(rec, 0x498u + U32(slot, 0x11c) * 4u);
    U32(shot, 0x8e) = U32(slot, 4);
    U32(shot, 0x70) = U32(weapon, 0x78) + U32(slot, 0x28);
    if ((U32(rec, 0xbd3) & 0x400) != 0)
        U32(shot, 0x70) += 2;
    U32(shot, 0x74) = U32(weapon, 0x7c);
    U32(shot, 0x78) = U32(weapon, 0x80);
    U16(shot, 0x7c) = U16(weapon, 0x84);
    U32(shot, 0x7e) = U32(weapon, 0x86);
    Sim_100d6f16(obj, seg);
    Sim_1009cd36(seg, (float)Sim_100d6e99(obj));
    U32(shot, 0x18) = U32(seg, 0xc);
    U32(shot, 0x1c) = U32(seg, 0x10);
    U32(shot, 0x20) = U32(seg, 0x14);
    Sim_100a9cfb((void *)(uintptr_t)U32(shot, 0x3c), (uint32_t *)&a, (uint32_t *)&b, (uint32_t *)&c);
    if (U32(weapon, 0x10) == 6) {
        for (i = 0; i < 15; i++)
            Sim_1009201f((uint32_t)(uintptr_t)shot + 0x92u + (uint32_t)i * 0xcu, (int)(uintptr_t)shot,
                         (uint32_t)i * 0x16u, 0x1f);
    } else if (U32(weapon, 0x10) == 4) {
        for (i = 0; i < 15; i += 2)
            Sim_10091085((uint32_t)(uintptr_t)shot + 0x92u + (uint32_t)i * 0xcu, (int)(uintptr_t)shot,
                         (uint32_t)i * 0x16u);
    } else {
        for (i = 0; i < 15; i++)
            Sim_10092956((uint32_t)(uintptr_t)shot + 0x92u + (uint32_t)i * 0xcu, shot, 7);
    }
    return ret;
}

typedef double(__cdecl *W10dFmodFn)(double x, double y);

#define W10D_FMOD(x) hg_fmod((x), 360.0)
#define W10D_F(p, off) (*(float *)(BYTES(p) + (off)))
#define W10D_FLAGS(b) U16(b, 0x560)

/* d - [0x1012bd24] above [0x1012bd20], d + it below [0x1012bd28] (unordered counts as below); the first test uses the
 * unrounded difference d, the second the stored float */
static float W10d_Wrap(double d)
{
    float f = (float)d;

    if (d > (double)GF(0x1012bd20))
        return (float)((double)f - GF(0x1012bd24));
    if (!(f >= GF(0x1012bd28)))
        return (float)((double)f + GF(0x1012bd24));
    return f;
}

// FUNCTION: HGSIM 0x100d6833
void __cdecl Sim_100d6833(uint8_t *b)
{
    uint8_t *unit = BYTES(U32(b, 0));
    uint32_t kind, node, wdef, w86;
    float dx, dy, dz, r, heading, aim, yaw, pitch, side;
    double d2, t;
    int inrange;

    if ((U32(unit, 0x14) == 2 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8))) &&
        U32(unit, 0x1e8) != 0 && (W10D_FLAGS(b) & 0x80) != 0)
        return;
    kind = U32(unit, 0);
    W10D_FLAGS(b) &= 0xff7f;
    node = U32(unit, 0x48);
    /* the weapon definition of the selected slot, read before the slot is tested (as the original does) */
    wdef = 0x10145508u + *(volatile uint32_t *)&U32(U32(b, 0x84) + U32(b, 0x4fc) * 0x128u, 4) * 0xd6u;
    if (S32(b, 0x4fc) == -1 || U32(unit, 0xf0) == 0 || (U32(unit, 0xf0) & 0x1000) != 0 ||
        (U32(unit, 0xf0) & 0x100) != 0 || (U32(SIM_PTR(uint8_t, wdef), 0x86) & 1) == 0) {
        W10D_FLAGS(b) &= 0x7fff;
        W10D_FLAGS(b) &= 0xffbf;
        return;
    }
    wdef = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, wdef);
    dx = (float)((double)W10D_F(unit, 0x58) - W10D_F(unit, 0xdc));
    dy = (float)((double)W10D_F(unit, 0x5c) - W10D_F(unit, 0xe0));
    dz = (float)((double)W10D_F(unit, 0x60) - W10D_F(unit, 0xe4));
    d2 = (double)dx * dx + (double)dy * dy + (double)dz * dz;
    r = W10D_F(wdef, 0x4c);
    inrange = !((double)r * r >= d2);
    if (inrange) {
        r = W10D_F(wdef, 0x50);
        inrange = (double)r * r >= d2;
    }
    if (!inrange) {
        yaw = 17.0f;
        side = 17.0f;
        W10D_FLAGS(b) &= 0x7fff;
    } else {
        heading = (float)W10D_FMOD(W10D_FMOD((double)W10D_F(unit, 0x68)) + *SIM_PTR(double, 0x1012bd00));
        if ((kind == 1 || kind == 9) && *(const int8_t *)BYTES(U32(node, 0x90)) != 0x53) {
            t = W10D_FMOD((double)W10D_F(unit, 0x74));
            aim = (float)(t * GF(0x1012bd08) /
                          (((double)W10D_F(G32(0x1014cae0), 0x18) - *SIM_PTR(double, 0x1012bd10)) *
                               *SIM_PTR(double, 0x1012bd18) +
                           *SIM_PTR(double, 0x1012bd10)));
        } else {
            aim = (float)W10D_FMOD((double)W10D_F(unit, 0x74));
        }
        pitch = W10d_Wrap((double)W10D_F(unit, 0xe8) - heading);
        yaw = W10d_Wrap((double)pitch - aim);
        side = (float)W10D_FMOD((double)W10D_F(b, 0x1c) + W10D_F(unit, 0xec));
        w86 = U32(wdef, 0x86);
        /* below [0x1012bd30], an unordered compare counting as below */
        if ((!(hg_fabs((double)yaw) >= *SIM_PTR(double, 0x1012bd30)) &&
             !(hg_fabs((double)side) >= *SIM_PTR(double, 0x1012bd30))) ||
            ((w86 & 0x20) != 0 && (w86 & 1) != 0)) {
            W10D_FLAGS(b) |= 0x8000;
            if ((W10D_FLAGS(b) & 0x40) == 0) {
                W10D_FLAGS(b) |= 0x40;
                U32(b, 0x4d8) = 0x16a;
            } else if (S32(b, 0x4d8) <= 0 || (U32(b, 0xbcf) & 0x20) != 0) {
                U32(b, 0x4d8) = 0;
                W10D_FLAGS(b) |= 0x80;
            } else {
                U32(b, 0x4d8) -= G32(0x1015dc80);
            }
            return;
        }
    }
    (void)yaw;
    (void)side;
    W10D_FLAGS(b) &= 0x7fff;
    if ((W10D_FLAGS(b) & 0x40) != 0) {
        U32(b, 0x4d8) += G32(0x1015dc80);
        if (S32(b, 0x4d8) >= 0x16a)
            W10D_FLAGS(b) &= 0xffbf;
    }
}