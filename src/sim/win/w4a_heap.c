#include "w4a.h"
#include "../simmem.h"
#include "../crt/hg_winheap.h"
#include "../util/access.h"

typedef void *(__stdcall *LoadLibraryAFn)(const char *name);

#define IAT_LOADLIBRARYA 0x10ac55e4u

// FUNCTION: HGSIM 0x100092bd
void __cdecl Heap_Free(void *block)
{
    if (block != 0)
        Hg_HeapFree(G32(0x1017b8f8), 1, block);
}

// FUNCTION: HGSIM 0x10028559
void *__cdecl Heap_AllocZero(unsigned int size)
{
    return Hg_HeapAlloc(G32(0x1017b8f8), 9, size);
}

// FUNCTION: HGSIM 0x10027d5a
void __cdecl Sim_10027d5a(void)
{
    *SIM_PTR(void *, 0x1015dc30) = Hg_HeapAlloc(G32(0x1017b8f8), 9, 0xfc4);
}

// FUNCTION: HGSIM 0x1003c569
void *__cdecl Lib_Load_1003c569(const char *name)
{
    *SIM_PTR(void *, 0x1015eb68) = SIM_API(LoadLibraryAFn, IAT_LOADLIBRARYA)(name);
    return *SIM_PTR(void *, 0x1015eb68);
}
