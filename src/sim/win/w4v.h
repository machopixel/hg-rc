/*
 * Round 4, letter v: small functions of HGSIM.DLL that call Windows through the DLL's import slots (heap,
 * cursor, GDI, library loading, the force feedback library). Plain C, original cdecl convention.
 */
#ifndef HG_SIM_WIN_W4V_H
#define HG_SIM_WIN_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* HeapAlloc(game heap, 1, size): a block that is NOT zero-filled (flag 1 is HEAP_NO_SERIALIZE). Returns the block or NULL. */
void *__cdecl Heap_AllocRaw(uint32_t size);
/* HeapFree(game heap, 1, block) with no NULL check; returns what HeapFree returns. */
int __cdecl Heap_FreeRaw(void *block);
/* Heap_AllocZero(size) (another copy of the call in the original). */
void *__cdecl Sim_100a77ef(uint32_t size);

/* ClipCursor(NULL) and clears the dword at 0x10179d64. Returns 0. (src/hooks/hooks_input.c patches it: MouseUnclip.) */
int __cdecl Mouse_Unclip(void);
/*
 * Two identical functions: StretchDIBits(dc [0x1017b3c0], 0, 0, [0x109e3d14], [0x109e3d18], x0, [0x109e3d18] - y1 - 1,
 * x1 - x0 + 1, y1 - y0 + 1, bits [0x1015e9a0], info 0x109e38c0, 1, SRCCOPY): copies the rectangle (x0,y0)-(x1,y1)
 * of the bottom-up bitmap to the top-left of the DC. The result is stored in [0x103729c4]; returns 0 when it
 * equals the height [0x109e3d18], else -1.
 */
int __cdecl Sim_10083de1(int x0, int y0, int x1, int y1);
int __cdecl Sim_10083e4e(int x0, int y0, int x1, int y1);

/*
 * Loads the libraries of the table at 0x1017dfb8 (entries 1..[0x1017e1bc], 0x20 bytes each: name, handle at +0x18,
 * the address of its export "Uncompress" at +0x1c). A library that fails to load is retried after Sleep(2000), for ever.
 * Returns 1.
 */
int __cdecl Sim_100bcc50(void);
/*
 * Calls the force feedback library's TSFF_ExitSystem through the pointer at 0x1015ef44. Returns 0 when the library
 * is not in use ([0x1015ef3c] == 0), 0x8004050f when the pointer is NULL, else the call's result (a negative one is
 * reported with OutputDebugStringA).
 */
int __cdecl Force_ExitSystem(void);

#endif
