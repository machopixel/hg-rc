/*
 * SDL3 audio backend (audio.h). One output stream at 48 kHz stereo float; the
 * device asks for more data through a callback and every playing voice is
 * resampled (linear) and added in. A mutex guards the voice list and the voice
 * fields, since the game thread changes them while the device thread mixes.
 */
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

#include "audio.h"
#include "../launcher/debug.h"

#define OUT_RATE 48000
#define MAX_VOICES 64
#define BLOCK_FRAMES 512

struct AudioVoice {
    uint8_t *data;
    uint32_t bytes;
    uint32_t rate, ownRate;
    uint32_t channels, bits, frameBytes, frames;
    double position; /* in clip frames */
    int playing, looping;
    int32_t volume, pan;
    float gainL, gainR; /* from volume and pan */
    AudioFillFn fill;   /* a stream: data holds the last block it gave, refilled when played through */
    void *user;
};

static SDL_AudioStream *g_stream;
static SDL_Mutex *g_lock;
static AudioVoice *g_voices[MAX_VOICES];
static int g_open;
static AudioStats g_stats;

static float DbGain(int hundredths)
{
    if (hundredths <= -10000)
        return 0.0f;
    return powf(10.0f, (float)hundredths / 2000.0f);
}

/* DirectSound pan lowers the far channel only; the near one stays at the volume. */
static void UpdateGains(AudioVoice *v)
{
    float base = DbGain(v->volume);

    v->gainL = base * (v->pan > 0 ? DbGain(-v->pan) : 1.0f);
    v->gainR = base * (v->pan < 0 ? DbGain(v->pan) : 1.0f);
}

static void Sample(const AudioVoice *v, uint32_t frame, float *l, float *r)
{
    const uint8_t *p = v->data + (size_t)frame * v->frameBytes;

    if (v->bits == 8) {
        *l = ((int)p[0] - 128) * (1.0f / 128.0f);
        *r = v->channels == 2 ? ((int)p[1] - 128) * (1.0f / 128.0f) : *l;
    } else {
        *l = (int16_t)(p[0] | (p[1] << 8)) * (1.0f / 32768.0f);
        *r = v->channels == 2 ? (int16_t)(p[2] | (p[3] << 8)) * (1.0f / 32768.0f) : *l;
    }
}

/* Adds up to `count` frames of one voice to `out` (stereo float). */
static void MixVoice(AudioVoice *v, float *out, int count)
{
    double step = (double)v->rate / OUT_RATE;
    int i;

    for (i = 0; i < count && v->playing; i++) {
        uint32_t i0 = (uint32_t)v->position;
        uint32_t i1 = i0 + 1;
        float frac = (float)(v->position - i0);
        float l0, r0, l1, r1;

        if (i1 >= v->frames)
            i1 = v->looping ? 0 : i0;
        Sample(v, i0, &l0, &r0);
        Sample(v, i1, &l1, &r1);
        out[i * 2] += (l0 + (l1 - l0) * frac) * v->gainL;
        out[i * 2 + 1] += (r0 + (r1 - r0) * frac) * v->gainR;
        v->position += step;
        if (v->position >= v->frames && v->fill != NULL) {
            /* next block of the stream; none left: the stream has ended */
            v->position -= v->frames;
            v->frames = v->fill(v->user, (int16_t *)v->data, v->bytes / v->frameBytes);
            if (v->frames == 0) {
                v->playing = 0;
                v->position = 0;
            }
        } else if (v->position >= v->frames) {
            if (v->looping) {
                v->position = fmod(v->position, (double)v->frames);
            } else {
                v->playing = 0;
                v->position = 0;
            }
        }
    }
}

static void SDLCALL AudioCallback(void *user, SDL_AudioStream *stream, int additional, int total)
{
    float block[BLOCK_FRAMES * 2];
    int frames = additional / (int)(sizeof(float) * 2);

    (void)user;
    (void)total;
    while (frames > 0) {
        int n = frames < BLOCK_FRAMES ? frames : BLOCK_FRAMES;
        int i, active = 0;

        memset(block, 0, (size_t)n * 2 * sizeof(float));
        SDL_LockMutex(g_lock);
        for (i = 0; i < MAX_VOICES; i++) {
            if (g_voices[i] != NULL && g_voices[i]->playing) {
                MixVoice(g_voices[i], block, n);
                active++;
            }
        }
        if ((unsigned)active > g_stats.peakVoices)
            g_stats.peakVoices = (unsigned)active;
        g_stats.blocksMixed++;
        SDL_UnlockMutex(g_lock);
        for (i = 0; i < n * 2; i++) {
            if (block[i] > 1.0f)
                block[i] = 1.0f;
            else if (block[i] < -1.0f)
                block[i] = -1.0f;
        }
        SDL_PutAudioStreamData(stream, block, n * 2 * (int)sizeof(float));
        frames -= n;
    }
}

