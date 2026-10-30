#include "w5z.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../win/w4a.h"
#include "../view/w2l.h"
#include "../unsorted_100a/w5z.h"

typedef int(__stdcall *HeapFreeFn)(void *heap, uint32_t flags, void *block);
typedef void(__cdecl *Fn1002feef)(void *obj);
typedef int(__cdecl *Fn1000a6f0)(uint32_t a, uint32_t b);
typedef uint32_t(__cdecl *Fn1000ea40)(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint16_t a5, uint32_t a6);

#define ID_TABLE SIM_PTR(int32_t, 0x1012da40)

// FUNCTION: HGSIM 0x10006d75
void __cdecl Sim_10006d75(void)
{
    int i;

    for (i = 0; ID_TABLE[i] != -1; i++)
        Sim_100a7758(ID_TABLE[i], (const void *)(uintptr_t)G32(0x1014ceb8));
}

// FUNCTION: HGSIM 0x10009369
void __cdecl Sim_10009369(void *obj)
{
    uint32_t block = U32(obj, 0x2c);

    while (block != 0) {
        uint32_t cur = block;

        block = U32(cur, 0xc);
        Heap_Free(BYTES(cur));
    }
    SIM_FN(Fn1002feef, 0x1002feef)(obj);
    SIM_API(HeapFreeFn, 0x10ac5634)((void *)(uintptr_t)G32(0x1017b8f8), 1, obj);
}

// FUNCTION: HGSIM 0x10009741
void __cdecl Sim_10009741(void *obj)
{
    if (obj != 0) {
        Sim_1008a16d(obj);
        Sim_10009369(obj);
    }
}

// FUNCTION: HGSIM 0x1000c5f8
int __cdecl Sim_1000c5f8(uint32_t a)
{
    return SIM_FN(Fn1000a6f0, 0x1000a6f0)(a, 0) == 0;
}

// FUNCTION: HGSIM 0x1000ed25
uint32_t __cdecl Sim_1000ed25(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint16_t a5, uint32_t a6)
{
    return SIM_FN(Fn1000ea40, 0x1000ea40)(a0, a1, a2, a3, a4, a5, a6);
}
