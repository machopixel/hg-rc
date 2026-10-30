/*
 * Round 5 (letter y) functions of HGSIM.DLL in 0x100bxxxx. Plain C, original cdecl convention.
 * They call helpers that are not converted yet through their original address (SIM_FN), so the
 * original code still runs for those. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_100B_W5Y_H
#define HG_SIM_UNSORTED_100B_W5Y_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * table[0] is a count, table[1] points to that many 8-byte entries (text pointer, value) sorted for string compare
 * (the compare function Sim_100b6b87 gives strcmp of the two text pointers; the search is bsearch through
 * Sim_10100620). key is a text pointer. Returns the value of the entry whose text equals key, otherwise key itself;
 * with a NULL table returns key.
 */
uint32_t __cdecl Sim_100b7834(const uint32_t *table, uint32_t key);

#endif
