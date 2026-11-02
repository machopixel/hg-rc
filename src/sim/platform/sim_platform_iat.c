/* The Windows backend of src/platform/sim_platform.h: each call goes through HGSIM.DLL's own import slot. */
#include "../../platform/sim_platform.h"
#include "../simmem.h"
#include "../crt/hg_winheap.h"

#ifndef _WIN32
#define __stdcall
#endif

/* the stdcall shapes of the imported functions */
typedef uint32_t(__stdcall *PlatFn1)(uint32_t, uint32_t);
typedef uint32_t(__stdcall *PlatFn2)(uint32_t);
typedef void(__stdcall *PlatFn3)(void *);
typedef int32_t(__stdcall *PlatFn6)(const char *, const char *);

uint32_t __cdecl SimPlat_GlobalAlloc(uint32_t flags, uint32_t bytes)
{
    return SIM_API(PlatFn1, 0x10ac5624)(flags, bytes);
}

uint32_t __cdecl SimPlat_GlobalSize(uint32_t mem)
{
    return SIM_API(PlatFn2, 0x10ac5620)(mem);
}

uint32_t __cdecl SimPlat_GlobalFree(uint32_t mem)
{
    return SIM_API(PlatFn2, 0x10ac560c)(mem);
}

void __cdecl SimPlat_GetSystemTime(void *systemTime)
{
    SIM_API(PlatFn3, 0x10ac561c)(systemTime);
}

void __cdecl SimPlat_GlobalMemoryStatus(void *status)
{
    SIM_API(PlatFn3, 0x10ac5618)(status);
}

/* the game's heaps: our own (src/sim/crt/hg_winheap.c), no longer Windows' */
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

int32_t __cdecl SimPlat_SetEnvironmentVariable(const char *name, const char *value)
{
    return SIM_API(PlatFn6, 0x10ac5610)(name, value);
}

/* GetSystemMetrics, SendMessageA, ShowCursor, GetDC, ReleaseDC, GetDeviceCaps, PeekMessageA and MessageBoxA are the
   platform layer's now (platform/plat.h; DECISIONS.md, "Windows calls: the screen"). */
