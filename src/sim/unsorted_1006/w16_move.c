#include <string.h>

#include "w16.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../util/w3o.h"
#include "w6e.h"
#include "w9b.h"
#include "w10.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1000/w5c.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1000/w6d.h"
#include "../unsorted_1002/w3o.h"
#include "../unsorted_1003/w7m.h"
#include "../unsorted_1003/w8u.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_1004/w6e.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1009/w3o.h"
#include "../unsorted_1009/w6f.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100a/w5z.h"
#include "../unsorted_100d/w3q.h"
#include "../unsorted_100d/w5c.h"
#include "../unsorted_100d/w8z.h"
#include "../view/view.h"
#include "../render/w7o.h"
#include "../render/w2n.h"
#include "../unsorted_1004/w2k.h"

/* The C library of the DLL, called at its address as the original does (results in st(0)). */
#define W16M_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)
#define W16M_SQRT SIM_FN(double(__cdecl *)(double), 0x100fca04)
#define W16M_FMOD SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
#define W16M_ATAN SIM_FN(double(__cdecl *)(double), 0x100fcc74)
#define W16M_C(addr) ((double)GFL(addr))
#define W16M_CODE(node) ((int8_t)U8(U32(node, 0x90), 0))

/* The original's __ftol: truncation, 0x80000000 out of range. */
static int32_t W16M_Ftol(double d)
{
    return Render_Ftol(d);
}

static uint32_t W16M_B(float f)
{
    uint32_t b;

    memcpy(&b, &f, 4);
    return b;
}

static float W16M_F(uint32_t b)
{
    float f;

    memcpy(&f, &b, 4);
    return f;
}

/* x87 "equal" (C3), which an unordered compare also sets. */
static int W16M_Eq(double a, double b)
{
    return !(a < b) && !(a > b);
}

/* The locals of 0x1006e22c that cross the helpers (named by their frame offset). */
typedef struct W16Move {
    uint8_t *rec, *unit;
    uint32_t node;      /* [ebp-0x1e4] */
    int own;            /* [ebp-0x24] */
    uint8_t *slot;      /* [ebp-0x20], 0x10a1d140 + id * 0x38 */
    uint32_t kind;      /* [ebp-0x54] */
    float ground0;      /* [ebp-0x78], the unit's ground height at entry */
    float vx, vy, vz;   /* [ebp-0x1f4] / [ebp-0x204] / [ebp-0x14] */
    float ax, az;       /* [ebp-0x200] / [ebp-0x38] */
    float lift;         /* [ebp-0x10] */
    float drive;        /* [ebp-0x2c], 0 when not written (DECISIONS.md round 16) */
    float dx, dy, dz;   /* [ebp-0x20c] / [ebp-0x1c] / [ebp-0x3c] */
    float px, py, pz;   /* [ebp-0x1f8] / [ebp-0x208] / [ebp-0x18] */
    float height;       /* [ebp-0xc] */
    float roll;         /* [ebp-0x1fc] */
    uint32_t object;    /* [ebp-0x44] */
    uint32_t gear;      /* [ebp-0x28] */
} W16Move;

#define W16M_BLK(m) BYTES(U32((m)->unit, 0x54))
#define W16M_R562(m) U16((m)->rec, 0x562)

/* The start: the timers of the record (Sim_10047bcd), the burn marker (+0xc39) and the stun (+0xc3d / +0xc41). */
static void W16M_Timers(W16Move *m)
{
    uint8_t *rec = m->rec;

    if (U32(rec, 0xc3d) == 0) {
        Sim_10047bcd((uint32_t *)(rec + 4));
        Sim_10047bcd((uint32_t *)(rec + 0x14));
    }
    Sim_10047bcd((uint32_t *)(rec + 0x34));
    Sim_10047bcd((uint32_t *)(rec + 0x44));
    Sim_10047bcd((uint32_t *)(rec + 0x54));
    Sim_10047bcd((uint32_t *)(rec + 0x64));
    Sim_10047bcd((uint32_t *)(rec + 0x74));
    Sim_10047bcd((uint32_t *)(rec + 0x24));
    m->vx = FL(rec, 0x548);
    m->vy = FL(rec, 0x54c);
    m->vz = FL(rec, 0x550);
    m->ax = 0.0f;
    m->lift = -GFL(0x1013abc0);
    m->az = 0.0f;
    if (U32(rec, 0xc39) != 0) {
        if ((U16(rec, 0x560) & 0x20) != 0) {
            uint32_t a, b, c;

            Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), &a, &b, &c);
            Sim_100949e8(FL(m->unit, 0x58), W16M_F(b), FL(m->unit, 0x60), 0.0f, 400.0f, 0.0f);
        } else {
            Sim_100949e8(FL(m->unit, 0x58), (float)((double)FL(m->unit, 0x5c) + W16M_C(0x1012b6a4)), FL(m->unit, 0x60),
                         0.0f, 5.0f, 0.0f);
        }
        U32(rec, 0xc39) -= G32(0x1015dc80);
        if (S32(rec, 0xc39) <= 0)
            U32(rec, 0xc39) = 0;
    }
    if (U32(rec, 0xc3d) == 0)
        return;
    if ((U16(rec, 0x560) & 0x20) != 0) {
        U32(rec, 0xc3d) = 0;
        U32(rec, 0xc15) = 1;
        return;
    }
    if (m->own) {
        FlashText_Start(0x15);
        G32(0x10144fa0) = 0;
    }
    U32(rec, 0xc3d) -= G32(0x1015dc80);
    if (U32(rec, 0xc41) == 0)
        U32(rec, 0xc41) = m->node;
    if (S32(rec, 0xc3d) < 0) {
        U32(rec, 0xc3d) = 0;
        U32(rec, 0xc41) = 0;
        if (m->own) {
            G32(0x10144fa0) = 1;
            Sim_1000ebfb(SIM_PTR(const char, 0x101453d4), 0, 0, 5, 0x32);
        } else if ((U16(rec, 0x560) & 0x20) == 0) {
            Sim_10013eba((void *)m->unit, 0);
        }
    }
    if (U32(rec, 0xc41) != 0) {
        uint32_t p = U32(rec, 0xc41), t, first;
        int32_t n = 0, i, found = 0;

        if (U32(p, 0x8c) != 0)
            U32(U32(p, 0x8c), 0xb0) = 1;
        if (U32(U32(rec, 0xc41), 4) != 0)
            t = U32(U32(rec, 0xc41), 4);
        else if (U32(U32(rec, 0xc41), 8) != 0)
            t = U32(U32(rec, 0xc41), 8);
        else
            t = m->node;
        if (t != 0) {
            first = t;
            for (; t != 0; t = U32(t, 8)) {
                n++;
                if (U32(t, 0x8c) != 0)
                    U32(U32(t, 0x8c), 0xb0) = 1;
            }
            t = first;
            n = Sim_1009c929(100) % n;
            for (i = 0; t != 0; t = U32(t, 8), i++) {
                if (i == n) {
                    U32(rec, 0xc41) = t;
                    found = 1;
                }
            }
            if (!found)
                U32(rec, 0xc41) = m->node;
        }
    }
    G32(0x10179cfc) += 1;
}

