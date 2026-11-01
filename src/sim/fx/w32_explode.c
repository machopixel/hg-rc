/* Round 32, E: 1.2's explosions: tracer (type 12, replaces 1.0's Sim_10096ac3), missile (10, Sim_10095e0a), grenade (8,
   Sim_10094fcb), the grenade's bounce dust (types 17..20, new in 1.2), smoke (7, Sim_100949e8), gun (6, from
   1.2's 0x100bb425), skate dust (5, Sim_100941c3), shockwave (4, Sim_10092f9d) and sparks /
   puffs (13..16, Sim_100939a3). */
#include "w32.h"
#include "../unsorted_1008/w8w.h"
#include "../render/w2n.h"
#include "../unsorted_1002/w7l.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_1009/w3p.h"
#include "../unsorted_1009/w6g.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"
#include "../unsorted_100d/w8z.h"
#include "../util/util.h"
#include "../util/access.h"

/* the graphics library, still original: a pixel on the 2D surface */
#define W32T_CALL_100e84f0 SIM_FN(void(__cdecl *)(void *, int32_t, int32_t, uint32_t), 0x100e84f0)
#define W32T_NODE(rec) ((void *)(uintptr_t)U32(rec, 0x332))

static uint32_t W32t_Bits(float f)
{
    return *(uint32_t *)&f;
}

// FUNCTION: HGSIM12 0x10083dde
int __cdecl Sim12_10083dde(uint8_t *rec)
{
#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if ((int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)) <= 0x17 && (U16(rec, 0x10) & 2) == 0) {
#else
    if ((int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)) <= 0x16 && (U16(rec, 0x10) & 2) == 0) {
#endif
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), W32_F(rec, 0x352), W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        U32(rec, 0x346) = 0x1f;
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x10083e8a
int __cdecl Sim12_10083e8a(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float x, y, z, a, c;
    int32_t i;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    /* 1.2 projects (0, 0, node z) with x and y in the same variable; the result is not used */
    a = 0.0f;
    c = *(float *)&pz;
    Sim_100d2669(&a, &a, &c);
    for (i = 0; i < 8; i++) {
        x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
        y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
        z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
        if (flags == 0)
            return 0;
        if (G32(0x1017b940) == 0) {
            W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), flags);
            continue;
        }
#ifdef W32E_MUTATE /* the checker's negative check only: the particle one size step bigger */
        U32(rec + i * 0x32, 0x38) = (uint32_t)W32_ParticleSize(rec, 1) + 1;
#else
        U32(rec + i * 0x32, 0x38) = (uint32_t)W32_ParticleSize(rec, 1);
#endif
        W32_F(points + i * 0x10, 0) = x;
        W32_F(points + i * 0x10, 4) = y;
        W32_F(points + i * 0x10, 8) = z;
        U32(points + i * 0x10, 0xc) = 1;
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

float W32_InSight(float x, float y, float z)
{
    uint8_t *cam = BYTES(G32(0x1014cae0));
    uint8_t seg[0x38];
    uint32_t hit;
    float vx, vy, vz, d;
    double full;

    vx = (float)((double)x - (double)W32_F(cam, 0));
    vy = (float)((double)y - (double)W32_F(cam, 4));
    vz = (float)((double)z - (double)W32_F(cam, 8));
    full = Sim_10084a94(vx, vy, vz);
    d = (float)full;
    if (!(full > (double)W32_F(cam, 0x40))) {       /* fcomp / test ah, 0x41: within reach */
        float tx = x, ty = y, tz = z;

        if (Sim_100d2669(&tx, &ty, &tz) != 0) {
            Sim_1009cab0((float *)seg, x, y, z, W32_F(cam, 0), W32_F(cam, 4), W32_F(cam, 8));
            Sim_1009cc82(seg);
            hit = 0;
            if (Sim_1002dda9(seg, &hit, -1) == 0)
                return d;
        }
    }
    return 0.0f;
}

