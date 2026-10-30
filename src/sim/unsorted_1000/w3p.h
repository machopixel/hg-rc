/*
 * Round 3 functions of HGSIM.DLL in 0x1000xxxx (letter p).
 */
#ifndef HG_SIM_UNSORTED_1000_W3P_H
#define HG_SIM_UNSORTED_1000_W3P_H

#include <stdint.h>

#include "../simmem.h"

/*
 * 1 when -b < a < b and -d < c < d, else 0 (floats). Each comparison is the x87 one that is also true when a NaN is
 * involved, so a NaN counts as inside.
 */
int __cdecl Sim_1000b01f(float a, float b, float c, float d);

/*
 * Does nothing. The original walks the entries of the table at [0x1015d0a4] (count at +4) that have a non-zero
 * dword in two parallel arrays, compares a value of each with dword 0x1015dc78 and takes the same path whatever the
 * result; it only reads. (It would fault on a bad table pointer, this version does not.)
 */
void __cdecl Sim_1000e6c5(void);

#endif
