#include "w4u.h"
#include "../unsorted_100f/w2e.h"
#include "../win/w4u_api.h"

// FUNCTION: HGSIM 0x1005a138
void __cdecl Sim_1005a138(void *record)
{
    if (G32(0x1017b910) != 0)
        G32(0x1017b8f8) = G32(0x1017b910);
    if (U32(record, 0x88) != 0)
        W4u_HeapFree(U32(record, 0x88));
    if (U32(record, 0xc8) != 0)
        W4u_HeapFree(U32(record, 0xc8));
    if (U32(record, 0xd0) != 0)
        W4u_HeapFree(U32(record, 0xd0));
    W4u_HeapFree((uint32_t)(uintptr_t)record);
    G32(0x1017b8f8) = G32(0x1017b900);
}

// FUNCTION: HGSIM 0x1005b026
void *__cdecl Sim_1005b026(const uint8_t *image)
{
    uint32_t record = W4u_HeapAlloc(0x1a1);
    uint32_t offset = 0;

    Mem_Move(BYTES(record) + 9, image + offset, 0x12c);
    offset += 0x12c;
    if (U32(record, 0x12d) != 0) {
        U32(record, 0x131) = W4u_HeapAlloc(U32(record, 0x12d) * 0x48);
        Mem_Move(BYTES(U32(record, 0x131)), image + offset, U32(record, 0x12d) * 0x48);
        offset += U32(record, 0x12d) * 0x48;
    } else {
        U32(record, 0x131) = 0;
    }
    Mem_Move(BYTES(record) + 0x135, image + offset, 0x6c);
    return BYTES(record);
}

// FUNCTION: HGSIM 0x1005b112
void __cdecl Sim_1005b112(void *record)
{
    if (U32(record, 0x131) != 0)
        W4u_HeapFree(U32(record, 0x131));
    W4u_HeapFree((uint32_t)(uintptr_t)record);
}

// FUNCTION: HGSIM 0x1005b14e
void *__cdecl Sim_1005b14e(const uint8_t *image, uint32_t *size)
{
    uint8_t *record = (uint8_t *)Sim_1005b026(image);

    if (record != 0)
        *size = U32(record, 0x12d) * 0x48 + 0x1a1;
    else
        *size = 0;
    return record;
}
