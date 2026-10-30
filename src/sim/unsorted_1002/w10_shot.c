#include "w10.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "w3o.h"
#include "w5a.h"
#include "w6d.h"
#include "w7l.h"
#include "w8t.h"
#include "../unsorted_1001/w6d.h"
#include "../unsorted_1001/w8t.h"
#include "../unsorted_1004/fns.h"
#include "../unsorted_1005/w2h.h"
#include "../unsorted_1006/w10.h"
#include "../unsorted_1006/w6e.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1009/w10.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_1009/w5c.h"
#include "../unsorted_1009/w6f.h"
#include "../unsorted_1009/w6g.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100b/w2j.h"

typedef double(__cdecl *W10_2SFmodFn)(double x, double y);

#define W10_2S_SHOT(i) (SIM_PTR(uint8_t, 0x1096fc80) + (uint32_t)(i) * 0x156u)
#define W10_2S_SELF() G32(0x1017b8c8)

typedef union {
    float f;
    uint32_t u;
} W10_2SBits;

/* one axis of the shot's step: pos += (dt * acc * 0.5 + vel) * dt and vel += (float)(dt * acc), in the x87's
 * double precision with the product kept unrounded for the position */
static void W10_2S_Step(int32_t dt, float acc, float *vel, float *pos)
{
    double d = (double)dt * (double)acc;
    float t = (float)d;

    *pos = (float)((d * (double)GFL(0x10111230) + (double)*vel) * (double)dt + (double)*pos);
    *vel = (float)((double)*vel + (double)t);
}

