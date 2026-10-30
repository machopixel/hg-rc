#include "w8t.h"
#include "w2e.h"
#include "w5x.h"
#include "w5z.h"
#include "w6d.h"
#include "w7l.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1005/w2e.h"
#include "../unsorted_1005/w4v.h"
#include "../unsorted_1005/w5b.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w2f.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100b/fns.h"
#include "../unsorted_100d/w2g.h"
#include "../unsorted_100d/w4v.h"
#include "../unsorted_100d/w6h.h"
#include "../unsorted_100d/w7q.h"
#include "../unsorted_100f/w2e.h"
#include "../unsorted_100f/w6h.h"
#include "../util/util.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10020256
void __cdecl Sim_10020256(int32_t kind, int32_t index)
{
    uint8_t *a, *b;

    if (index < 0)
        return;
    if (kind == -1 || kind >= 0xd)
        return;
    a = index != -1 ? GBYTES(0x10133728u + (uint32_t)index * 0x18u) : 0;
    b = GBYTES(0x10133770u + (uint32_t)kind * 0x18u);
    switch (kind) {
    case 1:
        Sim_100200d1(index, 2);
        break;
    case 2:
        Sim_100200d1(index, 7);
        break;
    case 3:
        Sim_100200d1(index, 3);
        break;
    case 4:
        Sim_100200d1(index, 5);
        break;
    case 5:
        Sim_100200d1(index, 6);
        break;
    case 7:
        break;
    case 8:
    case 9:
        Sim_100200d1(index, 4);
        break;
    case 10:
        Sim_100200d1(index, 1);
        break;
    case 11:
        Sim_100200d1(index, 9);
        break;
    case 12:
        Sim_100200d1(index, 8);
        break;
    default:    /* kind 6 and kinds below 1 */
        Sim_1001f790(a, b, -1);
        break;
    }
}

// FUNCTION: HGSIM 0x10021d65
void __cdecl Sim_10021d65(void)
{
    int i;

    if (G32(0x1017694c) != 0)
        U32(G32(0x1017694c), 0) = 0;
    for (i = 0; i < 0x200; i++) {
        uint8_t *e = GBYTES(0x1096fc80) + i * 0x156;

        if (U32(e, 0x48) == 0 || U32(e, 0x3c) == 0)
            continue;
        U32(e, 0x30) += G32(0x1015dc80);
        U32(e, 0x34) -= G32(0x1015dc80);
        if (U32(e, 0x4c) == 0)
            W8T_CALL_100232d7(i);
        else if (U32(e, 0x4c) == 3)
            W8T_CALL_100225c7(i);
    }
    if (G32(0x1017b92c) != 0)
        return;
    for (i = 0; i < 0x14; i++) {
        uint8_t *e = GBYTES(0x109a3f80) + i * 0x156;

        if (U32(e, 0x48) == 0)
            continue;
        U32(e, 0x30) += G32(0x1015dc80);
        U32(e, 0x34) -= G32(0x1015dc80);
        W8T_CALL_10021e8c(i);
    }
}

// FUNCTION: HGSIM 0x1002411e
void __cdecl Sim_1002411e(void *obj, float *a, float *b, float *c)
{
    float x = -FL(obj, 0x20);
    float y = 0.0f;
    float z = FL(obj, 0x18);

    Sim_10084c95(&x, &y, &z);
    *a = (float)(W8T_CALL_10084766_1002(FL(obj, 0x38)) * (double)x + (double)*a);
    *c = (float)(W8T_CALL_10084766_1002(FL(obj, 0x38)) * (double)z + (double)*c);
    *b = (float)(W8T_CALL_10084766_1002((float)((double)FL(obj, 0x38) + (double)GFL(0x1011124c))) * (double)z +
                 (double)*b);
    FL(obj, 0x38) = (float)((double)GS32(0x1015dc80) * (double)GFL(0x10111250) + (double)FL(obj, 0x38));
    FL(obj, 0x38) = (float)W8T_FMOD_1002((double)FL(obj, 0x38), 360.0);
}

