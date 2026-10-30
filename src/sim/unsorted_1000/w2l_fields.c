#include "w2l.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x1000716a
void __cdecl Sim_1000716a(int index, unsigned value)
{
    uint32_t v = value & 0xffff;
    uint8_t *entry = SIM_PTR(uint8_t, 0x10ac1b00u + (uint32_t)((int32_t)(int16_t)index * 14));

    if (v >= 0x20)
        v = 0;
    if (S16(entry, 8) < 0)
        return;
    if ((int8_t)*SIM_PTR(uint8_t, 0x10a51ac0u + (uint32_t)((int32_t)S16(entry, 0) * 0x300) + v * 0x18) == 0)
        return;
    U16(entry, 2) = (uint16_t)v;
    U32(entry, 0xa) = 0;
}

// FUNCTION: HGSIM 0x1000944e
uint32_t __cdecl Sim_1000944e(const void *obj)
{
    return U16(obj, 0x26);
}

// FUNCTION: HGSIM 0x100096b2
void __cdecl Sim_100096b2(void *obj, int index, unsigned value)
{
    uint32_t p = U32(obj, 0x30);

    if (p != 0 && index < (int32_t)S16(p, 6))
        U16(p + U32(p, 8) + (uint32_t)index * 0x28, 0) = (uint16_t)value;
}

// FUNCTION: HGSIM 0x10009707
uint32_t __cdecl Sim_10009707(const void *p)
{
    return U16(p, 0) & 0x10f;
}

// FUNCTION: HGSIM 0x1000c813
int __cdecl Sim_1000c813(const void *obj, int index)
{
    uint32_t entry;
    uint8_t *record;
    int32_t kind;

    if (obj == 0)
        return 0;
    entry = U32(obj, 0x84) + (uint32_t)index * 0x128;
    record = SIM_PTR(uint8_t, 0x10145508u + U32(entry, 4) * 0xd6);
    kind = S32(record, 0x10);
    if (kind == 0x3d)
        return 1;
    if (kind > 0x41 && kind <= 0x43)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x1000ede4
void __cdecl Sim_1000ede4(void)
{
    uint32_t table = G32(0x1015d0a4);
    int i;

    if (table == 0)
        return;
    /* The loop of the original tests each dword and does nothing either way; the reads are kept so a bad pointer faults as before. */
    for (i = 0; i < 0x10; i++)
        (void)*(volatile u32a *)(BYTES(table) + 0x54 + (uint32_t)i * 4);
}
