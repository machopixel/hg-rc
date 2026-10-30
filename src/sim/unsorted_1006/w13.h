/*
 * Round 13: game functions of HGSIM.DLL around 0x10060000..0x1006ffff whose last original callee was converted in
 * round 12.
 */
#ifndef HG_SIM_UNSORTED_1006_W13_H
#define HG_SIM_UNSORTED_1006_W13_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * HUD markers of the player's object (first dword of holder). Its waypoint +0x118 (kind 0x100, not flagged 0x1000,
 * index & 0xff with a live entry at 0x109a8640 + index * 0x54): the next of the seven marker names (0x109e857c,
 * counter [0x10176954] % 7) and Sim_100ca996; the point +0x104..+0x10c projected with Sim_100d2669. On screen and
 * more than 30 pixels from [0x1017e580] / [0x1017e584]: the marker (Sim_1006519d) and, with the font
 * [0x109b98c0], the waypoint name and its range ([0x1015e4c4] / 100: "%3.0fm", above 1000 "%2.2fk" in thousands)
 * beside it in a box with a line to the point. Otherwise the two texts go to the corner (surface 0x109e87f0, rows 0
 * and 15) in the font [0x109b9900]. Then the target +0xf0 (not flagged 0x1000): units (0x200) Sim_100648a0 and
 * things (0x400) Sim_10064d05. Sim_10063037(holder) unless the target kind is 0x100.
 */
void __cdecl Sim_10063529(void *holder);

#endif
