/*
 * Wave 2 (letter i) additions to unsorted_1002: state of the table at 0x1096fcc0, empty functions and a table setup around 0x10025000..0x1002e000.
 * Plain C, original cdecl convention. Names carry the original address
 * because the code alone does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1002_W2I_H
#define HG_SIM_UNSORTED_1002_W2I_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Entries of 0x156 bytes from 0x1096fcc0 (index in dwords 0x1013488c, current
 * selection in 0x10134890, -1 = none).
 * Sim_100258d7: when the selection is not -1 and its entry's dword at +0x20
 * is non-zero returns the address 0x1015dbd8; otherwise sets the selection to
 * -1 and returns NULL.
 */
void *__cdecl Sim_100258d7(void);
/*
 * Sim_1002590c: with c = [0x1013488c]: when c > 0, the entry's dword at +8 is
 * non-zero and its dword at +0x10 equals [0x1017b8c8], selects c, sets the
 * entry's dword at +0x20 to 1 and returns 1; otherwise sets the selection to
 * -1 and returns 0.
 */
int __cdecl Sim_1002590c(void);

void __cdecl Sim_1002850e(void);               /* does nothing */
int __cdecl Sim_10028513(int a);               /* returns its argument */
int __cdecl Sim_1002895a(int a);               /* returns its argument */
void __cdecl Sim_10028a74(void *obj, int16_t value); /* the 16-bit word at +6 of obj */
/* Zeroes the dwords at +0, 4, 8, 0xc, 0x10, 0x14, 0x18, 0x1c, 0x20 of entry `index` of the 0x30-byte table at 0x109eeb00. */
void __cdecl Sim_1002990f(int index);
/*
 * Loads the working tables for layout `which` (0 or 1) of the preset in
 * [0x101c64f0]: set = which + 2 * preset selects 0x258 bytes at
 * 0x101355c0 (copied to 0x109e8660), 0xf0 bytes at 0x10135f20 (to 0x109e8240)
 * and 0x770 bytes at 0x101362e0 (a pointer to each 16-byte piece goes to
 * 0x109e8480). Also stores constants and four rect numbers at 0x10145290.
 */
void __cdecl Sim_1002ba28(int which);
int __cdecl Sim_1002d263(int a, int b);        /* returns 0 */

#endif
