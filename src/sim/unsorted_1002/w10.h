/*
 * Round 10: game functions of HGSIM.DLL around 0x10020000..0x1002ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1002_W10_H
#define HG_SIM_UNSORTED_1002_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Whether the point (x, y) lies inside the polygon poly (vertex count = word +2, at least 3; byte indexes into
 * the 0x58-byte vertices at poly + dword +4; a vertex's x, y are the floats +0x10, +0x14): 0 unless the vertices
 * surround the point on all four sides, then the edges are walked (from the last index down) counting crossings to
 * the left and right, as the original does with its two flags; returns 0 or 1. */
int __cdecl Sim_1002f021(const uint8_t *poly, const uint8_t *verts, float x, float y);

/* Per-frame cockpit update of the own unit (view = obj, unit = [obj]): Sim_1006561a, the camera 0x100d1f90 when
 * [0x1017e618] is set, the view mode [0x109e89d8] = obj+0x4f0 (the panel [0x109e8920]'s +0x13c unless mode 2;
 * nothing more in modes 4 and 5), the headings 0x109e88bc = fmod(fmod(unit+0x68, 360) + 360, 360) and 0x109e88b8 =
 * fmod(obj+0xc, 360), the differences to the aim heading (wrapped into -180..180) and the aim distance, the flags
 * 0x1015dd10 (Sim_10069f68 in mode 2) and 0x1015f1d8 (bit 8 of obj+0x560), Sim_1002c76d(obj), then by mode the
 * mode routine (Sim_100999bc / Sim_1009b99b / Sim_1009bc94, unless a replay byte and the network flag say no) and
 * the 30 panels' routine +0x13c / +0x138 / +0x140; mode 2 ends with Sim_100622f0. */
void __cdecl Sim_1002c1f9(uint8_t *obj);

/* Cockpit setup of the own unit (widgets = (table 0x109e8b40 [0x1017b8c8])+0x28): the 30 panel flags 0x109e8960 set (but panel 2,
 * and panel 11 for a network game without replay), the 30 panels 0x109e88e0 made (HeapAlloc 0x148, Sim_10028810) and set up
 * through their routines, Sim_1002b68f(widgets), the routines of panels 18, 1, 11..19, 25 set, Sim_10050c5b,
 * 0x1015ef1c / 0x1015ef20, Sim_10077e34, Sim_10062a3a, Sim_10099928 (not for a network game without replay), the
 * 0xfe01-byte block 0x1015dc2c, 0x1015dc38 read through 0x1008cdc9 (default 0x51f) and 0x1017e5e8 = [0x109e3e1c] / 50. */
void __cdecl Sim_1002bca0(void);

/* Blast damage at (x, y, z): every unit (only the own one, or the own player's kind-3 one, in a network game [0x1015ecd0])
 * whose radius (record+0x53c) plus radius reaches the point (Sim_10084a94) takes a hit of trunc(frame time * rate) on
 * parts 2..6 and a quarter of it on part 1 (Sim_1006c161 with a shot record of kind 1, factor 0.1). */
void __cdecl Sim_100255de(int32_t killer, float x, float y, float z, float radius, float rate);

/* The running blast (time left 0x1015dc0c): its radius 0x1015dbf4 grows by 200 * frame time up to [0x1015dbd0]; the units
 * in reach take damage (Sim_100255de) and are pushed (Sim_100257ec) from the centre 0x1015dbf8..; the time runs down. */
void __cdecl Sim_10026057(void);

/* Spawns the effect of code (low byte; the high byte's flags pick ground / air / water variants) at (x, y, z): the own unit's
 * shake near it ([0x1015ef3c]), a free effect slot (0x109a5a40, 0x2c bytes; at most two close ones of a kind with
 * [0x10152418] = 1) with its model placed (node of the hit, rotation ra..rc), a light and a ground glow for the bright ones,
 * the camera moved to (tx, ty, tz) for the big ones, the burst 0x10b (Sim_10024616) and the sound (Sim_1000ed52). The
 * codes 11, 37, 38 make craters (Sim_10025c8c) instead. */
void __cdecl Sim_100246b3(uint32_t owner, uint8_t *node, int32_t code, float x, float y, float z, uint32_t tx, uint32_t ty,
                          uint32_t tz, uint32_t ra, uint32_t rb, uint32_t rc);

/* One frame of the shot idx (record 0x1096fc80 + idx * 0x156): the node's position (Sim_100a9cfb) moved by velocity
 * +0x18.. and acceleration +0x24.. over the frame time 0x1015dc80, the path as a segment (Sim_1009cab0, normalised by
 * Sim_1009cc82 beyond 3000 or Sim_1009cbfe), the hit (forced by flag 0x8000 of +0x48 for a unit / building target, else
 * Sim_1002dda9): a unit hit (flag 0x100) is scored, damaged (Sim_1006c161) and told (Sim_10015afd), other hits bounce or
 * stop the shot; a miss below y = 0 meets the ground (Sim_1002d961). Ending shots go to Sim_10024548; the others check the
 * kind-5 shots near them (kinds 0x3e / 0x3f), steer (Sim_100241ed), keep above the ground, move the node and its light,
 * mark the camera target and draw their trail by kind. */
void __cdecl Sim_100225c7(int idx);

#endif
