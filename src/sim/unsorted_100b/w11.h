/*
 * Round 11: game functions of HGSIM.DLL around 0x100b0000..0x100bffff, converted one at a time.
 */
#ifndef HG_SIM_UNSORTED_100B_W11_H
#define HG_SIM_UNSORTED_100B_W11_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Pushes a model part onto the transform stack. The first push (depth [0x1017d224] = 0) clears 0x101a1168 and the
 * [0x101c42a4] slots of [0x1017d204] to -1. While the next free node [0x1017d220] is 0..31: depth + 1, the stack
 * entry 0x101c4220 + depth * 4 = node; node 0x101a1180 + node * 0x7c gets the part's box (the six ints +8..+0x1c
 * of part, as floats, each axis ordered min..max), its centre (box sums / 2), the parent from the stack, the 9
 * dwords 0x10150800 (scale, rotation, translation) at +0x24 and the matrix +0x48 = translate(-centre) x
 * rotate(+0x30..) x rotate(+0x3c..) x translate(centre), then multiplied by each parent's matrix up the chain;
 * 0x10150800 = the 9 dwords at 0x10150828, 0x101507f8 = node, next node + 1. */
void __cdecl Sim_100b3b16(const int32_t *part);

#endif
