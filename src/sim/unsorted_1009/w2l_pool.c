#include "w2l.h"
#include "../fx/w32.h"
#include "../util/access.h"

#define FIRST G32(0x1017cc28)
#define LAST G32(0x1017cc2c)

// FUNCTION: HGSIM 0x1009044e
void __cdecl Sim_1009044e(void *node)
{
    if (node == 0 || FIRST == 0 || LAST == 0)
        return;
    if (FIRST == (uint32_t)(uintptr_t)node) {
        if (LAST == (uint32_t)(uintptr_t)node) {
            FIRST = 0;
            LAST = 0;
        } else {
            FIRST = U32(node, 4);
            U32(FIRST, 0) = 0;
        }
    } else if (LAST == (uint32_t)(uintptr_t)node) {
        LAST = U32(node, 0);
        U32(LAST, 4) = 0;
    } else {
        uint32_t prev = U32(node, 0);
        uint32_t next = U32(node, 4);

        U32(prev, 4) = next;
        U32(next, 0) = prev;
    }
    U32(node, 0) = 0;
    U32(node, 4) = 0;
}

// FUNCTION: HGSIM 0x10090513
void *__cdecl Sim_10090513(int index)
{
    void *record = 0;

    if ((uint32_t)GS32(0x10215ae8) != (uint32_t)GS32(0x1017cc20) - 1u && index >= 0 && index < GS32(0x1017cc20)) {
        record = SIM_PTR(uint8_t, 0x10217c20u + (uint32_t)index * W32_REC_SIZE);
        GS32(0x10215ae8) = (int32_t)((uint32_t)GS32(0x10215ae8) + 1u);
        SIM_PTR(s32a, 0x10216b80)[GS32(0x10215ae8)] = index;
        Sim_1009044e(record);
    }
    return record;
}

// FUNCTION: HGSIM 0x10090cc8
void __cdecl Sim_10090cc8(int index)
{
    if (index > 0 && index < GS32(0x1017cc20)) {
        U32(SIM_PTR(uint8_t, 0x10217c20u + (uint32_t)index * W32_REC_SIZE), W32_REC_ARG) = 0;
        Sim_10090513(index);
    }
}

// FUNCTION: HGSIM 0x1009ca21
uint32_t __cdecl Sim_1009ca21(void)
{
    return 0;
}
