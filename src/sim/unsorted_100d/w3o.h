/*
 * Round 3 (letter o): 0x100d2a7b.
 */
#ifndef HG_SIM_UNSORTED_100D_W3O_H
#define HG_SIM_UNSORTED_100D_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * Classifies the object (word flags at +0, centre floats +0x44, +0x48, +0x4c, radius +0x50) against the
 * camera planes (matrix floats at 0x10185c20..0x10185c44, near distance 0x10185c50, camera record
 * [0x1014cae0] with floats at +0x18, +0xa8, +0xb0, +0xac). Returns 1 when flag 0x1000 is set, 4 when the
 * object is in front of the near plane (also stores the centre depth in the float 0x101c7aa0), 6 or 7
 * when it lies outside the horizontal / vertical side, else 0.
 */
uint32_t __cdecl Sim_100d2a7b(void *obj);

#endif
