/*
 * Round 5, letter b: functions around 0x100c0558..0x100ccaa0 that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100C_W5B_H
#define HG_SIM_UNSORTED_100C_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. The second argument of 0x1001403a is only ever read as a word. */
#define SIM_CALL_1001403a SIM_FN(int(__cdecl *)(void *, int16_t), 0x1001403a)
/* Takes a double on the stack, returns its absolute value in st(0). */
#define SIM_FABS SIM_FN(double(__cdecl *)(double), 0x100fcb70)
#define SIM_SPRINTF SIM_FN(int(__cdecl *)(char *, const char *, ...), 0x100fc780)

/*
 * Sim_100c48fa(obj, obj[0x218]); 0x1001403a(obj, word obj[0x1aa]); Sim_1000ae00(obj, Sim_10015266(obj)); and, when the dword
 * obj[0x204] is less than [0x1015dc78] (signed), Sim_100c48fa(obj, 2).
 */
void __cdecl Sim_100c0558(void *obj);
/* 0x1001403a(obj, value); Sim_1000ae00(obj, Sim_10015266(obj)); returns 0 when float obj[0xe0] + [0x1012bb48] is greater than float obj[0x5c], else 1. */
int __cdecl Sim_100c0d40(void *obj, int16_t value);
/*
 * 1 when the block b = obj[0x28] has dword +0x510 of 6 or more (signed), |float b[0x540]| is at least [0x1012bb34] and
 * (int)float b[0x4e8] (truncated like _ftol) is less than limit (signed); else 0.
 */
int __cdecl Sim_100c3892(void *obj, int limit);
/* Writes seconds as "%2.2d:%2.2d:%2.2d" (hours, minutes, seconds) to the buffer at 0x1017e5d0 and returns it. */
char *__cdecl Sim_100ccaa0(int seconds);

#endif