uint8_t *W32_Explosion(float x, float y, float z, float dx, float dy, float dz, int32_t type, uint32_t update,
                       uint32_t draw, int speed100, double vdiv, double sdiv, float *step)
{
    uint8_t *rec, *block;
    float dist = W32_InSight(x, y, z);

    if (!(dist < 0.0f || dist > 0.0f))               /* fcomp 0.0 / test ah, 0x40 */
        return 0;
    rec = W32_NewSystem(update, draw, 0, 0, type);
    if (rec == 0)
        return 0;
#ifdef W32X_MUTATE /* the checker's negative check only: the scale a fifth */
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 5.0);
#else
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 4.0);
#endif
    W32_SetTemplate(rec);
    block = BYTES(U32(BYTES(U32(rec, 0x332)), 0x8c));
    if (speed100)
        W32_F(block, 0x50) = (float)Sim_10084a94((float)((double)dx * 100.0), (float)((double)dy * 100.0),
                                                 (float)((double)dz * 100.0));
    else
        W32_F(block, 0x50) = (float)Sim_10084a94(dx, dy, dz);
    W32_F(block, 0x44) = x;
    W32_F(block, 0x48) = y;
    W32_F(block, 0x4c) = z;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    W32_F(rec, 0x34e) = (float)((double)dx / vdiv);
    W32_F(rec, 0x352) = (float)((double)dy / vdiv);
    W32_F(rec, 0x356) = (float)((double)dz / vdiv);
    step[0] = (float)((double)dx / sdiv);
    step[1] = (float)((double)dy / sdiv);
    step[2] = (float)((double)dz / sdiv);
    Sim_100aa39f(W32T_NODE(rec));
    Sim_100a9d47(W32T_NODE(rec), W32t_Bits(x), W32t_Bits(y), W32t_Bits(z));
    Sim_100aa799(W32T_NODE(rec));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(rec, 0x33e) = G32(0x1015dc78);
    return rec;
}

