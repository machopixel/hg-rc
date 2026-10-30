#include "w2g.h"
#include "../simmem.h"
#include "../unsorted_1003/w2g.h"
#include "../unsorted_100a/w2g.h"
#include "../unsorted_100d/w2g.h"

#define U16(p, off) (*(uint16_t *)((uint8_t *)(p) + (off)))
#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))
#define G32(addr) (*SIM_PTR(uint32_t, addr))
#define ADDR(a) ((uint32_t)(uintptr_t)SIM_PTR(void, a))
#define LAST G32(0x1017CC2C)
#define FIRST G32(0x1017CC28)
#define TOP (*SIM_PTR(int32_t, 0x10215AE8))

// FUNCTION: HGSIM 0x100903e0
void __cdecl Sim_100903e0(uint8_t *obj)
{
    if (obj == 0)
        return;
    if (FIRST == 0 && LAST == 0) {
        U32(obj, 0) = 0;
        U32(obj, 4) = 0;
        FIRST = (uint32_t)(uintptr_t)obj;
        LAST = (uint32_t)(uintptr_t)obj;
        return;
    }
    U32(obj, 0) = LAST;
    U32(obj, 4) = 0;
    U32(PTR(LAST), 4) = (uint32_t)(uintptr_t)obj;
    LAST = (uint32_t)(uintptr_t)obj;
}

// FUNCTION: HGSIM 0x10090581
uint8_t *__cdecl Sim_10090581(int force)
{
    uint8_t *obj = 0;

    if (TOP >= 0) {
        obj = SIM_PTR(uint8_t, 0x10217C20u + SIM_PTR(uint32_t, 0x10216B80)[TOP] * 0x57Eu);
        if (G32(0x1014B9B8) != 0 || force != 0) {
            Sim_100903e0(obj);
            TOP = TOP - 1;
        } else {
            obj = 0;
        }
    }
    return obj;
}

// FUNCTION: HGSIM 0x10090c7d
uint8_t *__cdecl Sim_10090c7d(uint32_t a, uint32_t b, uint32_t c, int force)
{
    uint8_t *obj = Sim_10090581(force);

    if (obj != 0) {
        U32(obj, 0x556) = a;
        U32(obj, 0x55A) = b;
        U32(obj, 0x57A) = c;
    }
    return obj;
}

// FUNCTION: HGSIM 0x10091071
int __cdecl Sim_10091071(int value)
{
    return value != 0;
}

/* The part both constructors share once the object exists. */
static void InitObject(uint8_t *obj)
{
    uint8_t *block = PTR(U32(PTR(U32(obj, 0x552)), 0x8C));

    U32(block, 0x50) = 0x3F800000u;
    U32(block, 0x44) = 0x3F800000u;
    U32(block, 0x48) = 0x3F800000u;
    U32(block, 0x4C) = 0x3F800000u;
    U32(block, 0x38) = 0;
    U32(block, 0x3C) = 0;
    U32(block, 0x40) = 0;
    G32(0x1017CC24) = G32(0x1017CC24) + 1;
    U32(obj, 0x55E) = G32(0x1015DC78);
    U16(obj, 0x2A) = 1;
    U32(obj, 0xC) = 0xD;
}

// FUNCTION: HGSIM 0x10091085
void __cdecl Sim_10091085(uint32_t arg, int unused, uint32_t param)
{
    uint8_t *obj;

    (void)unused;
    if ((int32_t)(G32(0x1017CC24) + 1) >= (int32_t)G32(0x1017CC20))
        return;
    obj = Sim_10090c7d(ADDR(0x10001433), ADDR(0x10002E78), arg, 1);
    if (obj == 0)
        return;
    U16(obj, 0x10) = 1;
    U32(obj, 0x562) = param;
    U32(obj, 0x566) = 0;
    InitObject(obj);
}

// FUNCTION: HGSIM 0x1009201f
void __cdecl Sim_1009201f(uint32_t arg, int unused, uint32_t param, uint32_t param2)
{
    uint8_t *obj;

    (void)unused;
    if ((int32_t)(G32(0x1017CC24) + 1) >= (int32_t)G32(0x1017CC20))
        return;
    obj = Sim_10090c7d(ADDR(0x10003049), ADDR(0x10002077), arg, 1);
    if (obj == 0)
        return;
    U16(obj, 0x10) = 1;
    U32(obj, 0x562) = param;
    U32(obj, 0x566) = param2;
    InitObject(obj);
}

// FUNCTION: HGSIM 0x100987fa
void __cdecl Sim_100987fa(int unused, uint32_t value)
{
    (void)unused;
    U32(PTR(G32(0x1015D1B8)), 0x64) = value;
}

// FUNCTION: HGSIM 0x10098881
int __cdecl Sim_10098881(int unused)
{
    (void)unused;
    /* through the original entry: hooks_view.c forces the option on there */
    return SIM_FN(int(__cdecl *)(int), 0x100ac603)(-1);
}

// FUNCTION: HGSIM 0x10098896
void __cdecl Sim_10098896(int unused, int value)
{
    (void)unused;
    SIM_FN(int(__cdecl *)(int), 0x100ac603)(value);
    U32(PTR(G32(0x1015D1B8)), 0x30) = (uint32_t)value;
}

// FUNCTION: HGSIM 0x10098a18
void __cdecl Sim_10098a18(int a, int b)
{
    Sim_100d2c86(a, b);
    Sim_100332e9(a, b);
    U32(PTR(G32(0x1015D1B8)), 0x1C) = (uint32_t)b;
}

// FUNCTION: HGSIM 0x10098e62
int __cdecl Sim_10098e62(void)
{
    return -1;
}

// FUNCTION: HGSIM 0x10098e74
int __cdecl Sim_10098e74(void)
{
    return 1;
}

// FUNCTION: HGSIM 0x1009b2b4
void __cdecl Sim_1009b2b4(void)
{
    if ((int32_t)G32(0x1015D1AC) <= 2 && (int32_t)G32(0x1015D1CC) <= 2) {
        G32(0x1015D1CC) = G32(0x1015D1AC) + 1;
        G32(0x1015D1CC) = (uint32_t)((int32_t)G32(0x1015D1CC) % 3);
    }
}

// FUNCTION: HGSIM 0x1009c710
void __cdecl Sim_1009c710(void)
{
    static const uint32_t blocks[3] = {0x1014C31Cu, 0x1014C3C4u, 0x1014C46Cu};
    int i;

    for (i = 0; i < 3; i++) {
        uint32_t *b = SIM_PTR(uint32_t, blocks[i]);

        b[0] = G32(0x101C528C);
        b[1] = ADDR(0x101D02C0);
        b[2] = G32(0x101C4BCC);
        b[3] = ADDR(0x101D02A0);
        b[4] = G32(0x101C4F78);
        b[5] = ADDR(0x101D0340);
        b[7] = G32(0x101C45F4);
        b[8] = G32(0x101C4B90);
    }
}

// FUNCTION: HGSIM 0x1009e5ac
void __cdecl Sim_1009e5ac(uint32_t value)
{
    G32(0x1017D064) = value;
}
