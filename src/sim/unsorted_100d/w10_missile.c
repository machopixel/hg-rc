#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "fns.h"
#include "w5c.h"
#include "w8z.h"
#include "../render/w2n.h"
#include "../unsorted_1000/w2l.h"
#include "../unsorted_1002/w5a.h"
#include "../unsorted_1004/w2k.h"
#include "../unsorted_1005/w2g.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1008/w7n.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100a/w6g.h"
#include "../unsorted_100c/w10.h"
#include "../fx/w32.h"

/* 0x100d71b9 stays original (it rounds the 80-bit result of the CRT asin) */
typedef void(__cdecl *W10dAimFn)(uint8_t *obj, uint32_t node, uint32_t x, uint32_t y, uint32_t z, int big,
                                 const uint8_t *weapon, uint8_t *slot);

#define W10D_MISSILE(i) (0x1096fc80u + (uint32_t)(i) * 0x156u)

// FUNCTION: HGSIM 0x100d5446
int __cdecl Sim_100d5446(uint8_t *obj, const uint8_t *weapon, uint8_t *slot)
{
    uint32_t light[14];       /* the original's 0x38-byte light record at ebp-0x64 (Sim_100585e1 reads +0..+0xc, +0x2c) */
    float x, y, z;            /* the launch direction (ebp-0x1c, -0x24, -0x2c) */
    float a, b, c;            /* the node's position (ebp-0x20, -0x28, -0x6c) */
    float speed, f;
    uint8_t *rec, *me_rec, *p = 0;
    uint32_t self = G32(0x1017b8c8), ctl;
    int ret = 1, mine, found = 0, big;
    int32_t i, k, v;

    me_rec = BYTES(U32(PTRTAB(self), 0x28));
    if (obj == 0)
        return 0;
    rec = BYTES(U32(obj, 0x28));
    if (U32(rec, 0x498u + U32(slot, 0x11c) * 4u) == 0)
        return 0;
    if (U32(slot, 0x20) == 0 || U32(rec, 0xc3d) != 0)
        return 0;
    mine = U32(U32(rec, 0), 4) == G32(0x1017b8c8);
    if (mine) {
        if (G32(0x1015ef3c) != 0)       /* 1.2: its force-feedback request (1.0: Sim_10045f3b(kind, 0x2d, 0x32)) */
            Sim_100382fb(U32(weapon, 0xd2), 0, 0, 0);
        ctl = U32(U32(rec, 0), 0x54);
        if (*(float *)BYTES(ctl + 8) >= *SIM_PTR(float, 0x1012bcd8) && (U16(rec, 0x562) & 1) == 0) {
            v = (int32_t)((uint32_t)U16(weapon, 0x74) + U32(slot, 0x24) - U32(rec, 0xbd7));
            f = (float)v;
            if ((double)v > (double)*SIM_PTR(float, 0x1012bcdc)) {
                switch (Render_Ftol((double)f)) {
                case 1:
                    *SIM_PTR(float, 0x10179cec) = (float)((double)*SIM_PTR(float, 0x10179cec) + *SIM_PTR(float, 0x1012bce0));
                    break;
                case 2:
                    *SIM_PTR(float, 0x10179cec) = (float)((double)*SIM_PTR(float, 0x10179cec) + *SIM_PTR(float, 0x1012bce4));
                    break;
                case 3:
                    *SIM_PTR(float, 0x10179cec) = (float)((double)*SIM_PTR(float, 0x10179cec) + *SIM_PTR(float, 0x1012bce8));
                    break;
                case 4:
                    *SIM_PTR(float, 0x10179cec) = (float)((double)*SIM_PTR(float, 0x10179cec) + *SIM_PTR(float, 0x1012bcec));
                    break;
                default:
                    *SIM_PTR(float, 0x10179cec) = (float)((double)*SIM_PTR(float, 0x10179cec) + *SIM_PTR(float, 0x1012bcf0));
                    break;
                }
            }
        }
        *SIM_PTR(u16a, 0x101653f3) += 1;
    }
    if (U32(PTRTAB(U32(obj, 4)), 8) == U32(PTRTAB(G32(0x1017b8c8)), 8))
        *SIM_PTR(u16a, 0x1016540c) += 1;
    if (S32(slot, 0x20) != -1 && !(U8(G32(0x1017b8c4), 0) != 0 && U32(obj, 4) == G32(0x1017b8c8))) {
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
    if (U32(weapon, 0x24) != 0) {
        for (i = 0; !found && i < 0x200; i++) {
            p = GBYTES(W10D_MISSILE(i));
            if (U32(p, 0) == U32(weapon, 0x10) && U32(p, 0x48) == 0 && U32(p, 0x3c) != 0)
                found = 1;
        }
        if (found) {
            /* the original's further tests for the case "mine" can never hold there */
            if (!mine)
                Sim_100d6d6a(obj);
            U32(p, 0x54) = (uint32_t)(uintptr_t)slot;
            U32(p, 0x86) = (uint32_t)(uintptr_t)weapon + 0xc2u;
            U32(p, 4) = U32(weapon, 8);
            U32(p, 0x30) = U32(slot, 0x10);
            U32(p, 0x34) = U32(weapon, 0x5c) - U32(slot, 0x10);
            U32(p, 8) = U32(weapon, 0x10);
            U32(p, 0x58) = U32(weapon, 0x40);
            U32(p, 0x5c) = U32(weapon, 0x48);
            U32(p, 0x48) = 1;
            U32(p, 0x4c) = U16(weapon, 0x76);
            U32(p, 0x40) = U32(slot, 0x18);
            U32(p, 0x44) = U32(slot, 0x1c);
            U32(p, 0x50) = U32(obj, 4);
            U32(p, 0x8e) = U32(slot, 4);
            U32(p, 0x70) = U32(weapon, 0x78) + U32(slot, 0x28);
            if ((U32(rec, 0xbd3) & 0x400) != 0)
                U32(p, 0x70) += 2;
            U32(p, 0x74) = U32(weapon, 0x7c);
            U32(p, 0x78) = U32(weapon, 0x80);
            U16(p, 0x7c) = U16(weapon, 0x84);
            U32(p, 0x7e) = U32(weapon, 0x86);
            U32(p, 0x152) = 1;
            if ((U32(p, 0x7e) & 0x20) != 0 && (U32(p, 0x7e) & 1) != 0) {
                U32(p, 0x82) = (uint32_t)Render_Ftol((double)Sim_100c97c0(obj, slot) / *SIM_PTR(float, 0x1012bcf4));
                Sim_100d6ea7(obj, &x, &y, &z, slot);
                U16(rec, 0x560) |= 0x40;
                U16(rec, 0x560) |= 0x8000;
            } else {
                U32(p, 0x82) = 0;
                Sim_100d6ea7(obj, &x, &y, &z, slot);
            }
            Sim_10084cee(&x, &y, &z);
            if (U32(weapon, 0x10) == 0x45)
                y = (float)((double)y + *SIM_PTR(double, 0x1012bcf8));
            speed = *(const float *)(weapon + 0x3c);
            *(float *)(p + 0x18) = (float)((double)speed * x);
            *(float *)(p + 0x1c) = (float)((double)speed * y);
            *(float *)(p + 0x20) = (float)((double)speed * z);
            *(float *)(p + 0x28) = (float)(-(double)*SIM_PTR(float, 0x1013abc0) * (double)S32(weapon, 0x38));
            U32(p, 0x2c) = 0;
            U32(p, 0x24) = 0;
            if ((U32(weapon, 8) & 0x20) != 0) {
                for (k = 0; k < 8; k++) {
                    Sim_1000716a((int)(U32(weapon, 0xc) + (uint32_t)k), 0);
                    Sim_1000711c((short)(U32(weapon, 0xc) + (uint32_t)k), 1);
                }
            }
            big = U32(p, 0) == 0x3e || U32(p, 0) == 0x3f;
            SIM_FN(W10dAimFn, 0x100d71b9)(obj, U32(p, 0x3c), *(uint32_t *)&x, *(uint32_t *)&y, *(uint32_t *)&z, big,
                                          weapon, slot);
            Sim_100aa39f((void *)(uintptr_t)U32(p, 0x3c));
            Sim_100aa406((void *)(uintptr_t)U32(p, 0x3c));
            Sim_100aa799((void *)(uintptr_t)U32(p, 0x3c));
            Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), (uint32_t *)&a, (uint32_t *)&b, (uint32_t *)&c);
            U32(p, 0xc) = *(uint32_t *)&a;
            U32(p, 0x10) = *(uint32_t *)&b;
            U32(p, 0x14) = *(uint32_t *)&c;
            if (G32(0x1017e67c) != 0) {
                U32(p, 0x68) = (uint32_t)Sim_10058575();
                light[0] = 1;
                light[1] = *(uint32_t *)&x;
                light[2] = *(uint32_t *)&y;
                light[3] = *(uint32_t *)&z;
                light[0x2c / 4] = 0x461c4000;
                Sim_100585e1(S32(p, 0x68), 7, light);
            }
            U32(p, 0x6c) = (uint32_t)Sim_100ac21a(2, p);
            if (W32_PARTICLES()) {
                /* 1.2 (0x100bb425): the launch smoke at the weapon's mount node, rising 5 per frame */
                Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x498u + U32(slot, 0x11c) * 4u), (uint32_t *)&a,
                             (uint32_t *)&b, (uint32_t *)&c);
#ifdef W10G_MUTATE /* the checker's negative check only: sideways */
                Sim12_1007f19f(a, b, c, 5.0f, 0.0f, 0.0f);
#else
                Sim12_1007f19f(a, b, c, 0.0f, 5.0f, 0.0f);
#endif
            }
            if (mine) {
                if (U32(p, 0) == 0x3e || U32(p, 0) == 0x41 || U32(p, 0) == 0x3f)
                    G32(0x1013488c) = (uint32_t)(i - 1);
                else
                    G32(0x1013488c) = 0xffffffffu;
            }
        }
    }
    if (!mine && Sim_100d5c9b(S32(slot, 4)) != 0 && (uint32_t)U16(obj, 0x1aa) == (G32(0x1017b8c8) | 0x200u) &&
        (U32(me_rec, 0xbcb) & 2) != 0 && U32(me_rec, 0xc49) == 0) {
        U32(me_rec, 0xc49) = 0x43e;
        Sim_100203cf(0x20, -1);
    }
    if (mine && U32(me_rec, 0xc49) != 0) {
        U32(me_rec, 0xc49) -= G32(0x1015dc80);
        if (S32(me_rec, 0xc49) < 0)
            U32(me_rec, 0xc49) = 0;
    }
    return ret;
}
