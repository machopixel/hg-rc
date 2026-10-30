/*
 * Round 11: game functions of HGSIM.DLL around 0x10080000..0x1008ffff, converted one at a time.
 * Methods of the DirectDraw surfaces are COM calls (__stdcall) through the object's table.
 */
#ifndef HG_SIM_UNSORTED_1008_W11_H
#define HG_SIM_UNSORTED_1008_W11_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#define __stdcall
#endif

/* Shows the frame: a locked back buffer ([0x1017b3ac]) is unlocked (method +0x80 with [0x10372d44]), the
 * primary [0x1017b394] and back buffer [0x1017b39c] restored when lost (DDERR_SURFACELOST from +0x60, Restore
 * +0x6c), then the primary flipped (+0x2c (0, 1)). Each result goes to 0x10372b94 (failures logged through
 * Sim_10083495); returns -1 when the flip failed, else 0. */
int __cdecl Sim_10080471(void);

/* Shows the frame buffer [0x1015e9a0] with the screen-noise effects [0x10179ca0] / [0x10179ca4] (+0 x, +4 y,
 * +8 width, +0xc rows, +0x10 chance, +0x14 jitter, +0x18 roll chance, +0x1c roll-stop chance; chances out of
 * Sim_1009c929(100)): a roll shows the picture in two parts split at line [0x1017b7dc] (which advances by 16
 * modulo the rows); a jitter shifts each row of the area right by a running random offset (the CRT memcpy within
 * the row); the picture goes to the window DC [0x1017b3c0] with SetDIBitsToDevice (format 0x109e38c0), the
 * result in 0x103729c4. The second effect only runs while [0x10144fa0] is set. */
void __cdecl Sim_10083801(void);

#endif
