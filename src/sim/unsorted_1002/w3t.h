/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1002xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the data is for.
 *
 * The three setters share the dwords 0x1013b98c and 0x1013b990 and hand the
 * value on to Sim_1004b7d2 (the display driver method). Their first argument
 * is unused; the second is multiplied by 15 / 65536 and rounded to nearest
 * (the 64-bit product shifted right by 16, plus the last bit shifted out).
 */
#ifndef HG_SIM_UNSORTED_1002_W3T_H
#define HG_SIM_UNSORTED_1002_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Stores the scaled value in 0x1013b990 and calls Sim_1004b7d2 with it, unless it equals the dword already there. */
void __cdecl Sim_100219c8(int unused, int value);
/* Stores the scaled value in 0x1013b98c, 0x1013b990 and at +0x44 of the structure whose pointer is at 0x1015d1b8, and calls Sim_1004b7d2 with it. */
void __cdecl Sim_10021a10(int unused, int value);
/* Copies 0x1013b98c to 0x1013b990 and calls Sim_1004b7d2 with it. */
void __cdecl Sim_10021a60(int unused);

/* Calls the function pointer at +0x34 of each of the 0x1015dc64 objects of the pointer table 0x109e8b40 with the dword at +0x28, when both are nonzero. */
void __cdecl Sim_10029fba(void);

#endif