// FUNCTION: HGSIM12 0x10084362
void __cdecl Sim12_10084362(float x, float y, float z, float dx, float dy, float dz)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, 12, (uint32_t)(uintptr_t)Sim12_10083dde,
                        (uint32_t)(uintptr_t)Sim12_10083e8a, 1, 8.0, 8.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = ax;
        W32_F(p, 4) = ay;
        W32_F(p, 8) = az;
    }
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x10081afb
int __cdecl Sim12_10081afb(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0x6f && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0x6e && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        U32(rec, 0x346) = *GBYTES(0x101d09c0u + (uint32_t)Render_Ftol((double)age / 110.0 * 15.0));
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x10081bdc
int __cdecl Sim12_10081bdc(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float hx, hy, hz, x, y, z, x2, y2, z2, a, c;
    int32_t i, size;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    hx = (float)((double)W32_F(rec, 0x34e) / 10.0);
    hy = (float)((double)W32_F(rec, 0x352) / 10.0);
    hz = (float)((double)W32_F(rec, 0x356) / 10.0);
    a = 0.0f;
    c = *(float *)&pz;
    Sim_100d2669(&a, &a, &c);
    if (!(c > 20000.0f)) {                          /* fcomp / test ah, 0x41 */
        for (i = 0; i < 8; i++) {
            x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
            y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
            z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
            U32(points + i * 0x10, 0xc) = 0;
            if (Sim_100d2669(&x, &y, &z) == 0)
                continue;
            flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
            if (flags == 0)
                return 0;
            size = W32_ParticleSize(rec, 1);
            /* one tenth of the velocity on, projected; 1.2 does not use the result */
            x2 = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px + (double)hx);
            y2 = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py + (double)hy);
            z2 = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz + (double)hz);
            Sim_100d2669(&x2, &y2, &z2);
            W32_F(points + i * 0x10, 0) = x;
            W32_F(points + i * 0x10, 4) = y;
            W32_F(points + i * 0x10, 8) = z;
            U32(points + i * 0x10, 0xc) = 1;
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
            size++;
#endif
            U32(rec + i * 0x32, 0x38) = (uint32_t)size;
        }
    } else {
        for (i = 0; i < 8; i++) {
            x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
            y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
            z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
            U32(points + i * 0x10, 0xc) = 0;
            if (Sim_100d2669(&x, &y, &z) == 0)
                continue;
            flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
            if (flags == 0)
                return 0;
            size = W32_ParticleSize(rec, 1);
            W32_F(points + i * 0x10, 0) = x;
            W32_F(points + i * 0x10, 4) = y;
            W32_F(points + i * 0x10, 8) = z;
            U32(points + i * 0x10, 0xc) = 1;
            U32(rec + i * 0x32, 0x38) = (uint32_t)size;
        }
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

// FUNCTION: HGSIM12 0x10082556
void __cdecl Sim12_10082556(float x, float y, float z, float dx, float dy, float dz)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, 10, (uint32_t)(uintptr_t)Sim12_10081afb,
                        (uint32_t)(uintptr_t)Sim12_10081bdc, 0, 8.0, 8.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = (float)((double)Sim_1009c929(10) + (double)ax - 5.0);
        W32_F(p, 4) = (float)((double)Sim_1009c929(0x32) + (double)ay - 25.0);
        W32_F(p, 8) = (float)((double)Sim_1009c929(10) + (double)az - 5.0);
    }
    U32(rec, 0x402) = 10;
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x1007ffb3
int __cdecl Sim12_1007ffb3(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, i;
    uint8_t *p;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0x16b && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0x16a && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        U32(rec, 0x346) = *GBYTES(0x10215af0u + (uint32_t)Render_Ftol((double)age / 362.0 * 15.0));
        for (i = 0; i < 8; i++) {
            p = rec + 0x12 + i * 0x32;
            W32_F(p, 0) = (float)((double)(Sim_1009c929(0x3c) - 0x1e) + (double)W32_F(p, 0));
            W32_F(p, 4) = (float)((double)(Sim_1009c929(0x3c) - 0x1e) + (double)W32_F(p, 4));
            W32_F(p, 8) = (float)((double)(Sim_1009c929(0x3c) - 0x1e) + (double)W32_F(p, 8));
        }
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1008011d
int __cdecl Sim12_1008011d(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float hx, hy, hz, x, y, z;
    int32_t i, size;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    /* one tenth of the velocity: 1.2 computes it and does not use it */
    hx = (float)((double)W32_F(rec, 0x34e) / 10.0);
    hy = (float)((double)W32_F(rec, 0x352) / 10.0);
    hz = (float)((double)W32_F(rec, 0x356) / 10.0);
    (void)hx;
    (void)hy;
    (void)hz;
    for (i = 0; i < 8; i++) {
        x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
        y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
        z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
        if (flags == 0)
            return 0;
        size = W32_ParticleSize(rec, 1);
        W32_F(points + i * 0x10, 0) = x;
        W32_F(points + i * 0x10, 4) = y;
        W32_F(points + i * 0x10, 8) = z;
        U32(points + i * 0x10, 0xc) = 1;
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
        size++;
#endif
        U32(rec + i * 0x32, 0x38) = (uint32_t)size;
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

// FUNCTION: HGSIM12 0x100805dc
void __cdecl Sim12_100805dc(float x, float y, float z, float dx, float dy, float dz)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, 8, (uint32_t)(uintptr_t)Sim12_1007ffb3,
                        (uint32_t)(uintptr_t)Sim12_1008011d, 0, 256.0, 16.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = (float)((double)(Sim_1009c929(0x32) - 0x19) + (double)ax);
        W32_F(p, 4) = (float)((double)(Sim_1009c929(0x32) - 0x19) + (double)ay);
        W32_F(p, 8) = (float)((double)(Sim_1009c929(0x32) - 0x19) + (double)az);
    }
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x10084a87
int __cdecl Sim12_10084a87(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, k;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0x5b && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0x5a && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        k = Render_Ftol((double)age / 90.0 * 3.0);
        U32(rec, 0x346) = *GBYTES(0x10215bc0u + (uint32_t)k);
        U32(rec, 0x34a) = *GBYTES(0x10215bb0u + (uint32_t)k);
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x10084b83
int __cdecl Sim12_10084b83(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float hx, hy, hz, x, y, z, a, c;
    int32_t i, size;
    uint8_t *p;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    /* one tenth of the velocity: 1.2 computes it and does not use it */
    hx = (float)((double)W32_F(rec, 0x34e) / 10.0);
    hy = (float)((double)W32_F(rec, 0x352) / 10.0);
    hz = (float)((double)W32_F(rec, 0x356) / 10.0);
    (void)hx;
    (void)hy;
    (void)hz;
    a = 0.0f;
    c = *(float *)&pz;
    Sim_100d2669(&a, &a, &c);
    for (i = 0; i < 8; i++) {
        p = rec + i * 0x32;
        W32_F(p, 0x12) = (float)((double)W32_F(p, 0x12) + (double)W32_F(p, 0x1e));
        W32_F(p, 0x16) = (float)((double)W32_F(p, 0x16) + (double)W32_F(p, 0x22));
        W32_F(p, 0x1a) = (float)((double)W32_F(p, 0x1a) + (double)W32_F(p, 0x26));
        x = (float)((double)*(float *)&px + (double)W32_F(p, 0x12));
        y = (float)((double)*(float *)&py + (double)W32_F(p, 0x16));
        z = (float)((double)*(float *)&pz + (double)W32_F(p, 0x1a));
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
        if (flags == 0)
            return 0;
        size = W32_ParticleScale(rec, Render_Ftol((double)(int32_t)(U32(rec, 0x34a) * 0x7d0u) / (double)z));
        if (size < 1) {
            if (G32(0x1017b940) == 0)
                W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), flags);
            else
                size = g_w32_1011e700[S32(rec, 0x402)].size >> 1;
        }
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
        size++;
#endif
        if (size >= 1) {
            W32_F(points + i * 0x10, 0) = x;
            W32_F(points + i * 0x10, 4) = y;
            W32_F(points + i * 0x10, 8) = z;
            U32(points + i * 0x10, 0xc) = 1;
            U32(rec + i * 0x32, 0x38) = (uint32_t)size;
        }
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

// FUNCTION: HGSIM12 0x1008514d
void __cdecl Sim12_1008514d(float x, float y, float z, float dx, float dy, float dz, uint32_t unused, int32_t kind)
{
    uint8_t *rec, *block, *p;
    float dist, ax = 0.0f, ay = 0.0f, az = 0.0f, sx = 0.0f, sy = 0.0f, sz = 0.0f;
    int32_t type = 0x11, i;

    (void)unused;
    if (G32(0x1017b940) == 0)
        return;
    dist = W32_InSight(x, y, z);
    if (!(dist < 0.0f || dist > 0.0f))               /* fcomp 0.0 / test ah, 0x40 */
        return;
    if (kind >= 0xf && kind <= 0x1b)
        type = 0x14;
    else if (kind == 0x25 || kind == 0x2d)
        type = 0x13;
    else if ((kind >= 0x21 && kind <= 0x23) || kind == 0x28)
        type = 0x12;
    else
        type = 0x11;
    rec = W32_NewSystem((uint32_t)(uintptr_t)Sim12_10084a87, (uint32_t)(uintptr_t)Sim12_10084b83, 0, 0, type);
    if (rec == 0)
        return;
#ifdef W32X_MUTATE /* the checker's negative check only: the scale a fifth */
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 5.0);
#else
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 4.0);
#endif
    W32_SetTemplate(rec);
    block = BYTES(U32(BYTES(U32(rec, 0x332)), 0x8c));
    W32_F(block, 0x50) = (float)Sim_10084a94(dx, dy, dz);
    W32_F(block, 0x44) = x;
    W32_F(block, 0x48) = y;
    W32_F(block, 0x4c) = z;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    W32_F(rec, 0x34e) = (float)(-(double)dx / 5.0);
    U32(rec, 0x352) = 0xc1200000u;                  /* -10.0 */
    W32_F(rec, 0x356) = (float)(-(double)dz / 5.0);
    Sim_100aa39f(W32T_NODE(rec));
    Sim_100a9d47(W32T_NODE(rec), W32t_Bits(x), W32t_Bits(y), W32t_Bits(z));
    Sim_100aa799(W32T_NODE(rec));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(rec, 0x33e) = G32(0x1015dc78);
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)sx);
        ay = (float)((double)ay + (double)sy);
        az = (float)((double)az + (double)sz);
        W32_F(p, 0) = (float)((double)Sim_1009c929(10) + (double)ax - 5.0);
        W32_F(p, 4) = (float)((double)Sim_1009c929(10) + (double)ay - 5.0);
        W32_F(p, 8) = (float)((double)Sim_1009c929(10) + (double)az - 5.0);
        W32_F(p, 0xc) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x34e) - 20.0);
        W32_F(p, 0x10) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x352) - 20.0);
        W32_F(p, 0x14) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x356) - 20.0);
    }
    U32(rec, 0x402) = (uint32_t)type;
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x1007e99d
int __cdecl Sim12_1007e99d(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, k, i;
    uint8_t *p;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0x220 && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0x21f && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        k = Render_Ftol((double)age / 543.0 * 15.0);
        U32(rec, 0x346) = *GBYTES(0x10215bd0u + (uint32_t)k);
        U32(rec, 0x34a) = *GBYTES(0x10217b20u + (uint32_t)k);
        for (i = 0; i < 8; i++) {
            p = rec + i * 0x32;
            W32_F(p, 0x12) = (float)((double)(Sim_1009c929(0x14) - 10) + (double)W32_F(p, 0x12));
            W32_F(p, 0x16) = (float)((double)(Sim_1009c929(0x14) - 10) + (double)W32_F(p, 0x16));
            W32_F(p, 0x1a) = (float)((double)(Sim_1009c929(0x14) - 10) + (double)W32_F(p, 0x1a));
        }
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1007eb46
int __cdecl Sim12_1007eb46(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0, c, a12;
    float hx, hy, hz, x, y, z;
    int32_t i, size, a, b;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    /* one tenth of the velocity: 1.2 computes it and does not use it */
    hx = (float)((double)W32_F(rec, 0x34e) / 10.0);
    hy = (float)((double)W32_F(rec, 0x352) / 10.0);
    hz = (float)((double)W32_F(rec, 0x356) / 10.0);
    (void)hx;
    (void)hy;
    (void)hz;
    for (i = 0; i < 8; i++) {
        x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
        y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
        z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, 0xf0);
        if (flags == 0)
            return 0;
        size = W32_ParticleScale(rec, Render_Ftol((double)(int32_t)(U32(rec, 0x34a) * 0x7d0u) / (double)z));
        if (G32(0x1017b940) == 0) {
            if (size < 1) {
                W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), flags);
                continue;
            }
            /* a block of pixel pairs, every second row and column */
            for (a = 0; a < size; a += 2) {
                for (b = 0; b < size; b += 2) {
                    W32_F(rec, 0x36a) = (float)((double)a + (double)x);
                    W32_F(rec, 0x36e) = (float)((double)b + (double)y);
                    W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(W32_F(rec, 0x36a)),
                                       Render_Ftol(W32_F(rec, 0x36e)), flags);
                    W32_F(rec, 0x36a) = (float)((double)W32_F(rec, 0x36a) + 1.0);
                    W32_F(rec, 0x36e) = (float)((double)W32_F(rec, 0x36e) + 1.0);
                    W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(W32_F(rec, 0x36a)),
                                       Render_Ftol(W32_F(rec, 0x36e)), flags);
                }
            }
            continue;
        }
        if (size < 1)
            size = g_w32_1011e700[S32(rec, 0x402)].size >> 1;
        /* the first corner's colour from the colour table [0x10185760] (256 entries) by the flags. For a textured
           type the flags are group | 0x7000 and 1.2 reads past its table (1.2 0x10152a60) into its scene-object
           table (1.2 0x10164d40, ours 0x101a2100, 0x402 records of 0x88): the same bytes are read from ours
           (DECISIONS.md, round 32) */
        a12 = 0x10152a60u + flags * 4u;
        if (flags < 0x100)
            c = G32(0x10185760u + flags * 4u);
        else if (a12 >= 0x10164d40u && a12 - 0x10164d40u <= 0x402u * 0x88u - 4u)
