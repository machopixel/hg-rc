/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100a8943..0x100ac603. Names
 * carry the original address where the code alone does not say what the
 * variables are for.
 */
#ifndef HG_SIM_W2G_100A_H
#define HG_SIM_W2G_100A_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Returns the old [0x1017d178] and adds 0x10 to it. When the new value + 200
 * is not below [0x1017d174] (unsigned), also sets [0x101c9ea4] = 0.
 */
uint32_t __cdecl Sim_100a8943(void);
/* p + 0xc. */
uint8_t *__cdecl Sim_100aa807(uint8_t *p);
/*
 * `point` holds two ints. Compares them with the three pairs at 0x1014d1b8
 * (8 bytes apart, x then y) by the sum of absolute differences against
 * (point - 1) and stores the index of the nearest pair in [0x101c64f0]
 * (the first one wins a tie; none is stored if all three sums are 7 or more).
 */
void __cdecl Sim_100ab89a(const int32_t *point);
/* [0x1017d1e0] = value clamped to 0..15. */
void __cdecl Sim_100ac5da(int value);
/* If value == -1 returns [0x1014d1d8]; otherwise stores value there and returns it. */
int __cdecl Sim_100ac603(int value);

#endif