/* The drive (on the ground, not jumping): the target speeds from the record's speed +0x2c and the unit's heading. */
static void W16M_Drive(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;
    float fwd, side;
    int32_t dt;

    m->drive = FL(rec, 0x2c);
    if ((W16M_R562(m) & 1) != 0)
        m->drive = (float)((double)m->drive * W16M_C(0x1012b6a8));
    if (!((double)m->drive >= W16M_C(0x1012b6ac)) && (double)m->drive > W16M_C(0x1012b6b0))
        m->drive = 0.0f;
    fwd = (float)((double)m->drive * FL(unit, 0xd0));
    side = (float)((double)m->drive * FL(unit, 0xcc));
    if (U32(unit, 0x158) != 0) {
        fwd = (float)((double)fwd / W16M_C(0x1012b6b4) + fwd);
        side = (float)((double)side / W16M_C(0x1012b6b4) + side);
    }
    dt = GS32(0x1015dc80);
    if (dt < 0x2d) {
        m->ax = (float)(((double)fwd - FL(rec, 0x548)) / W16M_C(0x1012b6b8));
        m->az = (float)(((double)side - FL(rec, 0x550)) / W16M_C(0x1012b6b8));
    } else if (dt > 0) {
        m->ax = (float)(((double)fwd - FL(rec, 0x548)) / (double)dt);
        m->az = (float)(((double)side - FL(rec, 0x550)) / (double)dt);
    } else {
        m->ax = 0.0f;
        m->az = 0.0f;
    }
}

/* In the air or jumping: the original computes a braking term here that it never uses; only the call remains. */
static void W16M_Air(W16Move *m)
{
    float speed = (float)Sim_10084a94(m->vx, 0.0f, m->vz);

    (void)speed;
}

/* A side-step: the thrust (sign: +1 left / -1 right on the x axis) and the dust at a foot. */
static void W16M_Step(W16Move *m, double sign, uint32_t foot)
{
    uint8_t *unit = m->unit;
    uint32_t a, b, c;
    double cx = W16M_C(0x1012b6d4) * FL(unit, 0xcc) + ((double)m->drive * W16M_C(0x1012b6d8)) * FL(unit, 0xcc);
    double cz = W16M_C(0x1012b6d4) * FL(unit, 0xd0) + ((double)m->drive * W16M_C(0x1012b6d8)) * FL(unit, 0xd0);

    m->ax = (float)(sign * cx + m->ax);
    m->az = (float)(-sign * cz + m->az);
    Sim_100a9cfb((void *)(uintptr_t)foot, &a, &b, &c);
    if (W16M_Eq(FL(W16M_BLK(m), 8), W16M_C(0x1012b6dc)))
        Sim_100941c3(W16M_F(a), W16M_F(b), W16M_F(c), -FL(m->rec, 0x548), -FL(m->rec, 0x54c), -FL(m->rec, 0x550));
}

