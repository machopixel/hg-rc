# DirectSound calls of HGSIM.DLL

Read-only survey (2026-10-25) of how the sim uses DirectSound, and what the SDL3 stand-ins in
`src/hooks/hooks_audio.c` (`audio.h` / `audio_sdl3.c`, hgrc.cfg word `sdlaudio`) do with each call.

How the game uses sound: one DirectSound device; every sound effect is a whole 8-bit mono 22050 Hz clip loaded from
HEAVYG.PRJ into its own static buffer, played into one of 11 voice slots (the game's own mixer picks the slot,
priority and volume / pan). There is no streaming: no ring buffer and no music through DirectSound. Music is
CD audio through MCI (`src/sim/win/w6g_cd.c`). Optional DirectSound 3D (listener plus 3D buffers) is off in the
shipped HGSND.CFG.

| Object / method (vtable offset) | Where the game calls it | What it is used for | Stand-in (SDL3) |
| --- | --- | --- | --- |
| `CoCreateInstance(CLSID_DirectSound)`, `IDirectSound::Initialize` (+0x28), fallback `DirectSoundCreate` | 0x1003a0e4 (sound start) | create the device | replaced: the hooked 0x1003a0e4 hands out a stand-in device (SDL3 audio opened) |
| `IDirectSound::SetCooperativeLevel` (+0x18) | 0x1003a0e4 | exclusive / priority level for the window | accepted, does nothing |
| `IDirectSound::SetSpeakerConfig` (+0x24) | 0x1003a0e4 | speaker setup from HGSND.CFG | accepted, does nothing |
| `IDirectSound::CreateSoundBuffer` (+0xc) | 0x1003a0e4 (primary 3D buffer), clip loading | the primary buffer (3D only) and one static buffer per clip | implemented: a primary buffer object, or an SDL3 voice of the clip's size / format |
| `IDirectSoundBuffer::QueryInterface` (+0) | 0x1003a0e4, clip set-up | asks for the 3D listener / 3D buffer interfaces | answers "no interface"; the game then works without 3D |
| `IDirectSoundBuffer::Lock` (+0x2c), `Unlock` (+0x4c) | 0x1000e0d9 (load a clip into a slot) | copy the clip's samples into the buffer once | implemented: points at the voice's own memory; Unlock does nothing |
| `IDirectSoundBuffer::Play` (+0x30) | 0x10039c00 (per-frame mixer update), 0x1003a402 | start a clip, once or looping (engines) | implemented (`Audio_VoicePlay`, loop flag) |
| `IDirectSoundBuffer::Stop` (+0x48) | 0x10039c00, 0x1003a327 | stop a voice (slot reused, pause) | implemented |
| `IDirectSoundBuffer::SetVolume` (+0x3c) | 0x10039c00 | distance / priority volume of each voice | implemented, DirectSound range checks kept |
| `IDirectSoundBuffer::SetPan` (+0x40) | 0x10039c00 | left / right placement | implemented |
| `IDirectSoundBuffer::SetFrequency` (+0x44) | 0x10039708 (via 0x1000e9c9) | pitch of a running clip (engine revs) | implemented |
| `IDirectSoundBuffer::GetStatus` (+0x24) | 0x10039c00, 0x1003a327, 0x1003a402 | is the voice still playing / looping (frees finished slots) | implemented |
| `IDirectSoundBuffer::GetCurrentPosition`, `GetVolume`, `GetPan`, `GetFrequency`, `SetCurrentPosition`, `Restore` | (stand-in provides them; not seen in the sim's sound code) | - | implemented |
| `IDirectSoundBuffer::Release` (+8), `IDirectSound::Release` | clip unload, 0x1003a548 (shutdown, then CoUninitialize) | free buffers / the device | implemented (voice destroyed, SDL3 audio closed) |
| `GetCaps`, `GetFormat`, `Initialize`, `SetFormat`, `DuplicateSoundBuffer`, `GetSpeakerConfig` | not used by the game | - | stubbed: "not implemented" |
| `IDirectSound3DListener::SetAllParameters` (+0x28), `CommitDeferredSettings` (+0x44) | 0x100397e7 (listener from the camera, every frame) | 3D listener position / velocity / orientation | not provided (no 3D interface is handed out); runs only with 3D on |
| `IDirectSound3DBuffer::SetAllParameters` (+0x30) | 0x10039931 (unit position) | 3D position / velocity of a voice | not provided; runs only with 3D on |
| listener set-up method (+0x14) on the primary buffer's 3D interface | 0x1003a0e4 | 3D set-up (factor 2) | not provided |

Conclusion: everything the game needs without 3D sound (create, load clips, play / loop, stop, volume, pan, pitch,
status, release) already runs through SDL3. Only the optional 3D path has no SDL3 equivalent; with 3D off (as
shipped) the game computes volume and pan itself.

## All call sites, every module (read-only, 2026-10-25)

Modules checked: HGSIM.DLL, HGSHELL.DLL, HG.EXE and every DLL in the game folder and PLUGIN\ (3DFXGEAR, DX5DRAW, HGVK,
I76DRAW, RENDDRAW, ANET2, DKUPDDLL, FFORCE, KEYBOARD, PKWARE, SMACKW32, ZLIB). Only HGSIM.DLL and HGSHELL.DLL
import DSOUND.dll. HG.EXE (our launcher, with SDL3 inside) uses COM only for SDL3's WASAPI audio and DirectInput,
not DirectSound. The CoCreateInstance in HGSHELL (0x10022418) makes a DirectShow FilterGraph (video), not
DirectSound. Object types come from where each pointer is kept (HGSIM: device 0x109e369c, sound block [0x1015d0a4]:
+8 primary buffer, +0xc 3D listener, +0x94 + 8i clip buffers, +0x114 + 4i 3D buffers; HGSHELL: the sound object's
+0xc device, +0x10 and +0x14 buffers). One line per call site: module, calling function, method, purpose.

### HGSIM.DLL (36 call sites in 10 functions)

| Module | Caller | Method | Purpose |
| --- | --- | --- | --- |
| HGSIM | 0x1003a0e4 | CoCreateInstance (Aureal A3D, IID_IDirectSound) | try the A3D driver first |
| HGSIM | 0x1003a0e4 | IDirectSound::Initialize (+0x28) | initialise the A3D-made device |
| HGSIM | 0x1003a0e4 | DirectSoundCreate | fallback: the plain DirectSound device |
| HGSIM | 0x1003a0e4 | IDirectSound::SetCooperativeLevel (+0x18) | level 3 (exclusive) for the window |
| HGSIM | 0x1003a0e4 | IDirectSound::QueryInterface (+0) | ask for the A3D interface |
| HGSIM | 0x1003a0e4 | A3D interface method (+0x14) | A3D setting (value 2) |
| HGSIM | 0x1003a0e4 | A3D interface Release (+8) | done with it |
| HGSIM | 0x1003a0e4 | IDirectSound::CreateSoundBuffer (+0xc) | the primary buffer (3D only) |
| HGSIM | 0x1003a0e4 | IDirectSoundBuffer::QueryInterface (+0) | the 3D listener from the primary buffer |
| HGSIM | 0x1003a0e4 | IDirectSoundBuffer::Release (+8) | primary buffer released when no listener |
| HGSIM | 0x1003a0e4 | IDirectSound::SetSpeakerConfig (+0x24) | speaker setting from HGSND.CFG |
| HGSIM | 0x100399f4 | IDirectSound::CreateSoundBuffer (+0xc) | a clip's static buffer |
| HGSIM | 0x100399f4 | IDirectSoundBuffer::QueryInterface (+0) | its 3D buffer interface (3D only) |
| HGSIM | 0x1000e0d9 | IDirectSoundBuffer::Lock (+0x2c) | open the new clip buffer for writing |
| HGSIM | 0x1000e0d9 | IDirectSoundBuffer::Unlock (+0x4c) | after copying the samples |
| HGSIM | 0x10039708 | IDirectSoundBuffer::SetFrequency (+0x44) | pitch of a playing voice |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::SetVolume (+0x3c), 2 sites | voice volume each frame |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::SetPan (+0x40), 2 sites | voice pan each frame |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::Play (+0x30), 2 sites | start a voice, once or looping |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::GetStatus (+0x24) | finished voices free their slot |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::Stop (+0x48) | stop a voice |
| HGSIM | 0x10039c00 | IDirectSoundBuffer::Release (+8), 2 sites | release finished one-shot buffers |
| HGSIM | 0x1003a327 | IDirectSoundBuffer::GetStatus (+0x24) | which voices play (pause) |
| HGSIM | 0x1003a327 | IDirectSoundBuffer::Stop (+0x48) | pause: stop them all |
| HGSIM | 0x1003a402 | IDirectSoundBuffer::GetStatus (+0x24) | which voices were paused |
| HGSIM | 0x1003a402 | IDirectSoundBuffer::Play (+0x30), 2 sites | resume, once or looping |
| HGSIM | 0x100397e7 | IDirectSound3DListener::SetAllParameters (+0x28) | listener from the camera (3D only) |
| HGSIM | 0x100397e7 | IDirectSound3DListener::CommitDeferredSettings (+0x44) | apply it (3D only) |
| HGSIM | 0x10039931 | IDirectSound3DBuffer::SetAllParameters (+0x30) | a unit's voice position (3D only) |
| HGSIM | 0x1003a548 | IDirectSound::Release (+8) | shutdown |
| HGSIM | 0x1003a548 | CoUninitialize | shutdown |

### HGSHELL.DLL (20 call sites in 6 functions; the menus' own sound object, real DirectSound)

| Module | Caller | Method | Purpose |
| --- | --- | --- | --- |
| HGSHELL | 0x1002f91d | DirectSoundCreate | the menus' device |
| HGSHELL | 0x1002f91d | IDirectSound::SetCooperativeLevel (+0x18) | level 3 for the window |
| HGSHELL | 0x1002fa6a | IDirectSound::Release (+8) | shutdown |
| HGSHELL | 0x10031fb1 | IDirectSoundBuffer::Release (+8) | drop the previous clip of channel 2 |
| HGSHELL | 0x10031fb1 | IDirectSound::CreateSoundBuffer (+0xc) | buffer for a numbered menu clip (8-bit mono 22050 Hz) |
| HGSHELL | 0x10031fb1 | IDirectSoundBuffer::SetVolume (+0x3c) | -1500 (quieter) |
| HGSHELL | 0x10031fb1 | IDirectSoundBuffer::Lock (+0x2c) / Unlock (+0x4c) | copy the clip in (2 sites) |
| HGSHELL | 0x10031fb1 | IDirectSoundBuffer::Play (+0x30) | play it once |
| HGSHELL | 0x100321b1 | IDirectSoundBuffer::Stop (+0x48), SetCurrentPosition (+0x34), Release (+8) | stop channel 2 (3 sites) |
| HGSHELL | 0x10032228 | IDirectSoundBuffer::Release (+8) | drop the previous clip of channel 1 |
| HGSHELL | 0x10032228 | IDirectSound::CreateSoundBuffer (+0xc) | buffer for a named menu clip |
| HGSHELL | 0x10032228 | IDirectSoundBuffer::Lock (+0x2c) / Unlock (+0x4c) | copy the clip in (2 sites) |
| HGSHELL | 0x10032228 | IDirectSoundBuffer::Play (+0x30) | play it once |
| HGSHELL | 0x100323fa | IDirectSoundBuffer::Stop (+0x48), SetCurrentPosition (+0x34), Release (+8) | stop channel 1 (3 sites) |

### HG.EXE and the plugins: no DirectSound call sites.

**Update (2026-10-25)**: the SDL3 stand-in objects described above were removed from hooks_audio.c. With `sdlaudio`,
the 10 HGSIM functions run as SDL3 versions (`src/sim/sound/sdl_sound.c`); without it, the original DirectSound code runs.
