/*
 * Round 12: game functions of HGSIM.DLL around 0x10060000..0x1006ffff that read uninitialised stack in the original;
 * converted with the start values recorded in DECISIONS.md ("Round 12: uninitialised stack").
 */
#ifndef HG_SIM_UNSORTED_1006_W12_H
#define HG_SIM_UNSORTED_1006_W12_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * HUD marker of a thing: the point is projected (Sim_100d2669). Off screen: the projected point, clipped to the
 * rectangle 0x109e3e00 (Sim_1008e266), gets the edge arrow of its side (names [0x109e85a4] left, [0x109e85ac] top,
 * [0x109e85a8] right, else [0x109e85a0]) in palette 10 (kind 0) / 11 (kind 2) / 9; Sim_10063415(1, id, x, y, kind)
 * and Sim_10065216 draw it. On screen: half size d = camera +0x94 * size / z, at least [0x1015dc38]; with palette
 * 1 / 0 / 2 for kind 0 / 1 / 2 (0 for any other kind: the original reads an uninitialised local, DECISIONS.md),
 * when Sim_10063415(0, id, x, y, kind) allows it the four corner pieces ([0x109e85b0] above, [0x109e85bc] below,
 * [0x109e85b8] right, [0x109e85b4] left) are drawn d away. Palettes are 0x10176960 + 512 * p with [0x1017b940],
 * else 0x10178b60 + 256 * p.
 */
/* 0x100648a0: the marker of a unit, at its position +0x58..+0x60 (+0x5c lowered by record +0x520 when the record's
 * word +0x562 has bit 8), size record +0x53c, id unit +4. */
void __cdecl Sim_100648a0(const uint8_t *unit, int32_t kind);
/* 0x10064d05: the marker of a thing with a model (+0x8c, none: nothing), its centre and size from Sim_1000946e. */
void __cdecl Sim_10064d05(uint32_t id, const uint8_t *thing, int32_t kind);

/*
 * The compass of the HUD on canvas [0x109e8940] +0xec (anchor +0xf0 -> two dwords, image [0x1015dc2c]): the dial
 * ([0x109e8484]) rotated by heading = trunc(360 - yaw) % 360, as trunc(heading * 10); the frame ([0x109e8480],
 * 0x100f0205 with [0x1017b940], else 0x100e91dc); the bearing needle ([0x109e8488]) when the player's target
 * (+0x118 low byte) has a live record at 0x109a8640 + 0x54 * that, at trunc(360 - (yaw - [0x1015e4d4])) * 10;
 * the target arrow ([0x109e848c] when Sim_10052be7 of the unit target (+0xf0 low byte, kind 0x200, record +0x4f0 = 2)
 * is 1, else [0x109e85c8]) at trunc(pitch * 10); and the heading as text ("%d") in font [0x109b9900] at +0xac / +0xb0
 * (0x100f2f01 with [0x101668e8], 0x100f1e10 with [0x1017b940], else 0x100ecea6; colour byte 0x109b992e = 12, then 9).
 * Every loaded resource is released with Sim_100bb2b0. Without the dial the heading text is 0 (the original prints an
 * uninitialised local, DECISIONS.md).
 */
void __cdecl Sim_10064364(float yaw, float pitch);

#endif