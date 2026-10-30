#include "w2l.h"
#include "../util/access.h"
#include "../util/util.h"
#include "../unsorted_1000/w2l.h"

// FUNCTION: HGSIM 0x100b534b
void __cdecl Sim_100b534b(int index)
{
    uint8_t *entry = SIM_PTR(uint8_t, 0x101a2100u + (uint32_t)index * 0x88);

    if (U32(entry, 0x28) != 0) {
        Sim_10009787((void *)(uintptr_t)U32(entry, 0x28));
        U32(entry, 0x28) = 0;
    }
}

// FUNCTION: HGSIM 0x100b5bbb
uint32_t __cdecl Sim_100b5bbb(uint32_t unused)
{
    (void)unused;
    return G32(0x1015084c);
}

// FUNCTION: HGSIM 0x100bf23f
uint32_t __cdecl Sim_100bf23f(const void *table, int key)
{
    int16_t i;

    for (i = 0; i < S16(table, 0); i = (int16_t)(i + 1)) {
        uint32_t records = U32(table, 4);

        if ((int16_t)key == S16(records + (uint32_t)((int32_t)i * 0x12), 0))
            return (records & 0xffff0000u) | (uint16_t)i;
    }
    return ((uint32_t)(int32_t)S16(table, 0) & 0xffff0000u) | 0xffffu;
}

// FUNCTION: HGSIM 0x100bf0b8
uint32_t __cdecl Sim_100bf0b8(const void *obj)
{
    uint32_t table = U32(obj, 0x27c);
    uint32_t high;
    uint16_t result;

    if (S16(obj, 0x1fa) == -1) {
        int16_t index = (int16_t)Sim_1009c929((int)S16(table, 0) - (int)S16(table, 2));

        result = U16(U32(table, 4) + (uint32_t)((int32_t)index * 0x12), 0);
        high = table;
    } else {
        int16_t index = (int16_t)Sim_100bf23f((const void *)(uintptr_t)table, (int16_t)U16(obj, 0x1fa));
        uint32_t record;
        int picked;

        if (index == -1)
            index = 0;
        record = U32(table, 4) + (uint32_t)((int32_t)index * 0x12);
        picked = Sim_1009c929((int)S16(record, 2));
        result = U16(record + (uint32_t)picked * 2u + 4u, 0);
        high = (uint32_t)picked;
    }
    return (high & 0xffff0000u) | result;
}
