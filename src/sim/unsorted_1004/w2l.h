/*
 * Wave 2l: functions around 0x10047fe0..0x1004d26d: a byte table, a 3-byte
 * translation through a table and a family of thresholds applied to the low 16
 * bits of a word (Sim_1004c318). Names carry the original address because the
 * code alone does not say what the values are. Plain C, original cdecl
 * convention.
 */
#ifndef HG_SIM_UNSORTED_1004_W2L_H
#define HG_SIM_UNSORTED_1004_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The 256 bytes at 0x109b9920 become 0, 1, 2, ... 255; then the byte at 0x109b992e is set to 9. */
void __cdecl Sim_10047fe0(void);
/* out[i] = table[in[i]] for i = 0..2, table being 0x109b9080 + 0x40 * [0x1013b98c]. */
void __cdecl Sim_1004b804(const uint8_t *in, uint8_t *out);
/* The low 16 bits of word (the other two functions below take the same value). */
uint32_t __cdecl Sim_1004c318(uint32_t word);
/* Up to 0x3b6 the value itself; above it 0x3b6 + 10 * ((value - 0x3b6) / 0x32), value = Sim_1004c318(word). */
int __cdecl Sim_1004c325(uint32_t word);
/* 0 below 0x258, 1 below 0x2ee, 2 below 0x3b6, else 3 (value = Sim_1004c318(word)). */
int __cdecl Sim_1004c3dd(uint32_t word);
/*
 * base = dword at 0x10124ec4 + 0x98 * Sim_1004c30d(word). With value = Sim_1004c318(word): base + 1 below
 * 0x190, base below 0x258, base - 1 below 0x2ee, base - 2 below 0x3b6, else base - 3.
 */
int __cdecl Sim_1004c426(uint32_t word);
/* Returns 1. */
int __cdecl Sim_1004c95b(void);
/* The dword at 0x101116b2 + 0xaa * index (same as Sim_1004d330). */
int __cdecl Sim_1004ce50(int index);
/* The low 16 bits of word (same as Sim_1004c318). */
uint32_t __cdecl Sim_1004d26d(uint32_t word);

#endif
