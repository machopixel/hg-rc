/*
 * The sim's HUD at the window's full resolution (docs/renderer/hud.md).
 *
 * Since round 30 the sim's 2D drawing library is converted (src/sim/render/w30_vfx.c, hooked by conv_w30.inc) and
 * its entry points (w30_hud.c) repeat each drawing on the renderer's HUD buffer at full resolution themselves. This
 * file only hands them what lives outside the sim: HGVK.DLL's HUD interface (hud_api.h) and memory for the scratch
 * buffer and the bitmaps. Without them the routines draw only the 640x480 HUD.
 *
 * hgrc.cfg word "nohudhi" leaves the full-resolution HUD out (the old 640x480 HUD).
 * Only active when the Vulkan renderer (first word "hgvk" or HG_RENDERER=hgvk) is in use.
 */
#include <stdio.h>
#include "../platform/plat.h"
#include <stdlib.h>
#include <string.h>

#include "../launcher/debug.h"
#include "../plugins/hgvk/hud_api.h"
#include "../sim/render/w30.h"
#include "hook.h"

/*
 * The scratch buffer the routines draw into instead of the real one. The sim's window record gives the window's
 * height (1440 lines at 1440p), not the picture's 480, and the drawing code clips to that, so the buffer has as
 * many rows as the window says, and slack before and after it.
 */
#define SCRATCH_SLACK 0x10000

static const HudApi *g_api;
static int g_apiLooked;
static unsigned char *g_scratchMem;
static size_t g_scratchBytes;
static uint16_t *g_cells;
static size_t g_cellsCap;

static uint16_t *Scratch(uint32_t bytes)
{
    unsigned char *mem;

    if (bytes > g_scratchBytes) {
        mem = (unsigned char *)calloc((size_t)bytes + 2 * SCRATCH_SLACK, 1);
        if (mem == NULL)
            return NULL;
        free(g_scratchMem);
        g_scratchMem = mem;
        g_scratchBytes = bytes;
        DebugPrintf("HUD: scratch buffer of %u bytes\n", (unsigned)bytes);
    }
    return (uint16_t *)(g_scratchMem + SCRATCH_SLACK);
}

static uint16_t *Cells(uint32_t cells)
{
    if (cells > g_cellsCap) {
        free(g_cells);
        g_cells = (uint16_t *)malloc((size_t)cells * sizeof(uint16_t));
        g_cellsCap = g_cells != NULL ? cells : 0;
    }
    return g_cells;
}

/* The renderer (built into the program: Plat_LoadLibrary("HGVK.DLL")) is looked up on first use. */
static const HudApi *Api(void)
{
    if (!g_apiLooked) {
        void *plugin = Plat_LoadLibrary("HGVK.DLL");
        const HudApi *(__cdecl * get)(void);

        if (plugin == NULL)
            return NULL;
        g_apiLooked = 1;
        get = (const HudApi *(__cdecl *)(void))Plat_LibraryFunction(plugin, "HgvkHudApi");
        g_api = get != NULL ? get() : NULL;
        if (g_api != NULL && g_api->version != HUD_API_VERSION) {
            DebugPrintf("HUD: HGVK.DLL has a different HUD interface, full-resolution HUD off\n");
            g_api = NULL;
        }
        DebugPrintf("HUD: full-resolution interface %s\n", g_api != NULL ? "found" : "not found");
    }
    return g_api;
}

static int ConfigHasWord(const char *word)
{
    char buf[32];
    FILE *fp = fopen("hgrc.cfg", "r");
    int found = 0;

    if (fp == NULL)
        return 0;
    while (!found && fscanf(fp, "%31s", buf) == 1)
        found = _stricmp(buf, word) == 0;
    fclose(fp);
    return found;
}

void HooksHud_OnSimLoaded(HMODULE sim)
{
    (void)sim;
    g_api = NULL; /* HGVK.DLL is loaded again, maybe elsewhere, with every mission */
    g_apiLooked = 0;
    g_w30HudApi = NULL;
    g_w30Scratch = NULL;
    g_w30Cells = NULL;
    if (ConfigHasWord("nohudhi")) {
        DebugPrintf("HUD: full-resolution HUD left out by hgrc.cfg\n");
        return;
    }
    if (!Hooks_VulkanRenderer())
        return;
    g_w30HudApi = Api;
    g_w30Scratch = Scratch;
    g_w30Cells = Cells;
    DebugPrintf("HUD: full-resolution HUD on (drawn by the converted library, round 30)\n");
}