/* The jump jets and the side-steps. Returns nothing; the vertical lift goes to m->lift. */
static void W16M_Jets(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;

    if (S32(rec, 0x510) < 0 && (int8_t)U8(W16M_BLK(m), 0x1e) != 0) {
        FlashText_Start(S32(rec, 0x510) == -1 ? 10 : 4);
        return;
    }
    if (((int8_t)U8(W16M_BLK(m), 0x1e) != 0 || U32(rec, 0x518) != 0) && U32(rec, 0x514) != 0) {
        float boost = 0.0f;

        if ((double)m->vy > W16M_C(0x1012b6c4))
            boost = (float)((((double)GFL(0x1013abc0) - FL(rec, 0x540)) * W16M_C(0x1012b6d0) * m->vy) /
                            W16M_C(0x101453bc));
        m->lift = (float)(((double)boost + FL(rec, 0x540)) + m->lift);
        Sim_10094fcb(FL(unit, 0x58), FL(unit, 0x5c), FL(unit, 0x60), 0.0f, 0.0f, 0.0f);
        return;
    }
    if (W16M_FABS((double)m->vy) >= W16M_C(0x1012b6c4) || U32(unit, 0x80) == 0)
        return;
    m->vy = 0.0f;
    m->lift = 0.0f;
    if (U32(unit, 0x80) != 0 && U32(rec, 0x4f0) == 2 && (int8_t)U8(W16M_BLK(m), 0x58) != 0 && W16M_CODE(m->node) != 0x53 &&
        U32(rec, 0xc3d) == 0 && (W16M_R562(m) & 1) == 0 && (W16M_R562(m) & 0x60e) == 0) {
        W16M_Step(m, 1.0, U32(rec, 0x4c4));
        W16M_R562(m) |= 0x100;
        U32(rec, 0xc29) = 1;
        return;
    }
    if ((int8_t)U8(W16M_BLK(m), 0x58) == 0 && (W16M_R562(m) & 0x100) != 0 && U32(rec, 0xc3d) == 0 &&
        (W16M_R562(m) & 0x60e) == 0) {
        W16M_R562(m) &= 0xfeff;
        U8(W16M_BLK(m), 0x58) = 0;
        U32(rec, 0xc2d) = 1;
        return;
    }
    if (U32(unit, 0x80) != 0 && U32(rec, 0x4f0) == 2 && (int8_t)U8(W16M_BLK(m), 0x57) != 0 && W16M_CODE(m->node) != 0x53 &&
        U32(rec, 0xc3d) == 0 && (W16M_R562(m) & 1) == 0 && (W16M_R562(m) & 0x60e) == 0) {
        W16M_Step(m, -1.0, U32(rec, 0x4c0));
        W16M_R562(m) |= 0x80;
        U32(rec, 0xc29) = 2;
        return;
    }
    if ((int8_t)U8(W16M_BLK(m), 0x57) == 0 && (W16M_R562(m) & 0x80) != 0 && U32(rec, 0xc3d) == 0 &&
        (W16M_R562(m) & 0x60e) == 0) {
        W16M_R562(m) &= 0xff7f;
        U8(W16M_BLK(m), 0x57) = 0;
        U32(rec, 0xc2d) = 1;
    }
}

/* One axis of the motion over the frame: the move (d) and the new speed (v) from the acceleration (a). */
static void W16M_Axis(float a, float *v, float *d)
{
    double dt = (double)GS32(0x1015dc80);
    double t = dt * a;
    float step = (float)t;

    *d = (float)((t * W16M_C(0x1012b6e0) + *v) * dt + *d);
    *v = (float)((double)*v + step);
}

/* The height above the ground after the move (0 for kind 5); sets the unit's +0x15c when well above. */
static void W16M_Height(W16Move *m)
{
    uint8_t *unit = m->unit;

    G32(0x1015dd54) = 0;
    G32(0x1015dd50) = G32(0x1015dd54);
    G32(0x1015dd4c) = G32(0x1015dd50);
    FL(unit, 0x7c) = (float)Sim_1002d961(m->px, m->py, m->pz);
    if (U32(unit, 0) == 5) {
        m->height = 0.0f;
    } else {
        double t = ((double)m->py - FL(m->rec, 0x520)) - FL(unit, 0x7c);

        m->height = (float)t;
        if (t >= W16M_C(0x1012b6e8))
            U32(unit, 0x15c) = 1;
    }
}

/* 1.2 (0x1005d75c): after a contact sound, the force-feedback bump (kind 0xf) towards the contact direction, at
   strength 100, only with the force-feedback library ([0x1015ef3c]). Bug fix (Diego): 1.2 pushes the angle
   (0x1006f4ee, Sim_1008486e's twin, a double) without converting it, so the request gets its low / high dwords as
   angle and strength and 100 as the unused third value; here it gets what that push order means: the angle in whole
   degrees (truncated, as 1.2's other kind-0xf call in the shot does), strength 100, 0. */
static void W16M_Bump(void)
{
    if (G32(0x1015ef3c) == 0)
        return;
#ifdef W16MF_MUTATE /* the checker's negative check only: strength 50 */
    Sim_100382fb(0xf, Render_Ftol((double)Sim_1008486e(GFL(0x1015dd58), GFL(0x1015dd60))), 0x32, 0);
#else
    Sim_100382fb(0xf, Render_Ftol((double)Sim_1008486e(GFL(0x1015dd58), GFL(0x1015dd60))), 0x64, 0);
#endif
}

/* The crash sound of a hard contact, its volume from the speed. */
static void W16M_Crash(W16Move *m)
{
    uint8_t *rec = m->rec;
    const char *name;
    double d;
    float level;

    G32(0x10179cb4) = 1;
    d = Sim_10084a94(FL(rec, 0x554), FL(rec, 0x558), FL(rec, 0x55c));
    level = (float)d;
    if (!(d > W16M_C(0x1012b6f0)))
        return;
    if (m->gear != 0) {
        if (G32(0x1015ec58) != 0)
            Sim_100415eb((int)U32(m->gear, 4), GFL(0x1015dd58), GFL(0x1015dd5c), GFL(0x1015dd60));
        name = SIM_PTR(const char, 0x101453ec);
    } else if (m->object != 0 && (U16(m->object, 2) & 0x200) != 0) {
        name = SIM_PTR(const char, 0x101453f8);
    } else {
        name = SIM_PTR(const char, 0x10145404);
    }
    if ((double)level > W16M_C(0x1012b6f4))
        level = 1.0f;
    else
        level = (float)((double)level / W16M_C(0x1012b6f4));
    level = (float)((double)level * W16M_C(0x1012b6dc));
    Sim_1000ebfb(name, (uint32_t)W16M_Ftol((double)level), 0, 5, 0x32);
    Sim_10061f8e(GFL(0x1015dd58), GFL(0x1015dd5c), GFL(0x1015dd60));
    W16M_Bump();
}

/* The move with its collision tests (Sim_100663c0 twice: the move, then the slide along the contact). Returns 0 when
 * the unit was handed to Sim_1006863d (state 4): the update ends there. */
