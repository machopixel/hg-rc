/*
 * Damage log (always on; observation only). The game's own debug print 0x10083495 is empty in the shipped DLL. The
 * damage routine (Sim_1006c161) still calls it after every hit with "ARMOR:%f INTERNAL:%f POWER:%d DAMAGE:%ld
 * TYPE:%d GP_ID:%d" (0x10145344): the hit part's armour and internal points left, the shot's power, the damage
 * done, the part number and the target unit's id. This replacement writes exactly that message to debug.log and
 * ignores every other one, so the log stays small. It replaces the converted Sim_10083495 (also empty) at that
 * address; C code that calls Sim_10083495 directly is not affected.
 *
 * The hit that destroys a part returns before that print, so every part destruction (Sim_1006abf6, from any caller)
 * is logged too, through the observer the sim leaves for it (src/sim/unsorted_1006/w8w.h): "DESTROYED GP_ID:%d
 * TYPE:%d BY:%d POWER:%d" with the unit's id, the part (code), the killer as the game passes it (-2 none, -1 the
 * network) and the destroying shot's power (-1 when it was not a hit, e.g. a collision or the self-destruct).
 */
#include <stdarg.h>
#include <stdio.h>

#include <windows.h>

#include "../launcher/debug.h"
#include "../sim/util/access.h"
#include "../sim/unsorted_1006/w8w.h"
#include "../sim/unsorted_1006/w9c.h"
#include "hook.h"

#define SIM_DAMAGE_FORMAT_RVA 0x145344

static const char *g_damageFormat;
static DWORD g_damageLines;
static DWORD g_destroyLines;

// FUNCTION: HGSIM 0x10083495
static void __cdecl DamageLogPrint(const char *fmt, ...)
{
    char line[256];
    va_list ap;

    if (fmt == NULL || fmt != g_damageFormat)
        return;
    va_start(ap, fmt);
    _vsnprintf(line, sizeof(line) - 1, fmt, ap);
    va_end(ap);
    line[sizeof(line) - 1] = 0;
    g_damageLines++;
    DebugPrintf("damage: %s\n", line);
}

static void __cdecl DamageLogDestroyed(uint32_t killer, const uint8_t *s, int32_t code, const uint8_t *shot)
{
    uint32_t unit = s != NULL ? U32(s, 0) : 0;

    g_destroyLines++;
    DebugPrintf("damage: DESTROYED GP_ID:%d TYPE:%d BY:%d POWER:%d\n", unit != 0 ? S32(unit, 4) : -1, code,
                (int)killer, shot != NULL ? S32(shot, 0x70) : -1);
}

/* round 22 fix: a weapon switched off because the part it sits on was destroyed (DECISIONS.md round 22) */
static void __cdecl DamageLogWeaponOff(const uint8_t *gear, int32_t slotIndex, int32_t part)
{
    uint32_t unit = gear != NULL ? U32(gear, 0) : 0;

    DebugPrintf("damage: WEAPON OFF GP_ID:%d SLOT:%d PART:%d\n", unit != 0 ? S32(unit, 4) : -1, slotIndex, part);
}

static const unsigned char kPrologueDebugPrint[] = {0x55, 0x8B, 0xEC, 0x5D, 0xC3};

static const HookDesc g_damageHook = {
    .module = "HGSIM.DLL", .rva = 0x83495, .name = "DamageLogPrint", .expect = kPrologueDebugPrint,
    .stolen = sizeof(kPrologueDebugPrint), .replacement = (void *)DamageLogPrint};

void HooksDamageLog_OnSimLoaded(HMODULE sim)
{
    int count;

    g_damageFormat = (const char *)sim + SIM_DAMAGE_FORMAT_RVA;
    count = Hook_InstallModule(sim, "HGSIM.DLL", &g_damageHook, 1);
    g_simDestroyObserver = DamageLogDestroyed;
    g_simWeaponOffObserver = DamageLogWeaponOff;
    DebugPrintf("HGSIM.DLL: damage log %d of 1 installed\n", count);
}

void HooksDamageLog_OnSimExit(void)
{
    g_simDestroyObserver = NULL;
    g_simWeaponOffObserver = NULL;
    DebugPrintf("damage log: %lu hit(s), %lu part destruction(s) logged\n", g_damageLines, g_destroyLines);
}
