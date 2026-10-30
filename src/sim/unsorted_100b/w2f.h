/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x100bxxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100B_W2F_H
#define HG_SIM_UNSORTED_100B_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Does nothing. */
void __cdecl Sim_100b3f8f(void);
/* Of the 0x88-byte entries at 0x101a2100, returns the dword +0x2c of entry i, or 0 unless 0 <= i < the signed dword 0x1017d218. */
uint32_t __cdecl Sim_100b5386(int i);
/* Returns the dword +0x24 of entry i of the same table when 0 <= i <= the dword 0x1017d218 and bit 0x400 of its dword +0x18 is set, else -1. */
int __cdecl Sim_100b54b3(int i);
/* Returns -1. */
int __cdecl Sim_100b8d5c(void);
/*
 * list: +0 capacity, +4 count, +8 pointer to an array of dwords. Removes the
 * first element equal to val, shifting the later ones down, and decrements the
 * count. Returns 1 if it found one, else 0.
 */
int __cdecl Array_RemoveValue(void *list, uint32_t val);
/* Sets the count (+4) to 0 when list is not null and zeroes the first capacity (+0) elements. */
void __cdecl Array_Clear(void *list);

#endif
