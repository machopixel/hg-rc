/*
 * Round 5, letter c: functions at 0x10029ab3..0x1002fb3d that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1002_W5C_H
#define HG_SIM_UNSORTED_1002_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (key, value). */
#define SIM_CALL_10028fb0 SIM_FN(void(__cdecl *)(uint32_t, uint32_t), 0x10028fb0)
/* Not converted: seven arguments; the last three are out pointers (the middle one receives the success flag). */
#define SIM_CALL_1002ce45 SIM_FN(void(__cdecl *)(uint32_t, uint32_t, uint32_t, uint32_t, uint32_t *, uint32_t *, uint32_t *), 0x1002ce45)
/* Not converted: (argument, six pointers to the dwords of the block just allocated). */
#define SIM_CALL_1002fba5 SIM_FN(void(__cdecl *)(void *, void *, void *, void *, void *, void *, void *), 0x1002fba5)

/* For a non-zero key: copies the 5 dwords at 0x1015dc48 over the first of the 0x80 entries (20 bytes at 0x109f09e0) whose dword +4 equals the key; then calls 0x10028fb0(key, value). */
void __cdecl Sim_10029ab3(uint32_t key, uint32_t value);
/* Calls 0x1002ce45; when it reports success stores its third result in *out and resets the dwords 0x1015dd40..0x1015dd48 (0, 1.0f, 0); returns the success flag. */
int __cdecl Sim_1002cde1(uint32_t a, uint32_t b, uint32_t c, uint32_t d, uint32_t *out);
/* Allocates and clears 0x78 bytes from the game heap and passes them to 0x1002fba5; returns the block (NULL when the allocation failed). */
void *__cdecl Sim_1002fad6(void *arg);
/* Unless bit 9 of the word at obj+0 is set: makes sure obj+0x98 holds a block from Sim_1002fad6 and, if it does, sets that bit. */
void __cdecl Sim_1002fb3d(void *obj);

#endif
