/*
 * Round 4, letter v: functions of HGSIM.DLL around 0x10009000..0x1000f000 that call Windows (the heap).
 */
#ifndef HG_SIM_UNSORTED_1000_W4V_H
#define HG_SIM_UNSORTED_1000_W4V_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Removes the node obj[0x30] from the singly linked list whose head is obj[0x2c] (the link is the dword at node+0xc)
 * and frees it. Does nothing when either is NULL or the node is not in the list. obj[0x30] is left as it is.
 */
void __cdecl Sim_100092dd(void *obj);
/*
 * Allocates the zero-filled 0x374-byte block at [0x1015d0a4]. Returns -1 when it exists already, -3 when the
 * allocation fails, else 1. The count argument is stored (limited to 11) at +4 and then replaced by 0, so
 * the four arrays that follow in the original are never filled.
 */
int __cdecl Sim_1000e520(uint32_t count);

#endif
