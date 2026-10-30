/*
 * Converted functions of HGSIM.DLL from around 0x10008000..0x1000f000 that
 * only call functions already converted. Their names carry the original
 * address because the code alone does not say what the fields mean.
 */
#ifndef HG_SIM_UNSORTED_1000_H
#define HG_SIM_UNSORTED_1000_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Sim_10008c69(obj, obj[0x2c]) */
void __cdecl Sim_10008c51(void *obj);
/* Sim_10008c69(obj, v) with v = obj[0x30][0xc] when obj[0x30] is set and that dword is not 0, else obj[0x2c]. */
void __cdecl Sim_10008c77(void *obj);
/*
 * obj[0x28] and the entry of the table at 0x109e8b40 selected by [0x1017b8c8]
 * point to records with a 16-bit word at +0x560. Sets bit 0x80 of the first
 * record's word when Sim_1009c929(100) < obj[0x1e8], and again when obj[0] is
 * 8 or 5.
 */
void __cdecl Sim_1000c172(void *obj);
/* -1 for a null obj; else 1 when obj[0xf0] has bit 0x200 and Sim_10079feb says so for the first dword of table entry (obj[0xf0] & 0xff), else 0. */
int32_t __cdecl Sim_1000c680(const void *obj);
/* Dword at 0x1012df30 + 4 * Sim_1009c929(10). */
uint32_t __cdecl Sim_1000edce(void);

#endif
