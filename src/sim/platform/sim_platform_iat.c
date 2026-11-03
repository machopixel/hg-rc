/* src/platform/sim_platform.h: the game's heaps, our own (src/sim/crt/hg_winheap.c). */
#include "../../platform/sim_platform.h"
#include "../simmem.h"
#include "../crt/hg_winheap.h"

#ifndef _WIN32
#define __stdcall
#endif

uint32_t __cdecl SimPlat_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum)
{
    return Hg_HeapCreate(options, initial, maximum);
}

uint32_t __cdecl SimPlat_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes)
{
    return (uint32_t)(uintptr_t)Hg_HeapAlloc(heap, flags, bytes);
}

int32_t __cdecl SimPlat_HeapDestroy(uint32_t heap)
{
    return Hg_HeapDestroy(heap);
}

/* GetSystemMetrics, SendMessageA, ShowCursor, GetDC, ReleaseDC, GetDeviceCaps, PeekMessageA and MessageBoxA are the
   platform layer's now (platform/plat.h; DECISIONS.md, "Windows calls: the screen"), and so are GlobalAlloc,
   GlobalSize, GlobalFree, GetSystemTime and GlobalMemoryStatus ("The last original code", step 2);
   SetEnvironmentVariableA is gone. */
