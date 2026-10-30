/* Wave 2e, original 0x1009cfbd and 0x1009d84e. */
#ifndef HG_SIM_UNSORTED_1009_W2E_H
#define HG_SIM_UNSORTED_1009_W2E_H

#include <stdint.h>

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Mem_Move(dst, src, 0x38); returns dst. */
void *__cdecl Sim_1009cfbd(void *dst, const void *src);
/*
 * With obj non-null and its pointer at +0 non-null, k = dword +4 of what that pointer
 * points to; when k <= 0x19 stores value at 0x1017cd50 + k * 0x16, copies s1 to 0x1017cd54 +
 * k * 0x16 and s2 to 0x1017cd5d + k * 0x16 (strcpy, no length check).
 */
void __cdecl Sim_1009d84e(void *obj, const char *s1, uint32_t value, const char *s2);

#endif