static int W16M_Collide(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;
    uint32_t saved, object2 = 0;
    float sx, sy, sz;

    m->px = FL(unit, 0x58);
    m->py = FL(unit, 0x5c);
    m->pz = FL(unit, 0x60);
    m->gear = 0;
    U32(unit, 0x84) = 0xffffffffu;
    if (Sim_100663c0(rec, &m->object, &m->gear, m->dx, m->dy, m->dz, &m->px, &m->py, &m->pz) == 0) {
        if (m->own)
            G32(0x10179cb4) = 0;
        W16M_Height(m);
        return 1;
    }
    m->dx = (float)((double)m->px - FL(unit, 0x58));
    m->dy = (float)((double)m->py - FL(unit, 0x5c));
    m->dz = (float)((double)m->pz - FL(unit, 0x60));
    m->vx = FL(rec, 0x548);
    m->vy = FL(rec, 0x54c);
    m->vz = FL(rec, 0x550);
    saved = U32(rec, 0x4f4);
    if (Sim_100663c0(rec, &object2, &m->gear, m->dx, m->dy, m->dz, &sx, &sy, &sz) != 0) {
        if (object2 != m->object) {
            m->px = FL(unit, 0x58);
            m->py = FL(unit, 0x5c);
            m->pz = FL(unit, 0x60);
        }
        FL(rec, 0x548) = (float)((double)m->vx * W16M_C(0x1012b6a8));
        m->vx = FL(rec, 0x548);
        FL(rec, 0x54c) = (float)((double)m->vy * W16M_C(0x1012b6a8));
        m->vy = FL(rec, 0x54c);
        FL(rec, 0x550) = (float)((double)m->vz * W16M_C(0x1012b6a8));
        m->vz = FL(rec, 0x550);
    }
    U32(rec, 0x4f4) = saved;
    if (U32(rec, 0x4f0) == 4) {
        Sim_1006863d((uint32_t *)rec);
        U32(unit, 0x80) = 1;
        return 0;
    }
    W16M_Height(m);
    if (G32(0x1015ec58) != 0 && (double)GFL(0x1015dd5c) > W16M_C(0x1012b6ec) && m->gear != 0) {
        Sim_100415eb((int)U32(m->gear, 4), GFL(0x1015dd58), GFL(0x1015dd5c), GFL(0x1015dd60));
        if (m->own && G32(0x1015ef0c) != 0 && G32(0x10179cb4) == 0) {
            double d;
            float level;

            G32(0x10179cb4) = 1;
            d = Sim_10084a94(FL(rec, 0x554), FL(rec, 0x558), FL(rec, 0x55c));
            level = (float)d;
            if (d > W16M_C(0x1012b6f0)) {
                if ((double)level > W16M_C(0x1012b6f4))
                    level = 25.0f;
                level = (float)((double)GFL(0x1012b6f8) * level / W16M_C(0x1012b6a4) / W16M_C(0x1012b6fc));
                Sim_1000ebfb(SIM_PTR(const char, 0x101453e0), (uint32_t)W16M_Ftol((double)level), 0, 5, 0x32);
                W16M_Bump();
            }
        }
        return 1;
    }
    if ((double)GFL(0x1015dd5c) > W16M_C(0x1012b700) && !((double)m->height >= W16M_C(0x1012b704)) &&
        (double)m->height > W16M_C(0x1012b708)) {
        U32(rec, 0x4f4) = 0;
        U32(&m->height, 0) = 0xbf800000;
        return 1;
    }
    if (m->own && G32(0x1015ef0c) != 0 && G32(0x10179cb4) == 0)
        W16M_Crash(m);
    if (m->gear != 0)
        SIM_FN(void(__cdecl *)(uint8_t *, uint32_t), 0x1006788c)(rec, U32(m->gear, 0x28));
    else
        SIM_FN(void(__cdecl *)(uint8_t *, uint32_t), 0x100680dd)(rec, m->object);
    return 1;
}

/* On the ground or falling: the unit's ground flags (+0x88), the landing and its fall damage. Returns 0 when the unit
 * was handed to Sim_1006863d. */
static int W16M_Land(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;

    U32(unit, 0x80) = 0;
    if (G32(0x109e8230) != 0) {
        uint32_t w = U16(G32(0x109e8230), 0x24);
        uint32_t cls = U16(G32(0x109e8230), 2) & 0xf00u;

        if ((cls & 0x200) != 0 && G32(0x109e9bac + w * 0x50) == 0x3e8)
            U32(unit, 0x158) = 1;
    } else {
        U32(unit, 0x158) = 0;
    }
    if ((double)m->height > (double)FL(rec, 0x520)) {
        U32(unit, 0x88) &= 0xfffffffeu;
        U32(unit, 0x88) |= 4;
        if (m->own && G32(0x101453c4) != 0)
            G32(0x10179cd0) = 1;
        return 1;
    }
    U32(unit, 0x88) &= 0xfffffffbu;
    if (!((double)m->height < W16M_C(0x1012b6c4)) && !(m->height != m->height))
        return 1;
    {
        float impact = (float)(-(double)FL(rec, 0x558) * W16M_C(0x1012b6fc));

        U32(unit, 0x80) = 1;
        if ((W16M_B(m->height) & 0x80000000u) == 0)
            return 1;
        if ((double)impact > W16M_C(0x1012b70c) && U32(rec, 0x4f0) == 4) {
            Sim_1006863d((uint32_t *)rec);
            return 0;
        }
        if ((double)impact > W16M_C(0x1012b710) && U8(G32(0x1017b8c4), 3) != 0) {
            uint8_t shot[0x90];

            memset(shot, 0, sizeof(shot));
            U32(shot, 0x86) = 0;
            U32(shot, 0x70) = (uint32_t)W16M_Ftol((double)impact);
            U32(shot, 0x74) = 0x3dcccccd;
            U32(shot, 0x78) = 0;
            U16(shot, 0x7c) = 1;
            Sim_1006c161(GS32(0x1017b8c8), rec, 0, shot, 6);
            Sim_1006c161(GS32(0x1017b8c8), rec, 0, shot, 5);
            if (Sim_10079feb((int32_t)m->kind) == 0)
                Sim_10061ecd((uint32_t *)rec, m->vy);
        }
        m->vy = 0.0f;
        FL(rec, 0x54c) = m->vy;
        m->py = (float)((double)FL(unit, 0x7c) + FL(rec, 0x520));
    }
    return 1;
}

