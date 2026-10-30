#include "w8z.h"
#include "w2e.h"
#include "w4v.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100f/w2e.h"

/*
 * The loader of Sim_100da805 / Sim_100daa08: `tag` is the value the resource's dword +0 must have, `size` the
 * record size, `at` the offset of the dword and name inside a record, the four extension strings, the list and
 * the counter.
 */
static int W8z_LoadList(const char *name, uint32_t tag, uint32_t size, uint32_t at, uint32_t ext0, uint32_t ext1,
                        uint32_t ext2, uint32_t ext3, uint32_t list, uint32_t counter)
{
    int loaded = 0;
    int32_t count = 0;
    uint32_t out = 0;
    uint32_t res;
    uint32_t src;
    uint32_t node;
    uint32_t copy;
    int32_t i;

    if (G32(0x1017e6bc) == 0 || G32(0x1017e6c0) == 0)
        Sim_100da45c();
    res = W8Z_CALL_100ba7d5_D(name, (const char *)SIM_PTR(char, ext0), &out);
    if (res != 0) {
        if (U32(res, 0) != tag) {
            Sim_100bb2b0((const uint32_t *)name, (const char *)SIM_PTR(char, ext1));
            W8Z_CALL_100b9203(name, (const char *)SIM_PTR(char, ext2));
            return 0;
        }
        count = S32(res, 0x28);
        src = res + 0x2c;
        for (i = 0; i < count; i++) {
            node = (uint32_t)(uintptr_t)W8Z_HEAP_ALLOC(W8Z_HEAP, 1, 0x38);
            Mem_Fill((void *)(uintptr_t)node, 0, 0x38);
            copy = (uint32_t)(uintptr_t)W8Z_HEAP_ALLOC(W8Z_HEAP, 1, size);
            Mem_Move((void *)(uintptr_t)copy, (const void *)(uintptr_t)src, size);
            U32(node, 0x34) = copy;
            Mem_Move(BYTES(node) + 0x10, BYTES(copy) + at, 4);
            Str_Copy((char *)BYTES(node) + 0x14, (const char *)BYTES(copy) + at);
            if (Sim_100da4f7((void *)(uintptr_t)node, SIM_PTR(uint8_t, list)) == 0) {
                W8Z_HEAP_FREE(W8Z_HEAP, 1, (void *)(uintptr_t)node);
                W8Z_HEAP_FREE(W8Z_HEAP, 1, (void *)(uintptr_t)copy);
                *SIM_PTR(u32a, counter) -= 1;
            }
            src += size;
        }
        loaded = 1;
        Sim_100bb2b0((const uint32_t *)name, (const char *)SIM_PTR(char, ext3));
        W8Z_CALL_100b9203(name, (const char *)SIM_PTR(char, ext3 + 4));
    }
    *SIM_PTR(u32a, counter) += (uint32_t)count;
    return loaded;
}

// FUNCTION: HGSIM 0x100da805
int __cdecl Sim_100da805(const char *name)
{
    return W8z_LoadList(name, 0, 0x60, 0, 0x101524a8, 0x101524ac, 0x101524b0, 0x101524b4, 0x1017e6d0, 0x1017e6c4);
}

// FUNCTION: HGSIM 0x100daa08
int __cdecl Sim_100daa08(const char *name)
{
    return W8z_LoadList(name, 1, 0x48, 0x20, 0x101524bc, 0x101524c0, 0x101524c4, 0x101524c8, 0x1017e740, 0x1017e6c8);
}