int Audio_Open(void)
{
    SDL_AudioSpec spec = {SDL_AUDIO_F32, 2, OUT_RATE};

    if (g_open)
        return 1;
    SDL_SetAppMetadata("Heavy Gear", NULL, NULL);
    SDL_SetHint(SDL_HINT_AUDIO_DEVICE_SAMPLE_FRAMES, "1024");
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        DebugPrintf("Audio: SDL_InitSubSystem failed: %s\n", SDL_GetError());
        return 0;
    }
    g_lock = SDL_CreateMutex();
    g_stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec, AudioCallback,
                                         NULL);
    if (g_lock == NULL || g_stream == NULL) {
        DebugPrintf("Audio: cannot open the output device: %s\n", SDL_GetError());
        if (g_lock != NULL)
            SDL_DestroyMutex(g_lock);
        g_lock = NULL;
        SDL_QuitSubSystem(SDL_INIT_AUDIO);
        return 0;
    }
    memset(&g_stats, 0, sizeof(g_stats));
    SDL_ResumeAudioStreamDevice(g_stream);
    g_open = 1;
    DebugPrintf("Audio: SDL3 output open (%d Hz stereo float, driver %s)\n", OUT_RATE,
                SDL_GetCurrentAudioDriver());
    return 1;
}

void Audio_Close(void)
{
    int i;

    if (!g_open)
        return;
    g_open = 0;
    SDL_DestroyAudioStream(g_stream); /* also closes the device and stops the callback */
    g_stream = NULL;
    for (i = 0; i < MAX_VOICES; i++) {
        if (g_voices[i] != NULL) { /* the game released its voices first; be safe */
            free(g_voices[i]->data);
            free(g_voices[i]);
            g_voices[i] = NULL;
        }
    }
    SDL_DestroyMutex(g_lock);
    g_lock = NULL;
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
    DebugPrintf("Audio: closed (voices created %u, plays %u, blocks mixed %u, peak voices %u)\n",
                g_stats.voicesCreated, g_stats.plays, g_stats.blocksMixed, g_stats.peakVoices);
}

int Audio_IsOpen(void)
{
    return g_open;
}

void Audio_GetStats(AudioStats *out)
{
    *out = g_stats;
}

AudioVoice *Audio_VoiceCreate(uint32_t bytes, uint32_t rate, uint32_t channels, uint32_t bits)
{
    AudioVoice *v;
    int i;

    if (!g_open || (channels != 1 && channels != 2) || (bits != 8 && bits != 16) || rate == 0 ||
        bytes == 0)
        return NULL;
    v = calloc(1, sizeof(*v));
    if (v == NULL)
        return NULL;
    v->frameBytes = channels * bits / 8;
    v->bytes = bytes - bytes % v->frameBytes;
    v->data = calloc(1, bytes);
    if (v->data == NULL || v->bytes == 0) {
        free(v->data);
        free(v);
        return NULL;
    }
    if (bits == 8)
        memset(v->data, 128, bytes); /* silence for unsigned samples */
    v->rate = v->ownRate = rate;
    v->channels = channels;
    v->bits = bits;
    v->frames = v->bytes / v->frameBytes;
    UpdateGains(v);
    SDL_LockMutex(g_lock);
    for (i = 0; i < MAX_VOICES && g_voices[i] != NULL; i++)
        ;
    if (i == MAX_VOICES) {
        SDL_UnlockMutex(g_lock);
        free(v->data);
        free(v);
        return NULL;
    }
    g_voices[i] = v;
    g_stats.voicesCreated++;
    SDL_UnlockMutex(g_lock);
    return v;
}

AudioVoice *Audio_StreamCreate(uint32_t rate, uint32_t channels, AudioFillFn fill, void *user)
{
    AudioVoice *v;

    if (fill == NULL)
        return NULL;
    v = Audio_VoiceCreate(AUDIO_STREAM_FRAMES * channels * 2, rate, channels, 16);
    if (v == NULL)
        return NULL;
    /* the first block now, in the caller's thread; the mixer asks for the next ones */
    v->frames = fill(user, (int16_t *)v->data, AUDIO_STREAM_FRAMES);
    SDL_LockMutex(g_lock);
    v->fill = fill;
    v->user = user;
    SDL_UnlockMutex(g_lock);
    return v;
}

