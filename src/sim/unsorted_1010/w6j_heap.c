#include "w6j.h"
#include "../util/access.h"

typedef uint32_t(__stdcall *W6jHeapCreateFn)(uint32_t options, uint32_t initial, uint32_t maximum);
typedef int(__stdcall *W6jHeapDestroyFn)(uint32_t heap);
typedef int(__stdcall *W6jVirtualFreeFn)(uint32_t address, uint32_t size, uint32_t type);

#define IAT_HEAPCREATE 0x10ac5614u
#define IAT_HEAPDESTROY 0x10ac5608u
#define IAT_VIRTUALFREE 0x10ac56a8u

// FUNCTION: HGSIM 0x10106590
int __cdecl Sim_10106590(uint32_t value)
{
    W6jCallbackFn fn = (W6jCallbackFn)(uintptr_t)G32(0x1017e96c);

    if (fn != 0 && fn(value) != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x101065b0
int __cdecl Sim_101065b0(void)
{
    uint32_t heap = SIM_API(W6jHeapCreateFn, IAT_HEAPCREATE)(1, 0x1000, 0);

    G32(0x10ac3714) = heap;
    if (heap == 0)
        return 0;
    if (W6J_REGION_NEW() == 0) {
        SIM_API(W6jHeapDestroyFn, IAT_HEAPDESTROY)(G32(0x10ac3714));
        return 0;
    }
    return 1;
}

// FUNCTION: HGSIM 0x101065f0
void __cdecl Sim_101065f0(void)
{
    uint8_t *head = SIM_PTR(uint8_t, 0x10159e38);
    uint8_t *rec = head;

    uint32_t block;

    do {
        block = U32(rec, 0x10);
        if (block != 0)
            SIM_API(W6jVirtualFreeFn, IAT_VIRTUALFREE)(block, 0, 0x8000);
        rec = BYTES(U32(rec, 0));
    } while (rec != head);
    SIM_API(W6jHeapDestroyFn, IAT_HEAPDESTROY)(G32(0x10ac3714));
}