/* After the move: the drive speed from the slope (for kinds Sim_10079feb does not count), the body height and
 * angles, the node's position and angles. */
static void W16M_Place(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;
    int32_t kind = (int32_t)m->kind;

    if (Sim_10079feb(kind) == 0 &&
        (W16M_FABS((double)GFL(0x1015dd4c)) >= W16M_C(0x1012b6c4) ||
         W16M_FABS((double)GFL(0x1015dd54)) >= W16M_C(0x1012b6c4)) &&
        U32(unit, 0x80) != 0) {
        double slope = (double)FL(unit, 0xcc) * GFL(0x1015dd54) + (double)FL(unit, 0xd0) * GFL(0x1015dd4c);
        float s = (float)slope;

        if (slope > W16M_C(0x101453c0) || -W16M_C(0x101453c0) > (double)s)
            FL(rec, 0x2c) = (float)((double)GFL(0x1013abc0) * s * (double)GS32(0x1015dc80) / W16M_C(0x1012b714) +
                                    FL(rec, 0x2c));
    }
    if ((G32(0x1015ec58) == 0 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8))) && kind == 2)
        m->py = FL(rec, 0x520);
    if (Sim_10079feb(kind) != 0) {
        double drop = (double)FL(unit, 0x7c) - m->ground0;
        float d270 = (float)drop, up = (float)(drop + W16M_C(0x1012b718));
        float down = (float)((double)d270 - W16M_C(0x1012b718));
        float body;

        if ((W16M_B(up) & 0x80000000u) != 0) {
            FL(unit, 0x242) = (float)((double)FL(unit, 0x242) - d270);
        } else if ((double)down > W16M_C(0x1012b6c4) &&
                   ((double)FL(unit, 0x5c) - FL(rec, 0x520)) - W16M_C(0x1012b71c) > (double)FL(unit, 0x7c)) {
            FL(unit, 0x242) = (float)((double)FL(unit, 0x242) - d270);
        }
        body = (float)((double)FL(unit, 0x23e) + FL(unit, 0x242));
        FL(rec, 0x68) = (float)((double)FL(unit, 0x7c) + body);
        if (kind == 8) {
            float rise = (float)((double)FL(rec, 0x68) - FL(rec, 0x6c));
            double q = (double)FL(rec, 0x548) * FL(rec, 0x548) + W16M_C(0x1012b6dc);
            float flat;

            q = q + (double)FL(rec, 0x550) * FL(rec, 0x550);
            flat = (float)W16M_SQRT(q);
            if (W16M_FABS((double)rise) - W16M_C(0x1012b720) > W16M_C(0x1012b6c4))
                FL(rec, 0x58) = -Sim_1008486e(rise, flat);
            else
                U32(rec, 0x58) = 0;
            U32(unit, 0x64) = U32(rec, 0x5c);
        }
        /* 1.2 (0x1005d75c): the body's height +0x68 unless [0x10179ce0] is set */
#ifdef W16MY_MUTATE /* the checker's negative check only: 1.0's */
        m->py = FL(rec, 0x6c);
#else
        m->py = G32(0x10179ce0) != 0 ? FL(rec, 0x6c) : FL(rec, 0x68);
#endif
    }
    FL(unit, 0x58) = m->px;
    FL(unit, 0x5c) = m->py;
    FL(unit, 0x60) = m->pz;
    FL(unit, 0x68) = (float)((double)GS32(0x1015dc80) / W16M_C(0x1012b724) * FL(rec, 0x3c) + FL(unit, 0x68));
    FL(unit, 0x68) = (float)W16M_FMOD((double)FL(unit, 0x68), 360.0);
    if (Sim_10079feb(kind) != 0) {
        U32(unit, 0x6c) = U32(rec, 0x4c);
        FL(unit, 0x6c) = (float)W16M_FMOD((double)FL(unit, 0x6c), 360.0);
    }
    if (kind == 5)
        U32(unit, 0x64) = U32(rec, 0x5c);
    if (kind == 1 || kind == 4 || kind == 10) {
        uint32_t sliding = W16M_R562(m) & 0x180u;

        if (kind == 1 && (S16(unit, 0x1a6) == 3 || S16(unit, 0x1a6) == 2 || U32(unit, 0x15c) != 0) && sliding == 0) {
            U32(rec, 0x58) = 0;
            U32(rec, 0x48) = 0;
        } else {
            FL(rec, 0x58) = (float)(W16M_C(0x1012b728) - (double)Sim_1008486e(GFL(0x1015dd50), GFL(0x1015dd54)));
            FL(rec, 0x48) = (float)((double)Sim_1008486e(GFL(0x1015dd50), GFL(0x1015dd4c)) + W16M_C(0x1012b72c));
        }
        if (kind == 1 && (sliding == 0 || !W16M_Eq(FL(W16M_BLK(m), 8), W16M_C(0x1012b6dc)))) {
            if (!((double)FL(rec, 0x58) >= W16M_C(0x1012b730)))
                U32(rec, 0x58) = 0xc1200000;
            else if ((double)FL(rec, 0x58) > W16M_C(0x1012b734))
                U32(rec, 0x58) = 0x41200000;
            if (!((double)FL(rec, 0x48) >= W16M_C(0x1012b730)))
                U32(rec, 0x48) = 0xc1200000;
            else if ((double)FL(rec, 0x48) > W16M_C(0x1012b734))
                U32(rec, 0x48) = 0x41200000;
        }
        U32(unit, 0x64) = U32(rec, 0x5c);
        U32(unit, 0x6c) = U32(rec, 0x4c);
    }
    FL(rec, 0x548) = m->vx;
    FL(rec, 0x54c) = m->vy;
    FL(rec, 0x550) = m->vz;
    {
        int counted = Sim_10079feb(kind) != 0;

        Sim_100a9d47((void *)(uintptr_t)U32(unit, 0x48), U32(unit, 0x58), U32(unit, 0x5c), U32(unit, 0x60));
        Sim_100aa0da((void *)(uintptr_t)U32(unit, 0x48), U32(unit, 0x64), U32(unit, 0x68), U32(unit, 0x6c),
                     counted ? 2 : 0);
    }
}

