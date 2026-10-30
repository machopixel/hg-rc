/*
 * HGSIM.DLL simulation core, converted to C (src/sim/core). Each converted
 * function takes the original's place. hgrc.cfg word "noconv" leaves them all
 * out, to compare against the original code.
 */
#include <windows.h>
#include <stdio.h>
#include <string.h>

#include <mmsystem.h>

#include "../launcher/debug.h"
#include "../sim/core/core.h"
#include "../sim/simmem.h"
#include "hook.h"

/* push ebp / mov ebp,esp / sub esp,0x24 */
static const unsigned char kPrologueRunSteps[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x24};

/* push ebp / mov ebp,esp / push ebx / push ecx / push edx */
static const unsigned char kProloguePush3[] = {0x55, 0x8B, 0xEC, 0x53, 0x51, 0x52};
/* push ebp / mov ebp,esp / push ebx / push ecx / xor ecx,ecx */
static const unsigned char kProloguePush2[] = {0x55, 0x8B, 0xEC, 0x53, 0x51, 0x33};
/* push ds / pushal / test [flags],imm: the address is the same at any 64K-aligned base */
static const unsigned char kPrologueStep[] = {0x1E, 0x60, 0xF7, 0x05, 0x7C, 0x31};
/* push ebp / mov ebp,esp / push ebx / xor eax,eax */
static const unsigned char kProloguePause[] = {0x55, 0x8B, 0xEC, 0x53, 0x33, 0xC0};

#define CORE_HOOK(rva_, name_, prologue_, fn_) \
    {.module = "HGSIM.DLL", .rva = (rva_), .name = (name_), .expect = (prologue_), \
     .stolen = sizeof(prologue_), .replacement = (void *)(fn_)}

static const HookDesc g_coreHooks[] = {
    CORE_HOOK(0xE4550, "Timer_Step", kPrologueStep, Timer_Step),
    CORE_HOOK(0xE4579, "Timer_Alloc", kProloguePush3, Timer_Alloc),
    CORE_HOOK(0xE45E7, "Timer_Elapsed", kProloguePush2, Timer_Elapsed),
    CORE_HOOK(0xE4622, "Timer_Reset", kProloguePush2, Timer_Reset),
    CORE_HOOK(0xE465D, "Timer_SetElapsed", kProloguePush2, Timer_SetElapsed),
    CORE_HOOK(0xE469B, "Timer_Free", kProloguePush2, Timer_Free),
    CORE_HOOK(0xE46D0, "Timer_Pause", kProloguePause, Timer_Pause),
    {.module = "HGSIM.DLL", .rva = 0x2A7E0, .name = "Core_RunSteps",
     .expect = kPrologueRunSteps, .stolen = sizeof(kPrologueRunSteps),
     .replacement = (void *)Core_RunSteps},
};

static uint32_t __cdecl ClockMs(void)
{
    return (uint32_t)timeGetTime();
}

void HooksCore_OnSimLoaded(HMODULE sim)
{
    unsigned char *base = (unsigned char *)sim;
    FILE *fp = fopen("hgrc.cfg", "r");
    char word[32];
    int off = 0, count;
    CoreImports imports = {
        .lastTick = (uint64_t *)(base + 0x15DCA8),
        .stepLength = (uint64_t *)(base + 0x15DC70),
        .step = Timer_Step,
        .clock = ClockMs,
    };

    while (fp != NULL && !off && fscanf(fp, "%31s", word) == 1)
        off = _stricmp(word, "noconv") == 0;
    if (fp != NULL)
        fclose(fp);
    if (off)
        return;

    Core_SetImports(&imports);
    count = Hook_InstallModule(sim, "HGSIM.DLL", g_coreHooks,
                               sizeof(g_coreHooks) / sizeof(g_coreHooks[0]));
    DebugPrintf("HGSIM.DLL: %d converted core function(s) installed\n", count);
}
