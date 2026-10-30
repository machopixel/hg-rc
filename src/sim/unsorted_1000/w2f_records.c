#include "w2f.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x10006db8
int __cdecl Sim_10006db8(int a, int b)
{
    uint8_t *e;

    if (b == -1) {
        b = GS32(0x1015cfe0);
        G32(0x1015cfe4) = 1;
    }
    if (b < 0 || b >= 0x200 || a < 0 || a >= 0x200)
        return 0;
    e = GBYTES(0x10ac1b00 + (uint32_t)a * 14);
    U16(e, 0) = (uint16_t)b;
    U16(e, 8) = 1;
    U16(e, 6) = 1;
    U16(e, 2) = 0;
    if (G32(0x1015cfe8) != 0)
        U16(e, 4) = 0x2d;
    return 1;
}

// FUNCTION: HGSIM 0x100093be
void __cdecl Sim_100093be(uint16_t *p, uint32_t v)
{
    if (p == 0)
        return;
    U16(p, 0) = (uint16_t)((v & 0x7ef0) | (U16(p, 0) & 0x810f) | 0x8000);
}

// FUNCTION: HGSIM 0x100095b6
void __cdecl Sim_100095b6(void *obj, int idx, uint32_t *o1, uint32_t *o2, uint32_t *o3)
{
    uint32_t rec = U32(obj, 0x30);

    if (rec == 0)
        return;
    rec += (uint32_t)idx * 0x58 + 0x18;
    if (o1 != 0)
        U32(o1, 0) = U32(rec, 0);
    if (o2 != 0)
        U32(o2, 0) = U32(rec, 4);
    if (o3 != 0)
        U32(o3, 0) = U32(rec, 8);
}

// FUNCTION: HGSIM 0x1000c6e3
int __cdecl Sim_1000c6e3(void *obj, int idx, int val)
{
    uint32_t rec;
    uint32_t entry;

    if (obj == 0)
        return 0;
    rec = U32(obj, 0x84) + (uint32_t)idx * 0x128;
    entry = 0x10145508 + U32(rec, 4) * 0xd6;
    return S32(GBYTES(entry), 0x10) == val;
}
