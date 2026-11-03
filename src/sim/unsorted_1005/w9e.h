/*
 * Round 9, letter e: functions of HGSIM.DLL at 0x10051c90, 0x100543ee and 0x1005a1db (HUD box, the input
 * mapping file, gear tables). Helpers that are not converted (or patched elsewhere) are called through their
 * original address with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1005_W9E_H
#define HG_SIM_UNSORTED_1005_W9E_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* A line on a surface (surface, x1, y1, x2, y2, 0, colour): 0x100ef727 takes a 16-bit colour (patched by
 * hooks_hud.c), 0x100e86a1 a palette index. */
#define W9E_LINE16 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int, uint32_t), 0x100ef727)
#define W9E_LINE8 SIM_FN(void(__cdecl *)(uint32_t, int32_t, int32_t, int32_t, int32_t, int, uint32_t), 0x100e86a1)
/* The game's fatal error report SystemError(code, format, ...) (hooks_sim.c taps it). */
#define W9E_SYSERR SIM_FN(int(__cdecl *)(int, const char *, ...), 0x100bc280)

/* ---- w9e_hud.c ---- */
/*
 * Fills the box (x, y, width, height) on the surface with horizontal lines in three shades of `colour`: the
 * first quarter of the rows in colour - 1, the second quarter in colour, the third in colour - 1 and the last
 * in colour - 2 (quarters by truncating division), each shade looked up in the palette table at 0x101659e0. Nothing is
 * drawn when width <= 0.
 */
void __cdecl Hud_DrawShadedBox(uint32_t surface, int32_t x, int32_t y, int32_t width, int32_t height,
                               int32_t colour);

/* ---- w9e_input.c ---- */
/*
 * Reads one input mapping file (inputw.map or a device's .std) from the stream until its end: each block is a
 * line "<control> {" naming an entry of the channel table (Sim_10053620; "walk_backward"/"walk_forward" also
 * set 0x10179ce4). An analog entry (dword +8 == 0) takes a line " [+-] <device> <key>" (sign, Sim_1005393a of
 * the device's channel, Input_AddAnalog, the sign at slot +0x10 and the chord of the slot by Input_ReadChord;
 * with an unknown key the block is skipped up to its "}"). Other entries are discrete controls (0x6e bytes at
 * 0x101620c0, count 0x1015fdf0, at most 100 for kind 1): the chord, the entry's dword +4, the kind byte (0 for
 * kind 1, else 1) and the "menu_" flag; for kind 1 a name ending in _plus/_minus also registers the base
 * (or base_delta) channel as an analog control. Returns 1 at the end of the file, 0 after an error (reported
 * with SystemError 0x11 and the line number 0x1015fdf8).
 * Buffers are sized like the original's (name and device 0x28, key 0x14 bytes): longer words overflow them
 * there and here differently.
 */
int __cdecl Input_ReadMap(void *stream);

/* ---- w9e_gear.c ---- */
/*
 * The largest "size" value among the components of a gear record: for each of the S32(gear, 0xb0) items of
 * 0x6c bytes at gear +0xc8 (type at +4, option word at +0xc) the dword +0 of the type's entry (0xaa bytes at
 * 0x10111688), and depending on the type's class (dword +4: 0, 1, 3, 4, 2, 5..10) the dword +0 of the entries
 * (0x98 bytes at 0x10124ea8) selected by the option word's fields, or fixed dwords for the flag bits of class 2.
 */
int __cdecl Sim_1005a1db(const uint8_t *gear);

#endif