// FUNCTION: HGSIM 0x100225c7
void __cdecl Sim_100225c7(int idx)
{
    uint8_t *p = W10_2S_SHOT(idx);
    int32_t dt = GS32(0x1015dc80);
    W10_2SBits sx, sy, sz, ex, ey, ez;
    float vx, vy, vz;
    float seg[14];
    uint32_t loc88[4];
    uint32_t hitrec = 0;
    uint16_t flags = 0;
    int hit, hitUnit = 0;
    double len;

    Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), &sx.u, &sy.u, &sz.u);
    vx = FL(p, 0x18);
    vy = FL(p, 0x1c);
    vz = FL(p, 0x20);
    ex = sx;
    ey = sy;
    ez = sz;
    W10_2S_Step(dt, FL(p, 0x24), &vx, &ex.f);
    W10_2S_Step(dt, FL(p, 0x28), &vy, &ey.f);
    W10_2S_Step(dt, FL(p, 0x2c), &vz, &ez.f);
    Sim_1009cab0(seg, sx.f, sy.f, sz.f, ex.f, ey.f, ez.f);
    len = Sim_10084a94((float)((double)ex.f - sx.f), (float)((double)ey.f - sy.f), (float)((double)ez.f - sz.f));
    if (len > (double)GFL(0x10111234))
        Sim_1009cc82(seg);
    else
        Sim_1009cbfe(seg);

    if ((U32(p, 0x48) & 0x8000) != 0) {
        hit = 1;
        if (U32(p, 0x44) == 0x200)
            hitrec = Sim_100a9b71((void *)(uintptr_t)U32(PTRTAB(U32(p, 0x40)), 0x48));
        else if (U32(p, 0x44) == 0x400)
            hitrec = Sim_100b53bb((int)G32(0x109e9ba8 + U32(p, 0x40) * 0x50));
    } else {
        hit = Sim_1002dda9(seg, &hitrec, (int)U32(p, 0x50));
    }

    if (hit != 0 && hitrec != 0) {
        uint8_t *hr = BYTES(hitrec);

        flags = U16(hr, 2);
        G32(0x10134898) = U32(p, 0x50);
        if ((flags & 0x100) != 0) {
            uint32_t tgt = U16(hr, 0x24);
            uint8_t *rec;
            uint8_t *unit;
            float ang;

            hitUnit = 1;
            if ((U32(SIM_PTR(uint8_t, 0x1014558e), U32(p, 0x8e) * 0xd6) & 8) != 0 && Sim_1009c929(100) < 10) {
                rec = BYTES(U32(PTRTAB(tgt), 0x28));
                if (U32(rec, 0x4f0) == 2 && U32(rec, 0xbbf) == 0 && (U16(rec, 0x560) & 0x20) == 0) {
                    if (tgt == W10_2S_SELF())
                        Sim_100203cf(0x2a, -1);
                    else
                        Sim_10015630((void *)(uintptr_t)U32(rec, 0), 10, 0, 1);
                    U32(rec, 0xc3d) = 0x21f;
                }
            }
            U32(PTRTAB(tgt), 0x150) &= 0x7fffffffu;
            U32(PTRTAB(tgt), 0x150) = (uint32_t)(int32_t)(int16_t)G32(0x10134898) & 0xffffu;
            U32(p, 8) |= 0x100;
            if ((U32(p, 0x7e) & 0x40) != 0)
                U32(p, 8) |= 0x1000;
            /* the heading difference wrapped by fmod; the original never reads the result back */
            unit = BYTES(PTRTAB(tgt));
            ang = (float)((double)FL(unit, 0x68) - Sim_1008486e(FL(p, 0x18), FL(p, 0x20)));
            (void)SIM_FN(W10_2SFmodFn, 0x100fcb30)((double)ang, 360.0);

            if (G32(0x10134898) == W10_2S_SELF()) {
                U32(BYTES(U32(PTRTAB(tgt), 0x28)), 0xc41) = U32(hr, 0x28);
                switch (Sim_10052be7((int)tgt)) {
                case 0: U16(SIM_PTR(uint8_t, 0x101653f9), 0)++; break;
                case 1: U16(SIM_PTR(uint8_t, 0x101653f5), 0)++; break;
                case 2: U16(SIM_PTR(uint8_t, 0x101653f7), 0)++; break;
                }
            }
            if (GS32(0x10134898) >= 0 && U32(PTRTAB(G32(0x10134898)), 8) == U32(PTRTAB(W10_2S_SELF()), 8)) {
                switch (Sim_10052be7((int)tgt)) {
                case 0: U16(SIM_PTR(uint8_t, 0x1016540e), 0)++; break;
                case 1: U16(SIM_PTR(uint8_t, 0x10165412), 0)++; break;
                case 2: U16(SIM_PTR(uint8_t, 0x10165410), 0)++; break;
                }
            }
            if (tgt == W10_2S_SELF())
                U16(SIM_PTR(uint8_t, 0x101653fb), 0)++;
            if (U32(PTRTAB(tgt), 8) == U32(PTRTAB(W10_2S_SELF()), 8))
                U16(SIM_PTR(uint8_t, 0x10165414), 0)++;
            if (G32(0x1015ecd0) == 0 || (U32(PTRTAB(tgt), 0x14) == 3 && U32(PTRTAB(tgt), 0xc) == W10_2S_SELF()) ||
                (tgt == W10_2S_SELF() && G32(0x10134898) != W10_2S_SELF())) {
                rec = BYTES(U32(PTRTAB(tgt), 0x28));
                FL(rec, 0x4e0) = (float)((double)FL(rec, 0x4e0) + (double)FL(p, 0x5c));
                Sim_1006c161((int32_t)U32(p, 0x50), BYTES(U32(PTRTAB(tgt), 0x28)), 1, p, (int32_t)U16(hr, 0x26));
            }
            if (tgt == W10_2S_SELF()) {
                U32(p, 8) |= 0x4000;
                if (U16(hr, 0x26) == 1 || U16(hr, 0x26) == 2)
                    U32(p, 8) |= 0x2000;
                Sim_10061f8e(vx, vy, vz);
            }
            Sim_10015afd((int)U32(p, 0x50), tgt);
        } else if ((flags & 0x200) != 0) {
            vx = -vx;
            vz = -vz;
        } else if ((flags & 0x400) != 0) {
            U32(p, 8) |= 0x200;
        } else if ((flags & 0x800) != 0) {
            U32(p, 8) |= 0x400;
            vx = (float)((double)vx / (double)GFL(0x1011123c));
            vz = (float)((double)vz / (double)GFL(0x1011123c));
            vy = -(float)((double)vy / (double)GFL(0x1011123c));
        } else {
            vx = -vx;
            vz = -vz;
        }
    } else if (!((double)ey.f >= (double)GFL(0x10111240))) {
        /* below the ground plane: the ground height comes back as the new y; the original then tests y below it,
         * which only an unordered height passes */
        float g = (float)Sim_1002d961(ex.f, ey.f, ez.f);

        ey.f = g;
        if (!(ey.f >= g)) {
            U32(p, 8) |= 0x400;
            Sim_1009cf67(seg, g);
        }
    }

    if (S32(p, 0x34) <= 0 || hitUnit != 0) {
        Sim_10024548(hitrec, idx, 1, ex.u, ey.u, ez.u, ex.u, ey.u, ez.u);
        return;
    }
    if (U32(p, 0) == 0x3e || U32(p, 0) == 0x3f) {
        int i;

        for (i = 0; i < 0x200; i++) {
            uint8_t *q = W10_2S_SHOT(i);
            W10_2SBits a, b, c, d, e, f;
            double dist;

            if (U32(q, 0x48) == 0 || U32(q, 0) != 5)
                continue;
            Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), &a.u, &b.u, &c.u);
            Sim_100a9cfb((void *)(uintptr_t)U32(q, 0x3c), &d.u, &e.u, &f.u);
            dist = Sim_10084a94((float)((double)a.f - d.f), (float)((double)b.f - e.f), (float)((double)c.f - f.f));
            if (!(dist >= (double)GFL(0x10111244)))
                U32(p, 0x48) |= 0x8000;
        }
    }
    if (U32(p, 0x44) != 0 && S32(p, 0x30) > S32(p, 0x82))
        Sim_100241ed(p, ex.f, ey.f, ez.f, &vx, &vy, &vz);
    if (!(Sim_1002d961(ex.f, ey.f, ez.f) >= (double)ey.f))
        Sim_100a9d47((void *)(uintptr_t)U32(p, 0x3c), ex.u, ey.u, ez.u);
    Sim_100aa799((void *)(uintptr_t)U32(p, 0x3c));
    FL(p, 0x18) = vx;
    FL(p, 0x1c) = vy;
    FL(p, 0x20) = vz;
    loc88[1] = ex.u;
    loc88[2] = ey.u;
    loc88[3] = ez.u;
    Sim_100585e1((int32_t)U32(p, 0x68), 1, loc88);
    if (U32(p, 0x60) != 0) {
        G32(0x10134890) = (uint32_t)idx;
        G32(0x1015dbd8) = ex.u;
        G32(0x1015dbdc) = ey.u;
        G32(0x1015dbe0) = ez.u;
        GFL(0x1015dbe4) = (float)Sim_1008486e(vx, vz);
        G32(0x1015dbe8) = 0;
        G32(0x1015dbec) = 0;
        G32(0x1015dbf0) = 1;
    }
    {
        float dx = (float)((double)ex.f - sx.f), dy = (float)((double)ey.f - sy.f), dz = (float)((double)ez.f - sz.f);

        switch (U32(p, 0)) {
        case 0x38:
        case 0x39:
            Sim_10096ac3(sx.u, sy.u, sz.u, dx, dy, dz);
            break;
        case 0x3f:
            Sim_10095e0a(sx.f, sy.f, sz.f, dx, dy, dz);
            Sim_10094fcb(sx.f, sy.f, sz.f, dx, dy, dz);
            break;
        case 0x40:
        case 0x41:
            Sim_10094fcb(sx.f, sy.f, sz.f, dx, dy, dz);
            break;
        case 0x3d:
        case 0x3e:
        case 0x42:
        case 0x43:
        case 0x44:
        case 0x45:
            if ((U32(p, 0x7e) & 0x40) != 0)
                Sim_10094fcb(sx.f, sy.f, sz.f, dx, dy, dz);
            else
                Sim_10095e0a(sx.f, sy.f, sz.f, dx, dy, dz);
            break;
        }
    }
}
