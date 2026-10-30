#include "w5y.h"
#include "../view/w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100a81fb
uint32_t __cdecl Sim_100a81fb(uint32_t a, int i, uint32_t b, uint32_t d)
{
    uint32_t handle = G32(0x101c65e0u + (uint32_t)i * 0x14au);

    SIM_FN(int(__cdecl *)(uint32_t, uint32_t, int), 0x10105fe0)(handle, b, 0);
    return SIM_FN(uint32_t(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100a796c)(handle, a, d);
}

// FUNCTION: HGSIM 0x100a9b8d
void __cdecl Sim_100a9b8d(void *obj, const char *text)
{
    U32(obj, 0x90) = (uint32_t)(uintptr_t)SIM_FN(char *(__cdecl *)(const char *), 0x1010c9f0)(text);
}

// FUNCTION: HGSIM 0x100aa4a2
void __cdecl Sim_100aa4a2(void *obj)
{
    uint32_t child;

    if (U32(obj, 0x8c) != 0)
        Sim_1008a1ea((void *)(uintptr_t)U32(obj, 0x8c));
    for (child = U32(obj, 4); child != 0; child = U32(child, 8))
        Sim_100aa4a2((void *)(uintptr_t)child);
}

// FUNCTION: HGSIM 0x100aa4f0
void __cdecl Sim_100aa4f0(void *obj, int value)
{
    uint32_t child;

    if (U32(obj, 0x8c) != 0)
        U16(U32(obj, 0x8c), 2) = (uint16_t)value;
    for (child = U32(obj, 4); child != 0; child = U32(child, 8))
        Sim_100aa4f0((void *)(uintptr_t)child, value);
}

// FUNCTION: HGSIM 0x100aa799
void __cdecl Sim_100aa799(void *obj)
{
    SIM_FN(void(__cdecl *)(void *), 0x100aa742)(obj);
    if (G32(0x1017c8c4) != 0 && U32(obj, 0) == 0)
        SIM_FN(void(__cdecl *)(uint32_t), 0x10088a2d)(U32(obj, 0x8c));
}

// FUNCTION: HGSIM 0x100aa87a
void __cdecl Sim_100aa87a(void *obj, const void *src)
{
    SIM_FN(void(__cdecl *)(const void *, void *), 0x10060d2e)(src, BYTES(obj) + 0x7c);
    U32(obj, 0x94) &= ~0x8000u;
    U32(obj, 0x70) = U32(src, 0x24);
    U32(obj, 0x74) = U32(src, 0x28);
    U32(obj, 0x78) = U32(src, 0x2c);
}

// FUNCTION: HGSIM 0x100ab151
void __cdecl Sim_100ab151(void *a, uint32_t b, uint32_t *p3, uint32_t *p4)
{
    if (a == 0)
        return;
    SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100ab0d6)(U32(a, 4), b);
    *p4 = G32(0x101c64f4);
    *p3 = G32(0x101c6504);
}
