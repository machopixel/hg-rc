/*
 * Wave 2b: table lookups that combine a packed word's fields (Sim_1004bf79,
 * Sim_1004bf87, Sim_1004bf98, Sim_1004c30d) with two fixed-stride tables:
 * records of 0xaa bytes from 0x101116a0 and records of 0x98 bytes from 0x10124ea0.
 * Names carry the original address because the code alone does not say what
 * the tables hold.
 */
#ifndef HG_SIM_UNSORTED_1004_W2B_H
#define HG_SIM_UNSORTED_1004_W2B_H

#include "fns.h"
#include "../unsorted_1003/fns.h"
#include "../view/view.h"

/* t1[row].f + t2[nibble0(word)].g + t2[nibble1(word)].g + t2[nibble2(word)].g */
int __cdecl Sim_1004c085(int row, unsigned word);
int __cdecl Sim_1004c161(int row, unsigned word);
int __cdecl Sim_1004c1c1(int row, unsigned word);
int __cdecl Sim_1004c297(int row, unsigned word);
/* t1[row].f + t2[word >> 16].g */
int __cdecl Sim_1004c3ad(int row, int word);
int __cdecl Sim_1004c4a1(int row, int word);
int __cdecl Sim_1004c4d1(int row, int word);

/*
 * id[0] indexes the object pointers at 0x109e8b40, id[1] is the kind. Returns 1
 * unless kind is 2, [0x1015ec58] is non-zero and the key slot that
 * Sim_1003dbc0(id[0]) selects does not hold 0xfa01; then the object decides:
 * 1 if its +0x160 is non-zero, else 1 only when Sim_10079e3b(id) is non-zero,
 * the byte its +0x168 points to is 0 and its +0x164 is 0.
 */
int __cdecl Sim_10042908(const unsigned char *id);

#endif
