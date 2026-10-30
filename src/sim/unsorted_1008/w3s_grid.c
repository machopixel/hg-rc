#include "w3s.h"
#include "../render/w3s_fp.h"
#include "../util/util.h"
#include "../view/w2l.h"

/* trunc(trunc((v + bias) / cell) + 0.5), the grid index along one axis. */
static int32_t GridIndex(float v, uint32_t biasAddr)
{
    int32_t cells = Render_Ftol(((double)v + (double)GF32(biasAddr)) / (double)GF32(0x1012b8e0));

    return Render_Ftol((double)cells + (double)GF32(0x1012b8e4));
}

// FUNCTION: HGSIM 0x10088775
int __cdecl Sim_10088775(float x, float z)
{
    int32_t column = GridIndex(x, 0x1014b8e0);
    int32_t row = GridIndex(z, 0x1014b8e4);

    if (column >= 0 && column < 9 && row >= 0 && row < 9)
        return S32(GBYTES(0x1017ba4c), (uint32_t)column * 0x18cu + (uint32_t)row * 0x2cu);
    return GS32(0x1017c8c8);
}

// FUNCTION: HGSIM 0x1008a46e
void __cdecl Sim_1008a46e(void *obj)
{
    uint32_t x, y, z;
    int32_t column, row;

    if (obj == 0)
        return;
    if ((U16(obj, 0) & 0x1000) == 0 || (U16(obj, 2) & 0xf0) == 0x70)
        return;
    U16(obj, 0) = (uint16_t)(U16(obj, 0) & 0xefff);
    if (U32(obj, 4) == 0)
        return;
    Sim_1008a871(obj);
    if (G32(0x1017c8c4) == 0) {
        Sim_1008a8f5(obj, GBYTES(0x1017b988));
        return;
    }
    Sim_100a9cfb(BYTES(U32(obj, 0x28)), &x, &y, &z);
    {
        column = GridIndex(*(f32a *)&x, 0x1014b8e0);
        row = GridIndex(*(f32a *)&z, 0x1014b8e4);
    }
    if (column < 0 || column >= 9 || row < 0 || row >= 9) {
        U32(obj, 0x8c) = 0x270f;
        U32(obj, 0x90) = 0x270f;
        Sim_1008a8f5(obj, GBYTES(0x1017b9b0));
    } else {
        U32(obj, 0x8c) = (uint32_t)column;
        U32(obj, 0x90) = (uint32_t)row;
        Sim_1008a8f5(obj, GBYTES(0x1017ba28) + (uint32_t)column * 0x18cu + (uint32_t)row * 0x2cu);
    }
}

// FUNCTION: HGSIM 0x1008c5e0
void __cdecl Sim_1008c5e0(void *obj)
{
    uint8_t *first = BYTES(U32(obj, 4)) + (uint32_t)U8(obj, 8) * 3;
    uint8_t *p = first;
    uint8_t saved[3];
    int i;

    for (i = 0; i < 3; i++)
        saved[i] = *p++;
    p = first;
    for (i = 0; i < (int32_t)((U32(obj, 9) - 1u) * 3u); i++) {
        *p = p[3];
        p++;
    }
    for (i = 0; i < 3; i++)
        *p++ = saved[i];
    ((void(__cdecl *)(uint32_t))(uintptr_t)U32(G32(0x1015e688), 0x18))(U32(obj, 4));
}
