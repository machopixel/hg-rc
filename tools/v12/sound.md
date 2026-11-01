# 1.2 sound: what changed and how to port it (round 33 follow-up)

Read-only analysis of GameRip_12/hgsim.dll and hgshell.dll (1.2) against GameRip (1.0), the converted sources and
the game data. Addresses are each version's own; "1.0 X" is the 1.0 twin. Nothing here is ported.

## Ported (2026-10-31, Diego's go: "played from the music folder ... Ignore the cd")

The music round's steps 4 and 5 in one go, with Ogg instead of WAV and no CD at all:

- `third_party/stb/stb_vorbis.c` (v1.22, public domain).
- `src/platform/music.h/.c`, the player: `MUSIC\TrackNN.ogg` of the game folder, streamed by a new
  `Audio_StreamCreate` voice (audio.h / audio_sdl3.c; the mixer pulls the next 4096-frame block). It uses 1.2's
  volume curve and its 8 s of silence before a finished track counts as stopped.
- The six music entry points carry 1.2's WAV-map code in the game build (`HG_MUSIC_FILES`, set in
  src/launcher/CMakeLists.txt only):
  - Sim_1001f26e start, Sim_1001f335 / Sim_1001f367 pause / resume, Sim_1001f399 stop;
  - Sim_1001f3ac restart of a finished track;
  - Win_AuxSetVolume volume;
  - the options setter Sim_1001ee7c (case 2), and Sim_1001edd0 without the CD stop;
  - Music_Close in SndSdl_1003a548.
- The checker builds do not define `HG_MUSIC_FILES`, so they keep the 1.0 CD code their specs compare. The music
  code itself has no spec yet.
- One 1.2 bug is not kept: 1.2 applies a changed music volume only while a CD is open.

## In short

- **The one big change is music.** 1.2 can play the soundtrack from WAV files instead of the CD. A file
  `wavmap.ini` lists one WAV file per CD track, and a new streaming player plays them through DirectSound. Without
  wavmap.ini, 1.2 plays the CD exactly as 1.0 does. Neither 1.2 nor the patch ships a wavmap.ini or any music WAV
  (it was for the DVD edition), but Diego's disc-2 image `HEAVYG2.mdf` holds the 11 music tracks as CD audio, so
  we can make the files ourselves.
- **There are no new sound effects.** HEAVYG.PRJ / HEAVYG.MAP are byte-identical in 1.0 and 1.2, and every sound
  name in 1.2's sim (AFXHBEEP, AFXHGENG, AFXHGFLS, AFXHDWPS...) also appears in 1.0's sim, the same number of
  times. Where 1.2 plays one of them from new code (footsteps, HUD key sounds), that is part of the movement /
  controls changes, not a sound change.
- **Changes to the effects mixer are small:** a crash guard when a voice cannot be made (worth porting), a
  DirectSound-only reference-count fix (not needed under SDL3), and one more radio voice bank (squad slot 3).
- **HGSHELL 1.2 changed no sound code.** It reads wavmap.ini only for the DVD movie path (DECISIONS.md, round 33).
- Of the 44 "sound" rows in unsorted.md plus the survey's 11 sound rows, 4 are not sound at all (network, damage,
  particles; see the end).

## Our sound path today

- HGSIM effects: SDL3 through `src/platform/audio.h` (voices with Lock / Unlock, play position, loop, volume, pan,
  rate). The 10 HGSIM functions that called DirectSound are replaced by `src/sim/sound/sdl_sound.c`
  (hooks_audio.c; hgrc word "sdlaudio", the default).
- CD music: `HookedCdAudioInit` (hooks_sim.c, 1.0 0x1009dfa0) reports "no CD audio", so music never starts. Its
  comment already says "Music will later play from files". 1.2's WAV mode is that.
- HGSHELL menu sounds: still DirectSound (splash\wav\AFX*.WAV). 1.2 did not change them.

## 1.2's WAV music

### How it works (1.2 0x10015a38 and the music table)