// FUNCTION: HGSIM 0x100241ed
void __cdecl Sim_100241ed(void *obj, float x, float y, float z, float *p, float *q, float *r)
{
    union { uint32_t u; float f; } tx, ty, tz, ox, oy, oz;
    float dx, dy, dz, len, height, angle;
    double l, s;

    if (S32(obj, 0x40) < 0)
        return;
    if (U32(obj, 0x44) == 0x200) {
        uint8_t *t = BYTES(PTRTAB(U32(obj, 0x40)));

        if ((S16(t, 0x1c) & 2) != 0 || (S16(t, 0x1c) & 4) != 0) {
            U32(obj, 0x2c) = 0;
            U32(obj, 0x28) = 0;
            U32(obj, 0x24) = 0;
            U32(obj, 0x44) = 0;
            return;
        }
        tx.u = U32(t, 0x58);
        ty.u = U32(t, 0x5c);
        tz.u = U32(t, 0x60);
    } else if (U32(obj, 0x44) == 0x400) {
        uint8_t *e = GBYTES(0x109e9ba0u + U32(obj, 0x40) * 0x50u);

        if ((S16(e, 0) & 4) != 0) {
            U32(obj, 0x2c) = 0;
            U32(obj, 0x28) = 0;
            U32(obj, 0x24) = 0;
            U32(obj, 0x44) = 0;
            return;
        }
        Sim_100b53f0((int)U32(e, 8), &tx.u, &ty.u, &tz.u);
    } else {
        return;
    }
    dx = (float)((double)tx.f - (double)x);
    dy = (float)((double)ty.f - (double)y);
    dz = (float)((double)tz.f - (double)z);
    l = Sim_10084a94(dx, dy, dz);
    len = (float)l;
    if (!(l > (double)GFL(0x10111238)))
        U32(obj, 0x48) |= 0x8000;
    dx = (float)((double)dx / (double)len);
    dy = (float)((double)dy / (double)len);
    dz = (float)((double)dz / (double)len);
    if (!((double)len >= (double)GFL(0x10111234))) {
        *p = (float)((double)*p * (double)GFL(0x10111254));
        *q = (float)((double)*q * (double)GFL(0x10111254));
        *r = (float)((double)*r * (double)GFL(0x10111254));
    }
    ox.u = U32(obj, 0x18);
    oy.u = U32(obj, 0x1c);
    oz.u = U32(obj, 0x20);
    Sim_10084c95(&ox.f, &oy.f, &oz.f);
    if ((U32(obj, 4) & 0x10) != 0) {
        float k = (float)(int32_t)(G32(0x1015dc80) * 4u * 0xb5u);
        float sx = (float)(((double)dx - (double)ox.f - (double)FL(obj, 0x24)) * (double)k);
        float sy = (float)(((double)dy - (double)oy.f - (double)FL(obj, 0x28)) * (double)k);
        float sz = (float)(((double)dz - (double)oz.f - (double)FL(obj, 0x2c)) * (double)k);

        FL(obj, 0x24) = (float)((double)FL(obj, 0x24) + (double)sx);
        if ((U32(obj, 0x7e) & 0x20) == 0 && (U32(obj, 0x7e) & 1) != 0)
            FL(obj, 0x28) = (float)((double)FL(obj, 0x28) + (double)sy);
        FL(obj, 0x2c) = (float)((double)FL(obj, 0x2c) + (double)sz);
    } else {
        FL(obj, 0x24) = (float)((double)dx - (double)ox.f);
        if ((U32(obj, 0x7e) & 0x20) == 0 && (U32(obj, 0x7e) & 1) != 0)
            FL(obj, 0x28) = (float)((double)dy - (double)oy.f);
        FL(obj, 0x2c) = (float)((double)dz - (double)oz.f);
    }
    height = (float)W8T_CALL_1008486e_1002(ox.f, oz.f);
    s = W8T_CALL_100fcc74((double)oy.f) * GF64(0x10111258);
    angle = (float)-s;
    if ((U32(obj, 0x7e) & 0x20) != 0 && (U32(obj, 0x7e) & 1) != 0)
        Sim_100aa0da((void *)(uintptr_t)U32(obj, 0x3c), Bits_OfFloat(angle), 0, 0, 0);
    else
        Sim_100aa0da((void *)(uintptr_t)U32(obj, 0x3c), Bits_OfFloat(angle), Bits_OfFloat(height), 0, 0);
}

