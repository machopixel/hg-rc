/*
 * Wave 2 (batch f) functions of HGSIM.DLL in 0x1006xxxx. Plain C, original
 * cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_1006_W2F_H
#define HG_SIM_UNSORTED_1006_W2F_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Walks 8 times over the 0x128-byte records at obj+0x84. The record pointer
 * only advances past records whose dword +4 is negative; for the others whose
 * +4 is 0x1c, 0x26 or 0x27 it sets +0 = 0, +8 = -1, +0x114 = 1, +0x20 = -2,
 * and then looks at the same record again (so one such record is rewritten
 * once per remaining pass, and a record that does not match blocks the walk).
 */
void __cdecl Sim_1006ab0c(void *obj);

#endif
