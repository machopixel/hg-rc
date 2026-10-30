#include "w4w.h"
#include "../win/w4w_api.h"
#include "../util/access.h"
#include "../unsorted_1002/w2j.h"

typedef int(__stdcall *W4wNetCall2)(uint32_t a, uint32_t b);
typedef int(__stdcall *W4wNetCall1)(const char *text);

// FUNCTION: HGSIM 0x10045d70
int __cdecl Sim_10045d70(uint32_t a, uint32_t b)
{
    if (Sim_10045ec2() == 0) {
        SIM_API(W4wOutputDebugStringFn, W4W_IAT_OUTPUTDEBUGSTRING)((const char *)GBYTES(0x1013afe0));
        return 0;
    }
    if (G32(0x1015ef40) != 0)
        return ((W4wNetCall2)(uintptr_t)G32(0x1015ef40))(a, b);
    return 0;
}

// FUNCTION: HGSIM 0x10045db9
int __cdecl Sim_10045db9(void)
{
    int result;
    char text[0x64];

    if (G32(0x1015ef3c) == 0)
        return 0;
    if (G32(0x1015ef48) != 0) {
        result = ((W4wNetCall1)(uintptr_t)G32(0x1015ef48))((const char *)GBYTES(0x1013af00));
        if (result < 0) {
            SIM_API(W4wWsprintfFn, W4W_IAT_WSPRINTF)(text, (const char *)GBYTES(0x1013affc), result);
            SIM_API(W4wOutputDebugStringFn, W4W_IAT_OUTPUTDEBUGSTRING)(text);
        }
    } else {
        result = (int)0x8004050f;
    }
    return result;
}

// FUNCTION: HGSIM 0x10045e5d
int __cdecl Sim_10045e5d(void *module)
{
    G32(0x1015ef40) = (uint32_t)(uintptr_t)SIM_API(W4wGetProcAddressFn, W4W_IAT_GETPROCADDRESS)(module, (const char *)GBYTES(0x1013b050));
    G32(0x1015ef44) = (uint32_t)(uintptr_t)SIM_API(W4wGetProcAddressFn, W4W_IAT_GETPROCADDRESS)(module, (const char *)GBYTES(0x1013b060));
    G32(0x1015ef48) = (uint32_t)(uintptr_t)SIM_API(W4wGetProcAddressFn, W4W_IAT_GETPROCADDRESS)(module, (const char *)GBYTES(0x1013b070));
    if (G32(0x1015ef40) != 0 && G32(0x1015ef44) != 0 && G32(0x1015ef48) != 0)
        return 1;
    return 0;
}

// FUNCTION: HGSIM 0x10045ec2
void *__cdecl Sim_10045ec2(void)
{
    char name[12];
    uint32_t module;
    int i;

    for (i = 0; i < 11; i++)
        name[i] = (char)GBYTES(0x1013b080)[i];
    module = (uint32_t)(uintptr_t)SIM_API(W4wGetModuleHandleFn, W4W_IAT_GETMODULEHANDLE)(name);
    if (module == 0) {
        module = (uint32_t)(uintptr_t)SIM_API(W4wLoadLibraryFn, W4W_IAT_LOADLIBRARY)(name);
        if (module < 0x20)
            return 0;
    }
    if (module <= 0x20)
        return 0;
    if (Sim_10045e5d((void *)(uintptr_t)module))
        return (void *)(uintptr_t)module;
    return 0;
}

// FUNCTION: HGSIM 0x100481dd
void __cdecl Sim_100481dd(int index)
{
    if (index >= 0) {
        Sim_1004821c(index);
        return;
    }
    for (index = 0; index < 7; index++)
        Sim_1004821c(index);
}

// FUNCTION: HGSIM 0x1004821c
void __cdecl Sim_1004821c(int index)
{
    uint32_t slot = 0x109b94e0u + (uint32_t)index * 4u;
    uint32_t rec = G32(slot);

    if (rec != 0) {
        if (U32(rec, 0x18) != 0)
            Sim_1002828e((int)(U32(rec, 0x14) + G32(0x101c64f0)), (const void *)(uintptr_t)G32(0x1014cec0));
        SIM_API(W4wHeapFreeFn, W4W_IAT_HEAPFREE)(W4W_GAME_HEAP, 1, BYTES(rec));
        G32(slot) = 0;
    }
}
