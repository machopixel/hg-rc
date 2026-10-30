/*
 * Round 4, letter w (area 0x10074xxx..0x1007fxxx): a list of 0x18 byte entries, and the display window functions that
 * send WM_COPYDATA (0x4a) messages to the window 0x1017b35c or draw the pixel buffer 0x1015e9a0 into the window.
 */
#ifndef HG_SIM_UNSORTED_1007_W4W_H
#define HG_SIM_UNSORTED_1007_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Appends a new zeroed 0x18 byte entry holding `value` at +0 to the list with head dword 0x1015dc14, tail dword
 * 0x1015dc10 and next pointer at +0x14. Returns 1, or 0 when the allocation fails.
 */
int __cdecl Sim_10074b80(uint32_t value);

/*
 * Sends WM_COPYDATA (dwData 0x02400401, cbData dword 0x109e3ce8 when dword 0x1015e9a0 is not 0, lpData dword 0x1015e9a0)
 * to the window in dword 0x1017b35c, keeps the result in dword 0x10372d8c and reports a non-zero result with
 * Sim_10083495(0x10149fd4, result). Clears the dword that dword 0x1015e698 points to. Returns the stored result.
 */
uint32_t __cdecl Sim_1007ef27(void);

/*
 * StretchBlt of the source rectangle (x1, y1)-(x2, y2) of the source DC 0x1017b364 into the DC 0x1017b368 at (0,0) with
 * size (dword 0x109e3d14, dword 0x109e3d18); the failure code of GetLastError goes to Sim_10083495(0x10149ff4, code).
 * Then does what Sim_1007ef27 does with the pointer dword 0x1017b374 instead of 0x1015e9a0 and the string 0x1014a008.
 * Returns the result of that last message.
 */
uint32_t __cdecl Sim_1007efb4(int x1, int y1, int x2, int y2);

/*
 * Copies `count` RGB triples to the table at 0x1015e6a0 starting at entry `first`, writes each component << 2 to the
 * 4 byte entries at 0x109e38e8 and sends them to the window as WM_COPYDATA (0x400, 0x428 bytes at 0x109e38c0).
 * Returns -1 when dword 0x1017b378 is 0, the range is wrong (first 0..255, count 1..256-first) or the message result is
 * not 0 (reported with Sim_10083495(0x1014a03c, result)), else 0.
 */
int __cdecl Sim_1007f0c8(int first, int count, const uint8_t *rgb);

#endif
