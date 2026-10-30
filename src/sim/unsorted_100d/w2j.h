/*
 * Wave 2, letter j: leaf functions of HGSIM.DLL around 0x100d0000..0x100e0000.
 */
#ifndef HG_SIM_UNSORTED_100D_W2J_H
#define HG_SIM_UNSORTED_100D_W2J_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Nodes have links +4 (previous) and +8 (next) and an owner dword +0x14. Takes
 * the chain b belongs to (first = follow +4 to the end, last = follow +8),
 * gives every node of it the owner of node a and inserts the chain behind a:
 * first+4 = a, last+8 = a's old next, a+8 = first, and the old next's +4 (or,
 * when a had no next and has an owner, the owner's dword +0x10) = last.
 * Returns last.
 */
void *__cdecl Sim_100d1780(void *a, void *b);
/* Does nothing with its five arguments. */
void __cdecl Sim_100d8ce7(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e);
/*
 * Does nothing (an empty function). Callers pass three arguments (the
 * addresses 0x1013513c and 0x1013511c and the number 0x1a4), which it ignores.
 */
void __cdecl Sim_100d9fc0(void);

#endif
