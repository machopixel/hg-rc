/*
 * Wave 2 functions of HGSIM.DLL around 0x100d6000..0x100d7000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_100d_W2H_H
#define HG_SIM_UNSORTED_100d_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * With e = obj[0x84] + 0x128 * (signed dword obj+0x4fc) (0 returned when that dword
 * is -1): when the signed dword at 0x10145534 + 0xd6 * e[4] and e[0x20] are both
 * positive, clears e[0x20], sets e[8] to -1 and returns 1; otherwise returns 0.
 */
int __cdecl Sim_100d624d(const void *obj);

#endif
