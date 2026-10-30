#include "fns.h"
#include "../util/util.h"
#include "../util/access.h"
#include "../unsorted_1003/fns.h"
#include "../view/view.h"

// FUNCTION: HGSIM 0x100b3530
void __cdecl Sim_100b3530(uint32_t unused, uint32_t value)
{
    Sim_1003a52b(value);
}

// FUNCTION: HGSIM 0x100b53f0
void __cdecl Sim_100b53f0(int index, uint32_t *a, uint32_t *b, uint32_t *c)
{
    uint32_t entry, block, object;

    if (index > GS32(0x1017D218) || index < 0) {
        *c = 0;
        *b = 0;
        *a = 0;
        return;
    }
    entry = 0x101A2100u + (uint32_t)index * 0x88u;
    block = G32(entry + 0x28);
    if (block != 0) {
        *a = U32(block, 0x44);
        *b = U32(block, 0x48);
        *c = U32(block, 0x4c);
        return;
    }
    object = G32(entry + 0x2c);
    if (object != 0) {
        Sim_100a9cfb((void *)(uintptr_t)object, a, b, c);
        return;
    }
    *c = 0;
    *b = 0;
    *a = 0;
}

// FUNCTION: HGSIM 0x100beea2
int __cdecl Sim_100beea2(void *obj)
{
    int result = 0;

    if (S32(obj, 0x514) > 0) {
        if (Sim_1009c929((int)((0x14u - U32(obj, 0x514)) << 1)) == 0)
            result = 1;
    }
    return result;
}

// FUNCTION: HGSIM 0x100bf026
uint32_t __cdecl Sim_100bf026(void *obj)
{
    uint32_t high = (uint32_t)(uintptr_t)obj & 0xffff0000u; /* upper half of eax, see fns.h */
    uint32_t entry = 0;
    uint16_t result = 0xffff;

    if ((U16(obj, 0x1aa) & 0x200) != 0) {
        uint32_t index = U16(obj, 0x1aa) & 0xff;

        high = 0;
        entry = PTRTAB(index);
    }
    if (U32(obj, 0) != 1) {
        if (entry == 0) {
            result = 0;
        } else {
            high = 0;
            if (Sim_10079feb((int32_t)U32(entry, 0)) != 0) {
                if (Sim_10079feb((int32_t)U32(obj, 0)) == 0)
                    result = 2;
            }
        }
    }
    return high | result;
}
