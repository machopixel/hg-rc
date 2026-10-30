#include "w6i.h"
#include "../simmem.h"

typedef void *(__stdcall *W6iGetModuleHandleAFn)(const char *name);
typedef void *(__stdcall *W6iGetProcAddressFn)(void *module, const char *name);
typedef int(__stdcall *W6iFeatureFn)(uint32_t feature);

// FUNCTION: HGSIM 0x10100a10
int __cdecl Fpu_HasFdivBug(void)
{
    /* volatile: the division must happen at run time on the player's processor, as in the original */
    volatile double x = 4195835.0;
    volatile double y = 3145727.0;
    volatile double rest = x - (x / y) * y;

    return rest > *SIM_PTR(double, 0x1012bf38) ? 1 : 0;
}

// FUNCTION: HGSIM 0x10100a60
int __cdecl Fpu_CheckFdivBug(void)
{
    void *kernel = SIM_API(W6iGetModuleHandleAFn, 0x10ac55ec)(SIM_PTR(char, 0x1012bf5c));

    if (kernel != 0) {
        W6iFeatureFn present = (W6iFeatureFn)SIM_API(W6iGetProcAddressFn, 0x10ac55dc)(kernel, SIM_PTR(char, 0x1012bf40));

        if (present != 0)
            return present(0);
    }
    return Fpu_HasFdivBug();
}
