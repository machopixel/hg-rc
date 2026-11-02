/*
 * HGSIM.DLL's heap imports on our heaps (src/sim/crt/hg_winheap.c; DECISIONS.md, "Windows calls: memory"). The
 * DLL's import slots for HeapCreate / HeapAlloc / HeapReAlloc / HeapFree / HeapSize / HeapValidate / HeapDestroy
 * point here, so the original code still in use and the converted code share the same heaps. A heap made before
 * the slots were changed (the DLL's own C runtime heap, made while it loads) is still Windows': its calls go on to
 * Windows' functions. "noconv" leaves the slots alone.
 */
#include <windows.h>

#include "../launcher/debug.h"
#include "../sim/crt/hg_winheap.h"
#include "hook.h"

typedef HANDLE(WINAPI *HeapCreateFn)(DWORD, SIZE_T, SIZE_T);
typedef BOOL(WINAPI *HeapDestroyFn)(HANDLE);
typedef LPVOID(WINAPI *HeapAllocFn)(HANDLE, DWORD, SIZE_T);
typedef LPVOID(WINAPI *HeapReAllocFn)(HANDLE, DWORD, LPVOID, SIZE_T);
typedef BOOL(WINAPI *HeapFreeFn)(HANDLE, DWORD, LPVOID);
typedef SIZE_T(WINAPI *HeapSizeFn)(HANDLE, DWORD, LPCVOID);
typedef BOOL(WINAPI *HeapValidateFn)(HANDLE, DWORD, LPCVOID);

static HeapDestroyFn g_destroy;
static HeapAllocFn g_alloc;
static HeapReAllocFn g_realloc;
static HeapFreeFn g_free;
static HeapSizeFn g_size;
static HeapValidateFn g_validate;

#define OURS(heap) Hg_HeapOwns((uint32_t)(uintptr_t)(heap))

static HANDLE WINAPI StandInHeapCreate(DWORD options, SIZE_T initial, SIZE_T maximum)
{
    return (HANDLE)(uintptr_t)Hg_HeapCreate(options, (uint32_t)initial, (uint32_t)maximum);
}

static BOOL WINAPI StandInHeapDestroy(HANDLE heap)
{
    return OURS(heap) ? Hg_HeapDestroy((uint32_t)(uintptr_t)heap) : g_destroy(heap);
}

static LPVOID WINAPI StandInHeapAlloc(HANDLE heap, DWORD flags, SIZE_T bytes)
{
    return OURS(heap) ? Hg_HeapAlloc((uint32_t)(uintptr_t)heap, flags, (uint32_t)bytes) : g_alloc(heap, flags, bytes);
}

static LPVOID WINAPI StandInHeapReAlloc(HANDLE heap, DWORD flags, LPVOID block, SIZE_T bytes)
{
    return OURS(heap) ? Hg_HeapReAlloc((uint32_t)(uintptr_t)heap, flags, block, (uint32_t)bytes)
                      : g_realloc(heap, flags, block, bytes);
}

static BOOL WINAPI StandInHeapFree(HANDLE heap, DWORD flags, LPVOID block)
{
    return OURS(heap) ? Hg_HeapFree((uint32_t)(uintptr_t)heap, flags, block) : g_free(heap, flags, block);
}

static SIZE_T WINAPI StandInHeapSize(HANDLE heap, DWORD flags, LPCVOID block)
{
    return OURS(heap) ? Hg_HeapSize((uint32_t)(uintptr_t)heap, flags, block) : g_size(heap, flags, block);
}

static BOOL WINAPI StandInHeapValidate(HANDLE heap, DWORD flags, LPCVOID block)
{
    return OURS(heap) ? Hg_HeapValidate((uint32_t)(uintptr_t)heap, flags, block) : g_validate(heap, flags, block);
}

void HooksHeap_OnSimLoaded(HMODULE sim)
{
    HMODULE kernel = GetModuleHandleA("kernel32.dll");
    int n = 0;

    if (Hooks_ConfigWord("noconv") || kernel == NULL)
        return;
    g_destroy = (HeapDestroyFn)(void *)GetProcAddress(kernel, "HeapDestroy");
    g_alloc = (HeapAllocFn)(void *)GetProcAddress(kernel, "HeapAlloc");
    g_realloc = (HeapReAllocFn)(void *)GetProcAddress(kernel, "HeapReAlloc");
    g_free = (HeapFreeFn)(void *)GetProcAddress(kernel, "HeapFree");
    g_size = (HeapSizeFn)(void *)GetProcAddress(kernel, "HeapSize");
    g_validate = (HeapValidateFn)(void *)GetProcAddress(kernel, "HeapValidate");
    if (g_destroy == NULL || g_alloc == NULL || g_realloc == NULL || g_free == NULL || g_size == NULL ||
        g_validate == NULL) {
        DebugPrintf("HGSIM.DLL: heap imports left to Windows (kernel32 lookup failed)\n");
        return;
    }
    /* the forwarding functions first, HeapCreate last: no heap of ours exists before every slot is ours */
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapDestroy", (void *)StandInHeapDestroy);
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapAlloc", (void *)StandInHeapAlloc);
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapReAlloc", (void *)StandInHeapReAlloc);
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapFree", (void *)StandInHeapFree);
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapSize", (void *)StandInHeapSize);
    n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapValidate", (void *)StandInHeapValidate);
    if (n == 6)
        n += Hook_PatchImport(sim, "KERNEL32.dll", "HeapCreate", (void *)StandInHeapCreate);
    DebugPrintf("HGSIM.DLL: %d of 7 heap imports on our heaps%s\n", n,
                n == 7 ? "" : " (HeapCreate left to Windows, so every heap stays Windows')");
}
