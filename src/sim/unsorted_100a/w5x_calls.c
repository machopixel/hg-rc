#include "w5x.h"
#include "w2k.h"
#include "../util/access.h"
#include "../view/w2l.h"
#include "../unsorted_1008/fns.h"

// FUNCTION: HGSIM 0x100a6009
void *__cdecl Sim_100a6009(uint32_t a, uint32_t b)
{
    return SIM_CALL_100a5d38(a, 0, b, 0);
}

/* the helpers of Sim_100a8816, through their addresses as the original calls them */
#define W5XA_ALLOC_ZERO SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x10028559)
#define W5XA_FILL SIM_FN(void *(__cdecl *)(uint32_t, int, uint32_t), 0x1002858a)
#define W5XA_SYSTEM_ERROR SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x100bc280)
#define W5XA_TRIG_TABLES SIM_FN(int(__cdecl *)(void), 0x100846e0)
#define W5XA_10084A17 SIM_FN(int(__cdecl *)(void), 0x10084a17)

// FUNCTION: HGSIM 0x100a8816
int __cdecl Sim_100a8816(int arenaKiB, int sortEntries)
{
    uint32_t size;

    G32(0x1017d17c) = (uint32_t)sortEntries;
    G32(0x1014d1a4) = (uint32_t)arenaKiB << 10;
    size = ((uint32_t)arenaKiB << 10) + (G32(0x1017d17c) << 1) * 8u;
    G32(0x1017d180) = W5XA_ALLOC_ZERO(size);
    if (G32(0x1017d180) == 0)
        W5XA_SYSTEM_ERROR(0x18, 0);
    W5XA_FILL(G32(0x1017d180), 0, size);
    G32(0x1017d170) = G32(0x1017d180);
    G32(0x1017d168) = G32(0x1017d180) + G32(0x1014d1a4);
    G32(0x1017d16c) = G32(0x1017d168) + G32(0x1017d17c) * 8u;
    G32(0x1017d178) = G32(0x1017d170);
    G32(0x1017d174) = G32(0x1017d170) + G32(0x1014d1a4) - 0x30u;
    W5XA_TRIG_TABLES();
    return W5XA_10084A17();     /* the original returns what 0x10084a17 leaves in eax */
}

// FUNCTION: HGSIM 0x100a9ca3
void __cdecl Sim_100a9ca3(void *obj, uint32_t a, uint32_t b, uint32_t c)
{
    SIM_CALL_100a9ba7(obj);
    SIM_CALL_10060681((uint8_t *)obj + 0x4c, a, b, c);
}

// FUNCTION: HGSIM 0x100aa351
void __cdecl Sim_100aa351(void *node)
{
    uint32_t child;

    if (U32(node, 0x8c) != 0)
        Sim_1008a40a((void *)(uintptr_t)U32(node, 0x8c));
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa351((void *)(uintptr_t)child);
}

// FUNCTION: HGSIM 0x100aa454
void __cdecl Sim_100aa454(void *node)
{
    uint32_t child;

    if (U32(node, 0x8c) != 0)
        Sim_1008a220((uint16_t *)(uintptr_t)U32(node, 0x8c));
    for (child = U32(node, 4); child != 0; child = U32(child, 8))
        Sim_100aa454((void *)(uintptr_t)child);
}

// FUNCTION: HGSIM 0x100aa812
void __cdecl Sim_100aa812(void *dst, const void *src)
{
    SIM_CALL_10060d2e(src, (uint8_t *)dst + 0x3c);
    U32(dst, 0x94) = U32(dst, 0x94) & 0xffffbfffu;
    U32(dst, 0x30) = U32(src, 0x24);
    U32(dst, 0x34) = U32(src, 0x28);
    U32(dst, 0x38) = U32(src, 0x2c);
}

// FUNCTION: HGSIM 0x100aa863
void *__cdecl Sim_100aa863(void *obj)
{
    SIM_CALL_100a9ba7(obj);
    return (uint8_t *)obj + 0x4c;
}

// FUNCTION: HGSIM 0x100aaf9b
void __cdecl Sim_100aaf9b(void *node)
{
    if (node == 0)
        return;
    if (U32(node, 8) != 0)
        Sim_100aaf9b((void *)(uintptr_t)U32(node, 8));
    if (U32(node, 4) != 0)
        Sim_100aaf9b((void *)(uintptr_t)U32(node, 4));
    SIM_CALL_100aaf32(node);
}

// FUNCTION: HGSIM 0x100ac58e
void __cdecl Sim_100ac58e(int index)
{
    if (index < 0 || index >= GS32(0x1017d1c8))
        return;
    Sim_100aa351((void *)(uintptr_t)U32(GBYTES(0x101c57e0u + (uint32_t)index * 0x34u), 0x30));
    Sim_100abbe8(index);
}

// FUNCTION: HGSIM 0x100ae64d
int __cdecl Sim_100ae64d(void *p, uint32_t a, uint32_t b)
{
    int r = SIM_CALL_100feb20(U32(p, 0), a, b);

    if (r != 0)
        return r;
    U32(p, 4) = (uint32_t)SIM_CALL_100fe970(U32(p, 0));
    return 0;
}
