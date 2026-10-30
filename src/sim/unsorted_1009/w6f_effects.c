#include "w6f.h"
#include "w2g.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../unsorted_1000/w2e.h"
#include "../unsorted_1000/w5x.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"

#define ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, a))
#define NODE(obj) ((void *)(uintptr_t)U32(obj, 0x552))
#define CAMERA BYTES(G32(0x1014cae0))
#define AGE(obj) ((int32_t)(G32(0x1015dc78) - U32(obj, 0x55e)))

/* The end of every update: the effect is no longer alive. */
static int32_t Effect_Expire(uint8_t *obj)
{
    U16(obj, 0x10) = (uint16_t)(U16(obj, 0x10) & 0xfffe);
    Sim_100aa351(NODE(obj));
    return 0;
}

// FUNCTION: HGSIM 0x1009117d
int32_t __cdecl Sim_1009117d(uint8_t *obj)
{
    uint32_t x, y, z;

    if (AGE(obj) > S32(obj, 0x562))
        return Effect_Expire(obj);
    Sim_100a9cfb((void *)(uintptr_t)U32(obj, 0x57a), &x, &y, &z);
    Sim_100a9d47(NODE(obj), x, y, z);
    Sim_100aa799(NODE(obj));
    return 1;
}

/* Copies the 6 dwords of the record +0x57a to +0x12 (unaligned fields). */
static void CopyRecord(uint8_t *obj)
{
    uint32_t rec = U32(obj, 0x57a);
    int i;

    for (i = 0; i < 6; i++)
        U32(obj, 0x12 + 4 * i) = U32(rec, 4 * i);
}

// FUNCTION: HGSIM 0x100917ed
int32_t __cdecl Sim_100917ed(uint8_t *obj)
{
    if (AGE(obj) > S32(obj, 0x562))
        return Effect_Expire(obj);
    CopyRecord(obj);
    Sim_100aa39f(NODE(obj));
    Sim_100a9d47(NODE(obj), U32(obj, 0x12), U32(obj, 0x16), U32(obj, 0x1a));
    Sim_100aa799(NODE(obj));
    return 1;
}

// FUNCTION: HGSIM 0x10092117
int32_t __cdecl Sim_10092117(uint8_t *obj)
{
    if (AGE(obj) > S32(obj, 0x562)) {
        Sim_1000ecb0(SIM_PTR(const char, 0x1014b9c4));         /* "AFXWLLLC" */
        return Effect_Expire(obj);
    }
    CopyRecord(obj);
    Sim_100aa39f(NODE(obj));
    Sim_100a9d47(NODE(obj), U32(obj, 0x1e), U32(obj, 0x22), U32(obj, 0x26));
    Sim_100aa799(NODE(obj));
    return 1;
}

/* Moves the node by (+0x56e, +0x572 + random 0..9, +0x576); +0x576 is read before the random number. */
static void Effect_Drift(uint8_t *obj)
{
    float dz = FL(obj, 0x576);
    int32_t r = Sim_1009c929(10);

    Sim_100a9d7c(NODE(obj), FL(obj, 0x56e), (float)((double)r + (double)FL(obj, 0x572)), dz);
    Sim_100aa799(NODE(obj));
}

/* trunc(age / div * mul) as the runtime's _ftol does it. */
static int32_t Effect_Step(int32_t age, uint32_t divAddr, uint32_t mulAddr)
{
    return Render_Ftol((double)age / (double)GFL(divAddr) * (double)GFL(mulAddr));
}

// FUNCTION: HGSIM 0x10093295
int32_t __cdecl Sim_10093295(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i;

    if (age > 0x5a)
        return Effect_Expire(obj);
    Effect_Drift(obj);
    i = Effect_Step(age, 0x1012b940, 0x1012b94c);      /* 90, 3 */
    U32(obj, 0x566) = *(SIM_PTR(uint8_t, 0x1036f050) + i);
    U32(obj, 0x56a) = 1;
    return 1;
}

// FUNCTION: HGSIM 0x10093376
int32_t __cdecl Sim_10093376(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i;

    if (age > 0x5a)
        return Effect_Expire(obj);
    Effect_Drift(obj);
    i = Effect_Step(age, 0x1012b940, 0x1012b94c);      /* 90, 3 */
    U32(obj, 0x566) = *(SIM_PTR(uint8_t, 0x10215b18) + i);
    U32(obj, 0x56a) = 1;
    return 1;
}

