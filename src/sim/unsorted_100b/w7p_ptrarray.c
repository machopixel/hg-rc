#include "w7p.h"
#include "../util/access.h"
#include "../unsorted_100f/w6i.h"

static void *create(uint32_t bytes, int capacity)
{
    uint8_t *arr = Heap_Malloc(0xc);
    int i;

    if (arr != 0) {
        U32(arr, 4) = 0;
        U32(arr, 8) = (uint32_t)(uintptr_t)Heap_Malloc(bytes);
        if (U32(arr, 8) == 0) {
            Heap_FreeBlock((uint32_t)(uintptr_t)arr);
            return 0;
        }
    }
    S32(arr, 0) = capacity;
    for (i = 0; i < S32(arr, 0); i++)
        U32(U32(arr, 8), (uint32_t)i * 4) = 0;
    return arr;
}

// FUNCTION: HGSIM 0x100bd560
void *__cdecl PtrArray_Create(void)
{
    return create(0x40, 0x10);
}

// FUNCTION: HGSIM 0x100bd5fc
void *__cdecl PtrArray_CreateSized(int n)
{
    return create((uint32_t)n << 2, n);
}

// FUNCTION: HGSIM 0x100bd85b
int __cdecl PtrArray_Remove(void *arr, uint32_t value, void(__cdecl *fn)(uint32_t))
{
    int found = 0;
    int i;

    (void)*(volatile u32a *)(uintptr_t)U32(arr, 8);
    for (i = 0; i < S32(arr, 4); i++) {
        if (U32(U32(arr, 8), (uint32_t)i * 4) == value) {
            if (fn != 0)
                fn(U32(U32(arr, 8), (uint32_t)i * 4));
            else if (U32(U32(arr, 8), (uint32_t)i * 4) != 0)
                Heap_FreeBlock(U32(U32(arr, 8), (uint32_t)i * 4));
            U32(U32(arr, 8), (uint32_t)i * 4) = 0;
            found = 1;
            break;
        }
    }
    if (found != 0) {
        for (; i < S32(arr, 4) - 1; i++)
            U32(U32(arr, 8), (uint32_t)i * 4) = U32(U32(arr, 8), (uint32_t)i * 4 + 4);
        S32(arr, 4) = S32(arr, 4) - 1;
    }
    return found;
}
