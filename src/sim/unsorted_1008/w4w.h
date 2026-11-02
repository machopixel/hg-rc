/*
 * Round 4, letter w (area 0x10083xxx..0x1008cxxx): the device context and pixel buffer of the display window, drawing
 * the buffer, and releasing a record of the display driver.
 */
#ifndef HG_SIM_UNSORTED_1008_W4W_H
#define HG_SIM_UNSORTED_1008_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Releases the window drawing state: dword 0x1017b7d8 = 0; when the DC in dword 0x1017b3c0 is set, selects the old object
 * (dword 0x103729dc) back, deletes the bitmap in 0x1017b3c4 (when set), frees the pixel buffer 0x1015e9a0 (when set) and
 * releases the DC for the window 0x1017b8f0. Clears the dwords 0x1017b3c0, 0x1017b3c4, 0x1017b7c8, 0x1017b7cc,
 * 0x1017b7d0, 0x1015e9a0, the dword the pointer 0x1015e698 points to, and 0x1017b7d4. Returns 0.
 */
int __cdecl Sim_10083692(void);

/*
 * SetDIBitsToDevice of the rectangle (x1, y1)-(x2, y2) of the pixel buffer 0x1015e9a0 (bitmap info 0x109e38c0, DC 0x1017b3c0)
 * to (x1, y1 - GetSystemMetrics(15)). The result goes to dword 0x103729c4; a result of 0 reports GetLastError with
 * Sim_10083495(0x1014afd0, code). Returns 0 when the result equals the scan line count dword 0x109e3d18, else -1.
 */

/*
 * Frees the block in dword +4 of the record (when set) and clears it, then calls the function at +0x18 of the display
 * driver table that dword 0x1015e688 points to with the dword +0 of the record. Returns what that function returns.
 */
int __cdecl Sim_1008c6b6(void *record);

#endif
