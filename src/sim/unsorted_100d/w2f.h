/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x100dxxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100D_W2F_H
#define HG_SIM_UNSORTED_100D_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Stores v in the dwords 0x1017e654 and 0x10152424. */
void __cdecl Sim_100d3861(uint32_t v);
/*
 * obj+0x84 points to 0x128-byte records (+0: id or -1, +4: index into the
 * 0xd6-byte table at 0x10145508, +8, +0xc), obj+0x4f8 is a count and obj+0x4fc
 * the number of the current record. Returns 0 when v is not in 0..2 or the
 * count is not positive; 1 when the current record already has +0xc == v;
 * otherwise looks for the first of the 8 records with a non-negative +4,
 * +0 != -1, +0xc == v, +8 != -1 and the bit with value 4 in the dword +8 of its table entry
 * set, makes it current (obj+0x4fc) and returns 1, else 0.
 */
int __cdecl Sim_100d60b6(void *obj, int v);
/* -1 when obj+0x4fc is -1, else 1 if the dword +8 of the current record (obj+0x84 + 0x128 * obj+0x4fc) is 1, else 0. */
int __cdecl Sim_100d6200(void *obj);

#endif
