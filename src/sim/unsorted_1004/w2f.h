/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1004xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1004_W2F_H
#define HG_SIM_UNSORTED_1004_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Appends an entry to the table of 0x60 eight-byte entries at 0x109b95c0, whose
 * next free index is the signed dword 0x1013b360 (-1 means the table has not
 * been initialised: Sim_100480ab is called first). The entry gets the dword
 * a & 0xffff at +0, the words b at +4 and c at +6. Returns the index used and
 * advances the counter, or -1 when the table is full (counter -1 or >= 0x60).
 */
int __cdecl Sim_10048030(uint32_t a, int16_t b, int16_t c);
/* Resets all 0x60 entries of that table (Sim_100480f2) and the counter 0x1013b360 to 0. */
void __cdecl Sim_100480ab(void);
/* Resets one entry: dword +0 = -1, words +4 and +6 = 0. */
void __cdecl Sim_100480f2(void *entry);
/* With the pointer table at 0x109b94e0, clears the dwords +0 and +0xc of the structure pointed to by entry i (its range test is always true, so it never skips). */
void __cdecl Sim_10048422(int i);

#endif
