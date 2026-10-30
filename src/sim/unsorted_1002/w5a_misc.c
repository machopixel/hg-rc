#include "w5a.h"
#include "w2i.h"
#include "w2l.h"
#include "w4u.h"
#include "../unsorted_1003/w4u.h"
#include "../unsorted_1003/w5a.h"
#include "../util/access.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x100203cf
void __cdecl Sim_100203cf(int a, int b)
{
    uint32_t sub = U32(PTRTAB(GS32(0x1017b8c8)), 0x28);
    void *second;

    if (b == -1)
        second = 0;
    else
        second = GBYTES(0x10133950) + U32(sub, 0x613) * 0x60u + (uint32_t)b * 0x18u;
    W5A_CALL_1001f790(GBYTES(0x10133a10) + U32(sub, 0x613) * 0x420u + (uint32_t)a * 0x18u, second, -1);
}

// FUNCTION: HGSIM 0x10029125
void __cdecl Sim_10029125(int i)
{
    uint8_t *entry = GBYTES(0x109f09e0) + (uint32_t)i * 0x14u;
    uint32_t object = U32(entry, 4);
    int key;
    int k;

    if (object == 0)
        return;
    W5A_CALL_10029b1f(object);
    if (U32(entry, 0x10) != 0)
        ((void(__cdecl *)(uint32_t))(uintptr_t)U32(entry, 0x10))(object);
    key = Sim_1002997e((int)object);
    if (key != -1)
        Sim_1002990f(key);
    for (k = 0; k < 5; k++)
        U32(entry, k * 4) = G32(0x1015dc48u + k * 4u);
}

// FUNCTION: HGSIM 0x1002fa3f
void __cdecl Sim_1002fa3f(void *obj)
{
    uint8_t *block;

    if (obj == 0)
        return;
    if ((U16(obj, 0) & 0x200) != 0)
        return;
    if (U32(obj, 0x98) == 0)
        U32(obj, 0x98) = (uint32_t)(uintptr_t)Sim_1002f9e0();
    block = (uint8_t *)(uintptr_t)U32(obj, 0x98);
    if (block == 0)
        return;
    W5A_CALL_1002fba5(obj, block, block + 4, block + 8, block + 0xc, block + 0x10, block + 0x14);
    U16(obj, 0) |= 0x200;
}

// FUNCTION: HGSIM 0x1002d1c0
int __cdecl Sim_1002d1c0(void *obj, float a, int unused, float b)
{
    const uint8_t *block;

    Sim_1002fa3f(obj);
    block = (const uint8_t *)(uintptr_t)U32(obj, 0x98);
    if (block == 0)
        return 0;
    if (a >= *(const float *)(block + 0) && a <= *(const float *)(block + 4) &&
        b >= *(const float *)(block + 0x10) && b <= *(const float *)(block + 0x14))
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1002feef
void __cdecl Sim_1002feef(void *obj)
{
    uint32_t block;

    if (obj == 0)
        return;
    block = U32(obj, 0x98);
    if (block == 0)
        return;
    switch (S32(obj, 0x34)) {
    case 0:
        W4u_HeapFree(block);
        break;
    case 5:
        Sim_10031917((void *)(uintptr_t)block);
        break;
    case 8:
        Sim_100305f8((void *)(uintptr_t)block);
        break;
    default:
        break;
    }
    U32(obj, 0x98) = 0;
}
