/*
 * Hook layer: replaces functions inside the original DLLs while the game runs.
 *
 * Functions are named by module and RVA, because the DLLs prefer the same load
 * address and Windows may move them. Source files carry the address the
 * function has at the preferred base as `// FUNCTION: HGSIM 0x1009dfa0`.
 */
#ifndef HG_HOOK_H
#define HG_HOOK_H

#include <windows.h>

#define HOOK_MAX_PROLOGUE 16

/* Called before the original function runs. `args` points at its first argument. */
typedef void(__cdecl *HookTapFn)(const DWORD *args);

typedef struct HookDesc {
    const char *module;     /* "HGSIM.DLL" */
    DWORD rva;              /* function start, relative to the module base */
    const char *name;       /* for the log */
    const unsigned char *expect; /* original bytes at the start of the function */
    size_t stolen;          /* bytes of `expect` to overwrite, 5 to HOOK_MAX_PROLOGUE */
    void *replacement;      /* same calling convention as the original, or NULL for a tap */
    void **original;        /* set to a callable copy of the original, or NULL */
    HookTapFn tap;          /* observe the call and then run the original unchanged */
} HookDesc;

/*
 * Installs every hook in `hooks` that belongs to `moduleName`. Returns the
 * number installed. A hook whose bytes do not match `expect` is skipped, so a
 * different build of the game is never patched blindly.
 */
int Hook_InstallModule(HMODULE module, const char *moduleName, const HookDesc *hooks,
                       size_t count);

/*
 * Points the import-table entry `functionName` of `dllName` in `module` at
 * `replacement` (same calling convention). Returns 1 when it was found.
 */
int Hook_PatchImport(HMODULE module, const char *dllName, const char *functionName,
                     void *replacement);

/* The hooks for each game module (hooks_sim.c). Called as modules load. */
void Hooks_OnModuleLoaded(const char *moduleName, HMODULE module);

/* Detail and view-distance hooks (hooks_view.c), from Hooks_OnModuleLoaded. */
void HooksView_OnSimLoaded(HMODULE sim);

/* The converted memory cache functions (hooks_mem.c). */
void HooksMem_OnSimLoaded(HMODULE sim);

/* HGSIM.DLL's heap imports on our heaps (hooks_heap.c). */
void HooksHeap_OnSimLoaded(HMODULE sim);

/* The converted simulation core functions (hooks_core.c). */
void HooksCore_OnSimLoaded(HMODULE sim);

/* The small converted helper functions (hooks_conv.c). */
void HooksConv_OnSimLoaded(HMODULE sim);
/* Round 31: the mission's texture loads in .TEX mode (.TEX / .XEL fallback / none found) to the log. */
void HooksConv_OnSimExit(void);

/* The HUD at the window's resolution (hooks_hud.c), with the Vulkan renderer. */
void HooksHud_OnSimLoaded(HMODULE sim);

/* SDL3 input (hooks_input.c): on with Hooks_SdlPlatform. */
void HooksInput_OnSimLoaded(HMODULE sim);
void HooksInput_OnSimExit(void);

/* SDL3 sound (hooks_audio.c): the default; the original DirectSound only with the hgrc.cfg word "noconv"
 * ("sdlaudio" forces SDL3). HooksAudio_SdlSound says which, for every hook file. */
int HooksAudio_SdlSound(void);
/* The SDL3 platform (DECISIONS.md, round 28): SDL3 runs the window events, keyboard, mouse and the menus' screen.
 * The default in converted runs; the original Windows code with "noconv" or with "winplat". "sdlinput" forces it on. Reads hgrc.cfg and hgrc_extra.cfg. */
int Hooks_SdlPlatform(void);
/* Whether a word is in hgrc.cfg or hgrc_extra.cfg. */
int Hooks_ConfigWord(const char *word);
/* Whether the Vulkan renderer is in use: HG_RENDERER=hgvk, or (without HG_RENDERER) the first word of hgrc.cfg. */
int Hooks_VulkanRenderer(void);
void HooksAudio_OnSimLoaded(HMODULE sim);
void HooksAudio_OnSimExit(void);

/* Uninitialised-stack watchers (hooks_uninit.c), only with the hgrc.cfg word "uninittrace"; observation only. */
void HooksUninit_OnSimLoaded(HMODULE sim);
void HooksUninit_OnSimExit(void);

/* The menus' screen through SDL3 (hooks_ddraw.c): with Hooks_SdlPlatform or the hgrc.cfg word "sdldisplay". */
void HooksDDraw_OnShellLoaded(HMODULE shell);
int Hooks_DisplayWanted(void);
void HooksDDraw_OnShellExit(void);

/* The movies from the game folder's video\ when no disc holds them (hooks_cd.c). */
void HooksCd_OnShellLoaded(HMODULE shell);
LPARAM HooksDDraw_TranslateMouse(UINT msg, LPARAM lParam);

/* Removed display drivers (hooks_dispprobe.c): the sim's DirectDraw / DISPDIB drivers answer "not available". */
void HooksDispProbe_OnSimLoaded(HMODULE sim);
void HooksDispProbe_OnSimExit(void);

/* Damage log (hooks_damagelog.c): the game's own per-hit message to debug.log. */
void HooksDamageLog_OnSimLoaded(HMODULE sim);
void HooksDamageLog_OnSimExit(void);

#endif
