/*
 * Round 3 (letter t) functions of HGSIM.DLL in 0x1004xxxx. Plain C, original
 * cdecl convention. Names carry the original address because the code alone
 * does not say what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1004_W3T_H
#define HG_SIM_UNSORTED_1004_W3T_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Calls the method at +0x18 of the display driver object whose pointer is at
 * 0x1015e688 with the pointer 0x109b8c00, while the dword 0x1013b98c holds
 * `value` (the old value is put back afterwards).
 */
void __cdecl Sim_1004b7d2(int value);
/* Round 25: when set (by the hook layer), Sim_1004b7d2 also rebuilds the 3D texture palette (Sim_100d3000). */
extern int g_simLiveBrightness;

/*
 * a / b (ints, as float) compared with fixed limits. Returns in st(0): 0.25
 * above 1.26, else 0.5 above 1.2, 0.6 above 1.15, 0.75 above 1.1, else 1.0.
 */
float __cdecl Sim_1004bc65(int a, int b);

/* a / b (ints, as float): -3 when a == b, -2 from 0.75 up, -1 from 0.5 up, else 0. */
int __cdecl Sim_1004bec6(int a, int b);

/*
 * Table 0xaa bytes per `index` (dword at +0x101116b2) plus the dwords at +0 of the
 * 0x98-byte table 0x10124eb8, selected by the three fields of the packed word
 * (Sim_1004bf79, Sim_1004bf87, Sim_1004bf98); as float times the three floats at
 * +4 of the same selected entries (0x10124ebc), each product rounded to
 * float, the last one truncated.
 */
int __cdecl Sim_1004bfbd(int index, int word);

/*
 * The dword at 0x101116ba + 0xaa * index plus the dwords at +0 of the three
 * entries of the table 0x10124ec4 (stride 0x98) selected by the fields of
 * `word`, plus Sim_1004bec6(ratio, the dword at 0x101116ae + 0xaa * index).
 */
int __cdecl Sim_1004c0e5(int index, int word, int ratio);

#endif
