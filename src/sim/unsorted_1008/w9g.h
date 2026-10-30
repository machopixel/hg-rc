/*
 * Round 9, letter g: the function of HGSIM.DLL at 0x10086ebf (applies the NMSAVE.CFG record of w8w_nodes.c).
 */
#ifndef HG_SIM_UNSORTED_1008_W9G_H
#define HG_SIM_UNSORTED_1008_W9G_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* ---- w9g_nmsave.c ---- */
/*
 * When obj's dword +0x16 (a word table) is set: reads the NMSAVE.CFG record with Sim_10086e27 into a local
 * 0x6c-byte record (its result is not checked: without the file the record is whatever the stack held, as in
 * the original) and copies it into obj: dword +0 to +0xc, +4 (the count) to +0x12, +0x10 (the item table,
 * 0x1e bytes each) to +0x126, the text +0x26 to +0x1e, 0x12 bytes from +0x14 to +0x12a; then the word table
 * gets the word +0x18 of every item (count of them) and the word +0x10 is the table's entry at the record's
 * dword +8.
 */
void __cdecl Sim_10086ebf(void *obj);

#endif
