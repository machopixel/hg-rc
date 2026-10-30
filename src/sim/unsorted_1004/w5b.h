/*
 * Round 5, letter b: functions around 0x10047e1a..0x1004e441 that call C library routines of the DLL
 * (fmod 0x100fcb30, free 0x100fefe0, malloc 0x100fef30) through their address with SIM_FN.
 */
#ifndef HG_SIM_UNSORTED_1004_W5B_H
#define HG_SIM_UNSORTED_1004_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. fmod(x, y) with both arguments on the stack as doubles, the result in st(0). */
#define SIM_FMOD SIM_FN(double(__cdecl *)(double, double), 0x100fcb30)
#define SIM_FREE SIM_FN(void(__cdecl *)(void *), 0x100fefe0)
#define SIM_MALLOC SIM_FN(void *(__cdecl *)(uint32_t), 0x100fef30)

/*
 * Wraps x into the range of the period p = obj[0x10] (float): r = fmod(x, p) (as a double); when r is more than p / 2
 * the result is (float)r - p, when -(p / 2) is more than (float)r it is (float)r + p, else (float)r. The result is
 * stored in the float obj[4] and returned (in st(0), as float).
 */
float __cdecl Sim_10047e1a(void *obj, float x);
/* Frees and clears the three buffers [0x1015f35c], [0x1015f358], [0x1015f354] (in that order) that are not NULL. */
void __cdecl Sim_1004e3d7(void);
/*
 * Sim_1004e3d7(), then allocates [0x1015f354] (0x1000 bytes), [0x1015f358] (0x4000) and [0x1015f35c] (0x9bd4);
 * when an allocation returns NULL everything is released again and 3 is returned; else 0.
 */
int __cdecl Sim_1004e441(void);

#endif
