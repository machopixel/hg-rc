/*
 * Round 4, letter v: functions of HGSIM.DLL around 0x10053000..0x10057000 that call Windows (heap, joystick).
 */
#ifndef HG_SIM_UNSORTED_1005_W4V_H
#define HG_SIM_UNSORTED_1005_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Frees the singly linked list of blocks headed by [0x1015f3b0] (the link is the first dword) and clears it and [0x109b6880]. */
void __cdecl Sim_10053132(void);
/* Frees the blocks at +0x60, +0x64, +0x68 (and the block its first dword points to), +0x6c (likewise) and +0x70 of obj, each when not NULL. Returns 0. */
uint32_t __cdecl Sim_10056d04(void *obj);
/*
 * Reads joystick cfg[0] with joyGetPosEx (flags cfg[1]) and fills axes[0..5] and buttons. Returns 1 when axes or
 * buttons is NULL, 0 when the joystick system is off ([0x1017b920] == 0), else 0 (also when joyGetPosEx fails, with
 * axes and buttons[0..1] cleared). On success buttons[0] = the button bits and, when cfg[1] has 0x40, the hat position
 * (0, 9000, 18000, 27000) ORs cfg[0x1e..0x21] into buttons[cfg[0x1a..0x1d]]. axes: Y, X, then Z (flag 4), R (8), U
 * (0x10), V (0x20) as present, each Sim_100570fb(value, cfg[b], cfg[c], double from two dwords).
 */
uint32_t __cdecl Sim_10056dfa(const uint32_t *cfg, uint32_t *axes, uint32_t *buttons);

#endif
