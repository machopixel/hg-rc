/*
 * Round 10 (helper d): game functions of HGSIM.DLL around 0x10070000..0x1007ffff.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_1007_W10D_H
#define HG_SIM_UNSORTED_1007_W10D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Test of a row id (byte 0 index, byte 1 kind) for a side: 0 when the side's dword 0x109b7d40 + side * 0x6c is
 * negative and [0x1015ec58] is 0. Kind 2 (entity index): 1 when an entity of the side (pointer table, flags +0x1c
 * without 6) is within 4000.0 of that entity (height +0x5c only counted when both Sim_10079feb agree). Kind 4
 * (0x109e9ba0 + index * 0x50, position from Sim_100b53f0 of its +8): 1 when an entity of the side is within 20000.0
 * (height only for Sim_10079feb entities). Kind 1 (area 0x109a8640 + index * 0x54, radius +0x10, 20000.0 when not
 * above 0): 1 when every entity of the side without flag 4 is inside the area (and there is one); the player's
 * entity with flag 0x2000 marks the area (+0x24 bit 0x20, +0x26 bit 1 << side) once and plays a sound. Else 0. */
int __cdecl Sim_1007a4af(const uint8_t *id, int32_t side);

/* As Sim_1007a4af for kinds 2 and 4. Kind 1: 1 as soon as an entity of the side is inside the area (the player's
 * entity with flag 0x2000 marks it once, the sound 0x10149c20); every entity of the side outside it counts +0x16c
 * up and at 2 has its block +0x28 scored to nobody (Score_RecordKill(-2, block)). Else 0. */
int __cdecl Sim_1007a955(const uint8_t *id, int32_t side);

/* As Sim_1007a4af, but for the side's own entity only (the pointer table entry of the dword 0x109b7d40 + side * 0x6c):
 * kind 2 within 4000.0 of the entity (flags +0x1c without 6), kind 4 within 20000.0 of the point, kind 1 inside the
 * area (marked once with the sound 0x10149c08 when that entity is the player's with flag 0x2000). The height term is the
 * constant [0x1012b848] squared: in kind 2 when Sim_10079feb differs for the two entities, in kinds 4 and 1 when it
 * is set for the side's entity. */
int __cdecl Sim_1007a005(const uint8_t *id, int32_t side);

#endif
