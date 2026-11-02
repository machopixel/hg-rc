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

/*
 * Two identical functions: StretchDIBits(dc [0x1017b3c0], 0, 0, [0x109e3d14], [0x109e3d18], x0, [0x109e3d18] - y1 - 1,
 * x1 - x0 + 1, y1 - y0 + 1, bits [0x1015e9a0], info 0x109e38c0, 1, SRCCOPY): copies the rectangle (x0,y0)-(x1,y1)
 * of the bottom-up bitmap to the top-left of the DC. The result is stored in [0x103729c4]; returns 0 when it
 * equals the height [0x109e3d18], else -1.
 */
int __cdecl Sim_10083de1(int x0, int y0, int x1, int y1);

/*
 * Sets up the archive's unpackers, the table at 0x1017dfb8 (entries 1..[0x1017e1bc], 0x20 bytes each: the method's
 * name, a library handle at +0x18, its "Uncompress" at +0x1c). The original loaded each name as a DLL (retrying after
 * Sleep(2000) for ever); now "pkware.dll" and "zlib.dll" get our unpackers (util/unpack.h), the handle is 0, and any
 * other name an unpacker that always fails. Returns 1.
 */
int __cdecl Sim_100bcc50(void);
/*
 * Force feedback stop (FFORCE.DLL's TSFF_ExitSystem, now platform/force.h's Force_Exit). Returns 0 when force
 * feedback is not in use ([0x1015ef3c] == 0), 0x8004050f without a device (the original: FFORCE.DLL missing), else
 * Force_Exit's result (a negative one is reported as debug text).
 */
int __cdecl Force_ExitSystem(void);

#endif
