#include "w5b.h"
#include "../unsorted_1000/w2g.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100e2a31
void *__cdecl Sim_100e2a31(uint32_t a, int32_t *count, int32_t limit, uint32_t d)
{
    void *result = 0;
    void *unused = 0;

    G32(0x1017e838) = 0;
    if (*count < limit) {
        if (SIM_CALL_100e2aa3(a, count, &result, d, &unused) == 0 && result != 0)
            Sim_1000971b(result, G32(0x1017e850));
    }
    G32(0x1017e858) = 0;
    return result;
}

// FUNCTION: HGSIM 0x100e3ba4
void *__cdecl Sim_100e3ba4(uint32_t a, int32_t *count, int32_t limit, uint32_t d, void *result)
{
    void *unused = 0;

    G32(0x1017e838) = 0;
    if (*count < limit) {
        if (SIM_CALL_100e2aa3(a, count, &result, d, &unused) == 0 && result != 0)
            Sim_1000971b(result, G32(0x1017e850));
    }
    G32(0x1017e858) = 0;
    return result;
}

// FUNCTION: HGSIM 0x100e3c0d
void *__cdecl Sim_100e3c0d(uint32_t a, int32_t *count)
{
    int32_t zero = 0;
    void *result;

    SIM_CALL_100e2776(1.0f, 1.0f, 1.0f);
    result = Sim_100e2a31(a, &zero, *count, 0);
    if (result != 0)
        U16(result, 0) = 0x100;
    *count = 0xb4;
    return result;
}
