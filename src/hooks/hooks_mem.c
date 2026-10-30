/*
 * HGSIM.DLL memory cache module, converted to C (src/sim/mem). Each converted
 * function takes the original's place. hgrc.cfg word "noconv" leaves them all
 * out, to compare against the original code.
 */
#include <stdio.h>
#include <string.h>

#include "../launcher/debug.h"
#include "../sim/mem/mem.h"
#include "hook.h"

#define MEM_HOOK(rva_, name_, prologue_, fn_) \
    {.module = "HGSIM.DLL", .rva = (rva_), .name = (name_), .expect = (prologue_), \
     .stolen = sizeof(prologue_), .replacement = (void *)(fn_)}

/* push ebp / mov ebp,esp / sub esp,0x14 */
static const unsigned char kPrologueStampNewer[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14};
/* push ebp / mov ebp,esp / sub esp,8 */
static const unsigned char kPrologueFindName[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08};
/* push ebp / mov ebp,esp / mov eax,[ebp+8] */
static const unsigned char kPrologueArg0[] = {0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08};
/* push ebp / mov ebp,esp / push ecx / mov eax,[ebp+8] */
static const unsigned char kPrologueTrimEnd[] = {0x55, 0x8B, 0xEC, 0x51, 0x8B, 0x45, 0x08};

/* push ebp / mov ebp,esp / push ecx / cmp [ebp+8],0 */
static const unsigned char kPrologueFreeRecord[] = {0x55, 0x8B, 0xEC, 0x51, 0x83, 0x7D, 0x08, 0x00};
/* push ebp / mov ebp,esp / sub esp,0x1c */
static const unsigned char kPrologueFreeHashTable[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x1C};

static const HookDesc g_memHooks[] = {
    MEM_HOOK(0xB9748, "Cache_StampNewer", kPrologueStampNewer, Cache_StampNewer),
    MEM_HOOK(0xBA1C9, "Cache_FindName", kPrologueFindName, Cache_FindName),
    MEM_HOOK(0xBA21C, "Str_CutAtBlank", kPrologueArg0, Str_CutAtBlank),
    MEM_HOOK(0xBA252, "Str_SkipBlanks", kPrologueArg0, Str_SkipBlanks),
    MEM_HOOK(0xBA285, "Str_TrimEnd", kPrologueTrimEnd, Str_TrimEnd),
    MEM_HOOK(0xBA631, "Cache_FreeRecord", kPrologueFreeRecord, Cache_FreeRecord),
    MEM_HOOK(0xBA68E, "Cache_FreeHashTable", kPrologueFreeHashTable, Cache_FreeHashTable),
};

/* What the converted code still calls in the original DLL. */
static void BindImports(HMODULE sim)
{
    unsigned char *base = (unsigned char *)sim;
    MemImports imports = {
        .free = (void(__cdecl *)(void *))(base + 0xFEFE0),
        .assertFail = (void(__cdecl *)(const char *, const char *, int))(base + 0xFF860),
        .listFree = (void(__cdecl *)(void *, void(__cdecl *)(void *)))(base + 0x2428),
        .recordAssertExpr = (const char *)(base + 0x1510CC),
        .recordAssertFile = (const char *)(base + 0x1510A8),
        .tableAssertExpr = (const char *)(base + 0x1510F4),
        .tableAssertFile = (const char *)(base + 0x1510D0),
    };

    Mem_SetImports(&imports);
}

void HooksMem_OnSimLoaded(HMODULE sim)
{
    FILE *fp = fopen("hgrc.cfg", "r");
    char word[32];
    int off = 0, count;

    while (fp != NULL && !off && fscanf(fp, "%31s", word) == 1)
        off = _stricmp(word, "noconv") == 0;
    if (fp != NULL)
        fclose(fp);
    if (off) {
        DebugPrintf("HGSIM.DLL: converted functions off (noconv)\n");
        return;
    }
    BindImports(sim);
    count = Hook_InstallModule(sim, "HGSIM.DLL", g_memHooks,
                               sizeof(g_memHooks) / sizeof(g_memHooks[0]));
    DebugPrintf("HGSIM.DLL: %d converted memory function(s) installed\n", count);
}
