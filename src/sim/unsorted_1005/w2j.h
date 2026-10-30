/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x10050000..0x10060000.
 */
#ifndef HG_SIM_UNSORTED_1005_W2J_H
#define HG_SIM_UNSORTED_1005_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Dword 0x1013f140 = 0. */
void __cdecl Sim_1005570e(void);
/* Returns 0. */
int __cdecl Sim_10056df3(void);
/*
 * Resets the table of 0x100 records of 0x4c bytes at 0x109b16e0: dword 0x101668d4
 * and 0x101668d0 = 0, dword 0x109b0eac (count) = 0x100, dword 0x109b0eb0 = 0xff,
 * the dword table at 0x109b12c0 = 0..0xff, and each record cleared except +0xc =
 * its number and +0x10 and +0x44 = -1.
 */
void __cdecl Sim_10058389(void);
/* When 0 <= index < dword 0x109b0eac: dword +8 of that record is 1 for a non-zero flag, else 0. */
void __cdecl Sim_1005892e(int index, int flag);
/* Copies 26 dwords of the 0x101c4xxx..0x101c5xxx area to fixed places in 0x10144xxx. */
void __cdecl Sim_1005fa30(void);

#endif
