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
 * Loads the library (Sim_10045ec2); on failure prints the string at 0x1013afe0 with OutputDebugStringA and returns 0.
 * Otherwise returns the result of the stdcall function stored in dword 0x1015ef40 called with (a, b), or 0 when that
 * dword is 0.
 */
int __cdecl Sim_10045d70(uint32_t a, uint32_t b);

/*
 * When dword 0x1015ef3c is set: calls the stdcall function in dword 0x1015ef48 with the string at 0x1013af00 and, for a
 * negative result, formats it with wsprintfA(format 0x1013affc) and prints it with OutputDebugStringA; returns the result.
 * Returns 0x8004050f when the function pointer is 0, and 0 when 0x1015ef3c is 0.
 */
int __cdecl Sim_10045db9(void);

/*
 * Looks the three exports up in the module with GetProcAddress (names at 0x1013b050, 0x1013b060, 0x1013b070) and stores
 * them in 0x1015ef40, 0x1015ef44, 0x1015ef48. Returns 1 when all three were found, else 0.
 */
int __cdecl Sim_10045e5d(void *module);

/*
 * Finds the library whose 11 byte name is at 0x1013b080 (GetModuleHandleA, then LoadLibraryA). Returns the module when
 * it is above 0x20 and Sim_10045e5d found its exports in it, else 0.
 */
void *__cdecl Sim_10045ec2(void);

/* For index >= 0 Sim_1004821c(index); for a negative index Sim_1004821c(0..6). */
void __cdecl Sim_100481dd(int index);

/*
 * Releases the record in entry `index` of the pointer table at 0x109b94e0: when its dword +0x18 is set it calls
 * Sim_1002828e(dword +0x14 + dword 0x101c64f0, dword 0x1014cec0); frees the record and clears the entry.
 */
void __cdecl Sim_1004821c(int index);

#endif