1. `.\wavmap.ini`, section `[WAVMap]`, is read once:
   - `DVDDrive`: a drive letter.
   - `Track1` .. `Track40`: a WAV path for each CD track number, 255 characters at most. When a path's second
     character is `:`, its drive letter is replaced by DVDDrive. Gaps are allowed. The highest track found is
     the track count.
   - `DVDMoviePath`: folder of the movies (a `\` is added). It is stored; the shell has its own copy.
2. 1.2 keeps a table of six music functions (start, pause, resume, stop, per-frame poll, volume) at
   0x10108c8c..0x10108ca0. With at least one track mapped, the table points to the WAV versions; otherwise it
   points to the CD versions, which are 1.0's CD music code unchanged (1.0 0x1001f26e, 0x1001f335, 0x1001f367,
   0x1001f399, 0x1001f3ac, 0x1009eb85). Every caller (settings, pause menu, sim main loop, options volume) calls
   through the table.
3. The track number comes from the same place as in 1.0: the text at 0x1015dcb5 (1.2 0x1028ce85), read with
   sscanf when the mission starts. `Track<N>` stands for CD track N.
4. WAV start: plays `Track<N>` on the music stream. Poll: every 0x16a ticks, when not paused and the music
   option is on, restarts a finished track (the music loops, as 1.0 restarts the CD track). Pause / resume:
   stop and restart the stream. Volume: 0..0xffff mapped linearly onto DirectSound -3333..0 (hundredths of a dB,
   so -33 dB at the bottom of the slider).
5. The sound start (1.2 0x1002e159 = 1.0 0x1003a0e4) also creates the music stream object (0x1002e918; pointer
   [0x10131310]) and sets the primary buffer to 22050 Hz, 16-bit stereo ("Couldn't set high quality format for
   music (bummer)" when that fails). 1.0's primary format was left alone.

### The streaming player (all new in 1.2)

A WAV file is streamed through one DirectSound secondary buffer used as a ring, refilled every 250 ms by a winmm
timer (`timeSetEvent(250, 250, 0x100169fa)`):

- 0x100b98c0 / 0x100b994c / 0x100b9983 / 0x100b9c56 / 0x100b9ce7 / 0x100b9e17: a WAV reader on winmm's mmio
  (`mmioOpen`, descend into RIFF / WAVE / `fmt ` / `data`, `mmioRead`, rewind, close). Any PCM format the file
  has. (The survey labelled three of these "HUD".)
- 0x100161b4: opens a file, makes a buffer in the file's format (`CreateSoundBuffer`), primes it. 0x100163de /
  0x10016a10: stop and close. 0x10016475 / 0x10016596: fill part of the ring (Lock, read or silence, Unlock).
  0x100166a9: play position (`GetCurrentPosition`). 0x1001671d: the timer's refill pass, guarded by
  `InterlockedExchange`, with `timeGetTime`. 0x10016842: rewind. 0x10016896: volume. 0x100168bf: play and start
  the timer. 0x100169b7: restart. (The survey labelled five of these "input".)
- 0x1002e690 .. 0x1002ea59: the music-stream object (create, destroy, play a file, stop, pause, volume, "is
  playing"), plus the timer start and stop.

**Fit with our path.** Everything the player needs exists in `platform/audio.h`. Its ring buffer is one
`Audio_VoiceCreate(bytes, rate, channels, bits)` voice played looping, `Lock` / `Unlock` map to
`Audio_VoiceLock` / `Unlock`, `GetCurrentPosition` to `Audio_VoiceGetPosition`, `SetCurrentPosition` to
`SetPosition`, and volume to `Audio_VoiceSetVolume` (DirectSound units). Two things differ:

- The winmm timer thread: refill from the sim's frame loop or from an SDL timer instead.
- mmio: read the RIFF chunks ourselves, or use SDL's WAV loader in streaming mode.

The primary-format change has no counterpart; our SDL3 output format is fixed.

### Which files it expects, and what exists

- 1.2 expects nothing by name. wavmap.ini names the files, and wavmap.ini comes from the DVD edition's set-up.
  It is not in GameRip_12, HeavyGearIso, Project, Ref12 or ProjectIso, and no music WAV is either (the only WAVs
  outside the archive are the shell's splash\wav sounds).
- The music is on disc 2: `C:\Users\Diego\Downloads\Heavy-Gear_Win_EN_ISO-Version\Heavy_Gear ISO\HEAVYG2.mds` /
  `.mdf` (raw 2448-byte sectors with subchannel). Track 1 is data (the VIDEO folder, the disc mounted now as V:
  "HEAVYG"). **Tracks 2 to 12 are CD audio, 11 pieces of about 3 minutes each** (182 to 194 s). Disc 1
  (`Heavy Gear.mds`) is data only.
- Ripped as plain CD-quality WAV (44.1 kHz, 16-bit stereo) they come to about 360 MB, or about 180 MB at 22 kHz.
  1.2's primary format (22050 Hz stereo 16-bit) suggests that is what the DVD's files were. A wavmap.ini with
  `Track2=...` .. `Track12=...` would make 1.2's code (and a port of it) play them.

## Sound files and settings

- HEAVYG.PRJ / HEAVYG.MAP: the same files in 1.0 and 1.2 (MD5 of the MAP equal, same size); 103 WAVs in group A.
  The 1.2 patch folder holds textures and missions only, no sounds.
- The sound names in the sims are the same set with the same counts in both versions: AFXHBEEP 2, AFXHGENG 3,
  AFXHGFLS 1, AFXHGWHE 5, AFXHDMSB 7, AFXHDWPS 56 (string occurrences). So 1.2 adds no sounds; it only calls some
  from rewritten code (1.2 0x1005ebbc in the gear movement: AFXHGFLS / AFXHGWHE / AFXHGWEN / AFXHGWDE; the key
  handler 1.2 0x1003b5aa: AFXHDWPS / AFXHDMSB).
- HGSND.CFG (108 bytes) has the same layout in both; the two copies differ only in saved settings values.

## HGSHELL 1.2

Same sound imports as 1.0 (DirectSoundCreate, CoInitialize / CoCreateInstance, timeGetTime) and the same sound
strings, except `.\wavmap.ini`, `DVDDrive`, `WAVMap` and `DVDMoviePath`. It reads DVDDrive and DVDMoviePath only,
for the movies (already covered by hooks_cd.c, DECISIONS.md "1.2's intro movie" / "No CD for the movies"). No
music, and no change to the menu sounds.

## Function by function

"Port" says whether our build needs it. "Fit" says where it lands in our code.

### Effects mixer (sdl_sound.c territory)

| 1.2 | 1.0 | what it does | 1.2 change | port | fit |
|---|---|---|---|---|---|
| 0x10007bf9 (also row 0x10007c14) | 0x1000e0d9 `SndSdl_1000e0d9` | Loads a clip into a free voice slot | When making the voice fails, skips the Lock and copy (1.0 went on with a null buffer) | **Yes, small.** Our `Audio_VoiceLock` reads `v->bytes` and would crash on a null voice (e.g. out of memory or audio closed). | Two lines in SndSdl_1000e0d9 |
| 0x1002dc30 | 0x10039c00 `SndSdl_10039c00` | Per-frame voice update | When freeing a finished voice, calls `Release` until the count reaches 0 (frees duplicated DirectSound buffers) | No: `Audio_VoiceDestroy` frees once, with no reference count | none |
| 0x1002e159 (+ 0x1002e2b1 inside it) | 0x1003a0e4 `SndSdl_1003a0e4` | Sound start | Creates the music stream; sets primary 22050 Hz 16-bit stereo | Only the music-stream creation, with the music port | SndSdl_1003a0e4 |
| 0x1002e655 | 0x1003a548 `SndSdl_1003a548` | Sound shutdown | (unchanged apart from freeing the stream through the object) | With the music port | SndSdl_1003a548 |
| 0x1002d817 | 0x100397e7 | 3D listener data | layout only | No | already SDL |
| 0x1002d961 | 0x10039931 | 3D voice position | layout only (loads reordered) | No | already SDL |
| 0x100173ad | 0x1002009d | Stops one radio voice bank | Banks 0..3 instead of 0..2 | **Yes, tiny**, with the next row | w5x_calls.c |
| 0x100173e1 | 0x100200d1 | Queues a radio voice of a bank | Bank 3 allowed (1.0: 0, 1, 2, 4), so squad slot 3 gets radio replies | **Yes, tiny.** Goes with the ~46 extra text lines (unsorted.md 0x10018050). | w7l_misc.c |
| 0x10017043 | inside 0x1001fc18 | Plays a queued sound | none (misaligned piece; whole function equal) | No | — |

### Music: the switch between CD and WAV

| 1.2 | 1.0 | what it does | 1.2 change | port | fit |
|---|---|---|---|---|---|
| 0x10015a38 | new | Reads wavmap.ini; fills the six-function music table | new | Yes (music) | new C next to w4u_cd.c; called where 1.0 starts the CD |
| 0x10015450 | 0x1001edd0 `Sim_1001edd0` | Music on / off setting | The table's start / stop; option 2 stops only when no WAV map and not networked | Yes (music) | w9d_misc.c |
| 0x100159f6 | 0x1001f4ba | Pause-menu music hook | Calls the table's pause / resume | Yes (music) | w5y_misc.c |
| 0x1002e9fd | 0x1001f4d3 | Resume helper | Takes the stream handle (now the WAV "is it playing" call) | Yes (music) | w4u_cd.c |
| 0x10015e83 | new (CD twin 1.0 0x1001f26e) | WAV start | new | Yes | new |
| 0x10015f1b | new (CD twin 0x1001f335) | WAV pause | new | Yes | new |
| 0x10015f59 | new (CD twin 0x1001f367) | WAV resume (the survey paired it with 1.0 0x10006530 by mistake) | new | Yes | new |
| 0x10015f97 | new (CD twin 0x1001f399) | WAV stop | new | Yes | new |
| 0x10015fbd (+0x1001601f, 0x10016028 inside) | new (CD twin 0x1001f3ac) | WAV poll: restarts a finished track | new | Yes | new |
| 0x1001603f | new (CD twin 0x1009eb85) | WAV volume | new | Yes | new |

### Music: the WAV player (all new)

| 1.2 | what it does | port | fit |
|---|---|---|---|
| 0x100b98c0, 0x100b994c, 0x100b9983 (+0x100b999a), 0x100b9c56, 0x100b9ce7, 0x100b9e17 | WAV file reader (mmio) | Yes, rewritten on plain file reads or SDL's WAV loader | platform side |
| 0x10016130, 0x100161a1, 0x100161b4, 0x100163de | Open / close a stream, create its ring voice | Yes | audio.h voices |
| 0x10016475, 0x10016596 (+0x100165a6), 0x100166a9, 0x1001671d, 0x10016842 | Ring refill, play position, refill pass, rewind | Yes | audio.h Lock / Unlock / GetPosition |
| 0x10016896, 0x100168bf, 0x100169b7, 0x100169fa, 0x10016a10, 0x10016a8e | Volume, play + timer, restart, timer callback, stop, field getter | Yes; the timer becomes a frame-loop or SDL-timer refill | audio.h |
| 0x1002e690, 0x1002e6b3, 0x1002e722, 0x1002e811, 0x1002e830, 0x1002e855 (+0x1002e8b0), 0x1002e8f5, 0x1002e918 (+0x1002e965), 0x1002e983, 0x1002e9ce, 0x1002e9df, 0x1002e9ec, 0x1002ea0a, 0x1002ea20 | Music-stream object and its wrappers | Yes (can be one small C file) | new |
| 0x1002ea38, 0x1002ea59 | winmm timer stop / start | Replaced | frame loop / SDL timer |

### Sound names called from rewritten code (not sound changes)

| 1.2 | what | belongs to |
|---|---|---|
| 0x1005ebbc (inside 1.2 0x1005d75c = 1.0 0x1006e22c) | Gear movement plays footsteps / wheel / engine sounds AFXHGFLS, AFXHGWHE, AFXHGWEN, AFXHGWDE from its rewritten code | movement round 2 |
| 0x1003b5aa (inside 1.2 0x1003b3ba = 1.0 0x1004954e) | Key handler's HUD sounds AFXHDWPS / AFXHDMSB | controls thread |

### Survey "sound" rows that are not sound

| 1.2 | really |
|---|---|
| 0x1002fd99 | Network: loads the anet2.dll entry points (dpDestroyGroup ... dpEnumTransports) |
| 0x10058aab | Damage: the tail of 1.2 0x100589ad (= 1.0 0x100680dd, w17_damage.c), layout only |
| 0x1005c1f8 (and row 0x1005c245) | Effects: 1.0 0x1006c161 spawns 1.2 particles (extra argument); render round |
| 0x1005815c, 0x100589ad, 0x10067cfc, 0x100878fc | Damage / objectives / modes, layout only |
| 0x1005b5c6 | 1.0 0x1006b52f, already ported |

## Proposed order for a sound round

1. **The null-voice guard** in `SndSdl_1000e0d9` (1.2 0x10007f0e). Two lines; a checker spec against 1.2.
2. **Radio bank 3** (0x100173ad / 0x100173e1). Tiny, best done together with the extra radio / status texts
   (unsorted.md 0x10018050) so squad slot 3's replies show and sound.
3. **Music data.** Rip disc 2 tracks 2..12 to WAV and write a wavmap.ini (Diego's choices below). The same files
   would let an original 1.2 install play WAV music, which gives a by-ear reference.
4. **Music switch.** Port 0x10015a38 and the six-function table, with the CD versions being our existing 1.0 C.
   Point the callers (0x10015450, 0x100159f6, 0x1002e9fd, sim main, options volume) at it. Keep
   `HookedCdAudioInit` returning "no CD" so the CD versions stay silent without a disc.
5. **WAV player** on `platform/audio.h`: one looping ring voice, refilled from the frame loop (or an SDL timer);
   our own RIFF reader; the 1.2 volume curve. Create and free the stream in SndSdl_1003a0e4 / SndSdl_1003a548.
6. Leave out: the Release loop, the primary-buffer format, A3D, the 3D layout rows (nothing to do under SDL3).

Steps 1 and 2 need no decisions. Steps 3 to 5 are the music feature and wait on the choices below.

## Choices for Diego

1. **Music at all?** It is off today. 1.2's WAV mode is the way to get it without a CD. Recommended: yes.
2. **Where the music comes from.**
   - (a) Recommended: rip HEAVYG2.mdf tracks 2..12 to WAV once (a small script reading the raw sectors; no
     tool to install).
   - (b) Play the mounted CD image through 1.0's MCI code. This needs the image mounted every time, and the CD
     hook changed.
   - (c) Leave music off.
3. **File format.**
   - (a) CD quality 44.1 kHz, about 360 MB.
   - (b) 22 kHz, about 180 MB, probably what the DVD shipped.
   - (c) A compressed format (OGG / FLAC), which needs a decoder and goes beyond what 1.2 does.

   Recommended: (b) or (a). 1.2's player reads any PCM WAV.
4. **Where the files and wavmap.ini go.** Suggested: `Project\music\Track02.wav` .. `Track12.wav`, plus a
   `Project\wavmap.ini` that we write (not a 1.2 file) with relative paths. No drive letters, so DVDDrive stays
   unused. The files would not go in the repo.
5. **Refill timing for our player.** Recommended: the sim's frame loop, which avoids a second thread touching the
   voice. The alternative is an SDL timer, closer to 1.2's 250 ms winmm timer.
