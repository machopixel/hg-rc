#include "sdl_sound.h"
#include "../simmem.h"
#include "../util/access.h"
#include "../unsorted_1000/w5a.h"
#include "../unsorted_1000/w8t.h"
#include "../unsorted_1003/w7m.h"
#include "../unsorted_1008/w8w.h"
#include "../unsorted_1000/w2g.h"
#include "../unsorted_1000/w2k.h"
#include "../unsorted_100b/w6g.h"
#include "../unsorted_100d/w2j.h"

#define SND() BYTES(G32(0x1015d0a4))
#define SND_VOICE(i) ((AudioVoice *)(uintptr_t)U32(SND(), 0x94 + (uint32_t)(i) * 8))
#define SND_STATE(i) U32(SND(), 0x54 + (uint32_t)(i) * 4)
#define SND_COUNT() U32(SND(), 4)

// FUNCTION: HGSIM 0x10039708
void __cdecl SndSdl_10039708(AudioVoice *voice, uint32_t rate)
{
    if (voice != 0)
        Audio_VoiceSetRate(voice, rate);    /* was SetFrequency */
}

// FUNCTION: HGSIM 0x1003a327
void __cdecl SndSdl_1003a327(void)
{
    uint32_t i;

    for (i = 0; i < SND_COUNT(); i++) {
        if (SND_STATE(i) != 0 && U32(SND(), 0x154 + i * 4) != 0) {
            uint32_t status = Audio_VoiceGetStatus(SND_VOICE(i));      /* was GetStatus */

            SND_STATE(i) = (status & AUDIO_STATUS_LOOPING) == 0 ? 5 : 4;
            Audio_VoiceStop(SND_VOICE(i));                              /* was Stop */
        }
    }
    G32(0x1015ea7c) = 1;
}

// FUNCTION: HGSIM 0x1003a402
void __cdecl SndSdl_1003a402(void)
{
    uint32_t i;

    for (i = 0; i < SND_COUNT(); i++) {
        if (SND_STATE(i) != 0 && U32(SND(), 0x154 + i * 4) != 0 && (SND_STATE(i) == 5 || SND_STATE(i) == 4)) {
            (void)Audio_VoiceGetStatus(SND_VOICE(i));   /* was GetStatus; the original ignores the answer */
            if (SND_STATE(i) == 4)
                Audio_VoicePlay(SND_VOICE(i), 1);       /* was Play(0, 0, DSBPLAY_LOOPING) */
            else
                Audio_VoicePlay(SND_VOICE(i), 0);       /* was Play(0, 0, 0) */
            SND_STATE(i) = 1;
        }
    }
    G32(0x1015ea7c) = 0;
}

// FUNCTION: HGSIM 0x10039931
void __cdecl SndSdl_10039931(int idx, const uint8_t *unit)
{
    const uint8_t *rec;

    (void)idx;
    if (G32(0x1015d0a4) == 0 || U32(SND(), 0xc) == 0 || unit == 0)
        return;
    rec = BYTES(U32(unit, 0x28));
    G32(0x109e3724) = U32(unit, 0x58);
    G32(0x109e3728) = U32(unit, 0x5c);
    G32(0x109e372c) = U32(unit, 0x60);
    G32(0x109e3730) = U32(rec, 0x548);
    G32(0x109e3734) = U32(rec, 0x54c);
    G32(0x109e3738) = U32(rec, 0x550);
    /* was the 3D buffer's SetAllParameters(block 0x109e3720): no SDL3 counterpart */
}

