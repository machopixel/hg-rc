/*
 * Round 5, letter a: functions of 0x10031917..0x1003ff72 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1003_W5A_H
#define HG_SIM_UNSORTED_1003_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sprintf (destination, format, ...). */
#define W5A_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: no arguments (both). */
#define W5A_CALL_100345e0 SIM_FN(int(__cdecl *)(void), 0x100345e0)
#define W5A_CALL_100345fc SIM_FN(int(__cdecl *)(void), 0x100345fc)
/* Not converted: one dword. */
#define W5A_CALL_1004954e SIM_FN(int(__cdecl *)(uint32_t), 0x1004954e)

/*
 * Frees a tree of blocks: when the dword p + 0x18 is 0 first the four blocks whose pointers are the
 * dwords p + 0x1c..0x28 (each the same way), then HeapFree(p). Nothing for a null p.
 */
void __cdecl Sim_10031917(void *p);
/*
 * Copies the first min(strlen(s), 0x50) characters of s to every second byte from 0xb0f00 (the text screen,
 * character cells of 0xb0000; the attribute bytes between are not touched), after calling 0x100345e0() and
 * 0x100345fc().
 */
void __cdecl Sim_1003462f(const char *s);
/*
 * Does nothing for a null obj. With d = [0x1015dc18]: when c == d - 0x31, or c == obj[0x10] and d == 0xd:
 * clears the byte [[0x1017b8c4] + 1], calls 0x1004954e(0x76) with the dword [0x1015ebbc] set to 0 around
 * it, and Sim_10074c8f(). The arguments a, b, d and e are not used.
 */
void __cdecl Sim_1003ce00(uint32_t a, uint32_t b, int c, uint32_t d, uint32_t e, void *obj);
/*
 * For a non-empty slot idx of the pointer table 0x109e8b40: clears the byte obj + 0x51, formats
 * "%s: %s" with the name at slot + 0x124 and obj + 2 into a 100-byte buffer, passes it to
 * Sim_1007df6c(text, [0x109b98c0], 0x43e, 0x32) and calls Sim_1000ebfb("MECHUDON", 0, 0, 5, 0x32).
 */
void __cdecl Sim_1003ff72(void *obj, int idx);

#endif
