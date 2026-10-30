#include "w2i.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1002850e
void __cdecl Sim_1002850e(void)
{
}

// FUNCTION: HGSIM 0x10028513
int __cdecl Sim_10028513(int a)
{
    return a;
}

// FUNCTION: HGSIM 0x1002895a
int __cdecl Sim_1002895a(int a)
{
    return a;
}

// FUNCTION: HGSIM 0x10028a74
void __cdecl Sim_10028a74(void *obj, int16_t value)
{
    U16(obj, 6) = (uint16_t)value;
}

// FUNCTION: HGSIM 0x1002990f
void __cdecl Sim_1002990f(int index)
{
    uint32_t entry = 0x109EEB00u + (uint32_t)index * 0x30u;

    G32(entry) = 0;
    G32(entry + 4) = 0;
    G32(entry + 0x10) = 0;
    G32(entry + 0xc) = 0;
    G32(entry + 8) = 0;
    G32(entry + 0x1c) = 0;
    G32(entry + 0x18) = 0;
    G32(entry + 0x14) = 0;
    G32(entry + 0x20) = 0;
}

// FUNCTION: HGSIM 0x1002d263
int __cdecl Sim_1002d263(int a, int b)
{
    (void)a;
    (void)b;
    return 0;
}
