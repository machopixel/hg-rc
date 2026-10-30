/*
 * Round 5, letter x: the function at 0x100b5b76, which calls a helper that is not converted yet
 * (called through its address, SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100B_W5X_H
#define HG_SIM_UNSORTED_100B_W5X_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted: (object, function pointer or NULL). */
#define SIM_CALL_100aab03 SIM_FN(void(__cdecl *)(void *, void *), 0x100aab03)

/*
 * With obj non-NULL runs 0x100aab03(obj, f) where f is the function at 0x100010f5 when Sim_100a9b71(obj)
 * (the dword at obj+0x8c) is non-zero, else NULL.
 */
void __cdecl Sim_100b5b76(void *obj);

#endif
