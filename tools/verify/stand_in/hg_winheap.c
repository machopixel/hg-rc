/* Checker stand-in for src/sim/crt/hg_winheap.c: what the converted code called before (Windows' heap functions
 * through HGSIM.DLL's import slots, which the specs answer), so the specs compare the code around them.
 * hg_winheap.c itself is standard C (DECISIONS.md, "Windows calls: memory"). */
#include <stdint.h>
#include "../../../src/sim/simmem.h"
#include "../../../src/sim/crt/hg_winheap.h"

typedef uint32_t(__stdcall *HgCreateFn)(uint32_t, uint32_t, uint32_t);
typedef int(__stdcall *HgDestroyFn)(uint32_t);
typedef void *(__stdcall *HgAllocFn)(uint32_t, uint32_t, uint32_t);
typedef void *(__stdcall *HgReAllocFn)(uint32_t, uint32_t, void *, uint32_t);
typedef int(__stdcall *HgFreeFn)(uint32_t, uint32_t, void *);
typedef uint32_t(__stdcall *HgSizeFn)(uint32_t, uint32_t, const void *);
typedef int(__stdcall *HgValidateFn)(uint32_t, uint32_t, const void *);

uint32_t Hg_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum)
{
    return SIM_API(HgCreateFn, 0x10ac5614)(options, initial, maximum);
}

int Hg_HeapDestroy(uint32_t heap)
{
    return SIM_API(HgDestroyFn, 0x10ac5608)(heap);
}

void *Hg_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes)
{
    return SIM_API(HgAllocFn, 0x10ac5658)(heap, flags, bytes);
}

void *Hg_HeapReAlloc(uint32_t heap, uint32_t flags, void *block, uint32_t bytes)
{
    return SIM_API(HgReAllocFn, 0x10ac5660)(heap, flags, block, bytes);
}

int Hg_HeapFree(uint32_t heap, uint32_t flags, void *block)
{
    return SIM_API(HgFreeFn, 0x10ac5634)(heap, flags, block);
}

uint32_t Hg_HeapSize(uint32_t heap, uint32_t flags, const void *block)
{
    return SIM_API(HgSizeFn, 0x10ac5638)(heap, flags, block);
}

int Hg_HeapValidate(uint32_t heap, uint32_t flags, const void *block)
{
    return SIM_API(HgValidateFn, 0x10ac56a4)(heap, flags, block);
}

int Hg_HeapOwns(uint32_t heap)
{
    return heap != 0;
}
