#include "fns.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../unsorted_1004/fns.h"

// FUNCTION: HGSIM 0x100100ca
int32_t __cdecl Sim_100100ca(int32_t value)
{
    int32_t result = -1;

    if (value < 8) {
        int index = Sim_10015c61(value);

        if (index != -1) {
            uint32_t block = PTRTAB(index) + 0x19c;

            if (block != 0)
                result = S16(block, 0xa);
        }
    }
    return result + 1;
}

// FUNCTION: HGSIM 0x10010870
void *__cdecl Sim_10010870(uint32_t unused1, const void *arg, uint32_t unused3, uint32_t unused4, uint32_t unused5)
{
    uint32_t index = (uint32_t)Sim_10015c61(S32(arg, 0xc));
    uint32_t record;
    int16_t word;
    int16_t low;

    if (index >= G32(0x1015dc64))
        return 0;
    record = PTRTAB(index);
    if (S16(record, 0x1a6) == 5)
        return 0;
    word = S16(record, 0x1aa);
    low = (int16_t)(word & 0xff);
    switch (word & 0xf00) {
    case 0x100:
        return SIM_PTR(uint8_t, 0x109a8668) + (int32_t)low * 0x54;
    case 0x400:
        return SIM_PTR(uint8_t, 0x109e9bb8) + (int32_t)low * 0x50;
    case 0x200:
        return BYTES(PTRTAB((int32_t)low) + 0x124);
    default:
        return 0;
    }
}

// FUNCTION: HGSIM 0x100140df
int32_t __cdecl Sim_100140df(const void *obj)
{
    uint32_t record = PTRTAB(Sim_10052aaf(S32(obj, 8)));
    uint32_t owner = U32(record, 0x28);

    return (U16(owner, 0x562) & 1) != 0;
}
