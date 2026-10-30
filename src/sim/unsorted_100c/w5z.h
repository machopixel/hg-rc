/*
 * Round 5 (letter z): small functions of HGSIM.DLL at 0x100c4d63 to 0x100cbdea.
 * Calls of functions that are not converted yet go through their original
 * address (SIM_FN); the others call the converted C functions by name.
 */
#ifndef HG_SIM_UNSORTED_100C_W5Z_H
#define HG_SIM_UNSORTED_100C_W5Z_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* 0 for NULL; else the original 0x100c4b67(p), or when that is 0 the original 0x100c4a1f(p). */
uint32_t __cdecl Sim_100c4d63(void *p);

/* Returns the original 0x100ca670(a, b, flags) where flags is 1, or 0x101 when c is not 0. */
uint32_t __cdecl Sim_100cbdb8(uint32_t a, uint32_t b, int c);

/* Returns the original 0x100ca670(entry [dword 0x1017b8c8] of the pointer table at 0x109e8b40, a, 4). */
uint32_t __cdecl Sim_100cbdea(uint32_t a);

#endif
