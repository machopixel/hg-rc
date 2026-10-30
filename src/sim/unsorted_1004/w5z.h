/*
 * Round 5 (letter z): small functions of HGSIM.DLL at 0x1004db2e to 0x1004f158.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1004_W5Z_H
#define HG_SIM_UNSORTED_1004_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Stores the original 0x100fd850(name, string at 0x1013b9a0) in dword 0x1015f344; returns 1 when that is 0, else 0. */
int __cdecl Sim_1004db2e(const char *name);

/* Returns 2 when the original 0x100febc0(buf, 1, count, dword 0x1015f344) returns less than count, else 0. */
int __cdecl Sim_1004db5b(void *buf, uint32_t count);

/*
 * Clears obj+0x94, sets obj+0x90 to the (unaligned) dword at +0x5ff of the block that obj+0x28 points to,
 * and calls the original 0x1004ee50(obj) when dword 0x1015f3a8 is 0.
 */
void __cdecl Sim_1004f158(void *obj);

#endif
