#include "w3r.h"
#include "w2l.h"
#include "../util/w3r.h"

// FUNCTION: HGSIM 0x10087b00
void __cdecl Sim_10087b00(float x, float y)
{
    GF32(0x1014b8e0) = (float)((double)GF32(0x1012b8d0) + (double)x);
    GF32(0x1014b8e4) = (float)((double)GF32(0x1012b8d0) + (double)y);
}

/* Fields +0 (word), +4..+0x20 of a cell. */
static void ClearCell(uint32_t cell)
{
    uint32_t off;

    for (off = 4; off <= 0x20; off += 4)
        G32(cell + off) = 0;
    *SIM_PTR(u16a, cell) = 0x4000;
}

// FUNCTION: HGSIM 0x100883a2
void __cdecl Sim_100883a2(void)
{
    int i, j;

    G32(0x1036f400) = 0;
    G32(0x1036f3f8) = 0;
    Sim_10087b41();
    for (i = 0; i < 0x19; i++)
        G32(0x1036f2a0u + (uint32_t)i * 4u) = 0;
    for (i = 0; i < 9; i++) {
        for (j = 0; j < 9; j++) {
            uint32_t cell = 0x1017ba28u + (uint32_t)i * 0x18cu + (uint32_t)j * 0x2cu;

            ClearCell(cell);
            G32(cell + 0x28) = 0;
            G32(cell + 0x24) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, cell);
        }
    }
    ClearCell(0x1017b9b0);
    ClearCell(0x1017b988);
    ClearCell(0x1017b9d8);
    ClearCell(0x1017c818);
    ClearCell(0x1017ba00);
    G32(0x1017c8cc) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1017b988);
    G32(0x1017c8d0) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1017b9d8);
    G32(0x1017c8c8) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1017b9b0);
    G32(0x1017c8d4) = (uint32_t)(uintptr_t)SIM_PTR(uint8_t, 0x1017ba00);
    Sim_10087b00(3979.0f, -3984.5f);
}
