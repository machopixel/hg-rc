/*
 * The music player (music.h): Ogg Vorbis files streamed through an audio.h stream voice. In place of 1.2's
 * streaming WAV player (0x100161b4 .. 0x10016a10, a DirectSound ring buffer refilled by a 250 ms winmm timer):
 * the mixer asks the decoder for each next block itself. Like 1.2's player, a track that has played to its end
 * still counts as playing for 8 seconds of silence (1.2 0x1001671d: 0x1f40 ms after the data ran out it stops the
 * stream), so the game's restart of a finished track (1.2 0x10015fbd) comes after that pause.
 */
#include <stdio.h>
#include <stdlib.h>

#include <SDL3/SDL.h>

#include "audio.h"
#include "music.h"
#include "../launcher/debug.h"

/* last: its short macro names would break the Windows headers */
#define STB_VORBIS_NO_PUSHDATA_API
#ifdef _MSC_VER
#pragma warning(push, 0) /* third-party code, kept as published */
#endif
#include "../../third_party/stb/stb_vorbis.c"
#ifdef _MSC_VER
#pragma warning(pop)
#endif

#define MUSIC_MAX_TRACK 40          /* 1.2's Track1..Track40 */
#define MUSIC_END_SILENCE_MS 8000u  /* 1.2's 0x1f40 */

static stb_vorbis *g_ogg;
static AudioVoice *g_voice;
static int g_playing;               /* 1.2's flag +0x10 */
static int g_paused;
static uint64_t g_endedAt;          /* 0, or when the voice played through */
static uint32_t g_volume = 0xffff;
static int g_lastTrack = -1;        /* -1 until looked up */

static void TrackPath(char *out, size_t size, int track)
{
    snprintf(out, size, "MUSIC\\Track%02d.ogg", track);
}

int Music_HasTrack(int track)
{
    char path[32];
    FILE *f;

    if (track < 1 || track > MUSIC_MAX_TRACK)
        return 0;
    TrackPath(path, sizeof(path), track);
    f = fopen(path, "rb");
    if (f == NULL)
        return 0;
    fclose(f);
    return 1;
}

int Music_LastTrack(void)
{
    int t;

    if (g_lastTrack < 0) {
        g_lastTrack = 0;
        for (t = 1; t <= MUSIC_MAX_TRACK; t++) {
            if (Music_HasTrack(t))
                g_lastTrack = t;
        }
        DebugPrintf("Music: last track with a file MUSIC\\TrackNN.ogg: %d\n", g_lastTrack);
    }
    return g_lastTrack;
}

/* the mixer's next block (audio thread, voice list locked) */
static uint32_t FillFromOgg(void *user, int16_t *out, uint32_t frames)
{
    stb_vorbis *ogg = (stb_vorbis *)user;
    int channels = ogg->channels > 2 ? 2 : ogg->channels;

    return (uint32_t)stb_vorbis_get_samples_short_interleaved(ogg, channels, out, (int)(frames * channels));
}

/* 1.2 0x1002e855: -3333 + value * 3333 / 0xffff hundredths of a dB */
static int32_t VolumeHundredths(uint32_t value)
{
    if (value > 0xffff)
        value = 0xffff;
    return (int32_t)(-3333.0 + 3333.0 * (double)value / 65535.0);
}

void Music_Stop(void)
{
    if (g_voice != NULL) {
        Audio_VoiceStop(g_voice);
        Audio_VoiceDestroy(g_voice);    /* off the mixer before the decoder goes */
        g_voice = NULL;
    }
    if (g_ogg != NULL) {
        stb_vorbis_close(g_ogg);
        g_ogg = NULL;
    }
    g_playing = 0;
    g_paused = 0;
    g_endedAt = 0;
}

int Music_Play(int track)
{
    char path[32];
    stb_vorbis_info info;
    int error = 0;

    Music_Stop();
    if (!Audio_IsOpen() || !Music_HasTrack(track))
        return 0;
    TrackPath(path, sizeof(path), track);
    g_ogg = stb_vorbis_open_filename(path, &error, NULL);
    if (g_ogg == NULL) {
        DebugPrintf("Music: cannot decode %s (stb_vorbis error %d)\n", path, error);
        return 0;
    }
    info = stb_vorbis_get_info(g_ogg);
    g_voice = Audio_StreamCreate(info.sample_rate, info.channels >= 2 ? 2 : 1, FillFromOgg, g_ogg);
    if (g_voice == NULL) {
        DebugPrintf("Music: no voice for %s (%u Hz, %d channels)\n", path, info.sample_rate, info.channels);
        stb_vorbis_close(g_ogg);
        g_ogg = NULL;
        return 0;
    }
    Audio_VoiceSetVolume(g_voice, VolumeHundredths(g_volume));
    Audio_VoicePlay(g_voice, 0);
    g_playing = 1;
    DebugPrintf("Music: playing %s (%u Hz, %d channels)\n", path, info.sample_rate, info.channels);
    return 1;
}

void Music_Pause(int pause)
{
    if (g_voice == NULL)
        return;
    if (pause) {
        Audio_VoiceStop(g_voice);
        g_paused = 1;
    } else if (g_paused) {
        g_paused = 0;
        if (g_endedAt == 0)
            Audio_VoicePlay(g_voice, 0);
    }
}

int Music_IsPlaying(void)
{
    if (g_playing && !g_paused && g_voice != NULL && !Audio_VoiceIsPlaying(g_voice)) {
        uint64_t now = SDL_GetTicks();

        if (g_endedAt == 0)
            g_endedAt = now;
        else if (now - g_endedAt > MUSIC_END_SILENCE_MS)
            Music_Stop();
    }
    return g_playing;
}

void Music_SetVolume(uint32_t volume)
{
    g_volume = volume > 0xffff ? 0xffff : volume;
    if (g_voice != NULL)
        Audio_VoiceSetVolume(g_voice, VolumeHundredths(g_volume));
}

void Music_Close(void)
{
    Music_Stop();
}
