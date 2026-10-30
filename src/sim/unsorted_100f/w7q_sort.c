#include "w7q.h"
#include "w6i.h"
#include "w2h.h"
#include "../unsorted_1010/w6j.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100fdcc0
void __cdecl Crt_QSort(uint8_t *base, uint32_t count, uint32_t width, W7qCompareFn compare)
{
    uint8_t *loStack[30];
    uint8_t *hiStack[30];
    int depth = 0;
    uint8_t *lo;
    uint8_t *hi;
    uint8_t *l;
    uint8_t *h;
    uint32_t size;

    if (count < 2 || width == 0)
        return;
    lo = base;
    hi = base + (count - 1u) * width;
    for (;;) {
        size = (uint32_t)(hi - lo) / width + 1u;
        if (size <= 8) {
            Sort_SelectionSort(lo, hi, width, (W6iCompareFn)compare);
        } else {
            Mem_Swap(lo + (size >> 1) * width, lo, width);
            l = lo;
            h = hi + width;
            for (;;) {
                do {
                    l += width;
                } while (l <= hi && compare(l, lo) <= 0);
                do {
                    h -= width;
                } while (h > lo && compare(h, lo) >= 0);
                if (h < l)
                    break;
                Mem_Swap(l, h, width);
            }
            Mem_Swap(lo, h, width);
            if ((int32_t)(uint32_t)(h - lo - 1) >= (int32_t)(uint32_t)(hi - l)) {
                if (lo + width < h) {
                    loStack[depth] = lo;
                    hiStack[depth] = h - width;
                    depth++;
                }
                if (l < hi) {
                    lo = l;
                    continue;
                }
            } else {
                if (l < hi) {
                    loStack[depth] = l;
                    hiStack[depth] = hi;
                    depth++;
                }
                if (lo + width < h) {
                    hi = h - width;
                    continue;
                }
            }
        }
        depth--;
        if (depth < 0)
            return;
        lo = loStack[depth];
        hi = hiStack[depth];
    }
}

// FUNCTION: HGSIM 0x100fef50
void *__cdecl Heap_MallocRetry(uint32_t size, int retry)
{
    void *block;

    if (size > 0xffffffe0u)
        return 0;
    if (size == 0)
        size = 1;
    for (;;) {
        block = size <= 0xffffffe0u ? Heap_AllocBlock(size) : 0;
        if (block != 0 || retry == 0)
            return block;
        if (Sim_10106590(size) == 0)
            return 0;
    }
}