#ifdef W32C_MUTATE /* the checker's negative check only: one byte off */
            c = G32(0x101a2101u + (a12 - 0x10164d40u));
#else
            c = G32(0x101a2100u + (a12 - 0x10164d40u));
#endif
        else
            goto keep;  /* not reached: a group is at most 0x1ff */
        rec[0x37e] = (uint8_t)(c >> 16);
        rec[0x37f] = (uint8_t)(c >> 8);
        rec[0x380] = (uint8_t)c;
    keep:
        W32_F(points + i * 0x10, 0) = x;
        W32_F(points + i * 0x10, 4) = y;
        W32_F(points + i * 0x10, 8) = z;
        U32(points + i * 0x10, 0xc) = 1;
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
        size++;
#endif
        U32(rec + i * 0x32, 0x38) = (uint32_t)size;
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

// FUNCTION: HGSIM12 0x1007f8a9
void __cdecl Sim12_1007f8a9(float x, float y, float z, float dx, float dy, float dz)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, 7, (uint32_t)(uintptr_t)Sim12_1007e99d,
                        (uint32_t)(uintptr_t)Sim12_1007eb46, 0, 256.0, 8.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = ax;
        W32_F(p, 4) = ay;
        W32_F(p, 8) = az;
    }
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x1007f19f
void __cdecl Sim12_1007f19f(float x, float y, float z, float dx, float dy, float dz)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, 6, (uint32_t)(uintptr_t)Sim12_1007e99d,
                        (uint32_t)(uintptr_t)Sim12_1007eb46, 0, 256.0, 8.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = ax;
        W32_F(p, 4) = ay;
        W32_F(p, 8) = az;
    }
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x1007dbe7
int __cdecl Sim12_1007dbe7(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, k, i, own;
    uint8_t *p;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0xb6 && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0xb5 && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        k = Render_Ftol((double)age / 181.0 * 15.0);
        U32(rec, 0x34a) = *GBYTES(0x10217b20u + (uint32_t)k);
        own = (U16(rec, 0x10) & 4) == 0;
        for (i = 0; i < 8; i++) {
            p = rec + 0x12 + i * 0x32;
            /* the particle's frame from its colour row (one of the two at 0x10215b20) by age */
            if (own)
                U32(p, 0x1e) = U32(BYTES(U32(p, 0x1a)), (uint32_t)k * 4);
            W32_F(p, 0) = (float)((double)(Sim_1009c929(0x64) - 0x32) + (double)W32_F(p, 0));
            W32_F(p, 4) = (float)((double)(Sim_1009c929(0x14) - 10) + (double)W32_F(p, 4));
            W32_F(p, 8) = (float)((double)(Sim_1009c929(0x64) - 0x32) + (double)W32_F(p, 8));
        }
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1007dd89
int __cdecl Sim12_1007dd89(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float x, y, z;
    int32_t i, size;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    for (i = 0; i < 8; i++) {
        x = (float)((double)W32_F(rec + i * 0x32, 0x12) + (double)*(float *)&px);
        y = (float)((double)W32_F(rec + i * 0x32, 0x16) + (double)*(float *)&py);
        z = (float)((double)W32_F(rec + i * 0x32, 0x1a) + (double)*(float *)&pz);
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, U32(rec + i * 0x32, 0x30));
        if (flags == 0)
            return 0;
        size = W32_ParticleScale(rec, Render_Ftol((double)(int32_t)(U32(rec, 0x34a) * 0x7d0u) / (double)z));
        if (size < 1) {
            if (G32(0x1017b940) == 0)
                W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), flags);
            else
                size = g_w32_1011e700[S32(rec, 0x402)].size >> 1;
        }
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
        size++;
