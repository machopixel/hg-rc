/*
 * Round 14: game functions of HGSIM.DLL around 0x10050000..0x1005ffff that were left original (uninitialised locals
 * of the model loader and its callers), settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_1005_W14_H
#define HG_SIM_UNSORTED_1005_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Builds a gear record obj from its design (GDF resource): the flags +0xbcb / +0xbcf / +0xbd3 and +0x613 from the
 * design, then for each of its parts (design +0xb0 of them, 0x6c bytes each from design +0xc8: kind, sub-kind, size,
 * flags word, the system word and 20 more words) the totals of its kind's tables (1 cockpit and sensors, 2 the
 * chassis: armour, class 1..5 by sub-kind, the chassis columns and jump jets, 3 / 4 armour, 5 / 6 engine, 7 the
 * locomotion, 8..16 a weapon mount: its POF segment found in the model (Sim_1005ae58, "the POF seg %s is missing"
 * when not), a weapon slot +0x84 and a link +0x8c filled), and the system entry +0x88 of each kind 1..16. Then the
 * weight, Sim_1004bce5 / Sim_1004bc65 / Sim_1004be11 for the top speed and the scale of the 17 systems, the speed
 * +0x4d4, the class +0xbdb, the totals +0xbab..+0xbe7, the jump jets +0x510.., and Sim_1005b7e0. Returns 1.
 */
int __cdecl Sim_1005bb7a(uint8_t *obj, uint8_t *design);

/*
 * Builds a gear record from a design: a gear design (kind +0x8c == 2) goes to Sim_1005bb7a; any other (the old part
 * format, kinds 1..0x2c) sums its parts' words into the class, armour, sensors, weights and the system entries +0x88,
 * fills a weapon slot and links for each mount kind (0x21..0x2c, POF segments "pof_...", Sim_1005ae58), takes the
 * flags +0xbcb / +0xbcf / +0xbd3 / +0x613 from the design header, and ends with Sim_1005b7e0 and
 * Sim_1005e295(obj, weight, weight sum). Returns 1 (or the answer of Sim_1005bb7a).
 */
int __cdecl Sim_1005cd4a(uint8_t *obj, uint8_t *design);

#endif
