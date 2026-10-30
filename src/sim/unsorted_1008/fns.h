/*
 * Wave 2 function of HGSIM.DLL at 0x1008a220: links an object into the list
 * of its grid cell (or of the object at [0x1017c8cc]) and into the object
 * list with List_InsertBefore.
 */
#ifndef HG_SIM_UNSORTED_1008_H
#define HG_SIM_UNSORTED_1008_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Does nothing unless the object has flag 0x800 (cleared here) and its dword
 * at +0x34 is not 4, and its dword at +4 and [0x1017c8cc] are non-zero.
 * With [0x1017c8c4] set the cell is the one at the 9x9 grid position given by
 * the dwords at +0x8c / +0x90 (0x1017ba28 + 0x18c * x + 0x2c * y; outside the
 * grid the entry at 0x1017b9b0); otherwise the cell is the object at
 * [0x1017c8cc]. The object becomes the head of the cell's chain (cell +0x10,
 * object +0x10 / +0xc) and is then passed to List_InsertBefore before the cell.
 */
void __cdecl Sim_1008a220(uint16_t *obj);

#endif
