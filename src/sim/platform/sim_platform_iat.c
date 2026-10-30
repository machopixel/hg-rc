/* The Windows backend of src/platform/sim_platform.h: each call goes through HGSIM.DLL's own import slot. */
#include "../../platform/sim_platform.h"
#include "../simmem.h"

#ifndef _WIN32
#define __stdcall
#endif

/* the stdcall shapes of the imported functions */
typedef int32_t(__stdcall *PlatFn0)(int32_t);
typedef uint32_t(__stdcall *PlatFn1)(uint32_t, uint32_t);
typedef uint32_t(__stdcall *PlatFn2)(uint32_t);
typedef void(__stdcall *PlatFn3)(void *);
typedef uint32_t(__stdcall *PlatFn4)(uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PlatFn5)(uint32_t);
typedef int32_t(__stdcall *PlatFn6)(const char *, const char *);
typedef int32_t(__stdcall *PlatFn7)(uint32_t, uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PlatFn8)(uint32_t, uint32_t);
typedef int32_t(__stdcall *PlatFn9)(uint32_t, int32_t);
typedef int32_t(__stdcall *PlatFn10)(void *, uint32_t, uint32_t, uint32_t, uint32_t);
typedef int32_t(__stdcall *PlatFn11)(uint32_t, const char *, const char *, uint32_t);

int32_t __cdecl SimPlat_GetSystemMetrics(int32_t index)
{
    return SIM_API(PlatFn0, 0x10ac577c)(index);
}

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

uint32_t __cdecl SimPlat_HeapCreate(uint32_t options, uint32_t initial, uint32_t maximum)
{
    return SIM_API(PlatFn4, 0x10ac5614)(options, initial, maximum);
}

uint32_t __cdecl SimPlat_HeapAlloc(uint32_t heap, uint32_t flags, uint32_t bytes)
{
    return SIM_API(PlatFn4, 0x10ac5658)(heap, flags, bytes);
}

int32_t __cdecl SimPlat_HeapDestroy(uint32_t heap)
{
    return SIM_API(PlatFn5, 0x10ac5608)(heap);
}

int32_t __cdecl SimPlat_SetEnvironmentVariable(const char *name, const char *value)
{
    return SIM_API(PlatFn6, 0x10ac5610)(name, value);
}

int32_t __cdecl SimPlat_SendMessage(uint32_t window, uint32_t message, uint32_t wParam, uint32_t lParam)
{
    return SIM_API(PlatFn7, 0x10ac57b8)(window, message, wParam,
                                                                                         lParam);
}

int32_t __cdecl SimPlat_ShowCursor(int32_t show)
{
    return SIM_API(PlatFn0, 0x10ac5768)(show);
}

uint32_t __cdecl SimPlat_GetDC(uint32_t window)
{
    return SIM_API(PlatFn2, 0x10ac57ac)(window);
}

int32_t __cdecl SimPlat_ReleaseDC(uint32_t window, uint32_t dc)
{
    return SIM_API(PlatFn8, 0x10ac57a8)(window, dc);
}

int32_t __cdecl SimPlat_GetDeviceCaps(uint32_t dc, int32_t index)
{
    return SIM_API(PlatFn9, 0x10ac5580)(dc, index);
}

int32_t __cdecl SimPlat_PeekMessage(void *msg, uint32_t window, uint32_t first, uint32_t last, uint32_t remove)
{
    return SIM_API(PlatFn10, 0x10ac5780)(msg, window, first,
                                                                                                 last, remove);
}

int32_t __cdecl SimPlat_MessageBox(uint32_t window, const char *text, const char *caption, uint32_t type)
{
    return SIM_API(PlatFn11, 0x10ac57e4)(window, text,
                                                                                                 caption, type);
}
