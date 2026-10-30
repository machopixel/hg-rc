#include "w29.h"
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

typedef double(__cdecl *W29FmodFn)(double x, double y);

#define W29_SHOT(i) (SIM_PTR(uint8_t, 0x1096fc80) + (uint32_t)(i) * 0x156u)
#define W29_SELF() G32(0x1017b8c8)

typedef union {
    float f;
    uint32_t u;
} W29Bits;

/* one axis of the shot's step, as Sim_100225c7: pos += (dt * acc * 0.5 + vel) * dt and vel += (float)(dt * acc) */
static void W29_Step(int32_t dt, float acc, float *vel, float *pos)
{
    double d = (double)dt * (double)acc;
    float t = (float)d;

    *pos = (float)((d * (double)GFL(0x10111230) + (double)*vel) * (double)dt + (double)*pos);
    *vel = (float)((double)*vel + (double)t);
}

// FUNCTION: HGSIM 0x100232d7
void __cdecl Sim_100232d7(int idx)
{
    uint8_t *p = W29_SHOT(idx);
    int32_t dt = GS32(0x1015dc80);
    W29Bits n0, n1, n2;                      /* the node's position this frame (the path's end) */
    W29Bits ix, iy, iz;                      /* the impact point */
    W29Bits bx, by, bz;                      /* a point just before it */
    W29Bits px, py, pz;                      /* the position the shot moves on from */
    float vx, vy, vz;
    float seg[14];
    uint32_t loc[4];
    uint32_t hitrec = 0, tgt = 0xffffffffu;
    uint16_t flags = 0;
    int forced = 0, found = 0;

    Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), &n0.u, &n1.u, &n2.u);
    Sim_1009cab0(seg, FL(p, 0xc), FL(p, 0x10), FL(p, 0x14), n0.f, n1.f, n2.f);
    Sim_1009cc82(seg);
    if ((U32(p, 0x48) & 0x8000) != 0) {
        forced = 1;
        if (U32(p, 0x44) == 0x200)
            hitrec = Sim_100a9b71((void *)(uintptr_t)U32(PTRTAB(U32(p, 0x40)), 0x48));
        else if (U32(p, 0x44) == 0x400)
            hitrec = Sim_100b53bb((int)G32(0x109e9ba8 + U32(p, 0x40) * 0x50));
        bx = ix = n0;
        by = iy = n1;
        bz = iz = n2;
    } else {
        found = Sim_1002dda9(seg, &hitrec, (int)U32(p, 0x50));
        if (found != 0) {
            float *f = seg;

            ix.f = f[3];
            iy.f = f[4];
            iz.f = f[5];
            bx.f = (float)((double)ix.f - (double)GFL(0x10111238) * (double)f[9]);
            by.f = (float)((double)iy.f - (double)GFL(0x10111238) * (double)f[10]);
            bz.f = (float)((double)iz.f - (double)GFL(0x10111238) * (double)f[11]);
        } else {
            ix.u = iy.u = iz.u = 0;          /* not read on this path (set again below before any use) */
            bx = ix;
            by = iy;
            bz = iz;
        }
    }
    FL(p, 0xc) = n0.f;
    FL(p, 0x10) = n1.f;
    FL(p, 0x14) = n2.f;
    vx = FL(p, 0x18);
    vy = FL(p, 0x1c);
    vz = FL(p, 0x20);
    px = n0;
    py = n1;
    pz = n2;

    if ((forced != 0 || found != 0) && hitrec != 0) {
        uint8_t *hr = BYTES(hitrec);

        flags = U16(hr, 2);
        G32(0x10134898) = U32(p, 0x50);
        if ((flags & 0x100) != 0) {
            uint8_t *rec;
            uint8_t *unit;
            float ang;

            tgt = U16(hr, 0x24);
            if ((U32(SIM_PTR(uint8_t, 0x1014558e), U32(p, 0x8e) * 0xd6) & 8) != 0 && Sim_1009c929(100) < 10) {
                rec = BYTES(U32(PTRTAB(tgt), 0x28));
                if (U32(rec, 0x4f0) == 2 && U32(rec, 0xbbf) == 0 && (U16(rec, 0x560) & 0x20) == 0) {
                    if (tgt == W29_SELF())
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
            (void)SIM_FN(W29FmodFn, 0x100fcb30)((double)ang, 360.0);
            if (G32(0x10134898) == W29_SELF()) {
                switch (Sim_10052be7((int)tgt)) {
                case 0: U16(SIM_PTR(uint8_t, 0x101653f9), 0)++; break;
                case 1: U16(SIM_PTR(uint8_t, 0x101653f5), 0)++; break;
                case 2: U16(SIM_PTR(uint8_t, 0x101653f7), 0)++; break;
                }
            }
            if (GS32(0x10134898) >= 0 && U32(PTRTAB(G32(0x10134898)), 8) == U32(PTRTAB(W29_SELF()), 8)) {
                switch (Sim_10052be7((int)tgt)) {
                case 0: U16(SIM_PTR(uint8_t, 0x1016540e), 0)++; break;
                case 1: U16(SIM_PTR(uint8_t, 0x10165412), 0)++; break;
                case 2: U16(SIM_PTR(uint8_t, 0x10165410), 0)++; break;
                }
            }
            if (tgt == W29_SELF())
                U16(SIM_PTR(uint8_t, 0x101653fb), 0)++;
            if (U32(PTRTAB(tgt), 8) == U32(PTRTAB(W29_SELF()), 8))
                U16(SIM_PTR(uint8_t, 0x10165414), 0)++;
            if (G32(0x1015ecd0) == 0 || (U32(PTRTAB(tgt), 0x14) == 3 && U32(PTRTAB(tgt), 0xc) == W29_SELF()) ||
                (tgt == W29_SELF() && G32(0x10134898) != W29_SELF())) {
                rec = BYTES(U32(PTRTAB(tgt), 0x28));
                FL(rec, 0x4e0) = (float)((double)FL(rec, 0x4e0) + (double)FL(p, 0x5c));
                Sim_1006c161((int32_t)U32(p, 0x50), BYTES(U32(PTRTAB(tgt), 0x28)), 1, p, (int32_t)U16(hr, 0x26));
            }
            if (tgt == W29_SELF()) {
                U32(p, 8) |= 0x4000;
                if (U16(hr, 0x26) == 1 || U16(hr, 0x26) == 2)
                    U32(p, 8) |= 0x2000;
                Sim_10061f8e(vx, vy, vz);
            }
            Sim_10015afd((int)U32(p, 0x50), tgt);
        } else if ((flags & 0x200) != 0) {
            U32(p, 8) |= 0x200;
            Sim_10025aee(U32(p, 0x50), (const void *)(uintptr_t)hitrec, (float)(double)U32(p, 0x70), ix.u, iy.u,
                         iz.u);
        } else if ((flags & 0x400) != 0) {
            U32(p, 8) |= 0x200;
        } else {
            U32(p, 8) |= 0x400;
        }
    } else if (!((double)n1.f >= (double)GFL(0x10111240))) {
        float g = (float)Sim_1002d961(n0.f, n1.f, n2.f);

        if (!((double)n1.f >= (double)g)) {
            forced = 1;
            U32(p, 8) |= 0x400;
            /* deliberate fix (DECISIONS.md round 29): with no hit found the original cuts a segment it never built
               ([ebp-0xdc]); the impact point is where the shot's own path (seg) meets the ground, as with a hit */
#ifdef W29S_MUTATE /* the checker's negative check only */
            Sim_1009cf67(seg, (float)((double)g + 1.0));
#else
            Sim_1009cf67(seg, g);
#endif
            bx.f = ix.f = seg[3];
            by.f = iy.f = seg[4];
            bz.f = iz.f = seg[5];
        }
    }

    if (forced != 0 || found != 0) {
        if ((flags & 0x100) != 0) {
            if (tgt == W29_SELF() && (U32(p, 0) == 0x3e || U32(p, 0) == 0x3f))
                U16(SIM_PTR(uint8_t, 0x101654ee), 0)++;
            Sim_10024548(hitrec, idx, 1, ix.u, iy.u, iz.u, Bits_OfFloat(vx), Bits_OfFloat(vy), Bits_OfFloat(vz));
        } else {
            Sim_10024548(hitrec, idx, 1, ix.u, iy.u, iz.u, bx.u, by.u, bz.u);
        }
        return;
    }
    if (S32(p, 0x34) <= 0) {
        Sim_10024548(hitrec, idx, 0, px.u, py.u, pz.u, px.u, py.u, pz.u);
        return;
    }
    if (U32(p, 0) == 0x3e || U32(p, 0) == 0x3f) {
        int i;

        if (Sim_10052be7((int)U32(p, 0x50)) == 1 && U32(p, 0x48) != 0) {
            uint32_t at = G32(0x1017694c);

            Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), (uint32_t *)(uintptr_t)(at + 4),
                         (uint32_t *)(uintptr_t)(at + 8), (uint32_t *)(uintptr_t)(at + 0xc));
            U32(G32(0x1017694c), 0x10) = U32(p, 0x50);
            U32(G32(0x1017694c), 0) = 1;
        }
        for (i = 0; i < 0x200; i++) {
            uint8_t *q = W29_SHOT(i);
            W29Bits a, b, c, d, e, f;
            double dist;

            if (U32(q, 0x48) == 0 || U32(q, 0) != 5)
                continue;
            Sim_100a9cfb((void *)(uintptr_t)U32(p, 0x3c), &a.u, &b.u, &c.u);
            Sim_100a9cfb((void *)(uintptr_t)U32(q, 0x3c), &d.u, &e.u, &f.u);
            dist = Sim_10084a94((float)((double)a.f - d.f), (float)((double)b.f - e.f), (float)((double)c.f - f.f));
            if (!(dist >= (double)GFL(0x10111248))) {
                /* the original compares its unset [ebp-0x2c] here (no unit hit on this path); no shipped data
                   makes a type-5 shot (DECISIONS.md round 29), so this never runs; tgt is -1 here */
                if (tgt == W29_SELF())
                    U16(SIM_PTR(uint8_t, 0x101654ec), 0)++;
                U32(p, 0x48) |= 0x8000;
            }
        }
    }
    if (U32(p, 0x152) == 0) {
        W29_Step(dt, FL(p, 0x24), &vx, &px.f);
        W29_Step(dt, FL(p, 0x28), &vy, &py.f);
        W29_Step(dt, FL(p, 0x2c), &vz, &pz.f);
    } else {
        U32(p, 0x152) = 0;
    }
    if (U32(p, 0x44) != 0 && S32(p, 0x30) > S32(p, 0x82))
        Sim_100241ed(p, px.f, py.f, pz.f, &vx, &vy, &vz);
    Sim_100a9d47((void *)(uintptr_t)U32(p, 0x3c), px.u, py.u, pz.u);
    Sim_100aa799((void *)(uintptr_t)U32(p, 0x3c));
    FL(p, 0x18) = vx;
    FL(p, 0x1c) = vy;
    FL(p, 0x20) = vz;
    loc[1] = px.u;
    loc[2] = py.u;
    loc[3] = pz.u;
    Sim_100585e1((int32_t)U32(p, 0x68), 1, loc);
    if (U32(p, 0x60) != 0) {
        G32(0x10134890) = (uint32_t)idx;
        G32(0x1015dbd8) = px.u;
        G32(0x1015dbdc) = py.u;
        G32(0x1015dbe0) = pz.u;
        GFL(0x1015dbe4) = (float)Sim_1008486e(vx, vz);
        G32(0x1015dbe8) = 0;
        G32(0x1015dbec) = 0;
        G32(0x1015dbf0) = 1;
    }
    {
        float dx = (float)((double)px.f - n0.f), dy = (float)((double)py.f - n1.f), dz = (float)((double)pz.f - n2.f);

        switch (U32(p, 0)) {
        case 0x38:
        case 0x39:
            Sim_10096ac3(n0.u, n1.u, n2.u, dx, dy, dz);
            break;
        case 0x3f:
            Sim_10095e0a(n0.f, n1.f, n2.f, dx, dy, dz);
            Sim_10094fcb(n0.f, n1.f, n2.f, dx, dy, dz);
            break;
        case 0x40:
        case 0x41:
        case 0x46:
        case 0x47:
        case 0x48:
            Sim_10094fcb(n0.f, n1.f, n2.f, dx, dy, dz);
            break;
        case 0x45:
            Sim_10095e0a(n0.f, n1.f, n2.f, dx, dy, dz);
            break;
        case 0x3d:
        case 0x3e:
        case 0x42:
        case 0x43:
        case 0x44:
            if ((U32(p, 0x7e) & 0x40) != 0)
                Sim_10094fcb(n0.f, n1.f, n2.f, dx, dy, dz);
            else
                Sim_10095e0a(n0.f, n1.f, n2.f, dx, dy, dz);
            break;
        }
    }
}
