#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x100881dd
void __cdecl Sim_100881dd(float x, float z)
{
    int count = 0;
    int cx, cz, rx, rz, u, v;

    if (G32(0x1017c8c4) == 0) {
        G32(0x1036f2a0) = G32(0x1017c8cc);
        G32(0x1036f2a4) = 0;
        return;
    }
    cx = Render_Ftol(((double)x + GFL(0x1014b8e0)) / GFL(0x1012b8e0));
    G32(0x1036f3f8) = (uint32_t)Render_Ftol((double)cx + GFL(0x1012b8e4));
    cz = Render_Ftol(((double)z + GFL(0x1014b8e4)) / GFL(0x1012b8e0));
    G32(0x1036f400) = (uint32_t)Render_Ftol((double)cz + GFL(0x1012b8e4));
    rx = Render_Ftol(((double)x + GFL(0x1014b8e0)) + GFL(0x1012b8e4)) % 0xd90a;
    u = Render_Ftol((double)rx / GFL(0x1012b8e8));
    rz = Render_Ftol(((double)z + GFL(0x1014b8e4)) + GFL(0x1012b8e4)) % 0xd90a;
    v = Render_Ftol((double)rz / GFL(0x1012b8e8));
    G32(0x1036f248) = ((uint32_t)u << 1) | (uint32_t)v;
    G32(0x1036f404) = 0;
    while (GS32(0x1036f404) < 4) {
        uint32_t row = G32(0x1036f248) << 5;
        uint32_t k = G32(0x1036f404);
        int32_t a = (int32_t)(G32(0x1036f3f8) + G32(0x1017c840u + row + k * 8));
        int32_t b = (int32_t)(G32(0x1036f400) + G32(0x1017c844u + row + k * 8));

        G32(0x1036f404) = k + 1;
        if (a > -1 && a < 9 && b > -1 && b < 9) {
            uint32_t cell = 0x1017ba4cu + (uint32_t)a * 0x18c + (uint32_t)b * 0x2c;

            if (U32(G32(cell), 8) != 0) {
                G32(0x1036f2a0u + (uint32_t)count * 4) = G32(cell);
                G32(0x1036f2a4u + (uint32_t)count * 4) = 0;
                count++;
            }
        }
    }
}
