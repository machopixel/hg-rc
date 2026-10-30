/*
 * Wave 3S, original 0x10029f50..0x1002db2d: calls of the callbacks stored in the records of the
 * pointer table at 0x109e8b40 (count in the dword 0x1015dc64; the current record is the one
 * the dword 0x1017b8c8 selects), and a search over the objects of a grid cell. Plain C,
 * original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1002_W3S_H
#define HG_SIM_UNSORTED_1002_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* For every record whose callback at +0x30 and whose dword +0x28 are both non-zero: calls callback(record). */
void __cdecl Sim_10029f50(void);
/* For every record whose callback at +0x38 and whose dword +0x28 are both non-zero: calls callback(dword +0x28). */
void __cdecl Sim_1002a019(void);
/* For the current record, when it and its callback at +0x3c and its dword +0x28 are non-zero: calls callback(dword +0x28). */
void __cdecl Sim_1002a086(void);
/* For every record whose callback at +0x44 is non-zero: calls callback(dword +0x28), whatever that is. */
void __cdecl Sim_1002a10e(void);

/*
 * Picks the grid cell (dword [0x1017c8cc] when [0x1017c8c4] is 0, else Sim_10088775(x, z)); with
 * no cell returns the float at 0x101112c4. Otherwise goes through the objects of the cell
 * (chain from cell+0x10, next at +0x10), and for each one of type (+0x34) 5, 8 or 0 calls the
 * function 0x100035df, 0x10001e1f or 0x10002775 respectively as f(object, x, y, z, &result);
 * whenever it returns non-zero and result is above the best so far (start -3.4e38, the float at
 * 0x101112cc) the best is replaced and the three dwords 0x1015dd40.. are copied to 0x1015dd4c...
 * Returns the best, or 0.0f when it is not above 0x101112cc. The float comes back in st(0).
 */
float __cdecl Sim_1002db2d(float x, float y, float z);

#endif
