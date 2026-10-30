/*
 * Sound output, as little as the game needs: a handful of voices, each one a
 * whole clip held in memory that is played once or looped, with a volume and a
 * left/right balance. Backend-neutral; today SDL3 (audio_sdl3.c). Volume and
 * pan use the units DirectSound used (hundredths of a decibel), because the
 * game computes them in those units.
 */
#ifndef HG_PLATFORM_AUDIO_H
#define HG_PLATFORM_AUDIO_H

#include <stdint.h>

typedef struct AudioVoice AudioVoice;

/* Starts the output device and the mixer. Returns 1 when sound is available. */
int Audio_Open(void);
void Audio_Close(void);
int Audio_IsOpen(void);

/* Counters for the log. */
typedef struct AudioStats {
    unsigned voicesCreated;
    unsigned plays;
    unsigned blocksMixed;
    unsigned peakVoices;
} AudioStats;
void Audio_GetStats(AudioStats *out);

/*
 * A voice holds `bytes` of PCM (1 or 2 channels, 8 or 16 bits; 8-bit is
 * unsigned) that the caller fills through Audio_VoiceData. NULL when out of
 * memory or the format is not one of those.
 */
AudioVoice *Audio_VoiceCreate(uint32_t bytes, uint32_t rate, uint32_t channels, uint32_t bits);
void Audio_VoiceDestroy(AudioVoice *voice);
uint8_t *Audio_VoiceData(AudioVoice *voice, uint32_t *bytes);

void Audio_VoicePlay(AudioVoice *voice, int loop);
void Audio_VoiceStop(AudioVoice *voice); /* keeps the play position */
int Audio_VoiceIsPlaying(AudioVoice *voice);
int Audio_VoiceIsLooping(AudioVoice *voice);

/* DirectSound's DSERR_INVALIDPARAM: what the setters below and Audio_VoiceLock return for a value DirectSound
 * refuses (the voice keeps its old value), so the game's error naming sees the same code. */
#define AUDIO_ERR_INVALIDPARAM ((int32_t)0x80070057)
/* Hundredths of a dB: volume -10000 (silent) to 0, pan -10000 (left) to 10000; 0, or AUDIO_ERR_INVALIDPARAM
 * outside that range (as DirectSound's SetVolume / SetPan). */
int32_t Audio_VoiceSetVolume(AudioVoice *voice, int32_t volume);
int32_t Audio_VoiceSetPan(AudioVoice *voice, int32_t pan);
int32_t Audio_VoiceGetVolume(AudioVoice *voice);
int32_t Audio_VoiceGetPan(AudioVoice *voice);
/* Playback rate in Hz, 100 to 100000 (DirectSound's DSBFREQUENCY_MIN / _MAX for the game's DirectSound version);
 * 0 restores the clip's own rate. 0, or AUDIO_ERR_INVALIDPARAM outside that range (the rate stays). */
int32_t Audio_VoiceSetRate(AudioVoice *voice, uint32_t rate);
uint32_t Audio_VoiceGetRate(AudioVoice *voice);
/* Play position in bytes of the clip. */
uint32_t Audio_VoiceGetPosition(AudioVoice *voice);
void Audio_VoiceSetPosition(AudioVoice *voice, uint32_t bytes);

/*
 * The calls the game made on DirectSound buffers, with DirectSound's meaning, so game code can call them one for
 * one (HGSIM's sound code, src/sim/sound):
 * GetStatus answers AUDIO_STATUS_PLAYING / AUDIO_STATUS_LOOPING (the DSBSTATUS_ values).
 * Lock gives the clip memory to write: bytes from offset, or the whole clip with AUDIO_LOCK_ENTIRE; a range past
 * the end wraps to the start in the second part. It always fills all four outputs (NULL / 0 when nothing) and
 * returns 0, or AUDIO_ERR_INVALIDPARAM for a range outside the clip. Unlock ends the write; the voice plays from that memory.
 */
#define AUDIO_STATUS_PLAYING 0x1u
#define AUDIO_STATUS_LOOPING 0x4u
#define AUDIO_LOCK_ENTIRE 0x2u
uint32_t Audio_VoiceGetStatus(AudioVoice *voice);
int Audio_VoiceLock(AudioVoice *voice, uint32_t offset, uint32_t bytes, void **part1, uint32_t *bytes1,
                    void **part2, uint32_t *bytes2, uint32_t flags);
void Audio_VoiceUnlock(AudioVoice *voice, void *part1, uint32_t bytes1, void *part2, uint32_t bytes2);

#endif
