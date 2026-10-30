/*
 * Wave 2 (letter i) additions to unsorted_1003: small functions around 0x10033000..0x1003c000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1003_W2I_H
#define HG_SIM_UNSORTED_1003_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* entry + 4, or NULL when entry is NULL. */
unsigned char *__cdecl Sim_10033555(unsigned char *entry);
/* Stores v in the dword 0x1015e53c when 0 <= v < 11, else 0. */
void __cdecl Sim_10035444(int v);
int __cdecl Sim_10037675(void); /* returns 0 */
/* The dword at +0x10 of the record that the pointer at 0x1015d0a4 points to. */
uint32_t __cdecl Sim_1003a53b(void);
/*
 * Table of 0x7e-byte entries at 0x109b9e20 (count at 0x1015ea80, see
 * unsorted_1003/slots.c). Sim_1003b471 stores value in the dword at +4 of an
 * entry (no range check); Sim_1003b4d0 returns the dword at
 * entry + 0x50 + 4 * (that dword), or 0 when index is not in 0 .. count - 1.
 */
void __cdecl Sim_1003b471(int index, uint32_t value);
uint32_t __cdecl Sim_1003b4d0(int index);

#endif
