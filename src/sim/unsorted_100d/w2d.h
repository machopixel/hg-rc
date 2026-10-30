/*
 * Wave 2 additions to unsorted_100d: functions that call converted helpers
 * of util and unsorted_1003. Plain C, original cdecl convention.
 */
#ifndef HG_SIM_UNSORTED_100D_W2D_H
#define HG_SIM_UNSORTED_100D_W2D_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/*
 * Calls Sim_100a9cfb(target, a, b, c) where target is
 * *(*(obj+0x28) + 0x498 + 4 * q[0x11c]) when q[0] != -1 (nothing is called when
 * that pointer is 0) and the dword obj+0x4c when q[0] == -1.
 */
void __cdecl Sim_100d709d(void *obj, uint32_t *a, uint32_t *b, uint32_t *c, void *q);
/*
 * With the signed dwords idx = 0x1017e6a4 and count = 0x1017e6a0: when idx <
 * count returns Sim_1003b417(table[idx]) over the dword table at 0x1017eec0 and
 * increments idx; otherwise returns 0.
 */
uint32_t __cdecl Sim_100dc286(void);

#endif
