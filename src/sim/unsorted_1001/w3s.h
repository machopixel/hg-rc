/*
 * Wave 3S, original 0x10012843. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1001_W3S_H
#define HG_SIM_UNSORTED_1001_W3S_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Group `index` of the 0x6c-byte table at 0x109b7d40 (dword 0 = the current record, see
 * Sim_10052aaf). Takes the records Sim_10016cb7(index, list, 1) lists and picks the first one
 * whose id (+4) differs from the current one and whose word +0x1c has none of the bits 6; if
 * the group lists any record at all, Sim_100526e3(index, id) is called (id = -1 when none was
 * found). Returns id, or -1.
 */
int __cdecl Sim_10012843(int index);

#endif
