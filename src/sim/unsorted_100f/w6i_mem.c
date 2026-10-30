#include "w6i.h"
#include "w2h.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1010/w2h.h"
#include <string.h>

typedef void *(__stdcall *W6iHeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t size);
typedef int(__stdcall *W6iHeapFreeFn)(uint32_t heap, uint32_t flags, uint32_t block);

// FUNCTION: HGSIM 0x100fde70
void __cdecl Sort_SelectionSort(uint8_t *lo, uint8_t *hi, uint32_t width, W6iCompareFn compare)
{
    uintptr_t first = (uintptr_t)lo;
    uintptr_t last = (uintptr_t)hi;

    while (last > first) {
        uintptr_t largest = first;
        uintptr_t p;

        for (p = first + width; p <= last; p += width) {
            if (compare((const void *)p, (const void *)largest) > 0)
                largest = p;
        }
        Mem_Swap((uint8_t *)largest, (uint8_t *)last, width);
        last -= width;
    }
}

// FUNCTION: HGSIM 0x100fdf00
void *__cdecl Mem_Copy(void *dst, const void *src, uint32_t n)
{
    return memmove(dst, src, n);
}

// FUNCTION: HGSIM 0x100fef30
void *__cdecl Heap_Malloc(uint32_t size)
{
    return W6I_FN_100FEF50(size, G32(0x1017e968));
}

// FUNCTION: HGSIM 0x100fefa0
void *__cdecl Heap_AllocBlock(uint32_t size)
{
    uint32_t rounded = (size + 0xf) & ~0xfu;

    if (rounded <= G32(0x1015be5c)) {
        void *block = W6I_FN_101069C0(rounded >> 4);

        if (block != 0)
            return block;
    }
    return SIM_API(W6iHeapAllocFn, 0x10ac5658)(G32(0x10ac3714), 0, rounded);
}

// FUNCTION: HGSIM 0x100fefe0
void __cdecl Heap_FreeBlock(uint32_t block)
{
    uint32_t node;
    uint32_t page;
    uint32_t entry;

    if (block == 0)
        return;
    entry = Sim_10106900(block, &node, &page);
    if (entry != 0)
        W6I_FN_10106960(node, page, entry);
    else
        SIM_API(W6iHeapFreeFn, 0x10ac5634)(G32(0x10ac3714), 0, block);
}