// FUNCTION: HGSIM 0x10093457
int32_t __cdecl Sim_10093457(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i;

    if (age > 0x5a)
        return Effect_Expire(obj);
    Effect_Drift(obj);
    i = Effect_Step(age, 0x1012b940, 0x1012b94c);      /* 90, 3 */
    U32(obj, 0x566) = *(SIM_PTR(uint8_t, 0x10215bc0) + i);
    U32(obj, 0x56a) = *(SIM_PTR(uint8_t, 0x10215bb0) + i);
    return 1;
}

// FUNCTION: HGSIM 0x1009589c
int32_t __cdecl Sim_1009589c(uint8_t *obj)
{
    int32_t age = AGE(obj);

    if (age > 0x16)
        return Effect_Expire(obj);
    Effect_Drift(obj);
    U32(obj, 0x566) = *(SIM_PTR(uint8_t, 0x101d09c0) + Effect_Step(age, 0x1012b97c, 0x1012b954));     /* 22, 15 */
    return 1;
}

// FUNCTION: HGSIM 0x10096095
int32_t __cdecl Sim_10096095(uint8_t *obj)
{
    int32_t age = AGE(obj);

    if (age > 0x2d)
        return Effect_Expire(obj);
    Effect_Drift(obj);
    U32(obj, 0x566) = *(SIM_PTR(uint8_t, 0x10215b10) + Effect_Step(age, 0x1012b988, 0x1012b98c));     /* 45, 7 */
    return 1;
}

// FUNCTION: HGSIM 0x100967cc
int32_t __cdecl Sim_100967cc(uint8_t *obj)
{
    if (AGE(obj) > 0x2d)
        return Effect_Expire(obj);
    {
        float dz = FL(obj, 0x576);
        float dy = FL(obj, 0x572);

        Sim_100a9d7c(NODE(obj), FL(obj, 0x56e), dy, dz);
    }
    Sim_100aa799(NODE(obj));
    U32(obj, 0x566) = 0x1f;
    return 1;
}

/* The block of the node: position 1.0, 1.0, 1.0, size 1.0 (+0x44.. +0x50) and +0x38.. +0x40 cleared. */
static void Effect_ResetBlock(uint8_t *obj)
{
    uint8_t *block = BYTES(U32(U32(obj, 0x552), 0x8c));

    U32(block, 0x50) = 0x3f800000u;
    U32(block, 0x44) = 0x3f800000u;
    U32(block, 0x48) = 0x3f800000u;
    U32(block, 0x4c) = 0x3f800000u;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
}

// FUNCTION: HGSIM 0x10092956
void __cdecl Sim_10092956(uint32_t arg, const uint8_t *rec, uint32_t param)
{
    uint8_t *obj;

    if ((int32_t)(G32(0x1017cc24) + 1) >= GS32(0x1017cc20))
        return;
    obj = Sim_10090c7d(ADDR(0x10002707), ADDR(0x1000158c), arg, 1);
    if (obj == 0)
        return;
    U16(obj, 0x10) = 1;
    U32(obj, 0x562) = U32(rec, 0x34) - U32(rec, 0x30);
    U32(obj, 0x566) = param;
    Effect_ResetBlock(obj);
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(obj, 0x55e) = G32(0x1015dc78);
    U16(obj, 0x12 + 0x18) = 1;
    Sim_1000ec60(SIM_PTR(const char, 0x1014b9d0), 0xfffffc18u, 0, 5, 0x150);     /* "AFXWLLLC" */
    U32(obj, 0xc) = 0xd;
}

/*
 * The start of every particle constructor: returns the new effect object, or null when the count is
 * exhausted, the point is out of the camera's range or the pool is empty.
 */
static uint8_t *Effect_Start(float x, float y, float z, uint32_t a, uint32_t b)
{
    float dx, dy, dz;
    uint8_t *obj;

    if ((int32_t)(G32(0x1017cc24) + 1) >= GS32(0x1017cc20))
        return 0;
    dx = (float)((double)x - (double)FL(CAMERA, 0));
    dy = (float)((double)y - (double)FL(CAMERA, 4));
    dz = (float)((double)z - (double)FL(CAMERA, 8));
    if (Sim_10084a94(dx, dy, dz) > (double)FL(CAMERA, 0x40))
        return 0;
    if (a == 0)
        return 0;
    obj = Sim_10090c7d(a, b, 0, 0);
    return obj;
}

/* Node block: size = length of the speed, position (x, y, z), +0x38.. +0x40 cleared. */
static void Effect_SetBlock(uint8_t *obj, float x, float y, float z, float sx, float sy, float sz)
{
    uint8_t *block = BYTES(U32(U32(obj, 0x552), 0x8c));

    U16(obj, 0x10) = 1;
    FL(block, 0x50) = (float)Sim_10084a94(sx, sy, sz);
    U32(block, 0x44) = Bits_OfFloat(x);
    U32(block, 0x48) = Bits_OfFloat(y);
    U32(block, 0x4c) = Bits_OfFloat(z);
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
}

