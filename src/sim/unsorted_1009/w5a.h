/*
 * Round 5, letter a: functions of 0x10098f30..0x1009d8b9 that call helpers which are not converted yet.
 * Those helpers are called through their address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1009_W5A_H
#define HG_SIM_UNSORTED_1009_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (value, b, c, d, 0). */
#define W5A_CALL_10098f8c SIM_FN(int(__cdecl *)(int, uint32_t, uint32_t, uint32_t, uint32_t), 0x10098f8c)
/* Not converted: one dword; returns a dword. */
#define W5A_CALL_1009db70 SIM_FN(uint32_t(__cdecl *)(uint32_t), 0x1009db70)

/*
 * With the low nibble n of a: when n is 0 or 3 and bit 0 of [0x1014caf8] is set calls
 * 0x10098f8c((a & 0xff0) >> 4, b, c, d, 0).
 */
void __cdecl Sim_10098f30(uint32_t a, uint32_t b, uint32_t c, uint32_t d);
/*
 * p points to a structure whose first dword is a pointer q (null: nothing); with n = the dword q[1] (a
 * signed index) of at most 0x19: the dword table 0x1017cf78 [n] = 0x1009db70(q[0x12]).
 */
void __cdecl Sim_1009d8b9(const uint32_t *p);

#endif
