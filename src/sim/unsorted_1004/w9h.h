/*
 * Round 9, letter h: the per-record owner refresh (0x10042a11) and a table lookup (0x1004d344).
 */
#ifndef HG_SIM_UNSORTED_1004_W9H_H
#define HG_SIM_UNSORTED_1004_W9H_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9h_misc.c ---- */
/*
 * Every record PTRTAB(i), i < [0x1015dc64], with dword +0x14 == 3 gets +0xc = [0x1017b8c8]; after Sim_10010f70() each
 * such record gets +0xc = idx with idx = Sim_10052aaf(rec +8), or Sim_10012843(rec +8) when that is -1, or [0x1017b8c8]
 * when both are; when PTRTAB(idx) +0x14 is neither 0 nor 1, idx = Sim_1003db56(word [0x1015ecd8]) instead.
 */
void __cdecl Sim_10042a11(void);
/* The dword at 0x101116a6 + index * 0xaa. */
int32_t __cdecl Sim_1004d344(int index);

#endif
