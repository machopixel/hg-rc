#include "w7o.h"
#include "w2g.h"
#include "../util/access.h"
#include "../util/w3o.h"
#include "../util/util.h"
#include "../unsorted_1008/w3o.h"
#include "../unsorted_100a/w3t.h"
#include "../unsorted_100a/w5b.h"
#include "../unsorted_100a/w5x.h"
#include "../unsorted_100a/w5y.h"

#define ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(uint8_t, a))
#define NODE(obj) ((void *)(uintptr_t)U32(obj, 0x552))
#define CAMERA BYTES(G32(0x1014cae0))
#define AGE(obj) ((int32_t)(G32(0x1015dc78) - U32(obj, 0x55e)))
#define PARTICLE(obj, i) (BYTES(obj) + 0x12 + (i) * 0x2a)

/* The end of every update: the effect is no longer alive. */
static int32_t Expire(uint8_t *obj)
{
    U16(obj, 0x10) = (uint16_t)(U16(obj, 0x10) & 0xfffe);
    Sim_100aa351(NODE(obj));
    return 0;
}

/* Moves the node by (+0x56e, +0x572 + random 0..9, +0x576); +0x576 is read before the random number. */
static void Drift(uint8_t *obj)
{
    float dz = FL(obj, 0x576);
    int32_t r = Sim_1009c929(10);

    Sim_100a9d7c(NODE(obj), FL(obj, 0x56e), (float)((double)r + (double)FL(obj, 0x572)), dz);
    Sim_100aa799(NODE(obj));
}

/* trunc(age / div * [0x1012b954]) as the runtime's _ftol does it. */
static int32_t Step(int32_t age, uint32_t divAddr)
{
    return Render_Ftol((double)age / (double)GFL(divAddr) * (double)GFL(0x1012b954));
}

/* f = (float)(r + f) */
static void AddInt(uint8_t *p, int32_t r)
{
    FL(p, 0) = (float)((double)r + (double)FL(p, 0));
}

// FUNCTION: HGSIM 0x10090f71
int32_t __cdecl Sim_10090f71(uint8_t *obj)
{
    uint32_t src = U32(obj, 0x57a);

    if (AGE(obj) > S32(obj, 0x562)) {
        W7O_CALL_100246b3(-2, 0, 0x49, U32(obj, 0x12), U32(obj, 0x16), U32(obj, 0x1a), 0, 0, 0, 0, 0, 0);
        Sim_100aa351(NODE(obj));
        U16(obj, 0x10) = (uint16_t)(U16(obj, 0x10) & 0xfffe);
        return 0;
    }
    U32(obj, 0x12) = U32(src, 0xc);
    U32(obj, 0x16) = U32(src, 0x10);
    U32(obj, 0x1a) = U32(src, 0x14);
    Sim_100aa39f(NODE(obj));
    Sim_100a9d47(NODE(obj), U32(obj, 0x12), U32(obj, 0x22), U32(obj, 0x26));
    Sim_100aa799(NODE(obj));
    return 1;
}

// FUNCTION: HGSIM 0x10091597
void __cdecl Sim_10091597(void *src, int32_t size)
{
    uint32_t x, y, z;
    uint8_t *cam;
    uint8_t *obj;
    uint8_t *block;
    float dx, dy, dz, scale;
    int32_t i;

    if ((int32_t)(G32(0x1017cc24) + 1) >= GS32(0x1017cc20) || size == 0)
        return;
    Sim_100a9cfb(src, &x, &y, &z);
    cam = CAMERA;
    dx = (float)((double)Float_OfBits(x) - (double)FL(cam, 0));
    cam = CAMERA;
    dy = (float)((double)Float_OfBits(y) - (double)FL(cam, 4));
    cam = CAMERA;
    dz = (float)((double)Float_OfBits(z) - (double)FL(cam, 8));
    if (Sim_10084a94(dx, dy, dz) > (double)FL(CAMERA, 0x40))
        return;
    obj = Sim_10090c7d(ADDR(0x100032c9), ADDR(0x1000356c), 0, 0);
    if (obj == 0)
        return;
    U16(obj, 0x10) = 1;
    U32(obj, 0x57a) = (uint32_t)(uintptr_t)src;
    block = BYTES(U32(U32(obj, 0x552), 0x8c));
    U32(block, 0x50) = 0;
    U32(block, 0x44) = 0;
    U32(block, 0x48) = 0;
    U32(block, 0x4c) = 0;
    U32(block, 0x38) = 0;
    U32(block, 0x3c) = 0;
    U32(block, 0x40) = 0;
    U32(obj, 0x56e) = 0;
    U32(obj, 0x572) = 0;
    U32(obj, 0x576) = 0;
    Sim_100aa39f(NODE(obj));
    Sim_100a9d47(NODE(obj), x, y, z);
    Sim_100aa799(NODE(obj));
    G32(0x1017cc24) = G32(0x1017cc24) + 1;
    scale = (float)(int32_t)((uint32_t)size * 0x1eu);
    U32(obj, 0x55e) = G32(0x1015dc78);
    U32(obj, 0x562) = (uint32_t)size * 0xb5u;
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);
        int32_t r;
        float angle;

        U16(p, 0x18) = 1;
        r = Sim_1009c929(0x168);
        angle = (float)r;
        FL(p, 0) = (float)(W7O_CALL_10084766((float)((double)r + (double)GFL(0x1012b940))) * (double)scale);
        U32(p, 4) = 0;
        FL(p, 8) = (float)(W7O_CALL_10084766(angle) * (double)scale);
    }
    U32(obj, 0xc) = 1;
}

