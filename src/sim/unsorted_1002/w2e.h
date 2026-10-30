/*
 * Wave 2e, original 0x10021ac0..0x10028981: initialisers of two tables, thin
 * wrappers around the library routines of unsorted_100f/w2e.h and an object
 * initialiser. Names carry the original address where the code does not say
 * what the data is for.
 */
#ifndef HG_SIM_UNSORTED_1002_W2E_H
#define HG_SIM_UNSORTED_1002_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Resets the 0x200 records of 0x156 bytes at 0x1096fc80 (dword +0 = -1, +0x3c = 0, then
 * Sim_10021c3c) and the 0x100 records of 0x2c bytes at 0x109a5a40 (see the code for
 * the fields, then Sim_10021be0), zeroes six counters at 0x1015dbd8..0x1015dbf0 and the
 * 0x1d4 bytes at 0x101653e0.
 */
void __cdecl Sim_10021ac0(void);
/* Clears the fields +4, +8, +0xc, +0x10, +0x14, +0x18, +0x20 of record `index` of the 0x2c-byte table at 0x109a5a40. */
void __cdecl Sim_10021be0(int index);
/* Resets record `index` of the 0x156-byte table at 0x1096fc80 to its initial values. */
void __cdecl Sim_10021c3c(int index);
/* Clears the 0x1d4 bytes at 0x101653e0. */
void __cdecl Sim_10025d93(void);
/* Mem_Move(dst, src, n), returns dst. */
void *__cdecl Sim_10028571(void *dst, const void *src, uint32_t n);
/* Mem_Fill(dst, c, n), returns dst. */
void *__cdecl Sim_1002858a(void *dst, int c, uint32_t n);
/*
 * Initialises a 0x148-byte object: +0 points to itself, a 0x20-byte block at +0x10
 * is cleared, and +0x104..+0x134 receive the addresses of thirteen functions of the DLL.
 */
void __cdecl Sim_10028810(void *obj);
/* Copies at most 0x20 bytes of the string to obj+0x10 (strncpy rules) and ends it with a 0 at obj+0x2f. */
void __cdecl Sim_10028981(void *obj, const char *s);

#endif
