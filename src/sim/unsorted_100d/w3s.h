/*
 * Wave 3S, original 0x100daea4. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100D_W3S_H
#define HG_SIM_UNSORTED_100D_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Appends a record to the table of 0x74-byte records at 0x109b68e0 (count [0x1015f3ac], at most 0x20). src has a
 * name at +8 (0x10 bytes) and from +0x18 triples of dwords (x, y, z) of count (src[4] - 0x18) / 12 <= 8; the record gets
 * the name (strncpy rules), the floats x[8] at +0x10, y[8] at +0x30, z[8] * [0x1012bd88] (1/65536) at +0x50 and the
 * count at +0x70. Returns 1, or 0 when the table is full or there are more than 8 triples.
 */
int __cdecl Sim_100daea4(const void *src);

#endif
