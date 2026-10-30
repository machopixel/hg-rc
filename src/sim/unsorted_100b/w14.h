/*
 * Round 14: game functions of HGSIM.DLL around 0x100b0000..0x100bffff that were left original for a stack overflow,
 * settled by the shipped data (DECISIONS.md, "Round 14").
 */
#ifndef HG_SIM_UNSORTED_100B_W14_H
#define HG_SIM_UNSORTED_100B_W14_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Loads the model of scene object `index` (entry 0x101a2100 + index * 0x88: name at +0, parent +0x14, flags +0x18,
 * +0x1c, the trigger +0x24, the model +0x28, the node +0x2c, scale +0x30..+0x38, position / angles +0x3c..+0x50,
 * matrix +0x54) for the scene `scene` (matrix 0x101a11c8 + scene * 0x7c). 0 for a disabled (flag 0x800) or empty
 * entry, or when its trigger unit (0x109e9ba0 + trigger * 0x50, word bit 0x8000) is already gone (Sim_10028b50);
 * 1 when its model is already loaded and not flagged 0x400 (Sim_100093ea). Otherwise the WTB resource of the name
 * (0x100ba7d5, [0x1014cedc]) is read with Sim_100e2a31 after Sim_100e2776 / Sim_100e282a; a name starting with
 * 'o' also has an SDF part list: "oc..." loads each sub-model (up to 9, the 32-byte slots +0xc of the SDF) as its own
 * model into a list, any other "o..." adds each to the main model (Sim_100e3ba4, with its colour +0x14c), and the
 * SDF is released. Then the model is placed: under the scene (parent -2: matrix product, ground height
 * Sim_1002d961 below [0x1012bac0], the BOLD objects turned to the sun [0x1015dd4c..54]), as the scene's own node
 * (parent -1) or with each listed sub-model as a child node of the parent's model, and its kind (+0x18 >> 12)
 * handled (5 Sim_1003129f, 8 Sim_1003079c). 1 on success, 0 when a load fails or the entry has no node.
 */
uint32_t __cdecl Sim_100b48de(int32_t index, int32_t scene);

#endif
