/* Wave 2e, original 0x100e2837. */
#ifndef HG_SIM_UNSORTED_100E_W2E_H
#define HG_SIM_UNSORTED_100E_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * The lookup that goes with Sim_100da4f7 (unsorted_100d/w2e.h): finds the node with the string
 * `name` in the structure of 27 lists in `table`. The key is the first four bytes of the name,
 * compared as a signed int with the key at node+0x10. Returns the node or 0.
 */
void *__cdecl Sim_100e2837(const char *name, void *table);

#endif
