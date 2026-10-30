/*
 * Wave 2 functions of HGSIM.DLL around 0x10052000..0x10059000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1005_W2H_H
#define HG_SIM_UNSORTED_1005_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resets 16 records of the table at 0x109b7d40 (stride 0x6c: dwords +0, +8 and
 * +0x18 cleared) and overwrites the 16 records of the table at 0x109b8420
 * (stride 0x74) with the 0x74 bytes at 0x109b68e0.
 */
void __cdecl Sim_10052530(void);
/* Dword at 0x109b7d58 + 0x6c * n for n < 0x10 (signed), 0 otherwise. */
uint32_t __cdecl Sim_10052632(int32_t n);
/* 1 when Sim_10052be7(a) == Sim_10052be7(b). */
int __cdecl Sim_10052c60(int a, int b);
/*
 * Checks n 12-byte entries {pointer to a dword, mask, kind}: an entry of kind 1
 * needs (*pointer & mask) == 0, any other kind needs it non-zero. Returns 0 at the
 * first entry that fails, 1 when all pass.
 */
int __cdecl Sim_1005436d(int32_t n, const uint32_t *entries);
/*
 * With head = dword 0x10166868 and tail = dword 0x1016686c of a 64-entry ring of
 * words at 0x10165ee0: stores 0 in *out when they are equal, otherwise the
 * word at head, and steps head (back to 0 at 0x40). Returns 0.
 */
int __cdecl Sim_10057aa9(uint16_t *out);
/*
 * When src is not NULL and index is in 0..[0x109b0eac) (signed): copies fields of
 * the 0x4c-byte record 0x109b16e0 + 0x4c * index (fields at +0x10) from src,
 * as selected by the bits of mask: 1 dwords +4/+8/+0xc, 2 dword +0, 4 dword +0x2c,
 * 8 dword +0x34 (else it is set to -1), 0x10 dword +0x30, 0x20 dwords
 * +0x1c/+0x20/+0x24, 0x40 dword +0x28; the record's dword +8 becomes 1 with bit 0x80, else 0.
 */
void __cdecl Sim_100585e1(int32_t index, uint32_t mask, const uint32_t *src);

#endif
