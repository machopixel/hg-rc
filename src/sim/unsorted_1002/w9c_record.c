#include "w9c.h"
#include "../unsorted_1004/w3p.h"
#include "../unsorted_100f/w2e.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1002a2f6
void __cdecl Sim_1002a2f6(void *rec)
{
    U32(rec, 0x00) = 0;
    U32(rec, 0x04) = 0;
    U32(rec, 0x08) = 0;
    U32(rec, 0x10) = 0;
    U32(rec, 0x14) = 0;
    U16(rec, 0x1c) = 0;
    U16(rec, 0x1e) = 0;
    U32(rec, 0x20) = 0;
    U32(rec, 0x24) = 0xffffffffu;
    U32(rec, 0x28) = 0;
    U32(rec, 0x2c) = 0;
    U32(rec, 0x30) = 0;
    U32(rec, 0x34) = 0;
    U32(rec, 0x38) = 0;
    U32(rec, 0x3c) = 0;
    U32(rec, 0x40) = 0;
    U32(rec, 0x44) = 0;
    U32(rec, 0x48) = 0;
    U32(rec, 0x4c) = 0;
    U32(rec, 0x50) = 0;
    U32(rec, 0x54) = 0;
    U32(rec, 0x16c) = 0;
    U32(rec, 0x60) = 0;
    U32(rec, 0x5c) = 0;
    U32(rec, 0x58) = 0;
    U32(rec, 0x6c) = 0;
    U32(rec, 0x68) = 0;
    U32(rec, 0x64) = 0;
    U32(rec, 0x78) = 0;
    U32(rec, 0x74) = 0;
    U32(rec, 0x70) = 0;
    U32(rec, 0x7c) = 0;
    U32(rec, 0x80) = 0;
    U32(rec, 0x84) = 0xffffffffu;
    U32(rec, 0x88) = 0;
    U32(rec, 0x8c) = 0;
    U32(rec, 0x90) = 0;
    U32(rec, 0x94) = 0;
    U8(rec, 0x98) = 0;
    U32(rec, 0xa8) = 0;
    Sim_10047aa0((int32_t *)(BYTES(rec) + 0xac), 50000, 50000, 0.2);
    Sim_10047aa0((int32_t *)(BYTES(rec) + 0xbc), 50000, 50000, 20.0);
    U32(rec, 0xcc) = 0;
    U32(rec, 0xd0) = 0;
    U32(rec, 0xd4) = 0;
    U32(rec, 0xd8) = 0;
    U32(rec, 0xe4) = 0;
    U32(rec, 0xe0) = 0;
    U32(rec, 0xdc) = 0;
    U32(rec, 0xe8) = 0;
    U32(rec, 0xec) = 0;
    U32(rec, 0xf0) = 0xffffffffu;
    U32(rec, 0xf4) = 0;
    U32(rec, 0xf8) = 0;
    U32(rec, 0xfc) = 0;
    U32(rec, 0x100) = 0;
    U32(rec, 0xe4) = 0;
    U32(rec, 0xe0) = 0;
    U32(rec, 0x104) = 0;
    U32(rec, 0x110) = 0;
    U32(rec, 0x114) = 0;
    U32(rec, 0x118) = 0xffffffffu;
    U32(rec, 0x11c) = 0;
    U32(rec, 0x120) = 0;
    Mem_Fill(BYTES(rec) + 0x124, 0, 0x16);
    U32(rec, 0x150) = 0x80000000u;
}
