/*
 * Wave 2g: leaf functions of HGSIM.DLL from 0x10010145..0x1001c618. Names
 * carry the original address because the code alone does not say what the
 * objects are.
 */
#ifndef HG_SIM_W2G_1001_H
#define HG_SIM_W2G_1001_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* When both pointers are non-null and p = b[2] (dword at +8) is non-null: *p = the dword at 0x10149a90. */
void __cdecl Sim_10010145(const void *a, const void *b);
/* The word at obj + 0x198 = 0; returns 1. The second argument is not used. */
int __cdecl Sim_10013e20(void *obj, int unused);
/*
 * The word at 0x10473929 + 0x4fc3a * a + 0x13f * (int16)w[a] + 2 * c,
 * where w is the word table at 0x104727a0 (4 bytes per entry, the word is at
 * the start); the top half of c is returned unchanged above the word. The
 * second argument is not used.
 */
uint32_t __cdecl Sim_10016c73(int a, int unused, uint32_t c);
/* 1 when the low byte of v is one of 4..8, 13, 17, 21, 23, 24, 25, else 0. */
int __cdecl Sim_1001c618(uint32_t v);

#endif
