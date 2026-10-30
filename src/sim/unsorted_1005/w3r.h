/* Round 3 (letter r), original 0x10054d98 and 0x1005b98c. */
#ifndef HG_SIM_UNSORTED_1005_W3R_H
#define HG_SIM_UNSORTED_1005_W3R_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Updates a value `p` = [param+0x14] (a record with unaligned fields) from its input flags:
 * +0x15 byte state (also the return value), +0x2a / +0x2e / +0x22 / +0x26 pointers to bytes
 * that act as flags, +4 pointer to the int being driven, +0x1a int result, +0x10 int,
 * +0xc / +8 ints, +0 pointer to a float, +0x14 / +0x1e shift counts.
 * With the flag at +0x2a set, param+8 non-zero calls the method at +0x10 of the object
 * table 0x1015fd20[param[0]] with (dword at 0x1015f8f0 + param[0] * 0x74, param[1]).
 * Returns the new state (1 when anything was set).
 */
int __cdecl Sim_10054d98(void *param);
/*
 * For gear `index` of the table 0x109e8b40: multiplies the float at +8 of each of the 0x11
 * records (0x3c bytes, array pointer at +0x88 of the gear's data) by a factor and stores
 * clamp(trunc((f8 * 0.5 + f0 * 0.5) / 5.0), 0, 0xffff) at +0x38. The factor is 12, 6 or 1 for the
 * byte 0, 1 or 2 at [[0x1017b8c4] + 5] when [0x1015ec58] is 0 and this is the player's gear
 * (same value at +8 as the gear [0x1017b8c8]); it is 4 when [0x1015ec58] is set and the gear is the
 * player's or has 1 at +0x14 of the record its data points to. Other values of that byte make
 * the original use an uninitialised stack value; this C does the same (not testable).
 */
void __cdecl Sim_1005b98c(int index);

#endif
