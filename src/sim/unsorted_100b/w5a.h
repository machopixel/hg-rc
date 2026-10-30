/*
 * Round 5, letter a: a function of 0x100b4342 that calls a helper which is not converted yet.
 * The helper is called through its address (SIM_FN) with the prototype below.
 */
#ifndef HG_SIM_UNSORTED_100B_W5A_H
#define HG_SIM_UNSORTED_100B_W5A_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: no arguments. */
#define W5A_CALL_100b469d SIM_FN(int(__cdecl *)(void), 0x100b469d)

/*
 * Entry a of the 0x88-byte entries at 0x101a2100 (count [0x1017d218]; nothing for a out of range): bit 0x400
 * is set in its dword +0x18, bit 0x200 in +0x1c, and +0x20 and +0x24 become the signed 16-bit values b and c.
 * When b is not negative the entry b (not range checked) gets bit 0x800 in its dword +0x18 and 0x100b469d()
 * is called.
 */
void __cdecl Sim_100b4342(int a, int b, int c);

#endif