/* The cockpit node's height from the record's +0x4d0 and the gun part (+0x4c0). */
static void W16M_Eye(W16Move *m)
{
    uint32_t a, b, c;

    Sim_100aa799((void *)(uintptr_t)U32(m->unit, 0x48));
    Sim_100a9cfb((void *)(uintptr_t)U32(m->rec, 0x4c0), &a, &b, &c);
    Sim_100a9d7c((void *)(uintptr_t)U32(m->unit, 0x48), 0.0f, (float)((double)FL(m->rec, 0x4d0) - W16M_F(b)), 0.0f);
}

/* The crouch / prone states and the stand-up (+0x562 bits 4 / 0x400 / 0x10, the slot's state). */
static void W16M_Stance(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit, *slot = m->slot;

    if ((int8_t)U8(W16M_BLK(m), 0x1d) != 0)
        G32(0x1015f1e0) = 1;
    if ((int8_t)U8(W16M_BLK(m), 0x55) != 0 && (W16M_R562(m) & 8) != 0) {
        U8(W16M_BLK(m), 0x55) = 0;
        if ((U16(rec, 0x560) & 0x20) == 0) {
            W16M_R562(m) |= 0x200;
            W16M_R562(m) &= 0xfff7;
            U32(rec, 0xc15) = 0;
            U32(rec, 0xc1d) = 1;
            U32(rec, 0xc19) = 0;
            m->roll = -FL(rec, 0x564);
        }
    }
    if (S32(rec, 0xc15) > 0 && (W16M_R562(m) & 4) == 0 && (W16M_R562(m) & 0x400) == 0) {
        uint32_t a, b, c;

        /* 1.2: the force-feedback lean (kind 0x11) to one side or the other */
        if (S32(rec, 0xc15) == 1) {
            W16M_R562(m) |= 0x400;
            if (m->own && G32(0x1015ef3c) != 0)
                Sim_100382fb(0x11, 0, 0, 0);
        } else {
            W16M_R562(m) |= 4;
            if (m->own && G32(0x1015ef3c) != 0)
                Sim_100382fb(0x11, 0xb4, 0, 0);
        }
        U32(rec, 0xc19) = U32(rec, 0xc15);
        U32(rec, 0xc1d) = 0;
        U32(rec, 0xc15) = 0;
        W16M_R562(m) &= 0xffef;
        Sim_100aa799((void *)(uintptr_t)U32(unit, 0x48));
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), &a, &b, &c);
        U32(rec, 0x4d0) = b;
        U32(W16M_BLK(m), 8) = 0;
        U8(W16M_BLK(m), 0x25) = 1;
        if (m->own) {
            Sim_1000ebfb(SIM_PTR(const char, 0x10145410), 0xfffffc18u, 0, 5, 0x50);
            FlashText_Start(0x14);
        }
        m->roll = 0.0f;
        U32(rec, 0x564) = 0;
        return;
    }
    if ((W16M_R562(m) & 4) != 0 || (W16M_R562(m) & 0x400) != 0 ||
        (U32(slot, 0x18) != 0 && ((U32(slot, 0x1c) & 0xff) == 5 || (U32(slot, 0x1c) & 0xff) == 4))) {
        W16M_Eye(m);
        m->roll = -FL(rec, 0x564);
        return;
    }
    if (U32(unit, 0x80) != 0 && ((int8_t)U8(W16M_BLK(m), 0x54) != 0 || (int8_t)U8(W16M_BLK(m), 0x5b) != 0) &&
        U32(rec, 0xc3d) == 0 && (W16M_R562(m) & 0x11) == 0 && (W16M_R562(m) & 0x60e) == 0) {
        uint32_t a, b, c;

        W16M_R562(m) |= 0x10;
        if (U32(unit, 0x14) != 1 || (U32(unit, 0x14) == 3 && U32(unit, 0xc) != G32(0x1017b8c8))) {
            U8(W16M_BLK(m), 0x54) = 0;
            U32(rec, 0xc21) = 1;
            U32(rec, 0xc25) = 0;
        }
        Sim_100aa799((void *)(uintptr_t)U32(unit, 0x48));
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), &a, &b, &c);
        U32(rec, 0x4d0) = b;
        if (m->own)
            G32(0x10179cf8) = 0;
        return;
    }
    if ((int8_t)U8(W16M_BLK(m), 0x54) == 0 && (int8_t)U8(W16M_BLK(m), 0x5b) == 0 && (W16M_R562(m) & 0x10) != 0) {
        W16M_Eye(m);
        W16M_R562(m) &= 0xffef;
        U32(rec, 0xc25) = 1;
        U32(rec, 0xc21) = 0;
        return;
    }
    if ((W16M_R562(m) & 0x10) != 0 || (U32(slot, 0x18) != 0 && (U32(slot, 0x1c) & 0xff) == 3)) {
        if ((int8_t)U8(W16M_BLK(m), 0x5b) != 0 && U32(slot, 0x30) != 0 && m->own && G32(0x10179cf8) == 0) {
            int v;

            if (U32(rec, 0x5ff) != 0)
                v = Sim_100386d5((uint32_t *)rec, -1, 1);
            else
                v = Sim_10038913(rec, -1, 0);
            G32(0x10179cf8) = (uint32_t)v;
        }
        W16M_Eye(m);
    }
}

