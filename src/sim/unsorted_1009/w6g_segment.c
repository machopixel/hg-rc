#include "w6g.h"
#include "w3o.h"
#include "../util/w3o.h"

// FUNCTION: HGSIM 0x1009cc82
void __cdecl Sim_1009cc82(void *seg)
{
    double dx, dy, dz, length;

    if (U32(seg, 0x34) == 2)
        return;
    dx = (double)FL(seg, 0xc) - FL(seg, 0);
    dy = (double)FL(seg, 0x10) - FL(seg, 4);
    dz = (double)FL(seg, 0x14) - FL(seg, 8);
    length = W6G_SQRT(dx * dx + dy * dy + dz * dz);
    dx = dx / length;
    dy = dy / length;
    dz = dz / length;
    FL(seg, 0x24) = (float)dx;
    FL(seg, 0x28) = (float)dy;
    FL(seg, 0x2c) = (float)dz;
    FL(seg, 0x30) = (float)length;
    U32(seg, 0x34) = 2;
}

// FUNCTION: HGSIM 0x1009cdd5
void __cdecl Sim_1009cdd5(void *seg, float t)
{
    uint32_t state = U32(seg, 0x34);

    if (state == 0)
        Sim_1009cbfe(seg);
    if (!(FL(seg, 0x30) > GFL(0x1012ba04)))
        return;
    FL(seg, 0) = (float)((double)t * FL(seg, 0x24) + FL(seg, 0));
    FL(seg, 4) = (float)((double)t * FL(seg, 0x28) + FL(seg, 4));
    FL(seg, 8) = (float)((double)t * FL(seg, 0x2c) + FL(seg, 8));
    FL(seg, 0x18) = (float)((double)FL(seg, 0xc) - FL(seg, 0));
    FL(seg, 0x1c) = (float)((double)FL(seg, 0x10) - FL(seg, 4));
    FL(seg, 0x20) = (float)((double)FL(seg, 0x14) - FL(seg, 8));
    U32(seg, 0x34) = 0;
    if (state == 2)
        Sim_1009cc82(seg);
    else
        Sim_1009cbfe(seg);
}

// FUNCTION: HGSIM 0x1009ceab
void __cdecl Sim_1009ceab(void *seg, float x, float y, float z)
{
    uint32_t state;

    /* the original copies the three arguments as dwords */
    U32(seg, 0xc) = *(const u32a *)&x;
    U32(seg, 0x10) = *(const u32a *)&y;
    U32(seg, 0x14) = *(const u32a *)&z;
    FL(seg, 0x18) = (float)((double)x - FL(seg, 0));
    FL(seg, 0x1c) = (float)((double)y - FL(seg, 4));
    FL(seg, 0x20) = (float)((double)z - FL(seg, 8));
    state = U32(seg, 0x34);
    U32(seg, 0x34) = 0;
    if (state == 1) {
        Sim_1009cbfe(seg);
    } else if (state == 2) {
        Sim_1009cc82(seg);
    } else {
        U32(seg, 0x30) = 0;
        U32(seg, 0x2c) = 0;
        U32(seg, 0x28) = 0;
        U32(seg, 0x24) = 0;
    }
}
