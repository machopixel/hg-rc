/*
 * Round 5, letter c: functions at 0x100ba5d1..0x100bee33 that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_100B_W5C_H
#define HG_SIM_UNSORTED_100B_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (handle, address of a function of the DLL). */
#define SIM_CALL_100bd69c SIM_FN(void(__cdecl *)(uint32_t, const void *), 0x100bd69c)
#define SIM_CALL_100bd134 SIM_FN(void(__cdecl *)(void), 0x100bd134)
/* Not converted: takes the path; returns non-zero on success. */
#define SIM_CALL_100bcd61 SIM_FN(int(__cdecl *)(const char *), 0x100bcd61)
/* Not converted: (path, mode string); returns the opened stream or NULL. */
#define SIM_CALL_100fd850 SIM_FN(uint32_t(__cdecl *)(const char *, const char *), 0x100fd850)
/* Not converted: (object, word, word, flag); the words are read as 16-bit values. */
#define SIM_CALL_10015630 SIM_FN(uint32_t(__cdecl *)(void *, int16_t, int16_t, int), 0x10015630)

/* Releases the two handles at 0x1017d604 and 0x1017d600 (each with its own function of the DLL as the second argument) and, if [0x1017d5fc] is set, calls 0x100bd134. */
void __cdecl Sim_100ba5d1(void);
/*
 * Stores the byte and the dword in 0x1017e1b8 / 0x1017e1cc, then works on `path`: changes its extension (Path_ReplaceExtension) to the
 * strings at 0x10151554, calls 0x100bcd61, changes it to the one at 0x10151558, opens it with the mode string at 0x1015155c
 * into [0x1017e1c0] and calls Sim_100bcc50. Returns 1 when everything worked, else 0.
 */
int __cdecl Sim_100bcbcf(char *path, uint32_t value, int byte);
/*
 * If the dword at obj+0x1fc is not -1: it is either -2 (then 0x10015630(obj, 4, word at +0x1aa, 0) is called and the dword at +0x1ae set to 1)
 * or a word that is returned; the dword is set to -1. Returns 0xffff or that word. The original leaves garbage in the top half of eax: the
 * top half of the object pointer, or of the return value of 0x10015630; that is kept, because a caller could read all of eax.
 */
uint32_t __cdecl Sim_100bee33(void *obj);

#endif