void Audio_VoiceDestroy(AudioVoice *v)
{
    int i;

    if (v == NULL || !g_open)
        return;
    SDL_LockMutex(g_lock);
    for (i = 0; i < MAX_VOICES; i++) {
        if (g_voices[i] == v)
            g_voices[i] = NULL;
    }
    SDL_UnlockMutex(g_lock);
    free(v->data);
    free(v);
}

uint8_t *Audio_VoiceData(AudioVoice *v, uint32_t *bytes)
{
    if (bytes != NULL)
        *bytes = v->bytes;
    return v->data;
}

void Audio_VoicePlay(AudioVoice *v, int loop)
{
    SDL_LockMutex(g_lock);
    v->looping = loop != 0;
    v->playing = 1;
    g_stats.plays++;
    SDL_UnlockMutex(g_lock);
}

void Audio_VoiceStop(AudioVoice *v)
{
    SDL_LockMutex(g_lock);
    v->playing = 0;
    SDL_UnlockMutex(g_lock);
}

int Audio_VoiceIsPlaying(AudioVoice *v)
{
    return v->playing;
}

int Audio_VoiceIsLooping(AudioVoice *v)
{
    return v->playing && v->looping;
}

int32_t Audio_VoiceSetVolume(AudioVoice *v, int32_t volume)
{
    if (volume > 0 || volume < -10000)
        return AUDIO_ERR_INVALIDPARAM; /* as DirectSound, leaving the volume as it was */
    SDL_LockMutex(g_lock);
    v->volume = volume;
    UpdateGains(v);
    SDL_UnlockMutex(g_lock);
    return 0;
}

int32_t Audio_VoiceSetPan(AudioVoice *v, int32_t pan)
{
    if (pan < -10000 || pan > 10000)
        return AUDIO_ERR_INVALIDPARAM;
    SDL_LockMutex(g_lock);
    v->pan = pan;
    UpdateGains(v);
    SDL_UnlockMutex(g_lock);
    return 0;
}

int32_t Audio_VoiceGetVolume(AudioVoice *v)
{
    return v->volume;
}

int32_t Audio_VoiceGetPan(AudioVoice *v)
{
    return v->pan;
}

int32_t Audio_VoiceSetRate(AudioVoice *v, uint32_t rate)
{
    if (rate != 0 && (rate < 100 || rate > 100000))
        return AUDIO_ERR_INVALIDPARAM;
    SDL_LockMutex(g_lock);
    v->rate = rate != 0 ? rate : v->ownRate;
    SDL_UnlockMutex(g_lock);
    return 0;
}

uint32_t Audio_VoiceGetRate(AudioVoice *v)
{
    return v->rate;
}

uint32_t Audio_VoiceGetPosition(AudioVoice *v)
{
    return (uint32_t)v->position * v->frameBytes;
}

void Audio_VoiceSetPosition(AudioVoice *v, uint32_t bytes)
{
    SDL_LockMutex(g_lock);
    v->position = bytes / v->frameBytes < v->frames ? (double)(bytes / v->frameBytes) : 0.0;
    SDL_UnlockMutex(g_lock);
}

uint32_t Audio_VoiceGetStatus(AudioVoice *v)
{
    uint32_t s = 0;

    SDL_LockMutex(g_lock);
    if (v->playing) {
        s |= AUDIO_STATUS_PLAYING;
        if (v->looping)
            s |= AUDIO_STATUS_LOOPING;
    }
    SDL_UnlockMutex(g_lock);
    return s;
}

int Audio_VoiceLock(AudioVoice *v, uint32_t offset, uint32_t bytes, void **part1, uint32_t *bytes1,
                    void **part2, uint32_t *bytes2, uint32_t flags)
{
    uint32_t size = v->bytes;

    *part1 = NULL;
    *bytes1 = 0;
    if (part2 != NULL)
        *part2 = NULL;
    if (bytes2 != NULL)
        *bytes2 = 0;
    if (flags & AUDIO_LOCK_ENTIRE) {
        offset = 0;
        bytes = size;
    }
    if (offset >= size || bytes > size)
        return AUDIO_ERR_INVALIDPARAM;
    *part1 = v->data + offset;
    *bytes1 = bytes < size - offset ? bytes : size - offset;
    if (part2 != NULL && bytes > *bytes1)
        *part2 = v->data; /* a lock past the end wraps to the start */
    if (bytes2 != NULL)
        *bytes2 = bytes - *bytes1;
    return 0;
}

void Audio_VoiceUnlock(AudioVoice *v, void *part1, uint32_t bytes1, void *part2, uint32_t bytes2)
{
    (void)v;
    (void)part1;
    (void)bytes1;
    (void)part2;
    (void)bytes2; /* the mixer reads the clip memory in place */
}
