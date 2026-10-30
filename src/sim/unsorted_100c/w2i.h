/*
 * Wave 2 (letter i) additions to unsorted_100c: two small functions around 0x100c3700..0x100c7000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100c_W2I_H
#define HG_SIM_UNSORTED_100c_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * obj[0x54] points to a record: byte +0x21 = the low byte of value, byte +0x22
 * = 1 when the low 16 bits of value are 0, else 0.
 */
void __cdecl Sim_100c3702(void *obj, int value);
/* The dword at obj + 0xc + 12 * index, or -1 when index is negative or not below the signed dword at +0x4b4. */
int __cdecl Sim_100c6f0e(void *obj, int index);

#endif
