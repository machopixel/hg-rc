#include "w5y.h"
#include "../util/access.h"



// FUNCTION: HGSIM 0x10053620
void *__cdecl Sim_10053620(const char *name)
{
    uint32_t i;

    for (i = 0; i < 0x6d; i++) {
        uint32_t record = 0x1013f160u + (i << 5);

        if (SIM_FN(int(__cdecl *)(const char *, const char *), 0x100ff1e0)((const char *)(uintptr_t)G32(record), name) == 0)
            return GBYTES(record);
    }
    return 0;
}

// FUNCTION: HGSIM 0x1005aee5
int __cdecl Sim_1005aee5(uint32_t a, char *b)
{
    SIM_FN(char *(__cdecl *)(char *), 0x1010d180)(b);
    return SIM_FN(int(__cdecl *)(uint32_t, char *), 0x1005af06)(a, b);
}
