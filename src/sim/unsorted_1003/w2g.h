/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100332e9..0x1003cba3. Names
 * carry the original address because the code alone does not say what the
 * variables are for.
 */
#ifndef HG_SIM_W2G_1003_H
#define HG_SIM_W2G_1003_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The flag at 0x1014cb34 = (value == 0) (the flag Sim_100332d2 reads). The first argument is not used. */
void __cdecl Sim_100332e9(int unused, int value);
/*
 * The inverse of Sim_1003c7b0 (same keys from `seed`): per byte, subtract the
 * second key, xor the first key, then subtract 0x6c if bit 0 is set.
 */
void __cdecl Sim_1003c860(uint8_t *buf, uint32_t count, uint32_t seed);
/* The dword at 0x109b9e08. The argument is not used. */
int32_t __cdecl Sim_1003cba3(int unused);

#endif
