/*
 * Round 4, letter w (area 0x10038xxx..0x1003exxx): the singly linked list of 0x698 byte records at dword 0x109e3760
 * (head) / 0x109e3770 (tail; next pointer at +0x690, previous at +0x694), a counted table block, and the table of
 * library handles at 0x1015ea8c.
 */
#ifndef HG_SIM_UNSORTED_1003_W4W_H
#define HG_SIM_UNSORTED_1003_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Allocates a block of 0x2c + count*4 bytes (not cleared by the heap), stores the six arguments at +0..+0x14 and the count at +0x18, clears
 * +0x1c..+0x2b and, when count > 0, the count dwords after that. Returns the block. When the allocation fails it prints
 * the string at 0x10138c44 with Mono_PrintText and returns 0.
 */
void *__cdecl Sim_1003184b(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t e, uint32_t f,
                           int count);

/* Allocates the first record (0x698 bytes), makes it head and tail (0x109e3760, 0x109e3770) and clears its links and +8. */
void __cdecl Sim_10038ccd(void);

/* Frees every record of the list that starts at dword 0x109e3760 (the head dword itself is left unchanged). */
void __cdecl Sim_10039202(void);

/* Sim_10038ccd(). */
void __cdecl Sim_10039241(void);

/* Sim_10039202(). */
void __cdecl Sim_1003924b(void);

/*
 * Clears the 55 dwords at 0x1015ea8c..0x1015eb64, then calls FreeLibrary on the module in dword 0x1015eb68 until it
 * returns 0, and clears that dword.
 */
void __cdecl Sim_1003c313(void);

/*
 * Calls Sim_100420a0(); when dword 0x1015eccc is not 0 frees the blocks in 0x1015ec88, 0x1015ecc4, 0x1015ec90,
 * 0x1015ec94, 0x1015d084 and 0x1015ecc8 (each when not 0) and then the block in 0x1015d008 (HeapFree is called for it
 * even when it is 0). The dwords themselves are not cleared.
 */
void __cdecl Sim_1003edda(void);

#endif
