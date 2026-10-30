# Sound

HGSIM.DLL has its own mixer (11 voice slots) that sits on top of DirectSound.
Since 2026-10-29 that sound output goes through SDL3 by default. The original
DirectSound code runs only in an original-reference run (hgrc.cfg word
`noconv`: the dev.cmd noconv entries) or with the word `dsound`; `sdlaudio`
forces SDL3 in any case. One function (`HooksAudio_SdlSound`) makes that choice
for every hook file, so a run is never half of each.

## What the game does

- Every sound effect is a whole clip, 8-bit mono 22050 Hz, loaded from
  HEAVYG.PRJ through the cache into a static DirectSound buffer
  (`Lock`, copy, `Unlock`).
- A voice is started with `Play` (looping or once), ended with `Stop`, and
  checked with `GetStatus` (playing, looping). The game frees a voice when it
  has stopped playing. Pause and resume are `Stop` and `Play` again.
- Volume (`SetVolume`) and pan (`SetPan`) are computed by the game from the
  distance, in hundredths of a decibel; the distance volume is clamped to
  +-0x1D4C (7500). A positive volume is refused by DirectSound; the SDL3
  path refuses it the same way (the voice keeps its old volume).
- The sound system is created by `0x1003A0E4`: DirectSound object, cooperative
  level, optional 3D listener (only if HGSND.CFG dword 16 is not 0, which it is
  not in the shipped config). `0x1003A548` releases it.
- The code is in the sim twice: once at `0x1000xxxx` as a table of jump stubs
  (`0x1000233D` jumps to `0x1003A0E4`) and the real code at `0x1003xxxx`.

## What replaces it

- `src/hooks/hooks_audio.c` replaces the 10 HGSIM functions that call
  DirectSound with their SDL3 versions (`src/sim/sound/sdl_sound.c`): the game's
  own code, with each DirectSound call swapped for the matching audio.h call
  (DECISIONS.md, "HGSIM sound: DirectSound replaced by SDL3").
- `src/platform/audio.h` is the neutral interface (voices with volume, pan,
  play, stop). `audio_sdl3.c` mixes all playing voices in the device callback:
  48 kHz stereo float, linear resampling, DirectSound's volume and pan curves.
- If SDL3 audio cannot start, the game takes its own "no DirectSound" path
  (no sound).
- Not done: DirectSound 3D, CD music (MCI; the CD functions at
  `0x1009DFA0` to `0x1009EB85` are still the "no music" stub), the menus'
  sound (HGSHELL.DLL, below).

## The menus (HGSHELL.DLL), still DirectSound

Six methods of one C++ sound object (thiscall, `this` in ecx) call DirectSound:
`0x1002F91D` creates it (DirectSoundCreate, cooperative level 3) into this+0xc,
`0x1002FA6A` releases it, `0x10031FB1` plays a menu sound by index (table
this+0x1a8, 0x29-byte entries, its own buffer this+0x14, volume -1500),
`0x10032228` plays a named sound (buffer this+0x10), `0x100321B1` and
`0x100323FA` stop and release those two buffers. Every clip is 8-bit mono
22050 Hz (CreateSoundBuffer, Lock, copy, Unlock, Play). The menu videos are
separate: Smacker plays their sound through its own DirectSound
(`SmackSoundUseDirectSound(0)`).

## Log lines (debug.log)

`HGSIM.DLL: 4 converted helper function(s) left to the SDL3 sound (sdlaudio)`,
`HGSIM.DLL: 10 of 10 SDL3 sound function(s) installed`, `Audio: SDL3 output open ...`, `Audio: closed (voices created N, plays N,
blocks mixed N, peak voices N)`.
