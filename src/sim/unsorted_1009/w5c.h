/*
 * Round 5, letter c: functions at 0x100988fc and 0x1009964b that call helpers which are not converted yet.
 * Those helpers are called through their original address (SIM_FN) with the prototypes below.
 */
#ifndef HG_SIM_UNSORTED_1009_W5C_H
#define HG_SIM_UNSORTED_1009_W5C_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: takes a double on the stack, returns its absolute value in st(0). */
#define SIM_FABS_C SIM_FN(double(__cdecl *)(double), 0x100fcb70)
/* Not converted: takes the value that Sim_100988fc stores. */
#define SIM_CALL_100a96d8 SIM_FN(void(__cdecl *)(uint32_t), 0x100a96d8)
/* Not converted: fills the dword at 0x1017cc3c. */
#define SIM_CALL_10099510 SIM_FN(void(__cdecl *)(void), 0x10099510)

/* Calls Sim_100332a3 for the masks 0x100, 0x200 and 0x800 with `value` as its on/off argument, calls 0x100a96d8 and stores `value` in [0x1014c9fc] and at +0x14 / +0x18 of the record at [0x1015d1b8]. The first argument is not used. */
void __cdecl Sim_100988fc(uint32_t unused, uint32_t value);
/* Returns the dword at 0x1017cc3c, calling 0x10099510 first when it is still zero. */
uint32_t __cdecl Sim_1009964b(void);

/*
 * Floating point (x87 at the 53-bit precision the Windows loader sets: intermediates are doubles, stored results floats):
 * unless fabs(obj[+0x1c]) < 1e-7f (0x1012ba08) calls Sim_1009cd36(obj, (t - obj[+4]) * obj[+0x30] / obj[+0x1c]).
 */
void __cdecl Sim_1009cf67(void *obj, float t);

#endif
