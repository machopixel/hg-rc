#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"

#define PTR(v) ((void *)(uintptr_t)(v))
#define TICK G32(0x1015dc78)
#define CURRENT G32(0x1017e5b8)

// FUNCTION: HGSIM 0x100cc79a
void __cdecl Sim_100cc79a(uint32_t *list, void *node)
{
    uint32_t n = (uint32_t)(uintptr_t)node;
    uint32_t prev;

    if (n == 0)
        return;
    if (n == list[0]) {
        list[0] = U32(n, 0x14);
    } else {
        for (prev = list[0]; prev != 0; prev = U32(prev, 0x14)) {
            if (U32(prev, 0x14) == n) {
                U32(prev, 0x14) = U32(n, 0x14);
                break;
            }
        }
    }
    CURRENT = n;
    (*(TimerCallback *)PTR(n))(2, 0, (int)TICK, (int)U32(n, 8));
    Heap_FreeRaw(PTR(U32(n, 4)));
}

// FUNCTION: HGSIM 0x100cc830
void __cdecl Sim_100cc830(uint32_t *list)
{
    uint32_t n;

    for (n = list[0]; n != 0; n = U32(n, 0x14))
        Sim_100cc79a(list, PTR(n));
}

// FUNCTION: HGSIM 0x100cc6ea
void *__cdecl Sim_100cc6ea(uint32_t *list, TimerCallback callback, uint32_t interval, uint32_t arg)
{
    uint8_t *node = Heap_AllocRaw(0x18);

    if (node == 0)
        return 0;
    U32(node, 0) = (uint32_t)(uintptr_t)callback;
    U32(node, 4) = 0;
    U32(node, 8) = interval;
    U32(node, 0xc) = TICK;
    U32(node, 0x10) = TICK;
    U32(node, 0x14) = list[0];
    list[0] = (uint32_t)(uintptr_t)node;
    CURRENT = (uint32_t)(uintptr_t)node;
    if (callback != 0 && callback(0, (int)arg, (int)TICK, (int)interval) == 0) {
        Sim_100cc79a(list, PTR(CURRENT));
        return 0;
    }
    return node;
}

// FUNCTION: HGSIM 0x100cc8c5
void __cdecl Sim_100cc8c5(uint32_t *list)
{
    for (CURRENT = list[0]; CURRENT != 0; CURRENT = U32(CURRENT, 0x14)) {
        if (U32(CURRENT, 0) != 0 && S32(CURRENT, 0x10) <= (int32_t)TICK) {
            U32(CURRENT, 0x10) = TICK + U32(CURRENT, 8);
            if ((*(TimerCallback *)PTR(CURRENT))(1, 0, (int)TICK, (int)U32(CURRENT, 8)) == 0)
                Sim_100cc79a(list, PTR(CURRENT));
            else
                U32(CURRENT, 0xc) = TICK;
        }
    }
}
