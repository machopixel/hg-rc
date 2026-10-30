/*
 * Removed display driver (always on). The sim's DISPDIB.DLL driver (mode 4 and device class 1) is not used: with the
 * renderer plugin the sim runs in mode 6 (device class 3). Its original code is left unconverted and its start
 * function is replaced here by "not available". The sim's own DirectDraw driver (modes 0..3, device class 0) stays:
 * the /D3D renderer (option 3) starts it (DECISIONS.md, round 21).
 *
 * The sim picks a display mode from the table at 0x1013a2a8 (Display_SetMode, 0x1003795b). Every DISPDIB mode starts
 * through 0x1007ea30 (slot +0x10 of the mode record, called with buffer, width, height; 0 means started). A non-zero
 * answer makes Display_SetMode mark the mode unavailable and try the next one.
 *
 * At sim exit one debug.log line: how often the removed driver was asked for and the mode the sim ended in.
 */
#include <windows.h>

#include "../launcher/debug.h"
#include "hook.h"

#define SIM_MODE_RVA  0x15E68C  /* current mode record: id, device class, ... */
#define SIM_CLASS_RVA 0x15E688  /* current device class record */

static HMODULE g_sim;
static DWORD g_dispdibCalls;

// FUNCTION: HGSIM 0x1007ea30
static int __cdecl RemovedDispDibStart(void *buffer, int width, int height)
{
    if (g_dispdibCalls++ == 0)
        DebugPrintf("display: removed DISPDIB driver asked for (%dx%d), answered not available\n", width, height);
    (void)buffer;
    return -1;
}

static const unsigned char kPrologueDispDibStart[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x28};

static const HookDesc g_removed[] = {
    {.module = "HGSIM.DLL", .rva = 0x7EA30, .name = "RemovedDispDibStart",
     .expect = kPrologueDispDibStart, .stolen = sizeof(kPrologueDispDibStart),
     .replacement = (void *)RemovedDispDibStart},
};

void HooksDispProbe_OnSimLoaded(HMODULE sim)
{
    int count;

    g_sim = sim;
    count = Hook_InstallModule(sim, "HGSIM.DLL", g_removed, sizeof(g_removed) / sizeof(g_removed[0]));
    DebugPrintf("HGSIM.DLL: removed display driver %d of 1 stubbed\n", count);
}

void HooksDispProbe_OnSimExit(void)
{
    const DWORD *mode;
    const DWORD *devClass;

    if (g_sim == NULL)
        return;
    mode = *(const DWORD *const *)((const unsigned char *)g_sim + SIM_MODE_RVA);
    devClass = *(const DWORD *const *)((const unsigned char *)g_sim + SIM_CLASS_RVA);
    DebugPrintf("display: removed driver asked for %lu (DISPDIB) time(s); final mode %ld, device class %ld\n",
                g_dispdibCalls, mode != NULL ? (long)mode[0] : -1L, devClass != NULL ? (long)devClass[0] : -1L);
}
