/*
 * Round 4, letter w (area 0x10045xxx..0x10048xxx): the network library loaded by name (three exported functions kept
 * at 0x1015ef40, 0x1015ef44 and 0x1015ef48; module state in 0x1015ef3c), and the table of seven records at 0x109b94e0.
 */
#ifndef HG_SIM_UNSORTED_1004_W4W_H
#define HG_SIM_UNSORTED_1004_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Force feedback start (FFORCE.DLL's TSFF_InitSystem, now platform/force.h): without a device prints the string at
 * 0x1013afe0 ("Error loading TS_SFRCE.dll") as debug text and returns 0, as the original did without the DLL;
 * otherwise Force_Init(a, b).
 */
int __cdecl Sim_10045d70(uint32_t a, uint32_t b);

/*
 * When dword 0x1015ef3c is set: Force_Effect with the request table at 0x1013af00 and, for a negative result, the
 * text of format 0x1013affc as debug text; returns the result. Returns 0x8004050f without a force-feedback device
 * (the original: FFORCE.DLL missing), and 0 when 0x1015ef3c is 0.
 */
int __cdecl Sim_10045db9(void);

/* For index >= 0 Sim_1004821c(index); for a negative index Sim_1004821c(0..6). */
void __cdecl Sim_100481dd(int index);

/*
 * Releases the record in entry `index` of the pointer table at 0x109b94e0: when its dword +0x18 is set it calls
 * Sim_1002828e(dword +0x14 + dword 0x101c64f0, dword 0x1014cec0); frees the record and clears the entry.
 */
void __cdecl Sim_1004821c(int index);

#endif
