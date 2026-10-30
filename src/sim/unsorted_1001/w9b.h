/*
 * Round 9, letter b, unsorted_1001: the table of loaded animation records (0x18 bytes each: data pointer, state,
 * name) and the list that registers the standard animation names at start-up.
 */
#ifndef HG_SIM_UNSORTED_1001_W9B_H
#define HG_SIM_UNSORTED_1001_W9B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The record of an animation id: bits 12..19 pick the table, bits 0..7 (and for the first two tables bits 8..11
 * times 26) the row. Tables: 0 -> 0x109f4240, 0x1000 -> 0x10a37b80 (78 rows of 0x18); 0x2000 and 0x3000 -> one
 * of seven / six fixed records for the rows 0, 4, 5, 9, 0xa, 0xe, 0x12; 0x4000 -> 0x10a382e0, 0x5000 -> 0x10a382f8.
 * 0 for any other id.
 */
uint8_t *__cdecl Anim_RecordOfId(int id);
/*
 * Registers the standard animation names with Sim_1001bb1f (name, id, reload): the 66 gear animations when
 * [0x1017e800] is set, the two spin animations for [0x1017e7ec] / [0x1017e7f0], the 0x3xxx and 0x2xxx stands for
 * [0x1017e7f8] / [0x1017e7fc] and the 48 0x1xxx animations for [0x1017e7f4].
 */
void __cdecl Anim_RegisterNames(void);

#endif
