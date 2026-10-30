/*
 * Round 7, letter n: functions at 0x10083530..0x10088804. Helpers that are not converted yet (or that another
 * hook file patches) are called through their original address with SIM_FN and the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1008_W7N_H
#define HG_SIM_UNSORTED_1008_W7N_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted (in the w7m list): (the byte block 0x1014af28), see Sim_10083530. */
#define W7N8_CALL_100381d0 SIM_FN(int(__cdecl *)(void *), 0x100381d0)
/* Not converted: the C library sqrt (double argument, result in st(0)). */
#define W7N8_CALL_100fca04 SIM_FN(double(__cdecl *)(double), 0x100fca04)
/* Not converted: the C library sprintf (destination, format, ...). */
#define W7N8_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: (5) gives non-zero while that condition holds, see Sim_10086c32. */
#define W7N8_CALL_100766ca SIM_FN(int(__cdecl *)(int), 0x100766ca)
/* Patched by another hook file: (0x80, 0 / 1), see Sim_10086c32. */
#define W7N8_CALL_100e46d0 SIM_FN(int(__cdecl *)(int, int), 0x100e46d0)

/*
 * Sets up the 8-bit off-screen bitmap once ([0x1017b3c0] = GetDC of the window [0x1017b8f0]; 0 when it is already
 * set): unless the display driver [0x1015e688] is of kind 2 it is shut down (its +0x10), 0x100381d0(0x1014af28)
 * runs when [0x1015e9bc] differs from [0x1014af2c] and the driver becomes [0x1013a2a0]. Then the DIB header for
 * width x height, the identity colour table 0x109e38e8, the palette (-1 when Gdi_SetPalette fails) and a zeroed
 * pixel block from HeapAlloc: out = {pixels, width - 1, height - 1, 0x109e38c0, 0} (2 when the allocation fails).
 * Returns 1 after Sim_10083692 when Sim_10083c45 succeeds, else sets [0x1017b7d8] and returns 0.
 */
int __cdecl Sim_10083530(uint32_t *out, int width, int height);
/*
 * Normalises the vector (*x, *y, *z) with the C library sqrt when its length is above the float [0x1012b870]
 * (each component divided by the length rounded to float).
 */
void __cdecl Sim_10084cee(float *x, float *y, float *z);
/*
 * sprintf into 0x101655c0 of the heap check result (Heap_Check: -1 empty, -2 OK, -3 bad begin, -4 bad node,
 * -6 bad pointer, else unknown) with the text `where`; returns what sprintf returns.
 */
int __cdecl Heap_ReportCheck(const char *where);
/*
 * Switches between the paused and the running state ([0x1017b92c]): pausing hides the cursor, prints
 * "WinMain(3): pause_timer(false)" and calls 0x100e46d0(0x80, 0), Sim_1001f4d3 and Sim_1005571d; resuming shows the
 * cursor (unless 0x100766ca(5) or [0x1015e9bc] is 1), calls 0x100e46d0(0x80, 1), Sim_1001f4ba and Sim_1005570e.
 * [0x1017b930] / the word [0x1015fd4c] select Sim_1008b773 / Sim_1008b757 first.
 */
void __cdecl Sim_10086c32(void);
/*
 * The 9 x 9 grid cell (0x2c bytes at 0x1017ba28, 0x18c per column) of the object's position (from its object
 * +0x28, else +0x44 / +0x4c): column trunc(trunc((x + [0x1014b8e0]) / [0x1012b8e0]) + [0x1012b8e4]), row the same
 * with z and [0x1014b8e4]; both are stored at +0x8c / +0x90. Outside the grid the cell is 0x1017b9b0, for
 * objects with flag 0x1000 0x1017b9d8. An object whose first 4 name bytes (+0xa0) match "TL1M" (Sim_1010d240) is
 * entered in the cell (+0x28); with "BO" at +0xa6 it also calls Sim_10087b00(-x, -z). Returns the cell.
 */
uint8_t *__cdecl Sim_10088804(uint8_t *obj);

#endif
