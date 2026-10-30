#include "w3q.h"
#include "../util/access.h"

typedef void(__cdecl *MethodFn)(void *self, uint32_t value);

/* Calls the method at +0x11c of the object whose pointer is stored at objAddr. */
static void CallMethod11c(uint32_t objAddr, uint32_t valueAddr)
{
    uint32_t value = G32(valueAddr);
    uint32_t obj = G32(objAddr);

    (*(MethodFn *)(BYTES(G32(objAddr)) + 0x11c))((void *)(uintptr_t)obj, value);
}

// FUNCTION: HGSIM 0x100781e0
void __cdecl Sim_100781e0(void)
{
    CallMethod11c(0x109e8928, 0x1096fc08);
    CallMethod11c(0x109e8938, 0x1096fc0c);
    CallMethod11c(0x109e890c, 0x1096fc04);
    CallMethod11c(0x109e88e8, 0x1096fc00);
    CallMethod11c(0x109e894c, 0x1096fc10);
    CallMethod11c(0x109e8950, 0x1096fc14);
    CallMethod11c(0x109e8954, 0x1096fc18);
}

// FUNCTION: HGSIM 0x100797f2
void __cdecl Sim_100797f2(void)
{
    int i;

    for (i = 0; i < GS32(0x104737e0); i++) {
        if (SIM_PTR(u32a, 0x1017ee40)[i] != 0) {
            volatile int j;

            for (j = 0; j < 0x400; j++)
                ;
        }
    }
}
