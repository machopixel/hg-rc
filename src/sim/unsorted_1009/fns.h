/*
 * Wave 2: small predicates around 0x100988b9..0x100989dd that combine the
 * converted bit-set and flag tests of unsorted_1003 and unsorted_100d.
 * Plain C, original cdecl convention. The original takes one argument it
 * never reads (the Ghidra text calls them fastcall/thiscall; the code uses
 * the stack).
 */
#ifndef HG_SIM_UNSORTED_1009_H
#define HG_SIM_UNSORTED_1009_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* 1 when bit 0x100 or bit 0x200 of the bit set at 0x1014cb38 is clear. */
int __cdecl Sim_100988b9(uint32_t unused);
/* 1 when bit 0x800 of that bit set is clear or the dword 0x1017d1b0 is non-zero. */
int __cdecl Sim_10098966(uint32_t unused);
/* 1 when Sim_100d2c6f() is non-zero or the flag at 0x1014cb34 is zero. */
int __cdecl Sim_100989dd(uint32_t unused);

#endif
