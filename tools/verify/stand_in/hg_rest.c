/* Checker stand-in for src/sim/crt/hg_rest.c: what the converted code called before the C runtime replacement (the
 * original's time 0x100ff500, localtime 0x101071e0, getenv 0x10100130 and assert 0x100ff860; ExitProcess,
 * GetCurrentProcess + TerminateProcess through the DLL's import slots), so the specs compare the code around them.
 * hg_rest.c itself is standard C (DECISIONS.md, "C runtime replacement: the rest"). */
#include <stdint.h>
#include "../../../src/sim/simmem.h"
#include "../../../src/sim/crt/hg_rest.h"

typedef void(__stdcall *HgExitProcessFn)(uint32_t);
typedef int(__stdcall *HgTerminateProcessFn)(uint32_t, uint32_t);
typedef uint32_t(__stdcall *HgCurrentProcessFn)(void);

int32_t hg_time(void)
{
    return SIM_FN(int32_t (__cdecl *)(int32_t *), 0x100ff500)(0);
}

int32_t *hg_localtime(const int32_t *t)
{
    return SIM_FN(int32_t *(__cdecl *)(const int32_t *), 0x101071e0)(t);
}

char *hg_getenv(const char *name)
{
    return SIM_FN(char *(__cdecl *)(const char *), 0x10100130)(name);
}

void hg_exit_process(uint32_t code)
{
    SIM_API(HgExitProcessFn, 0x10ac5640)(code);
}

void hg_terminate_process(uint32_t code)
{
    SIM_API(HgTerminateProcessFn, 0x10ac5644)(SIM_API(HgCurrentProcessFn, 0x10ac5648)(), code);
}

void __cdecl Crt_Assert(const char *expr, const char *file, uint32_t line)
{
    SIM_FN(void(__cdecl *)(const char *, const char *, uint32_t), 0x100ff860)(expr, file, line);
}
