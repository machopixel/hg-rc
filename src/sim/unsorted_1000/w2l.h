/*
 * Wave 2l: small functions around 0x1000716a..0x1000ede4: field accessors,
 * table lookups and the release of an object (Sim_10009787 unlinks it with the
 * link functions of view/w2l.h). Names carry the original address because the
 * code alone does not say what the objects are. Plain C, original cdecl
 * convention.
 */
#ifndef HG_SIM_UNSORTED_1000_W2L_H
#define HG_SIM_UNSORTED_1000_W2L_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Entry `index` (short) of the 14-byte table at 0x10ac1b00; value is taken as an unsigned short, 0 if it
 * is 0x20 or more. When the entry's short at +8 is not negative and the byte at 0x10a51ac0 + 0x300 * (short
 * at +0) + 0x18 * value is non-zero: the entry's short at +2 = value and its dword at +0xa = 0.
 */
void __cdecl Sim_1000716a(int index, unsigned value);
/* The unsigned short at obj+0x26. */
uint32_t __cdecl Sim_1000944e(const void *obj);
/*
 * p = dword obj+0x30. When p is non-zero and index < short p+6: the short at p + dword (p+8) + 0x28 * index
 * = value (low 16 bits).
 */
void __cdecl Sim_100096b2(void *obj, int index, unsigned value);
/* The unsigned short at p, masked with 0x10f. */
uint32_t __cdecl Sim_10009707(const void *p);
/* For a non-NULL obj: Sim_1008a16d(obj), then Sim_1008a14f(obj). */
void __cdecl Sim_10009764(void *obj);
/*
 * owner = Sim_100096ee(obj); when non-zero Sim_100a9b4c(owner, 0). Clears the dword 0x1017e678 when it is obj.
 * Then Sim_10009764(obj).
 */
void __cdecl Sim_10009787(void *obj);
/*
 * 1 when entry `index` of the 0x128-byte array at *(obj+0x84) points (by its dword +4) to a record of the
 * 0xd6-byte table at 0x10145508 whose dword +0x10 is 0x3d, 0x42 or 0x43; 0 otherwise and for a NULL obj.
 */
int __cdecl Sim_1000c813(const void *obj, int index);
/* When the dword 0x1015d0a4 is non-zero reads the 16 dwords from that address + 0x54 and does nothing with them. */
void __cdecl Sim_1000ede4(void);

#endif
