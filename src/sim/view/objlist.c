#include "view.h"
#include "../simmem.h"

/* Field of a structure at a byte offset. */
#define FLD(type, p, off) (*(type *)((uint8_t *)(p) + (off)))
/* Pointers are stored as 32-bit values in the DLL's data. */
#define PTR(v) ((uint8_t *)(uintptr_t)(v))
#define NODE_NEXT 0x14
#define NODE_PREV 0x18

// FUNCTION: HGSIM 0x10088c9f
void __cdecl List_InsertBefore(void *node, void *before)
{
    uint8_t *owner;

    if (node == 0 || before == 0 || (FLD(uint16_t, node, 0) & 0x800) != 0 || FLD(uint32_t, node, NODE_PREV) != 0 ||
        FLD(uint32_t, node, NODE_NEXT) != 0)
        return;
    owner = PTR(FLD(uint32_t, node, 0x28));
    if (owner != 0 && FLD(uint32_t, owner, 0) != 0)
        return;
    if (FLD(uint32_t, before, NODE_PREV) != 0)
        FLD(uint32_t, PTR(FLD(uint32_t, before, NODE_PREV)), NODE_NEXT) = (uint32_t)(uintptr_t)node;
    FLD(uint32_t, node, NODE_PREV) = FLD(uint32_t, before, NODE_PREV);
    FLD(uint32_t, before, NODE_PREV) = (uint32_t)(uintptr_t)node;
    FLD(uint32_t, node, NODE_NEXT) = (uint32_t)(uintptr_t)before;
}

// FUNCTION: HGSIM 0x10088d23
void __cdecl ObjList_Unlink(void *node)
{
    if (node == 0)
        return;
    if (FLD(uint32_t, node, NODE_PREV) != 0)
        FLD(uint32_t, PTR(FLD(uint32_t, node, NODE_PREV)), NODE_NEXT) = FLD(uint32_t, node, NODE_NEXT);
    if (FLD(uint32_t, node, NODE_NEXT) != 0)
        FLD(uint32_t, PTR(FLD(uint32_t, node, NODE_NEXT)), NODE_PREV) = FLD(uint32_t, node, NODE_PREV);
    FLD(uint32_t, node, NODE_NEXT) = 0;
    FLD(uint32_t, node, NODE_PREV) = 0;
}

// FUNCTION: HGSIM 0x10072ff2
int32_t __cdecl Sim_10072ff2(void)
{
    return 0xc51 + 0x940 + 0x3fc + 0x510;
}

// FUNCTION: HGSIM 0x1007652f
void *__cdecl Sim_1007652f(int32_t key)
{
    uint8_t *node = PTR(*SIM_PTR(uint32_t, 0x1015dc14));

    while (node != 0 && FLD(int32_t, node, 0) != key)
        node = PTR(FLD(uint32_t, node, 0x14));
    return node;
}

// FUNCTION: HGSIM 0x10076624
int32_t __cdecl Sim_10076624(void *stack, int32_t value)
{
    int32_t count = FLD(int32_t, stack, 0xc);

    if (count >= 8)
        return 0;
    FLD(int32_t, PTR(FLD(uint32_t, stack, 8)), count * 4) = value;
    FLD(int32_t, stack, 0xc) = count + 1;
    return 1;
}

// FUNCTION: HGSIM 0x10077fd8
void __cdecl Sim_10077fd8(void *obj)
{
    FLD(uint32_t, obj, 0x30) += 1;
    if (FLD(uint32_t, obj, 0x30) == 0xe)
        FLD(uint32_t, obj, 0x30) = 0;
}

// FUNCTION: HGSIM 0x10078177
void __cdecl Sim_10078177(void)
{
    *SIM_PTR(uint32_t, 0x1096fc04) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e890c)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc00) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e88e8)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc08) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e8928)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc0c) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e8938)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc10) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e894c)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc14) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e8950)), 0x30);
    *SIM_PTR(uint32_t, 0x1096fc18) = FLD(uint32_t, PTR(*SIM_PTR(uint32_t, 0x109e8954)), 0x30);
}

// FUNCTION: HGSIM 0x10079e3b
int32_t __cdecl Sim_10079e3b(const uint8_t *id)
{
    uint32_t index = id[0];
    uint32_t kind = (uint32_t)id[1] << 8;

    if (kind == 0x200)
        return (int32_t)(int16_t)FLD(uint16_t, PTR(SIM_PTR(uint32_t, 0x109e8b40)[index]), 0x1c) & 0xe;
    if (kind == 0x400)
        return (int32_t)(int16_t)FLD(uint16_t, SIM_PTR(uint8_t, 0x109e9ba0), index * 0x50) & 0xe;
    return 0;
}

// FUNCTION: HGSIM 0x10079feb
int32_t __cdecl Sim_10079feb(int32_t kind)
{
    return kind == 8 || kind == 5;
}

// FUNCTION: HGSIM 0x100819af
int32_t __cdecl Bits_Count(uint32_t value)
{
    int32_t count = 0;

    do {
        if (value & 1)
            count++;
        value >>= 1;
    } while (value != 0);
    return count;
}

// FUNCTION: HGSIM 0x10087c86
void __cdecl Sim_10087c86(void)
{
    uint32_t *out = SIM_PTR(uint32_t, 0x1036f2a0);
    int32_t n = 0;
    int32_t i, j;
    uint8_t *node;

    if (*SIM_PTR(int32_t, 0x1017c8c4) == 0) {
        out[0] = *SIM_PTR(uint32_t, 0x1017c8cc);
        out[1] = 0;
        return;
    }
    node = PTR(*SIM_PTR(uint32_t, 0x1017c8c8));
    if (node != 0 && FLD(uint32_t, node, 8) != 0) {
        out[n] = (uint32_t)(uintptr_t)node;
        out[n + 1] = 0;
        n++;
    }
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            node = PTR(FLD(uint32_t, SIM_PTR(uint8_t, 0x1017ba4c), i * 0x18c + j * 0x2c));
            if (node != 0 && FLD(uint32_t, node, 8) != 0) {
                out[n] = (uint32_t)(uintptr_t)node;
                out[n + 1] = 0;
                n++;
            }
        }
    }
    out[n] = 0;
}

// FUNCTION: HGSIM 0x1008f630
void __cdecl Sim_1008f630(void **pp)
{
    uint32_t *p = *(uint32_t **)pp;

    p[0] = 1;
    p[1] = 0;
}
