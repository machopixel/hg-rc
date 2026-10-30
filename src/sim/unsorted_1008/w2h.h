/*
 * Wave 2 functions of HGSIM.DLL around 0x10083000..0x10090000 (letter h).
 * Names carry the original address because the code alone does not say what
 * the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1008_W2H_H
#define HG_SIM_UNSORTED_1008_W2H_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_10083490(void);
/*
 * Reads the dwords obj+4, +8, +0xc and +0x10 (the original builds a table of
 * four rectangles from them on its stack, which it never uses) and returns nothing.
 */
void __cdecl Sim_1008d405(const void *obj);

#endif