/* Places the node at (x, y, z) and counts the effect. */
static void Effect_Place(uint8_t *obj, float x, float y, float z)
{
    Sim_100aa39f(NODE(obj));
    Sim_100a9d47(NODE(obj), Bits_OfFloat(x), Bits_OfFloat(y), Bits_OfFloat(z));
    Sim_100aa799(NODE(obj));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    U32(obj, 0x55e) = G32(0x1015dc78);
}

#define PARTICLE(obj, i) (BYTES(obj) + (uint32_t)(i) * 0x2au + 0x12u)

/* (float)(random 0..m-1 + base - sub), the sum in double like the x87 code. */
static float Scatter(int32_t m, float base, uint32_t subAddr)
{
    int32_t r = Sim_1009c929(m);

    return (float)((double)r + (double)base - (double)GFL(subAddr));
}

// FUNCTION: HGSIM 0x100939a3
void __cdecl Sim_100939a3(float x, float y, float z, float sx, float sy, float sz, int32_t type)
{
    static const uint32_t first[3] = {0x10001f50, 0x100014d3, 0x10002482};
    float step[3], acc[3];
    uint8_t *obj;
    int i, k;

    obj = Effect_Start(x, y, z, (type >= 0 && type <= 2) ? ADDR(first[type]) : 0, ADDR(0x100026df));
    if (obj == 0)
        return;
    Effect_SetBlock(obj, x, y, z, sx, sy, sz);
    FL(obj, 0x56e) = (float)(-(double)sx / (double)GFL(0x1012b948));      /* 5 */
    U32(obj, 0x572) = 0xc1200000u;                                          /* -10.0f */
    FL(obj, 0x576) = (float)(-(double)sz / (double)GFL(0x1012b948));
    for (k = 0; k < 3; k++) {
        step[k] = 0.0f;
        acc[k] = step[k];
    }
    Effect_Place(obj, x, y, z);
    for (i = 0; i < 16; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U16(p, 0x18) = 1;
        for (k = 0; k < 3; k++)
            acc[k] = (float)((double)acc[k] + (double)step[k]);
        FL(p, 0) = Scatter(10, acc[0], 0x1012b948);         /* - 5 */
        FL(p, 4) = Scatter(10, acc[1], 0x1012b948);
        FL(p, 8) = Scatter(10, acc[2], 0x1012b948);
        FL(p, 0xc) = Scatter(40, FL(obj, 0x56e), 0x1012b95c);      /* - 20 */
        FL(p, 0x10) = Scatter(40, FL(obj, 0x572), 0x1012b95c);
        FL(p, 0x14) = Scatter(40, FL(obj, 0x576), 0x1012b95c);
    }
    U32(obj, 0xc) = 1;
}

/*
 * The common middle of the other particle constructors: node block, speed = speed / speedDiv (at the
 * addresses of the float divisors), steps = speed / 16, node placed and the effect counted.
 */
static void Effect_Setup(uint8_t *obj, float x, float y, float z, float sx, float sy, float sz,
                         uint32_t speedDiv, float step[3])
{
    Effect_SetBlock(obj, x, y, z, sx, sy, sz);
    FL(obj, 0x56e) = (float)((double)sx / (double)GFL(speedDiv));
    FL(obj, 0x572) = (float)((double)sy / (double)GFL(speedDiv));
    FL(obj, 0x576) = (float)((double)sz / (double)GFL(speedDiv));
    step[0] = (float)((double)sx / (double)GFL(0x1012b968));      /* 16 */
    step[1] = (float)((double)sy / (double)GFL(0x1012b968));
    step[2] = (float)((double)sz / (double)GFL(0x1012b968));
    Effect_Place(obj, x, y, z);
}

/* The steps of particle i: acc += step (each sum rounded to float). */
static void Effect_Advance(float acc[3], const float step[3])
{
    int k;

    for (k = 0; k < 3; k++)
        acc[k] = (float)((double)acc[k] + (double)step[k]);
}

// FUNCTION: HGSIM 0x100941c3
void __cdecl Sim_100941c3(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;
    int i;

    obj = Effect_Start(x, y, z, ADDR(0x1000139d), ADDR(0x1000107d));
    if (obj == 0)
        return;
    Effect_Setup(obj, x, y, z, sx, sy, sz, 0x1012b964, step);      /* 256 */
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    for (i = 0; i < 16; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U16(p, 0x18) = 1;
        Effect_Advance(acc, step);
        FL(p, 0) = acc[0];
        FL(p, 4) = acc[1];
        FL(p, 8) = acc[2];
        U32(p, 0x1a) = ((uint32_t)Sim_1009c929(2) << 6) + ADDR(0x10215b20);
    }
    U32(obj, 0xc) = 1;
}

