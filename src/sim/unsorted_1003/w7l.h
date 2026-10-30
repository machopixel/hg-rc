/*
 * Round 7, letter l: functions at 0x1003079c and 0x1003129f (bounding boxes and the 7 x 7 cell grid of a
 * model). Helpers that are not converted (or that another round-7 list converts) are called through their
 * original address with SIM_FN. Floating point follows the original x87 code with the game's control word
 * (53-bit precision); see util/w3o.h.
 */
#ifndef HG_SIM_UNSORTED_1003_W7L_H
#define HG_SIM_UNSORTED_1003_W7L_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (box, corner 0..3, model); the result is stored in the box. */
#define W7L_CALL_10031447 SIM_FN(uint32_t(__cdecl *)(void *, int, void *), 0x10031447)

/*
 * For an object whose byte +0xa0 is 't' or 'T' and that has a model +0x2c: a new grid (Sim_1003058f) goes to
 * obj+0x98 and is reset (Sim_10030672); every part of the model (0x28-byte parts from model + model[+8], count
 * word +6) is added (List_PushFront) to the cell (0xe0 * gx + 0x20 * gz + 0x24) of its centre from
 * Sim_10030228, gx and gz = trunc((centre - grid +0x18 / +0x20 + f[0x101112e0]) / f[0x101112e4]) in 0..6,
 * and the cell's box (min/max floats at +0..+0x14) grows by the part's box.
 */
void __cdecl Sim_1003079c(void *obj);

/*
 * Unless [0x1015dd6c] is set: the box of the points of the model obj+0x2c (word +4 points of 0x58 bytes from
 * +0x18) into a new Sim_1003184b record at obj+0x98, then its four dwords +0x1c.. = 0x10031447(box, k, model).
 */
void __cdecl Sim_1003129f(void *obj);

#endif
