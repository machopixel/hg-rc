#include "w4v.h"
#include "../util/access.h"

typedef void *(__stdcall *W4vLoadLibraryFn)(const char *name);
typedef void *(__stdcall *W4vGetProcAddressFn)(void *module, const char *name);
typedef void(__stdcall *W4vSleepFn)(uint32_t ms);
typedef void(__stdcall *W4vOutputDebugStringFn)(const char *text);
typedef int(__cdecl *ForceExitFn)(void);

#define IAT_GETPROCADDRESS 0x10ac55dcu
#define IAT_OUTPUTDEBUGSTRINGA 0x10ac55e0u
#define IAT_LOADLIBRARYA 0x10ac55e4u
#define IAT_SLEEP 0x10ac5604u

// FUNCTION: HGSIM 0x100bcc50
int __cdecl Sim_100bcc50(void)
{
    uint32_t i;

    for (i = 1; i < G32(0x1017e1bc) + 1u; i++) {
        uint32_t entry = (i << 5) + 0x1017dfb8u;

        G32(entry + 0x18) = (uint32_t)(uintptr_t)SIM_API(W4vLoadLibraryFn, IAT_LOADLIBRARYA)((const char *)GBYTES(entry));
        if (G32(entry + 0x18) != 0) {
            G32(entry + 0x1c) = (uint32_t)(uintptr_t)SIM_API(W4vGetProcAddressFn, IAT_GETPROCADDRESS)(
                (void *)(uintptr_t)G32(entry + 0x18), (const char *)GBYTES(0x10151560));
        } else {
            SIM_API(W4vSleepFn, IAT_SLEEP)(2000);
            i--;
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x10045e19
int __cdecl Force_ExitSystem(void)
{
    int result;

    if (G32(0x1015ef3c) == 0)
        return 0;
    if (G32(0x1015ef44) == 0)
        return (int)0x8004050f;
    result = (*(ForceExitFn *)GBYTES(0x1015ef44))();
    if (result < 0)
        SIM_API(W4vOutputDebugStringFn, IAT_OUTPUTDEBUGSTRINGA)((const char *)GBYTES(0x1013b02c));
    return result;
}
