/*
 * Wave 2 functions of HGSIM.DLL around 0x10098000..0x100a0000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1009_W2H_H
#define HG_SIM_UNSORTED_1009_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Copies eight dwords from the block at 0x101c4698..0x101c5124 to
 * 0x1014b9e8..0x1014bbfc: 0x101c47b0 to 0x1014bb94, 0x101c4750 to 0x1014bbac,
 * 0x101c4f64 to 0x1014bbc0, 0x101c4db8 to 0x1014bbd4, 0x101c510c to 0x1014bbe8,
 * 0x101c4fd8 to 0x1014bbfc, 0x101c5124 to 0x1014b9e8, 0x101c4698 to 0x1014b9ec.
 */
void __cdecl Sim_10098a80(void);
/* Returns its argument. */
int32_t __cdecl Sim_10098e54(int32_t value);

#endif