#endif
        if (size >= 1) {
            W32_F(points + i * 0x10, 0) = x;
            W32_F(points + i * 0x10, 4) = y;
            W32_F(points + i * 0x10, 8) = z;
            U32(points + i * 0x10, 0xc) = 1;
            U32(rec + i * 0x32, 0x38) = (uint32_t)size;
        }
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

/* the skate dust's spawn; 1.2's footsteps (0x10086007) are a copy of it with type 0x15 and their own copies of its
   update and draw */
static void W32e_Dust(float x, float y, float z, float dx, float dy, float dz, int32_t type, uint32_t update,
                      uint32_t draw)
{
    uint8_t *rec, *p;
    float step[3], ax, ay, az;
    int32_t i;

    rec = W32_Explosion(x, y, z, dx, dy, dz, type, update, draw, 0, 256.0, 8.0, step);
    if (rec == 0)
        return;
    ax = step[0];
    ay = step[1];
    az = step[2];
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)step[0]);
        ay = (float)((double)ay + (double)step[1]);
        az = (float)((double)az + (double)step[2]);
        W32_F(p, 0) = ax;
        W32_F(p, 4) = ay;
        W32_F(p, 8) = az;
        /* one of the two frame rows of 16 dwords at 0x10215b20 */
#ifdef W32K_MUTATE /* the checker's negative check only: always the first row */
        (void)Sim_1009c929(2);
        U32(p, 0x1a) = (uint32_t)(uintptr_t)GBYTES(0x10215b20u);
