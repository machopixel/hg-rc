/*
 * Wave 3S, original 0x100526e3..0x1005e295. Plain C, original cdecl convention.
 * Groups: the 0x6c-byte table at 0x109b7d40 (dword 0 = current record, +4 = count of records,
 * +8 flag, +0x1c.. indices into the pointer table 0x109e8b40, +0x2c = the fifth one); a group
 * also owns the 0x74-byte entry `index` of the table at 0x109b8400 whose floats at +0x30 and
 * +0x50 (8 each) are offsets.
 */
#ifndef HG_SIM_UNSORTED_1005_W3S_H
#define HG_SIM_UNSORTED_1005_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * val == -1: stores -1 as the current record of group `index` and returns 0. Otherwise, when val < [0x1015dc64]
 * and index < 16: stores val, subtracts from the 8 floats at 0x109b8430 and the 8 at 0x109b8450 (+ index * 0x74)
 * the value of element (record val)[0x10] of each, and returns 1; else 0.
 */
int __cdecl Sim_100526e3(int index, int val);
/*
 * Numbers the records of group `index`: the current record (Sim_10052aaf, else Sim_10012843) gets +0x10 = 0, the
 * other usable ones (+0 == 1, word +0x1c without bits 6) 1, 2, ...; the fifth one gets 4 when the flag +8 is set.
 * Returns 0 when there is no current record to be found, else 1. The 16-bit counters wrap like the original's.
 */
int __cdecl Sim_10052ad6(int index);
/*
 * a - c: 0 within +-b of zero; beyond it the excess times d, truncated (_ftol) and limited to +-0x10000.
 */
int __cdecl Sim_100570fb(int a, int b, int c, double d);
/*
 * val limited to 1..max, plus trunc(val * 0.2 / 0.15 / 0.1) for dword obj+0xbd3 bit 0x10 / 8 / 4 (first one set);
 * stores (float)(val * 47.1744 [0x1012b4d4]) at obj+0x4d4. Bit 0x20 of +0xbd3 halves the signed dword +0x60b;
 * bits 8 / 4 / 2 of +0xbcf add 3 / 2 / 1 to +0xbd7; bit 0x40 of +0xbd3 increments +0xbe3, bit 0x80 increments +0xbdf.
 */
void __cdecl Sim_1005e295(void *obj, int max, int val);

#endif
