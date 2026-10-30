/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x100d1a60..0x100da428. Names
 * carry the original address where the code alone does not say what the
 * variables are for.
 */
#ifndef HG_SIM_W2G_100D_H
#define HG_SIM_W2G_100D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * A lookup on a code: 0x2b -> [0x10144fa0], 0x54 -> [0x101452d0],
 * 0x58 -> [0x1015f1d8], 0xc4 -> Sim_1003cba3(0), 0xc5 -> 1 when [0x1014cb1c]
 * is 1 (else 0); every other value -> 0.
 */
int32_t __cdecl Sim_100d1a60(uint32_t code);
/* [0x10152418] = 1 when value is non-zero, else 2. The first argument is not used. */
void __cdecl Sim_100d2c86(int unused, int value);
/* The dword at 0x1017e65c. */
int32_t __cdecl Sim_100d3a67(void);
/*
 * The table of 0x128-byte entries at obj[0x84]: when 0 <= value < 3 (signed)
 * sets the dword at +0xc of entry `index` to value.
 */
void __cdecl Sim_100d635e(uint8_t *obj, int index, int value);
/*
 * Toggles bit 0x4000 of the word at obj + 0x560. Then with n = obj[0x4fc]:
 * calls Sim_100d635e(obj, n + 1 (or 0 when that entry's first dword is -1),
 * value of entry n's +0xc), i.e. the value of entry n goes to the next entry (or entry 0).
 */
void __cdecl Sim_100d67af(uint8_t *obj);
/* Does nothing (the original only clears six locals). */
void __cdecl Sim_100da428(void);

#endif
