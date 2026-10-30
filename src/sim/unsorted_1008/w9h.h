/*
 * Round 9, letter h: the debug / option key dispatcher (0x1008b922).
 */
#ifndef HG_SIM_UNSORTED_1008_W9H_H
#define HG_SIM_UNSORTED_1008_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_keys.c ---- */
/*
 * Acts on key code (low 16 bits of `key`): 0x15 Sim_100448cd(2); 0x17 / 0x18 Sim_10044a69(1, 0) / (0, 0); 0x26 a
 * debug break; 0x79..0x82 [0x1014b930] = 1..9, 0; 0x83..0x8a announce a text ([0x101c4fa4], [0x101c4fa8], [0x101c4f98],
 * [0x101c4fa0], [0x101c4fb8], [0x101c4fbc], [0x101c4fac], [0x101c4fb0]) and [0x1014b934] = 1..8; 0x8b..0x94 [0x101303d0]
 * = key - 0x8b (-1 when not below [0x104737e0]); 0x95 [0x101303d0] = -1 if it was not -1 else -2; 0x96 / 0x97
 * [0x1015d108] -1 / +1; 0x98 / 0x99 the byte 0x1047383a + [0x101303d0] * 0x4fc3a + [0x1015d108] * 0x13f from 3 to 6 / 5;
 * 0x9a..0x9d Sim_1003327f(0x100 / 0x200 / 0x400 / 0x800); 0x9f toggles [0x1012dc98] (copied to [0x1015d04c]); 0xa1 cycles
 * [0x1014cb20] 0, 1, 2; 0xa2 / 0xa4 / 0xa5 toggle [0x1015d02c] / [0x1015d034] / [0x1015d040] (copied to the next dword);
 * 0xa3 toggles the word [0x109e88e0] +6; 0xa8 / 0xa9 / 0xaa toggle the bytes [0x1017b8c4] +2 / +3 / +4 announcing
 * [0x101c4fe0] or [0x101c4824], [0x101c482c] or [0x101c4828], [0x101c4834] or [0x101c4830]; 0xab toggles [0x10152418]
 * between 2 and 1 announcing "LOD Quality HIGH" or [0x101c4838]; 0xac Cache_DumpLog(); 0xae [0x10152414], 0xaf
 * [0x1015d02c], 0xb1 [0x1015d040], 0xb2 [0x1015d034], 0xb4 [0x1015d01c] toggled; 0xb3 cycles [0x1014cb1c] 0, 2, 1.
 * Announcing is Crt_SPrintf(buf, text) and Sim_1007df6c(buf, [0x109b98c0], 0x16a, 0x32).
 */
void __cdecl Sim_1008b922(uint32_t key);

#endif
