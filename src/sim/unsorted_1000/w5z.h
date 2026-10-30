/*
 * Round 5 (letter z): small functions of HGSIM.DLL around 0x10006d75 to 0x1000ed25.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_1000_W5Z_H
#define HG_SIM_UNSORTED_1000_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Calls Sim_100a7758(id, dword 0x1014ceb8) for each id of the dword table at 0x1012da40, up to the entry -1. */
void __cdecl Sim_10006d75(void);

/* Frees the chain of blocks that starts at obj+0x2c (next at +0xc), calls the original 0x1002feef(obj), then frees obj itself. */
void __cdecl Sim_10009369(void *obj);

/* For a non-NULL obj: Sim_1008a16d(obj), then Sim_10009369(obj). */
void __cdecl Sim_10009741(void *obj);

/* Returns 1 when the original 0x1000a6f0(a, 0) returns 0, else 0. */
int __cdecl Sim_1000c5f8(uint32_t a);

/* Passes its seven arguments on to the original 0x1000ea40 unchanged; a5 is a 16-bit value (the callee masks it with 0xffff). */
uint32_t __cdecl Sim_1000ed25(uint32_t a0, uint32_t a1, uint32_t a2, uint32_t a3, uint32_t a4, uint16_t a5, uint32_t a6);

#endif