// FUNCTION: HGSIM 0x100397e7
void __cdecl SndSdl_100397e7(void)
{
    const uint8_t *rec = BYTES(U32(PTRTAB(G32(0x1017b8c8)), 0x28));
    const uint8_t *cam;

    if (G32(0x1015d0a4) == 0 || U32(SND(), 0xc) == 0)
        return;
    cam = BYTES(G32(0x1014cae0));
    G32(0x109e36e4) = U32(cam, 0);
    G32(0x109e36e8) = U32(cam, 4);
    G32(0x109e36ec) = U32(cam, 8);
    G32(0x109e36f0) = U32(rec, 0x548);
    G32(0x109e36f4) = U32(rec, 0x54c);
    G32(0x109e36f8) = U32(rec, 0x550);
    *SIM_PTR(float, 0x109e36fc) = Sim_10084766(*(const float *)(cam + 0xc));
    G32(0x109e3700) = 0;
    *SIM_PTR(float, 0x109e3704) =
        Sim_10084766((float)((double)*(const float *)(BYTES(G32(0x1014cae0)) + 0xc) +
                             (double)*SIM_PTR(const float, 0x10111498)));
    G32(0x109e3708) = 0;
    G32(0x109e370c) = 0x3f800000u;
    G32(0x109e3710) = 0;
    /* was the 3D listener's SetAllParameters(block 0x109e36e0) and CommitDeferredSettings: no SDL3 counterpart */
}

// FUNCTION: HGSIM 0x10039c00
int __cdecl SndSdl_10039c00(void)
{
    uint32_t i;

    if (G32(0x1015d0a4) == 0 || G32(0x109e369c) == 0)
        return 0;
    if (G32(0x1017b92c) == 0) {
        if (G32(0x1015ea7c) != 0) {
            SndSdl_1003a402();
            return 1;
        }
    } else if (G32(0x1015ea7c) == 0) {
        SndSdl_1003a327();
    }
    if (U32(SND(), 0x10) != 0) {
        for (i = 0; i < SND_COUNT(); i++) {
            if (SND_STATE(i) != 0 && U32(SND(), 0x154 + i * 4) != 0 && SND_STATE(i) == 1) {
                if (U32(SND(), 0x14 + i * 4) == 2)
                    SndSdl_10039931((int)i, BYTES(U32(SND(), 0x194 + i * 4)));
                else
                    Audio_VoiceSetVolume(SND_VOICE(i), S32(SND(), 0x294 + i * 4));   /* was SetVolume */
            }
        }
        SndSdl_100397e7();
    }
    for (i = 0; i < SND_COUNT(); i++) {
        AudioVoice *voice;

        if (SND_STATE(i) == 0 || U32(SND(), 0x154 + i * 4) == 0)
            continue;
        voice = SND_VOICE(i);
        switch (SND_STATE(i)) {
        case 1:
            if (U32(SND(), 0x10) == 0) {
                int32_t hr = Audio_VoiceSetVolume(voice, S32(SND(), 0x294 + i * 4));   /* was SetVolume */

                if (hr < 0)
                    Sound_ErrorName(hr);
            }
            /* the original names a failure of Play too; Audio_VoicePlay cannot fail */
            if ((U16(SND(), 0x314 + i * 2) & 0x2000) != 0)
                Audio_VoicePlay(voice, 1);                                          /* was Play, looping */
            else
                Audio_VoicePlay(voice, 0);                                          /* was Play */
            SND_STATE(i) = 2;
            break;
        case 2:
            if ((Audio_VoiceGetStatus(voice) & AUDIO_STATUS_PLAYING) == 0) {      /* was GetStatus */
                Audio_VoiceDestroy(voice);                                          /* was Release */
                SND_STATE(i) = 0;
                U32(SND(), 0x2d4 + i * 4) = 0;
                U16(SND(), 0x314 + i * 2) = 0;
                U8(SND(), 0x315 + i * 2) = 0;
                U8(SND(), 0x314 + i * 2) = 0;
                if (S32(SND(), 0x334 + i * 4) >= 0)
                    Sim_1000e737(0, 0, S32(SND(), 0x334 + i * 4), 0, 0, (int32_t)G32(0x101321b8), 0, 0x5622, -1,
                                 0x150, 1, 0);
                else if (U32(SND(), 0x14 + i * 4) == 1)
                    G32(0x1015d52c) = 0;
            } else if (U32(SND(), 0x10) == 0) {
                if (S32(SND(), 0x254 + i * 4) == -1)
                    Audio_VoiceSetPan(voice, Sim_1000ed77((float)S32(SND(), 0x214 + i * 4)));   /* was SetPan */
                else
                    Audio_VoiceSetPan(voice, S32(SND(), 0x254 + i * 4));                        /* was SetPan */
            }
            break;
        case 3:
            Audio_VoiceStop(voice);                                                 /* was Stop */
            Audio_VoiceDestroy(voice);                                              /* was Release */
            SND_STATE(i) = 0;
            U32(SND(), 0x2d4 + i * 4) = 0;
            U16(SND(), 0x314 + i * 2) = 0;
            U8(SND(), 0x315 + i * 2) = 0;
            U8(SND(), 0x314 + i * 2) = 0;
            break;
        }
    }
    return 1;
}

