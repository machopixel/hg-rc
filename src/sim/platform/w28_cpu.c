#include "w28.h"
#include "../../platform/plat.h"
#include "../util/access.h"

// FUNCTION: HGSIM 0x100b8f30
int __cdecl Sim_100b8f30(void)
{
    /* deliberate change (DECISIONS.md round 28): no cpuid; the vendor words of an AMD processor, what Diego's PC
       answers, so the file cache takes the millisecond clock everywhere */
    G32(0x10199194) = 0x68747541u; /* "Auth" */
    G32(0x1019917c) = 0x69746e65u; /* "enti" */
    G32(0x10199190) = 0x444d4163u; /* "cAMD" */
    return 1;
}

// FUNCTION: HGSIM 0x100b8fb1
uint64_t __cdecl Sim_100b8fb1(void)
{
    /* the processor's cycle counter (rdtsc) in the original; only the Intel choice of Sim_100b8f30 reached it */
    return Plat_TicksMs();
}

// FUNCTION: HGSIM 0x10099510
uint32_t __cdecl Sim_10099510(void)
{
    if (*GBYTES(0x1017cc38) != 0)
        return (uint32_t)(int32_t)(signed char)*GBYTES(0x1017cc38);
    G32(0x1017cc3c) = 1;
    return 1;
}
