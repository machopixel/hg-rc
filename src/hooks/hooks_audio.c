/*
 * Sound through SDL3 (platform/audio.h), the default (DECISIONS.md, "SDL3 sound and Vulkan by default").
 *
 * HGSIM.DLL has its own mixer (11 voice slots) on top of DirectSound. The 10 HGSIM functions that call DirectSound
 * are replaced by their SDL3 versions (sim/sound/sdl_sound.c), which are the game's own code with every DirectSound
 * call swapped for the matching audio.h call. The original DirectSound code runs only in an original-reference run
 * (hgrc.cfg word "noconv"); the word "sdlaudio" forces SDL3 sound in any case. The DirectSound comparison path
 * (word "dsound") is gone (DECISIONS.md, "DirectSound dropped").
 *
 * Not covered: DirectSound 3D (listener and 3D buffers). HGSND.CFG turns it off
 * (it is off in the shipped config), and then the game works the volume and pan
 * out itself.
 */
#include <stdio.h>
#include <stdlib.h>

#include <windows.h>

#include "../launcher/debug.h"
#include "../platform/audio.h"
#include "../sim/sound/sdl_sound.h"
#include "hook.h"

/* whether a word is in hgrc.cfg or hgrc_extra.cfg (the two files hooks_conv.c reads) */
static int ConfigHasWord(const char *wanted)
{
    static const char *const kFiles[] = {"hgrc.cfg", "hgrc_extra.cfg"};
    char word[32];
    int found = 0;
    size_t f;

    for (f = 0; f < sizeof(kFiles) / sizeof(kFiles[0]) && !found; f++) {
        FILE *fp = fopen(kFiles[f], "r");

        while (fp != NULL && !found && fscanf(fp, "%31s", word) == 1)
            found = _stricmp(word, wanted) == 0;
        if (fp != NULL)
            fclose(fp);
    }
    return found;
}

int Hooks_ConfigWord(const char *word)
{
    return ConfigHasWord(word);
}

int Hooks_SdlPlatform(void)
{
    if (ConfigHasWord("sdlinput"))
        return 1;
    return !ConfigHasWord("noconv") && !ConfigHasWord("winplat");
}

int Hooks_VulkanRenderer(void)
{
    const char *env = getenv("HG_RENDERER");
    char word[32];
    FILE *fp;
    int found = 0;

    if (env != NULL)
        return _stricmp(env, "hgvk") == 0;
    fp = fopen("hgrc.cfg", "r");
    if (fp == NULL)
        return 0;
    if (fscanf(fp, "%31s", word) == 1)
        found = _stricmp(word, "hgvk") == 0;
    fclose(fp);
    return found;
}

int HooksAudio_SdlSound(void)
{
    return ConfigHasWord("sdlaudio") || !ConfigHasWord("noconv");
}

/*
 * HGSIM's own sound functions with SDL3 in place of DirectSound (sim/sound/sdl_sound.c; DECISIONS.md, "HGSIM sound:
 * DirectSound replaced by SDL3"). These are what "sdlaudio" installs, the sound start 0x1003a0e4 among them.
 * (hooks_conv.c leaves out the sound trace / FPU log taps on 0xE0D9 / 0x39C00 when "sdlaudio" is set, so each
 * address is hooked once; the DirectSound C versions of 0x39708, 0x397E7, 0x39931 and 0x3A548 are deleted.)
 */
static const unsigned char kPro6_08[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x08};
static const unsigned char kPro6_14[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x14};
static const unsigned char kPro6_20[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x20};
static const unsigned char kPro6_28[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x28};
static const unsigned char kPro6_38[] = {0x55, 0x8B, 0xEC, 0x83, 0xEC, 0x38};
static const unsigned char kPro39708[] = {0x55, 0x8B, 0xEC, 0x83, 0x7D, 0x08, 0x00};
static const unsigned char kPro3A548[] = {0x55, 0x8B, 0xEC, 0x8B, 0x45, 0x08};

#define SDL_SOUND(rva_, fn_, pro_)                                                                            \
    {.module = "HGSIM.DLL", .rva = rva_, .name = #fn_, .expect = pro_, .stolen = sizeof(pro_),               \
     .replacement = (void *)fn_}

static const HookDesc g_sdlSoundHooks[] = {
    SDL_SOUND(0x3A0E4, SndSdl_1003a0e4, kPro6_20), /* sound start */
    SDL_SOUND(0x3A548, SndSdl_1003a548, kPro3A548), /* sound shutdown */
    SDL_SOUND(0x399F4, SndSdl_100399f4, kPro6_28), /* new clip voice */
    SDL_SOUND(0x0E0D9, SndSdl_1000e0d9, kPro6_38), /* clip loader (slot search) */
    SDL_SOUND(0x39C00, SndSdl_10039c00, kPro6_14), /* per-frame voice update */
    SDL_SOUND(0x3A327, SndSdl_1003a327, kPro6_08), /* pause */
    SDL_SOUND(0x3A402, SndSdl_1003a402, kPro6_08), /* resume */
    SDL_SOUND(0x39708, SndSdl_10039708, kPro39708), /* pitch */
    SDL_SOUND(0x397E7, SndSdl_100397e7, kPro6_08), /* 3D listener (game data only) */
    SDL_SOUND(0x39931, SndSdl_10039931, kPro6_08), /* 3D voice position (game data only) */
};
#undef SDL_SOUND

void HooksAudio_OnSimLoaded(HMODULE sim)
{
    int count;

    if (!HooksAudio_SdlSound())
        return;
    count = Hook_InstallModule(sim, "HGSIM.DLL", g_sdlSoundHooks,
                               sizeof(g_sdlSoundHooks) / sizeof(g_sdlSoundHooks[0]));
    DebugPrintf("HGSIM.DLL: %d of %d SDL3 sound function(s) installed\n", count,
                (int)(sizeof(g_sdlSoundHooks) / sizeof(g_sdlSoundHooks[0])));
}

/* Called when the sim has returned, before its DLL is unloaded. */
void HooksAudio_OnSimExit(void)
{
    Audio_Close(); /* does nothing when it is already closed */
}