// FUNCTION: HGSIM 0x100399f4
int __cdecl SndSdl_100399f4(uint32_t device, AudioVoice **out, uint32_t *out3d, uint32_t bytes, int kind)
{
    /* was CreateSoundBuffer(device, { 0x14, kind 2 ? 0x20012 : 0xe2, bytes, 0, PCM 1 channel 22050 Hz 8 bits }, out);
     * the control flags (volume, pan, frequency, 3D) need nothing with SDL3 */
    (void)device;
    *out = Audio_VoiceCreate(bytes, 22050, 1, 8);
    if (*out != 0) {
        if (kind != 0)
            *out3d = 0;     /* was QueryInterface for the 3D buffer; SDL3 has none (as the stand-ins answered) */
        return 1;
    }
    *out = 0;
    return 0;
}

typedef uint32_t(__cdecl *SndSdlLoadFn)(const uint8_t *entry, uint32_t kind, int *size);
typedef void *(__cdecl *SndSdlMemcpyFn)(void *dst, const void *src, uint32_t n);

// FUNCTION: HGSIM 0x1000e0d9
int __cdecl SndSdl_1000e0d9(int id, uint32_t data, uint32_t flags, uint32_t a4, uint32_t a5, int kind, uint32_t unit)
{
    int size = 0;
    AudioVoice *voice = 0;
    uint32_t v3d = 0;
    void *p1, *p2;
    uint32_t n1, n2;
    uint16_t want = (uint16_t)flags;
    int32_t slot, best;
    const uint8_t *entry;
    uint32_t i;
    int r;

    (void)a4;
    (void)a5;
    if (data != 0)
        id = -1;
    if (id <= 0 && data == 0)
        return -6;
    best = -1;
    slot = -1;
    if (kind == 1 && G32(0x1017b92c) == 0) {
        if (SND_STATE(0) != 0)
            return -8;
        slot = 0;
    }
    if (slot == -1) {
        for (i = 1; i < SND_COUNT() && slot == -1; i++) {
            if (U8(SND(), 0x315 + i * 2) != 0) {
                uint16_t w = U16(SND(), 0x314 + i * 2);

                if ((w & 0x100) != 0 || (w & 0x2000) != 0) {
                    if ((want & 0x100) != 0 && S32(SND(), 0x2d4 + i * 4) == id)
                        return -9;
                    continue;
                }
                if ((w & 0x400) != 0 && (want & 0x400) != 0 && S32(SND(), 0x2d4 + i * 4) == id)
                    return -10;
                if ((w & 0x200) != 0 && (want & 0x200) != 0 && S32(SND(), 0x2d4 + i * 4) == id) {
                    SND_STATE(i) = 0;
                    U16(SND(), 0x314 + i * 2) = 0;
                }
            }
            if (SND_STATE(i) == 0)
                slot = (int32_t)i;
            else if ((int32_t)(int8_t)U8(SND(), 0x314 + i * 2) < (int32_t)(flags & 0xffff))
                best = (int32_t)i;
        }
        if (slot == -1) {
            if (best == -1)
                return -8;
            slot = best;
            SND_STATE(slot) = 0;
        }
    }
    entry = Sim_1000e510(id);
    if (data != 0) {
        U32(SND(), 0x154 + (uint32_t)slot * 4) = data;
    } else {
        U32(SND(), 0x154 + (uint32_t)slot * 4) =
            SIM_FN(SndSdlLoadFn, 0x100ba7d5)(entry, G32(0x1014ceb4), &size);
        if (U32(SND(), 0x154 + (uint32_t)slot * 4) == 0)
            return -6;
    }
    Sim_1000e060((const uint8_t **)(SND() + 0x154 + (uint32_t)slot * 4), &size);
    SndSdl_100399f4(G32(0x109e369c), &voice, &v3d, (uint32_t)size, kind);
    /* was Lock; it always gives the clip memory of a voice just made (DECISIONS.md) */
    r = Audio_VoiceLock(voice, 0, (uint32_t)size, &p1, &n1, &p2, &n2, 0);
    if (r != 0)
        Sound_ErrorName(r);
    SIM_FN(SndSdlMemcpyFn, 0x100fcdc0)(p1, (const void *)(uintptr_t)U32(SND(), 0x154 + (uint32_t)slot * 4),
                                        (uint32_t)size);
    Audio_VoiceUnlock(voice, p1, n1, p2, n2);       /* was Unlock */
    r = 0;
    Sim_100bb2b0((const uint32_t *)entry, (const char *)(uintptr_t)G32(0x1014ceb4));
    U32(SND(), 0x94 + (uint32_t)slot * 8) = (uint32_t)(uintptr_t)voice;
    U32(SND(), 0x114 + (uint32_t)slot * 4) = v3d;
    U32(SND(), 0x194 + (uint32_t)slot * 4) = unit;
    SND_STATE(slot) = 1;
    U16(SND(), 0x314 + (uint32_t)slot * 2) = want;
    S32(SND(), 0x2d4 + (uint32_t)slot * 4) = id;
    S32(SND(), 0x14 + (uint32_t)slot * 4) = kind;
    return slot;
}