// FUNCTION: HGSIM 0x1002528f
void __cdecl Sim_1002528f(void)
{
    int i;

    for (i = 0; i < 0x100; i++) {
        uint8_t *e = GBYTES(0x109a5a40) + i * 0x2c;
        uint8_t *d;
        int go, restore;

        if (U32(e, 0x18) == 0)
            continue;
        d = GBYTES(0x10147b80u + U32(e, 0x24) * 0x3cu);
        if ((U32(d, 0x28) & 1) != 0 && U32(e, 4) != 0) {
            Sim_100a9cfb((void *)(uintptr_t)U32(e, 4), (uint32_t *)(e + 8), (uint32_t *)(e + 0xc),
                         (uint32_t *)(e + 0x10));
            Sim_100a9d47((void *)(uintptr_t)U32(e, 0), U32(e, 8),
                         Bits_OfFloat((float)((double)FL(e, 0xc) - (double)GFL(0x10111260))), U32(e, 0x10));
        }
        if (U32(d, 0x18) != 0) {
            uint32_t vx = U32(e, 8), vy = U32(e, 0xc), vz = U32(e, 0x10);
            union { uint32_t u; float f; } t;

            if (U32(e, 0x24) == 0x16) {
                if (U32(e, 0x14) == U32(d, 0))
                    W8T_CALL_100d3510(0x12, U32(e, 0x14), 2);
                t.f = (float)((double)(int32_t)(U32(d, 0) - U32(e, 0x14)) * (double)GFL(0x10111264));
            } else if (U32(e, 0x24) == 0x2c) {
                t.f = (float)((double)(int32_t)(U32(d, 0) - U32(e, 0x14)) * (double)GFL(0x10111268));
            } else {
                t.u = U32(U32(U32(e, 0), 0x8c), 0x50);
            }
            if (U8(G32(0x1017b8c4), 2) != 0 || U32(e, 0x24) == 0x2c || U32(e, 0x24) == 0x2d)
                W8T_CALL_100255de(U32(e, 0x28), vx, vy, vz, t.u, U32(d, 0x20), U32(d, 0x24));
            Sim_100257ec(U32(e, 0x28), Float_OfBits(vx), Float_OfBits(vy), Float_OfBits(vz), t.f,
                         Float_OfBits(U32(d, 0x20)));
        }
        U32(e, 0x14) -= G32(0x1015dc80);
        if (S32(e, 0x14) > 0)
            continue;
        go = 1;
        restore = 0;
        if (U32(e, 0x20) != 0 && G32(0x1015dc04) != 0 && GS32(0x10134888) == i) {
            if (Sim_100d3a67() == 0)
                restore = 1;
            else
                go = 0;
        }
        if (go == 0)
            continue;
        if (restore != 0) {
            G32(0x1013a018) = 1;
            U32(G32(0x1014cae0), 0x1c) = G32(0x109f14e0);
            U32(G32(0x1014cae0), 0x20) = G32(0x109f14e4);
            U32(G32(0x1014cae0), 0x24) = G32(0x109f14e8);
            U16(G32(0x1014cae0), 0x28) = (uint16_t)Render_Ftol(GFL(0x109f14ec));
            U16(G32(0x1014cae0), 0x2a) = *SIM_PTR(u16a, 0x109f14f0);
            G32(0x1014cb24) = G32(0x109f14f4);
            U32(e, 0x20) = 0;
            G32(0x1015dc04) = 0;
            G32(0x10134888) = 0xffffffffu;
        }
        if (U32(e, 0) != 0) {
            if (Sim_100aa7e7((const void *)(uintptr_t)U32(e, 0)) != 0)
                Sim_100aa8cb((void *)(uintptr_t)U32(e, 0));
            Sim_100aa351((void *)(uintptr_t)U32(e, 0));
        }
        if (S32(e, 0x1c) > 0) {
            Sim_1000711c((short)U16(e, 0x1c), 0);
            Sim_1000716a((int)U16(e, 0x1c), 0);
        }
        Sim_10021be0(i);
    }
}

// FUNCTION: HGSIM 0x10025aee
void __cdecl Sim_10025aee(uint32_t source, const void *hit, float dmg, uint32_t x, uint32_t y, uint32_t z)
{
    uint32_t index;
    uint8_t *e;

    if (hit == 0)
        return;
    index = U16(hit, 0x24);
    e = GBYTES(0x109e9ba0u + index * 0x50u);
    if ((S16(e, 0) & 4) != 0)
        return;
    if (U32(e, 0xc) == 0)
        return;
    if ((U16(hit, 2) & 0xf0) == 0xb0)
        U32(e, 0xc) = 0;
    else
        U32(e, 0xc) -= (uint32_t)Render_Ftol(dmg);
    U32(e, 0x4c) &= 0x7fffffffu;
    U32(e, 0x4c) = (uint16_t)source;
    if (S32(e, 0xc) > 0)
        return;
    U32(e, 0xc) = 0;
    if ((U32(e, 2) & 1) != 0)
        Sim_10024616(source, 0x24, x, y, z, x, y, z);
    else if ((U32(e, 2) & 2) != 0)
        Sim_10024616(source, 0x16, x, y, z, x, y, z);
    else if ((U16(hit, 2) & 0xf0) == 0xb0)
        Sim_10024616(source, 0x24, x, y, z, x, y, z);
    else
        Sim_10024616(source, 0x32, x, y, z, x, y, z);
    Sim_1002597b((int)index);
}

