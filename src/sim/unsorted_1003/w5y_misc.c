#include "w5y.h"
#include <stdarg.h>
#include "../util/access.h"

typedef int(__cdecl *SprintfFn)(char *dst, const char *format, ...);
typedef void *(__stdcall *LoadLibraryAFn)(const char *name);

#define IAT_LOADLIBRARYA 0x10ac55e4u

// FUNCTION: HGSIM 0x100345e0
void __cdecl Mono_ScrollUp(void)
{
    SIM_FN(void *(__cdecl *)(void *, const void *, uint32_t), 0x100fdf00)((void *)(uintptr_t)0xb0000, (const void *)(uintptr_t)0xb00a0, 0xf00);
}

// FUNCTION: HGSIM 0x10034891
int __cdecl Sim_10034891(const char *format, ...)
{
    va_list args;
    int result;

    va_start(args, format);
    result = SIM_FN(int(__cdecl *)(char *, int, const char *, va_list), 0x100fe3e0)((char *)GBYTES(0x10139f58), 0x50, format, args);
    va_end(args);
    return result;
}

// FUNCTION: HGSIM 0x1003c582
void *__cdecl Lib_Load_1003c582(const char *name)
{
    G32(0x1015eb68) = (uint32_t)(uintptr_t)SIM_API(LoadLibraryAFn, IAT_LOADLIBRARYA)(name);
    SIM_FN(void(__cdecl *)(void), 0x1003be30)();
    return (void *)(uintptr_t)G32(0x1015eb68);
}

// FUNCTION: HGSIM 0x1003cf35
void *__cdecl Sim_1003cf35(const char *name)
{
    char path[0x100];

    SIM_FN(SprintfFn, 0x100fc780)(path, (const char *)GBYTES(0x1013ac64), GBYTES(0x1013ac60), name, GBYTES(0x1013ac5c));
    return SIM_FN(void *(__cdecl *)(const char *, int), 0x100461e8)(path, 0);
}

// FUNCTION: HGSIM 0x1003e26a
int __cdecl Sim_1003e26a(void)
{
    if (G32(0x1015ec84) == 0)
        return 1;
    return SIM_FN(int(__cdecl *)(void), 0x10041d67)() != 0;
}
