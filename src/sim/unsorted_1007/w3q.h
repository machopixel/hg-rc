/*
 * Round 3 (letter q) functions of HGSIM.DLL around 0x10078000..0x1007a000.
 * Names carry the original address because the code alone does not say what
 * the fields mean. Floating point is done as the original's x87 did it
 * (see util/w3q.h).
 */
#ifndef HG_SIM_UNSORTED_1007_W3Q_H
#define HG_SIM_UNSORTED_1007_W3Q_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Calls the method stored at +0x11c of seven objects (object pointers at 0x109e8928, 0x109e8938,
 * 0x109e890c, 0x109e88e8, 0x109e894c, 0x109e8950, 0x109e8954) as method(object, value) with the
 * dwords 0x1096fc08, 0x1096fc0c, 0x1096fc04, 0x1096fc00, 0x1096fc10, 0x1096fc14, 0x1096fc18 in that order.
 */
void __cdecl Sim_100781e0(void);
/*
 * For each of the first [0x104737e0] dwords of the table at 0x1017ee40 that is non-zero, counts
 * 0x400 times in an empty loop (only costs time). The loop counter is volatile to keep that.
 */
void __cdecl Sim_100797f2(void);

#endif
