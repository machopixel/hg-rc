/*
 * Round 5, letter b: functions around 0x100e2a31..0x100e3c0d that call helpers which are not converted yet
 * (called through their address with SIM_FN).
 */
#ifndef HG_SIM_UNSORTED_100E_W5B_H
#define HG_SIM_UNSORTED_100E_W5B_H

#include <stdint.h>
#include "../simmem.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* Not converted. Returns 0 on success; the third argument receives the result pointer. */
#define SIM_CALL_100e2aa3 SIM_FN(int(__cdecl *)(uint32_t, int32_t *, void **, uint32_t, void **), 0x100e2aa3)
#define SIM_CALL_100e2776 SIM_FN(void(__cdecl *)(float, float, float), 0x100e2776)

/*
 * [0x1017e838] = 0; when *count < limit (signed) runs 0x100e2aa3(a, count, &result, d, &unused) and, when that returns 0 and
 * result is not NULL, Sim_1000971b(result, [0x1017e850]); then [0x1017e858] = 0. Returns result (NULL when not run).
 */
void *__cdecl Sim_100e2a31(uint32_t a, int32_t *count, int32_t limit, uint32_t d);
/* The same with the result variable being the fifth argument itself (its incoming value is passed on to 0x100e2aa3); returns it. */
void *__cdecl Sim_100e3ba4(uint32_t a, int32_t *count, int32_t limit, uint32_t d, void *result);
/*
 * 0x100e2776(1.0f, 1.0f, 1.0f); then r = Sim_100e2a31(a, &zero, *count, 0); when r is not NULL its word becomes 0x100;
 * *count = 0xb4. Returns r.
 */
void *__cdecl Sim_100e3c0d(uint32_t a, int32_t *count);

#endif