// FUNCTION: HGSIM 0x100272cb
uint32_t __cdecl Sim_100272cb(const char *name)
{
    union { uint32_t align; uint8_t b[0x10]; } key;    /* word number, 12-character name, terminating zero */
    union { uint32_t align; uint8_t b[0x20]; } out;
    uint32_t result = 0;
    int digit;
    void *set;

    Str_CopyN((char *)key.b + 2, name, 0xc);
    key.b[0xe] = 0;
    if (GS32(0x1015965c) > 1)
        digit = Crt_IsCType((int)(int8_t)name[0], 4);
    else
        digit = U16(G32(0x10159450), (int32_t)(int8_t)name[0] * 2) & 4;
    if (digit != 0)
        U16(key.b, 0) = (uint16_t)Crt_AToI(name);
    else
        U16(key.b, 0) = 0xffff;
    set = W8T_CALL_100278c0(key.b, out.b);
    if (set != 0) {
        if (Sim_100273a1(set) != 0)
            result = (uint32_t)(uintptr_t)Sim_10027654();
        Sim_10027a8f(set);
        Sim_10053132();
        Sim_100db351();
    }
    return result;
}

/* A dword stored big-endian (the resource headers), as a number. */
static uint32_t W8t_Swap(uint32_t v)
{
    return (v << 24) | ((v << 8) & 0xff0000u) | ((v >> 8) & 0xff00u) | ((v >> 24) & 0xffu);
}

// FUNCTION: HGSIM 0x100273a1
int __cdecl Sim_100273a1(void *src)
{
    int ok = 1;
    uint8_t *rec = Sim_100279f1(src);

    if (rec == 0 || U32(rec, 0) != G32(0x101349ec)) {
        W8T_CALL_100bc280(0x48, 0);
        return 0;
    }
    if (W8t_Swap(U32(rec, 8)) < W8t_Swap(G32(0x101348b0))) {
        W8T_CALL_100bc280(0x49, 0);
        return 0;
    }
    rec = Sim_100279f1(src);
    if (rec == 0 || U32(rec, 0) != G32(0x101349f0)) {
        W8T_CALL_100bc280(0x3e, 0);
        return 0;
    }
    if (Sim_100530a9(BYTES(src) + 4) == 0)
        Sim_10053010((const uint16_t *)(BYTES(src) + 4));
    *SIM_PTR(u16a, 0x109f1448) += U16(rec, 8);
    *SIM_PTR(u16a, 0x109f144a) += U16(rec, 0xa);
    *SIM_PTR(u16a, 0x109f144c) += U16(rec, 0xc);
    *SIM_PTR(u16a, 0x109f144e) += U16(rec, 0xe);
    *SIM_PTR(u16a, 0x109f1450) += U16(rec, 0x10);
    *SIM_PTR(u16a, 0x109f1452) += U16(rec, 0x12);
    *SIM_PTR(u16a, 0x109f1454) += U16(rec, 0x14);
    *SIM_PTR(u16a, 0x109f1456) += U16(rec, 0x16);
    G32(0x109f1458) += U32(rec, 0x18);
    while (rec != 0) {
        uint32_t tag = U32(rec, 0);

        if (G32(0x1015dc28) != 0)
            W8T_CALL_10027b69(tag);
        if (tag == G32(0x10134a14))
            ok &= Sim_100da098(rec);
        else if (tag == G32(0x10134ab0))
            ok &= Sim_100dafa8(rec, SIM_FN(W7qVisitFn, W8T_FN_10001a37));
        else if (tag == G32(0x10134ab8))
            ok &= Sim_100db259(rec);    /* the original also pushes 0x10001a37, which Sim_100db259 does not read */
        rec = Sim_100279f1(src);
    }
    return ok;
}
