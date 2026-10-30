/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x100cxxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100C_W2F_H
#define HG_SIM_UNSORTED_100C_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Stores the dword at obj+0x25a: the dword 0x1015dc78 plus 10 when obj+4 is
 * the current index 0x1017b8c8, or when obj+0 is 1 and obj+0x27c is 0x10151b40
 * or bit 0 of the word at +0x562 of the structure at obj+0x28 is set; plus 0x14
 * when obj+0 is 0xa; otherwise plus 10 if obj+0x24a is non-zero, else 90.
 */
void __cdecl Sim_100c275b(void *obj);
/*
 * w holds a table number in its bits 8..11 (0x200: the pointer table 0x109e8b40,
 * 0x400: the 0x50-byte entries at 0x109e9ba8, whose dword +0 is an index for
 * Sim_100b5386) and an index in its low byte; returns the dword +0x8c of the
 * object found, or 0 when there is none.
 */
uint32_t __cdecl Sim_100c38fe(uint16_t w);
/* Same lookup, taking the selector from the dword +0xf0 of the current object (pointer table entry [0x1017b8c8]); returns the object, or 0. */
uint32_t __cdecl Sim_100cba68(void);
/* Returns +0x8c of the object Sim_100cba68 finds (Sim_100a9b71), or 0 when there is none. */
uint32_t __cdecl Sim_100cba42(void);

#endif
