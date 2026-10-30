/*
 * HGSIM's sound code with SDL3 (platform/audio.h) in place of DirectSound (DECISIONS.md, "HGSIM sound: DirectSound
 * replaced by SDL3"). Each function is the game's own code line for line; only every DirectSound call is replaced,
 * at the same place, by the matching audio.h call with the same values. The voice slots of the sound block
 * [0x1015d0a4] (+0x94 + 8 * slot) hold AudioVoice pointers instead of DirectSound buffers, and the device global
 * 0x109e369c a token instead of the DirectSound object. Installed by hooks_audio.c with the hgrc.cfg word
 * "sdlaudio" only; without it the DirectSound code runs as before.
 */
#ifndef HG_SIM_SOUND_SDL_SOUND_H
#define HG_SIM_SOUND_SDL_SOUND_H

#include <stdint.h>
#include "../../platform/audio.h"

#ifndef _WIN32
#define __cdecl /* the default convention on other targets, for the test builds */
#endif

/* 0x10039708: the pitch of a voice (DirectSound SetFrequency). */
void __cdecl SndSdl_10039708(AudioVoice *voice, uint32_t rate);

/* 0x1003a327: pause; every voice in use is stopped, its slot state 4 when it was looping, else 5;
 * [0x1015ea7c] = 1. */
void __cdecl SndSdl_1003a327(void);

/* 0x1003a402: resume; the voices of states 4 / 5 play again (looping for 4), state 1; [0x1015ea7c] = 0. */
void __cdecl SndSdl_1003a402(void);

/* 0x10039931 / 0x100397e7: the 3D voice position and the 3D listener; the game data they set is kept, the
 * DirectSound 3D calls have no SDL3 counterpart (3D sound stays off with SDL3). */
void __cdecl SndSdl_10039931(int idx, const uint8_t *unit);
void __cdecl SndSdl_100397e7(void);

/* 0x10039c00: the per-frame voice update (pause / resume, 3D positions, start of new voices with their volume,
 * finished voices freed and the next queued sound started, pan, stop requests); returns 0 without sound, else 1. */
int __cdecl SndSdl_10039c00(void);

/* 0x100399f4: a new clip voice of `bytes` (8-bit mono 22050 Hz) into *out, *out3d = 0 when kind != 0 (no 3D
 * buffer); 1, or 0 with *out = 0 when it cannot be made. `device` is the token at 0x109e369c (not used). */
int __cdecl SndSdl_100399f4(uint32_t device, AudioVoice **out, uint32_t *out3d, uint32_t bytes, int kind);

/* 0x1000e0d9: finds a slot for sound `id` (or the clip `data`), loads the clip into a new voice and fills the slot;
 * the slot, or -6 (no sound), -8 (no free slot), -9 / -10 (already playing). */
int __cdecl SndSdl_1000e0d9(int id, uint32_t data, uint32_t flags, uint32_t a4, uint32_t a5, int kind, uint32_t unit);

/* The device token stored at 0x109e369c in place of the DirectSound object: its address only (tested against 0 and
 * passed on), never read. */
extern uint8_t SndSdl_Device[16];

/* 0x1003a0e4: sound start; SDL3 audio opened, *device = the token, the 3D flag (block +0x10) off, the ready flag
 * (block +0) on; 1, or -12 when the audio cannot be opened. */
int __cdecl SndSdl_1003a0e4(uint32_t *device, uint32_t window);

/* 0x1003a548: sound shutdown; SDL3 audio closed and *device = 0 when it was set; 1. */
int __cdecl SndSdl_1003a548(uint32_t *device);

#endif
