/*
 * Small leaf functions of HGSIM.DLL around 0x1001083a..0x1001c102. Names carry
 * the original address because the code alone does not say what the data means.
 */
#ifndef HG_SIM_UNSORTED_1001_W2K_H
#define HG_SIM_UNSORTED_1001_W2K_H

#include <stddef.h>
#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* With non-zero index: sets the dword at byte +2 of the record at 0x1047383a + [0x1013bb40] * 0x4fc3a + index * 0x13f to 0x10000. */
void __cdecl Sim_1001083a(int index);
/*
 * Returns the low 16 bits of sel, except: sel 0x40 gives the low 16 bits of b,
 * 0x25 those of c and 0x2a those of d. Only the low 16 bits of the result are
 * defined (the original leaves the caller's upper half in eax).
 */
unsigned __cdecl Sim_10013fe1(unsigned sel, unsigned b, unsigned c, unsigned d);
/* 1 if the signed 16-bit key is listed in one of the three lists at obj+0x174, +0x178, +0x17c, else 0. A list is a word count followed (at +2) by a pointer to 4-byte entries whose first word is the key. */
int __cdecl Sim_100155b7(const void *obj, int key);
/* 0x109e8b40[index] points to a record; its dword at +0x10, or -1 when the pointer is 0. */
int __cdecl Sim_10015cd2(unsigned index);
/* Does nothing. */
void __cdecl Sim_1001c102(void);

#endif
