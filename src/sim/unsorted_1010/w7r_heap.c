#include <string.h>
#include "w7r.h"
#include "w2h.h"
#include "w2i.h"
#include "w6j.h"
#include "w6k.h"
#include "../unsorted_100f/w6i.h"
#include "../util/access.h"

typedef uint32_t(__stdcall *W7rHeapAllocFn)(uint32_t heap, uint32_t flags, uint32_t size);
typedef uint32_t(__stdcall *W7rHeapReAllocFn)(uint32_t heap, uint32_t flags, uint32_t block, uint32_t size);

#define W7R_HEAP G32(0x10ac3714)
#define W7R_SMALL_LIMIT G32(0x1015be5c)

/* Copies min(paragraphs of the old small block * 16, size) bytes to the new block and frees the old one. */
static void w7r_move_small(uint32_t to, uint32_t from, uint32_t size, uint32_t node, uint32_t page, uint32_t map)
{
    uint32_t n = (uint32_t)U8(map, 0) << 4;

    if (n >= size)
        n = size;
    memcpy((void *)(uintptr_t)to, (const void *)(uintptr_t)from, n);
    Sim_10106960(node, page, BYTES(map));
}

// FUNCTION: HGSIM 0x101003e0
void *__cdecl Heap_Realloc(void *block, uint32_t size)
{
    uint32_t p = (uint32_t)(uintptr_t)block;
    uint32_t node, page, map, q;

    if (p == 0)
        return Heap_Malloc(size);
    if (size == 0) {
        Heap_FreeBlock(p);
        return 0;
    }
    if (size <= 0xffffffe0u)
        size = (size + 0xf) & ~0xfu;
    for (;;) {
        q = 0;
        if (size <= 0xffffffe0u) {
            map = Sim_10106900(p, &node, &page);
            if (map != 0) {
                if (size < W7R_SMALL_LIMIT) {
                    if (Sim_10106d80((void *)(uintptr_t)node, (void *)(uintptr_t)page, BYTES(map), size >> 4) != 0) {
                        q = p;
                    } else {
                        q = W7R_FN_101069C0(size >> 4);
                        if (q != 0)
                            w7r_move_small(q, p, size, node, page, map);
                    }
                }
                if (q == 0) {
                    q = SIM_API(W7rHeapAllocFn, 0x10ac5658)(W7R_HEAP, 0, size);
                    if (q != 0)
                        w7r_move_small(q, p, size, node, page, map);
                }
            } else {
                q = SIM_API(W7rHeapReAllocFn, 0x10ac5660)(W7R_HEAP, 0, p, size);
            }
        }
        if (q != 0 || G32(0x1017e968) == 0)
            return (void *)(uintptr_t)q;
        if (Sim_10106590(size) == 0)
            return 0;
    }
}

// FUNCTION: HGSIM 0x10100580
void *__cdecl Heap_Calloc(uint32_t count, uint32_t size)
{
    uint32_t n = count * size;
    uint32_t q;

    if (n <= 0xffffffe0u)
        n = n != 0 ? (n + 0xf) & ~0xfu : 0x10;
    for (;;) {
        q = 0;
        if (n <= 0xffffffe0u) {
            if (n <= W7R_SMALL_LIMIT) {
                q = W7R_FN_101069C0(n >> 4);
                if (q != 0)
                    memset((void *)(uintptr_t)q, 0, n);
            }
            if (q == 0)
                q = SIM_API(W7rHeapAllocFn, 0x10ac5658)(W7R_HEAP, 8, n);
        }
        if (q != 0 || G32(0x1017e968) == 0)
            return (void *)(uintptr_t)q;
        if (Sim_10106590(n) == 0)
            return 0;
    }
}
