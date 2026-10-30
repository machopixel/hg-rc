/*
 * Round 10: game functions of HGSIM.DLL around 0x10040000..0x1004ffff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1004_W10_H
#define HG_SIM_UNSORTED_1004_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Calls 0x10087d83(camera [0x1014cae0]) (patched by hooks_view.c: fills the visible cell list 0x1036f2a0, ended by
 * 0). For every entry of every listed cell (list at cell+0x20, next at entry+0x20) of kind (word +2 & 0xf0)
 * 0x10, 0x60 or 0x20, without bit 0x1000 in word +0, with a node at +0x28: turns the node with
 * Sim_100aa0da(node, 0, Sim_1008486e(camera.x - node.x, camera.z - node.z), 0, 0) (or (-45.0, 180.0) when
 * Sim_1009b2eb() is set) and calls Sim_100aa742(node). */
void __cdecl Sim_100456fc(void);

/* Free camera step (camera = [0x1014cae0], target from Sim_10044b2c, entity record [[0x109e8b40]+0x28]): unless
 * [0x1013aed4] is 1, the speed limits 0x1015eeec..0x1015eef8 are set from record+0x53c when 0x1015eeec is at its start
 * value, the five interpolators 0x109b9aa0.. are restarted at [0x1015dc78] from the camera's offset and Sim_10044909(0)
 * runs. Then the speed (+ speed, clamped), height (+ lift), heading 0x1013aec8 (+ turn, fmod 360), the camera's two
 * angles (Sim_100cbc6b towards the target, yaw), the position around the target (Sim_10084766 of the heading) kept
 * above the ground (Sim_1004487c) and the height limits of the record (+0x520, flag 0x20 of +0x560). */
void __cdecl Sim_10044d0b(float speed, float lift, float yaw, float turn);

/* Picks the animation of obj (+0x90, its speed class +0x94) from its record (obj+0x28): standing, falling (the fall kind of
 * Sim_1004f1df), the control bytes 0x57 / 0x58 / 0x59, the speed tiers of record+0x7c ([0x1012b410]..), crouching / hit
 * states (+0x562); kind-5 objects always 0x17. Then Sim_1004ee50 and, for the own unit with obj+0x88 bit 8, the step sound. */
void __cdecl Sim_1004f257(uint8_t *obj);

#endif
