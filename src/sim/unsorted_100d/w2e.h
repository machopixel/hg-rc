/* Wave 2e, original 0x100d7fc0 and 0x100da4f7. */
#ifndef HG_SIM_UNSORTED_100D_W2E_H
#define HG_SIM_UNSORTED_100D_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Initialises the nine blocks of 0x80c bytes at 0x10180a00 and the 32-dword table at
 * 0x10180980. Block n gets (as float bit patterns) +0xc = -16384, 0 or 16384 and +0x14 =
 * -16384, 0 or 16384 (n % 3 and n / 3), +0x10 = 10000, +0xe4 = +0xec = 20000, +0xe8 = +0xf4 =
 * +0xf8 = 0, +0x750 = 0x2d, +0x754 = 0; its 0x2d entries of 0x24 bytes from +0xfc get
 * +0 = index, +4 = -1, +0x20 = the block, and the index table at +0x758 is 0..0x2c; the
 * 0x80 bytes of the table at 0x10180980 are copied to +0x18.
 */
void __cdecl Sim_100d7fc0(void);
/*
 * Inserts node into a structure of 27 lists (table[(first character - 'A')], 26 for any other
 * character). Nodes have next/prev links at +0/+4 ordered by the int key at +0x10 and,
 * for equal keys, links at +8/+0xc ordered by the string at +0x14. Returns 0 when a node with
 * the same key and name is already there, 1 otherwise (also when the node was put in).
 */
int __cdecl Sim_100da4f7(void *node, void *table);

#endif
