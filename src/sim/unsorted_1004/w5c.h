/*
 * Round 5, letter c: functions at 0x100474db..0x1004f0d6 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1004_W5C_H
#define HG_SIM_UNSORTED_1004_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library sprintf (destination, format, ...). */
#define SIM_CALL_100fc780 SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: the C library buffer flush of a stream (character, stream); returns the character or -1. */
#define SIM_CALL_100ff030 SIM_FN(int(__cdecl *)(int, void *), 0x100ff030)
/* Not converted: (destination buffer, character); returns non-zero on failure. */
#define SIM_CALL_1004db5b SIM_FN(int(__cdecl *)(void *, int), 0x1004db5b)
/* Not converted: (value, three dwords of the record at [0x109e3da4], 6); returns a flag. */
#define SIM_CALL_1004d980 SIM_FN(int(__cdecl *)(int, uint32_t, uint32_t, uint32_t, int), 0x1004d980)
/* Not converted: (handle at [0x109b8be0], two arguments of the caller). */
#define SIM_CALL_100e85cb SIM_FN(int(__cdecl *)(uint32_t, uint32_t, uint32_t), 0x100e85cb)
/* Not converted: takes the object; returns flag bits that are or-ed into obj+0x8c. */
#define SIM_CALL_1004ee50 SIM_FN(uint32_t(__cdecl *)(void *), 0x1004ee50)

/* Builds the path in the 0x50-byte buffer at 0x1015ef50: "<dir>\<name>" from the string at 0x1015f050 when it is set and the name has no \ or /, else the name itself; returns the buffer. */
char *__cdecl Sim_100474db(const char *name);
/* Writes a character to the stream at [0x1015f344] like the C library putc (macro): returns 2 when the stream reports an error (-1), else 0. */
int __cdecl Sim_1004dcaa(int c);
/* Writes the byte [0x1015f340] (plus 1 unless [0x1015f348] is 8) to the stream and 0x1004db5b; resets the state. Returns 0, or -1 on a failure. */
int __cdecl Sim_1004e1c6(void);
int __cdecl Sim_1004edca(uint32_t a, uint32_t b);
void __cdecl Sim_1004f0d6(void *obj);

#endif
