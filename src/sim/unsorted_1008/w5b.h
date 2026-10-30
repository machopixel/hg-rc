/*
 * Round 5, letter b: functions around 0x10083c45..0x1008e687 that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1008_W5B_H
#define HG_SIM_UNSORTED_1008_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. */
#define SIM_CALL_10083801 SIM_FN(void(__cdecl *)(void), 0x10083801)
/* Stores the low 16 bits of value in the word table at 0x10156dcc, entry index. */
#define SIM_CALL_100f01cc SIM_FN(void(__cdecl *)(int, uint32_t), 0x100f01cc)
#define SIM_CALL_100eb6e8 SIM_FN(int(__cdecl *)(const void *, int, int, int, int, int), 0x100eb6e8)

/*
 * When [0x10179ca0] or [0x10179ca4] is set and [0x1017b92c] is 0 runs 0x10083801(); otherwise SetDIBitsToDevice of the
 * whole bitmap [0x1015e9a0] (size [0x109e3d14] by [0x109e3d18], info block 0x109e38c0) to the window's DC [0x1017b3c0],
 * result in [0x103729c4]. When that is 0 the last error is passed to the (empty) log function Sim_10083495 with the message
 * 0x1014af9c. Returns -1 unless all [0x109e3d18] lines were drawn, else 0.
 */
int __cdecl Sim_10083c45(void);
/*
 * Copies 256 RGB triples from rgb to 0x109b8c00, converts each with Sim_1004b804 into 0x1015e6a0, then Gdi_SetPalette(0, 0x100,
 * 0x1015e6a0) and Sim_10083c45(), and [[0x1015e698]] = [0x1015e9a0]. Returns 0.
 */
int __cdecl Sim_1008404b(const uint8_t *rgb);
/* For i = start .. start + count - 1 (signed): 0x100f01cc(i, (r >> 1) << 11 | g << 5 | b >> 1) with r, g, b the bytes rgb[3 * i ..]. Returns 0. */
int __cdecl Sim_100878bb(int start, int count, const uint8_t *rgb);
/* The same for i = 0 .. 0xff, with the triples at rgb. Returns 0. */
int __cdecl Sim_1008792c(const uint8_t *rgb);
/*
 * With the rectangle rect (dwords +4 +8 +0xc +0x10 = left, top, right, bottom): w2 = (right - left + 1) >> 1,
 * h2 = (bottom - top + 1) >> 1, radius = (right - left + 1) / 2 - 1 (truncating) and
 * scaled = (int)(radius * float[ [0x1014cae0] + 0x44 ]); calls and returns 0x100eb6e8(rect, w2, h2, radius, scaled, arg).
 */
int __cdecl Sim_1008e687(const void *rect, int arg);

#endif
