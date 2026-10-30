/*
 * Round 10: game functions of HGSIM.DLL around 0x100a0000..0x100affff, converted one at a time.
 * Calls of functions that are not converted yet go through their original address (SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100A_W10_H
#define HG_SIM_UNSORTED_100A_W10_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* The vertex where the edge a-b meets the plane z (fields +8): when |a.z - b.z| (of the lower one minus the
 * higher one) is at least 1e-7 ([0x1012ba74]), the fields +0, +4, +0x20, +0x24, +0x30, +0x34, +0x38 are
 * interpolated from the lower end (lo + (hi - lo) * (z - lo.z) / (hi.z - lo.z)), otherwise a's are kept.
 * Writes them into a new vertex (Sim_100a88ff) with z = [0x10185c50] and returns it. */
uint32_t __cdecl Sim_100a6ee4(const float *a, const float *b, float z);

#endif