#else
        U32(p, 0x1a) = (uint32_t)(uintptr_t)GBYTES(0x10215b20u + ((uint32_t)Sim_1009c929(2) << 6));
#endif
    }
    U32(rec, 0xc) = 1;
}

// FUNCTION: HGSIM12 0x1007e25c
void __cdecl Sim12_1007e25c(float x, float y, float z, float dx, float dy, float dz)
{
    W32e_Dust(x, y, z, dx, dy, dz, 5, (uint32_t)(uintptr_t)Sim12_1007dbe7, (uint32_t)(uintptr_t)Sim12_1007dd89);
}

// FUNCTION: HGSIM12 0x10085992
int __cdecl Sim12_10085992(uint8_t *rec)
{
    return Sim12_1007dbe7(rec);
}

// FUNCTION: HGSIM12 0x10085b34
int __cdecl Sim12_10085b34(uint8_t *rec)
{
    return Sim12_1007dd89(rec);
}

// FUNCTION: HGSIM12 0x10086007
void __cdecl Sim12_10086007(float x, float y, float z, float dx, float dy, float dz)
{
#ifdef W32FS_MUTATE /* the checker's negative check only: the skate dust's type */
    W32e_Dust(x, y, z, dx, dy, dz, 5, (uint32_t)(uintptr_t)Sim12_10085992, (uint32_t)(uintptr_t)Sim12_10085b34);
#else
    W32e_Dust(x, y, z, dx, dy, dz, 0x15, (uint32_t)(uintptr_t)Sim12_10085992, (uint32_t)(uintptr_t)Sim12_10085b34);
#endif
}

// FUNCTION: HGSIM12 0x1007bb48
int __cdecl Sim12_1007bb48(uint8_t *rec)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, k, i;
    uint8_t *p;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= S32(rec, 0x342) + 1 && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= S32(rec, 0x342) && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        k = Render_Ftol((double)age / (double)S32(rec, 0x342) * 15.0);
        U32(rec, 0x34a) = *GBYTES(0x10215bb0u + (uint32_t)k);
        U32(rec, 0x346) = 0xfe;
        for (i = 0; i < 8; i++) {
            p = rec + 0x12 + i * 0x32;
            W32_F(p, 0) = (float)((double)W32_F(p, 0) + (double)W32_F(p, 0xc));
            W32_F(p, 4) = (float)((double)W32_F(p, 4) + (double)W32_F(p, 0x10));
            W32_F(p, 8) = (float)((double)W32_F(p, 8) + (double)W32_F(p, 0x14));
        }
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1007bca5
int __cdecl Sim12_1007bca5(uint8_t *rec)
{
    uint8_t points[8 * 0x10];
    uint32_t px, py, pz, flags = 0;
    float x, y, z, a, c;
    int32_t i, size;

    if (rec == 0)
        return 0;
    Sim_100a9cfb(W32T_NODE(rec), &px, &py, &pz);
    a = 0.0f;
    c = *(float *)&pz;
    Sim_100d2669(&a, &a, &c);
    for (i = 0; i < 8; i++) {
        x = (float)((double)*(float *)&px + (double)W32_F(rec + i * 0x32, 0x12));
        y = (float)((double)*(float *)&py + (double)W32_F(rec + i * 0x32, 0x16));
        z = (float)((double)*(float *)&pz + (double)W32_F(rec + i * 0x32, 0x1a));
        U32(points + i * 0x10, 0xc) = 0;
        if (Sim_100d2669(&x, &y, &z) == 0)
            continue;
        flags = W32_ParticleFlags(rec, i, U32(rec, 0x346));
        if (flags == 0)
            return 0;
        size = W32_ParticleScale(rec, Render_Ftol((double)(int32_t)(U32(rec, 0x34a) * 0x7d0u) / (double)z));
        if (size < 1) {
            if (G32(0x1017b940) == 0)
                W32T_CALL_100e84f0(GBYTES(0x109e3e00), Render_Ftol(x), Render_Ftol(y), flags);
            else
                size = g_w32_1011e700[S32(rec, 0x402)].size >> 1;
        }
#ifdef W32M_MUTATE /* the checker's negative check only: one size step bigger */
        size++;
#endif
        if (size >= 1) {
            W32_F(points + i * 0x10, 0) = x;
            W32_F(points + i * 0x10, 4) = y;
            W32_F(points + i * 0x10, 8) = z;
            U32(points + i * 0x10, 0xc) = 1;
            U32(rec + i * 0x32, 0x38) = (uint32_t)size;
        }
    }
    Sim12_10078c29(rec, points, flags, i);
    return 1;
}

