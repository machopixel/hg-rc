/*
 * Wave 2b: functions around 0x10033000..0x10039000 that call already
 * converted functions (entry table accessors, the preset selector, an object
 * list). Names carry the original address because the code alone does not say
 * what the objects are.
 */
#ifndef HG_SIM_UNSORTED_1003_W2B_H
#define HG_SIM_UNSORTED_1003_W2B_H

#include "fns.h"
#include "../unsorted_100d/fns.h"
#include "../util/util.h"

/* The entry of the table (count [0x109e4cc0], 0x44 bytes each from 0x109e4cc4) whose Sim_10033580 value is `value`; NULL if none. */
unsigned char *__cdecl Sim_10033c67(int value);
/* Same search, returns the entry's index, or -1. */
int __cdecl Sim_10033ced(int value);
/* Sim_100d32e5(0). */
void __cdecl Sim_10035469(void);
/*
 * Finds the object in the list at [0x109e3760] (next pointer at +0x690) whose
 * first dword equals `id` and copies Sim_100a9cfb(id) into its +0x680..+0x688.
 */
void __cdecl Sim_10038d65(int id);

#endif
