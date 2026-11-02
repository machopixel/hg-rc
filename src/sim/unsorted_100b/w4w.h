/*
 * Round 4, letter w (area 0x100b3xxx..0x100bdxxx): five 0x9000 byte work buffers, a string trim, and the release of the
 * loaded libraries.
 */
#ifndef HG_SIM_UNSORTED_100B_W4W_H
#define HG_SIM_UNSORTED_100B_W4W_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Frees the five buffers (dwords 0x1017d214, 0x1017d204, 0x1017d1f4, 0x1017d208, 0x1017d1f8; each when set, then cleared)
 * and clears the dwords 0x1017d20c, 0x1017d210, 0x101a1160, 0x101c42a4, 0x101c42a0 and 0x101a1168.
 */
void __cdecl Sim_100b372b(void);

/*
 * Sim_100b372b(), then allocates the five 0x9000 byte zeroed buffers and sets 0x1017d210 = 0x1200,
 * 0x101a1160 = 0x101c42a4 = 0x2400 once the first buffer exists. Returns 1 when all five were allocated, else 0.
 */
int __cdecl Sim_100b35b0(void);

/* Removes the trailing space, CR, LF and TAB characters of the string in place (index 0 is never examined). */
void __cdecl Sim_100ba285(char *s);

/* Did FreeLibrary on the archive's unpacker DLLs (handles at 0x1017dfd0 + i * 32); does nothing now (Sim_100bcc50). */
void __cdecl Sim_100bd1b6(void);

#endif
