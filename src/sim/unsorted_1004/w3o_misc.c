#include "w3o.h"
#include "fns.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x10044296
void __cdecl Sim_10044296(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f, double g)
{
    if (GS32(0x1013aec4) < 10) {
        uint8_t *entry = GBYTES(0x1015ed70u + G32(0x1013aec4) * 0x1c);

        U32(entry, 0) = a;
        U32(entry, 4) = b;
        U32(entry, 8) = c;
        U32(entry, 0x10) = d;
        U32(entry, 0xc) = e;
        U32(entry, 0x14) = f;
        U32(entry, 0x18) = (uint32_t)Render_Ftol(g * GF64(0x10111558));
        G32(0x1013aec4) = G32(0x1013aec4) + 1;
    }
}

// FUNCTION: HGSIM 0x1004c965
int __cdecl Sim_1004c965(int a, int b)
{
    uint32_t sum = G32(0x101116b2u + (uint32_t)a * 0xaa);
    uint32_t k[5];
    float value;

    k[0] = (uint32_t)Sim_1004c909((unsigned)b) * 0x98;
    sum += G32(0x10124eb8u + k[0]);
    k[1] = (uint32_t)Sim_1004c917(b) * 0x98;
    sum += G32(0x10124eb8u + k[1]);
    k[2] = (uint32_t)Sim_1004c928(b) * 0x98;
    sum += G32(0x10124eb8u + k[2]);
    k[3] = (uint32_t)Sim_1004c939(b) * 0x98;
    sum += G32(0x10124eb8u + k[3]);
    k[4] = (uint32_t)Sim_1004c94a(b) * 0x98;
    sum += G32(0x10124eb8u + k[4]);
    value = (float)(int32_t)sum;
    k[0] = (uint32_t)Sim_1004c909((unsigned)b) * 0x98;
    value = (float)((double)value * GFL(0x10124ebcu + k[0]));
    k[1] = (uint32_t)Sim_1004c917(b) * 0x98;
    value = (float)((double)value * GFL(0x10124ebcu + k[1]));
    k[2] = (uint32_t)Sim_1004c928(b) * 0x98;
    value = (float)((double)value * GFL(0x10124ebcu + k[2]));
    k[3] = (uint32_t)Sim_1004c939(b) * 0x98;
    value = (float)((double)value * GFL(0x10124ebcu + k[3]));
    k[4] = (uint32_t)Sim_1004c94a(b) * 0x98;
    return Render_Ftol((double)value * GFL(0x10124ebcu + k[4]));
}
