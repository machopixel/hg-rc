#include "w5x.h"
#include "w2j.h"
#include "../util/access.h"

typedef int(__stdcall *HeapFreeFn)(void *heap, uint32_t flags, void *block);

#define IAT_HEAPFREE 0x10ac5634u

// FUNCTION: HGSIM 0x1002009d
// FUNCTION: HGSIM12 0x100173ad
void __cdecl Sim_1002009d(int index)
{
    /* 1.2 (0x100173ad): banks 0..3 (1.2 has a 4th record, all zero; nothing calls this in 1.2) */
    if (index > -1 && index < 4)
        SIM_CALL_1001f790(GBYTES(0x10133728u + (uint32_t)index * 0x18u), 0, -1);
}

// FUNCTION: HGSIM 0x10027a8f
void __cdecl Sim_10027a8f(void *p)
{
    if (p != 0) {
        if (U32(p, 0) == 0)
            SIM_API(HeapFreeFn, IAT_HEAPFREE)((void *)(uintptr_t)G32(0x1017b8f8), 1, (void *)(uintptr_t)U32(p, 0x17));
        else
            SIM_CALL_100b9203((uint8_t *)p + 6, G32(0x1014cefc));
    }
}

// FUNCTION: HGSIM 0x100284df
void __cdecl Sim_100284df(int key, const void *name)
{
    void *node = Sim_10027f33(key, name);

    if (node != 0)
        SIM_CALL_10027fb8(node);
}

// FUNCTION: HGSIM 0x1002d3a5
uint32_t __cdecl Sim_1002d3a5(void *obj, uint32_t x)
{
    return SIM_CALL_10031ce0(U32(obj, 0x98), U32(obj, 0x2c), x) & 1;
}

// FUNCTION: HGSIM 0x1002d3ca
void __cdecl Sim_1002d3ca(void *obj, uint32_t a, uint32_t b, uint32_t c, uint32_t d)
{
    SIM_CALL_10031f95(U32(obj, 0x98), U32(obj, 0x2c), a, b, c, d);
}
