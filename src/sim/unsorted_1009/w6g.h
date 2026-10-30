/*
 * Round 6, letter g: the segment functions around 0x1009cc82 (a segment: start point at +0, end point at
 * +0xc, end - start at +0x18, unit direction at +0x24, length at +0x30, state dword at +0x34; see
 * Sim_1009cbfe in w3o.h). The x87 code runs at the 53-bit precision of the Windows control word, so the
 * intermediates are doubles here and only stored results are floats.
 */
#ifndef HG_SIM_UNSORTED_1009_W6G_H
#define HG_SIM_UNSORTED_1009_W6G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library square root of the DLL (double in, st(0) out). */
#define W6G_SQRT SIM_FN(double(__cdecl *)(double), 0x100fca04)

/*
 * Unless the state at +0x34 is already 2: length = sqrt of the squared end - start (computed from the points,
 * not from +0x18), unit direction = (end - start) / length into +0x24..+0x2c, length into +0x30, state = 2.
 */
void __cdecl Sim_1009cc82(void *seg);

/*
 * Moves the start point by t along the direction: with state 0 runs Sim_1009cbfe first; when the length at +0x30
 * is greater than the float at 0x1012ba04: start += t * direction, +0x18 = end - start, state = 0, then the
 * length and direction are computed again (Sim_1009cc82 when the old state was 2, else Sim_1009cbfe).
 */
void __cdecl Sim_1009cdd5(void *seg, float t);

/*
 * Sets the end point to (x, y, z) and +0x18 = end - start, state = 0; then recomputes like the old state said
 * (1: Sim_1009cbfe, 2: Sim_1009cc82); any other old state clears the direction and the length (+0x24..+0x30).
 */
void __cdecl Sim_1009ceab(void *seg, float x, float y, float z);

#endif
