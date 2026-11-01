/*
 * The music: 1.2's WAV music (wavmap.ini, DVD edition) with the CD tracks as Ogg Vorbis files instead, and no CD
 * at all (Diego, 2026-10-31: "The music cd will instead be played from the music folder ... Ignore the cd").
 * CD track N is the file MUSIC\TrackNN.ogg of the game folder (two digits, e.g. MUSIC\Track02.ogg). One track
 * plays at a time, streamed through an audio.h stream voice (decoded block by block by stb_vorbis). Needs the
 * SDL3 sound (audio.h open); otherwise every call does nothing and Music_Play answers 0.
 * The game side (start, pause, resume, stop, the restart of a finished track, volume) is 1.2's code in the
 * converted functions (unsorted_1001/w4u_cd.c, w5a_misc.c, w6d_calls.c, w7l_misc.c, w9d_misc.c, win/w4w_win.c).
 */
#ifndef HG_PLATFORM_MUSIC_H
#define HG_PLATFORM_MUSIC_H

#include <stdint.h>

/* The highest track N (1..40, 1.2's wavmap.ini limit) with a file, 0 when there is none. Looked up once. */
int Music_LastTrack(void);
/* Whether track N has a file. */
int Music_HasTrack(int track);
/* Stops the track playing, if any, and starts track N from its start at the current volume. 1, or 0 when the
 * file is missing or cannot be decoded (then nothing plays). */
int Music_Play(int track);
/* 1.2's stream pause (0x1002e830): 1 stops keeping the place, 0 goes on from it. */
void Music_Pause(int pause);
/* Stops and closes the track. */
void Music_Stop(void);
/* 1 from Music_Play until Music_Stop or the end of the track, also while paused (1.2's "playing" flag +0x10). */
int Music_IsPlaying(void);
/* The game's music volume 0..0xffff (larger is clipped), as 1.2's stream volume (0x1002e855): linear in decibels
 * from -33.33 dB (0) to 0 dB (0xffff). Kept for the next track too. */
void Music_SetVolume(uint32_t volume);
/* Music_Stop, for the sound shutdown (before Audio_Close). */
void Music_Close(void);

#endif
