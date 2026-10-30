/*
 * Round 13: game functions of HGSIM.DLL around 0x10010000..0x1001ffff whose last original callee was converted in
 * round 12.
 */
#ifndef HG_SIM_UNSORTED_1001_W13_H
#define HG_SIM_UNSORTED_1001_W13_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Per-frame steering of a unit by its order word +0x1a6. First Sim_1001403a(unit, key), Sim_100c264a(unit) and the
 * limit f = Sim_10014061(key); for unit types Sim_10079feb accepts, Sim_10017aa9 (orders 10, 11) or
 * Sim_10017b3e(unit, 4000.0f). Order 2: clears the block's (+0x54) bytes +0x26 / +0x28; without a contact
 * (Sim_100c2ae5 == 0) block +8 = Sim_1001412d(unit, f) and the heading Sim_100144e9, else Sim_10015266; then
 * Sim_1000ae00(unit, heading). Order 4: +0xe8 = fmod(+0xe8 + 180, 360); a type-1 unit whose record word +0x562 has
 * bit 0 clear sets block +0x56 = 1, +8 = 1/128 and runs Sim_100144e9, else without a contact block +8 = 1/64 and
 * Sim_100144e9. Orders 5..8 (Sim_100178f4 first) and 13, 14: without a contact block +8 = Sim_1001412d(unit, f) and
 * Sim_100144e9; order 5 ends with Sim_10015a63. Order 15: with an entry index +0x23a >= 0, Sim_10015630(unit, 5,
 * 0x2201, 0) when Sim_10033c42 has no entry or Sim_100c45dc(unit, entry, unit + 0x228) answers 2. Any other order:
 * block +8 = 0.0125 for side +0x14 == 2, or side 3 owned (+0xc) by the player [0x1017b8c8].
 */
void __cdecl Sim_100139ff(uint8_t *unit, int16_t key);

#endif
