/*
 * Round 4 (letter u): the Windows imports and DLL globals shared by the w4u_*.c files.
 * Every call goes through the DLL's own import slot (SIM_API), like the original.
 */
#ifndef HG_SIM_WIN_W4U_API_H
#define HG_SIM_WIN_W4U_API_H

#include <stdint.h>

#include "../simmem.h"
#include "../util/access.h"

typedef int(__stdcall *W4uHeapFreeFn)(void *heap, uint32_t flags, void *block);
typedef void *(__stdcall *W4uHeapAllocFn)(void *heap, uint32_t flags, uint32_t bytes);
typedef int(__stdcall *W4uFreeLibraryFn)(void *module);
typedef uint32_t(__stdcall *W4uMciSendFn)(uint32_t device, uint32_t message, uint32_t flags, void *parms);
typedef uint32_t(__stdcall *W4uAuxGetNumDevsFn)(void);
typedef uint32_t(__stdcall *W4uAuxGetDevCapsFn)(uint32_t device, void *caps, uint32_t size);
typedef int(__stdcall *W4uDeleteFn)(void *handle);
typedef int(__stdcall *W4uShowWindowFn)(void *window, int command);
typedef int(__stdcall *W4uSendMessageFn)(void *window, uint32_t message, uint32_t wparam, void *lparam);
typedef uint32_t(__stdcall *W4uGetLastErrorFn)(void);
typedef uint32_t(__stdcall *W4uSetDIBColorTableFn)(void *dc, uint32_t start, uint32_t count, const void *colors);
typedef void *(__stdcall *W4uCreateDIBSectionFn)(void *dc, const void *info, uint32_t usage, void **bits,
                                                 void *section, uint32_t offset);
typedef void *(__stdcall *W4uSelectObjectFn)(void *dc, void *object);
typedef int(__stdcall *W4uSetDIBitsToDeviceFn)(void *dc, int xd, int yd, uint32_t w, uint32_t h, int xs, int ys,
                                               uint32_t scan, uint32_t lines, const void *bits, const void *info,
                                               uint32_t usage);
typedef void *(__stdcall *W4uCreatePaletteFn)(const void *logPalette);
typedef void *(__stdcall *W4uSelectPaletteFn)(void *dc, void *palette, int background);
typedef uint32_t(__stdcall *W4uSetPaletteEntriesFn)(void *palette, uint32_t start, uint32_t count, const void *entries);
typedef uint32_t(__stdcall *W4uRealizePaletteFn)(void *dc);
typedef void(__cdecl *W4uLogFn)(const void *format, uint32_t value);

#define W4U_IAT_HEAPFREE 0x10ac5634u
#define W4U_IAT_HEAPALLOC 0x10ac5658u
#define W4U_IAT_FREELIBRARY 0x10ac55d8u
#define W4U_IAT_MCISENDCOMMAND 0x10ac5850u
#define W4U_IAT_AUXGETNUMDEVS 0x10ac5830u
#define W4U_IAT_AUXGETDEVCAPS 0x10ac5858u
#define W4U_IAT_DELETEDC 0x10ac557cu
#define W4U_IAT_DELETEOBJECT 0x10ac5594u
#define W4U_IAT_SHOWWINDOW 0x10ac57dcu
#define W4U_IAT_SENDMESSAGE 0x10ac57b8u
#define W4U_IAT_GETLASTERROR 0x10ac5600u
#define W4U_IAT_SETDIBCOLORTABLE 0x10ac5568u
#define W4U_IAT_CREATEDIBSECTION 0x10ac558cu
#define W4U_IAT_SELECTOBJECT 0x10ac5590u
#define W4U_IAT_SETDIBITSTODEVICE 0x10ac5570u
#define W4U_IAT_CREATEPALETTE 0x10ac5564u
#define W4U_IAT_SELECTPALETTE 0x10ac5574u
#define W4U_IAT_SETPALETTEENTRIES 0x10ac5584u
#define W4U_IAT_REALIZEPALETTE 0x10ac5588u

/* The game's heap handle (dword 0x1017b8f8), read at every call like the original. */
#define W4U_HEAP (*SIM_PTR(void *, 0x1017b8f8))

/* HeapFree(heap, 1, block) and HeapAlloc(heap, 1, bytes): flag 1 only, the memory is not zeroed. */
static inline int W4u_HeapFree(uint32_t block)
{
    return SIM_API(W4uHeapFreeFn, W4U_IAT_HEAPFREE)(W4U_HEAP, 1, (void *)(uintptr_t)block);
}

static inline uint32_t W4u_HeapAlloc(uint32_t bytes)
{
    return (uint32_t)(uintptr_t)SIM_API(W4uHeapAllocFn, W4U_IAT_HEAPALLOC)(W4U_HEAP, 1, bytes);
}

/* Sim_10083495(format, value): an empty function in this build (it was the debug print); still called, like the original. */
#define W4U_LOG(format_address, value) \
    SIM_FN(W4uLogFn, 0x10083495)(SIM_PTR(void, format_address), (uint32_t)(value))

#endif