/* The dodge roll (+0x607 / +0x60f) and the evasive jump jets (+0x562 bit 1). */
static void W16M_Dodge(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;

    if (U32(rec, 0x607) != 0) {
        U32(rec, 0x60f) -= G32(0x1015dc80);
        if (U32(unit, 0x14) == 2 || S32(rec, 0x60f) <= 0) {
            U32(rec, 0x607) = 0;
            U32(W16M_BLK(m), 8) = 0x3c000000;
            U8(W16M_BLK(m), 0x25) = 1;
            W16M_R562(m) |= 1;
            U8(W16M_BLK(m), 0x56) = 0;
            if (m->own)
                Sim_1000ec60(SIM_PTR(const char, 0x1014541c), 0xfffff830u, 0, 5, 0x150);
        }
        return;
    }
    if ((int8_t)U8(W16M_BLK(m), 0x56) != 0 && !((double)FL(W16M_BLK(m), 8) > W16M_C(0x1012b738)) &&
        !(W16M_CODE(m->node) == 0x53 && (int8_t)U8(U32(m->node, 0x90), 1) == 0x41) && U32(rec, 0xc3d) == 0 &&
        (W16M_R562(m) & 0x60e) == 0) {
        if ((W16M_R562(m) & 1) == 0) {
            U32(W16M_BLK(m), 8) = 0;
            U8(W16M_BLK(m), 0x25) = 1;
            U32(rec, 0x607) = 1;
            U32(rec, 0x60f) = U32(rec, 0x60b);
            if (m->own) {
                U16(GBYTES(0x101654f0), 0) = (uint16_t)(U16(GBYTES(0x101654f0), 0) + 1);
                Sim_1000ebfb(SIM_PTR(const char, 0x10145428), 0xfffff830u, 0, 5, 0x50);
            }
        } else {
            if (m->own) {
                Sim_1000ecb0(SIM_PTR(const char, 0x10145434));
                Sim_1000ebfb(SIM_PTR(const char, 0x10145440), 0xfffff830u, 0, 5, 0x50);
            }
            W16M_R562(m) &= 0xfffe;
        }
        return;
    }
    if ((W16M_R562(m) & 1) == 0)
        return;
    /* 1.2 (0x1005d75c): +0x560 bit 0x20 cuts the jets whatever the throttle */
#ifdef W16MJ_MUTATE /* the checker's negative check only: 1.0's */
    if (!((double)FL(W16M_BLK(m), 8) > W16M_C(0x1012b6dc)) &&
#else
    if (((U16(rec, 0x560) & 0x20) != 0 || !((double)FL(W16M_BLK(m), 8) > W16M_C(0x1012b6dc))) &&
#endif
        (G32(0x1015ec58) == 0 || m->own || (U32(unit, 0x14) == 3 && U32(unit, 0xc) == G32(0x1017b8c8)))) {
        if (m->own) {
            Sim_1000ecb0(SIM_PTR(const char, 0x1014544c));
            Sim_1000ebfb(SIM_PTR(const char, 0x10145458), 0xfffff830u, 0, 5, 0x50);
        }
        W16M_R562(m) &= 0xfffe;
        return;
    }
    {
        uint32_t a, b, c;

        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c0), &a, &b, &c);
        Sim_100941c3(W16M_F(a), W16M_F(b), W16M_F(c), -FL(rec, 0x548), -FL(rec, 0x54c), -FL(rec, 0x550));
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x4c4), &a, &b, &c);
        Sim_100941c3(W16M_F(a), W16M_F(b), W16M_F(c), -FL(rec, 0x548), -FL(rec, 0x54c), -FL(rec, 0x550));
        if (m->own) {
            double d = Sim_10084a94(FL(rec, 0x548), FL(rec, 0x54c), FL(rec, 0x550));

            Sim_1000e9c9(SIM_PTR(const char, 0x10145464),
                         (uint32_t)W16M_Ftol(d * W16M_C(0x1012b71c) + W16M_C(0x1012b73c)));
        }
    }
}

