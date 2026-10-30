#include "w2g.h"
#include "../simmem.h"

#define U32(p, off) (*(uint32_t *)((uint8_t *)(p) + (off)))
#define PTR(v) ((uint8_t *)(uintptr_t)(v))
#define G32(addr) (*SIM_PTR(uint32_t, addr))
#define FIRST G32(0x101668D0)
#define LAST G32(0x101668D4)
#define ENTRY_COUNT (*SIM_PTR(int32_t, 0x109B0EAC))
#define TOP (*SIM_PTR(int32_t, 0x109B0EB0))
#define STACK SIM_PTR(int32_t, 0x109B12C0)
#define ENTRY(n) SIM_PTR(uint8_t, 0x109B16E0u + (uint32_t)(n) * 0x4Cu)

// FUNCTION: HGSIM 0x100570f1
int __cdecl Sim_100570f1(void)
{
    return 2;
}

// FUNCTION: HGSIM 0x10057f53
void __cdecl Sim_10057f53(void)
{
    int i;

    for (i = 0; i < 4; i++)
        SIM_PTR(uint32_t, 0x1015E650)[i] = 0;
}

// FUNCTION: HGSIM 0x100581a0
void __cdecl Sim_100581a0(uint8_t *entry)
{
    if (entry == 0)
        return;
    if (FIRST == 0 && LAST == 0) {
        U32(entry, 4) = 0;
        U32(entry, 0) = 0;
        FIRST = (uint32_t)(uintptr_t)entry;
        LAST = (uint32_t)(uintptr_t)entry;
        return;
    }
    U32(entry, 4) = LAST;
    U32(entry, 0) = 0;
    U32(PTR(LAST), 0) = (uint32_t)(uintptr_t)entry;
    LAST = (uint32_t)(uintptr_t)entry;
}

// FUNCTION: HGSIM 0x1005820d
void __cdecl Sim_1005820d(uint8_t *entry)
{
    if (entry == 0 || FIRST == 0 || LAST == 0)
        return;
    if (FIRST == (uint32_t)(uintptr_t)entry) {
        if (LAST == (uint32_t)(uintptr_t)entry) {
            FIRST = 0;
            LAST = 0;
        } else {
            FIRST = U32(entry, 0);
            U32(PTR(FIRST), 4) = 0;
        }
    } else if (LAST == (uint32_t)(uintptr_t)entry) {
        LAST = U32(entry, 4);
        U32(PTR(LAST), 0) = 0;
    } else {
        uint32_t prev = U32(entry, 4);
        uint32_t next = U32(entry, 0);

        U32(PTR(prev), 0) = next;
        U32(PTR(next), 4) = prev;
    }
    U32(entry, 4) = 0;
    U32(entry, 0) = 0;
}

// FUNCTION: HGSIM 0x100582d2
uint8_t *__cdecl Sim_100582d2(int number)
{
    uint8_t *entry = 0;

    if (TOP != ENTRY_COUNT - 1 && number >= 0 && number < ENTRY_COUNT) {
        entry = ENTRY(number);
        TOP = TOP + 1;
        STACK[TOP] = number;
        Sim_1005820d(entry);
    }
    return entry;
}

// FUNCTION: HGSIM 0x1005833d
uint8_t *__cdecl Sim_1005833d(void)
{
    uint8_t *entry = 0;

    if (TOP >= 0) {
        entry = ENTRY(STACK[TOP]);
        Sim_100581a0(entry);
        TOP = TOP - 1;
    }
    return entry;
}

// FUNCTION: HGSIM 0x10058493
void __cdecl Sim_10058493(void)
{
    int32_t found = 0;
    int32_t i;

    if (FIRST == 0)
        return;
    {
        uint8_t *node = PTR(FIRST);

        for (; node != 0; node = PTR(U32(node, 0))) {
            if (U32(node, 0x44) == 0xFFFFFFFFu)
                continue;
            U32(node, 0x48) = U32(node, 0x48) + G32(0x1015DC80);
            if ((int32_t)U32(node, 0x48) > (int32_t)U32(node, 0x44)) {
                SIM_PTR(uint32_t, 0x109B0EC0)[found] = U32(node, 0xC);
                found++;
            }
        }
    }
    if (found == 0)
        return;
    for (i = 0; i < found; i++)
        Sim_100585b3((int)SIM_PTR(uint32_t, 0x109B0EC0)[i]);
}

// FUNCTION: HGSIM 0x10058575
int __cdecl Sim_10058575(void)
{
    int32_t result = -1;

    if (G32(0x10143FD8) != 0) {
        uint8_t *entry = Sim_1005833d();

        if (entry != 0) {
            result = (int32_t)U32(entry, 0xC);
            U32(entry, 0x48) = 0;
        }
    }
    return result;
}

// FUNCTION: HGSIM 0x100585b3
uint8_t *__cdecl Sim_100585b3(int number)
{
    return Sim_100582d2(number);
}
