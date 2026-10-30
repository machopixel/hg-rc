/*
 * Wave 2 functions of HGSIM.DLL around 0x10074bfa: set or clear the dword at
 * +8 of the list node that Sim_1007652f finds by key.
 */
#ifndef HG_SIM_UNSORTED_1007_H
#define HG_SIM_UNSORTED_1007_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Unless [0x1015dbcc] is set, sets the dword at +8 of the node with this key to 1; returns 1 if a node was found, else 0. */
int __cdecl Sim_10074bfa(int32_t key);
/* Clears the dword at +8 of the node with this key, if there is one. */
void __cdecl Sim_100751e4(int32_t key);

#endif