/* The turret / torso (+0x490), the head (+0x48c) and the unit's aim angles. */
static void W16M_Aim(W16Move *m)
{
    uint8_t *rec = m->rec, *unit = m->unit;
    int32_t kind = (int32_t)m->kind;
    float tilt;

    if (kind == 1 || kind == 9)
        tilt = (float)((double)FL(rec, 0x1c) / W16M_C(0x1012b6c8));
    else if (kind == 4 || kind == 10)
        tilt = 0.0f;
    else
        tilt = (float)((double)FL(rec, 0x1c) * W16M_C(0x1012b6e0));
    if (U32(rec, 0x490) != 0) {
        uint8_t seg[0x38];
        uint32_t saved = 0, a, b, c;
        float tx, ty, tz, t2d0, pitch;
        double len, q;

        if ((W16M_R562(m) & 4) != 0 || (W16M_R562(m) & 0x400) != 0) {
            Sim_100a9ca3((void *)(uintptr_t)U32(unit, 0x4c), (uint32_t)(uintptr_t)&a, (uint32_t)(uintptr_t)&b,
                         (uint32_t)(uintptr_t)&c);
            saved = U32(rec, 0x1c);
            FL(rec, 0x1c) = (float)((double)FL(rec, 0x1c) - W16M_F(a));
        }
        Sim_100d6f16((void *)unit, seg);
        if ((W16M_R562(m) & 4) != 0 || (W16M_R562(m) & 0x400) != 0)
            U32(rec, 0x1c) = saved;
        t2d0 = (float)Sim_100d6e99((const void *)unit);
        Sim_1009cd36(seg, t2d0);
        Sim_100a9cfb((void *)(uintptr_t)U32(rec, 0x490), &a, &b, &c);
        tx = (float)((double)FL(seg, 0xc) - W16M_F(a));
        ty = (float)((double)FL(seg, 0x10) - W16M_F(b));
        tz = (float)((double)FL(seg, 0x14) - W16M_F(c));
        len = Sim_10084a94(tx, ty, tz);
        q = (double)ty / len;
        q = W16M_ATAN(q) * *SIM_PTR(const double, 0x1012b740);
        pitch = (float)(-q - (double)tilt);
        if (kind == 1 || kind == 9) {
            if (U32(rec, 0x494) != 0)
                Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x494),
                             W16M_B((float)((double)pitch / W16M_C(0x1012b6a8))),
                             W16M_B((float)((double)FL(rec, 0xc) / W16M_C(0x1012b6a8))), 0, 1);
            if (W16M_CODE(m->node) != 0x53)
                Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x490), W16M_B(pitch),
                             W16M_B((float)((double)FL(rec, 0xc) / W16M_C(0x1012b6a8))), 0, 0);
        } else {
            Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x490), W16M_B(pitch), 0, 0, 0);
        }
    }
    if (U32(rec, 0x48c) != 0) {
        if (kind == 1 || kind == 9) {
            if ((U16(rec, 0x560) & 0x2000) != 0)
                FL(unit, 0x70) = (float)((double)FL(rec, 0x1c) / W16M_C(0x1012b6a8) + m->roll);
            else
                FL(unit, 0x70) = m->roll;
        } else {
            if ((U16(rec, 0x560) & 0x2000) != 0)
                FL(unit, 0x70) = tilt;
            else
                U32(unit, 0x70) = 0;
        }
        U32(unit, 0x74) = U32(rec, 0xc);
        U32(unit, 0x78) = 0;
        if (kind == 1 || kind == 9)
            Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x48c), W16M_B((float)((double)tilt / W16M_C(0x1012b6a8))),
                         W16M_B((float)((double)FL(rec, 0xc) / W16M_C(0x1012b748))), U32(unit, 0x78), 1);
        else if (kind == 4 || kind == 10)
            Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x48c), 0, U32(unit, 0x74), 0, 1);
        else
            Sim_100aa0da((void *)(uintptr_t)U32(rec, 0x48c), U32(unit, 0x70), U32(unit, 0x74), U32(unit, 0x78), 1);
    } else {
        FL(unit, 0x70) = m->roll;
    }
    if (m->own && W16M_CODE(m->node) != 0x53) {
        double k = ((double)FL(G32(0x1014cae0), 0x18) - *SIM_PTR(const double, 0x1012b750)) *
                   *SIM_PTR(const double, 0x1012b758) + *SIM_PTR(const double, 0x1012b750);

        FL(unit, 0x74) = (float)((double)FL(rec, 0xc) * k);
    }
}

// FUNCTION: HGSIM 0x1006e22c
void __cdecl Sim_1006e22c(uint8_t *rec)
{
    W16Move m;
    float heading;

    memset(&m, 0, sizeof(m));
    if (rec == 0)
        return;
    m.rec = rec;
    m.unit = BYTES(U32(rec, 0));
    m.node = U32(m.unit, 0x48);
    if ((U32(rec, 0x4f0) == 4 && U32(m.unit, 0x80) != 0) || (U16(rec, 0x560) & 0x100) != 0)
        return;
    m.own = U32(m.unit, 4) == G32(0x1017b8c8);
    m.slot = GBYTES(0x10a1d140 + U32(m.unit, 4) * 0x38);
    U32(m.unit, 0x15c) = 0;
    m.kind = U32(m.unit, 0);
    m.ground0 = FL(m.unit, 0x7c);
    W16M_Timers(&m);
    if (U32(m.unit, 0x80) != 0 && (int8_t)U8(W16M_BLK(&m), 0x1e) == 0 && U32(rec, 0x518) == 0)
        W16M_Drive(&m);
    else
        W16M_Air(&m);
    W16M_Jets(&m);
    W16M_Axis(m.ax, &m.vx, &m.dx);
    W16M_Axis(m.az, &m.vz, &m.dz);
    if (!(W16M_FABS((double)m.vx) >= W16M_C(0x1012b6e4))) {
        m.vx = 0.0f;
        m.dx = 0.0f;
    }
    if (!(W16M_FABS((double)m.vz) >= W16M_C(0x1012b6e4))) {
        m.vz = 0.0f;
        m.dz = 0.0f;
    }
    if ((W16M_Eq(m.vy, W16M_C(0x1012b6dc)) && W16M_Eq(m.lift, W16M_C(0x1012b6dc))) || G32(0x10179ce0) == 0)
        m.dy = 0.0f;
    else
        W16M_Axis(m.lift, &m.vy, &m.dy);
    FL(rec, 0x554) = m.vx;
    FL(rec, 0x558) = m.vy;
    FL(rec, 0x55c) = m.vz;
    if (!W16M_Collide(&m))
        return;
    if (!W16M_Land(&m))
        return;
    W16M_Place(&m);
    W16M_Stance(&m);
    W16M_Dodge(&m);
    Sim_100aa799((void *)(uintptr_t)U32(m.unit, 0x48));
    if (m.own) {
        if (W16M_CODE(m.node) == 0x53) {
            U32(rec, 8) = 0;
            U32(rec, 0xc) = 0;
        }
        Sim_100d6d6a((void *)m.unit);
    }
    W16M_Aim(&m);
    heading = FL(m.unit, 0x68);
    FL(m.unit, 0xd0) = Sim_10084766(heading);
    FL(m.unit, 0xcc) = Sim_10084766((float)((double)heading + W16M_C(0x1012b728)));
}