// FUNCTION: HGSIM 0x10092a71
int32_t __cdecl Sim_10092a71(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i, k;

    if (age > S32(obj, 0x562))
        return Expire(obj);
    Drift(obj);
    k = Render_Ftol((double)age / (double)S32(obj, 0x562) * (double)GFL(0x1012b954));
    U32(obj, 0x56a) = *(GBYTES(0x10215bb0) + k);
    U32(obj, 0x566) = 0xfe;
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);

        FL(p, 0) = (float)((double)FL(p, 0) + (double)FL(p, 0xc));
        FL(p, 4) = (float)((double)FL(p, 4) + (double)FL(p, 0x10));
        FL(p, 8) = (float)((double)FL(p, 8) + (double)FL(p, 0x14));
    }
    return 1;
}

// FUNCTION: HGSIM 0x10093ce7
int32_t __cdecl Sim_10093ce7(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i, k;

    if (age > 0xb5)
        return Expire(obj);
    Drift(obj);
    k = Step(age, 0x1012b960);
    U32(obj, 0x56a) = *(GBYTES(0x10217b20) + k);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);

        U32(p, 0x1e) = U32(U32(p, 0x1a), (uint32_t)k * 4);
        AddInt(p, Sim_1009c929(0x64) - 0x32);
        AddInt(p + 4, Sim_1009c929(0x14) - 0xa);
        AddInt(p + 8, Sim_1009c929(0x64) - 0x32);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10094423
int32_t __cdecl Sim_10094423(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i, k;

    if (age > 0x21f)
        return Expire(obj);
    Drift(obj);
    k = Step(age, 0x1012b96c);
    U32(obj, 0x566) = *(GBYTES(0x10215bd0) + k);
    U32(obj, 0x56a) = *(GBYTES(0x10217b20) + k);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);

        AddInt(p, Sim_1009c929(0x14) - 0xa);
        AddInt(p + 4, Sim_1009c929(0x14) - 0xa);
        AddInt(p + 8, Sim_1009c929(0x14) - 0xa);
    }
    return 1;
}

// FUNCTION: HGSIM 0x10094c2a
int32_t __cdecl Sim_10094c2a(uint8_t *obj)
{
    int32_t age = AGE(obj);
    int32_t i;

    if (age > 0x16a)
        return Expire(obj);
    Drift(obj);
    U32(obj, 0x566) = *(GBYTES(0x10215af0) + Step(age, 0x1012b974));
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);

        AddInt(p, Sim_1009c929(0x3c) - 0x1e);
        AddInt(p + 4, Sim_1009c929(0x3c) - 0x1e);
        AddInt(p + 8, Sim_1009c929(0x3c) - 0x1e);
    }
    return 1;
}

/* f = (float)(random(6n) - 3n + (float)(f + base)) with n = +0x566 */
static void Scatter(uint8_t *obj, uint8_t *p, uint32_t base)
{
    float t = (float)((double)FL(p, 0) + (double)Float_OfBits(base));
    int32_t r = (int32_t)((uint32_t)Sim_1009c929((int32_t)(U32(obj, 0x566) * 6u)) - U32(obj, 0x566) * 3u);

    FL(p, 0) = (float)((double)r + (double)t);
}

// FUNCTION: HGSIM 0x1009524d
int32_t __cdecl Sim_1009524d(uint8_t *obj)
{
    int32_t age = AGE(obj);
    uint32_t x, y, z;
    int32_t i;

    if (age > 0xb5)
        return Expire(obj);
    Drift(obj);
    U32(obj, 0x566) = *(GBYTES(0x10217b90) + Step(age, 0x1012b960));
    U32(obj, 0x56a) = 1;
    Sim_100a9cfb(NODE(obj), &x, &y, &z);
    for (i = 0; i < 0x10; i++) {
        uint8_t *p = PARTICLE(obj, i);

        Scatter(obj, p, x);
        Scatter(obj, p + 4, y);
        Scatter(obj, p + 8, z);
    }
    return 1;
}
