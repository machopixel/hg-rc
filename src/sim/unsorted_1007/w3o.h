/*
 * Round 3 (letter o): loading of the two files and a third resource of a record (0x10074fce) and a table-driven record start (0x10075063).
 */
#ifndef HG_SIM_UNSORTED_1007_W3O_H
#define HG_SIM_UNSORTED_1007_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * Loads the cached files named by the dwords at obj+0x10 and obj+0x1c (when non-zero, else the result is 0)
 * into obj+0x14 and obj+0x20, with the argument [0x1014cec0]; then obj+0x44 = [0x109b98c0] and
 * obj+0x48 = the file named by obj+0x44 loaded with [0x1014cec4]. The loader is the original
 * (thunk 0x10003413), called with (name, argument, 0).
 */
void __cdecl Sim_10074fce(void *obj);

/*
 * p[0] in 1..13 selects obj = p[3] = the dword at 0x10149958 + 4 * p[0]. Allocates timer 0x10149948 on first use
 * (-1) and resets it. Returns 0 with obj NULL. Otherwise runs Sim_10076667(obj), Sim_10076624(obj, obj+0x80),
 * increments [0x1015dbcc], calls Sim_1005570e when bit 0 of obj+4 is set, and returns 1.
 * NOTE: for p[0] outside 1..13 the original reads an uninitialised stack variable as obj (p[3] is not used);
 * here that case is obj = NULL.
 */
int __cdecl Sim_10075063(int *p);

#endif