// FUNCTION: HGSIM12 0x1007c1a1
void __cdecl Sim12_1007c1a1(float x, float y, float z, float dx, float dy, float dz, int32_t strength)
{
    uint8_t *rec, *block, *p;
    float dist, ax = 0.0f, ay = 0.0f, az = 0.0f, sx = 0.0f, sy = 0.0f, sz = 0.0f, radius, angle;
    int32_t i, n;

    if (strength == 0)
        return;
    dist = W32_InSight(x, y, z);
    if (!(dist < 0.0f || dist > 0.0f))               /* fcomp 0.0 / test ah, 0x40 */
        return;
    rec = W32_NewSystem((uint32_t)(uintptr_t)Sim12_1007bb48, (uint32_t)(uintptr_t)Sim12_1007bca5, 0, 0, 4);
    if (rec == 0)
        return;
#ifdef W32X_MUTATE /* the checker's negative check only: the scale a fifth */
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 5.0);
#else
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 4.0);
#endif
    W32_SetTemplate(rec);
    block = BYTES(U32(BYTES(U32(rec, 0x332)), 0x8c));
    W32_F(block, 0x50) = (float)Sim_10084a94(dx, dy, dz);
    W32_F(block, 0x44) = x;
    W32_F(block, 0x48) = y;
    W32_F(block, 0x4c) = z;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    U32(rec, 0x34e) = 0;
    U32(rec, 0x352) = 0;
    U32(rec, 0x356) = 0;
    Sim_100aa39f(W32T_NODE(rec));
    Sim_100a9d47(W32T_NODE(rec), W32t_Bits(x), W32t_Bits(y), W32t_Bits(z));
    Sim_100aa799(W32T_NODE(rec));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    n = strength * 0x50;
    radius = (float)(n + Sim_1009c929(strength * 0x50));
    U32(rec, 0x33e) = G32(0x1015dc78);
    U32(rec, 0x342) = (uint32_t)(strength * 0xb5);
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)sx);
        ay = (float)((double)ay + (double)sy);
        az = (float)((double)az + (double)sz);
        W32_F(p, 0) = (float)((double)Sim_1009c929(10) + (double)ax - 5.0);
        W32_F(p, 4) = (float)((double)Sim_1009c929(0x1e) + (double)ay - 15.0);
        W32_F(p, 8) = (float)((double)Sim_1009c929(10) + (double)az - 5.0);
        angle = (float)Sim_1009c929(0x168);
        W32_F(p, 0xc) = (float)((double)Sim_10084766((float)((double)angle + 90.0)) * (double)radius);
        U32(p, 0x10) = 0;
        W32_F(p, 0x14) = (float)((double)Sim_10084766(angle) * (double)radius);
    }
    U32(rec, 0xc) = 1;
}