// FUNCTION: HGSIM 0x100949e8
void __cdecl Sim_100949e8(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;
    int i;

    obj = Effect_Start(x, y, z, ADDR(0x10001677), ADDR(0x1000394f));
    if (obj == 0)
        return;
    Effect_Setup(obj, x, y, z, sx, sy, sz, 0x1012b964, step);      /* 256 */
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    for (i = 0; i < 16; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U16(p, 0x18) = 1;
        Effect_Advance(acc, step);
        FL(p, 0) = acc[0];
        FL(p, 4) = acc[1];
        FL(p, 8) = acc[2];
    }
    U32(obj, 0xc) = 1;
}

// FUNCTION: HGSIM 0x10094fcb
void __cdecl Sim_10094fcb(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;
    int i, k;

    obj = Effect_Start(x, y, z, ADDR(0x10001816), ADDR(0x1000257c));
    if (obj == 0)
        return;
    Effect_Setup(obj, x, y, z, sx, sy, sz, 0x1012b964, step);      /* 256 */
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    for (i = 0; i < 16; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U16(p, 0x18) = 1;
        Effect_Advance(acc, step);
        for (k = 0; k < 3; k++)
            FL(p, 4 * k) = (float)((double)(Sim_1009c929(0x32) - 0x19) + (double)acc[k]);
    }
    U32(obj, 0xc) = 1;
}

/* Particles scattered by random 0..xm-1 - xsub, 0..49 - 25, 0..xm-1 - xsub around the steps. */
static void Effect_Spray(uint8_t *obj, const float step[3], float acc[3], int32_t xm, uint32_t xsub)
{
    int i;

    for (i = 0; i < 16; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U16(p, 0x18) = 1;
        Effect_Advance(acc, step);
        FL(p, 0) = Scatter(xm, acc[0], xsub);
        FL(p, 4) = Scatter(0x32, acc[1], 0x1012b978);      /* - 25 */
        FL(p, 8) = Scatter(xm, acc[2], xsub);
    }
}

// FUNCTION: HGSIM 0x10095616
void __cdecl Sim_10095616(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;

    obj = Effect_Start(x, y, z, ADDR(0x10002f0e), ADDR(0x100011ef));
    if (obj == 0)
        return;
    Effect_SetBlock(obj, x, y, z, sx, sy, sz);
    FL(obj, 0x56e) = (float)((double)sx / (double)GFL(0x1012b968));      /* 16 */
    U32(obj, 0x572) = 0x40a00000u;                                        /* 5.0f */
    FL(obj, 0x576) = (float)((double)sz / (double)GFL(0x1012b968));
    step[0] = (float)((double)sx / (double)GFL(0x1012b968));
    step[1] = (float)((double)sy / (double)GFL(0x1012b968));
    step[2] = (float)((double)sz / (double)GFL(0x1012b968));
    Effect_Place(obj, x, y, z);
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    Effect_Spray(obj, step, acc, 10, 0x1012b948);                     /* - 5 */
    U32(obj, 0xc) = 1;
}

// FUNCTION: HGSIM 0x10095e0a
void __cdecl Sim_10095e0a(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;

    obj = Effect_Start(x, y, z, ADDR(0x10002ce8), ADDR(0x10002801));
    if (obj == 0)
        return;
    Effect_Setup(obj, x, y, z, sx, sy, sz, 0x1012b984, step);      /* 8 */
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    Effect_Spray(obj, step, acc, 10, 0x1012b948);                 /* - 5 */
    U32(obj, 0xc) = 1;
}

// FUNCTION: HGSIM 0x10096541
void __cdecl Sim_10096541(float x, float y, float z, float sx, float sy, float sz)
{
    float step[3], acc[3];
    uint8_t *obj;

    obj = Effect_Start(x, y, z, ADDR(0x1000236a), ADDR(0x100030da));
    if (obj == 0)
        return;
    Effect_Setup(obj, x, y, z, sx, sy, sz, 0x1012b950, step);      /* 2 */
    acc[0] = step[0];
    acc[1] = step[1];
    acc[2] = step[2];
    Effect_Spray(obj, step, acc, 4, 0x1012b950);                  /* - 2 */
    U32(obj, 0xc) = 2;
}