uint8_t SndSdl_Device[16];

// FUNCTION: HGSIM 0x1003a0e4
int __cdecl SndSdl_1003a0e4(uint32_t *device, uint32_t window)
{
    (void)window;
    U32(SND(), 0) = 0;
    /* was CoInitialize, the Aureal A3D object (CoCreateInstance and Initialize) and, without A3D,
     * DirectSoundCreate; SDL3 is opened in their place, so A3D is never used */
    if (!Audio_Open()) {
        Sim_100d9fc0();     /* the original's "no DirectSound" message (empty log function) */
        return -12;
    }
    *device = (uint32_t)(uintptr_t)SndSdl_Device;
    /* was SetCooperativeLevel(*device, window, 3) with Sound_ErrorName and the shutdown 0x1003a548 on failure:
     * SDL3 has no counterpart (and nothing that can fail there) */
    /* was, with HGSND.CFG's 3D word ([[0x1015d1b8] + 0x40]), the primary buffer with 3D control, its listener
     * (block +8 / +0xc), SetSpeakerConfig and Sim_10039725: SDL3 has no 3D, so the 3D flag stays off as when
     * HGSND.CFG turns 3D off (the shipped config) */
    U32(SND(), 0x10) = 0;
    U32(SND(), 0) = 1;
    return 1;
}

// FUNCTION: HGSIM 0x1003a548
int __cdecl SndSdl_1003a548(uint32_t *device)
{
    if (*device == 0)
        return 1;
    Audio_Close();          /* was the DirectSound object's Release */
    *device = 0;
    /* CoUninitialize went with CoInitialize (sound start) */
    return 1;
}