#include "w4v.h"
#include "../util/access.h"
#include "../win/w4v.h"
#include "../view/view.h"
#include "../unsorted_1007/w2j.h"

// FUNCTION: HGSIM 0x100a9017
int __cdecl Sim_100a9017(uint32_t unused, void *owner)
{
    uint8_t *rec;
    uint32_t first, second, third;
    int i;

    (void)unused;
    U32(owner, 0x28) = 0;
    rec = Heap_AllocRaw((uint32_t)Sim_10072ff2());
    if (rec == 0)
        return 0;
    first = (uint32_t)(uintptr_t)rec + 0xc51;
    second = first + 0x940;
    third = second + 0x3fc;
    U32(rec, 0x84) = first;
    U32(rec, 0x88) = second;
    U32(rec, 0x8c) = third;
    U32(rec, 0) = (uint32_t)(uintptr_t)owner;
    for (i = 0; i < 10; i++)
        U32(rec, 0x498 + 4 * i) = 0;
    U32(owner, 0x28) = (uint32_t)(uintptr_t)rec;
    U32(owner, 0x2c) = 0xc51;
    Sim_10072e35(rec);
    return 1;
}
