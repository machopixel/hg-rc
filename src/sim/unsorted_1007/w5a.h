/*
 * Round 5, letter a: functions of 0x10074c8f..0x1007d8ba that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1007_W5A_H
#define HG_SIM_UNSORTED_1007_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: the C library's sprintf (destination, format, ...). */
#define W5A_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)
/* Not converted: one dword (a node of the list at [0x1015dc14]). */
#define W5A_CALL_10075124 SIM_FN(int(__cdecl *)(uint32_t), 0x10075124)

/*
 * Walks the list whose head is the dword [0x1015dc14] (next pointer at +0x14): for every node
 * 0x10075124(node), then HeapFree(game heap, 1, node). Then clears [0x1015dc14] and [0x1015dc10].
 */
void __cdecl Sim_10074c8f(void);
/*
 * Fills the record obj (the first argument is not used): +0x58 = 2, +0x5c = 3, the strings "mouse" at +0
 * (sprintf with that format) and +0x4c, the text [0x101c49d0] used as a format at +0xc, the four pointers
 * 0x10179d38, 0x10179d50, 0x10179d40, 0x10179d58 at +0x60, +0x64, +0x68, +0x6c and +0x70 = 0. Returns 0.
 */
int __cdecl Sim_1007d8ba(uint32_t unused, uint8_t *obj);

#endif
