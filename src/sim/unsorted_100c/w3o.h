/*
 * Round 3 (letter o): 0x100c476c and the list walk 0x100cc863 / 0x100cc887.
 */
#ifndef HG_SIM_UNSORTED_100C_W3O_H
#define HG_SIM_UNSORTED_100C_W3O_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif


/*
 * Like the sweep in unsorted_1001/w3o.h over the records at 0x1096fc80: returns 1 if some record with a
 * non-zero dword at +0x3c whose +0x50 maps (Sim_10052be7) to a different value than obj+4 does has
 * dist <= the float at 0x1012bbc0 (dist = Sim_10084a94 of obj+0x58.. minus the dwords Sim_100a9cfb reads
 * from the object at +0x3c) and rec+0x18.. dotted with that difference greater than the float at 0x1012bb18.
 */
int __cdecl Sim_100c476c(void *obj);

/* Calls the function stored at node+0 with (-1, 0, [0x1015dc78], node+8). `list` is unused. */
void __cdecl Sim_100cc863(void *list, void *node);

/* Sim_100cc863(list, node) for every node of the chain that starts at *list (next at +0x14). */
void __cdecl Sim_100cc887(void *list);

#endif