/* the sparks' updates (2D mode, by their kind): the dust's, colour from a table by age, size 1 */
static int W32x_SparkUpdate(uint8_t *rec, uint32_t table)
{
    int32_t age = (int32_t)(G32(0x1015dc78) - U32(rec, 0x33e)), r, k;

#ifdef W32X_MUTATE /* the checker's negative check only: one frame longer */
    if (age <= 0x5b && (U16(rec, 0x10) & 2) == 0) {
#else
    if (age <= 0x5a && (U16(rec, 0x10) & 2) == 0) {
#endif
        r = Sim_1009c929(10);
        Sim_100a9d7c(W32T_NODE(rec), W32_F(rec, 0x34e), (float)((double)r + (double)W32_F(rec, 0x352)),
                     W32_F(rec, 0x356));
        Sim_100aa799(W32T_NODE(rec));
        k = Render_Ftol((double)age / 90.0 * 3.0);
        U32(rec, 0x346) = *GBYTES(table + (uint32_t)k);
        U32(rec, 0x34a) = 1;
        return 1;
    }
    Sim_100aa351(W32T_NODE(rec));
    U16(rec, 0x10) = (uint16_t)(U16(rec, 0x10) & 0xfff8);
    return 0;
}

// FUNCTION: HGSIM12 0x1007c996
int __cdecl Sim12_1007c996(uint8_t *rec)
{
    return W32x_SparkUpdate(rec, 0x1036f050u);
}

// FUNCTION: HGSIM12 0x1007ca8b
int __cdecl Sim12_1007ca8b(uint8_t *rec)
{
    return W32x_SparkUpdate(rec, 0x10215b18u);
}

/* 1.2 0x1007cb80 and 0x1007cc7c are byte for byte the dust's update 0x10084a87, 0x1007cd78 its draw 0x10084b83 */
// FUNCTION: HGSIM12 0x1007cb80
int __cdecl Sim12_1007cb80(uint8_t *rec)
{
    return Sim12_10084a87(rec);
}

// FUNCTION: HGSIM12 0x1007cc7c
int __cdecl Sim12_1007cc7c(uint8_t *rec)
{
    return Sim12_10084a87(rec);
}

// FUNCTION: HGSIM12 0x1007cd78
int __cdecl Sim12_1007cd78(uint8_t *rec)
{
    return Sim12_10084b83(rec);
}

// FUNCTION: HGSIM12 0x1007d342
void __cdecl Sim12_1007d342(float x, float y, float z, float dx, float dy, float dz, int32_t kind, int32_t ground)
{
    uint8_t *rec, *block, *p;
    float dist, ax = 0.0f, ay = 0.0f, az = 0.0f, sx = 0.0f, sy = 0.0f, sz = 0.0f;
    int32_t type = 0xd, i;
    int(__cdecl *update)(uint8_t *);

    dist = W32_InSight(x, y, z);
    if (!(dist < 0.0f || dist > 0.0f))               /* fcomp 0.0 / test ah, 0x40 */
        return;
    if (G32(0x1017b940) == 0) {
        if (kind == 0)
            update = Sim12_1007c996;
        else if (kind == 1)
            update = Sim12_1007ca8b;
        else if (kind == 2)
            update = Sim12_1007cb80;
        else
            update = 0;
    } else {
        update = Sim12_1007cc7c;
        if (ground >= 0xf && ground <= 0x1b)
            type = 0xe;
        else if (ground == 0x25 || ground == 0x2d)
            type = 0x10;
        else if ((ground >= 0x21 && ground <= 0x23) || ground == 0x28)
            type = 0xf;
        else
            type = 0xd;
    }
    if (update == 0)
        return;
    rec = W32_NewSystem((uint32_t)(uintptr_t)update, (uint32_t)(uintptr_t)Sim12_1007cd78, 0, 0, type);
    if (rec == 0)
        return;
#ifdef W32X_MUTATE /* the checker's negative check only: the scale a fifth */
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 5.0);
#else
    W32_F(rec, 0x3fe) = (float)((double)W32_F(GBYTES(0x1015eef4), 0) / (double)dist / 4.0);
#endif
    W32_SetTemplate(rec);
    block = BYTES(U32(BYTES(U32(rec, 0x332)), 0x8c));
    W32_F(block, 0x50) = (float)Sim_10084a94(dx, dy, dz);
    W32_F(block, 0x44) = x;
    W32_F(block, 0x48) = y;
    W32_F(block, 0x4c) = z;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    W32_F(rec, 0x34e) = (float)(-(double)dx / 5.0);
    U32(rec, 0x352) = 0xc1200000u;                  /* -10.0 */
    W32_F(rec, 0x356) = (float)(-(double)dz / 5.0);
    Sim_100aa39f(W32T_NODE(rec));
    Sim_100a9d47(W32T_NODE(rec), W32t_Bits(x), W32t_Bits(y), W32t_Bits(z));
    Sim_100aa799(W32T_NODE(rec));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(rec, 0x33e) = G32(0x1015dc78);
    for (i = 0; i < 8; i++) {
        p = rec + 0x12 + i * 0x32;
        U16(p, 0x18) = 1;
        ax = (float)((double)ax + (double)sx);
        ay = (float)((double)ay + (double)sy);
        az = (float)((double)az + (double)sz);
        W32_F(p, 0) = (float)((double)Sim_1009c929(10) + (double)ax - 5.0);
        W32_F(p, 4) = (float)((double)Sim_1009c929(10) + (double)ay - 5.0);
        W32_F(p, 8) = (float)((double)Sim_1009c929(10) + (double)az - 5.0);
        W32_F(p, 0xc) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x34e) - 20.0);
        W32_F(p, 0x10) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x352) - 20.0);
        W32_F(p, 0x14) = (float)((double)Sim_1009c929(0x28) + (double)W32_F(rec, 0x356) - 20.0);
    }
    U32(rec, 0x402) = (uint32_t)type;
    U32(rec, 0xc) = 1;
}
