# Functions that use DirectSound, and their conversion status

Read-only (2026-10-25). One row per function that calls DirectSound itself (see `directsound_calls.md` for every
call site). Status: **converted** = C, proven by the checker and hooked; **replaced** = a hand-written hook with
different behaviour; **original** = the original code runs (reason given); **n/a** = HGSHELL.DLL, which has no
conversions yet.

| Module | Address | What it does | DirectSound methods | Status |
| --- | --- | --- | --- | --- |
| HGSIM | 0x1000e0d9 | load a clip into a voice slot | Lock, Unlock (via 0x100399f4: CreateSoundBuffer) | original: calls 0x100399f4 (left original); wrapped by a trace tap in hooks_conv.c |
| HGSIM | 0x100397e7 | 3D listener from the camera | 3D listener SetAllParameters, CommitDeferredSettings | converted (round 11) |
| HGSIM | 0x10039708 | set a voice's pitch | SetFrequency | converted (round 11) |
| HGSIM | 0x10039931 | 3D position of a unit's voice | 3D buffer SetAllParameters | converted (round 11) |
| HGSIM | 0x100399f4 | create a clip buffer | CreateSoundBuffer, QueryInterface | original: COM call fills stack out-pointers (frame-pointer rule) |
| HGSIM | 0x10039c00 | per-frame voice update | SetVolume, SetPan, Play, GetStatus, Stop, Release | original: GetStatus fills a stack dword (frame-pointer rule) |
| HGSIM | 0x1003a0e4 | sound start (A3D / DirectSound device, cooperative level, 3D listener) | CoCreateInstance, Initialize, DirectSoundCreate, SetCooperativeLevel, QueryInterface, CreateSoundBuffer, SetSpeakerConfig, Release | original (the earlier SDL3 stand-in objects are removed) |
| HGSIM | 0x1003a327 | pause all voices | GetStatus, Stop | original: GetStatus fills a stack dword (frame-pointer rule) |
| HGSIM | 0x1003a402 | resume paused voices | GetStatus, Play | original: GetStatus fills a stack dword (frame-pointer rule) |
| HGSIM | 0x1003a548 | sound shutdown | Release, CoUninitialize | converted (round 11) |
| HGSHELL | 0x1002f91d | menus: create the device | DirectSoundCreate, SetCooperativeLevel | n/a (real DirectSound) |
| HGSHELL | 0x1002fa6a | menus: release the device | Release | n/a (real DirectSound) |
| HGSHELL | 0x10031fb1 | menus: play a numbered clip (channel 2) | Release, CreateSoundBuffer, SetVolume, Lock, Unlock, Play | n/a (real DirectSound) |
| HGSHELL | 0x100321b1 | menus: stop channel 2 | Stop, SetCurrentPosition, Release | n/a (real DirectSound) |
| HGSHELL | 0x10032228 | menus: play a named clip (channel 1) | Release, CreateSoundBuffer, Lock, Unlock, Play | n/a (real DirectSound) |
| HGSHELL | 0x100323fa | menus: stop channel 1 | Stop, SetCurrentPosition, Release | n/a (real DirectSound) |

**Totals**: HGSIM 10 functions: 4 converted, 6 original (4 by the frame-pointer rule, 1 calling one of those, and
the sound start). HGSHELL 6 functions: none converted (the shell has no conversions;
its menu sounds go to the real DirectSound, there is no stand-in for HGSHELL). HG.EXE and the plugins: none.

**Update (2026-10-25, SDL3 sound)**: the table above is what runs without `sdlaudio`. With `sdlaudio` in hgrc.cfg,
all 10 HGSIM functions run as SDL3 versions (`src/sim/sound/sdl_sound.c`, all proven by the checker, see DECISIONS.md
"HGSIM sound: DirectSound replaced by SDL3"). HGSIM then makes no DirectSound calls. The earlier stand-in objects in
hooks_audio.c were removed after the game test. HGSHELL is unchanged.