#include "w2j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1005570e
void __cdecl Sim_1005570e(void)
{
    G32(0x1013f140) = 0;
}

// FUNCTION: HGSIM 0x10056df3
int __cdecl Sim_10056df3(void)
{
    return 0;
}

// FUNCTION: HGSIM 0x10058389
void __cdecl Sim_10058389(void)
{
    int i;

    G32(0x101668d4) = 0;
    G32(0x101668d0) = G32(0x101668d4);
    G32(0x109b0eac) = 0x100;
    G32(0x109b0eb0) = G32(0x109b0eac) - 1;
    for (i = 0; i < 0x100; i++) {
        uint8_t *record = GBYTES(0x109b16e0) + i * 0x4c;

        SIM_PTR(u32a, 0x109b12c0)[i] = (uint32_t)i;
        U32(record, 8) = 0;
        U32(record, 0) = 0;
        U32(record, 4) = 0;
        U32(record, 0xc) = (uint32_t)i;
        U32(record, 0x10) = 0xffffffffu;
        U32(record, 0x14) = 0;
        U32(record, 0x18) = 0;
        U32(record, 0x1c) = 0;
        U32(record, 0x2c) = 0;
        U32(record, 0x30) = 0;
        U32(record, 0x34) = 0;
        U32(record, 0x38) = 0;
        U32(record, 0x3c) = 0;
        U32(record, 0x40) = 0;
        U32(record, 0x44) = 0xffffffffu;
    }
}

// FUNCTION: HGSIM 0x1005892e
void __cdecl Sim_1005892e(int index, int flag)
{
    if (index < 0 || index >= GS32(0x109b0eac))
        return;
    U32(GBYTES(0x109b16e0) + index * 0x4c, 8) = flag != 0 ? 1 : 0;
}

// FUNCTION: HGSIM 0x1005fa30
void __cdecl Sim_1005fa30(void)
{
    G32(0x101444ec) = G32(0x101c47ec);
    G32(0x10144504) = G32(0x101c4968);
    G32(0x10144518) = G32(0x101c4fd8);
    G32(0x101448f4) = G32(0x101c5160);
    G32(0x1014490c) = G32(0x101c497c);
    G32(0x10144920) = G32(0x101c4b9c);
    G32(0x10144934) = G32(0x101c49b0);
    G32(0x10144948) = G32(0x101c4844);
    G32(0x1014495c) = G32(0x101c46e4);
    G32(0x10144970) = G32(0x101c4fd8);
    G32(0x10144360) = G32(0x101c5110);
    G32(0x10144364) = G32(0x101c4fc4);
    G32(0x101443a8) = G32(0x101c5280);
    G32(0x101443ac) = G32(0x101c49c8);
    G32(0x101443f0) = G32(0x101c4724);
    G32(0x101443f4) = G32(0x101c4794);
    G32(0x10144438) = G32(0x101c5110);
    G32(0x1014443c) = G32(0x101c4724);
    G32(0x10144440) = G32(0x101c49d4);
    G32(0x10144444) = G32(0x101c4794);
    G32(0x10144644) = G32(0x101c49e4);
    G32(0x1014465c) = G32(0x101c4b9c);
    G32(0x10144670) = G32(0x101c4fd8);
    G32(0x1014479c) = G32(0x101c4578);
    G32(0x101447b4) = G32(0x101c49c0);
    G32(0x101447c8) = G32(0x101c4fd8);
}
