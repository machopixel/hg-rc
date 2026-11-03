# Decisions

This file records the places where the port deliberately differs from the original HGSIM.DLL, and the functions
that are still the original code on purpose. Every converted function is proven against the original by the
checker (`tools/verify/difftest.py`, one spec per function in `tools/verify/spec_w10_<address>.py`), and by
default that proof is bit-exact. The entries below are the exceptions. New decisions get appended here.

## Interpreted: modern math instead of the x87 C runtime

The original game was built with Visual C++ 5.0 for the x87 FPU. Its C runtime computes `sin`, `cos`, `asin` and
`acos` with x87 instructions (`fsin`, `fcos`, `fpatan`) and the game keeps computing with those results. The port
uses 64-bit doubles and the platform math library (the UCRT on Windows, bionic on Android), whose results can
differ from the x87 ones in the last bits. Diego accepted that on 2026-10-25 for the functions below. They call the
platform's `sin` / `cos` / `asin` / `acos`; everything else in them (control flow, memory writes, calls, return
value) is exactly the original's.

**How they are proven** (two steps):

1. One shared body (`src/sim/util/w10_interp_body.h`) is compiled twice. `src/sim/util/w10_interp_x87.c` does the
   math with the original runtime's own x87 instruction sequences (control word 0x27f, `asin` / `acos` as
   `fpatan` of `sqrt((1 + x)(1 - x))`). That reference is proven **bit-exact against the original** by the checker
   (`tools/verify/spec_w10_<address>_x87.py`). It is kept in the repository but not hooked and not in the game
   build (gcc inline assembly, 32-bit x86 only).
2. `src/sim/util/w10_interp.c` is the same body with the platform math; this is the hooked version.
   `tools/verify/interp_measure.c` measures it against the reference natively, C against C, with the Windows
   UCRT (`ucrtbase.dll`, the library the game build links), over random inputs that include the domain-error
   cases. Every output word, every word of sim memory the function writes and every logged callee argument is
   compared.

The specs `tools/verify/spec_w10_<address>.py` (no `_x87`) check the hooked version directly against the original
in the emulator with a float tolerance (`fp=True`). They are slower and are kept for spot checks; their short runs
passed.

**Shared behaviour**:

- **Domain errors** stay as in the original. For `asin` / `acos` outside -1..1, and for `sin` / `cos` of an
  infinite or NaN argument, the original runtime stores EDOM (33) in its `errno` at 0x1017e888 and returns the x87
  "indefinite" NaN (or the NaN it was given). The wrappers `W10i_Asin`, `W10i_Acos`, `W10i_Sin` and `W10i_Cos`
  do the same and call the platform function only inside the domain.
- **Huge angles**: for a `sin` / `cos` argument beyond about 1e6 radians, the original reduces it with an 80-bit
  2 pi and the platform library with an exact one, so results there can differ widely. The game passes ordinary
  angles (degrees times pi/180, blend factors around 0..1), so neither step tests huge finite angles.

### 0x10060681 Sim_10060681: rotation matrix to pitch, heading, roll

Turns a 3x3 rotation matrix into angles in degrees: pitch from `asin` (near level) or `acos` of the horizontal
length, and heading and roll from Sim_1008486e. The original multiplies the runtime's x87 `asin` / `acos` result
before rounding it to a float. Modern calls: `asin`, `acos`. Reference proof: 6000 trials, 0 mismatches, 112/112
instructions run. Measured: 1,000,000 trials, no output differs (tolerance 0). Source: `w10_interp_body.h`.

### 0x100d71b9 Sim_100d71b9: aim a node along a direction

Sets a node's rotation from a direction (pitch from `asin` of the vertical part, heading from Sim_1008486e). It
optionally adds a random spread along the side and up vectors, places the node and sets its velocity. The original
multiplies the x87 `asin` result before rounding. Modern call: `asin`. Reference proof: 6000 trials,
0 mismatches, 140/140 instructions. Measured: 1,000,000 trials, no output or logged argument differs
(tolerance 0). Source: `w10_interp_body.h`.

### 0x1000cbaa Sim_1000cbaa: weapon ballistic table

For every weapon kind with flag 0x40, it fills a row of 20 ranges with the launch angle
`0.5 * asin(g * range / speed^2)` in degrees (45 when out of reach) into the table at 0x10a3a600. The original
multiplies the x87 `asin` result before rounding. Modern call: `asin`. Reference proof: 6000 trials, 0 mismatches,
118/118 instructions. Measured: 100,000 trials (46 kinds x 20 entries each), no table word differs (tolerance 0).
Source: `w10_interp_body.h`.

### 0x100608ef Sim_100608ef: quaternion slerp

Blends two orientations: spherical blend with `acos` / `sin`, linear when they are nearly equal, and through a
perpendicular one when they are nearly opposite. The original divides the x87 `sin` results before rounding.
Modern calls: `acos`, `sin`. Reference proof: 6000 trials, 0 mismatches, 192/207 instructions run; the 15 unrun
ones are the three null-pointer assert calls, which the game never triggers (the original would crash right after
them). Measured: 1,000,000 trials, no output differs (tolerance 0). Source: `w10_interp_body.h`.

### 0x10060220 Sim_10060220: rotation matrix from angles

Builds a rotation matrix from three angles in degrees: a fast path for a single axis and the general product in
three axis orders, with an option for the transpose. The original stores and multiplies the x87 `sin` / `cos`
results. Modern calls: `sin`, `cos`. Reference proof: 6000 trials, 0 mismatches, 363/363 instructions. Measured:
1,000,000 trials. Matrix entries of magnitude 1e-6 or more never differ. Entries within 4.05e-21 of zero
(for example cos of 90 degrees) differ in 17% of trials: x87 `fcos` / `fsin` use a 66-bit pi internally, while the
UCRT is exact there. **Tolerance: absolute 4.05e-21 near zero, 0 elsewhere.** Source: `w10_interp_body.h`.

### 0x1001a706 Sim_1001a706: a unit following a path (round 14)

The path-follow task handler: mode 0 parses the task text ("key;how,turn,path") into a 0x88-byte record, mode -1
restarts it, mode 1 places the unit's node along the path. With turning on, the pitch is the point's pitch less
`asin(rise / 65536)` in degrees; the original multiplies the x87 `asin` result before rounding. Modern call: `asin`.
Reference proof (`spec_w14_1001a706_x87.py`): 15000 trials, 0 mismatches, 825/825 instructions. The hooked version
against the original (`spec_w14_1001a706.py`, fp): 15000 trials, worst deviation 0. Measured
(`interp_measure.c`): 1,000,000 trials of mode 1, no word of the record or the logged arguments differs
(tolerance 0). An `asin` argument outside -1..1 (a rise over 65536) makes the original loop forever on the NaN
angle; the specs stay inside the domain. Source: `w10_interp_body.h`. Its text fields: see "Round 14" below.

## Left original on purpose

These functions are not converted, so the original code runs. The rule leaves a function out when the original
reads stack memory it never wrote (the result then depends on whatever the stack holds, which C cannot
reproduce), when it can overflow a stack buffer into its own frame, when it hands Windows a pointer into its frame,
or when it calls a function left out for one of these reasons. One line per function, by address.
- `0x1000aa9e`: passes uninitialised locals [ebp-0x64] and [ebp-4] to 0x100cbc6b (the three differences are all stored to [ebp-0x60])
- `0x100139ff`: calls 0x100c2ae5 (reads uninitialised stack; its garbage comes from the caller's frame)
- `0x10019fe1`: copies four locals that sscanf (0x100fd710, '%f %f %f %f') may leave unwritten (uninitialised stack)
- `0x1001a23e`: uses locals that sscanf (0x100fd710) may leave unwritten (uninitialised stack)
- `0x1001a4fa`: when the text has no ';' (or sscanf stops early) it passes its uninitialised 100-byte stack buffer [ebp-0x78] to 0x100b8d5c
- `0x1001a706`: reads its text with sscanf("%[^,],%[^,],%[^,]") into fixed stack buffers: a line with fewer fields leaves buffers unwritten that stricmp then reads, and a long field overflows into the frame (also uses an 80-bit asin result, which alone would have been interpreted)
- `0x10021e8c`: passes the never-written locals [ebp-0x8c], [ebp-0x94], [ebp-8] to Sim_100246b3 as the camera target (uninitialised stack)
- `0x100232d7`: uninitialised stack (miss path reads target [ebp-0x2c] at 0x10023d35; 0x1009cf67 gets the never-built segment [ebp-0xdc] at 0x10023add)
- `0x1003f3f8`: caller of left-out 0x1009d200 at 0x1003fc14
- `0x10048f02`: caller of left-out 0x1009d200 at 0x10049464 (chain to 0x1005bb7a)
- `0x1004954e`: overflow into the frame (sprintf of runtime message strings into [ebp-0x40]: 40+ chars overwrite [ebp-0x18], read at the end; 64+ reach the saved ebp/return)
- `0x10055048`: when the input device [0x1015fdf4] is not open it still reads the local word [ebp-4] that only the device's +0x18 routine writes (uninitialised stack)
- `0x1005621b`: passes its own 0x194-byte stack buffer to Windows (import [0x10ac5860], joystick caps) - Windows pointer-to-frame call, excluded by rule
- `0x100567cb`: passes its own stack buffers (0x194-byte caps, 0x34-byte position record) to Windows (imports [0x10ac5860] / [0x10ac585c], joystick caps / position) - Windows pointer-to-frame calls, excluded by rule
- `0x1005ab44`: on the path where the node has no name at this level it passes the uninitialised local [ebp-0x10] to its own recursive call
- `0x1005bb7a`: uninitialised stack ([ebp-0xc0]/[ebp-0xa8] written only in one switch case at 0x1005bee8 yet passed to 0x1004bce5 after the loop; [ebp-0x84] read at 0x1005c4f0 written only by the 0x1004d330 cases)
- `0x1005cd4a`: caller of left-out 0x1005bb7a (uninitialised stack) at 0x1005cde1
- `0x10062b7c`: copies the localised HUD labels (loaded at run time) into 0x10/0x14-byte stack buffers with strcpy; a label of 20+ characters runs into the screen x and the saved frame pointer, so the shown text then depends on the original's frame bytes (C proven equal for labels up to 19; spec and C kept in src/sim/unsorted_1006/w10_hud.c, not hooked)
- `0x10063529`: calls 0x100648a0 / 0x10064d05, which use an uninitialised local as the palette index
- `0x10064364`: when the first shape ([0x109e8484]) does not load, the heading text uses the uninitialised local [ebp-0xc] (_ftol of stack garbage into sprintf)
- `0x100648a0`: as 0x10064d05, for a kind other than 0..2 the on-screen branch uses the uninitialised local [ebp-0x14] as the palette index
- `0x10064d05`: for a kind other than 0..2 the on-screen branch uses the uninitialised local [ebp-0x14] as the palette index
- `0x100663c0`: calls 0x10066907 (left out: uninitialised-stack chain through 0x100687a3)
- `0x10066907`: calls 0x100688a3 / 0x100689c0, which call 0x100687a3 (reads uninitialised locals)
- `0x1006788c`: calls 0x10067ec6 (uninitialised stack; its garbage comes from the caller's frame)
- `0x10067ec6`: when both units are of kind 9 (or neither is), the killer [ebp-4] it passes to Sim_1006c161 is never set (uninitialised stack)
- `0x100680dd`: calls 0x10067ec6 (uninitialised stack; its garbage comes from the caller's frame)
- `0x100687a3`: reads uninitialised locals [ebp-0x18]/[ebp-0x1c]/[ebp-4] (compared with 0x1012b600 before they are written)
- `0x100688a3`: calls 0x100687a3 which reads uninitialised locals; C + spec kept, proven 1500
- `0x100689c0`: calls 0x100687a3 (reads uninitialised locals)
- `0x1006e22c`: calls 0x100663c0 (left out: uninitialised-stack chain through 0x100687a3)
- `0x10070eb5`: overflow into the frame (sprintf of a runtime message into [ebp-0x7c] at 0x1007234d: 92+ chars overwrite the unit pointer [ebp-0x20] used afterwards, 124+ reach the saved ebp/return)
- `0x1007581a`: uninitialised stack (strncpy of a label longer than the width into [ebp-0x104] leaves no NUL; strchr(' ') at 0x10076201 then scans unwritten stack when the label has no space)
- `0x100782a9`: calls 0x100b66a0, which tests and divides by an uninitialised local
- `0x10089066`: as 0x1008950e, the leaf path returns a value from the uninitialised local [ebp-4]
- `0x1008950e`: on the leaf path (no children) its return value comes from the uninitialised local [ebp-4]
- `0x100918e6`: hands the renderer ([0x1014cb4c]) stack vertices whose fields +8, +0x14, +0x18 it never sets (uninitialised stack), as 0x1009596a
- `0x1009221d`: hands the renderer ([0x1014cb4c]) stack vertices at ebp-0x90 whose fields +8, +0x14, +0x18 it never sets (uninitialised stack), as 0x100918e6
- `0x1009353e`: hands the renderer ([0x1014cb4c]) stack vertices with fields it never sets (+8, +0x18: uninitialised stack)
- `0x100945b8`: hands the renderer ([0x1014cb4c]) a stack array of four 0x24-byte vertices with fields it never sets (+8, +0x18, and +0x14 of three vertices: uninitialised stack)
- `0x1009596a`: hands the renderer ([0x1014cb4c]) four stack vertices whose fields +8, +0x14, +0x18 it never sets (uninitialised stack), as 0x1009353e
- `0x1009d200`: caller of left-out 0x1005cd4a at 0x1009d41a (chain to 0x1005bb7a)
- `0x1009ef50`: calls 0x100ad96e (left out: uninitialised-stack chain through 0x100ac9ab)
- `0x1009fada`: hands the renderer ([0x109e3d30]) a 0x1200-byte stack vertex buffer whose vertices have bytes it never sets (+0x17 and others: uninitialised stack)
- `0x100a0063`: uses the many locals of an unchecked sscanf (0x100fd710) of a file line (uninitialised stack when the line is short)
- `0x100a0880`: builds four 0x58-byte stack vertices (only +0xc..+0x1c and +0x2c set) and hands them to the clipper / renderer path 0x100a0512 -> 0x100a0339; the unset fields (+0..+8, +0x31..+0x57) are uninitialised stack
- `0x100a539e`: calls 0x1009fada (passes uninitialised stack to the renderer)
- `0x100a65e7`: at 0x100a695f it copies the local [ebp-0x78] into [ebp-0x2c] before [ebp-0x78] is ever written (uninitialised stack read)
- `0x100ac9ab`: builds 0x58-byte stack vertices (only +0xc..+0x1c and +0x2c set) in its 0x57b0-byte frame buffer and hands them to 0x100ac738 (clip / render path): uninitialised stack, as 0x100a0880
- `0x100ad96e`: calls 0x100ac9ab (uninitialised stack; its garbage comes from the caller's frame)
- `0x100b48de`: strcpy's model and sub-model names (32-byte slots of the loaded model) into a 16-byte stack buffer right below its loop counter and model count; a name of 16+ characters overwrites them in the original, which C cannot reproduce without the same frame
- `0x100b66a0`: tests the uninitialised local [ebp-0x10] on the first loop pass (and divides by it)
- `0x100be0f0`: calls 0x100c2ae5 (reads uninitialised stack; its garbage comes from the caller's frame)
- `0x100c0158`: for a selection kind other than 0x200/0x400 it passes the uninitialised local [ebp-0x14] to 0x100aa863
- `0x100c2ae5`: reads the local [ebp-0xc] that Sim_1002dda9 / Sim_100c2851 leave unwritten when they find nothing (Sim_100c292c can still say hit): uninitialised stack
- `0x100c7120`: uninitialised stack (type 0x100 path never writes [ebp-0x28] / [ebp-0x70..-0x7c] before cmp at 0x100c786d and the 0x100d2669 call; 0x400 path the same when 0x100cba68 returns 0)
- `0x100e2aa3`: uninitialised stack (strncpy(buf [ebp-0x94], name, 16) leaves no NUL for a 16-char name; strupr at 0x100e2c3f and the fprintf %s read on into unwritten locals)

## Round 11 deviations (2026-10-25)

- **0x1005f6c6 Sim_1005f6c6 (language DLL setup)**: the original passes the language word to the DLL function
  [0x1015dc5c] with `mov cx, word ptr [eax]; push ecx`, so the upper 16 bits of that argument are whatever the
  preceding call through [0x1015dc3c] left in ecx (not reproducible). The C passes the word zero-extended; the
  checker compares only the low 16 bits of that argument. The function takes a 16-bit language id.
- **0x1005f6c6 and 0x10080471 call Sim_10083495 without arguments**: the original pushes a message (and an error
  code) for it, but the converted Sim_10083495 takes none and ignores them, so the C does not pass them.

## Left original in round 11

From the 40 never-attempted functions (analysis in `tools/round11/analysis.txt`). Calls a Windows / COM function
with a pointer into the frame (left original by the rule; Diego may decide otherwise):

- `0x100819e2`: passes a stack DDPIXELFORMAT to the DirectDraw method GetPixelFormat (+0x54), which fills it
- `0x100399f4`: passes stack structs to a COM method (+0xc, sound buffer creation) that writes an out-pointer
- `0x1003a327`, `0x1003a402`, `0x10039c00`: COM method +0x24 (sound buffer GetStatus) fills the stack dword [ebp-4]
- `0x10081286`, `0x1008181e`, `0x10081416`: COM method +0x14 gets a stack out-pointer [ebp-0x10]
- `0x100815e8`: COM method +0x1c gets a stack out-pointer [ebp-0x10]
- `0x1007f35c`: SendMessageA with a stack record [ebp-0x1814] (Windows only reads it)
- `0x1007ea30`: LoadModule with a parameter block on the stack

Other reasons:

- `0x10020055`: reads the never-written local [ebp-4] (uninitialised stack)
- `0x100b9dde`: fgets may leave its 256-byte stack buffer unwritten before strstr reads it, and [ebp-4] is read when strstr fails (uninitialised stack)
- `0x100b8d6c`: stores the word [ebp-0x10] through its first argument on a path that never writes it (uninitialised stack)
- `0x100db78b`: strcpy of record names into the 12-byte stack buffer [ebp-0x98] can overflow into [ebp-0x80], used afterwards
- `0x100ed36f`, `0x100ed388`: part of the HUD 2D library (excluded)
- `0x1003cfd6`, `0x10040af8`, `0x1007fe6f`, `0x100805d6`, `0x10080c57`, `0x10081a7a`, `0x10081e1d`, `0x100821b1`, `0x100a0def`, `0x100b82a0`, `0x100dcc12`: call a function left original (register arguments, the HUD library or an uninitialised-stack one)

## HGSIM sound: DirectSound replaced by SDL3 (2026-10-25)

All of HGSIM's DirectSound calls are in 10 functions. Each one is rewritten in `src/sim/sound/sdl_sound.c`
(`SndSdl_<address>`) as the game's own code, line for line. The only change is that every DirectSound call is
replaced, in the same place, by the matching `platform/audio.h` call with the same values (DirectSound units:
volume and pan in 1/100 dB, rate in Hz, Lock byte ranges). The sound block's voice slots ([0x1015d0a4] + 0x94 + 8 ×
slot) hold `AudioVoice` pointers instead of DirectSound buffers. The device global 0x109e369c holds the address of
`SndSdl_Device`, a token that is only tested against 0 and passed on, never read.

`hooks_audio.c` installs these functions only when hgrc.cfg has the word `sdlaudio` (since 2026-10-29: by default,
see "SDL3 sound and Vulkan by default" below). Without that word, the original
DirectSound code (and the round-11 CONV versions of 0x10039708, 0x100397e7, 0x10039931 and 0x1003a548) runs as
before. With `sdlaudio`, `hooks_conv.c` leaves those 4 CONV entries out, along with the `soundtrace` hook on 0xE0D9
and the FPU log tap on 0x39C00, so each address is hooked once. The earlier stand-in COM objects in `hooks_audio.c` (fake DirectSound objects
that fed audio.h) were removed after Diego's game test (2026-10-25).

Proof: `tools/verify/spec_snd_*.py` (`tools/verify/sound_common.py`). The original runs against fake DirectSound
objects and the C runs against stubs of the audio.h functions. Both sides log the same events (Create, Release,
Play, Stop, Volume, Pan, Rate, Status, Lock, Unlock, Open, Close) and get the same answers. The checker compares the
events, the memory and the return value exactly. Logs are in `buildlogs/sound`.

| DirectSound call (method offset) | audio.h call |
|---|---|
| DirectSoundCreate (A3D CoCreateInstance + Initialize before it, CoInitialize) | `Audio_Open` |
| IDirectSound Release (+0x08), CoUninitialize | `Audio_Close` |
| IDirectSound CreateSoundBuffer (+0x0c), 8-bit mono 22050 Hz static buffer | `Audio_VoiceCreate(bytes, 22050, 1, 8)` |
| IDirectSoundBuffer Release (+0x08) | `Audio_VoiceDestroy` |
| GetStatus (+0x24) | `Audio_VoiceGetStatus` (same PLAYING 0x1 / LOOPING 0x4 flags) |
| Lock (+0x2c) / Unlock (+0x4c) | `Audio_VoiceLock` / `Audio_VoiceUnlock` (a pointer into the voice's own clip memory) |
| Play (+0x30), DSBPLAY_LOOPING | `Audio_VoicePlay(voice, loop)` |
| SetVolume (+0x3c) / SetPan (+0x40) / SetFrequency (+0x44) | `Audio_VoiceSetVolume` / `Audio_VoiceSetPan` / `Audio_VoiceSetRate` |
| Stop (+0x48) | `Audio_VoiceStop` |

| Function | What it does | Trials | Original instructions run |
|---|---|---|---|
| 0x1003a0e4 | sound start | 1000 | 58/181 (A3D, SetCooperativeLevel failure, 3D: below) |
| 0x1003a548 | sound shutdown | 500 | 12/12 |
| 0x100399f4 | new clip voice | 1000 | 69/69 |
| 0x1000e0d9 | slot search and clip load | 3000 | 291/295 (Unlock failure) |
| 0x10039c00 | per-frame voice update | 3000 | 284/292 (error names after a failed Play) |
| 0x1003a327 | pause | 3000 | 57/57 |
| 0x1003a402 | resume | 3000 | 77/77 |
| 0x10039708 | pitch | 1000 | 14/14 |
| 0x100397e7 | 3D listener | 3000 | 74/82 (error names after failed 3D calls) |
| 0x10039931 | 3D voice position | 2000 | 49/53 (error names after failed 3D calls) |

What is not exact (SDL3 has no counterpart):

- **3D sound**. 0x100397e7 and 0x10039931 keep all their game logic and memory writes (the parameter blocks at
  0x109e36e0 / 0x109e3720), but the listener's SetAllParameters / CommitDeferredSettings and the 3D buffer's
  SetAllParameters are gone. 0x100399f4 sets the 3D buffer out-pointer to 0 instead of calling QueryInterface;
  that is the answer the earlier stand-in objects gave too. In the sound start, HGSND.CFG's 3D word (`[[0x1015d1b8] + 0x40]`) no
  longer creates the 3D primary buffer and listener, and SetSpeakerConfig and Sim_10039725 are not called. The 3D
  flag (block + 0x10) stays 0, as when HGSND.CFG turns 3D off (the shipped config). The game then works out volume
  and pan itself.
- **Aureal A3D**. The sound start no longer tries A3D (CoCreateInstance, Initialize, and the A3D-only
  QueryInterface with its +0x14 call). The checker runs the original with A3D absent.
- **SetCooperativeLevel** is gone. SDL3 has nothing in its place, so its failure path (Sound_ErrorName and the
  shutdown) never runs. If `Audio_Open` fails, the original's "DirectSoundCreate failed" path runs (Sim_100d9fc0,
  return -12). The earlier stand-in objects fell back to DirectSound instead.
- **CoInitialize / CoUninitialize** are gone. They were only there for A3D and DirectSound.
- **Lock / Unlock failure (0x1000e0d9)**. `Audio_VoiceLock` always returns a valid pointer for a voice just created
  with that size, so the Lock-failure path (Sound_ErrorName) cannot happen. Coordinator's approval: "Our
  Audio_VoiceLock always returns valid pointers for a buffer we just created, so the Lock-failure path can't happen;
  record that in DECISIONS.md." `Audio_VoiceUnlock` has no result, so the Unlock error-name call is gone.
- **Error names after other failed DirectSound calls** (Play in 0x10039c00, the 3D functions). Those audio.h calls
  cannot fail, so those paths never run.
- **Out-of-range volume, pan and rate** (fixed 2026-10-25). The audio.h setters now refuse what DirectSound refuses,
  keep the old value and return DSERR_INVALIDPARAM (`AUDIO_ERR_INVALIDPARAM`, 0x80070057). The ranges are: volume
  -10000..0, pan -10000..10000, rate 100..100000 or 0 (the clip's own rate). The rate limits are the DirectSound
  version the game uses; DirectX 9 raised the maximum to 200000 for its newer interfaces. Audio_VoiceLock returns the
  same code for a bad range. 0x10039c00 names a refused SetVolume (Sound_ErrorName) where the original does: the
  start of a voice. The other SetVolume, the SetPan calls and SetFrequency ignore the result, as in the original. The
  checker's fake DirectSound refuses the same values, and the specs feed out-of-range volumes, pans and rates.
  0x10039c00 now runs 284/292 original instructions; the 8 left are the error names after a failed Play, which
  Audio_VoicePlay cannot do. Sound_ErrorName only looks up a text, so the checker can't see whether it was called. The
  part you can hear (the old value kept) is in audio_sdl3.c, which the checker stubs out.
- **Clips under 4 bytes**. DirectSound refuses a buffer under 4 bytes, and the original then crashes calling Lock
  through the null buffer (0x1000e0d9). `Audio_VoiceCreate` refuses only 0 bytes. Real clips always have data, so
  this is left as it is.

## Round 12: uninitialised stack (2026-10-26)

These functions read a local the original never writes on some path, so the original sees whatever an earlier
function left on the stack. Each one is converted with a chosen start value for that local.

- The evidence is either the uninittrace watcher runs (`tools/round10/uninit_run1.md` with the C,
  `tools/round10/uninit_run2.md` with noconv), or "harmless": only broken or missing data files reach the path.
- None of these unset reads was seen in play, except 0x100b66a0, which was 1 in every call with the original.
- The checker proves every other path exactly. For the unset path, the spec puts the chosen start value into the
  original's stack as the leftover, so both sides compare.

| Function | Unset local, and the path that reads it | Start value | Evidence |
|---|---|---|---|
| 0x100b9dde (language letter from lang.ini) | the 256-byte line [ebp-0x104] when fgets reads nothing; the pointer [ebp-4] when the line has no "language =" | the line empty; the pointer at the line's start, so the result is the line's first letter upper-cased, or 0 | harmless: only a missing or bad lang.ini (not watched) |
| 0x1001a4fa (`key;n,name,n` record handler) | the 100-byte name buffer [ebp-0x78] when the text has no ';' or sscanf stops early | none needed: the buffer only goes to 0x100b8d5c, which ignores its arguments and returns -1 | harmless: never observable (not watched) |
| 0x100c0158 (position of a selected thing) | the node [ebp-0x14], for a selection kind other than 0x200 / 0x400 | 0 | watcher: 21 calls in each run, the unset path never taken |
| 0x10019fe1 (`key;a,b,c,d` motion record handler) | the four floats [ebp-0x10..-0x1c] that sscanf `%f,%f,%f,%f` leaves unfilled when a line has fewer fields | 0.0 for each unfilled field | harmless: only a short line in the data file (not watched) |
| 0x1001a23e (`key;a,b,n,n` turning record handler) | the sscanf `%f,%f,%d,%d` fields [ebp-0x28], [ebp-0x30], [ebp-0x14] (and the unused [ebp-0x2c]) when a line has fewer fields | 0 for each unfilled field | harmless: only a short line in the data file (not watched) |
| 0x100a0063 (sky and ground settings from skygnd.par) | the sscanf targets on the stack (mission name [ebp-0x248], hardware horizon [ebp-0x2ac], gintensity, gambient, the four sky colours, the three densities) when the mission's line is shorter than the format | 0 / empty; the intensities keep the original's 0x80 and the hardware ambient its 0 | harmless: only a short line in skygnd.par (not watched) |
| 0x100b66a0 (HUD damage display) | the divisor [ebp-0x10], tested against 0 on the first pass before it is written | 1 | watcher: unset on every call, always 1 with the original (run 2) and 5 with the C (run 1); only its being non-zero matters, and 0 would flip the branch |
| 0x100648a0 / 0x10064d05 (HUD markers of a unit / a thing) | the palette index [ebp-0x14] on screen for a kind other than 0..2 | 0 (the palette of kind 1) | harmless: the callers pass kinds 0..2 (not watched). The eight name strings copied into the 16-byte stack buffers are the 16-byte pieces of 0x101362e0 (all NUL-terminated in the DLL), so they cannot overflow |
| 0x10064364 (HUD compass) | the heading [ebp-0xc], printed as text when the dial shape did not load | 0.0 (the text shows 0) | harmless: only when a HUD resource is missing (not watched) |
| 0x100c2ae5 (contact test of a moving unit) | the thing found [ebp-0xc], when both finders find nothing but Sim_100c292c reports a hit (first probe or the opposite-side probe) | 0 (nothing found: the probing stops) | watcher: 75k calls over both runs; the hit path ran 146 times, the value was always written |
| 0x10055048 (input polling) | the key word [ebp-4], when the keyboard device [0x1015fdf4] is not open | 0 (no key) | watcher: 20.7k calls over both runs, always written by the keyboard first |
| 0x100c7120 (target readout pane) | the target position / size [ebp-0x70..-0x7c] and rank [ebp-0x28] for a target kind other than 0x200 / 0x400 (and the position / size for 0x400 when Sim_100cba68 gives no object) | 0 | watcher: 20k calls over both runs, the projection part never reached with an unset value |
| 0x100e2aa3 (ReadWTB, one model object) | the stack after the 16-byte name buffer [ebp-0x94], read by strupr and the log fprintf when the name has exactly 16 characters (strncpy leaves no NUL) | the name ends after its 16th character | harmless: only exactly-16-character object names (not watched) |

Left original in round 12 (they read an unset local that `tools/round10/uninit_stack_list.md` did not list):

- `0x1007581a` (label wrapping): the item position [ebp-0x144] / [ebp-0x140] is written only when the menu entry's
  +4 text is set, but always passed to 0x1008d65d / 0x1008d6f7.
- `0x1005bb7a` (POF model loader): the class [ebp-0x14] is set only by a part of type 2 with sub-kind 0..4, but it
  is always stored to +0xbdb (0x1005cb66).

## Removed: the sim's DirectDraw and DISPDIB display drivers (2026-10-27)

The sim chooses a display mode from the table at 0x1013a2a8: modes 0..3 use its built-in DirectDraw driver (device
class 0), mode 4 DISPDIB.DLL (class 1), mode 5 the windowed GDI driver (class 2) and mode 6 the renderer plugin
(class 3). A game run with a probe on both drivers' start functions (dev.cmd 2, Vulkan) saw 0 starts of either and
ended in mode 6, class 3. So these drivers are dead code with the renderer plugin. They are not translated:
`src/hooks/hooks_dispprobe.c` replaces their two start functions with "not available" (Display_SetMode then tries the
next mode), and a debug.log line reports if one is ever asked for.

**Corrected in round 21 (2026-10-29): the DirectDraw driver is NOT dead.** The probe run used Vulkan only. The /D3D
renderer (option 3) points the display at the DirectDraw driver table 0x1014b838 and starts the driver at 640 x 480.
With the stub answering "not available", no mode worked and the sim stopped with SystemError 0x50 ("Error profiling
video modes", Diego's option 3 run). Its stub is removed: the original DirectDraw driver runs again, and only the
DISPDIB removal stays. The DirectDraw functions below are reachable with option 3. They are still original code.

Functions only these drivers use (call graph from build/ghidra/HGSIM.c, data references checked in the DLL):

- Still original, now never reached: `0x1007fe6f` (DirectDraw start), `0x1007ea30` (DISPDIB start), `0x1007f35c`,
  `0x1007fa74`, `0x10080372`, `0x100805d6`, `0x10080c57`, `0x10081286`, `0x100815e8`, `0x100819e2`, `0x10081a7a`,
  `0x10081e1d`, `0x100821b1`, `0x10082790`.
- Converted earlier, now never reached (harmless): `0x1003772b`, `0x1003775f`, `0x1007eda5`, `0x1007ee9a`,
  `0x1007ef27`, `0x1007efb4`, `0x1007f0b4`, `0x1007f0c8`, `0x1007f224`, `0x1007f950`, `0x1007f9da`, `0x10080471`,
  `0x100819af`.
- `0x1008181e` (DirectDraw fill) is NOT in the list: SimMain and the mission start-up (0x10084feb) call it directly.

Also unreferenced anywhere in the DLL (no call, no address in code or data), so dropped: `0x1002a8a4`, `0x1002a8b0`,
`0x10081416`.

## Damage log (2026-10-27)

The shipped DLL's debug print 0x10083495 is empty. `src/hooks/hooks_damagelog.c` replaces it with a function that
writes only the damage routine's own message ("ARMOR:%f INTERNAL:%f POWER:%d DAMAGE:%ld TYPE:%d GP_ID:%d", 0x10145344,
sent by Sim_1006c161 after every hit) to debug.log as `damage: ...`. Observation only; the conv hook for
Sim_10083495 (conv_w2i.inc) is commented out because this one takes its place.

## Round 14: overflows and the model loader, settled by the shipped data (2026-10-27)

These functions were left original for a stack overflow or an unset local (and their callers for calling them). A
scan of every entry of HEAVYG.PRJ and the loose game files (2026-10-27) shows what the real data can reach. The
archive names are at most 8 characters and the cache lookup 0x100baf3f compares 8; no simtext.txt / lang.txt /
temp.txt ships, so every sim message is the English default string in the DLL. The specs keep their string inputs
within these shipped lengths (the proven range); where it applies, a negative check (`W14_NEG=1`) shows that the
checker sees the difference beyond it. These entries replace the earlier "left original" lines of the same
functions. Specs: `tools/verify/spec_w14_<address>.py`; results: `tools/round14/progress.txt`.

- `0x1004954e` (command dispatcher, converted): five messages; "Waiting for players...Function deactivated." (43 chars, network game only) writes 44 bytes into the 40-byte [ebp-0x40] and so overwrites the local [ebp-0x18] with "ed.\0", but that path returns right after showing the text and [ebp-0x18] is never read there. A C copy with an ordinary buffer (64 bytes) gives the identical result; the spec includes that path (12000 trials, 1402/1402; negative check with 44-character messages on the other paths: 272 of 300 mismatch).
- `0x10062b7c` (HUD weapon label, converted): the HUD labels are pointers into the fixed DLL table at 0x101362e0 (16-byte slots, 4 sets of 0x77 entries, set by 0x1006a12d / 0x10062780), longest 8 characters. Fits; the round 10 C now uses ordinary 16-byte buffers (6000 trials, 388/388).
- `0x10070eb5` (gear update, converted): the only message is "MASC malfunction." (17 chars). Fits the original's 40-byte buffer, which the C keeps (15000 trials, 2036/2036).
- `0x100b48de` (scene object model loader, converted): the same object names (copied only after they load as WTB) and SDF sub-model names, longest 9, into a 16-byte buffer. Fits (8000 trials, 829/829).
- `0x100db78b` (world OBJ chunk object, converted): the names it strcpy's into the 12-byte [ebp-0x98] are OBJ chunk names at +0x3a (10-byte field) of all 716 BWD world files (12,015 OBJ chunks, longest 9 characters) and SDF sub-model names (+0xc + n*0x20, longest 9). Fits (8000 trials, 706/767: the 61 unrun instructions are dead, [ebp-0x74] is 1 before every test of it).
- `0x1001a706` (path-follow task, converted as an interpreted function, see above): its sscanf("%[^,],%[^,],%[^,]") reads the task records of type 5 in the BWD world files. All 8 of them (in the 716 BWD files) have three non-empty words of at most 8 characters ("oneshot,rotate,landing", "oneshot,norotate,pseg2".."pseg7", "pengine"), so the three 64-byte buffers are always filled and never overflow. (Checked in this round: the earlier entry above did not settle it.)
- `0x1005bb7a` (POF model loader, converted): the design record is a GDF resource (0x100599a0 / 0x10059a34). All 41 gear designs (kind 2) in the archive have exactly one type-2 (chassis) part with sub-kind 0..15, so the class [ebp-0x14] and [ebp-0xc0] / [ebp-0xa8] are always written; [ebp-0x84] is written by every case 8..0x10 and read only inside those cases. The C uses 0 as the start value of those locals, which only matters for a design without a chassis (none ships). (The design scan is the coordinator's; not re-derived here.) 10000 trials, 1101/1103; negative check without a chassis and with non-zero leftovers: 300 of 300 mismatch. The C calls the debug print 0x10083495 at its address with the original's arguments (the damage log hooks it there).
- `0x1005cd4a` (gear record from a design, converted): only called the model loader; no reason of its own (10000 trials, 1122/1124).
- `0x1009d200` (spawn at a start point, converted): besides the loader it reads the start point x / z / heading [ebp-0x54] / [ebp-0x10] / [ebp-0x24] unset when its loop is skipped, which needs a start-zone count <= 0. The zone loader 0x100daea4 makes the count (chunk size - 0x18) / 0xc unsigned and below 9, and a count of 0 divides by zero in Sim_1009c929 first, in both versions. So it cannot happen; the C starts them at 0 (10000 trials, 488/488).
- `0x1003f3f8` (network unit update) stays original: its sprintf of "'%s' %s '%s'" (two group names at +0x124 and "destroyed by") into the 80-byte [ebp-0x80] is not bounded by the shipped data. The names are network player names in a 44-byte field whose writer was not found; two unterminated names and a two-byte word +0x150 give 110 bytes, reaching the group pointer [ebp-0x14] that is read afterwards.
- `0x10048f02` (mission-end keys) stays original: in a network game with no regenerations left (key 0x6b) it sprintf's the 77-character default "Out of regenerations: Press SPACEBAR to advance viewpoint, or CTRL-Q to exit." ([0x101c47f0]) into the 32-byte [ebp-0x20], overwriting the saved ebp and the return address: the original crashes on return there, which C cannot reproduce.
- `0x10040af8` (network regeneration) stays original: for the player's own unit with a regeneration pending ([0x1015ecd0] != 1, [0x1015f200] set) it stores 0 to [ebp-4] + 0x164, but [ebp-4] is set only on the other paths (a write through an unset local pointer).
- `0x100dcc12` (world file chunk reader) stays original for now: not attempted in this round (4019 instructions, 17 KB, 289 calls). A first scan found no Windows call; its name copies into [ebp-0x7e0] come from the 8-character [ebp-0x5ec].

## Round 15: world-file reader 0x100dcc12 (2026-10-28)

`0x100dcc12` (the BWD world-file chunk reader, 72 chunk tags) is converted (`src/sim/unsorted_100d/w15_world.c`,
hooked in `src/hooks/conv_w15.inc`) with two start values, in the round 12 manner. Evidence: the instruction listing
`buildlogs/round15/dis_100dcc12.txt` (`tools/round15/dis.py`), a scan of all 716 BWD files (705 in HEAVYG.PRJ, 11
loose in the game folder; `tools/round15/bwdscan.py`, `tools/round15/hgscan.py`, output in `buildlogs/round15/`), and
a watcher run in the game. Spec: `tools/verify/spec_w15_100dcc12.py`; result: `tools/round15/progress.txt`.

- **Start value 1: the HGBT / HGXP colour is 0.** These cases build an OBJ record on the stack ([ebp-0x314] /
  [ebp-0x4a4], 0x80 bytes) and hand it to 0x100db78b once per listed id. They never write its dword +0x34, which
  0x100db78b reads at 0x100db846 as the object's colour: it becomes the model's word +2 (or goes to Sim_100b39c3 /
  Sim_1003aa7d). The shipped data reaches this: GEARSHT1.BWD has 17 HGBT chunks (380 objects, the weapon shots) and
  17 HGXP chunks (250 objects, explosions and muzzle flashes), and 176 of the 716 world files include it (INCL).
  Watcher (a read-only tap at the two calls, removed again; one session on Diego's machine, 2026-10-28): with
  converted functions off (dev.cmd option 9) every call saw HGBT 0x02726540 and HGXP 0x01010000; with them on
  (option 2) HGBT 0 and HGXP 1. So the value is whatever an earlier call left on the stack (the low word of a heap
  pointer in the original) and already differs between the two builds. The C uses 0.
- **What reads that word (hard evidence, from the converted sources).** The model's word +2 is not a colour but its
  class / flags word: bits 0xf0 the kind, 0x100 / 0x200 / 0x400 / 0x800 flags, read by the hit and collision code
  (0x100225c7 treats `flags & 0x100` as hitting a unit, 0x1002dda9 / w7l_collide, w7m_collide), targeting
  (w8z_targets), sight (w8t_sight) and the view (w3p_view). But for these objects it is overwritten before anything
  can read it: right after each 0x100db78b call the same loop fills a shot slot (table 0x1096fc80, 0x200 slots) or
  an explosion slot (0x109a5a40, 0x100 slots) and calls 0x100aa4f0(node, 0x400 / 0x20), which stores 0x400 (shots)
  or 0x20 (explosions) into the word +2 of the node's model (node +0x8c, set by 0x100a9b4c in 0x100db78b) and of its
  children. Between the two, 0x100db78b only calls node and bounds helpers that do not read the word (0x100a9830,
  0x100a9b4c, 0x100aa0da, 0x100a9d47, 0x100aa742, 0x10089e91, 0x1002d890). This holds for the shipped data: all 176
  INCLs of GEARSHT1 are outside any BLK block and REPR list (so 0x100db78b takes the model path, not the scene-entry
  or deferred-unit path that keeps the value), no BTHG / XPLO chunk ships, and 380 / 250 objects fit the 0x200 /
  0x100 slots. So for shots and explosions the leftover value is never used for drawing or game logic; the start
  value cannot be seen in play.
- **Start value 2: the GP gear pointer is 0.** In the GP case, when Sim_100b36d8 finds no object, the original
  stores 0 to [ebp-0xcc] + 0x48 (0x100ddd17), but [ebp-0xcc] is set only further down the case (by an earlier GP
  chunk of the same call, if any). The C starts it as 0 (null) and then stores nothing. No GP chunk ships (0 in 716
  files).
- **Buffers (safe).** HGBT / HGXP names (+0x23c / +0x23e) are strcpy'd to the record +0x3a (room for 69
  characters; 0x100db78b itself was proven for names up to 9): longest shipped 8, largest id count 40 (HGBT) / 20
  (HGXP). The PITF / VPTF file names (+0xa) go into the 16-byte [ebp-0x10c] / [ebp-0x14c] + 2 (room for 13; the C
  has 64 bytes): none ship, the spec uses 1..13. The GPS names are strncpy'd with a length of 8 and terminated, and
  [ebp-0x7e0] (256 bytes) gets the 8-character [ebp-0x5ec]. No Windows call.
- **Proof.** 8000 trials, 0 mismatches, coverage 4006/4019 (763 both-fault, mostly trials with a missing gear, unit
  or gun part, where both versions read address 0; half the trials are "safe" ones without those: `W15_SAFE=1`
  gives 4000 trials, 0 mismatches, 106 both-fault). The 13 unrun instructions: the GP "kind not in the gear table"
  report (dead: the kind is range-checked to 0..10 first and the table 0x10135300 holds 0..10), the GPS "design not
  found" report (both versions then read the design at address 0), and the GPS "no current gear" report (dead: the
  gear set-up writes through that pointer first). Synthetic world files with every tag, real recursion for PITF /
  VPTF, the iterator 0x100279f1 and the string / field helpers run for real, every other callee a logging stub
  (0x100db78b logs the record it gets). The spec gives the original's stack record +0x34 the start value 0 and
  points [ebp-0xcc] into the checker's (uncompared) stack. Negative checks: `W15_NEG=1` (a non-zero leftover colour
  in the original's record) 59 of 300 mismatch; `W15_NEG=2` (PITF / VPTF names of 14..20 characters, past the
  original's 16-byte buffer) 16 of 300 mismatch.

## Round 16: the within-range test 0x100687a3, the Behemoth collision bug (2026-10-28)

`0x100687a3` (is a point within a radius of a box-collision unit's box; called by the unit-against-unit collision
0x100688a3 / 0x100689c0) is converted (`src/sim/unsorted_1006/w16_range.c`, hooked in `src/hooks/conv_w16.inc`)
without its first three tests. Before the range test the original compares three locals it never writes, [ebp-0x18],
[ebp-0x1c] and [ebp-4], with 0.0 and returns 0 (no contact) if any is 0.0 or NaN. The values it sees are what the
caller's previous call left on the stack: 0x10068d76 (rotate a point), whose helpers 0x10068bb2 / 0x10068c48 /
0x10068ce0 leave their float results exactly there.

- **The bug.** 0x10068d76 was converted in an earlier round. The C leaves different bytes in that stack area, and
  sometimes 0, so the original 0x100687a3 then skipped the range test: contact with a box-collision unit (the
  Behemoth, V03BEHEM) was missed, the player's gear sank into it, and the collision damage hit it every frame. Diego's
  watcher runs of M09_SCN1 (`dev.cmd` options D and E, 2026-10-28, same machine): with the converted code (D) the
  local [ebp-0x1c] was 0 in 80 of 577 calls (the others 5 / 4 / 3, [ebp-0x18] 0x00483222, [ebp-4] 0x0019f544), the
  Behemoth (GP_ID 5) took 68 hits from armour 53 to internal 0 and died in one ramming, and the collision kill credit
  0x10067ec6 ran 66 times; with the original code (E) the three locals were never 0 in 390 calls (leftover floats such
  as 0x447a0000 = 1000.0 and 0xc2524a26 = -52.6, [ebp-4] 0x0019f53c), the Behemoth took 3 hits (armour 53 to 43) and
  the player took the damage, and 0x10067ec6 ran 7 times.
- **The fix.** The C always runs the range test, which is what the original does whenever the leftovers are not 0.0
  (every call seen with the original code). Its callers reach it through its address, so this one hook fixes the
  build while 0x100688a3 / 0x100689c0 / 0x10068d76 and the rest of the chain stay as they are.
- **Proof.** `tools/verify/spec_w16_100687a3.py`: 20000 trials exact, coverage 80/80, Sim_10068716 for real, the
  original's three locals given non-zero, non-NaN leftovers (the observed 0x447a0000 / 0xc2524a26 / 0x0019f53c /
  0x00483222 / 5, and random ones). Negative check (`W16_NEG=1`, leftovers 0.0 or NaN): 358 of 500 mismatch.
- The collision kill credit 0x10067ec6 (its "killer" is a never-written local, garbage in both runs) and the rest of
  the collision chain are unchanged.

- **The rest of the collision chain (same day, "do 1").** Converted and hooked in `conv_w16.inc`, all in
  `src/sim/unsorted_1006/w16_range.c` except 0x100688a3 (round 10 C in `w10_misc.c`, now hooked):
  - `0x100688a3` (point against a box unit, the other unit's box): round 10 C, re-proven 8000 trials exact, 114/114.
  - `0x100689c0` (the same with the roles swapped, the moving unit's box): 8000 trials exact, 132/132.
  - `0x10066907` (unit-against-unit collision): 12000 trials exact, 723/732; the 9 unrun instructions restore the
    position for a box unit inside the branch that only runs for a unit without a box (dead).
  - `0x100663c0` (a unit's move with its collision tests): 15000 trials exact, 400/400.
  The specs (`tools/verify/spec_w16_<address>.py`) run the converted helpers for real and stub the tests each function
  makes (0x100687a3 / the box tests / 0x10066907 / 0x1006754e / 0x1002e362), with effects for what the stubs can
  change (hit, position, push direction, a shortened segment). Negative checks (one-off mutations of the C, then
  reverted): 0x100663c0 using the unrounded dot product in the second component, 74 of 3000 mismatch; 0x10066907
  testing `>` instead of `>=` for the radius, 1 of 3000 (it differs only on exact equality).
  Signalling NaNs are not modelled: the original loads some floats through the x87 (which quiets them on the real
  FPU), the emulator keeps them, and the C copies bits; such values do not occur in play.
- **Leftovers of the original callers (the Behemoth lesson).** All callees of these four are converted, so no
  original function reads stack they leave. The remaining original caller 0x1006e22c later calls the collision damage
  0x1006788c / 0x100680dd, which call the kill credit 0x10067ec6 with its never-written "killer" [ebp-4]. In both, the
  distance 0x10084a94 (3 arguments, so the same frame position as 0x10067ec6) always runs first
  (`tools/round16/reach.py`: no path to the 0x10067ec6 call avoids it), and every watched run shows a float there
  (D 0x4021f82c.., E 0x3ff15798 / 0x3e3fffa0 / 0, the new D 0x401fc0bf..), so its value comes from that call, not
  from the movement code converted here. 0x10067ec6, 0x1006788c, 0x100680dd and 0x1006e22c stay original for now.

- **0x1006e22c (the per-frame movement update), converted with two start values** (`src/sim/unsorted_1006/w16_move.c`,
  hooked in `conv_w16.inc`):
  - **The drive speed [ebp-0x2c] is 0 when not written.** The original writes it only on the drive branch (0x1006e77b:
    a unit on the ground, jump-jet key flag (+0x54 -> +0x1e) and jets-burning flag (+0x518) clear), but the side-step
    thrust reads it at 0x1006ebc2 / 0x1006ebf5 / 0x1006eddc / 0x1006ee11, which a unit on the ground reaches with the
    jump key pressed (or the jets still burning) and no jet charges left (+0x514 0), while side-stepping
    (`tools/round16/reach_addr.py`: those reads are reachable without the write). Evidence: an uninittrace watcher
    run of M09_SCN1 by Diego (2026-10-28, a gear without jump jets, J while side-stepping): with the original code
    (option E) the unwritten path ran 50 times, at 0x1006ebc2 / 0x1006ebf5, and the leftover was 0 every time; the
    other two reads ran 1525 times, never unwritten; three converted runs (option D) never reached it.
  - **The second collision test's hit object [ebp-0x238] is 0 when not written.** After a contact the original calls
    Sim_100663c0 again for the slide and compares the hit object it returns (0x1006f1e9) with the first one, putting
    the position back to the unit's own when they differ. Sim_100663c0 leaves that output unwritten when the slide is
    tiny (all three components below [0x1012b5f8] = 1e-7) and its contact counter is non-zero, and then returns
    non-zero. In that case the first test's position already equals the unit's own on every axis whose coordinate is
    at least 1 in size (two different floats there differ by at least 2^-23 > 1e-7), so the comparison can only make
    a difference below 1e-7, on an axis within 1 unit of the world origin. Not watched.
  - **Proof.** `tools/verify/spec_w16_1006e22c.py`: 12000 trials exact, coverage 3021/3031 (the 10 unrun instructions
    are dead: the random pick in the stun chain always finds its index, `dt <= 0` cannot follow `dt >= 0x2d`, and a
    `== 1` test is followed by `== 3` on the same value). The original's two locals get the start value 0 before the
    call. The distance, segment, node-read and C library calls run for real; the timers, ground height (with the
    ground normal it leaves in [0x1015dd4c..54]), eye height, angle and heading helpers are float stubs; every other
    callee a logging stub (the collision tests with the hit, position and contact counter they can write, the fall
    damage 0x1006c161 logging its record). Negative check (`W16_NEG=1`, a non-zero leftover drive speed in the
    original): 49 of 400 mismatch.
  - **Leftovers of its still-original callees.** The collision damage 0x1006788c / 0x100680dd only read the kill
    credit's unset killer, which the distance call fills first (above). The fall-damage record [ebp-0x1d8] it hands
    0x1006c161 has only +0x70 / +0x74 / +0x78 / +0x7c / +0x86 set, and with the flag 0 that is all 0x1006c161 reads
    (the sparks, which read +0x18.., need the flag). The C library calls (fabs, sqrt, fmod, 0x100fcc74) read no
    unset stack.

## Round 17: the kill credit's unset killer reproduced, the collision damage 0x1006788c (2026-10-28)

- **Who writes the killer.** 0x10067ec6 (kill credit) never writes its killer local [ebp-4] when it has a second
  unit b and neither or both units are kind 9. Its two callers: 0x100680dd always passes b = 0 (killer -2), so only
  0x1006788c reaches the unset path. There the distance estimate 0x10084a94 runs on every path before the kill credit
  (unconditionally, right after the damage-switch test), both take 3 arguments from the same stack pointer, so its
  [ebp-4] is the kill credit's [ebp-4]; the only calls in between are the draws 0x1009c929, whose frame ends 8 bytes
  above that slot. 0x10084a94 puts |z| there, then swaps it with |x| when |x| < |z| or either is NaN (0x10084b1c);
  the later sort against |y| does not touch it. So the original's killer is the bit pattern of min(|x|, |z|) of the
  speed differences a - b (+0x554 / +0x55c): |x| when |x| < |z| or either is NaN, else |z|. That fits the values
  watched with the original code (option E): 0x3ff15798 (1.885), 0x3e3fffa0 (0.1875) and 0 (a zero component).
- **Not an approximation.** The C (`src/sim/unsorted_1006/w17_damage.c`) reproduces that leftover exactly:
  Sim_1006788c computes W17_KillerLeftover(x, z) and passes it to the kill credit's core Sim_10067ec6_Credit as
  `unsetKiller`, used only on the path where the original reads the unset local. The original's quirks stay: a
  zero component (also -0.0) gives killer 0, unit 0, which Score_RecordKill then counts and which goes into the
  target's last-attacker word +0x150; any other value is a float's bits, not a unit, so it is not counted and only
  its low 16 bits are OR'd into +0x150.
- **Why today's build differed.** The distance estimate 0x10084a94 was converted in round 3, so the still-original
  0x1006788c called the C Sim_10084a94, which leaves its own stack, not min(|x|, |z|), in that slot (the D runs showed
  other floats). With 0x1006788c and the kill credit converted, the value no longer depends on any stack.
- **Proof, 0x10067ec6** (`tools/verify/spec_w17_10067ec6.py`): the original gets a leftover in its [ebp-4] (random,
  the watched values, 0..7, -2), the C core the same value through a trampoline; 0x1006863d / 0x1006c161 logging
  stubs (every killer, part and the shot record compared), 0x1008486e a float stub. 20000 trials exact, coverage
  188/188. Negative check (`W17_NEG=1`, the C given the leftover + 1): 189 of 400 mismatch. The hooked 3-argument
  entry Sim_10067ec6 (for the still-original 0x100680dd, b = 0) passes 0, which is never read there.
- **Proof, 0x1006788c** (`tools/verify/spec_w17_1006788c.py`): the ORIGINAL 0x1006788c runs with the original
  0x10084a94 and the original 0x10067ec6 inside it, against the C; the stack below the call is filled with random
  bytes first, so a killer not written by the distance call would show. The speed differences include exact zeros,
  -0.0, |x| == |z|, NaNs and infinities. 0x1009c929, 0x1006863d and 0x1006c161 are logging stubs (0x1006c161's
  killer, record, flag, part and the shot fields +0x70 / +0x74 / +0x78 / +0x7c / +0x86; it sometimes adds 2 to
  +0x70 as the real one does), 0x1008486e a float stub. 20000 trials exact, coverage 438/438, and 187/188 inside the
  kill credit (0x10067f28 is the b = 0 path, which 0x1006788c cannot take). Negative check (`W17_NEG=1`, the C kill
  credit handed max(|x|, |z|) instead): 153 of 400 mismatch. The killer reaching 0x1006c161 is compared on every
  hit, and through it everything 0x1006c161 / 0x1006abf6 do with it (Score_RecordKill, the +0x150 word).
- **Leftovers (the Behemoth lesson).** Every callee of both is converted (0x10084a94, 0x1009c929, 0x1006863d,
  0x1008486e, 0x1006c161), and both callers (0x1006e22c, 0x1004170e) are C, so no original code reads stack these
  two leave. The shot record of 0x1006788c has +0x70 / +0x74 / +0x78 / +0x7c / +0x86 set, all 0x1006c161 reads with
  flag 0; the C zeroes the rest.
- **0x100680dd stays original: a leftover round 16 missed.** Round 16 said the collision damage only reads the
  kill credit's killer; that holds for 0x1006788c but not 0x100680dd. Its shot record [ebp-0x170] gets +0x70 /
  +0x74 / +0x78 / +0x86 but not the kind word +0x7c ([ebp-0xf4]), and 0x1006c161 reads that word (kinds 3 / 4 halve
  the armour in the damage, and 1..4 pick how the damage is split between armour and structure). Its value is what
  the caller 0x1006e22c's earlier calls left at that depth; 0x1006e22c is C since round 16, so in today's build it
  is already the C stack's leftover, not the original's. Watched now: uninittrace entry for 0x100680dd [ebp-0xf4]
  (16 bits) and the read site 0x100685be (the kill-credit call), for a run with the original code (option E) to
  find the value before 0x100680dd is converted with it.
- **Who leaves 0x100680dd's kind word (traced, not watched yet).** `tools/round17/footprint.py` (which stack bytes a
  function and everything it calls can write) on the original 0x1006e22c: before the 0x100680dd call, the ground
  height 0x1002d961 (with both of its callbacks) cannot reach the slot. The last calls that can are the second
  (slide) 0x100663c0 call, which always runs on that path, and, only for the own unit's first crash sound,
  0x1000ebfb / 0x10061f8e. Inside 0x100663c0 the last write depends on how the slide ended. On a slide that reaches
  the push-out, it is the saved ebp that the distance call 0x10084a94 pushes, or the segment pointer 0x1009cd36 pushes;
  both are stack addresses. A slide stopped by a ground object leaves the unit record's address (the argument pushed
  for 0x1006754e). A slide with no world hit leaves the flags word (+2) of the last world object 0x1002e362 looked at.
  So the original's kind is the low 16 bits of one of these, not a fixed value. 0x1006c161 only distinguishes 1 / 2
  / 3 / 4 from anything else. With any other value, a hit that gets through the armour takes nothing off armour or
  structure while armour is left, and still takes it off the structure once the armour is gone. Of the values
  above, stack and record addresses are 4-aligned and far from 1..4; only an object flags word could be that small.
  Today's build (0x1006e22c in C) leaves the C stack there instead, which would differ only if its low 16 bits are
  1..4. The watcher (option E for the original, D for today's build) checks both.
- **0x100680dd's kind word settled for every path but one.** Finished trace of the second (slide) 0x100663c0 call,
  which always runs before 0x100680dd. Unit hit: not this path (it goes to 0x1006788c). Ground-object hit: the unit
  record pointer it pushed for 0x1006754e. Tiny slide: that pointer, or the low dword of a float widened to a double
  inside fabs, whose low 16 bits are 0. Otherwise 0x1009cab0 always clears the segment's +0x34, so 0x1009cd36 pushes
  the segment's stack address there. Then 0x1002e362 overwrites it with the flags word +2 of the last cell object it
  walks, if there is one. On a hit, the saved ebp of 0x10084a94 (or of strncmp on the TL1M path) comes last, again
  a stack address. The pointers are 4-aligned, and the record pointer comes from an allocation (no fixed array
  of that stride), so none can be 1..4. The objects' flags words, as data (`tools/round17/objflags.py`):
  all 12,015 OBJ chunks of the 716 world files give only 0, 0x10, 0xb0, 0xd0, 0xe0 or 0x800. Scene entries get the
  same word (and `|= 0x200`). As code: every caller of 0x100aa4f0 / 0x10009408 passes 0x20, 0x50, 0x60, 0x70, 0x1a0,
  0x400, 0x800, 0xff9f, 0, a scene entry's word or a value or'ed with 0x100; 0x100aa5e8 only clears 0x300; the
  direct word stores of 1 at +2 are cache nodes, not models. So on all these paths the original's kind is never
  1..4, and 0x1006c161 behaves as "other". The exception: for the own unit's crash with the crash sound (once, while
  [0x10179cb4] is clear), 0x1000ebfb and the crash effect 0x10061f8e run after the slide and can leave a float's
  bits there (rotation math down to 0x10060681 / 0x10060b76). Those are 1..4 with odds of about 4 in 65536, and
  the value cannot be reproduced. Converting 0x100680dd with an explicit "other" kind (0) is therefore identical to
  the original on every path except that one, where it differs only in that unreproducible 1-in-16000 case.
- **0x100680dd converted with the kind 0** (`Sim_100680dd`, hooked in `conv_w17.inc`). Proof
  (`tools/verify/spec_w17_100680dd.py`): the ORIGINAL runs with the original 0x10084a94 and 0x10067ec6 inside it, the
  stack below the call random, and the never-set kind word given one of the traced leftovers (stack and record
  addresses, the world objects' flags words 0 / 0x10 / 0xb0 / 0xd0 / 0xe0 / 0x800, the runtime words 0x20 / 0x50 /
  0x60 / 0x70 / 0x1a0 / 0x400 / 0xff9f / 0x1xx, the watched 0xc300, random aligned values). 0x1006c161 (logging
  stub) logs the shot fields with the kind as its class (1..4 itself, else "other"); 0x1009c929, 0x10024616,
  0x1002597b and 0x10025aee are logging stubs. 20000 trials exact, coverage 390/390. Negative check (`W17_NEG=1`, the
  original's kind 1..4): 244 of 400 mismatch. All callees are converted and the caller 0x1006e22c is C, so no
  original code reads stack it leaves. **The one difference:** after the own unit's crash sound
  (0x1000ebfb / 0x10061f8e run between the slide test and 0x100680dd), the original's kind is a float's low 16 bits,
  1..4 about 4 times in 65536 (1 in 16000); there the original would split the damage by that kind, the C does not.
- **Damage log: part destructions.** The hit that destroys a part returns from 0x1006c161 before its print, and
  other callers destroy parts without one. Sim_1006abf6 now calls an observation-only pointer first
  (`g_simDestroyObserver`, null unless `hooks_damagelog.c` sets it; `g_simDestroyShot` is the destroying shot, set by
  Sim_1006c161 around that one call). Game logic is unchanged: the 0x1006c161, 0x1006abf6 (w8w set) and round 17
  specs pass as before. Line: `damage: DESTROYED GP_ID:%d TYPE:%d BY:%d POWER:%d` (unit id, part, killer as passed:
  -2 none / -1 network, the shot's power or -1 for a destruction that is not a hit). Only with converted functions
  on (the observer lives in the C 0x1006abf6).

## Round 18: the unset-stack readers, audit first (2026-10-28)

- **Audit** (`buildlogs/round18/audit.md`): for each still-original function that reads an unset local, who leaves
  the value and whether today's build feeds it a different one. Every caller of the eight round-18 targets
  (0x1000aa9e, 0x10020055, 0x10021e8c, 0x100232d7, 0x1005ab44, 0x1007581a, 0x100a65e7, 0x100b8d6c) is already C, so
  on any unset path that runs, today's build already hands them a C-stack leftover. Possible behaviour differences:
  0x1005ab44 (the parent key of POF sub-units; see below) and 0x1000aa9e (AI aim angles from unset dy / dz; M06 only).
  Different but not seen in play: 0x10021e8c (camera target), 0x100232d7 (near-miss counter, ground impact point),
  0x100b8d6c (matters only if the leftover word is -1, which would HeapFree archive data). No difference: 0x10020055
  (dead code), 0x100a65e7 (the leftover is always overwritten before use), 0x10089066 / 0x1008950e (identical E and
  D values).
- **The watchers changed what they watched (tap taint).** Since round 10 every read-site tap ran its register saves
  and its C handler on the game's stack below the tapped instruction, the region that becomes the next calls' frames
  (the entry watchers always used a private stack). Verdicts on earlier conclusions:
  - 0x1006e22c drive speed [ebp-0x2c] = 0 (round 16): the frame loop 0x10084feb calls the gear dispatcher 0x10029fba,
    which calls each gear's handler back to back, so the player (gear 0) sees what ran before the loop. The input
    tap at 0x100556a0 (every frame, shortly before the loop) does store the stack-pointer image of its pushad exactly
    there, but all 50 unset reads saw 0, so a later writer stored 0 in those frames: the 0x10048f02 subtree
    (0x1001fee5's [ebp-8], 0 or 1, lands on that slot), which runs without taps too; the calls after it (0x100cc8c5,
    0x1003e291, 0x1002a086) have no code that writes the slot. The 0 is very likely genuine and the start value
    stands; which 0x10048f02 path writes 0 rather than 1 depends on the data, so the next clean E run re-checks it.
  - 0x100dcc12 (round 15): unaffected; the decision rests on the code proof that 0x100aa4f0 overwrites the word first.
  - 0x100687a3 (round 16) and the kill credit (round 17): unaffected; their leftovers are traced in the code to the
    call right before (the rotation helpers, 0x10084a94), with no tapped function in between.
  - 0x1005ab44: its E values were tainted by its own read-site taps, which wrote the next sibling call's
    [ebp-0x10].
  Fix: every read-site watcher now runs on the private stack too (`MakeSiteStub` in `src/hooks/hooks_uninit.c`);
  `tools/verify/uninit_native.c` passes. Observation only.
- **0x1005ab44 parent keys watched.** On its no-name path the walk passes its unset [ebp-0x10] to the sibling call as
  the parent key; 0x1005aa45 looks it up (Sim_100b3839(Sim_100db770(key))) for the parent of the sub-unit
  Sim_1003aa7d makes. A private-stack watcher at 0x1005acbf counts every key and whether it names an entry, so one
  clean E and one D run show whether any parent differs. Held back until then.
- **0x10020055 converted** (`src/sim/unsorted_1002/w18_misc.c`, `conv_w18.inc`): nothing in HGSIM.DLL calls it or
  holds its address or its thunk's (0x10001b68), so the never-written slot index can never be seen; the C takes it as
  negative and stores nothing. Proof (`tools/verify/spec_w18_10020055.py`): the original with a negative leftover
  index, 3000 trials exact, coverage 22/26 (the 4 unrun are the store); negative check (`W18_NEG=1`, an index of 0..63)
  300 of 300 mismatch.
- **0x100a65e7 converted** (`src/sim/render/w18_poly.c`, `conv_w18.inc`; the model polygon clipped and handed to the
  renderer, 23 million calls in an M09 run). Its never-written [ebp-0x78] is copied into the edge pointer [ebp-0x2c]
  at 0x100a695f, before the table pointer is chosen; that copy happens once, and every path overwrites [ebp-0x2c]
  before reading it (0x100a6a26 when there is a table; the no-table path never reads it), so the C has no such
  copy. Proof (`tools/verify/spec_w18_100a65e7.py`): random leftovers in the original's [ebp-0x78], every callee a
  logging stub (Sim_100a5c10, Sim_100a5d25, Render_ClipEdgeNear, Sim_100a6009, Sim_100a8943, and the renderer
  callbacks [0x1014cb44] (which appends vertices with random depths to 0x101c76a0 and sometimes clears
  [0x101cc2a0]) and [0x1014cb48]); 20000 trials exact, coverage 698/698. Negative check (`W18_NEG=1`, the original's
  overwrite at 0x100a6a26 patched to NOPs, so the leftover becomes the first clipped edge's pointer): 40 of 400
  mismatch. Callee leftovers: all callees are converted except the renderer's vertex callback 0x100a62c4 (behind
  [0x1014cb44]), which writes every local before reading it.
- **0x1005ab44 converted** (`src/sim/unsorted_1005/w18_walk.c`, `conv_w18.inc`; the walk of a gear design's node tree
  that makes the sub-units). Clean watcher runs (2026-10-28, M09, E and D): 1665 sub-units each, every parent key
  the same in both and found. Shipped data (`tools/round18/gdfscan.py`, all 78 GDF designs: 76 in HEAVYG.PRJ, the
  loose ones): the walk is modelled from the GDF layout (0x438-byte items, +0x54 next, +0x78 sub-tree, +0x7c six
  names) and matches the watcher exactly (M09: 4187 calls, 1665 named, 403 nameless, with the player's design
  counted three times); 257 walks visit 497 nameless nodes with a sub-tree, and no named node is ever reached with
  the never-written parent. So that value never reaches Sim_1005aa45 with the shipped data, and the C passes -1 (no
  parent) there. A design made with the game's own editor (the loose USERGEAR.GDF is one, already in the scan) keeps
  its chassis' tree. Proof (`tools/verify/spec_w18_1005ab44.py`): the shipped trees and random shape-free trees,
  random stack below the call; Sim_1005aa45 (name, index, parent, the low words of a4 / a5, the transform, field) and
  Sim_1010c9f0 stubbed, Crt_StrUpper / Crt_StrStr for real; 4000 trials exact, coverage 209/209. Negative check
  (`W18_NEG=1`, random trees with that shape): 300 of 300 mismatch. Callees and the caller 0x1005a91a are all C.
- **0x1000aa9e stays original (watched, not reproducible).** It is the AI's line-of-fire check (called per unit from
  0x100c4d63 -> 0x100c4b67, all C): the original stores all three differences to [ebp-0x60], so Sim_100cbc6b gets
  the z difference as dx and its never-written [ebp-0x64] / [ebp-4] as dy / dz; the yaw (atan2 of z difference and
  that dz) decides the facing test and the range the length of the ray, so they decide whether a unit has a clear
  shot. A clean E run of M06 (2026-10-28, 58 calls) shows different plausible floats every call (dy about +70..+87,
  dz about -11..-137); the slots lie below the frames of the C callers, and in the original they are filled by
  whatever ran at that depth earlier in the frame (of the callers' own calls only 0x100c4a1f's subtree can reach one
  of them, on some paths), so there is no single value or chain to reproduce. The converted build already hands it
  other leftovers (the old D run of M06: dy a code address read as a float, about 0, dz about 16900), so which
  shots the AI judges clear may already differ from the original there. Held back for a decision.

## Round 19: the node-tree walks 0x10089066 / 0x1008950e (2026-10-28)

- **The two walks** (`src/sim/unsorted_1008/w19_bounds.c`, hooked in `conv_w19.inc`) are the same code with the flag
  bit 0x1000 / 0x800 and the outputs +0x70.. / +0x54..: the centre of an object's box over its sub-tree. A leaf (no
  node or no children) answers whether its never-written minimum x [ebp-4] is not the marker 0x7f7fc99e, and a parent
  leaves out every child that answers 0, so the leftover can change a model's box.
- **Who writes it.** The walk only calls itself, always from the same stack pointer, so every call at depth d has the
  same frame and [ebp-4] is one slot per depth. Only non-leaf calls at that depth write it (the marker or the
  node's +0x44, then the running minimum). So within one walk a leaf sees the last non-leaf result at its depth
  (deterministic). Two other sources exist: (b) for 0x1008950e, the slot that the 0x10089066 walk run just before it
  (both from 0x100897c7, same stack pointer) left, partly overwritten by 0x10088e37 (C since round 8) in between;
  (c) the first value at a depth with no non-leaf call yet: whatever the callers' earlier calls left (0x100897c7,
  0x1003ad51, 0x100b469d, all C; 29 functions store the marker constant in their own locals).
- **Play evidence** (a classifying watcher in `src/hooks/hooks_uninit.c`, observation only; clean E run of M06,
  2026-10-28): 0x10089066 156 leaf reads: 58 from (a), all matching the model, 0 from (b), 98 from (c); 0x1008950e
  108: 22 from (a), 5 from (b) (changed in between), 81 from (c). No leaf read saw the marker, from any source. (The
  earlier "14 marker reads" of the generic watcher were non-leaf calls that write the marker themselves while the
  leftover was also the marker.)
- **Conversion.** The C keeps the slot per depth for one walk, exactly as the original within the walk, and counts a
  depth without a non-leaf call yet as "not the marker". This matches every read seen in play. **Deviation:** where
  (b) or (c) would give the marker, the original would leave that leaf out of its parent's box and the C does not;
  in today's build those values already came from C frames.
- **Proof** (`tools/verify/spec_w19_walks.py`): the original's recursion runs for real on random object trees (with
  flagged objects whose box stays empty, so later leaves at that depth see the marker), random stack below the call;
  10000 trials each exact, coverage 227/227 each. Mutation (the C leaf always answering 1): 40 and 34 of 1000
  mismatch, so the in-walk marker case is exercised. Negative check (`W19_NEG=1`, the marker planted in the
  original's slot of every depth before the call): 244 and 241 of 400 mismatch. The walks call nothing but
  themselves, and the caller 0x100897c7 is C.

## Round 20: the "not attempted" group (2026-10-29)

- **0x10040af8 converted** (`src/sim/unsorted_1004/w20_net.c`, `conv_w20.inc`; the network "RG" regeneration
  message, reached only from the network message dispatch in `w9b_net.c`). In its two own-unit branches (not the
  host, the message about the own unit or the own unit + 8) the original clears +0x164 of its gear pointer [ebp-4]
  before it ever sets it. The caller is C, and in the original the slot holds whatever the network dispatch's earlier
  calls left there; no single writer. As in the round 15 GP case the C stores nothing there. Network-only path.
  Proof (`tools/verify/spec_w20_10040af8.py`): every callee a logging stub (all converted), Mem_Move and sprintf for
  real, the original's leftover pointing into the uncompared stack; 8000 trials exact, coverage 238/238 (223
  both-fault: a null unit in the table, where both read address 0). Negative check (`W20_NEG=1`, the leftover
  pointing at a compared unit record): 45 of 300 mismatch.
- **0x100a0def (view clear and sky) not converted yet.** It calls the clip-vertex builder 0x100a0880 (still
  original), which hands the renderer stack vertices with fields it never sets; those leftovers come from
  0x100a0def's earlier calls at the same depth, so converting 0x100a0def alone would change them (drawing only).
  To be done together with 0x100a0880.
- **The other 12 stay original:**
  - The DirectDraw display driver: 0x100805d6, 0x10080c57, 0x10081286, 0x10081416, 0x10081a7a, 0x10081e1d,
    0x100821b1 (the driver table 0x1014a144..0x1014a1f8 and its set-up 0x1007fe6f). Recorded here as unreachable
    from a Vulkan run; corrected in round 21: the /D3D renderer (option 3) starts this driver, so they run there,
    as original code.
  - Unreachable: 0x1003cfd6, a screen callback for the renderer slot [0x1014cb3c] installed only by 0x1003d863,
    which nothing in the DLL calls or points to.
  - Reached, platform code: 0x1008181e (SimMain's DirectDraw clear; it returns at once without a DirectDraw surface
    [0x1017b3ac]) - replaced by the Vulkan clear; 0x100b82a0 (start-up: the multimedia timer timeSetEvent /
    timeBeginPeriod and the launch path) - the SDL3 / platform timer replaces it.
  - Graphics library: 0x100ed36f, 0x100ed388 (helpers of 0x100ed491) - go with that library.

## Round 21: SimMain (2026-10-29)

- **0x10084feb SimMain converted** (`src/sim/unsorted_1008/w21_simmain.c`, `w21.h`, `conv_w21.inc`): the exported
  entry point the launcher calls (heaps, command line, display mode, about sixty subsystem starts, the frame loop
  inside its `__try` / `__finally`, shutdown, heaps freed). Every callee is still called at its original address (15
  of the 151 are still original code); every Windows call goes through the new platform header
  `src/platform/sim_platform.h` (17 calls, the same ones in the same order), whose Windows backend
  `src/sim/platform/sim_platform_iat.c` calls through HGSIM.DLL's own import slots.
- **Frame layout:** the file is compiled without optimisation (`#pragma optimize("", off)`). Each call then pushes
  its arguments and pops them right after, as the original does, so every callee's frame starts at the same offset
  from SimMain's stack pointer as before. The leftovers that still-original callees read from earlier sibling calls
  therefore keep their layout (the round 16 Behemoth lesson). SimMain's own frame size differs, which moves only
  what lies below it from before SimMain ran: launcher stack, C since M2, never the original's.
- **`__try` / `__finally`:** MSVC's own `__try` / `__finally` with `AbnormalTermination()`, as in the original
  (its scope table 0x1012b8c0 holds the single `__finally` at 0x1008663f): the error box (when [0x1015dca0] is set)
  shows only when an exception unwinds through SimMain.
- **Deviation: the display driver's start record.** The 0x50-byte record at [ebp-0x88] (built only at 640 x 480
  with /3DFX or /D3D, and passed to [0x109e3d28]) has 5 to 9 dwords the original never sets. The C zeroes them. In
  the original they held whatever the launcher's calls had left on the stack (C since M2), so they were never the
  original's values anyway, and the record only reaches the old original-renderer path (renderer option 3), never
  Vulkan.
- **Proof** (`tools/verify/spec_w21_simmain.py`): the full call log. All 149 callees are logging stubs (the two of
  the `__finally` run for real), every Windows call is logged through the import stubs, and the five function
  pointers it calls ([0x1014cb3c], [0x109e3d28] with the record logged whole, [0x109e3d34], the driver's +0x14 and
  +0x20) are logging stubs too. Stub effects end the loops (frame loop, driver-less loop, ShowCursor,
  PeekMessage). The original's record leftovers are zeroed. Result: 4000 trials exact, 0 both-fault, coverage
  1355/1395. Not run: the null-record system error (the checker's fs stands at page 0), the two dead re-tests of
  [0x1017b940] in the frame loop, and the error box and late HeapDestroy, which run only after an exception (the
  gcc checker build has no `__try`). Negative check (`W21_NEG=1`, random leftovers left in the original's record):
  30 of 300 mismatch, which is every trial that builds the record.
- **Deviation: the objective-line wrap (0x100ccc60 F12 objectives list, w9c; 0x100cd70f current objective, w9g).**
  Diego, option D (M09, converted): on F12 the first objective showed twice, the second line one letter short;
  option E (original) wrapped it cleanly. Both functions wrap a long objective by copying cut + 1 bytes (0x20 + 1
  for the list, 19 or 20 + 1 by resolution for the current objective, table 0x10151d60) into a 0x100-byte stack
  buffer with no terminating zero, then search the buffer for its last space. The search runs past the copy into the
  stack's leftovers, up to the first zero. With no space there, the cut lands on the objective's own last space within
  the copy (M09: "Locate Paratroops Near Way" / "Yankee", what E shows). With a space there, the cut lands past the
  objective: line 1 is the whole text and line 2 is read from the entry memory after it. The C was faithful; only
  the leftovers differ, because the converted functions' frames are not the original's. For the first objective of
  a call those leftovers come from whatever ran at that depth before (the draw calls, the HUD text hooks): no
  single writer, so they cannot be reproduced. The original wraps correctly only when they hold no space. The C
  now zeroes the buffer on entry. That removes only the outside leftovers: when one call draws several objectives,
  the buffer still holds the previous objective's text, as in the original, so that deterministic leftover is
  kept. (Terminating the copy instead failed the checker on exactly that case.)
  Proof (`tools/verify/spec_w21_wrap_ccc60.py`, `spec_w21_wrap_cd70f.py`): the round 9c / 9g generators, with the
  stack below the caller filled with random bytes that hold no space and end in a zero. 4000 trials exact each
  (0x100cd70f: 91 both-fault, as in its own spec). The original specs still pass (2000 and 1500 trials).
  Negative check (`W21W_NEG=1`, spaces allowed in the filler): 39 and 48 of 300 mismatch.
  How widespread (`buildlogs/round21/objective_lengths.txt`, all 175 shipped `*SCN*.BWD`, 349 objective texts):
  the F12 list wraps only 2 (M03 "Remain at Tango Until Transport Arrives", 39 characters; M09, 33). The
  current-objective line wraps 190 of 349 at width 19 and 110 at width 20. There a stray space shows the whole
  objective on one line, with no second line.
- **The DirectDraw driver restored (option 3).** Diego's option 3 (/D3D) run stopped with SystemError 0x50 ("Error
  profiling video modes") right after debug.log's "removed DirectDraw driver asked for (640x480)". It was not a
  fault, and not round 21: the zeroed record is only built after a successful mode set. The cause was the
  2026-10-27 stub of the DirectDraw start 0x1007fe6f. It is removed from `src/hooks/hooks_dispprobe.c`; the DISPDIB
  stub stays (that section and round 20 are corrected).
- **The /D3D sim runs in a separate process (launcher only).** After the DirectDraw fix, an option 3 mission that
  ends with code 0 (finished, to the debrief) left the menu black with only sound. Ending with code 255 (quit) was
  fine. The original code does the same: dev.cmd H (option 3 with noconv) on M02 had code 255, then the menu came
  back; then code 0, then a black menu. So it is not a conversion. The sim's DirectDraw exclusive mode in the menu's
  own process breaks the menu screen on modern Windows. The launcher already ran the Vulkan sim in a child process
  (RunSimIsolated), and the menu comes back fine there. `ChooseRenderer` now sets `g_isolateSim` for d3d too, and
  RunSimIsolated logs "Sim started in a separate process". No sim code is changed.
  **Correction: this did NOT fix the black menu.** The isolated run (child process, hgrc.cfg "d3d", 2309 of 2311)
  ended with code 0 and the menu was black again. The change stays: it is harmless and matches Vulkan.
- **Parked (Diego, 2026-10-29): the black menu after an option 3 mission.** Every /D3D mission that ends with code 0
  is followed by a black menu with sound only, with original and converted code alike, and in the menu's process or
  isolated. A quit (code 255) brings the menu back, and Vulkan is fine. Likely cause (unproven): the shell's own
  DirectDraw screen after the sim's 640 x 480 exclusive mode switch, while the next briefing's SMK video
  (Project\SMK, e.g. B02*SMK for M02) plays its audio. A possible test: option 3 with the SDL menu screen
  (`d3d sdldisplay`).
  Also seen: M02 /D3D ended at its first frame with code 0 (no frames counted, 0 damage hits) in both runs after the
  first M02 run, once with noconv and once converted and isolated. The first M02 run played normally (654 hits,
  quit 255). Not investigated.

## Round 22: the renderer group (2026-10-29)

- **Watchers first (Diego).** `tools/round22/unsetscan.py` lists, for the 12 functions, the stack reads that can come
  before any write on some path, and the fields of stack records handed to a callee or the renderer that the
  function never writes (`buildlogs/round22/unset_static.txt`). Private-stack watchers in `uninit_table.inc` log
  the real leftovers at those points (0x1009ef50 `[ebp-4]` and the renderer record `[ebp-0x3c]`; 0x100a0def's eight
  vertex locals `[ebp-0x90]..[ebp-0x44]`; 0x1009fada `[ebp-0x120c]` and the record `[ebp-0x1210]`; the callee
  records of 0x1009353e and 0x100a0880). The limit went from 5 to 8 locals per function. False alarm dropped:
  0x100a0def `[ebp-0x2c]` / `[ebp-0x28]`, filled first by a 5-dword `rep movsd` from 0x109e3e00. Never called in
  any Vulkan run: 0x100918e6, 0x1009221d.
- **The graphics library calls.** Five of the twelve call the still-original graphics library: 0x1009353e,
  0x100945b8 and 0x100a539e call 0x100e84f0; 0x100a539e also calls 0x100e86a1; 0x1009ef50 calls 0x100e4780 and
  0x100e5477; 0x100a0def calls 0x100e5477. All four library functions are plain cdecl: arguments on the stack
  (Ghidra: `(int *, int, int, byte)` for 0x100e84f0, `(int *, int, int, uint *)` for 0x100e5477), a plain `ret`,
  and every caller pops the arguments with `add esp` and does not use eax or any other register afterwards. No
  register inputs or outputs, no flags. So the five are converted, and they call the library at its original
  address through SIM_FN: one place to redirect when the library is replaced.
- **Converted** (`src/sim/render/w22_items.c`, `w22_poly.c`, `w22.h`, `conv_w22.inc`; specs `tools/verify/spec_w22_*.py`,
  progress in `tools/round22/progress.txt`): 0x100ad96e (draw-item dispatcher), 0x100a539e (queued polygon to the
  display), 0x1009fada (spark ring), 0x1009ef50 (the display driver's polygon routine, slot [0x1014cb4c]), 0x100ac9ab
  (item star), 0x100a0880 + 0x100a0def (sky band and view background, together), 0x100945b8 / 0x1009353e /
  0x1009596a (particle sparks), 0x100918e6 / 0x1009221d (beam; one C body, the two differ only in instruction order).
  All 4000-6000 trials exact; coverage complete except dead jumps (the 0x1000-multiple kind switches) and 0x100a0def's
  u reset, which the shipped constants make unreachable (the spec varies [0x1012ba50] to cover it). Negative checks
  catch every planted leftover (22..300 of 300 trials, the lower counts where only some paths log the record).
- **Deviation: z of the sprite / particle quads.** 0x1009353e, 0x100945b8, 0x1009596a (and the never-called
  0x100918e6 / 0x1009221d) build four plugin vertices on the stack and never write z (+8). 0x1009ef50 passes them on to
  the renderer, and both HGVK (depth order: 1 < z < 1e9 is a depth, anything else takes the polygon's nearest) and
  DX5DRAW (z times a scale: no division) read z. What the original hands over (Diego's E run, M09, watchers on all 5
  call sites, all unset in every call):

  | vertex | 0x1009353e (10.8k quads) | 0x100945b8 (18k) | 0x1009596a (296k) |
  |---|---|---|---|
  | v0 | 0 (99.8%) | 0 (all) | 0 (99%) |
  | v1 | 0x10002856 (64%), 3 (29%), rare floats like 9557.6 | the same pattern | 0x10002856 (95%), 3, heap pointers |
  | v2 | 0x10006738 (94%), 0 | 0x10006738, 0 | 0x10006738 (68%), 0x0e4aaee8 (27%), others |
  | v3 | 0x101c0040 (92%), ...0040 | the same pattern | 0x101c0040 (67%), 0x101c9f50, ... |

  These are stack leftovers of the dispatcher's earlier calls: a thunk address (0x10002856), a return address inside
  0x10006609 (0x10006738), data pointers. No single writer, and that code is now partly C, so they cannot be
  reproduced. As floats nearly all are below 1 or above 1e9, "no depth" for HGVK and depth 0 for DX5DRAW; a rare
  minority are in-range stale depths of earlier frames, which the original then used. The C writes z = 0 (Diego /
  coordinator choice (a)): what the original effectively does in the overwhelming majority of calls; only those rare
  garbage depths are dropped.
- **Other never-written fields, all unread and so 0 in the C:**
  - 0x1009ef50: the record handed to the renderer as its texture argument ({0, colour or 0}): HGVK reads that
    argument only in textured modes; DX5DRAW's mode jump table sends modes 13 and 16 to code that never reads it
    (0x100043a5, 0x10004741; only modes 0..12 read [esp+0x20]). The fourth of the seven arguments it pushes to
    0x10006609 is its never-written local [ebp-4] (E: 1 / 0 / 2 in 12M calls): 0 here; 0x10006609 reads three.
  - 0x1009fada: the texture record {0, 0x40} (0x40 in all 6427 watched calls; its writer is an earlier call that
    value watchers cannot name; written explicitly) and the vertex alpha bytes: mode 0x10, unread as above.
  - Vertex alpha bytes (0x1009fada, 0x100a0def's haze): DX5DRAW turns alpha blending off in modes 2, 13 and 16
    (state index 5 = D3D render state 0x1b ALPHABLENDENABLE set to 0); HGVK reads alpha only in blended modes.
  - The 0x58-byte clip vertices of 0x100a0880 / 0x100ac9ab (+0..+8, +0x31..+0x57): 0x100a5c10 copies them only into
    projected +0xc..+0x14 / +0x30..+0x38, which 0x100a62c4, 0x100a0339, 0x100ac738 and the edge clippers never read.
  - 0x100a0def's software haze vertices +0xc..+0x14 (the library 0x100e5477, software path only).
- **The graphics library** (0x100e84f0, 0x100e86a1, 0x100e4780, 0x100e5477) stays original, called at its original
  address (plain cdecl, above). The unset-read scan finds one possible unset read in it (0x100e86a1 [ebp-0x34] at
  0x100e908b), in the software line drawer that runs only without a renderer plugin.
- **Behemoth check.** Every callee of the twelve is converted except the graphics library (software path only) and
  the renderer plugins. The still-original unset-stack readers left in the DLL are 0x10021e8c and 0x100232d7 (camera),
  0x1000aa9e (hit lookup) and 0x100b8d6c (resource name lookup), outside the render path; Diego's D (converted) and
  E (original) watcher runs compare their leftovers.
- **Deliberate fix of an original bug: a weapon on a destroyed part kept firing (Diego, 2026-10-29).** In D (M09)
  the Behemoth destroyed part 10 of the player's gear ("DESTROYED GP_ID:0 TYPE:10 BY:5 POWER:750"): the gun fell
  (0x1006ad98 hides the part's model) but kept firing from its mount. The original does the same, for every gear
  built by the design loader 0x1005bb7a (the player's and the other new-format designs):
  - that loader gives each part its damage entry by kind (entry kind - 1, so the destroyed part code is the kind),
    but gives the weapon slot on it the mount code 0x1005b923(kind): 7 (kind 8), 3 (9, 10), 4 (11, 12), 2 (13..16);
  - 0x1006abf6 passes the destroyed part code (9, 10, 11, 13..16) to 0x1006bf33, which switches off the first slot
    whose +0x30 equals that code. No slot of such a design ever has one, so its weapon is never switched off, and
    the fire routine 0x100d4590 keeps firing it (state +8 still 1).
  - The old-format loader 0x1005cd4a sets +0x30 from its own table, and those designs keep the original rule.
  All these functions are converted and checker-proven, so the C had reproduced the bug exactly.
  The fix uses the loader's own data: Sim_1005bb7a records, for each weapon slot it fills, the part it sits on (its
  kind), in a table kept on the C side only (`src/sim/unsorted_1006/w22_mounts.c`); Sim_1005cd4a drops the record of
  a slot it fills. Sim_1006bf33 does its original search first, unchanged; only when that finds no slot does it
  switch off the recorded slot of the destroyed part (while the slot still holds the recorded weapon, +0x34, and is
  in use, +4 >= 0), with exactly the original's state change (+0 = 0, +8 = -1, +0x114 = 1, +0x20 = -2) and cockpit
  message for the player. debug.log gets "damage: WEAPON OFF GP_ID:x SLOT:y PART:z" for each such switch-off.
  It applies to every gear, the AI's included: enemies now also stop firing a weapon whose part is destroyed.
  Kinds 8 and 12 stay as in the original (0x1006abf6 never passes codes 8 and 12 to 0x1006bf33).
  Proof: the original-vs-C specs of every changed function still pass with the table empty (spec_w9c 0x1006bf33,
  spec_w8w 0x1006abf6, spec_w10_1006b52f, spec_w14_1005cd4a; spec_w14_1005bb7a 10000 trials with the C-side table
  left out of the memory comparison, new difftest option `ignore_syms`). The new behaviour is proven by
  `tools/verify/test_w22_weaponoff.py` (the C alone in the checker's emulator): a recorded slot is switched off and
  nothing else changes; another part, a changed weapon, a forgotten record or a disabled slot change nothing; a
  slot matching the original rule is handled as before.

## Round 23: the functions Ghidra's index missed (2026-10-29)

Eight functions reached only through pointers (tools/round22/missed_functions.txt), hooked in
`src/hooks/conv_w23.inc` (8 entries; expected 2329 of 2331 hooks):

| Function | C | What it is | Checker |
|---|---|---|---|
| 0x1009b772 | `src/sim/render/w23_scene.c` | the scene pass's polygon routine (driver slot [0x1014cb4c]) | 4000 exact, coverage 72/72 |
| 0x1001efbe | `src/sim/unsorted_1001/w23_audio.c` | set an audio option (0..4) | 3000 exact, 95/95 |
| 0x1001f14f | `src/sim/unsorted_1001/w23_audio.c` | read an audio option (0..4) | 3000 exact, 64/64 |
| 0x100d422f | `src/sim/render/w23_screens.c` | a cockpit weapon-slot panel image | 4000 exact, 95/95 |
| 0x1007760b | `src/sim/unsorted_1007/w23_menu.c` | a menu choice item | 6000 exact, 255/255 |
| 0x1007792c | `src/sim/unsorted_1007/w23_menu.c` | a menu text item | 4000 exact, 91/91 |
| 0x100ce586 | `src/sim/render/w23_screens.c` | the sliding side panel: the current objective (mode 1) or a "name: number" unit list | 4000 exact, 247/247 |
| 0x100cde94 | `src/sim/render/w23_screens.c` | the F12 objectives screen: title, objective lines, mission status line (page 1) or the unit list | 4000 exact, 446/446 |

Specs: `tools/verify/spec_w23_*.py`. Every spec has a mutation (`W23_NEG=1`; macros W23S_, W23A_, W23H_,
W23M_, W23T_MUTATE, W23H_MUTATE2, W23F_MUTATE) the checker catches (10/300, 26/300, 30/300, 18/300, 2/300, 9/300, 43/300, 11/300).
- Graphics-library calls (the resource loader / release 0x100ba7d5 / 0x100bb2b0, font height 0x100ecce0, shape
  drawers, the three text routines, the DLL's sprintf 0x100fc780) are plain cdecl and called through SIM_FN.
- The DLL's sprintf faults in the checker's emulator (as its sscanf does), so the specs of 0x100ce586 and 0x100cde94
  stub it with an effect that formats the line itself ("%s: %i", "%s%s"); the C calls the same DLL sprintf.
- 0x100ce586 reads the player's gear block (unit +0x28) and never uses the value; the C keeps the read (volatile) so a
  missing player unit faults exactly as in the original.
- **Leftover: 0x100cde94, status byte 1 or above 4.** The jump table sends status 1 (and the default) straight to the
  draw of the status line, which the original never wrote on that path. Who leaves that text:
  - The line is a stack buffer ([ebp-0x104]), not a global: it holds whatever the routines that last ran at that
    depth left there. This screen is panel 13's draw routine, called from the panel loop of 0x1002c1f9; before it run
    panels 1, 11 and 12 (0x100782a9, 0x100c7deb, 0x100c7120, frames 0x90..0xcc, so their callees reach the buffer's
    bytes too), 0x1002c76d, and the previous frame's panels 14..29 and 0x100622f0. All of these are C now, so no
    original frame lays those bytes down any more: the original's content cannot be recovered in this build.
  - The path is never taken in play: the status byte is in .bss (zero at load) and the DLL writes it only with 0, 2, 3
    or 4 (byte stores at 0x10071299, 0x1007be26, 0x1007bea7, 0x1007c115, 0x1007c5ac, 0x1007c5dc, 0x1007c623,
    0x1007cb97; a scan of every instruction near the block's address arithmetic finds no other write or block copy).
  So the C sets the line to empty at entry (fixed value, invisible: statuses 1 and above 4 do not occur). In the
  checker the stack starts zeroed, so the original draws an empty line there too and the 4000 trials (statuses 0..5
  and 200 included) match.
- Behemoth check: no function leaves state that still-original code reads, beyond what the checker compares
  (all memory writes, the stub calls and their text).

## SDL3 sound and Vulkan by default (2026-10-29)

Diego: "For sound I want to use sdl and for rendering I want to use vulkan."
- **SDL3 sound is the default.** `HooksAudio_SdlSound()` (hooks_audio.c) decides it once from hgrc.cfg and
  hgrc_extra.cfg, and hooks_audio.c and hooks_conv.c both use it, so a run never has half of each: SDL3 sound unless
  the word `noconv` (an original-reference run) or `dsound` is present; `sdlaudio` still forces it. So every
  converted-code dev.cmd entry (1, 2, D, F, option 3 ...) runs the 10 SndSdl_* functions, and the noconv entries
  (E, G, H, 9) keep the original DirectSound as true original references. dev.cmd is unchanged.
- **Vulkan (HGVK) is the target renderer.** Diego, correction: "no, we still need to compare to d3d until we are
  done." Option 3 (/D3D, DX5DRAW) and HGSIM's DirectDraw display driver (14 functions, 0x1007f35c..0x10082790) stay
  fully working as the comparison reference until the whole port is finished. They are not translated: they are
  DirectX code. Option 3 runs converted code, so it has SDL3 sound too; the comparison there is about rendering.
- **The menus' sound (HGSHELL.DLL) is still DirectSound**: six thiscall methods of one sound object (0x1002f91d create,
  0x1002fa6a release, 0x10031fb1 / 0x10032228 play, 0x100321b1 / 0x100323fa stop and release; docs/audio.md). Moving
  them needs a thiscall hook path for HGSHELL (nothing in HGSHELL is converted yet), the six rewritten with audio.h
  as HGSIM's ten were, and checker support for HGSHELL. The menu videos' sound is Smacker's own DirectSound
  (SmackSoundUseDirectSound(0)) and would need its own replacement. Not done yet.

## Round 24: the leftover readers (2026-10-30)

Diego: "go" on the plan for the five still-original functions that read stack memory they never wrote (the round
23 analysis): convert 0x100b8d6c with a fixed value, scan the menu data for 0x1007581a, and watch who leaves the
values 0x1000aa9e reads.

- **0x100b8d6c converted** (`src/sim/unsorted_100b/w24_resource.c`, `w24.h`, `conv_w24.inc`): load a resource from
  the cache (0x100ba7d5), else as a loose file (Sim_100474db path, 0x10046ac4, _close), with an optional copy
  (0x100b659a, memcpy, release 0x100bb2b0 / 0x100b9203); it stores a word to key[0]. Two unset reads, fixed values:
  - key[0] on the cache path is the never-written [ebp-0x10] (the loose-file path writes -1). The C writes 0. The
    callers (0x100462c0, 0x100463bf, 0x100466b6, 0x100468c3, 0x100b5502, 0x100b581d, all C) only test key[0] == -1
    (HeapFree, else the cache release 0x100bb2b0), so every leftover other than 0xffff behaves exactly as 0. A
    leftover 0xffff (a loose-file load just before at the same depth) would HeapFree cache memory: an original bug,
    not reproduced.
  - The loose file's name: the original appends the extension (strncat, 4 characters) to the 8 never-written bytes
    [ebp-0x24] (a NUL is written only at [ebp-0x1c]), so the name is stack leftover + ".mgi" / ".3di" / ".hdi" /
    ".cpi". The C uses the extension alone. Invisible: no file with any of these extensions exists in the game
    folder or in HEAVYG.PRJ's loose files, so the open fails whatever the name (data 0, key[0] = -1) in both.
  Proof: `tools/verify/spec_w24_100b8d6c.py`, 4000 trials exact, coverage 99/99 (the checker's stack starts zeroed,
  so the original reads 0 and an empty name there); negative check `W24_NEG=1` (key[0] 0 on the loose path) caught.
  The function never ran in any watched run (uninittrace, original and converted).
- **0x1007581a converted** (`src/sim/unsorted_1007/w24_menu.c`, `w24.h`, `conv_w24.inc`): the in-mission menu draw
  (MAIN MENU on Esc, MISSION CHANNEL on F1, SQUAD MATE 1..6 CHANNEL on F2..F7, SYSTEMS STATUS; GAMEKEY.MAP). Three
  unset reads:
  - The label wrap. Correction to the first scan: 0x100ffdb0 is strrchr (the last space), not strchr. A label of
    width + 2 or more characters leaves the strncpy(width + 2) copy without a NUL, and strrchr then reads on into
    what the earlier labels of the same call (or, for the first one, the stack) left in [ebp-0x104]. Width
    [0x10149950 + 4 * [0x101c64f0]] is 25 (320 x 200) or 27 (640 x 480; 1024 x 768 needs a hand-edited HGSND.CFG, as
    above). No shipped label is that long: the 29 fixed labels of the 14 menu records are at most 22 characters, and
    the three MISSION CHANNEL labels are "NO MESSAGE" or the name of a group with code 0x8000..0x8002 (Sim_10010183 /
    0x10010267 / 0x1001034b): 33 such names in HEAVYG.PRJ, longest 25 ("Stiletto, please respond."). The C keeps the
    line within the call, as the original does, and starts it zeroed (fixed value, unreachable).
  - The title position [ebp-0x144] / [ebp-0x140], unset without a title: every record has one (above). The C uses 0.
  - The key code [ebp-0x128] passed to Sim_10075753 on a menu's first frame (record state 0) whose selection starts
    on a text line (kind 3) while the key [0x1015dc18] is a digit: only the ABORT MISSION confirmation (0x101444e8,
    "Confirmation requested" then "Accept"), and only with number keys on two consecutive frames as it opens. Diego's
    run (entry 1 with hgrc_extra.cfg "noconv lastwriter", 2026-10-30): 0 such reads, 5 key-code stores. It would
    matter only if the value were Esc (0x1b closes the menu; up / down / tab only move the highlight back to
    "Accept"). The C carries the code the menu draw itself last stored there (g_w24MenuKey: the key, or 0xd for a
    number key that picked an item), the most likely writer (the previous frame's call at the same depth); 0 before
    any store.
  Proof: `tools/verify/spec_w24_1007581a.py`, 6000 trials exact, coverage 809/809: the same random key code in the
  original's [ebp-0x128] and in g_w24MenuKey, labels longer than the width included; negative check `W24_NEG=1`
  (the carried code dropped) caught. Behemoth check: its callers (Sim_10075667) and callees are C or the graphics
  library; the still-original unset readers left are the shot functions 0x10021e8c / 0x100232d7, outside the menus.
- **0x1000aa9e: last-writer watcher** (`src/launcher/lastwriter.c`, diagnostic only). In a watcher run (hgrc.cfg
  "uninittrace") started with a mission, the launcher runs the sim in a child copy of itself (HG_CHILD=1,
  _NO_DEBUG_HEAP=1) as its debugger: DR2 an execute breakpoint at the read 0x1000ab09, DR0 / DR1 write watches on
  [ebp-4] (dz) and [ebp-0x64] (dy), re-armed at each read. Each write records the instruction after it, the frame's
  return address and the value; each read checks its value against the last recorded write. Nothing runs in the game
  process, so nothing is written to the game's stack (an exception handler there would have the system push its
  records onto that stack). Output: lastwriter.log.
- **0x1000aa9e converted, its two leftovers reproduced exactly** (`src/sim/unsorted_1000/w24_sight.c`, `w24_aim.c`,
  `w24.h`, `conv_w24.inc`).
  - Who leaves them (lastwriter.log, Diego's G run of M06, original code, 2026-10-30): 223 reads, 223 of 223 match
    for both slots, all for the same unit, one writer each, no other write in between, no read without a prior
    store. [ebp-4] (dz) is 0x100cada2's y difference [ebp-0x3c] (all four branches store it there); [ebp-0x64] (dy)
    is the first angle 0x100cbc6b stores at its [ebp-0x10] when 0x100cada2 calls it (0x100cb185). That 0x100cada2
    call is Sim_100cb4fb's (case 0x200, the unit's own target check), at the same stack depth as the order step
    that calls 0x1000aa9e (Sim_100be0f0 steps 16 / 17 -> Sim_100c4d63 -> 0x100c4b67). The first watched run (data
    watches, M09) showed the same two writers 6 of 6. An earlier run with 0 reads ran fine but never reached the
    check (the AI uses it only in order steps 16 / 17 within 15000 units of the player).
  - In C: Sim_100cada2 keeps the y difference and that angle of every call that gets as far as Sim_100cbc6b
    (g_w24TargetDy / g_w24TargetAngle, flag g_w24TargetSet cleared at its entry, as the original leaves its slots
    untouched when it returns earlier); Sim_100cb4fb copies them to g_w24AimDz / g_w24AimDy after its call when the
    flag is set; Sim_1000aa9e passes g_w24AimDy / g_w24AimDz to Sim_100cbc6b where the original passes its two
    slots. Before any target check both are 0 (fixed value; never observed: no read came without a prior store).
  - Proof: `tools/verify/spec_w24_1000aa9e.py`, 4000 trials exact, coverage 162/162: each trial puts the same random
    float bit patterns (NaN, infinities and denormals included) into the original's two stack slots and the C's two
    values, and the logged Sim_100cbc6b arguments match. Negative check `W24_NEG=1` (the two swapped) caught. The two
    changed functions re-proven with the new C-only values left out of the memory comparison (`ignore_syms`):
    spec_w10d 0x100cada2 6000 trials, spec_w8z 0x100cb4fb 1500 trials, both exact.
  - Behemoth check: the slots are not changed for still-original code. All callers and callees of 0x1000aa9e are C;
    the still-original unset-stack readers left (0x10021e8c, 0x100232d7, 0x1007581a) are the shot loop and the menu,
    which do not read the AI chain's leftovers by an established chain (round 18 audit), and their read sites did not
    run in the watched runs.

## Round 25: game file loading (2026-10-30)

Diego: "convert them in one round": the archive and the resource / memory cache family.

- **0x100ba7d5 converted** (the resource loader; `src/sim/unsorted_100b/w8y_cache.c` next to its sibling 0x100baff4,
  `conv_w25.inc`): the heap of the type, the cache (Sim_100baf3f), else the file "name.ext<language letter>", else
  "name.ext", else the pack (Sim_100bca49) while [0x1017d5fc] is set, else the search path (Sim_100bae0a); a file is
  read whole into a block of that heap and filed with Sim_100b9b3f. The C library calls (fseek, ftell, malloc on
  [0x1017b8f8], fread, fclose, sprintf, assert) are HGSIM's own at their addresses. Like 0x100baff4, ext must be at
  most 6 characters (the original formats ext and the language letter into 8 bytes of its stack; the game's types
  are 3). Proof: spec_w8y.py g_ba7d5 (new), 4000 trials exact, coverage 414/414 (search path and allocation
  failures included); negative check `W25_NEG=1` (a pack item filed one byte longer) caught.
- **The other nine re-hooked** (`conv_w8y.inc`): their round-8 C (w8y_archive.c, w8y_cache.c, w8y_pack.c) was
  checker-proven but left unhooked for three reasons, now settled:
  - "leaves strtok's pointer in its own dead stack" (0x100b9eab, 0x100b9fa5, 0x100ba304): the DLL's strtok keeps
    its position at [0x1017e884], which these leave pointing into their frames. Every strtok(NULL) in the DLL
    (0x10019da0, 0x1002ad50, 0x10054a79, 0x100b9fa5, 0x100ba304; nine callers in all) is reachable only after a
    strtok(string) in the same call (a control-flow scan from each function's entry), which resets the position.
    The stale pointer is never read.
  - "reads unset stack in a rare error case" (0x100a79db, 0x100bc70d): the 12-byte archive header and the 0x16-byte
    entry headers are read with _read / fread into stack records; a short read would leave them unset. The shipped
    HEAVYG.PRJ is complete (all 19,545 entries tile the file, `tools/prj/prjtool.py verify`), so every read is whole.
    (0x100a79db's dword read of the word [ebp-8] is masked to 16 bits.) 0x100bc610's 16-byte name buffer gets 8
    bytes from strncpy; its only callback, Sim_100b9b3f (set by Sim_100bcbcf), reads exactly 8.
  - "calls an unhooked function above by name" (0x100a76a0, 0x100baff4, 0x100bc610, 0x100bca49): all hooked now.
  Re-proof: spec_w8y.py, all nine exact; coverage 0x100a76a0 43/43, 0x100a79db 189/189, 0x100b9eab 62/62,
  0x100baff4 191/191, 0x100bc610 82/82, 0x100bca49 95/95, 0x100bc70d 238/238 (4000 trials, allocation failures
  added), 0x100b9fa5 148/156 and 0x100ba304 191/201 (4000 trials each): their uncovered instructions are only the
  assert after a failed list insertion (0x100bd76b out of memory) and the null tests of blocks whose allocation
  already failed and asserted, unreachable in a running game; the C mirrors them.
- Behemoth check: callers are C; the only still-original unset-stack readers left are the shot functions
  0x10021e8c / 0x100232d7, outside loading. No register adapters.
- Expected hook line: 2338 of 2340 (10 more).
- **Deliberate addition: live brightness (Diego, 2026-10-30).** The Monitor Brightness slider stores the gamma row
  [0x1013b98c] (HGSND.CFG +0x44; Sim_100219c8 preview, Sim_10021a10 accept, Sim_10021a60 cancel, all through
  Sim_1004b7d2). The original then sets only the 2D palette ([0x1015e688] + 0x18); the 3D texture palette 0x10185760
  is built through the gamma row once, at mission start (Sim_100d3000), and both DX5DRAW and HGVK cache textures
  converted with it, so the 3D world needed a restart. Now Sim_1004b7d2 first rebuilds it exactly as mission start
  does (Sim_100d3000 with the new row), then makes the original's 2D palette call, so the 2D palette ends as before.
  HGVK's SetTexturePalette drops its cached textures (at the end of the frame) when the palette's contents change,
  so they are converted again: the same final colours as a restart at that setting. Only in the game (the hook layer
  sets g_simLiveBrightness); the checker keeps 0, and spec_w3t still passes for the four functions.
  Why it cannot change game logic: Sim_100d3000 writes only colour tables (the texture palette 0x10185760, the
  16-bit colour words 0x101659e0, the HUD colour words via Sim_10062386), sets the driver's palette (+0x14; the
  original +0x18 call follows and leaves the 2D palette as before) and hands the palette to the renderer. Its one
  load of the mission palette (Sim_100ba7d5) is a cache hit (loaded at mission start, never released): no
  allocation, no file read; it adds 1 to that entry's use count (+8) and renews its time stamp (+0x10). The cache only
  purges entries with a use count of 0 or less, which this entry never has after the mission-start load, so purge
  order and memory stay as before. No random number, no unit, timer or mission state is read or written.
- **Deliberate fix of an original bug: the menus lose their colours when the brightness slider moves** (Diego, the
  menus turned brown, also before the live brightness). With the renderer (Vulkan or /D3D; display table 0x1014b838)
  the 2D palette slot +0x18 is 0x1008792c, which rewrites the 2D colour table 0x10156dcc (RGB565 per palette index,
  used by the menus and the 2D layer) from the palette it is given. Sim_1004b7d2 gives it [0x109b8c00], which only the
  GDI (0x1008404b), DirectDraw (0x10081e1d) and DISPDIB (0x1007f224) drivers fill; in renderer modes it stays empty,
  so every menu colour became 0 (black in the original; transparent in our 2D layer, showing the world through). In
  the game with the renderer, Sim_1004b7d2 now runs Sim_100d3000 instead of that call: it sets the 2D colour table
  from the mission palette through the driver's +0x14 (0x100878bb), as at mission start, together with the texture
  palette. So both end exactly as after a restart at the new brightness. The use count and time stamp of the
  mission palette's cache entry are saved before and put back after (Sim_100baf3f's lookup without its updates), so
  the cache is as the original leaves it. Software mode and the checker keep the original call.

## Round 26: the sine / arctangent tables (2026-10-30)

Diego chose to embed the tables ("approach 2").
- **0x100846e0 Trig_InitTables hooked** (`src/sim/unsorted_1008/w5a_misc.c`, the table
  `w26_trig_tables.inc`, `conv_w5a.inc`). The original fills 512 floats sin(i * pi / 1024) at 0x103719a0 and 512
  floats atan(i / 512) in degrees at 0x103721c0 through the C runtime's sin 0x100ff644 / atan 0x10100044, which use the
  x87 fsin / fpatan (64-bit mantissa, stored once to float), then the words 1.0f at 0x103721a0 and 45.0f at
  0x103729c0; it returns 1 and changes nothing else (sin keeps the control word 0x27f the game sets). Its round-5 C
  computed them through doubles, a second rounding, so it was proven only within a tolerance and left unhooked.
- The values: `tools/round26/trigcap.c` maps HGSIM.DLL without running its start-up code, sets the control word
  0x27f and runs the original 0x100846e0 on the game machine's CPU (Diego's PC), dumping the 1024 floats and the two
  words as a C table (copy in `tools/round26/trig_tables_dump.inc`). x87 fsin / fpatan results can differ between
  processors; the embedded table makes them the same everywhere. On this machine all 1024 values also equal the
  double-then-float computation, and the checker's emulated original agrees with them (Unicorn's x87).
- Proof: `tools/verify/spec_w26_100846e0.py`, original (emulated) against the C, memory exact, coverage 34/34;
  negative check `W26_NEG=1` (one sine bit flipped) caught.
- Expected hook line: 2339 of 2341.

### Round 26: the beam-shot update, with a deliberate fix (2026-10-30)

Diego: "if its a bug, fix it".
- **0x10021e8c Sim_10021e8c hooked** (`src/sim/unsorted_1002/w26_beam.c`, `conv_w26.inc`): one frame of a beam shot
  (lasers, particle accelerators, two autocannons; records 0x109a3f80 + idx * 0x156): trail, hit test, unit /
  building hits, statistics counters, damage, and the hit effect Sim_100246b3.
- **The bug**: on a unit hit the original passes Sim_100246b3 three stack words as the effect's camera point
  (arguments 7..9: [ebp-0x8c], [ebp-0x94], [ebp-8], read at 0x1002258a), which the function never writes. The
  watcher (`lastwriter.c`, laser runs on option 2 with the Kodiak carrying a Light Laser Cannon) found no single
  writer: run 1 had -1, 0, 0 (23 of 23), run 2 varied (51 of 90 from the epilogue of 0x100232d7, 39 other). So the
  values cannot be reproduced; the C passes the hit point (the effect's own x, y, z) instead.
- **Why it is safe**: in Sim_100246b3 the camera point is used only for the big effects (fx +0x10 set, [0x10134894]
  set): it becomes the current camera's light position [0x1014cae0] +0x1c..+0x24 (the old one saved at 0x109f14e0..
  and put back by Sim_1002528f in `w8t_misc.c` when the effect ends) and goes on to Sim_10024616's burst, which calls
  Sim_100246b3 again with the same point. Sim_100d1f90 hands +0x1c..+0x24 to Sim_10058716 (0x109b16c4 / c8 / c0), the
  light vector of the renderer (`w3p_view.c`) and the polygon shade levels (`unsorted_1005/w10_misc.c`). The near /
  far test in Sim_100246b3 reads the previous light point and only sets the light bookkeeping (0x1015dc04,
  0x1013a018, read by the frame renderer). No random draw (Sim_1009c929) and no game state depends on it. Visible
  effect: the flash lights the scene from where the beam hit instead of from a garbage point.
- Proof: `tools/verify/spec_w26_10021e8c.py`, every callee stubbed (float stubs for Sim_1008486e and fmod), the
  original's three camera words seeded with the hit point the segment stub gives; 15000 trials exact, coverage
  488/488. Negative check `W26_NEG=1` (W26B_MUTATE: camera x and y swapped) caught. 0x100232d7 stays original.
- The watcher entry for 0x10021e8c in `uninit_table.inc` stays; the CONV hook is installed first, so it finds the
  prologue changed and is skipped.
- Expected hook line: 2340 of 2342.

### Network functions parked (2026-10-30)

Diego: "We will rewrite network code later." 0x10042125 (the dpEnumPlayers callback, stdcall), 0x10048f02 (mission-end
keys; the original crashes on the 77-character "Out of regenerations" text in a 32-byte buffer) and 0x1003f3f8
(network unit update; name lengths unproven) stay original until that rewrite. The sim calls only six ANET2 functions
(dpSend, dpReceive, dpNumPlayers, dpEnumPlayers, dpSetConnCallback, dpReportScore) through the pointer table at
0x1015ea8c; HGSHELL uses about 27 (the lobby). Listings: `buildlogs/round27/`.

## Round 27: the text-table reader and joystick removal (2026-10-30)

Diego's go, through the coordinator.
- **0x100b69d0 Sim_100b69d0 hooked** (the language-file reader, C from round 8, `w8y_lang.c`, `conv_w8y.inc`). It was
  held back because the original leaves its two text pointers [ebp-4] (english) and [ebp-0x20] (translated) unset
  before the first block and reads them when a :TRANSLATED: line comes before any :ENGLISH: line; the C starts them
  at 0, so such a block is skipped. No language file ships (Project or ProjectIso), and a real one starts with
  :ENGLISH:. Proof: `spec_w8y.py` Sim_100b69d0 now includes that case (15% of the trials start with a translated
  block) with the original's two words seeded with 0; 800 trials exact, coverage 138/138. Negative check `W27_NEG=1`
  (W27L_MUTATE: the table starts with room for 99 pairs) caught, 340 of 800. (A first planted bug, keeping the old
  translation pointer, was not caught: it is invisible, the next block always overwrites it.)
- **Joystick support removed: 0x1005621b and 0x100567cb hooked** (`src/sim/unsorted_1005/w27_joystick.c`,
  `conv_w27.inc`). They are the name (+4) and open (+8) entries of the joystick driver table 0x101426f0 (driver
  layout in `w7n.h`). Both are now the original's "no joystick" path: return 1 and change nothing, with no Windows
  call (the original asks joyGetDevCapsA / joyGetPosEx).
- **The driver's count 0x10056210 (Win_JoyGetNumDevs, round 4) now returns 0** instead of joyGetNumDevs (Diego:
  "remove joystick"; the coordinator asked for no joystick call at all). Its only caller: the table entry +0 is read
  only by Input_AddDevice 0x1005366e (`w7n_input.c`; every reference to the driver list 0x1013f150 is inside it,
  and nothing else references 0x101426f0 or 0x10056210). With count n > 0 it calls the name entry n times, which now
  always fails, so nothing is stored; the only other thing in that loop is the "Too many input devices" check
  (records >= 5). Input_GetDevice adds a record only for a device name not seen yet, and the shipped .MAP files
  (Project and ProjectIso) name five devices in all (keyboard, mouse, joystick, fltstck, msjstick; the code adds
  "keyboard"), so at most four records exist when a new name is looked up and the check never fires. Count 0 skips
  the joystick driver; the result is the same.
- Proof: `tools/verify/spec_w27_joystick.py`, the original run with Windows answering "no joystick" (errors 6,
  0xa5, 0xa7; joyGetNumDevs 0) or no state block; the joystick calls are taken out of the original's call log (the
  one deliberate difference), result and memory exact: count 300 trials, name and open 3000 each. Negative check
  `W27_NEG=1` (all three give the other answer) caught in every trial. The old Win_JoyGetNumDevs entry of
  `spec_w4w.py` moved there.
- Expected hook line: 2343 of 2345.

## Round 28: the platform functions (2026-10-30)

Diego: "apply the changes but ignore linux quirks for now, just remember that the architecture needs to support
linux later ... only fill the gaps for windows now." The sim's platform functions are converted onto a platform
layer with plain C types; the Windows backend is the only one filled in.

**Interface.** `src/platform/plat.h` (new; backend `plat_win32.c`): window show / painted / default handling /
event time / quit / cursor visibility, a millisecond clock, the current folder, and library loading (the renderer
plugins). `input.h`: the Windows message hook is replaced by `Input_SetMessageRules` (quit flag, cursor-shown flag,
plain pointers) and `Input_WaitEvent`; without SDL3 open, `Input_Pump` runs one OS message by the same rules. No
Windows type in either header; the game code (`src/sim/platform/w28_*.c`) includes no Windows header.

**Default.** The SDL3 platform (SDL3 input, the SDL3 menus' screen `sdldisplay`, and the functions of
`conv_w28.inc`) is on in converted runs (`Hooks_SdlPlatform`); off with `noconv`, `d3d` (option 3 stays the original
reference) or the new word `winplat`; `sdlinput` forces it. `sdlloop` and `sdlnomouse` are gone. `conv_w28_all.inc`
(pure game logic: SystemError, the CD search, the CPU check / cycle counter, the plugin load and choice) is
installed in every converted run; the trace taps of hooks_sim.c on 0xBC280 / 0x3650D / 0x3695B remain only for
`noconv`, otherwise the converted functions report through `g_w28ErrorTrace` / `g_w28Trace` (trace.log).

**Converted and proven** (checker: the original's Windows calls and the C's platform calls log the same events with
the same answers; exact; planted bug caught):
- 0x10057afc keyboard flush, 0x10057f83 next typed key: the SDL3 key events go to the sim's own key handling
  (Key_SetState, Input_QueueKey) as the Windows handler 0x10057b4e did (`spec_w28_input.py`, 3000 each).
- 0x1007d948 mouse release, 0x1007d9d8 mouse poll (6000; 122/136, the rest is the "cursor outside the box" branch,
  impossible inside the box, and padding), 0x1007d961 re-centre (proven through the poll). Not identical: the
  original also writes the box's screen corners to 0x10373040 for ClipCursor (read by nothing else); the checker's
  cursor model holds the cursor in 0..width-1 / 0..height-1.
- 0x100867a2 window procedure (`spec_w28_window.py`, 20000; 241/253: the key path and the two ShowCursor count
  loops). Not identical: key messages are left alone (the keys come from SDL3).
- 0x100b82a0 StartSupanim, 0x100b8834 its tick (`spec_w28_anim.py`, 8000 / 4000). **Deliberate change:** on every
  display driver but an exclusive one (so with HGVK and D3D) the original started a Windows multimedia timer
  (250 ms) whose thread drew animation frames through the display driver while the main thread loaded; here no
  thread: `W28_AnimPoll` runs the tick on the game's thread at the start of each file load (Sim_100ba7d5 calls it
  through `g_w8yLoadPoll`), at most every 250 ms. [0x1017d5cc] (the timer) stays 0; [0x10199448] = 10, the period
  the original picks on a PC. The not-run instructions of the start are the branches that test [0x1017d5dc] /
  [0x1017d5e0] right after clearing them.
- 0x100bc280 SystemError (`spec_w28_error.py`, 8000, 69/69), pure game logic, the case table read from the DLL.
- 0x1003650d LoadPlugin (8000, 180/180) and 0x1003695b PluginChoice (3000, 41/41) (`spec_w28_plugin.py`): the
  plugin model stays (HGVK.DLL, DX5DRAW.DLL for option 3), loading goes through plat.h.

**Replaced** (no game logic to keep): 0x10057b4e (Windows key handler: nothing), 0x10086b99 (message loop step:
wait while inactive, stop after quit, open SDL3 around the game window once, pump, hand on the keys).

**Deliberate changes without a checker proof:**
- 0x100b8f30 CPU check: no cpuid; the vendor words of an AMD processor (what Diego's PC answers), result 1. Its
  only use: the file cache start (0x100b9eab) takes the cycle counter for "GenuineIntel", else the millisecond
  clock, to stamp cache entries (+0x10/+0x14), which nothing reads back. 0x100b8fb1 cycle counter: Plat_TicksMs (no
  rdtsc; unreachable now).
- 0x10099510 CD drive search: the original's CD-ROM test and its search for the movie file are without effect
  (its FindClose is called on a path), so with any drive it sets the drive count [0x1017cc3c] = 1 and returns 1;
  Windows always has a drive. Here that answer without asking the OS (the cached letter [0x1017cc38] is still
  returned when set). The old C copy (w7o_misc.c, never hooked) is removed.
- 0x1008667c, the shutdown piece: the tail of the original SimMain; round 21's converted SimMain already has its
  logic (the __finally: heaps destroyed, cursor shown, 0 / 0xff), so the original fragment no longer runs.

**Still Windows-only, for later:** Sim_100bc47e (the fatal box: ShowCursor / ShowWindow / MessageBox through the
DLL's import slots), SimMain's __try / __finally (MSVC; a fallback exists), the 67 Windows calls of earlier rounds,
and the Linux backend (paths: separators, letter case, data root).

**Fix after Diego's option 1 run (crash when the mission started from the menus):** with sdldisplay the launcher
ran the sim in its own process (the separate sim process was only used to escape the 256-colour compatibility
layer). After the menus, SDL3 and the renderer, HGSIM.DLL could not get its own address 0x10000000 and was moved;
the 269 hooks whose first bytes hold an address no longer fit ("bytes differ ... skipped", 2092 of 2361) and the game
crashed after the renderer plugin loaded. Now the sim always runs in the separate process for HGVK / D3D (not in a
network game), as option 1 did before round 28, and debug.log names the address HGSIM.DLL was loaded at.

## Round 29: the kind-0 shot update 0x100232d7 (2026-10-30)

Diego: "You have almost all the code converted, surely you can deduce the missing functions." Settled from the code
and the shipped data, no play run.
- **Which shots:** a shot's kind (+0x4c) is the weapon's +0x76 (0x100d5446, 0x100d4f71), from the weapon table in
  the DLL (0x10145508, 46 weapons). Kind 3 (Sim_100225c7) is only the thrown grenades (Hand Grenade, Haywire
  Grenade, Heavy Grenade, weapons 28 / 38 / 39); every other projectile weapon (machine guns, autocannons, field
  guns, grenade launchers, mortars, rocket packs, missiles, bazookas) fires kind 0, this function. Beams use the
  20-entry table (0x10021e8c). A shot's type (+0) is the weapon's +0x10: 0x3e the incendiary rocket packs, 0x3f the
  anti-tank / anti-gear missiles.
- **Unset read 1, [ebp-0x2c] at 0x10023d35 (unreachable):** a flying rocket / missile (type 0x3e / 0x3f) checks the
  active shots of type 5; one within 500 counts a near miss ([0x101654ec]) when [ebp-0x2c], set only on the unit-hit
  path, is the player. Type 5 is never made: the code writes a shot's type only from a weapon (2, 4, 6, 56..72) or
  -1, and the world files' shot chunks (0x100dcc12: BTHG, HGBT) carry only 56..72 (HGBT in GEARSHT1.BWD; no BTHG in
  any of the 716 + ProjectIso BWD files). So the path never runs; the C uses -1 there (never the player).
- **Unset read 2, [ebp-0xdc] at 0x10023add (reachable; fixed):** a shot with no hit (no forced target, Sim_1002dda9
  finds nothing) whose new position is below height 0.0 and below the ground (Sim_1002d961) ends there: the original
  cuts the segment [ebp-0xdc], which it never builds, at the ground height (Sim_1009cf67) and takes the cut end as
  the impact point (the explosion of Sim_10024548). The branch beside it (a found hit without a hit record) cuts the
  shot's own path [ebp-0x11c]; that is the correct segment, so the C cuts the path (deliberate fix, Diego's "if it's
  a bug, fix it"). Effect: such a shot explodes where its path meets the ground instead of at a garbage point.
- Proof: `tools/verify/spec_w29_100232d7.py` (from spec_w10_100225c7.py), the original's [ebp-0xdc] seeded with the
  path segment's bytes and [ebp-0x2c] with -1: 20000 trials exact, coverage 1018/1021 (the 3 not run: the near-miss
  counter). Negative check `W29_NEG=1` (the fixed cut 1.0 higher) caught, 2461 of 20000. Hooked in `conv_w29.inc`.
- Expected hook line: 2360 of 2362 (options 1 / 2), 2350 of 2352 (option 3).

## Round 30: the HUD's 2D drawing library in C (2026-10-30)

Diego: "do 1" (convert the drawing routines so the HUD no longer needs the original DLL's code).
- **Converted** (`src/sim/render/w30_vfx.c`, hooked by `conv_w30.inc`), each from its hand-written assembly: line
  0x100ef727 (outcode clipping with the 0.32 fixed-point rounding of all 16 cases, x- / y-major, diagonal and
  straight stepping), pane wipe 0x100f0865, shape 0x100f0205 (row skipping above the pane, left / right cuts that
  switch the run-length decoder's mode as the original's), unclipped shape 0x100f0727, character 0x100f1c93,
  string 0x100f1e10, and the 8-bit set's 1-bit font character 0x100f2d3c with its string 0x100f2f01.
- **Proof:** `tools/verify/spec_w30_vfx.py`, random windows / panes / ends / shapes / fonts, the whole buffer
  compared: line 30000 trials (not run: only the mode-1 / mode-2 drawing, dead because any mode but 0 returns -3,
  and two unreachable jumps), wipe 3000, shape 20000, unclipped shape 5000, character 10000, string 5000, 1-bit
  character 10000, its string 5000; all exact; planted bugs (W30*_MUTATE) caught. The checker loads a flat 32-bit
  descriptor table for these routines (they push / pop ES; Unicorn has none by default).
- **Unset read:** the line routine returns ([ebp-0x34] >= 1) also for horizontal and vertical lines, where that flag
  is never written. No caller reads the line's result (the converted callers drop it; the string routine and the
  dead ellipse do not use it either); the C returns 0 there, and the checker seeds the original's slot with 0.
- **Not reproduced:** a font of height 0 makes the original's row counter wrap and draw until it faults (no font
  has that); the C draws nothing. The generator uses heights of at least 1.
- **0x100f2d3c is the 8-bit set's routine:** it writes one byte per cell (the colour byte [0x10157598]), not
  RGB565; converted exactly, with its quirks (clip offsets in bytes for 1-bit data, the chunk length after the
  first 8 bits not capped, the byte row counter). It gets no full-resolution copy.
- **Left out:** the ellipse 0x100f094e has no callers (dead).
- **Full resolution:** the logic of hooks_hud.c moved into the entry points (`w30_hud.c`): W30_Line / W30_Shape /
  W30_ShapeRaw / W30_Wipe / W30_Char run the converted routine on a scratch buffer when the pane draws into the
  renderer's HUD buffer and hand the drawing to HGVK.DLL's HudApi, as before; the string routine calls W30_Char, so
  text gets full resolution per character. hooks_hud.c no longer patches the DLL: it only supplies the HUD
  interface (GetModuleHandle / GetProcAddress of HGVK.DLL) and the scratch / bitmap memory through g_w30HudApi /
  g_w30Scratch / g_w30Cells. "nohudhi" and option 3 leave them unset (640x480 HUD only). No Windows call in the
  sim code.
- Expected hook line: 2368 of 2370 (options 1 / 2), 2358 of 2360 (option 3).

## Round 31: 1.2 ports start; round B, the Patch folder (2026-10-30)

Diego: "okay, i want to bring shots and weapons changes" (from version 1.2, GameRip_12, 1998-07-23). Analysis of
the shot group (1.0 vs 1.2, addresses and the 1.2 object record's -4 field shift mapped): the beam has no change;
the main shot only adds a force-feedback jolt (no effect without the force-feedback library; AFTER_DLL.md); the
grenade adds a bounce-dust call; the visible change is 1.2's new particle explosions, which need a particle system
(PartSpec.ini), 1.2's .TEX texture path and its Patch folder. Order chosen by Diego: B (Patch folder) -> T (.TEX
textures) -> C (PartSpec.ini) -> D (particle core, Vulkan transparency) -> E (explosions) -> F (other effects).
Picks: when no .TEX exists, fall back to the 1.0 .XEL (round T); the whole 1.2 patch folder is used; 1.2 data and
textures only with the Vulkan renderer, option 3 and noconv stay the 1.0 reference.

Round B:
- **Ported:** 1.2's file loader 0x100a2bc9 is 1.0's 0x100ba7d5 with two file opens put first,
  "Patch\name.ext<language letter>" and "Patch\name.ext", each read as 1.0's file of the same kind (its asserts at
  1.2's syscache.c lines 0x44f / 0x479); the rest (1.0's two loose files, the pack, the search path) is unchanged.
  `Sim_100ba7d5` does the same when `g_w8yPatch` is set.
- **Deliberate deviation:** `g_w8yPatch` is set at sim load only with the Vulkan renderer (`Hooks_VulkanRenderer`:
  HG_RENDERER=hgvk or the first word of hgrc.cfg; hooks_conv.c). 1.2 always looks in Patch; here option 3 never
  does, so it keeps the 1.0 data (Diego's pick). noconv runs the untouched original.
- **Left out:** 1.2's second Patch reader 0x100a3a92 has no callers in 1.2 (dead).
- **Proof:** `tools/verify/spec_w8y.py`, new spec `Sim_100ba7d5_patch`: the C with the Patch folder on against the
  1.0 original given the file the C finds in Patch as its own file of that kind (no Patch file / the language file /
  the plain file); difftest's new spec options `pre_c` (C-side setup) and `log_map` (the C's Patch paths and 1.2
  assert lines brought to the 1.0 form) do the mapping. 4000 trials, 0 mismatches; coverage of the C function
  177/178; planted bug (W31B_MUTATE: the language file passed over) caught (709 mismatches). The plain spec still
  passes (Patch off).
- **Data:** GameRip_12\patch copied whole into Project\Patch (1379 files; not in the repo). Its 88 BWD files use
  no chunk types 1.0 lacks; their changes are values: the mission tables name one boulder colour table per boulder
  (boulde01-36, shipped in Patch) instead of the shared "boulders", one scene's music track, the gear sheet
  GEARSHT1's HGXP / HGBT record mix, small byte edits. Nothing found that needs unported 1.2 code; values tuned
  for 1.2 code can only show in play.

Round T1 (1.2's .TEX world textures; Diego: "Everything vulkan if we can"; standing rule from 2026-10-30 20:26:
"everything to be 1.2, unless is a bug we have fixed"):
- **Texture name table moved** out of the DLL into `g_w31TexNames` (src/sim/render/w31.h, w31_tex.c) in 1.2's
  layout: 512 groups x 32 frames of 0x1c bytes (1.0: 0x18 at 0x10a51ac0; the luma table sits 0x20 bytes after
  it, so it cannot grow in place). Every user points at it: w2j (anim), w3q (registration), w2m (preload), w2h
  (reset; also clears the new +0x14 as 1.2's 0x10001ef7), w2l, w7l (release, 1.2 releases with the 1.0 type even
  for a .TEX image), w2n (getter, submitters), w7q (group names), w21 (the sim main's walk and its pointer
  [0x1015dafc], now a real pointer into our table). AFTER_DLL.md notes it.
- **.TEX mode** (`W31_TexMode`: `g_w31Tex`, set by the hook layer with the Vulkan renderer, and the 3D mode
  [0x1017b940]; 1.2 sets its flag 0x10105048 from the 3D mode): registration loads nothing (1.2 0x1000189d);
  the preload loads every image as .TEX, mirrors its rows, offers it with its own palette and the description's
  new flag +0x18, and stops 30 s after the mission load began (1.2 0x10001b4c; start time taken where 1.2's sim
  main takes it, after the heaps); the getter of the three submitters is 1.2's 0x10002391 (`W31_GetTexImage`)
  with the texture's palette (1.2 0x100010d9 / 0x100012c5 / 0x10001513), the description's flag set by the
  second and third in 3D mode.
- **Not a deviation: render mode.** 1.2's second and third submitters ask for mode 8 where 1.0 asks for 6. 1.2's
  dx5draw.dll renumbered the modes: its mode 8 sets exactly the render states 1.0's mode 6 set (texture with
  colour key, states 0x10000 / 0x40000 / 0 / 0x400000 at 1.2's 0-based state indexes 0 / 4 / 2 / 6 = 1.0's 1 / 5 /
  3 / 7), and its modes 5 and 6 draw nothing. Our renderers keep 1.0's numbering, so the C sends 6 = 1.2's look.
  Every later 1.2 port has to map 1.2's mode numbers to 1.0's.
- **Bug fix (1.2): missing textures.** 1.2's archive is 1.0's; in 3D mode 1.2 only tries .TEX, and 268 textures
  exist only as .XEL. 31 of them are named in texture / material lists, 4 in lists missions use (OCONWALL,
  OGROUNDA, OGROUNDC in mission 1, TIMSPLAN in mission 30): 1.2 loads nothing for them and the polygons are not
  drawn. Here a missing .TEX falls back to the 1.0 .XEL (game palette), recorded in the frame's +0x14 (0 = .XEL,
  1 = .TEX), which 1.2 only ever clears (Diego's pick 1a).
- **1.2's CACHE.INI** is needed: 1.0's has `tex 0 0`, a hash size of 0, and filing the first .TEX divides by zero
  (Sim_100b9b3f). Project\CACHE.INI is 1.2's (`tex 1023 4000000`); the 1.0 file kept as CACHE.INI.bak-20261030.
- **Not ported (inactive for our renderers):** 1.2's vertex-colour branch for 3DFX / PowerVR plugins ([0x1014b954]
  from the plugin's name), the "grid" command line word ([0x1014a5f0]) and the "16bit" / "8bit" words that force the
  .TEX flag. The description's +0x14 is 0 here (1.2 leaves it unset; no renderer reads it).
- **Vertex alpha (1.2):** 1.2's second submitter (0x100012c5) no longer forces the vertex alpha to 0xff (only its
  3DFX / PowerVR branch does): the callers' alpha (0x40, 0x80, the global [0x101cfc8c]) now shows, as in 1.2. With
  the Vulkan renderer (`g_w31Tex`).
- **Texture transparency (1.2, HGVK):** 56 of 1.2's .TEX files (the effect textures, but also trees OTREES01/03/04,
  shrubs, cactus, the WXFLGR set, smoke) carry a transparency in each palette colour's top byte. 1.2's dx5draw makes
  a texture offered with the description's flag +0x18 a 16-bit ARGB one: alpha 15 - (top byte >> 4), no colour
  key; it chooses the format when the texture is first offered (the preload). HGVK does the same (alpha
  (15 - top 4 bits) * 17, no colour key; made at PreloadTexture, its pixel pointer remembered so it keeps its alpha
  when made again; drawn without the colour-key discard: it blends in the blending modes, whose pipelines write no
  depth, and is solid in the others). Colours keep 8 bits (1.2's 4 bits were a limit of its texture format).
- **Render modes renumbered in 1.2:** 1.2's renderers take 21 modes, 1.0's 0..4 unchanged, 1.0's 5..18 as 7..20 (1.2's
  5 and 6 draw nothing): 1.2 code sends 8 for 1.0's 6, 0xf for 0xd, 0x12 for 0x10. Ported 1.2 code sends 1.0's
  numbers (our renderers keep them).
- **Kept from 1.0 on purpose:** .TEX only with the Vulkan renderer (Diego's pick 3a: option 3 and noconv stay the
  1.0 reference).
- **Checker:** the nine 1.0 specs of the table's users run the C on our table through a mapping step
  (tools/verify/w31_map.py: the 1.0 table into ours before the C run, back after it; pointers into it in the
  logs). Two generators now keep to the table's groups and records (outside them the original reads memory next
  to its table, other data once the table is ours), and the anim spec (0x10006a2f) no longer runs out of
  instructions in every trial (600 both-faults since round 2j: it checked nothing; now 600 real trials, exact).
- **Checker against 1.2's code:** difftest runs 1.2's DLL relocated to 0x20000000 in the same machine ('orig12',
  'to12', 'from12'); tools/verify/spec_w31_tex.py proves the .TEX paths against 1.2's functions: W31_GetTexImage
  (0x10002391) 3000 trials, registration (0x1000189d) 3000, preload (0x10001b4c) 400, the submitters (0x100010d9,
  0x100012c5, 0x10001513) 2000 each, all exact; the .XEL fallback against 1.0's getter 2000, exact; planted bug
  (W31T_MUTATE: the row mirror leaves the last row) caught (373 mismatches). The 1.0 specs still pass with the
  Vulkan switch off.
- **T2 next:** 1.2 also moved the two file textures (sky / ground items, 1.0 0x100a2a66, 0x100a0880, 0x100a0def)
  to .TEX; they keep their 1.0 .XEL until then.
- **1.2 content in Project** (Diego: "I want all the content from the patch included"; not in the repo): the Patch
  folder, CACHE.INI, INPUTW.MAP (1.2's walk / run keys) and SPLASH\STRINGS.TXT, each 1.0 file kept as
  *.bak-20261030. PARTSPEC.INI and SKYGND.PAR follow with the rounds that read them (1.2 ships two different copies
  of each); HG.IFR with a rumble round. Not copied: CHARINFO.HG1, CURRENT.CCT, HGSND.CFG (Diego's own state) and
  DLLFILE.INI (our renderer).

Round T2 (1.2's sky / cloud file textures; Diego: "yes"):
- **Ported (.TEX mode, `W31_TexMode`):** the file-texture preload (1.0 0x100a2a66 -> 1.2 0x1008fd87), the cloud layer
  (0x100a0880 -> 0x1008da61) and the sky band (0x100a0def -> 0x1008dff3) load their textures as .TEX (rows not
  mirrored, unlike the world textures) and describe them with the texture's own palette; the band's corners move to
  the far end (z 0x7f7fc99e, 1/z 1; HGVK draws z <= 1 and z >= 1e9 alike, as backdrop), its palette set only when it
  is drawn; before the clouds 1.2 sets render state 0xc (its numbering; 1.0's 0xd: 1.2's SetState codes are 1.0's
  minus 1) to 1 unless the display kind [0x1014cb1c] is 1 - its dx5draw does nothing with that state, nor do ours
  (HGVK now logs each distinct SetState once). The descriptions are ours (`g_w31FileTex`, 7 dwords, flag 0); the
  handles [0x101cfc90] / [0x101cfcb8] point at them. Correction: the cloud layer has no 1.2 geometry change (an
  unnormalised diff of its shifted stack offsets suggested one).
- **Bug fix (1.2):** its preload describes a file texture only when it loads it there and gives it the palette from a
  local set only on that path (stack garbage when the image was loaded before); here the description is filled from
  the image every time.
- **Every description HGVK gets has the flag:** w3r_draw's (0x100d1e10) now uses the 7-dword form with flag 0 (1.0's 5
  dwords left HGVK reading the stack as the flag; 1.2's version still passes 5).
- **SKYGND.PAR:** 1.2 reads it from the game folder by fopen (not through the cache, so not from Patch); Project has
  1.2's (1.0's kept as SKYGND.PAR.bak-20261030). All 28 sky / horizon textures it names exist as .TEX. 1.2 appended
  terrain light fields per mission (hambient, l1, l2, gl) that its reader turns into light directions; our reader
  (w12_sky.c) reads the earlier fields unchanged and ignores the rest until a lighting round ports them.
- **Log:** per mission, "HGSIM.DLL: textures this mission: N .TEX, N .XEL fallback, N not found" (hook layer at sim
  exit; counters `g_w31TexLoaded` / `g_w31TexFallback` / `g_w31TexMissing`).
- **"bad rhw" (HGVK's count of unusable 1/z):** round T changes no vertex position or 1/z HGVK reads (textures,
  palettes, vertex alpha, and the band's z, which HGVK treats as before), so that count does not come from it.
- **Checker:** tools/verify/v12_common.py runs an existing 1.0 spec against 1.2's version (the two listings aligned,
  their absolute addresses paired: globals copied across, stubs hooked at 1.2's callees too). spec_w31_sky.py (clouds,
  4000 trials) and spec_w31_skyband.py (sky band, 6000) on the round-22 generators, and spec_w31_tex.py's file
  preload (2000), all exact against 1.2; planted bugs (W31S_MUTATE: the file palette one entry late, the band's far
  depth one bit off) caught. The round-22 / w2m / w3r 1.0 specs still pass with the switch off. The sky spec keeps
  [0x1012ba50] at its real 4.0 (1.2's copy is read-only; the 1.0 spec also tries -10.0).

## Round 32: 1.2's particle effects, rounds C-F (2026-10-30)

Diego: "can you analyse 1 to 4? and do them in batch?" (C PartSpec.ini, D particle core, E explosions, F other
effects; one build and one test at the end). Analysis: tools/v12/particles.md. 1.2 replaced 1.0's effect objects
by particle systems in the same pool; every new 1.2 function is `Sim12_<1.2 address>` and every new 1.2 global
`g_w32_<1.2 address>` (src/sim/fx). All of it runs only in 1.2 mode, `W32_PARTICLES()` = `W31_TexMode()` (Vulkan
renderer and 3D mode, as the .TEX textures); option 3 and noconv keep 1.0.

Round C (PartSpec.ini):
- **Ported:** the particle type table (1.2 0x1011e700, 22 types of 0x1e bytes, its initial image), the settings
  reader (0x10078e34: Red, Green, Blue, Intensity, Size, Lock, Order, Loop and Texture00.. from
  <current folder>\PartSpec.ini, through the new platform calls `Plat_IniInt` / `Plat_IniString`) and the type
  setup (0x100790be: each type gets a free texture record of 0x100..0x1ff and its textures registered; in 2D mode
  every type is reset). Project\PARTSPEC.INI is 1.2's root copy (1.0 has none).
- **Checker:** spec_w32_partspec.py against 1.2's code: reader 3000 trials, setup 1500, exact; planted bug
  (W32C_MUTATE: one texture short) caught by both (1321 / 1142 mismatches).

Round D (particle core):
- **The pool in 1.2's layout:** 1.2 keeps 1.0's effect pool (1000 records, same globals) with records of 0x406 bytes
  (1.0 0x57e) holding 16 particles of 0x32 bytes; the node / update / draw / argument dwords move from +0x552 / +0x556 /
  +0x55a / +0x57a to +0x332 / +0x336 / +0x33a / +0x366, the type at +0x402. 1.2's records fit in 1.0's area, so the
  addresses stay; the size and offsets follow the mode (`W32_REC_SIZE` / `_OBJ` / `_UPDATE` / `_DRAW` / `_ARG`).
  Switched: 0x10090513, 0x10090cc8 (w2l), 0x10090581, 0x10090c7d (w2g), 0x10090e23, 0x10090f17 (w3o), the setup
  0x100908b3 (w8x: also +0x3fa = -1, 16 particles of 0x32 cleared, then the type setup 0x100790be) and the effects
  switch 0x10090d15, converted now (src/sim/fx/w32_pool.c, hook `conv_w32.inc`).
- **1.2 additions in the draw dispatcher 0x10090f17 (1.2 0x10079eee):** it first notes Sim_100448ff() == 1 in
  0x1014b58c, and a system whose type has Lock gets bit 2 of +0x10 after its draw.
- **Particle draw (1.2 0x10078c29, src/sim/fx/w32_draw.c):** each visible particle a square (corners at x -+ size/2,
  y +- size/2) with texture coordinates from a random corner of 1.2's table 0x1011e998, sorted far to near by the C
  library qsort (comparison 0x10078be0) when the system has a texture record, the record's frame set from the
  particle (1.2 0x1000204c = 1.0 Sim_1000716a), then the polygon routine [0x1014cb4c].
- **1.2's polygon changes:** the queued polygon 0x100a539e (1.2 0x1009185b) returns for no polygon, keeps only the
  low byte of the polygon's light dword +0x3c as the vertex light and puts its other three bytes in the vertex's
  specular colour (+0x18, under the fog byte); the polygon routine 0x1009ef50 (1.2 0x1008bd80) gives the flat-colour
  polygons (0xa000) 1/z. HGVK does not use the specular colour yet (the lighting round decides).
- **Not ported:** 1.2's 0x10077a07 / 0x10077b21 (nothing calls them, nothing holds their address); the particle
  families 0x1008139b and 0x10083679 (same).
- **Checker:** the seven changed 1.0 pool functions, 0x100a539e and 0x1009ef50 still pass their 1.0 specs with the
  switch off (spec_w2g / w3o / w8x only needed the new files in their build lists). Against 1.2's code:
  spec_w32_pool.py per-frame pass 300 trials and draw dispatcher 300, spec_w32_switch.py the switch (also against
  1.0, 600 each), spec_w32_setup.py the setup 300, spec_w32_draw.py the particle draw 3000, spec_w32_driver.py the
  queued polygon 2000, spec_w32_polyroutine.py the polygon routine 3000; all exact. Planted bugs caught: W32F_MUTATE
  (Lock ignored, 3), W32P_MUTATE (record not freed, 263 / 242), W32S_MUTATE (+0x3fa left, 255), W32D_MUTATE (frame not
  set, 1242), W32V_MUTATE (specular red / blue swapped, 1185), W32R_MUTATE (1/z left, 1053). The other four pool
  functions (0x10090513, 0x10090cc8, 0x10090581, 0x10090c7d) are 1.0's code with only the record size and the four
  offsets changed: their listings align instruction for instruction (tools/v12: the aligned listings).
- **Tools:** tools/v12/funcmap.py pairs 1.2 functions with 1.0's by identical listings (1513 of 2392;
  tools/v12/funcmap.json), tools/v12/funcfind.py lists the closest 1.0 functions for the others.

Round E (explosions; in progress):
- **How the families are switched:** each converted 1.0 spawn starts with `if (W32_PARTICLES())` and hands its
  arguments to its 1.2 replacement (the same arguments; one family takes one more), so every caller, converted or
  still in the DLL, gets 1.2's effect. Pairs (1.0 -> 1.2): 0x10091085 -> 0x1007a0a6 (railgun), 0x1009201f ->
  0x1007acf3 (accel), 0x10092956 -> 0x1007b956 (laser), 0x10092f9d -> 0x1007c1a1 (shockwave), 0x100939a3 -> 0x1007d342
  (sparks / puffs), 0x100941c3 -> 0x1007e25c (skate), 0x100949e8 -> 0x1007f8a9 (smoke), 0x10094fcb -> 0x100805dc
  (grenade), 0x10095e0a -> 0x10082556 (missile), 0x10096ac3 -> 0x10084362 (tracer). New 1.2 calls added in their
  converted callers: grenade bounce dust 0x1008514d, debris smoke, 1.2's 16 extra shockwave calls; footsteps
  (0x10086007, from 1.2's new 0x1003fbb7) and the gun effect (0x1007f19f, from 0x100d5485) with their callers (Diego:
  "Both now").
- **Shared code** (src/sim/fx/w32_family.c): 1.2 repeats its spawn prologue, frame advance, particle size and corner
  template in every family; here each is one function (`W32_NewSystem`, `W32_ParticleFlags`, `W32_ParticleSize`,
  `W32_SetTemplate`).
- **Reproduced (1.2; was a bug fix until round 33, Diego: "do as 1.2 does"):** every 1.2 spawn marks 0x20 particles' frames (dword +0x30 + j * 0x32 =
  -1) in a record that holds 16 particles of 0x32 bytes (record 0x406 bytes). j = 16..19 land in the spawn's own
  record (+0x350, +0x382, +0x3b4, +0x3e6: the velocity, the corner template, two corner positions), which the spawn
  or the next draw rewrites. j = 20..31 land in the NEXT pool record at +0x12 + k * 0x32, k = 0..11: the x position
  of that record's particles 0..11 becomes 0xffffffff (a NaN). That record is almost always live: the pool hands out
  free slots from the top of a stack filled in ascending order (setup 0x100908b3: [0x10216b80][i] = i, top = count
  - 1; Sim_10090581 pops), so slots come out 999, 998, 997, ... and the record after a new system is the one spawned
  just before it, usually still on screen. Its particles' x turns NaN, they fail the projection and are no longer
  drawn: in 1.2 each new effect wipes out the previous effect's particles (for a beam, rec +0x12 is its end's x,
  recopied every update: one bad frame). For the last slot (999) 1.2 writes past its pool. Here 16 particles are
  marked; the checker builds the C with `W32_LOOP32` (1.2's 0x20) so everything else is compared exactly.
- **Tracer (type 12; 1.0 Sim_10096ac3):** spawn 0x10084362, update 0x10083dde, draw 0x10083e8a
  (src/sim/fx/w32_tracer.c). Checker: spec_w32_tracer.py (tools/verify/w32_common.py: 1.2-only specs, the globals
  paired by tools/v12/globalmap.json) 3000 / 1500 / 3000 trials, exact; planted bug W32E_MUTATE (size one bigger)
  caught (2182). Sim_10096ac3's 1.0 spec still passes with the switch off (2000).
- **Tools:** funcmap.py also pairs globals (tools/v12/globalmap.json, 13398, from the identical functions); the
  checker's closure() adds the platform stub when a file calls the platform layer.
- **Missile (type 10; 1.0 Sim_10095e0a), grenade (8; Sim_10094fcb), bounce dust (17..20; new), smoke (7;
  Sim_100949e8), gun (6; called from 0x100d5485 once it is ported):** src/sim/fx/w32_explode.c. Every explosion spawn
  shares the in-sight test and prologue (`W32_InSight`, `W32_Explosion`: 1.2 repeats them verbatim in each spawn,
  differing only in type, speed scale and the two velocity divisors: tracer and missile 8 / 8, grenade 256 / 16,
  smoke and gun 256 / 8). The grenade shot (w10_shot.c) now calls the bounce dust where 1.2's does (the bounce branch,
  shot start, the bounced velocity, 2 and the shot's ground kind +0x8e; the shot record's layout is the same in both
  versions); 1.2's other changes to the grenade shot are gameplay (the weapons round).
- **Faithful structure (Diego: "the new C code as close as possible to the original"):** each 1.2 function is
  ported line for line; only blocks 1.2 repeats verbatim in every family (the in-sight test, the spawn prologue,
  the frame advance, the size scaling, the corner template) are written once as helpers, since they are one inline
  function in 1.2's source in all likelihood. Two-loop functions stay two loops; values 1.2 computes and never uses
  are computed (and marked) where they cost nothing, and its no-op projection calls are kept (they call the
  projection, which counts).
- **Reproduced (1.2): the smoke's corner colour.** 1.2's smoke / gun draw colours a particle's first corner from the
  256-entry colour table (1.2 0x10152a60, ours 0x10185760) indexed by the polygon flags. For a textured type the
  flags are group | 0x7000 (group 0x100..0x1ff), so 1.2 reads 0x1016ee60..0x1016f25c, about 115 KB past the table.
  No 1.2 code refers to anything between 0x10164d6c and 0x10186eec; that span is the scene-object table (1.2
  0x10164d40, ours 0x101a2100, an exact pair in tools/v12/globalmap.json; 0x402 records of 0x88 bytes: name, parent,
  flags, node, ...), so the colour comes from bytes of scene-object records 302..310. That is deterministic for a
  mission (whatever those records hold, zeros when the mission has fewer objects), so it is reproduced: the C turns
  1.2's address into the same position in our scene-object table. (A first version kept the type's colour as a
  bug fix; replaced by this after the coordinator's rule: find the mechanism, reproduce a deterministic value.)
  Checker: the scene-object table is paired like the other globals; the planted bug W32C_MUTATE (one byte off) is
  caught on its own (1463 mismatches).
- **Checker:** spec_w32_explode.py, all exact against 1.2: tracer, missile, grenade, dust (spawn 3000 / update 1500 /
  draw 3000 trials each), smoke and gun spawns 3000 each, their update 1500 and draw 3000 (36 both-fault: huge
  2D sizes run both out of instructions). Planted bugs caught in every function (W32X_MUTATE: scale / one frame
  longer; W32E / W32M_MUTATE: size one step bigger). Lesson: the listing diff hides constants and filtered views can
  hide an imul (the dust's * 2000); every family is now checked with a constant scan too.

Round F (other effects):
- **Skate dust (type 5; 1.0 Sim_100941c3), shockwave (4; Sim_10092f9d), sparks / puffs (13..16; Sim_100939a3):**
  src/sim/fx/w32_explode.c. The skate particles animate along one of two frame rows (1.2 0x1018ef80 = ours
  0x10215b20; each particle keeps a pointer to its row); the shockwave flies 8 particles out at random angles (k * 80
  speed, k * 181 frames, k the 7th argument); the sparks pick their update by the mode (2D: by kind; 3D: the dust's)
  and, in 3D, their type by the shot's ground kind, a new 8th argument: our sparks caller (Sim_1006c161's helper)
  passes the shot's +0x8e as 1.2 does (the shot record's layout is the same in both versions).
- **Beams: railgun (0; Sim_10091085), accelerator (2; Sim_1009201f), laser (3; Sim_10092956):** src/sim/fx/w32_beam.c.
  The systems follow their shot record (+0x366) and draw its two projected ends as two quads (5 wide, then 2..3),
  across or up and down by the larger extent (|dx| = |dy| or unordered take 1.2's own paths); the frame advances
  twice per draw and 1.2 adds 0x7000 after each, so a stopped animation keeps drawing. The laser keeps 1.0's sound
  calls (1.2 makes the same ones). The railgun's impact calls our 0x100246b3 (1.0's), as our 1.0 railgun does; 1.2's
  changed impact (0x1001b3a1) waits for the weapons round.
- **Debris smoke (new in 1.2):** our debris update (Sim_10029421, w8u_debris.c) spawns 1.2's smoke along the piece's
  move each frame it has not come to rest (1.2 0x1001ef21's new else branch); 1.2's whole update is otherwise 1.0's.
- **Pending Diego:** 1.2's network-only branch in the part-destroyed function (16 shockwaves and a reworked tally,
  1.2 0x1005ab68 / 0x10059e9d; [0x1015ec58] is the network flag); footsteps (1.2 0x1003fbb7, its unit-animation
  rework) and the gun trigger (1.2 0x100bb425): both read 1.2's moved object fields and need a layout map first.
- **Checker:** spec_w32_explode.py (skate 3000 / 1500 / 3000, shockwave 3000 / 1500 / 3000, sparks spawn 3000, its
  four updates 1500 / 1500 / 600 / 600, draw 1500), spec_w32_beam.py (spawns 2000 each, updates 2000, the accelerator
  draw 4000, the laser's 1000, the quad helper 2000), spec_w32_debris.py (1.2's debris update 3000): all exact
  against 1.2, planted bugs caught (only the railgun's one-line draw and the quad helper have none of their own; the
  helper is covered through the accelerator draw's). Every 1.0 function edited still passes its 1.0 spec with the
  switch off: Sim_10095e0a, 10094fcb, 100949e8, 100941c3, 10092956 (spec_w6f), 10092f9d (w8x), 10091085, 1009201f
  (w2g), the grenade shot 0x100225c7 (15000), Sim_1006c161 (6000), Sim_10029421 (1500), Sim_10096ac3 (2000). The 1.0
  specs link stand-ins for 1.2's spawns (tools/verify/w32_smoke_stub.c), which never run with the switch off.

Round 32, first option 2 test (Diego: "game crashed"):
- **Cause:** crash.log's fatal fault is in W32_Explosion (HG.exe+0x83242), reading block = node +0x8c of pool record
  999 (0x10312f8a = 0x10217c20 + 999 * 0x406) whose node (+0x332) was 0. The pool had been built in 1.0's layout:
  our sim main (1.0's order) runs the pool setup 0x100908b3 before it sets the 3D mode ([0x1017b940] = 1 when
  [0x1017b944]), so `W32_PARTICLES()` (Vulkan and 3D mode) was false at setup and true afterwards; the first spawn
  took slot 999 with 1.2's record size and found no node there.
- **Fix (1.2's own order):** 1.2's sim main (0x1006fae1) calls the pool setup (0x1007986e) after the display and
  renderer set-up, once the 3D mode is known, and sets its .TEX flag 0x10105048 just before it. Our sim main now does
  the same in 1.2 mode (`g_w31Tex`); 1.0 mode keeps 1.0's place. Round D had missed this move in the sim main.
  SimMain's 1.0 spec still passes (4000).
- **Also fixed:** an older hook (hooks_view.c "SetParticles", which forces the effects on) owns 0x10090d15 and called
  the DLL's 1.0 original, so the round-D conversion of the switch was skipped at install ("bytes differ"). The view
  hook now calls the converted Sim_10090d15 (the 1.0 original frees with 1.0's record size); conv_w32.inc has no
  entries.
- **Not from this round:** the log's earlier, handled faults at HG.exe+0x17576e .. (writes to 0x98009800 and the
  like) are outside the sim, in the statically linked library code; the game ran on past them.

Round 32, grenade dust not seen (Diego: "I dont see dust on grenade bounce"; his choice, option a: "replicate 1.2 and
we will fix it from there if is not good enough"):
- **Cause:** the counters from his run (177 bounces, 176 dust systems, 4888 draws) show the dust is made and drawn.
  1.2's particle squares carry no depth (corners keep z 1 and 1/z 1 from the template); 1.2's renderer had no depth
  buffer and drew in the game's order. HGVK gives such a polygon the previous polygon's depth, so the ground hid it.
- **Change (a deviation at the renderer boundary, Vulkan only):** the particle draw Sim12_10078c29 and the beam quad
  helper Sim12_10086810 call the renderer's SetState (slot 0x109e3d40) with our own code 0x48470001 (W32_STATE_PAINTER),
  1 before the polygon call and 0 after. HGVK draws polygons sent while it is on with pipelines that have no depth test
  and no depth write, in the order they come. Nothing else changes: other polygons keep the depth test, and 1.0 mode
  never sends the code.
- **Checked:** Sim12_10078c29 3000/3000 and the beam functions 0x10086810, 0x1007a379, 0x1007a092, 0x1007afdc pass
  against 1.2 with the new call given a bare ret (v12_common.painter_slot); the planted bugs are still caught.
  spec_w32_draw ignores the test counters g_w32Count while they exist.
- **Result (Diego: "that worked"):** the dust shows. That run: 96 grenade bounces, 85 dust systems in sight, 2384 dust
  draws, 2197 systems made, none refused, 150210 particle squares; HGVK logged SetState(0x48470001, 1/0); no new
  fault (crash.log unchanged since 09:05); all 188 .TEX textures found; the only skipped hooks are the two usual ones
  (0x585c4, 0xac603).
- **Counters removed** after that run (w10_shot.c, w32_explode.c, w32_family.c, w32_draw.c, w32.h, hooks_conv.c, and
  spec_w32_draw's ignore); the code is back to what the checker passed. Rebuilt (option 4).

Round 32, network part-destroyed branch (Diego: "do as 1.2 does, we can rework network later, but remember the
change"):
- **Ported (1.2 mode, `g_w31Tex`):** 1.2's part-destroyed function 0x1005ab68 differs from ours (Sim_1006ad98) only in
  one branch at its top: a kind-4 unit, a network game ([0x1015ec58]), part 2 and the local player as the killer
  ([0x1017b8c8]) give 6 shockwaves at node +0x4c0, 10 at node +0x4c4 and the kill tally, and the part is not marked.
  It calls our tally (1.0's) until Diego decides on 1.2's tally (network only; recorded in AFTER_DLL.md, network).
  1.0 mode is unchanged.
- **Checked:** spec_w10_1006ad98_v12.py against 1.2's 0x1005ab68, 5000 trials exact (the unit's node pointer copied
  from our +0x48 to 1.2's +0x44 for its run; the callees paired by hand, the aligner mispairs them); planted bug
  W10N_MUTATE (one shockwave fewer) caught in 789 trials. The 1.0 spec still passes with the switch off (5000).

Round 33 (Diego: "I want to migrate the whole game to 1.2 ... I WANT MY GAME TO BE LIKE 1.2"; from now on every 1.2
change is the game's behaviour, for every renderer, with no 1.0 reference kept):
- **The 0x20 particle marks, as 1.2 (round 32's fix undone):** W32_NewSystem marks 0x20 particles as every 1.2 spawn
  does, so a new effect wipes the previous effect's particles out, as in 1.2. For the pool's last record (slot 999,
  the first one handed out whenever the pool is empty) 1.2's marks run past its pool (1.2 0x10191060, 1000 x 0x406,
  ending at 0x1028c7d0) into the data after it: 0xffffffff at 1.2 0x1028c7e2 + k * 0x32, k = 0..11. Our pool keeps
  1.2's records in 1.0's larger buffer (0x10217c20), so a mark past its 1000th record is sent to our copy of the
  data 1.2 has there (`W32_Mark`): the same variables follow the pool in both versions (our 1.0 pool also ends right
  before them, at 0x1036f050), paired from the relocations of identical functions: 1.2 0x1028c7e0..0x1028c95f = ours
  + 0xe2900, 1.2 0x1028c960..0x1028caab = ours + 0xe2940. What the twelve marks hit: k = 0..4 the saved camera
  (0x1036f0e0, 0x38 dwords; Sim_1008fb20 saves it when a secondary view starts drawing, Sim_1008fd5a puts it back),
  k = 5 a gap after 0x1036f1d8, k = 6..7 the saved 0x1014cae8 (0x1036f1e0, 0x1a dwords, same pair), k = 8 a gap no
  1.2 code refers to, k = 9..11 dwords 17, 29/30 and 42 of the cell list 0x1036f2a0 (its readers stop at its 0 end,
  dword 4 at the latest). The saved copies are written again at every view start, so a mark only shows if an effect
  spawns while a secondary view draws (none found): exactly what 1.2 does in any case. The checker no longer builds
  with W32_LOOP32 (the C now marks 0x20 itself); the spawn specs re-run (beam and explosion spawns, all exact).
- **Option 3 runs 1.2 (Direct3D on 1.2's dx5draw.dll):** Project\plugin\DX5DRAW.DLL is now 1.2's (GameRip_12,
  82432 bytes; same 16 exports, same plugin id 1 and /D3D contract as 1.0's). 1.2's game (Patch folder, .TEX
  textures, particles and the other 1.2 ports, `g_w8yPatch` / `g_w31Tex`) is on with Vulkan and with Direct3D
  (`Hooks_Renderer12`: hgrc.cfg / HG_RENDERER "hgvk" or "d3d"). 1.2's renderers number render modes and states as
  1.2 does; our sim sends 1.0's numbering (which HGVK keeps), so the plugin load (Sim_1003650d) wraps the Direct3D
  plugin's Render, RenderNoClip and SetState slots (src/sim/render/w33_dx12.c): modes 5..18 become 7..20, state
  codes 1.0's minus 1, our painter state (round 32, Vulkan only) is dropped (1.2's dx5draw returns 1 and does nothing
  for codes it does not know; it has no depth buffer and draws in the game's order anyway). The full-resolution HUD
  stays with HGVK. Sim_1003650d's spec still passes (8000).
- **Option H (noconv)** runs the untouched 1.0 DLL, now on 1.2's renderer: not a valid reference any more. The
  reference becomes the untouched 1.2 game, run from its own copy (to be set up).
- **1.2's 10-player kill table and rating (network only; single player is unchanged):** the kill tally
  (Score_RecordKill = 1.2 0x10059e9d) is 1.2's: it no longer marks 0x1015d1b0[unit] in a network game, counts kills
  by players 0..9 (1.0: 0..7) in 1.2's 10 x 10 table (rows of 0x14 per killer, a column per victim; new C array
  g_w34Kills = 1.2 0x1028cc2c, 1.0's 8 x 8 0x1016552c is no longer used; 1.2 grew it in place and moved what
  follows by 0x48, so it gets its own array here and the winner pick keeps 0x101655ac), and counts the kills of the
  local player's kind-3 units, sent with 1.2's new message "AK" (Sim12_100352b7, 0xe bytes; receive
  Sim12_10035348). The "KI" send (Sim_100413c7 = 1.2 0x10035088) sends 10 entries (0xc bytes), the rating sum
  (Sim_10041444 = 0x10035105) covers 10 players into g_w34Rating (1.2 0x107caa60; 1.0's 8 at 0x1015d0f0, which the
  two scoreboards Sim_100ce586 / Sim_100cde94 now read from g_w34Rating), the "KI" receive (Sim_1004158c =
  0x1003524d) and the "AK" receive ignore messages once the game is over (1.2's guard 0x101314c0 = g_w34NetOver,
  set by the message loop from the game-over flag 0x1015f1fc before each message, as 1.2's loop sets it), and the
  winner pick (Sim_1007b02d = 0x10067f2d) reads the new table. The message loop (Net_ProcessMessages) takes "KI"
  with 0xc bytes and the new "AK" with 0xe; the rest of 1.2's reworked loop (its other new messages, 10-player
  loops) is for the network rewrite (AFTER_DLL.md, network).
- **Checked:** spec_w34_rating.py against 1.2: the tally 3000 (planted bug W34_MUTATE, the kind-3 kill in the
  wrong column, caught 174), the "KI" send / receive and "AK" send / receive 1500 each, the rating sum and the
  winner pick 2000 each; all exact (1.2's player records lack our dword +0x18 and are turned into its layout for
  its run). The scoreboards' 1.0 specs pass with the ratings in g_w34Rating (4000 each). The message loop's three
  changed lines are not checked against 1.2 (its 1.2 version as a whole is not ported).

Round 33, particles black with Vulkan (Diego: "the bullets and smoke in 1.2 are white in direct3d, in vulkan they
are black"; 1.2's dx5draw is the reference):
- **Cause:** the particle squares reuse their four corners (1.2's template, rec +0x36a), and 1.2's textured submitter
  (0x100012c5 = our Render_SubmitVerts_10006744, identical) shifts each corner's light in place at every draw,
  light = light << 3 | 7: 0xff, 0x7ff, ..., 0x7fffff, .., 0xffffffff after a few frames. 1.2's dx5draw (0x10004360)
  builds the vertex colour as colour byte times light, unsigned and wrapping, >> 8, packed by shifts and ORs with no
  clamp: these lights give white. HGVK multiplied as signed ints and clamped negatives to 0: black.
- **Fix (HGVK, exactly dx5draw's arithmetic):** PackColour now computes the packed colour as dx5draw does (unsigned
  wrap, overflow ORed into the next byte). Also from dx5draw: it never sets the texture blend, so D3D's default
  MODULATE applies, where a texture with alpha replaces the vertex alpha: HGVK now ignores the vertex alpha for
  textures with alpha (1.2's .TEX effect textures) and keeps it otherwise. The texture conversion (alpha 15 - top
  nibble, RGB from the palette's xRGB) already matched (checked on FXSMOK04, FXBOOM00, FXDUST02); HGVK keeps 8-bit
  colour where dx5draw had 4 bits.

Round 33, the untouched 1.2 reference (dev.cmd option H, replacing the noconv 1.0 run):
- **What it is:** C:\Coding\HeavyGear\Ref12, a copy of GameRip_12 (1.2's own HGSIM, HGSHELL, plugins and data; its
  hg.exe kept as hg12-original.exe), started by our launcher (HG.EXE) with "d3d noconv" on 1.2's dx5draw.dll, mission
  M01_SCN1 straight away. 1.2's own hg.exe asks for the Gameplay CD and there is no disc image.
- **No CD, 1.2's own way:** 1.2's hg.exe passes (flags & 2) as the last argument but one of SimMain and ShellMain
  (1.0 passed 1); flags is a build option stored in its hg.exe right after the bytes 34 12 98 40 f9 37 88 e9,
  shipped as 0. Both 1.2 DLLs read bit 2 as "no CD": without it 1.2's sim keeps [0x1014a590] = 1 and runs its CD
  search (1.2 0x10087460, which really looks for a CD-ROM drive holding \video\B51MOVIE.AVI; 1.0's was broken and
  always answered "found") and stops with SYSERR_CDNOTFOUND, and 1.2's shell ([0x10259598]) searches the CD drives.
  Our launcher now passes bit 2 (HG_LAUNCH_FLAGS, src/launcher/main.c), 1.2's own no-CD setting, to both, and our
  SimMain reads the argument as 1.2's does (g_w33CdNeeded = 1.2 [0x1014a590]; 1 or no bit 2: the CD search, else
  none). The reference runs untouched 1.2 code only (0 hooks with noconv). SimMain's 1.0 spec passes with that
  argument kept to values without bit 2 (4000; the no-CD path is 1.2's six lines, not checked against 1.2 since 1.2's
  SimMain as a whole is not ported). Not ported (Diego's choice, 2026-10-31): 1.2's crash handler around its WinMain,
  which uploads crash reports to Activision's servers through ANET2's dpReportCrash; our local crash.log stays.
- **Mission files:** a direct mission start needs the files the shell writes when it prepares a mission (USERSTAR.BWD,
  USERGEAR.GDF, HGPRM.CFG, HGMSN.CFG, HGCAR.CFG, the random-mission BWD / BRF / RMG files, ...): copied from Project
  (not CHARINFO.HG1 or CURRENT.CCT; Ref12 keeps 1.2's own). Checked: the untouched 1.2 game reaches mission 1
  (screenshot, Direct3D 640x480).

Round 33, 1.2's shell and DLLs in Project (whole game 1.2):
- **Compatibility (read-only analysis):** 1.2's HGSHELL.DLL exports the same ShellMain / ShellWindowProc; its imports
  differ only in three MSVCRT data symbols (same MSVCRT.DLL) and GetPrivateProfileStringA. Our shell hooks are all
  import patches by name (the SDL3 DirectDraw stand-in: DirectDrawCreate, DirectDrawEnumerateA, ScreenToClient,
  ClientToScreen, GetWindowRect), none by address. hgprm.cfg is read the same way by 1.2's hg.exe (0x344 bytes, the
  sim command line at +0x240). The save CHARINFO.HG1 has the same format in both shells (header, zlib stream,
  0x370-byte character records, the same field copies), so Diego's save is read as before. ShellMain's last argument
  is 1.2's "no CD" switch (above); the launcher passes bit 2. 1.2's other changed DLLs keep every 1.0 export
  (ANET2 adds many, KEYBOARD and FFORCE are unchanged in names, DKUPDDLL drops three nothing calls); 1.2's FFORCE
  needs the new IFORCE2.DLL. No hook of ours targets any of them by address.
- **Installed in Project** (1.0 files kept in Project\backup-1.0-20261031): HGSHELL.DLL, ANET2.DLL, anet\*.dll,
  DKUPDDLL.DLL, FFORCE.DLL, KEYBOARD.DLL, the plugins 3dfxgear / i76draw / renddraw, ANET.INF, update.txt,
  splash\SPLASH.EXE, and 1.2's new files IFORCE2.dll, HG.ifr, splash\SPLASH.DAT, M51DUES1.BWD, M51DUES2.BWD,
  M99MSCN1.BWD. Not touched: CHARINFO.HG1, CURRENT.CCT, HGSND.CFG, DLLFILE.INI, HG.EXE (ours), HGSIM.DLL (1.0's,
  which our converted code hooks).
- **Launch check:** the menus start with 1.2's shell (log: the shell loads, 0 hooks, 5 DirectDraw imports replaced,
  the intro screens in 16-bit then the 8-bit menu mode, same sequence as 1.0's shell). The menu picture itself was
  not confirmed: the screen capture showed Diego's desktop, not the game window.

Round 33, lighting (Diego: "I want to match 1.2 sky lighting"; Voodoo 2's look is the reference, his pick):
- **SKYGND.PAR's coloured light, as 1.2:** the reader (Sim_100a0063 = 1.2 0x1008ceab) takes 1.2's line format with four
  more keys: hambient (ambient colour), l1 and l2 (two lights: colour and direction) and gl (the global light's
  colour); the directions are made unit vectors (left as they were below a length of 1e-7) and the colours clamped
  to 0..255 (C globals g_w33*, 1.2 0x10138d20..0x10138d74). The view transform (Sim_1005874a = 1.2 0x1004b44a) turns
  the two directions into the camera's frame (1.2 0x101181a4.. / 0x101181a8..). The vertex lighting
  (Sim_10058c5e = 1.2 0x1004bad6) keeps 1.0's light (+0x3c, 0..0x1f) and adds a colour per point in +0x3c's upper
  three bytes: the ambient, plus the global light's share times gl / 128, plus light 1's and light 2's shares (their
  light times 255, the colour / 128), plus white for each point light reaching the point. The point lights (weapon
  fire, contrails: the readme's "New lighting has been added for weapon firing and contrails") now also count in 3D
  mode with fog on, except for the b000..e000 polygons; 1.0 skipped them whenever fog was on. The view function
  (Sim_10032a10 = 1.2 0x10026610) reads only +0x3c's low byte as the light and, darkening, keeps +0x3c & 0xffffff as
  1.2 does (the old light byte stays ORed in and the blue byte is lost: reproduced). 1.2's queued polygon (round 32)
  already sends the light byte as the vertex light and the colour as the vertex specular colour. No other 1.2 code
  masks +0x3c (byte scan of 1.2's code: these two functions only).
- **Checked against 1.2:** spec_w12_sky_v12.py 4000 (planted W12S_MUTATE caught 478), spec_w3p_light_v12.py 3000
  (W3PL_MUTATE 2552), spec_w10_10058c5e_v12.py 5000 (W10L_MUTATE 1483), spec_w9h_view_v12.py 3000 (W9HV_MUTATE 108);
  all exact. (spec_w3p / spec_w9h's own source lists predate rounds 24..33: the v12 specs add round 31's files or
  build the closure of their files.)
- **Where the colour shows:** 1.2's dx5draw never turns on specular, so with Direct3D (options 3 / H) the colour is not
  drawn. 1.2's 3dfxgear.dll never reads the specular bytes either; the colour reaches the screen because 1.2's sim, in
  its 3DFX / PowerVR mode ([0x1014b954], set when the renderer's name contains "3DFX" or "POWERVR"), writes it into
  the vertex colour (the textured submitters and 12 other functions test that mode). Diego wants Vulkan to show it
  (Voodoo 2 look): next, that mode for HGVK. The HGVK side needs no shader for it (3dfxgear draws vertex colour times
  texture); the per-mode colour combine of 3dfxgear (texture only in its keyed / blended modes) is held for Diego.
- **Fog (CPU side, HGVK):** FirstDevice's fog switch and colour are kept (Vk_SetFog) and each vertex carries D3D's fog
  factor (+0x1b / 255); nothing draws with them until the shaders change (a GLSL compiler is not installed yet).
- **1.2's 3DFX / PowerVR vertex colour, the textured submitters (step 1 of that mode):** g_w33Voodoo (1.2
  [0x1014b954]); with it the three textured submitters (0x10006609 / 0x10006744 / 0x100068d9 = 1.2 0x100010d9 /
  0x100012c5 / 0x10001513) colour each vertex from its specular colour (the coloured light) times the channel factors
  / 256, and the first two make it opaque, as 1.2 does. The flag is not yet switched on (how HGVK is recognised is
  Diego's pick); its other 10 users follow. Checked: spec_w31_tex.py's three submitter specs against 1.2 with the mode
  on in a third of the trials, 2000 each, exact (planted W2NV_MUTATE caught 97 / 108 / 97); spec_w2n.py's 1.0 specs of
  the three still pass with it off.
- **The shell's argument (Diego: option 1 showed only a blue screen with 1.2's shell):** 1.2's shell reads the same
  argument differently from the sim: its flag [0x10259598] = (arg == 1 ? 0 : !(arg & 2)). Only the shipped 0 sets it
  (its full setup: a list of 0x49 entries at 0x10139118, as 1.0's shell always has; 1 or 2 give 0x33 entries, a
  reduced setup), and with it the shell searches the CD drives only for the DVD edition's movie path (wavmap.ini,
  absent here). Our launcher passed 2 to both DLLs; the shell now gets the shipped 0 and the sim keeps 2 ("no CD").
  The log of Diego's run: the shell started, showed an 8-bit screen, a 16-bit screen that 1.0's shell does not have
  (2 pictures), then the 8-bit menu mode, and returned 255 (no fault, crash.log unchanged); his CHARINFO.HG1 was
  rewritten by the shell with the same size and header. Whether the reduced setup is what left the screen blue is
  not proven (no run of mine on his desktop); the shipped value is 1.2's own in any case.

- **The blue screen, read from 1.2's shell (round 33):** both mode switches of the run come from 1.2 HGSHELL
  0x1017b382 (DirectDraw init; 16 bits while [0x10247758] is set). [0x10247758] is the movie flag of 0x100319d5,
  1.2's new movie player: it opens a full-screen popup of its own class (window procedure 0x1002f450, not
  ShellWindowProc), plays the movie through DirectShow (CoInitialize, a filter graph), re-inits DirectDraw in 8 bits
  and destroys the popup. The shell then draws (0x100318fa) and takes mouse / keys (ShellWindowProc) only while
  its focus flag (object +0x1f8: WM_SETFOCUS 1, WM_KILLFOCUS 0) is set. In Diego's run the menu mode made 12 blits
  while the mouse moved for minutes (1.0's menu: 17642), so the menu was never drawn: the focus flag left at 0 by
  the popup is the leading suspect, not proven. The 255 is one of the shell's quit paths (WM_DESTROY of its window,
  WM_QUIT, the quit menu choice, Ctrl+Q); no DirectDraw call of the stand-in was refused. HeavyGearIso (the full
  1.1 + 1.2 install) has the same game files as Project, byte for byte: not the cause. Logging added (no change in
  behaviour): the shell's CreateWindowExA / DestroyWindow (with the focus after) and WM_SETFOCUS / WM_KILLFOCUS /
  WM_DESTROY on the shell route.
- **The focus given back (round 33, coordinator's go):** HGSHELL's DestroyWindow import is replaced (outside
  1.2's code, hooks_ddraw.c): after the shell destroys a window other than its game window, and the focus is not on
  the game window, the game window is brought forward and given the focus (SetForegroundWindow + SetFocus), so the
  shell gets the WM_SETFOCUS that sets its drawing flag, as on a 1998 desktop where the game window was next in
  line. Logged ("focus given back"). Undo: drop the two calls. The movie itself: 1.2's player opens the movie's name
  + ".AVI" through DirectShow; no .AVI file exists in Project or in HeavyGearIso (only smk\*.SMK), so the movie
  most likely fails to open (the CD's files) and the popup (black background) closes at once: the 2 pictures of the
  16-bit screen. Not a fault of ours; the next normal option 1 run shows both.
- **1.2's intro movie (round 33):** the shell finds its movies by going through the drive letters, keeping only
  CD-ROM drives (GetDriveType 5) and taking the first with a `video\` folder (0x1012b869; no volume label is read),
  unless wavmap.ini's [WAVMap] DVDDrive / DVDMoviePath (DVD edition) gives the path. On Diego's PC: V: "Heavy Gear"
  (disc 1, no video folder) and W: "HEAVYG" (disc 2, VIDEO\: 19 Indeo 5 AVIs, 186 MB; the intro is B51MOVIE.AVI);
  the 32-bit Indeo 5.10 decoder and its DirectShow filter are installed on that PC. (1.2's sim CD check, 0x10087460,
  looks only at the first CD-ROM drive for video\B51MOVIE.AVI: with V: first it fails; ours skips it.)
  The movie is a DirectShow video window made a child of the game window (owner = the shell's window,
  0x10022dce: WS_CHILD, 0,0 640x480, picture 2x the 320x240 source), above the shell's black full-screen popup.
  With the SDL3 display the picture covered that child: Diego saw our empty 16-bit picture (blue) for the movie's
  length. Fix (hooks_ddraw.c, outside 1.2's code): a 16-bit SetDisplayMode while the shell's popup exists opens
  no picture and makes the game window the mode's size, centred, on top (as the 640x480 screen of 1998); the 8-bit
  switch after the movie restores the full-screen window and picture. Undo: drop that branch. The focus hand-back
  on the popup's DestroyWindow stays as a safety net (it did not fire in Diego's run: the focus was back already).
- **Sim end and quit (round 33):** 1.2's SimMain returns 0xff only when [0x10131458] (ours 0x1015ebf8) is set, by
  1.2 0x100308a8 = our Sim_1003ce98 (identical: a menu item confirmed by Enter or its number key); 1.2's HG.EXE
  exits on a low byte 0xff as ours does. A destroyed gear does not set it. No difference.
- **No CD for the movies (round 33, Diego's go):** the disc's 19 AVIs (VIDEO\, 186 MB, copied by Diego to
  HeavyGearIso\VIDEO) are in Project\video and Ref12\video. 1.2's shell looks for them only on CD-ROM drives: its
  start-up check action (0x100fc406) and its movie action (0x100fc782, which takes wavmap.ini's DVD path only with
  its set-up flag 0, i.e. the reduced set-up; with ours it searches the drives too) and its set-up (0x1012b869).
  So wavmap.ini is not used. src/hooks/hooks_cd.c (outside 1.2's code, HGSHELL's import table only): when no
  CD-ROM drive holds X:\video\ and the game folder has video\, a free drive letter (from Z: down) is added to
  GetLogicalDriveStringsA, reported by GetDriveTypeA as a CD-ROM, and its "X:\video..." paths are turned into the
  game folder's video\ in _access and MultiByteToWideChar (the path DirectShow opens). A real disc with its video
  folder still wins. Logged ("no-CD movies: ..."). The sim's B51MOVIE check stays skipped by 1.2's no-CD bit
  (launch argument bit 2). Undo: drop the HooksCd_OnShellLoaded call.
- **Checker:** spec_w8x_voodoo_v12.py: Sim_100a4c45 against 1.2 0x10091065 (2000 trials) and Sim_100a4d8e (with the
  new Sim12_10090a85) against 1.2 0x100911ec (3000 trials), 0 mismatches; W8X_NEG=1 (W7OV_MUTATE, W8XV_MUTATE): 406
  and 82 mismatches. Both-fault trials (205 / 112) not yet looked at.
- **The movie's picture (round 33, Diego's run: sound, no picture, a 640x480 black box):** DirectShow builds the
  full chain for B51MOVIE.AVI on this PC (checked with a 32-bit test that only builds the graph: AVI splitter,
  Indeo 5.10 decoder, Video Renderer, DirectSound). The shell makes the video window a child of the game window,
  which has carried SDL's Direct3D 11 picture; the leading suspect for the missing picture is that window (not
  proven). Changed (outside 1.2's code): the shell's 16-bit movie switch now hides the game window and puts the
  black full-screen popup on top (hooks_ddraw.c); hooks_cd.c keeps the shell's filter graph (its CoCreateInstance
  import) and, at the movie loop's first wait (its MsgWaitForMultipleObjects import), gives the video window to
  the popup through DirectShow (IVideoWindow put_Owner / SetWindowPosition, IBasicVideo SetDestinationPosition):
  the largest 4:3 area of the screen, centred, the shell's own picture placement scaled to it (the 320x192 films
  keep their bands). A migration presentation choice: 1.2 showed them on a 640x480 screen. Undo: drop the two
  imports in HooksCd_OnShellLoaded and the hiding in DevSetDisplayMode.
- **Movie picture parked (round 33, Diego: "I can recode them"):** the intro and cutscenes play with sound only
  under our window layer; the original 1.2 install shows the picture on the same PC, so the cause is ours (not
  pinned down: the SDL3 Direct3D picture on the owner window is the leading suspect). The two presentation tries
  (game window 640x480 centred; then hidden, with DirectShow's window moved to the popup) are taken out again:
  with the second the menu after the movie drew 12 frames before Diego closed it, while the plain path (the 16-bit
  switch handled like any other) gave a working menu. To be solved in the video round by playing the movies
  ourselves from re-encoded copies (format chosen then). Kept: the no-CD drive mapping (hooks_cd.c), the movie
  popup tracking and the focus hand-back on its DestroyWindow (hooks_ddraw.c).
- **Renderer state codes, fog (round 33):** 1.0's dx5draw SetState(code, value) knows codes 1..7; code 2 is fog
  (value bit 0x200000 -> D3DRENDERSTATE_FOGENABLE). 1.0's sim called (0x200000, 1/0), a code its renderers reject
  (fog never toggled by those calls); 1.2 calls (1, 0x200000) / (1, 0), its code 1 = 1.0's 2, so "1.2 code = 1.0
  code - 1" holds and the ports' (2, 0) / (2, 0x200000) are right. Ported 1.2's two fixed call sites: the display-
  kind command 0xc5 of Sim_1004954e (1.2 0x1003b3ba, also its 3DFX / PowerVR state 0xe = our 0xf on / off) and
  Sim_1006b52f's case 0x1bee (1.2 0x1005b5c6). Checker: spec_w14_1004954e_v12.py (command 0xc5, 3000 trials, 0
  mismatches; W14_NEG12: 594) and spec_w10_1006b52f_v12.py (6000, 0; W10F_NEG: 2). Data found on the way: 1.2's
  player records hold their fields 4 bytes earlier (unit +0x24, flags +0x18) and its unit its control block at
  +0x50 (1.0 +0x54); our sim keeps 1.0's layout (the specs map them).
- **Voodoo port, deferred draw list (round 33):** Sim_100a4c45 (1.2 0x10091065: state 0xf off / on around the last
  group) and Sim_100a4d8e (1.2 0x100911ec: Sim12_10090a85, 1.2's partition by object kind 0x8000 / 0x9000 before
  the depth sort, when [0x1014cb1c] is 0; fog on -> state 2 = 0x200000), all under g_w33Voodoo (still 0).
- **Blue box after the intro (round 33):** after the movie the shell redraws its screen and palette only on a
  WM_SETFOCUS (ShellWindowProc 7: focus flag and redraw request +0x1dc). In 1998 its game window got one when the
  movie popup closed; here the 16-bit / 8-bit mode switches (PlaceWindow) had already given the focus back, so none
  came and the menu stayed blue (Diego's 17:34 run: 12 blits; his working 16:2x run had focus changes after the
  movie, from his own clicks). hooks_ddraw.c's DestroyWindow import now sends the game window WM_SETFOCUS when the
  shell's popup closes and the focus is already there. Correction (Diego: then black): sent from inside the
  DestroyWindow it was handled while the shell's movie routine (0x100319d5) still ran, before it reloaded the menu
  screen, so the one redraw came too early. Now posted: the shell's main loop dispatches it after the routine has
  returned, as the focus change in his working run (from his own click) did.
- **The 3DFX flag is 1.0's own (round 33):** 1.0's SimMain sets [0x1017d0ac] when the plugin path contains "3DFX";
  1.2 sets [0x1014b954] for "3DFX" or "POWERVR" - the same flag. g_w33Voodoo now stands for it everywhere: SimMain
  sets it (with 1.2's POWERVR test), the sky band tests it (1.0 [0x1017d0ac]; 1.2 0x1008f7f0 skips its band block in
  that mode), and SimMain also takes 1.2's other two uses: the device flags' 0x80 replaced by the flag (with
  g_w31Tex; HGVK reads only 0x200 and 2) and the 3D start-up block run also in that mode. With our plugin named
  HGVK.DLL the flag stays 0 (Diego's choice of switch still open). Checker: spec_w21_simmain.py (1.0, 4000 trials,
  0 mismatches; the C's flag compared at [0x1017d0ac] after the run - chained with w31_wrap's own post step, a host
  write marked dirty; the name test answers "no" with 3D off; built without the POWERVR test, W21V_MUTATE) and
  spec_w31_skyband.py (against 1.2, 6000, 0; the flag copied to 1.2's [0x1014b954]).
- **Black menu after a mission (round 33, Diego):** the sim runs in a separate process (Vulkan / d3d); the
  launcher hides the game window meanwhile and showed it again after, but the foreground stayed elsewhere (the
  child had it; Windows' foreground lock), and the 1.2 shell reads its focus once at start (0x1002fb63: +0x1f8 =
  GetFocus() == its window) and draws nothing without it (his run: 6 pictures, no focus message). In 1998 the sim
  used the same window, which kept the focus. Now the child grants the foreground before it exits
  (AllowSetForegroundWindow) and the parent brings the game window forward and gives it the focus before the shell
  starts (SetForegroundWindow + SetFocus, logged "Sim process ended: ...").
- **Particle setup in the 3DFX mode (round 33):** Sim12_100790be resets the particle types only without the 3D mode
  and the 3DFX / PowerVR mode (1.2 0x100790be). spec_w32_partspec.py maps g_w33Voodoo to 1.2's [0x1014b954]
  (generator 0 / 1) and builds w33_dx12.c: 1500 trials, 0 mismatches; W32PV_NEG (the 3DFX test left out): 120.
- **Display mode 6's start (round 33):** 1.2 0x10071b9c is the start function of display mode 6 / device class 3
  (the renderer plugin; mode records 1.0 0x1014b860, 1.2 0x1011e5e0). 1.0's (0x100877bc) returns 0; 1.2's first sets
  the screen format record's colour masks to 5:6:5 in its 3DFX / PowerVR mode (1.2 0x1002bd5b = 1.0 Sim_1003772b,
  record 1.2 0x107edf60 = 1.0 0x109e3880, found by 1.0's DirectDraw caller 0x100800ed). Ported in Sim_100877bc.
  spec_w2k_mode6_v12.py: 300 trials, 0 mismatches; W2K_NEG: 197.
- **Black menu after a mission, second look (round 33):** with the focus given back (his 18:34 run: focus and
  foreground the game window at the shell's start) the restarted menu still drew 6 pictures: 1.2's shell asks for its
  whole screen and palette only on WM_SETFOCUS (ShellWindowProc 7; FUN_100306c6, the only setter of +0x1dc, has no
  other caller), and an already focused window gets none. In 1998 every exclusive mode switch activated the game
  window. The DirectDraw stand-in's SetDisplayMode now posts WM_SETFOCUS when the window has the focus (else gives
  it the focus, which sends one): the shell's loop handles it after its screen is set up, as in the intro fix.
- **Black menu after a mission, third look (round 33):** the mode-switch post did not help (his 18:42 run): 1.2's
  start routine (0x100301d7) sends 0x420 and 0x1c, then removes and drops 10 waiting messages (PeekMessage
  PM_REMOVE), which ate the posted WM_SETFOCUS. Now (src/hooks/hooks_cd.c, imports of the shell): the first
  PeekMessage after that flush posts WM_SETFOCUS when the game window has the focus (else foregrounds it and gives it
  the focus, which sends one), so the shell's loop handles it; the constructor's one focus test (0x1002fb63, +0x1f8 =
  GetFocus() == its window) is logged with its value ("shell start:" lines). The launcher also calls SetActiveWindow
  on the game window after the isolated sim exits, before SetFocus, as Win98 activated it in the same thread.
- **Round 1 of the gameplay port: footsteps and the gun (round 33, Diego's go).**
  - Footsteps: 1.2's unit animation 0x1003fbb7 is our Sim_1004f257 (same constants and ladder) plus one addition:
    when the step bit (+0x88 bit 8) is set and the unit is moving, dust at the lower foot (unit record node +0x4c4,
    else +0x4c0) against the record's velocity +0x548, for every unit; the step sound stays the player's. Ported in
    src/sim/unsorted_1004/w10_anim.c behind W32_PARTICLES(). 1.2's footstep family (spawn 0x10086007, update
    0x10085992, draw 0x10085b34) is byte for byte the skate dust's with type 0x15: src/sim/fx/w32_explode.c shares
    the spawn (W32e_Dust) and its update / draw call the skate's.
  - Gun: 1.2's missile launch 0x100bb425 (our Sim_100d5446, src/sim/unsorted_100d/w10_missile.c) adds, after the
    shot's sound slot, smoke at the weapon's mount node (unit record +0x498 + 4 * slot +0x11c) moving (0, 5, 0)
    (0x1007f19f), behind W32_PARTICLES().
  - Not ported (force-feedback round): in the same function 1.2 replaced the force-feedback request
    Sim_10045f3b(kind, 0x2d, 0x32) with its reworked 0x100382fb(kind, 0, 0, 0), only while [0x10131718] is set; ours
    keeps 1.0's until that round.
  - Checker: spec_w10_1004f257_v12.py (6000) and spec_w10_100d5446_v12.py (4000) against 1.2, with the object, owner
    and player records shifted 4 bytes for 1.2's run; spec_w32_footstep.py (the footstep spawn 3000, update and draw
    1000 each, the skate spawn 3000): all exact, planted bugs caught (W10FS_MUTATE, W10G_MUTATE, W32FS_MUTATE). The
    1.0 specs spec_w10_1004f257.py (15000) and spec_w10_100d5446.py (4000) still pass with the switch off.
- **Controls round: 1.2's input changes and force feedback (2026-10-31, alongside round 2).**
  - Survey check: 1.2 adds no controls. Its HGSIM has no new command names (torso_tilt and the rest are in 1.0's
    HGSIM and HGSHELL); 1.2's INPUTW.MAP only rebinds existing commands (already in Project). Of the 29 "new input"
    functions in tools/v12/survey.md, about 15 are 1.2's zlib inflate (strings "invalid distance code" ...), the rest
    sound, the WTB reader, network, the dead Patch reader 0x100a3a92 and mid-function cuts. Of the 7 "changed" ones,
    5 are layout only (the 4-byte record shift written as an immediate, add reg,0x228 -> 0x224) or assert line numbers;
    0x10015018 (1.2 0x1000e1a8) adds an "or [0x1012d4e8] != 0" test that nothing in 1.2 ever sets (left out, no
    behaviour); 0x1003db56 (1.2 0x100312c6) scans 10 network player keys instead of 8 (left for the network round).
    The key handler's 1.2 changes (command 0xc5) were already ported in round 33.
  - Force feedback: Project now has 1.2's FFORCE.DLL, whose TSFF_SIM_Effect takes 1.2's table (0xc0 bytes, 18
    effect records with "changed" bits), not 1.0's (0xe0). The library is found, initialised, sent the table each
    frame and closed by the same code in 1.0 and 1.2 (0x10045d70 / db9 / ec2 / e5d = 1.2 0x10038130 / 179 / 282 / 21d,
    no logic difference), so only the table and its writers change. Ported: 1.2's request 0x100382fb as
    Sim_100382fb (src/sim/unsorted_1004/w2k_flags.c), its table kept at 1.0's 0x1013af00 with 1.2's field offsets;
    the size dword 0xc0 is set once by the hook layer at sim load (W2k_FfTable12, hooks_conv.c; 1.2's is static data).
    Calls: the missile launch Sim_100d5446 now asks Sim_100382fb(kind, 0, 0, 0) only while the library flag
    0x1015ef3c (1.2 0x10131718) is set (1.0: Sim_10045f3b(kind, 0x2d, 0x32) always); the shot Sim_100232d7 (1.2
    0x10019f1d, its only logic change) sends a jolt Sim_100382fb(0xf, the fmod'ed heading, 0x32, 0) when the own unit
    is hit and the flag is set; 1.2 also wraps a copy of the heading and never reads it (left out). Round 2 ports the
    calls in gear movement and the unit update. Still 1.0's: the near-blast call in Sim_100246b3 (1.2 0x1001b3a1, 7
    calls inside a larger rework, effects round); with a force-feedback stick its 1.0 record lands on 1.2's jolt
    record (+0x48 flag, +0x4c angle, +0x50 strength). Without a force-feedback stick (Diego: no joystick support)
    TSFF_InitSystem fails, the flag stays clear and none of this runs.
  - Checker: spec_ff_100382fb.py (1.2's request, 4000 trials), spec_w10_100d5446_v12.py (4000, flag random) and
    spec_w29_100232d7_v12.py (new, 6000; float stubs at their 1.2 twins, 1.2's never-written segment seeded at its
    ebp-0xe0, the weapon table window 4 * 0xd6): all exact; planted bugs caught (FF_NEG / W2K_FF_MUTATE, the 1.2 flag
    forced on for both call sites). The 1.0 specs spec_w10_100d5446.py (4000, flag set, 1.0's call mapped to the C's)
    and spec_w29_100232d7.py (20000, flag clear) still pass.
- **Round 2 of the gameplay port: gear movement and world AI (round 33, Diego's go).** Analysed with
  tools/v12/fdiff.py (1.0 / 1.2 listings aligned with registers and addresses blanked, layout-only blocks hidden)
  and the sort thread's tools/v12/unsorted.md; every 1.2 change of these functions ported line by line:
  - Gear movement (Sim_1006e22c = 1.2 0x1005d75c, w16_move.c): the object's height is the body height +0x68 unless
    [0x10179ce0] (1.0: +0x6c); the jump jets are also cut when unit +0x560 bit 0x20 is set; four force-feedback
    requests (Sim_100382fb, the controls thread's port, only with [0x1015ef3c]): the contact bump kind 0xf after both
    contact sounds and the lean kind 0x11 (0 or 0xb4) for the player.
  - **Bug fix (Diego, 2026-10-31: "Fix it"):** 1.2's contact bump pushes its angle (0x1006f4ee, a double) without
    converting it, so the request (kind 0xf: a = angle in degrees 0..0x167, b = strength capped at 100, c unused) gets
    the double's low dword as the angle and its high dword as the strength (capped to 100), and 100 as c. The push
    order shows the intended call (0xf, angle, 100, 0), the form of 1.2's other kind-0xf call (the shot's jolt
    (0xf, (int)angle, 0x32, 0)); ours passes that: the angle truncated to whole degrees, strength 100. Was
    reproduced for a while (round 2's first build, 20:23). spec_w16_move_v12.py logs 1.2's call in the fixed form
    (its double truncated) and compares the rest exactly (6000 trials; W16MF_MUTATE, strength 50, caught).
  - World loader (Sim_100dcc12 = 1.2 0x100c1152, w15_world.c): the VIEW chunk's second value two and a half times
    ([0x1014ca40]); network only: tanks as GREL infantry (AFTER_DLL.md's network list). Needed 1.2's _strlwr
    (0x100ea880), ported as Crt12_StrLwr (w7r_ctype.c); its locale branch uses 1.0's mapper 0x101053c0 (1.2's takes
    one more argument); the game never sets a locale.
  - Target selection (Sim_10016632 = 1.2 0x1000f7d8, w9d_select.c): network only, the own list entry is not held.
  - Action gate (1.2 0x10010cd2 with its tests 0x10010bb4 / 0x10010bf4 / 0x10010c72 and setter 0x10010cb0; ours
    Sim12_10010cd2 in w2h_objects.c): replaces 1.0's early-out Sim_10017a05 in the unit actions Sim_10011681
    (w8t_actions.c), with the opposite sense. A locally driven gear (+0x14 2, or 3 and the player's) does not act
    while its jets are cut (+0x560 bit 0x20) or it is busy (+0x562 & 0x60e); busy with bit 8 and jets not cut sets
    control byte +0x55. 1.0 let it act in that last case and did not test the jets. Sim_10017a05 stays (no caller).
  - Unit update (Sim_10070eb5 = 1.2 0x10060491, w14_unit.c; round 2 owns the file, tools/dev/claims.txt): the
    player's force-feedback rumble (kind 0x10, strength pitch * 1.25 + 30 with the jets, + 15 without) after the
    speed; the start-up sound only for the player's own unit (1.0: unless Sim_10052be7 answered 1), with the
    force-feedback kind 0xe. The pitch starts at 0 as 1.2's local does.
  - Nothing to port (layout only): groups 0x10017263, ranges 0x10066907 / 0x100689c0, the target point 0x1000f0f4,
    the steering step Sim_100c0471. Not round 2: the C runtime routine 0x101039a0 (1.2 compiler library), and
    Sim_1009fada's render mode 0x12 (1.2's mode numbering, translated by the renderer hand-off).
  - Checker (all exact against 1.2, planted bugs caught): spec_w9d_select_v12.py (3000), spec_w15_world_v12.py
    (3000; 1.2's gear-type function tables mapped to 1.0's thunks, the removed gear field +0x18 started at 0),
    spec_w16_move_v12.py (6000; 1.2's ground normal hook, the collision answer's globals mirrored, the C library's
    math state and errno windows), spec_w2h_gate_v12.py (4000), spec_w8t_actions_v12.py (3000),
    spec_w14_unit_v12.py (6000). Negative checks: W9DN, W15V / W15T / W7RL, W16MY / W16MJ / W16MF, W2HG, W8TG,
    W14UF / W14US. w10_common.py's float stubs also hook a 1.2 twin given in FSTUB12. These functions follow 1.2 in
    every mode, so their 1.0 specs now differ where 1.2 does; the v12 specs are their proof.
- **Round 3 of the gameplay port: mission flow (round 33, Diego's go).**
  - Mission won (Sim_1007c42f = 1.2 0x1006932f, w9f_objectives.c): the own side wins as soon as its objectives are
    done; 1.2 drops 1.0's single-game [0x1015f1e8] test and the wait for pending script events ([0x1015d528]).
    spec_w9f_won_v12.py (3000; W9FW_MUTATE caught).
  - HGCAR.CFG in 1.2's 0x21c layout (Diego: "option 1", assemble on write): Sim_10046f7f writes 0x21c bytes (1.2
    0x1003951f); Sim_10025dac (1.2 0x1001cf2a) builds 1.2's record from our record's first 0x14c bytes, g_w34Kills
    and the two dwords +0x1cc / +0x1d0; the mission-start clears Sim_10021ac0 / Sim_10025d93 (1.2 0x100186d0 /
    0x1001cf11) also clear g_w34Kills. 1.2's shell reads hgcar.cfg with fread(0x21c) and ignores the result, so a
    0x1d4 file was read short (1.0's table taken for 1.2's, two dwords stale), not refused. AFTER_DLL.md: one record
    after the DLL. spec_w2e_hgcar_v12.py (save 1000, clears 300 each; W2ET / W5YH caught), spec_w8u_hgcar_v12.py
    (1000; W8UH caught; the path helper's buffers windowed). spec_w8u.py / spec_w32_debris.py: w33_dx12.c added to
    their extra sources (they had stopped linking when the 3DFX flag moved there; no logic change).
  - The in-mission / mission-end menu (1.0 0x10048f02, never converted) converted from 1.2's 0x1003ad72 as
    Sim_10048f02 (src/sim/unsorted_1004/w35_menu.c, hooked by conv_1004.inc): 1.2 no longer clears byte +8 of the
    game settings [0x1017b8c4] (twice in 1.0); the end phase's main choices are skipped while the pending flag
    [0x1015ef10] is set (its own branch then takes 0x4b / 0x6b / 0x76); the continue path calls 1.2's new reset
    0x10089c60 (Sim12_10089c60: [0x1015d1ac] = 0, [0x1015d1cc] = 1, [0x1015d098] = 0) before the respawn. The key
    test 0x10048c84 and sprintf stay original calls. spec_w35_menu_v12.py (4000; W35C / W35P / W35R caught).
  - The respawn (Sim_1009d200, w14_spawn.c) in 1.2's form (0x1008a560; Diego: convert it in round 3): no player
    record -> 0; the gear's +0x150 = 0x80000000; 1.0's network call Sim_1007caab dropped; the start-point check asks
    the collision probe Sim_1002dc60 (1.2's 0x10022890 is that same probe) instead of comparing every gear's
    position; a gear of a played side has its actions (Sim_10011093) and group selection (Sim_10016632) redone; a
    computer gear whose group leader is a remote player's gear is handed to that group (Sim_100526e3, Sim_100128dd,
    message 0x389 for the local player). Uninitialised read checked: only with a start-zone count of 0, which
    divides by zero first (both versions) and the zone loader never makes; deterministic. 1.2's two slips in the
    start-point loop (the probe at the start point's heading instead of its height; no retry when occupied):
    fixed, Diego's choice 2026-10-31 ("Fix both"; the options were a: 1.2 exactly, b: fix both, c: fix the retry
    only): the probe at the start point's height (the y the side matrix gives) and the next point tried
    while occupied, as 1.0's loop did. W14S_FIX_HEIGHT / W14S_FIX_RETRY (w14_spawn.c) flip them; both 0 = 1.2
    exactly. spec_w14_spawn_v12.py compares the build with both off against 1.2 (6000; W14S_MUTATE and each fix
    switch differ from 1.2, so both loop paths run in the spec).
- **Round 4 (weapons, damage, HUD, animation), decided ahead of the port (Diego, 2026-10-31: "Fix: send the real
  direction"):** 1.2's blast impact (Sim_100246b3 = 1.2 0x1001b3a1) computes, in each of its 7 blast kinds, the
  direction from the player to an impact within 2000 (0x100b0d4b) and then sends the force-feedback request kind 0xd
  with the fixed angle 90 (0x5a), strength 50 (0x32), discarding the direction. Ours will send the computed direction
  instead of 90 (bug fix, like the movement bump's angle). To apply when round 4 is ported (no go yet); its v12 spec
  will log 1.2's call with the direction in place of the 90 and compare the rest exactly.
- **Voodoo look for Vulkan (Diego, 2026-10-31: "keep the name. full voodoo. no fog").**
  - Deviation: 1.2 sets its 3DFX / PowerVR mode (g_w33Voodoo, 1.2 [0x1014b954]) only when the renderer's path holds
    "3DFX" or "POWERVR". Our sim main (w21_simmain.c) also sets it for "HGVK" (our Vulkan renderer keeps the name
    HGVK.DLL). Every 1.2 Voodoo branch ported in round 33 now runs with Vulkan: the coloured vertex light, the .TEX
    textures, the sorting of the last group, the fog state codes, the 565 HUD screen (HGVK's is 565 already).
  - HGVK's colour combine follows 1.2's 3dfxgear.dll exactly (its Render switch 0x10001590, mode table 0x10001b24;
    grColorCombine / grAlphaCombine per case). In the sim's 1.0 mode numbers: 0..4 texture x vertex colour (as
    before); 5..11 texture only, the blended ones (5, 6, 10, 11) with the texture's alpha; 12 the constant colour of
    the call's colour argument (+4); 13 vertex colour; 14 / 15 vertex colour smooth, blended by its alpha. HGVK is
    only ever the Voodoo renderer (option 3 draws with 1.2's dx5draw), so no switch. The sim draws with modes 1, 2,
    6, 13, 15 and 16: in game the change is mode 6 (effects / particles: texture colour and alpha only, no vertex
    tint or vertex fade).
  - 1.2 oddity, raised: 3dfxgear's line and point modes (1.0 16..18) set no combine of their own and keep the last
    polygon's (after a mode-6 effect a line would take texture-only mixing with whatever texture is bound; the sim's
    lines are the spark sprays, Sim_1009fada, and the models' line polygons, kind 0xa000). Diego's choice 2026-10-31
    ("Keep their own colour"): HGVK draws lines / points in their own vertex colour.
  - No fog (no shader compiler): the per-vertex fog factor and FirstDevice's fog switch stay carried and unused.
  - **Sky over the scene in Voodoo mode (Diego, 2026-10-31: "the sky renders on top of the scene is back"),
    fixed in HGVK.** Cause (his 23:56 hgvk.log and F12 dump): the cloud layer (Sim_100a0880 = 1.2 0x1008da61, drawn
    by 0x100a0339 = 1.2 0x1008d51a) is a textured card from z 500 to 40000 (rhw = camera scale / z, 0.008..0.64),
    nearer than the hills it hangs over (the frame's ground at z ~311000). Before Voodoo mode it went as mode 6,
    blended, which writes no depth, so the world drawn after it covered it; 1.2's Voodoo branch sends it as mode 2,
    opaque, so it wrote its near depth and hid them. Checked against 1.2: the sky routine (0x1008dff3) sends state
    0xc (our 0xd; in 3dfxgear SetState 12 = a depth-only grBufferClear) and then the clouds, every frame, with
    3dfxgear's W-buffer (FirstDevice: grDepthBufferMode(2), LEQUAL, depth writes on, never switched per mode) and
    GrVertex oow = the vertex rhw. Note: by that arithmetic a real Voodoo would also put the cards in front (its
    depth 1/rhw is z / 320 for the clouds, z for the world); not confirmed on hardware. Fix (renderer boundary):
    HGVK draws the one Render that follows state 0xd (the cloud layer, the only sender) in painter's order, no depth
    test or write, and keeps its depth out of the last-depth fallback: the sky behind the scene, as it was drawn
    before Voodoo mode. No sim change; no spec affected.
    Diego's choice (2026-11-01, option a): keep the cloud layer behind the hills, as built (not the arithmetic of a
    real Voodoo, which would put it in front of far hills).
    **Correction (2026-11-01, sweep):** the premise was wrong. 1.2's frame step (0x1002a195 = Sim_100352d5) sends
    state 0xc (our 0xd, depth clear) again after the background and sky, before the world - so on a Voodoo the sky
    is behind the scene. Ported (sweep, graphics); HGVK now clears its depth buffer on state 0xd (Vk_ClearDepth, a
    queued vkCmdClearAttachments), as 3dfxgear does, replacing the stopgap above (the Render after state 0xd drawn
    without depth). Same picture, now 1.2's own way; Diego told.

## Round 33, round 4: weapons, damage, HUD, animation (Diego, 2026-11-01: "go go go")

Each change is ported from 1.2's code and checked against 1.2 with the checker (a v12 spec, 1.2's run compared with
the C's); each has a negative check (a planted 1.0 behaviour) that the spec catches.

- **Blast impact force feedback** (Sim_100246b3 = 1.2 0x1001b3a1, w10_effects.c): 1.0's one near-blast request before
  the switch (its distance from the own unit's +0x58 for all three axes, under 3000, kind 0 through Sim_10045f3b) is
  gone; 1.2 makes it in each blast kind's 0x400 branch (kinds 59 / 61-72, not 5): with the force-feedback library
  ([0x1015ef3c]) and the impact within 2000 of the own unit's object (+0x58 / +0x5c / +0x60), request 0xd through
  Sim_100382fb with strength 50. Bug fix (Diego, 2026-10-31: "Fix: send the real direction"): 1.2 computes the
  direction (Sim_100cbc6b) and sends the fixed angle 90; ours sends the computed direction (W10E_FIX_DIR, default 1).
  spec_w10_blast_v12.py (built with W10E_FIX_DIR=0): 6000 trials, 0 mismatches; distances around 2000 / 3000 included;
  W10E_NEG12=1 (1.0's 3000): 50, =d (the fix on): 108 (every request sent). Only felt with a force-feedback stick.
- **Weapon mount slot** (1.2's gear field +0x5fb; correction 2026-11-01: not the selected weapon, which is +0x4fc -
  +0x5fb is the mount index of the gear's kind-10 part, the node the weapons hang on, +0x498 + 4 * index): the gear set-up Sim_1005bb7a (1.2 0x1004e6a0) starts it at -1
  right after +0xc15 = 0 (the kind-10 part still sets its mount number later, as in 1.0); the weapon mount
  Sim_10038ad5 (1.2 0x1002cdc5; the survey's "new" function is its 1.2 version) makes the slot it fills that mount
  index when +0x5fb is -1 (a gear without a kind-10 part) (the record's +0x128 and +0x5fb), else copies +0x5fb as 1.0. spec_w2e_mount_v12.py 3000 / 0
  (W2EW_NEG12: 1419); spec_w14_pof_slot_v12.py: the set-up with the store left out still matches 1.0 exactly (10000 /
  0: nothing else in it changed; 1.2's other differences there are re-packed tables of the same values and a debug
  print through its empty log function), with it in only +0x5fb differs (7426 of 10000).
- **HUD: nothing to port.** The text tables (Sim_100204c4 = 1.2 0x100177d4): both copy the same 316 text pointers
  (parsed from both DLLs and mapped address by address) - the survey's "46 more lines" was the address shift. The
  "new formatter" 0x100b2a4a is 1.2's version of the target panel 0x100ce983 (panel 16's draw slot +0x13c, set by
  1.2 0x10020fc0 / 1.0 0x1002c017); with the side panel 0x100ce586 (1.2 0x100b26e6) and the F12 objectives screen
  0x100cde94 (1.2 0x100b21ce) they differ from 1.0 only by drawing each line through a new helper (1.2 0x100b1340:
  the same three-way choice of text routine as 1.0) and 1.2's kill-table ratings (ported in round 3). Checked:
  spec_w23_100ce586_v12.py 4000 / 0 and spec_w23_100cde94_v12.py 4000 / 0 against 1.2 (negatives 43 / 11); the
  target panel by side-by-side reading (its spec framework, spec_w9e.py, cannot run against 1.2): the same texts,
  colours and positions; 1.2 only leaves the colour byte at 0xc between lines that all set 0xc. Diego's choice (2026-11-01,
  option a): this reading is accepted; no emulator spec rebuilt for it. 1.2 no longer reads
  the player's gear block in the side panel (1.0 read it unused): dropped.
- **Weather emitters in 1.2's particle pool (a bug fix of our build).** The nine emitters a mission's world file can
  set up around the camera (Sim_100d82f6) spawn through Sim_100d9064 (style 1) / Sim_100d965e (style 2) with the
  callbacks 0x100d8d0a / 0x100d8e43 / 0x100d9306 / 0x100d943f. Round 32 put 1.2's 0x406-byte records into the effect
  pool in particle mode (Vulkan, 1.2's Direct3D), but these kept 1.0's offsets (up to +0x576 and 32 points of 0x2a
  bytes): in a mission with these emitters they wrote past their record into the next one. 1.2 (0x100be504 /
  0x100beafe / 0x100be1aa / 0x100be2e3 / 0x100be7a6 / 0x100be8df) uses its layout: node +0x332, time +0x33e, state
  +0x346, velocity +0x34e, argument +0x366, 16 points of 0x32 bytes; nothing else changed. New macros W32_REC_TIME /
  STATE / VEL / PT / NPT (fx/w32.h). spec_w10_weather_v12.py, in particle mode: both spawns 2000 / 0 against 1.2
  (W10W_NEG12, 1.0's 32 points: 822 each); the callbacks differ from 1.2 only by the layout (fdiff: displacements and
  the point stride). spec_w10_100d965e.py (1.0 mode) still passes (2000 / 0).
- **Checker housekeeping:** spec_w2e.py, spec_w6h.py and spec_w8z.py no longer link (their fixed source lists predate
  rounds 32 / 33: W31_TexMode, the railgun / accelerator systems, g_w34Kills); the new v12 specs build their own
  closures. Diego's choice (2026-11-01, option a): fixed in the tiny sweep (listed in tools/v12/unsorted.md).

## Round 33, music from the MUSIC folder (music thread; recorded 2026-11-01)

- **Music as 1.2's WAV music, with Ogg files** (Diego, 2026-10-31: "The music cd will instead be played from the
  music folder ... Ignore the cd"): CD track N plays MUSIC\TrackNN.ogg (src/platform/music.c, stb_vorbis, an SDL3
  stream voice). The game side is 1.2's WAV-map code ported line by line (start 0x10015e83, restart of a finished
  track 0x10015fbd, pause 0x10015f1b, resume 0x10015f59, stop 0x10015f97, volume 0x1001603f, option read / set
  0x10015450 / 0x1001550d), checked by tools/verify/spec_music_v12.py; notes in tools/v12/sound.md. 1.2's WAV player
  itself (wavmap.ini, .wav reading through mmio, the DirectSound buffer and its winmm refill timer, about 34
  functions) is not ported: the Ogg player does its jobs (Diego, 2026-11-01: option a, "keep the OGG music").
- **Deviation (1.2 bug not kept):** 1.2's music-option setter sets the music volume only while the CD is open, so
  with WAV music the volume option never changed the music's volume. Ours follows the option for the files too, as
  1.2's CD path does (the spec compares the option setter with 1.2's CD flag set).

## Round 33, tiny-sweep choices (Diego, 2026-11-01)

- **1.2's C-runtime copies not ported** (option a, "We will replace them all at the end."): about 90 routines of the
  Microsoft C library compiled into 1.2's HGSIM (a newer compiler; no game change). The converted code keeps using
  1.0's copies or the standard library (exactness rules above) until HGSIM.DLL goes; then every C-runtime call is
  replaced, each replacement checked against 1.2's C-runtime results (AFTER_DLL.md).
- **1.2's WAV player not ported** (option a): see the music section.
- **No PowerVR support** (Diego, 2026-11-01: "I don't want powervr support", option a). Checked in 1.2's HGSIM: the
  string "POWERVR" (0x1011e0c8) has one use, sim main 0x1006ffd2, setting the shared 3DFX / PowerVR mode flag
  [0x1014b954] (our g_w33Voodoo); there is no separate PowerVR-only flag or branch (the "PowerVR state toggles" of the
  survey are that shared mode's state 0xe, already ported with Sim_1004954e), and 1.2 ships no PowerVR renderer
  (its plugins: 3dfxgear, dx5draw, i76draw, renddraw). The POWERVR name test in our sim main stays (1.2's line).

## Tiny sweep, controls (Diego, 2026-11-01: "do controls and ai"; AI moved to its own thread)

Each function of the survey's input / controls rows was diffed against 1.2 (tools/v12/fdiff.py) and read:
- **Order-target helper** (Sim_10015018 = 1.2 0x1000e1a8, w7l_misc.c), the "next unit" selector 0x4200: 1.0 also took
  a unit whose state (Sim_10052be7) is 0 when [0x1015d110] is set - nothing ever writes it (one read in the DLL,
  initial 0). 1.2 tests its network flag (1.0 [0x1015ec58]) or its [0x1012d4e8] (likewise never written, initial 0).
  Ported: in a network game that unit is taken too. spec_w7l_order_v12.py 4000 / 0 (only ax compared, a new difftest
  option `retmask`; 0x1015d110 left out of the copied globals, the listing pairs it with 1.2's network flag);
  W7LO_NEG12 (1.0's flag): mismatches.
- **Cache entry** (Sim_100b9b3f = 1.2 0x100a1f1f, w8y_cache.c): the same code; its five out-of-memory asserts give
  source lines 2 higher (0x2f1 -> 0x2f3, ...). Ported (constants only; the assert fires only when an allocation fails).
- **Layout only, nothing to port** (the 4-byte record shift written as arithmetic, fdiff's only difference): steering
  0x100139ff (1.2 0x1000cb8f, +0x228 -> +0x224), the key test 0x10048c84 (0x1003aaf4, +0x124 -> +0x120), the two
  control states 0x100c145b / 0x100c1609 (0x100a8c28 / 0x100a8dd6, +0x224 -> +0x220).
- **Already in 1.2's form:** the textured submit 0x100d1e10 (1.2 0x100b7ff0: its mode 8 is our 1.0 mode 6, its 7-dword
  description: round 31).
- **Left for the network round:** the player-id lookup 0x1003db56 (1.2 0x100312c6) looks at 10 ids instead of 8 (part
  of 1.2's 10-player protocol).

## Tiny sweep, AI (Diego, 2026-11-01: "do controls and ai", "start the ai in a separate thread")

The four AI-side functions of the sweep, each diffed against 1.2 and run against 1.2's code in the checker:
- **Group orders** (Sim_10017263 = 1.2 0x10010412, w9h_group.c): layout only, every difference is the unit record's
  4-byte shift (+0x28 -> +0x24, +0x1a0 -> +0x19c, +0x1a6 -> +0x1a2, +0x1aa -> +0x1a6, +0x1ae -> +0x1aa). Nothing to
  port. spec_w9h_group_v12.py (unit records shifted for 1.2's run, the selected star's entries copied to 1.2's table,
  Sim_10016d83's word-list pointer logged by its contents): 5000 trials, 0 mismatches; W9HG_NEG (kind 4 taken as 5):
  90 mismatches.
- **Range test** (Sim_10066907 = 1.2 0x100571d7, w16_range.c): layout only (+0x1c -> +0x18, +0x28 -> +0x24, +0x58 ->
  +0x54, +0x84 -> +0x80, +0x29c -> +0x298). Nothing to port. spec_w16_range_v12.py: 12000 trials, 0 mismatches;
  W16R_NEG12 (box flag 2 taken as 1): 603 mismatches.
- **Maths helper** (1.0 0x101039a0 Math_FrExp = 1.2 0x100e1cf0, w7r_math.c): the C runtime's frexp. 1.2's copy only
  reorders instructions and adds a `movsx esi, si` after `and esi, 0x7ff`, which cannot change the value: same
  results. Not ported (Diego's rule for the C runtime: replaced at the end).
- **Sparks** (Sim_1009fada = 1.2 0x1008c922, w22_poly.c): the only difference is the render mode, 1.2's 0x12 for our
  0x10 (1.2's renumbering, round 33 "Render modes renumbered in 1.2"). Nothing to port. spec_w22_sparks_v12.py (bit 1
  of the mode and the state block's address masked in the log): 4000 trials, 0 mismatches; W22_NEG: 154 of 300.

## Weapon selection after a burst (Diego, 2026-11-01: option b, "keep my new selection")

- **Bug fix of a 1.2 / 1.0 behaviour:** the per-frame weapon handling (Sim_100d4590 = 1.2 0x100ba550, w10_weapons.c)
  makes every slot that fires a burst shot the current weapon (+0x4fc). A player who switched weapons while the old
  weapon's burst was still running was switched straight back; and since a finished burst refires by itself when its
  slot is the current one (state 3 -> 2), the old weapon kept firing. Now (W10F_KEEP_SEL, default 1) the local
  player's gear keeps its selection: the running burst finishes, the old weapon then stops unless the trigger is held.
  Computer gears keep 1.2's behaviour (their weapon choice and aim lead read +0x4fc).
- **Checked:** spec_w10_fireloop_v12.py against 1.2: with W10F_KEEP_SEL=0, 15000 / 0; with the fix (W10F_NEG12=fix,
  1.2's result given back the player's old +0x4fc), 14 of 15000 differ, all in that refire decision of a slot that is
  no longer current (expected); the fix applied to every gear (W10F_NEG12=1): 1534. The 1.0 spec with
  W10F_KEEP_SEL=0: 15000 / 0. Found while checking: a current slot without a weapon reads "kind -1", the 0xd6 bytes
  before the kind table, in both versions (the spec copies them too). Nothing else reads +0x4fc in the shot path
  (the fire functions get the slot and weapon record), and no network message carries it.

## Tiny sweep, graphics and sound (Diego, 2026-11-01: "do the graphics and sound next")

Every candidate was diffed against 1.2 (tools/v12/fdiff.py) and read in 1.2's code:
- **Depth clear before the world** (Sim_100352d5 = 1.2 0x1002a195, w9h_view.c): after the background quad or sky,
  render state 0xc (our 0xd) unless [0x1014cb1c] is 1. Ported (W31_TexMode). HGVK: state 0xd now clears the depth
  buffer at that point of the frame (Vk_ClearDepth: a queued vkCmdClearAttachments; the last-depth fallback reset), as
  3dfxgear's SetState 12 does; the round-33 stopgap (the Render after state 0xd drawn without depth) is gone.
  spec_w9h_view_clear_v12.py 3000 / 0 (1.2's state codes mapped +1); W9HD_NEG12 (no clear): 1669.
- **Backdrop shapes at the far end** (0x100a291d / 0x100a29da, w3q_driver.c = 1.2 0x1008fc0f / 0x1008fce8;
  0x100a2823, w6g_misc.c = 1.2 0x1008faed): each vertex's z is 3.4e38 (0x7f7fc99e) in 1.2's mode, as the sky band.
  spec_w3q_far_v12.py: 1500 / 0 each (the fields 1.2 writes compared); W3QZ_NEG12: all mismatch. HGVK treats both
  z values as a backdrop, so no visible change there; 1.2's Direct3D gets the far depth.
- **Sound banks** (0x1002009d stop = 1.2 0x100173ad, w5x_calls.c; 0x100200d1 queue = 1.2 0x100173e1, w7l_misc.c):
  1.2 accepts bank 3. Inert in 1.2 itself: bank 3's sound index (1.0 0x10133710, 1.2 0x1010a188) is never written in
  either DLL (-1: nothing queued), and nothing calls the stop routine; 1.2 also inserted an all-zero record 3 into
  its bank table (1.0's record 3 is 1.2's 4). Bounds ported. spec_w7l_bank_v12.py 3000 / 0; W7LB_NEG12: 512.
- **Already in 1.2's form:** the animated-texture table and its users (0x10006a2f, 0x1000716a, 0x100072ff,
  0x100da14c: round T's 0x380 / 0x1c layout), the lighting field split (Sim_10032a10, lighting round), the sparks'
  extra argument (Sim_1006c161, round 32), the textured submit (round 31).
- **Nothing to port:** the WTB model reader (1.2 0x100c5e73 is identical; the survey's "polygon flag bit" came from a
  misaligned decode).
- **Dead in both versions, not ported:** the 1.0 effect spawners 0x10095616 / 0x10096541 and the pool user
  0x10091597 (their jump stubs are referenced nowhere), with their callbacks (e.g. 0x1009524d = 1.2 0x10080d38, part
  of 1.2's family 0x1008139b, which 1.2 never spawns either). So no further 1.0-layout writes into 1.2's pool.
- **Moved to the network round:** 0x1004170e (1.2 0x100354eb: a network collision packet, 1.2's "game ending" check)
  and 0x1005b98c (1.2 0x1004e4cc: armour scaling by difficulty; in a network game 1.2 doubles every gear's armour,
  1.0 quadrupled only the player's own and kind-1 gears) - both network-only.

## Tiny sweep, mission / platform and test files (Diego, 2026-11-01: go for the rest of the sweep)

- **The survey's mission / platform rows:** the window procedure (focus pause, ported: "Focus pause and cache size
  0"); Cd_Open 0x1009e1ef (1.2 0x1008b36f: its volume call goes through 1.2's music table, which ours already does
  through Win_AuxSetVolume with HG_MUSIC_FILES; nothing to port); the CD drive search 0x10099510 (below).
- **CD drive search** (1.2 0x10087460): the GameRip 1.0 HGSIM is a patched copy (two jumps NOPed, FindClose called
  in place of FindFirstFileA), which is why 1.0 always answered "found". 1.2 really looks for a CD-ROM drive holding
  \video\B51MOVIE.AVI. In our build nothing changes either way: SimMain skips it (1.2's own "no CD" flag, round 33),
  the movie folder search gets no files from it in 1.2 either (1.2 answers 1, not a drive letter), and the CD audio
  start is hooked off. Kept as ours (option A), under Diego's standing rule "NO CD EVER" (project memory; hooks_cd.c
  fakes the drive): 1.2's search is not ported. Diego can still ask for 1.2's search.
- **Rows re-read against 1.2:** zlib is dead code in 1.2's HGSIM (no caller of uncompress 0x100b7c40 or of the
  "ZLIB" reader 0x10026f40), so no data file needs it; the full-screen toggle is not new (1.2 0x1002c5a1 = 1.0
  0x10037f71, identical, already converted); the missile release 0x10024548 only gains a dead store in 1.2; the
  target readout 0x100c7120 is layout only. Nothing to port; tools/v12/remaining.md corrected.
- **The three 1.0 checker files** (round 33's housekeeping, option a): spec_w2e.py, spec_w6h.py and spec_w8z.py run
  their hand-written source lists through w10_common's closure() (in its own namespace, so their CFLAGS and helpers
  stay theirs); spec_w2e.py also gets -mno-stack-arg-probe for the added files' large frames. All pass against 1.0:
  spec_w2e 66 functions, spec_w6h 29, spec_w8z 36, 0 failed.
- **1.2's duels: no HGSIM change.** 1.2 adds no duel missions: the duel files (M51DUEN1/2, M51DUES1/2) and the
  shell's "Duel" / "Duelist" texts are all in 1.0. 1.2's patch folder changes one byte in M51DUES1.BWD and
  M51DUES2.BWD: the gear file GQGMAMBA (which does not exist) becomes GPGMAMBA (in HEAVYG.PRJ). That matches the
  readme's "Fixed Error 55 in Duel missions" and is data only; neither sim has a duel-specific text or branch.
  Project already has 1.2's two files (byte for byte) and runs 1.2's HGSHELL.

## Sound round (Diego, 2026-11-01: "do the 4 sound functions")

The four HGSIM sound functions 1.2 changed (tools/v12/remaining.md, Sound), read in 1.2's code and ported line by
line onto the SDL3 sound (src/sim/sound/sdl_sound.c):
- **Sound load** (SndSdl_1000e0d9 = 1.2 0x10007bf9): when the new voice cannot be made, the Lock, the copy and the
  Unlock are skipped (1.2 0x10007f0e); 1.0 went on with a null voice. spec_snd_1000e0d9_v12.py 3000 / 0 (a quarter
  of the trials fail the voice); SND_NEG12 (1.0's code): 139.
- **Voice update** (SndSdl_10039c00 = 1.2 0x1002dc30): a finished or stopped voice is released until its reference
  count is 0 (1.2 0x1002dec9 / 0x1002e021, for duplicated DirectSound buffers). An audio.h voice has one owner, so
  the loop ends after one Audio_VoiceDestroy. spec_snd_10039c00_v12.py 3000 / 0 (3D flag off: 1.2's 3D functions
  differ by the unit layout only); SND_NEG12 (two destroys): 522.
- **Shutdown** (Sim_1003a052 = 1.2 0x1002e0a6, unsorted_1003/w5c_calls.c): 1.2 frees its music stream before the
  device shutdown; ours closes the MUSIC-folder player there (Music_Close moved out of SndSdl_1003a548, which 1.2's
  0x1002e655 matches again). spec_w5c_3a052_v12.py 2000 / 0 (built with HG_MUSIC_FILES); W5C_NEG12 (no close): 2000.
- **Sound start** (SndSdl_1003a0e4 = 1.2 0x1002e159): 1.2 now always makes the primary buffer, creates its music
  stream and sets the primary format to 22050 Hz 16-bit stereo for the music. None of this has an SDL3 counterpart:
  our mix is 48000 Hz float stereo and the Ogg player needs nothing made at start. No logic change; the comments say
  what 1.2 does. spec_snd_1003a0e4_v12.py 1000 / 0 (1.2's primary buffer, SetFormat and stream answered silently);
  SND_NEG12 (block +0 not set): 755.
- **3D sound: not new in 1.2.** The listener / 3D-buffer code (1.0 0x100397e7 / 0x10039931, the 3D primary buffer
  in the start) is the same in both versions and runs only when HGSND.CFG's word +0x40 is set, which is 0 in every
  copy (1.0, 1.2, Project). The word is the shell's 3D sound menu choice: its list (HGSHELL 1.0 0x10161d86) is Off,
  Headphones, Mono, Quad, Stereo, Surround Speakers, the order of DirectSound's speaker codes 0..5, and the start
  hands the word to SetSpeakerConfig (the shell's write of +0x40 itself not traced). So any choice but Off turns
  the original's 3D sound on; ours ignores it. With it off, the game sets each sound's volume from its distance and its pan from its
  direction (Sim_1000eb12, Sim_1000e737), which is what plays today. Our 3D stays off (see "What is not exact" under
  the SDL3 sound mapping). Diego was given the choice on 2026-11-01: (A) keep it, recommended; (B) an own small
  spatializer for the 3D word; (C) a library such as miniaudio. **Diego (2026-11-01): A**, "keep it as it is for
  now, will add support for modern alternatives onces the dlls are gone" (AFTER_DLL.md, 1.2 sound).
- The 1.0 specs of the changed functions still pass (spec_snd_1000e0d9 3000 / 0, spec_snd_10039c00 3000 / 0,
  spec_snd_1003a548 500 / 0).

## Network round (Diego, 2026-11-01: "Start the network round")

- **Transport (Diego, 2026-11-01): "we want to make the multiplatform, can't use directplay".** The transport becomes
  our own (no DirectPlay / ANET2.DLL; no play against the original 1.2 game). The sim side keeps 1.2's message format
  byte for byte, so this round does not depend on the transport: the handlers build and parse message bytes and send
  them through Net_DpSend_1007e950 / Net_DpSend_1007e920, which call DirectPlay through function pointers today.
  Project already runs 1.2's HGSHELL.DLL and ANET2.DLL (the same files as the 1.2 rip), so the shell hands the sim
  1.2's 10-player game record.
- **Player lookup by network key** (Sim_1003db56 = 1.2 0x100312c6): 10 keys (1.0: 8). spec_net_lookup_v12.py 3000 / 0
  (planted bug NETL_MUTATE, 8 keys: 246).
- **Armour in a network game** (Sim_1005b98c = 1.2 0x1004e4cc): every gear's 17 part records x2 (1.0: x4 for the own
  gear and the kind-1 gears, others untouched); single player unchanged. spec_net_armour_v12.py 3000 / 0 (planted bug
  NETA_MUTATE, 1.0's rule: 1986).
- **Game-over guard in twelve message handlers** (1.2's receive guard 0x101314c0 = g_w34NetOver, set by the message
  loop from the game-over flag before each message; tested first, message ignored while set): "CH" Sim_1003ff72,
  "WE" Sim_1004044c, "CA" Sim_100405e6, "AI" Sim_1004094e, "DS" Sim_10040a37, "NR" Sim_10040f2d, "OR" Sim_100410ff,
  "DR" Sim_1004131b, "CO" Sim_1004170e, "EG" Sim_10041a16, "SN" Sim_10041b2d, "PU" Sim_10041247. g_w34NetOver moved
  to its own file (src/sim/unsorted_1004/w34_netover.c; tools/verify/rt.c defines it for the checker builds with
  hand-written source lists). spec_net_guard_v12.py (NETG_PART per handler) 1500 / 0 each; planted bug NETG_MUTATE (no
  guard) caught in every one (21..495). The "TC" receive Sim_1004084a and the "DM" damage message Sim_10040191 have the
  guard too and more 1.2 changes: below.
- Checker notes: 1.2's unit records lack our dword +0x18; "CA" (and 1.0's "TC" / reset) clear it in ours, so the
  spec keeps it 0. The "DS" helper's second argument is compared as a word (both leave the register's upper half).
- **Open choices sent to Diego (2026-11-01):**
  - **"TC" / "DI" damage handover, a 1.2 bug in three places:**
    - 0x10033c90 writes the "DI" tag into buffer [0x10131500] but sends buffer [0x10131504], so the message goes out
      with an empty tag and every receiver drops it.
    - The "DI" receive 0x10033d7b tests `(unit+0x14 == 0) == 3`, which is never true.
    - The "TC" receive 0x10034468 unpacks the 17 part records (+0, +4, +8 each, via 0x10034342) from msg+0x23,
      past the end of the 0x21-byte "TC" message, so it reads stale receive-buffer bytes. Handed-over gears get
      garbage armour values.
    - Options:
      - A: exactly as 1.2.
      - B: 1.2's intent, fixed (recommended).
      - C: no damage handover.
  - **Scope:** the survey missed most of 1.2's network module: about 60 functions, with the loop's timed part
    rewritten into about 15 functions the survey did not see. The 10-player change also needs 7 per-player tables
    moved to 10-entry C tables. The 1.0 tables: 0x1015db00 / db20 / db40 / db80 (dwords), d1b0 / d090 (bytes),
    da40 (0x16-byte rows, cleared 0x11 bytes from +5; 1.2 clears 0x16). The 1.2 tables: 0x107cab60 / caba0 / cb440 /
    cb4a0, cb4d0 / cb430, caa80. Two of their users are still original code: "DA" 0x1003f3f8 = 1.2 0x10032cbe and
    the player-list callback 0x10042125 = 0x10035f3e. Options: A, port the whole 1.2 network module now
    (recommended); B, leave the rest for the transport rewrite.
  - **Transport (read-only analysis):**
    - The "dp" functions are ANET2.DLL's own API (Activision's library, transports in the anet folder), not
      Microsoft DirectPlay.
    - HGSIM loads 55 and calls 8, all from our C: send, receive, enum players, number of players, report score,
      connection callback, enum servers, debug print.
    - HGSHELL makes 61 calls into 29 ANET2 functions, plus 14 WSOCK32 calls in two places.
    - Options: (1) our own ANET2 stand-in on OS sockets behind the platform layer, with the shell's imports patched
      as for DirectDraw (recommended); (2) the same on SDL3_net (a download); (3) convert the shell's lobby first.
- **Until scope option A:** the 10-key lookup (above) lets players 9 and 10 get indices whose per-player entries
  overflow 1.0's 8-entry tables. It lands safely only together with the 10-entry tables. Not a concern before
  network play exists on our transport, but the tables must move before any 9- or 10-player game.

## Focus pause and cache size 0 (Diego, 2026-11-01: "port both but add fixes to bugs")

- **Focus pause** (Sim_100867a2 = 1.2 0x10071397, src/sim/platform/w28_window.c, message 0x1c): as in 1.2, losing
  the window's focus pauses the game (Sim_1008b8de: the clock's flag 0x80 and the sound pause Sim_1001f4ba, which
  pauses the music), and getting it back resumes it (Sim_1008b900) before the key state is cleared. Before, the
  mission clock and the music ran on while the game was in the background (frames were already skipped).
  - **Bug fix of 1.2:** the pause shares the clock flag with the HUD menu (Sim_1008b78f) and the clock's own hold
    (Sim_1002a983). In 1.2, switching away and back while the menu was open started the clock and music again
    behind the menu. Now the focus code pauses only when the clock is not already held (timer flag 0x200 of
    [0x1015317c]) and not in a network game, remembers it (g_w28FocusPaused, cleared at each sim load by
    W28_InputReset), and resumes only its own pause.
  - **Checked:** spec_w28_window_v12.py against 1.2, 20000 / 0, 1.2's pause / resume calls brought to the fix's
    form by the spec from the trial's state, and the flag the C leaves checked by the spec; negative check
    (resume without the flag test) caught, 557 / 20000.
- **Cache size 0** (Sim_100ba304 = 1.2 0x100a26e4, w8y_cache.c, the CACHE.INI reader): 1.2's asserts give its own
  source lines (0x367, 0x37c, 0x39a, 0x3a3, 0x3aa).
  - **Bug fix of 1.2:** 1.2 keeps a type whose hash size is 0 with no slot table (+0x10 = 0), but filing or finding
    a file of it still divides by the size (Sim_100b9b3f, Sim_100baf3f and the other lookups, unchanged in 1.2),
    so 1.0's own CACHE.INI ("tex 0 0") would still crash at the first .TEX. Now a size-0 line is left out: the type
    is made with the defaults (0x400 slots, 0x100000 bytes) when its first file is filed, as for a type the file
    does not list. An earlier line of the same name keeps its record unchanged. We still ship 1.2's CACHE.INI
    (round 31), so this only matters for an edited or old file.
  - **Checked:** spec_w8y_reader_v12.py against 1.2: files without size-0 lines 4000 / 0 (644 both-fault: an
    allocation failure before a null write, as in spec_w8y.py); size-0 lines, with 1.2 given each such line
    rewritten to what the fix makes of it, 3000 / 0; negative check (1.0's assert line) caught, 105 / 4000.
    spec_w8y.py's 1.0 check of Sim_100ba304 no longer applies (1.2's lines, the fix).
- **Diego's choices (2026-11-01):**
  - **Damage handover: B**, "Fix it the way 1.2 meant". The fixes are behind a switch, NETDI_FIX (default on). With
    the switch off the code is 1.2 exactly; with it on, only the intended differences.
  - **Scope: A**, "Port it all now". All of 1.2's network module, function by function, each checked against 1.2
    with a planted bug.
  - **Transport: option 1.** Our own ANET2 stand-in on plain OS sockets (UDP) behind the platform layer; HG.EXE swaps
    the shell's ANET2 imports as it does for DirectDraw; 1.2's message format is kept. It comes after the 1.2 port,
    and its plan goes to Diego first.
- **The 10-player tables** (src/sim/unsorted_1004/w34_netover.c, declared in w3p.h; tools/verify/rt.c has copies for
  the checker builds): g_net10Heard / Db20 / Left / SnStamp / Mark / Ready / Row, plus g_net10DiSent and
  g_net10DiBuf for "DI". Every C user was moved to them.
  - **The network game start** (Sim_1003dc19 = 1.2 0x10031389): 10 players; clears the whole 0x16-byte row; clears
    the "DI" cache; allocates the "DI" buffer after the third block. 1.2's "AK" block stays our static g_w34AkBuf
    (round 33). spec_net_start_v12.py 3000 / 0; planted bug NET10_MUTATE (1.0's 0x11 bytes per row) caught 653.
- **"DA", a remote unit's state** (1.0 0x1003f3f8, original code until now; converted from 1.2 0x10032cbe as
  Sim_1003f3f8 in src/sim/unsorted_1003/w36_net.c). 1.2's changes:
  - The game-over guard.
  - No skip for a state-3 unit run by the local player.
  - A unit whose second part is under 1e-7 is marked.
  - Killers 0..9.
  - The 10-player tables.
  Its texts go into a 0x100-byte buffer instead of 1.2's 80 bytes. Two 44-byte names plus a text can exceed 80
  (DECISIONS round 13); the text shown is the same. spec_net_da_v12.py 3000 / 0; planted bugs caught: NETDA_MUTATE
  (no guard) 626, NETDA_MUTATE8 (1.0's killer range) 17.
- **"DI", the damage of computer gears run from another machine (Diego's option B)**, in
  src/sim/unsorted_1004/w3p_net.c.
  - **Correction (2026-11-01):** I first reported three 1.2 slips. The third was wrong. 1.2's "TC" (the handover,
    0xef bytes, sent by 0x100343b5) carries the 17 part records at +0x23, and its receive unpacks them there,
    inside the message. The 0x21-byte message is "AI" (Sim_100408bd = 0x100344ef), not "TC". So 1.2's handover
    does carry the damage.
  - 1.2's new "DI" (0xd1 bytes, sent with each "AI" update) keeps the parts in step afterwards. It has the two
    real slips:
    - the tag is written into the "DM" message buffer [0x1015ecb0] (1.2 [0x10131500]), so "DI" goes out
      untagged and is dropped;
    - the receive tests (state == 0) == 3, never true.
  - NETDI_FIX (default 1) fixes those two. NETDI_FIX 0 is 1.2 exactly.
  - Functions: "DI" send Sim12_10033c90, pack Sim12_100342cf, unpack Sim12_10034342 (used by "TC" too),
    receive Sim12_10033d7b. The "AI" send Sim_100408bd sends "DI" first. The "TC" receive Sim_1004084a gets
    the game-over guard and 1.2's unpack from +0x23.
  - Checked by spec_net_handover_v12.py (TCR / TCS / DIR):
    - Fix off against 1.2: 3000 / 0 each.
    - Fix on against 1.2 with exactly the two fixes patched into its code: 3000 / 0 each.
    - Planted bugs (no guard, no "DI"): caught in each.
  - The message loop dispatches "DI" (size 0xd1).
- **"Player left" notice (Diego, 2026-11-01: option B, fix it).**
  - 1.2's loop takes the library's notice as "d2" / "d6" (1.0: "eD"). It then reads the player's key from the
    notice's first word, which is the tag itself, so it never finds the player.
  - NETLEFT_FIX (w9b_net.c, default 1) reads the key after the tag, as 1.0 did. NETLEFT_FIX 0 is 1.2 exactly.
  - 1.2 also skips a player already gone (+0x160 set; 1.2's +0x15c).
  - Our ANET2 stand-in must send the notice as "d2" + the player's key word (AFTER_DLL.md).
- **The loop's time-out:** a silent player's state-3 computer gear is handed on by the host (Sim_10040564; 1.0
  cleared its controller). 1.2's debug print there ("NextExternalCtrl...", 0x1006e4b5) is an empty function. The
  time-out loop covers 10 players. The loop also dispatches 1.2's "DI" (size 0xd1).
- **"RG", regeneration / revival** (Sim_10040af8, w20_net.c), in 1.2's rewritten form (0x1003474e); 1.0's version
  stays behind NETRG_MUTATE.
  - The game-over guard; it now takes the sender.
  - The host answers the sender (1.0: the unit's player), adding 10 (1.0: 8) when lives are left. It no longer
    handles a state-3 unit's controller.
  - The asker checks the unit and its owner. It clears the right unit's +0x164 (1.0 used a never-set pointer:
    round 20) and calls Sim12_10089c60 before its own respawn. "Out of regenerations" no longer clears config
    byte +8, and it clears the lives unconditionally.
  - A revived state-3 unit no longer gets Sim_10011093.
  - spec_net_rg_v12.py 3000 / 0. The 97 both-faults are a missing unit on the host's path, unchecked in both. The
    unit table is filled past the players so a revival's unit + 10 resolves. Planted bugs: NETRG_MUTATE (1.0's
    "RG") 2365; NETG_MUTATE 651.
- **The player update** (Net_UpdatePlayers = 1.2's rewritten 0x100360c4, w9b_net.c; 1.0's behind NETUP_MUTATE).
  1.2 split it into three new functions:
  - Sim12_100343b5: "TC", 0xef bytes, with the 17 parts at +0x23 (1.0 sent 0x23 bytes).
  - Sim12_10036590: the host's round-robin handover of a computer gear whose controller is gone.
  - Sim12_100362d2: a destroyed computer gear of the local player. Kill text with killers 0..9, then a revival:
    asked of the host, or done on the host with "'%s' resurrected.". 1.2 drops 1.0's Sim_10011093 and +0x164 on the
    host's revival.
  Also changed:
  - 1.0's clearing of +0x164 for an intact gear is gone.
  - So is its "+0x18" busy flag (1.2's record has none).
  - Only state-3 gears are looked at.
  spec_net_update_v12.py (UP / RR / DD / TC) 3000 / 0 each. UP's 20 both-faults are a random chase target beyond
  the unit table, read in both. The generator always leaves one free player in the list, since the round robin
  loops for ever otherwise, in both. Planted bug NETUP_MUTATE (1.0's update) caught 2411.

## Command line (Diego, 2026-11-01: "We want only 16 bit, no need for switch")

- **1.2's "/16bit" and "/8bit" not ported.** 1.2's reader (0x100201d0 = our CmdLine_Parse 0x1002ad50,
  w9d_cmdline.c) adds the two words after "nogrid"; both set 0x1014a5e0 (never read in 1.2) and "16bit" / "8bit"
  set the .TEX flag 0x10105048 to 1 / 0. 1.2's sim main sets 0x10105048 again from the 3D mode (0x10070a3d) before
  anything reads it (checked by walking every direct call between the reader's call 0x1006fe81 and that store: no
  read), so in 1.2 the switches change nothing. Neither 1.2's shell nor any data file passes them. Our .TEX mode
  stays `W31_TexMode()` (Vulkan with the 3D mode or Voodoo look), i.e. 16-bit only. The rest of the reader is the
  same as 1.0's apart from addresses and registers, so CmdLine_Parse is unchanged; the words, if typed, get 1.0's
  unknown-switch report (error 10).
- **"DM", a remote unit's part damage levels** (Sim_10040191, w7m_misc.c = 1.2 0x10033a8a):
  - 1.2 adds the game-over guard.
  - 1.2 no longer sets the parts' values (+0, +8) from the levels. It works the value out into a local and stores
    it nowhere.
  - A part reported destroyed gets +8 = 0, and the second part marks the unit (g_net10Mark).
  - 1.0's update stays behind NETDM_MUTATE.
  - spec_net_dm_v12.py 3000 / 0; planted bugs NETDM_MUTATE 331, NETG_MUTATE 118.
  - 1.2 behaviour kept: it is deterministic and looks deliberate (coordinator, 2026-11-01: not a choice, the whole
    game is 1.2). NETDM_MUTATE is only the checker's negative check.
- **The state sender** Net_SendStateMessage (1.0 0x1003eebc = 1.2 0x10032782): only the record shift. The "AI"
  payload Sim_10040721 (= 0x100341a6) and Sim_10040645 (= 0x100340ca) are also unchanged apart from layout.
- **"NR", the game settings** (Sim_10040eb3 = 1.2 0x10034b31):
  - The own state is sent first.
  - The two values sent are kept where the message loop compares them: [0x1013aca0] the +0x137 byte,
    [0x1013aca4] the +0x138 byte. 1.0 kept them crossed, so with the two bytes different the loop re-sent "NR"
    every frame.
  - spec_net_nr_v12.py 1000 / 0; planted bug NETNR_MUTATE (1.0's) 1000.
- **The host change** (Sim_100421b0 = 0x10035fe7) and its player-list callback:
  - The callback (0x10042125) was original code; it is now C, Sim_10042125, __stdcall, from 1.2's 0x10035f3e.
  - 1.2 keeps the host before the change (g_net10OldHost = 1.2 0x10110144).
  - When the new host differs, the callback cancels a pending request ([0x1015f200]).
  - spec_net_host_v12.py CB 3000 / 0, HC 3000 / 0; planted bug NETHOST_MUTATE CB 1259, HC 2831.
  - The callback is handed to the session's dpEnumPlayers (ANET2): its signature must stay as ANET2 calls it until
    our transport replaces that.
- **The message loop** (Net_ProcessMessages = 1.2 0x10031a86): 1.2's "player left" notice (with NETLEFT_FIX), a
  player already gone skipped, the "DI" dispatch, the host handing a silent player's computer gear on, 10 players.
  spec_net_loop_v12.py, on spec_w9b.py's stubs with each also hooked at its 1.2 twin:
  - NETLEFT_FIX 0 against 1.2: 3000 / 0.
  - NETLEFT_FIX 1 against 1.2 with the key read after the tag (a hook in its code): 3000 / 0.
  - Planted bug NETLOOP_MUTATE (1.0's notice and time-out) caught 111.
- **The rest of 1.2's network module** is 1.2's code already, or unchanged apart from the record shift:
  - GO 0x100356ce, SU 0x10035a50, the reset 0x10035c65 (three empty debug prints).
  - The helpers 0x10035eb9 / 35646 / 35673 / 35782 / 357d4 / 3583d / 366ce / 33dec / 33fe8 / 3470e.
  - The receive / send wrappers.
  - The "DM" send 0x100338e2 (now on g_net10Row).
- **Not part of this round (transport):** 1.2's extra ANET2 loader (0x1002fd99 / 0x1002fdf4: dpDestroyGroup ..
  dpReadyToFreeze). It goes with the ANET2 stand-in.
- **The old 1.0 network specs** (spec_w9b.py's Net_ProcessMessages / Net_UpdatePlayers / Score_RecordKill parts,
  spec_w3p.py's handlers, spec_w10_1003dc19.py) still describe 1.0's tables and logic. They are superseded by the
  spec_net_*_v12.py specs; no fix is planned for them.
- **Transport deferred (Diego, 2026-11-01): "only do the changes for hgsim, I don't want to rewrite the network
  library yet".** No ANET2 stand-in is written. The original ANET2.DLL stays, under the menus and the sim.
  tools/v12/anet2_shell.md stays as notes for later.
  The sim-side 1.2 code runs on the original ANET2:
  - **"Player left" fix B:** reads the key after the "d2" tag. 1.2's own menus read ANET2's "d2" notice the same
    way (id at +2, name at +0x10; hgshell 0x1002abfb and 0x1011f0bf), so the fix matches the real ANET2.
  - **"DI" fix B:** only the message's own bytes and receive test; nothing transport-specific.
  - **The host-change callback** (Sim_10042125): __stdcall with 4 arguments, as 1.0's 0x10042125 and 1.2's
    0x10035f3e (both `ret 0x10`), the way ANET2's dpEnumPlayers calls it.
  - No other network change assumes a new transport.
- **1.2's ANET2 name list** (its loader, the twin of Sim_1003be30; the survey's 0x1002fd99 / 0x1002fdf4 are points
  inside it): 54 names instead of 55, in another order. It has dpSetGameServerEx for dpSetGameServer and no
  dpEnumTransports. The sim never calls those, and our C uses its own slot table consistently, so nothing to port.

## 1.2 audit: the two missing changes (Diego, 2026-11-01: "apply both changes")

An independent audit of all of 1.2's HGSIM (tools/v12/audit.md) found two 1.2 changes not yet in the C. Both ported:
- **Force feedback when firing:** Sim_100d4f71 (1.2 0x100baf31), the fire routine the weapon tables point at (no
  direct caller, so the round-33 survey had called 1.2's version dead). For the own unit's shot, after the shot
  counter [0x101653f3], 1.2 sends the force-feedback request Sim_100382fb(weapon +0xd2, 0, 0, 0) while the library
  flag [0x1015ef3c] (1.2 [0x10131718]) is set. tools/verify/spec_w10_100d4f71_v12.py 4000 / 0 against 1.2
  (W10F_NEG12, no request: 1355); spec_w10_100d4f71.py (1.0) still passes 4000 / 0 (its flag stays clear).
- **Current-objective line wrap:** Sim_100cd70f (1.2 0x100b1c8f). 1.2 copies the objective whole into its own
  buffer, wraps only when it is longer than the width (1.0 also when equal) and always breaks at the objective's last
  space (it tests the byte it has just zeroed, so the search always runs, over the whole copy): line 1 is all but the
  last word and can be longer than the width. This replaces the round-21 deviation for this function (the zeroed
  buffer is gone; the stack-leftover search no longer exists). Deviation: an objective longer than the width with no
  space at all keeps the width as its cut (1.2 takes a cut from strrchr's NULL). tools/verify/spec_w9g_cd70f_v12.py
  3000 / 0 against 1.2 (W9GW_NEG12, 1.0's `>=` test: 228). Superseded for this function: spec_w9g.py's Sim_100cd70f
  entry and spec_w21_wrap_cd70f.py (both describe 1.0's wrap). The F12 list (Sim_100ccc60) keeps 1.0's wrap, as 1.2
  does, with its round-21 deviation.

## DLL removal step 1: graphics library, Vulkan only (Diego, 2026-11-01: "I only want to use vulkan for the final version, convert the functions required for this. I don't care for direct3d only or software only code.")

- **Scope.** The final program keeps only the Vulkan renderer. D3D option 3 and the sim's DirectDraw driver
  (0x1007f35c..0x10082790 and 0x1008667c) are dropped from the final program; the comparison reference for the look
  of the game is the untouched 1.2 game (dev.cmd option H). The 41 graphics-library functions the Vulkan build still
  runs are converted (below). The software-only C call sites are not deleted yet (AFTER_DLL.md).
- **Not converted, software renderer only** (every call site under `G32(0x1017b940) == 0`): 0x100e4780, 0x100e5477,
  0x100f9fd4, 0x100e96a7, 0x100eda16.
- **Not converted, dead** (no caller and no pointer in the DLL): 0x100e4c2c 0x100e5d25 0x100e6279 0x100e6b49
  0x100e84a8 0x100e84d1 0x100ea87d 0x100eac78 0x100eb14f 0x100eb4eb 0x100ecbf3 0x100ecfec 0x100ed02e 0x100ed1f5
  0x100ed226 0x100ed253 0x100ed2d5 0x100ed300 0x100ed765 0x100ed831 0x100ed879 0x100ed8d6 0x100ed938 0x100edc8d
  0x100ede11 0x100ef51c 0x100ef545 0x100ef564 0x100ef649 0x100f094e 0x100f1abb 0x100f1b73 0x100f1b99 0x100f1c60
  0x100f1c73 0x100f1e47 0x100f1fb0 0x100f2134 0x100f9e9a 0x100f9edf.
  (Both lists are the coordinator's survey, recorded as given; the conversion does not depend on them. Some call
  sites of the converted 0x100e86a1 / 0x100ecea6 are themselves under `[0x1017b940] == 0`.)
- **Already in C:** 0x100e440b / 0x100e4463 are W9d_Expand4 / W9d_Expand2, static functions of
  src/sim/unsorted_100e/w9d_decode.c (round 9, register arguments, proven through 0x100e41cc by spec_w9d.py).
- **Converted** (`src/sim/gfx/w37.h`, `w37_info.c`, `w37_text.c`, `w37_gif.c`, `w37_math.c`, `w37_draw.c`,
  `w37_encode.c`, `w37_line.c`, `w37_rotate.c`; hooked by `src/hooks/conv_w37.inc`, 36 entries; specs
  `tools/verify/spec_w37_*.py` with `w37_common.py`, logs in buildlogs/round37). Each from its hand-written assembly,
  all exact (no float); trials / mismatches:
  - records and colours (spec_w37_info): font height 0x100ecce0 300/0; shape count 0x100ed8c3 300/0; glyph width
    0x100eccf3 500/0; shape first dword 0x100ed743 500/0; shape origin 0x100f1f86 and its duplicate 0x100ed7bc
    500/0 each; shape size 0x100f1f52 and its duplicate 0x100ed788 500/0 each; shape palette 0x100ed7e6 1000/0;
    colour table get / set / load 0x100f01ea 500/0, 0x100f01cc 500/0, 0x100f01ad 200/0; code block 0x100f9eb7 50/0;
    GIF image size 0x100ed70b 1000/0; GIF palette 0x100ed6aa 1000/0.
  - text (spec_w37_text): 8-bit character 0x100ecd13 10000/0 (coverage 171/171); string 0x100ecea6 3000/0.
  - GIF (spec_w37_gif): row copy 0x100ecedd 5000/0 (114/114); decoder 0x100ed491 1500/0 with its five
    register-argument helpers 0x100ed327 (table reset), 0x100ed36f (data byte), 0x100ed388 (bits), 0x100ed3ce (new
    code), 0x100ed414 (pixel out) as static functions of w37_gif.c (only it calls them; their thunks are
    unreferenced): helpers 108/108, decoder 185/199 (the 14 not run: the 1-bit branch below). Real LZW streams,
    interlaced or not, 10% with corrupted bytes (the 31 both-fault trials are corrupted streams that run off the data
    in both).
  - rotation (spec_w37_math): wave lookup 0x100ecb3b 5000/0; point rotation 0x100ecc19 5000/0.
  - drawing (spec_w37_draw): pixel set 0x100e84f0 5000/0; pixel read 0x100e85cb 5000/0; pane fill 0x100eb070
    3000/0; unclipped shape 0x100e95e0 5000/0; clipped shape 0x100e91dc 20000/0 (433/439: the 6 not run are a
    dispatch head after an unconditional jump); their remapping twins (colours through 0x10154eac) 0x100e9b9e 5000/0
    and 0x100e96c6 20000/0 (478/484, the same dead head); checkerboard fill 0x100e90a3 5000/0 (138/145: the dead -3
    exit).
  - shape encoder (spec_w37_encode): packet writer 0x100eae93 5000/0 (164/164); row 0x100ead0a 5000/0 (138/138);
    pane encoder 0x100ea9f5 3000/0 (220/220).
  - lines (spec_w37_line): line 0x100e86a1 30000/0 (916/1016: the not-run are the callback mode's five loops and the
    two unreachable jumps round 30 found in the 16-bit twin); ellipse 0x100eb6e8 5000/0; filled ellipse 0x100eba29
    5000/0.
  - rotated / scaled shape (spec_w37_rotate): 0x100e9cd1 5000/0 (886/886).
  - A planted bug (0xfe for the transparent 0xff in 0x100ecd13) is caught (343 of 10000).
- **1.2:** every one of the 41 has a 1.2 twin identical apart from addresses (listings aligned instruction by
  instruction; the five helpers at 1.2 0x100cf517..). The tables they read hold the same bytes in 1.2: the bit masks
  0x10154e95, the interlace steps 0x10154e9e and the quarter-wave table 0x100ebd27 (1.2 0x10126d4d, 0x10126d56,
  0x100cdf17). The 1.0 proofs are therefore 1.2 proofs; no 1.2 spec needed.
- **Callers keep SIM_FN.** As round 30 did for the same library's 16-bit routines (and round 22's note: "one place
  to redirect"), the C callers still call these at their original addresses, where the CONV hooks jump to the C.
  Only calls between the converted routines are direct C calls. The switch-over is in AFTER_DLL.md.
- **Deviation (unreachable): the line's callback mode.** 0x100e86a1 with a mode above 1 calls the colour argument as
  a function for every cell, under a pushad, with the cell in EDI / ESI and the steps in EAX / EBX (register
  arguments). Every call site (about 75 in the DLL, all the C ones) passes mode 0, so no such callback exists; the
  C clips as the original and draws nothing for those modes (no assembly adapter). Mode 1 (cells through a table) is
  also unused and converted as it is.
- **Unset read kept as round 30:** for horizontal and vertical lines 0x100e86a1 returns ([ebp-0x34] >= 1), a flag
  only the clipping path writes (the 16-bit routine has the same). The C returns 0 there; the checker seeds the slot
  with 0. The ellipses return the line's result when a radius is 0. No caller found that reads these results.
- **Original quirks kept:** the checkerboard fill 0x100e90a3 sets the cell right of the rectangle on a shifted row of
  width 1 (one cell past the pane's right edge when the rectangle touches it); the packet writer 0x100eae93 writes a
  {0, cell} or {1} packet when a span is empty and did not widen the box; 0x100ecedd / 0x100eb6e8 / 0x100eba29 /
  0x100e9cd1 / 0x100ecc19 return the original's last EAX where it is not a status (callers ignore it); the shape
  palette 0x100ed7e6 runs 2^32 times for a count of 0; 0x100ed6aa lets a local GIF palette overwrite the global one
  from the start.
- **Not reproduced (no effect):** 0x100e95e0 / 0x100e9b9e keep the window's pitch in their fifth argument's stack
  slot (callers never read it; as round 30's 0x100f0727). 0x100eae93 keeps EAX / ECX for its only caller 0x100ead0a,
  which is C now (both hooked from the same .inc, so "convoff" cannot split them). The GIF decoder's 1-bit output
  branch (taken for a pixel width of 1 in EDX; EDX is always 8 there) is left out. 0x100e9cd1's per-row steps
  ([ebp-0xc8] / [ebp-0xcc]) are read stale (unset on the first row) for a row of one cell, where they only move the
  source past its last texel: the C starts them at 0. Division overflows (#DE) of 0x100e9cd1 for spans of 32768
  cells or more, and the GIF decoder on corrupted data running past the file, are not modelled (the specs keep sizes
  below that).
- **0x100f9eb7** reads the code segment register (one `mov` from CS; inline assembly for MSVC and GCC) and reports
  the DLL's own self-modifying code block 0x100f4fd4..0x100f9e9a, which Sim_10034cca makes writable with
  VirtualProtect. Meaningless without the DLL (AFTER_DLL.md).
- **Build:** the Release build (vcvars x86, `cmake --build build --config Release`, as dev.cmd option 4 without the
  copy into the game folder) compiles; the game was not run.

## C runtime replacement: maths (OpenLibm) (Diego, 2026-11-01: "Finish the math, ONLY"; compiler settings 2026-11-01: "ok, add those compiler settings")

- **What runs now.** OpenLibm v0.8.8 (third_party/openlibm, HG_VENDOR.md), compiled into the program through
  src/sim/libm (hg_sqrt, hg_fmod, hg_fabs, hg_pow, hg_sin, hg_cos, hg_atan, hg_asin, hg_acos, hg_floor;
  hg_math.h). Every C call of the original C runtime's x87 routines sqrt 0x100fca04, fmod 0x100fcb30, fabs
  0x100fcb70, pow 0x100fed29, asin 0x100fcc74, sin 0x100ff644, atan 0x10100044 calls OpenLibm instead, and round 10's
  interpreted functions use OpenLibm's sin / cos / asin / acos in place of the platform library (W10I_K_*). The exact
  C copy of fabs and its hook are gone. Plain C: the same results on Windows and Linux (with the settings below).
- **Where OpenLibm differs from the original, measured on the real CPU.** The original routines were run natively
  (HGSIM.DLL mapped without DllMain or imports, as round 26's trigcap.c; x87 control word 0x27f, the game's), next
  to OpenLibm and a 60-digit Decimal reference (the emulator's fprem / fyl2x / f2xm1 / fpatan / fsin are themselves
  inexact, so it cannot be the reference). Game-like inputs:
  | routine | inputs | bit-identical | differ | max error orig / OpenLibm (ulp) | where they differ, closer to exact |
  |---|---|---|---|---|---|
  | sqrt | 15000 | 15000 | 0 | 0 / 0 | - |
  | fmod | 15000 | 15000 | 0 | 0 / 0 (both exact) | - |
  | pow | 60000 | 55157 | 4843 (8.1%) | 0.634 / 0.794 | original 3099, OpenLibm 1744 |
  | asin | 60000 | 47026 | 12974 (21.6%) | 1.565 / 0.860 | OpenLibm 10643, original 2331 |
  | sin | 15000 | 14499 | 501 (3.3%) | 0.541 / 0.733 | original 492, OpenLibm 9 |
  | atan | 15000 | 14742 | 258 (1.7%) | 0.500 / 0.700 | original 256, OpenLibm 2 |
  Every difference is one unit in the last place of the double (the callers mostly round to float after). fabs is
  bit-identical by construction. So sqrt / fmod / fabs give the original's values exactly; pow / asin / sin / atan
  may differ in the last bit (asin: the original's is the less accurate one). With control word 0x37f (64-bit
  precision) the original is nearer exact (pow 3903, asin 3139 differences) and its sqrt double-rounds twice in
  15000; the game uses 0x27f.
- **Also different (not observable):** the original's routines store EDOM / ERANGE in the C runtime's errno
  (0x1017e888) for NaN, infinite or out-of-range arguments, keep scratch at 0x1017e900..0x1017e97f, and fabs reports
  a NaN / infinity to the runtime's maths error routine 0x10102e30 (called only from fabs 0x100fcbe3 and the shared
  x87 error path 0x1010a59d). OpenLibm does none of this; the values returned are the same. No code outside the C
  runtime refers to 0x1017e888 (all 46 references in the DLL are at 0x100fa000 and above). Round 10's interpreted
  wrappers still store the original's EDOM themselves (w10_interp_body.h).
- **1.2:** sqrt, fmod, fabs, asin and pow are the same code in 1.2 at 0x100daba4, 0x100dacd0, 0x100dad10, 0x100dae14
  and 0x100dd2e9 (byte match with call targets masked). 1.2's sin / atan were not located; the checker answers them only in 1.0.
- **Checker (tools/verify/difftest.py):**
  - fmod: the original's call (1.0 0x100fcb30, 1.2 0x100dacd0) with finite x and finite non-zero y is answered with
    Python's exact math.fmod, as real fprem (exact) and OpenLibm; NaN / infinite / zero-divisor calls run the
    original.
  - pow / asin / sin / atan: the original's calls (1.0 all four, 1.2 pow and asin) jump to the same OpenLibm function
    the C calls, so a spec compares the converted code around them; the differences are the table above.
    spec_crt_libm.py sets LIBM_ORIGINAL and runs the original routines.
  - In a trial in which a maths routine ran (either side), errno 0x1017e888 and the scratch 0x1017e900..0x1017e97f
    are not compared (not in spec_crt_libm.py).
  - The object cache (build/w10cache) is now also invalidated by any header change (most call sites were switched
    by header macros, which the .c timestamps missed), and rt.o is built per process.
  - tools/verify/run_specs.py runs a list of spec files on all cores, one process per file
    (tools/verify/crt_math_specs.txt: the 109 spec files / 150 entries whose C reaches an OpenLibm function, by a
    static call graph of the compiled sources).
- **Specs adapted (recorded exceptions):** spec_w10_100225c7, spec_w29_100232d7 (and _v12) and spec_w26_10021e8c
  stubbed fmod at 0x100fcb30 only, which the C no longer calls: the stub now also answers hg_fmod (same answers both
  sides, as before). spec_w8z: the fmod / sqrt stubs also answer hg_fmod / hg_sqrt; its Crt_Fabs entry (the deleted
  exact copy) is dropped; calls of 0x10102e30 (fabs's error report, above) are left out of both call logs.
- **spec_crt_libm.py** (the original routines under emulation against OpenLibm, 20000 each): sqrt 226, fabs 90,
  fmod 485 mismatches, every one only errno (values bit-identical); pow 8506, asin 3127, sin 615, atan 400, of which
  185 / 227 / 223 / 86 errno only and the rest last-bit differences in the emulated x87 (the native table above is
  the real comparison).
- **Re-run** (buildlogs/crt_math, merged in buildlogs/crt_math/index_final.txt): the 150 entries plus the rest of
  spec_w8z, 168 entries: 157 pass. The 11 that fail are not maths: 10 fail with the same mismatch counts on the last
  commit fa18a35 without the maths change (Sim_100246b3 2886, Sim_10058c5e 3669, Sim_100d4590 3138, Sim_100a0063
  1952, Sim_1005bb7a 7426, Sim_10070eb5 869, Sim_100dcc12 154, Sim_1006e22c 939, Sim_10040af8 5576, Sim_100a0def
  248; checked in a clean worktree), and spec_w14_1009d200.py describes 1.0's respawn (round 33 ported it to 1.2's,
  which drops Sim_1007caab; spec_w14_spawn_v12.py passes 6000 / 0).
  First full run: 109 spec files on 16 processes in 2939 s (the longest single file 743 s).
- **Compiler settings** (CMakeLists.txt, every target: the game, HGVK, OpenLibm, SDL): SSE2 doubles, never x87
  (MSVC `/arch:SSE2`; GCC / Clang on x86 `-msse2 -mfpmath=sse`), no fused multiply-add (MSVC `/fp:precise` without
  `/fp:contract`; GCC / Clang `-ffp-contract=off`), never fast-math. These are MSVC's own defaults for x86
  (SSE2 and /fp:precise unless told otherwise), so today's Release build compiles the same floating-point code; the
  settings pin it for other compilers and platforms.
- **Build:** the Release build (vcvars x86, `cmake --build build --config Release`) compiles and links HG.exe and
  HGVK.dll; the game was not run.

## C runtime replacement: memory, files, formatting (Diego, 2026-11-02: targets Windows, Linux and Mac, results identical on all three; "go with option 1": own copies + SDL3 formatting)

- **Rule.** The converted copies of the original C runtime keep their own logic; only what they took from Windows or
  from the DLL's unconverted code is replaced, at the bottom. Text, sorting and the rest of the copies stay as they
  are (plain C).
- **Maths round gap closed.** 27 spec files with a fixed source list (no `closure`) were never re-run in the maths
  round, and 38 old spec files could not be built at all (later rounds had moved callees). The checker now always
  links src/sim/crt and src/sim/libm, adds the files defining missing symbols from its object cache and retries,
  never emits stack probes, and builds rt.o per process. All of them were run in the memory round below.

### Memory

- **Now.** malloc / calloc / realloc / free (Heap_Malloc 0x100fef30, Heap_MallocRetry 0x100fef50, Heap_AllocBlock
  0x100fefa0, Heap_Calloc 0x10100580, Heap_Realloc 0x101003e0, Heap_FreeBlock 0x100fefe0) are standard C
  (src/sim/crt/hg_heap.c); memset / memmove (Mem_Fill 0x100fcab0, Mem_Move 0x100fcdc0) too.
- **The DLL's start-up blocks.** The DLL's own C runtime ran its start-up (environment, stdio tables, atexit) in its
  private heap before our hooks were in place, and its shut-down frees those blocks through the hooked free. Our
  blocks carry a 16-byte head (a mark and its own address); free / realloc of a block without it go to the
  original's heap code (still converted C), realloc of such a block stays in that heap.
- **Differences (recorded):** no new handler (the original retried through _callnewh when [0x1017e968] was set; out
  of memory now just answers NULL); blocks are no longer rounded to 16 bytes (the game never reads past its size);
  malloc(0) / calloc with 0 give a 1-byte block (the original: one 16-byte paragraph).
- **Checker.** The original's heap entry points (1.0 above; 1.2 0x100dd4f0, 0x100dd510, 0x100dd560, 0x100dd5a0,
  0x100de8a0, 0x100dea40) jump to the C ones, so both runs get the same blocks (rt.c: a bump allocator); a spec
  entry may set `heap_original`. spec_w6i Heap_FreeBlock and spec_w7r Heap_Realloc run the original's heap for
  blocks of the DLL's heap (recorded exception, `skip`: only real blocks, and no NULL realloc); their malloc /
  calloc entries are superseded.
- **Specs** (tools/verify/crt_mem_specs.txt, buildlogs/crt_mem*): 406 entries pass. 25 fail, all with the same
  failures on the last commit 0050ec5 (checked in a clean worktree): the old 1.0 specs of functions later ported to
  1.2 (network w3p / w9b, the cache reader w8y, objectives w9f, the respawn w14_1009d200) and the long-known ones
  (Sim_100d4590, Sim_10070eb5, Sim_100dcc12, Sim_1006e22c, Sim_10040af8, Sim_100a0063, Sim_1005bb7a,
  Sim_1003dc19, Sim_1006b52f, Sim_1004954e). Their 1.2 specs pass (e.g. spec_w8y_reader_v12 4000 + 3000 / 0).
  spec_w28_anim and spec_w9e still do not link (duplicate platform definitions; not maths or memory).

### Files

- **Now.** The converted low-level I/O keeps all of its own rules (text mode with CR LF and Ctrl-Z, append, the
  error numbers); the Windows calls under it are the platform's file functions (src/platform/plat.h "files",
  plat_file.c: standard C files, always binary, and SDL3 to list folders), in the same shape: Plat_FileCreate /
  Read / Write / Seek / Truncate / Flush / Close / Type / Delete / LastError / FindFirst / FindNext / FindClose. Used
  by _read, _write, _lseek, _close, _chsize (w8A_lowio.c), remove (w8A_stream.c), _commit (w6j_stream.c),
  _findfirst / _findnext / _findclose (w7q_find.c, w6i_stream.c) and the movie folder search Sim_1007e66c.
- **_sopen converted** (0x101099a0, was original code; Crt_SOpen in w6k_handles.c, called by _open and fopen):
  spec_crt_sopen.py 4000 / 0 against the original (handle tables, CreateFile failing, device / pipe types, the
  Ctrl-Z trim of a read/write text file).
- **Names.** "\" and "/" both work; on Linux and Mac the drive ("C:") is dropped, and a name that does not exist as
  written is looked up folder by folder ignoring case (the first match in an order sorted ignoring case). Files stay
  where the game puts them (the game folder); no settings overlay.
- **Settings files.** Plat_IniString / Plat_IniInt are our own reader (plat_ini.c) with Windows' rules, on every
  system (Windows' GetPrivateProfile* no longer used).
- **Measured against Windows itself** (native harnesses, scratchpad): 200,000 random create / read / write / seek /
  truncate / close / delete steps on three files through CreateFileA... and through plat_file.c: same results, error
  numbers, bytes and final files (after four fixes the test found: access rights, "truncate existing" needs write
  access, a failed seek keeps the position). 20,000 random settings files read through GetPrivateProfileStringA /
  IntA and plat_ini.c: same (after two fixes: names asked for lose their spaces; numbers take "0x" / "0o" / "0b").
- **Differences (recorded):** the folder listing gives no short (8.3) names (the movie search then uses the long
  one, the same for 8.3 names), attributes 0x10 / 0x20 only (no read-only / hidden / system bits), and its order is
  sorted ignoring case (Windows: the file system's order, also sorted on NTFS). Share modes are not enforced.
  Truncating a file rewrites it (standard C has no truncate).
- **Specs** (crt_files_specs.txt, buildlogs/crt_files*): every entry reaching the new functions passes (the stubs of
  spec_w6k / spec_w7s / spec_w8y now also answer the C names Crt_SOpen / Crt_FPrintf), except Sim_10046f7f of the
  1.0 spec_w8u: the HGCAR save writes 1.2's 0x21c-byte record (its 1.2 spec spec_w8u_hgcar_v12 passes).

### Formatting

- **Now.** The DLL's printf engine (0x10100fd0) is no longer used: sprintf, vsprintf, _vsnprintf, fprintf and
  printf (Crt_SPrintf, Crt_VSPrintf, Crt_VSNPrintf, Crt_FPrintf, Crt_Printf) format with src/sim/crt/hg_format.c,
  which walks the format and hands each conversion to SDL3's own SDL_snprintf (the same code on every system), with
  three parts of its own: the space flag (unknown to SDL; the game's string at 0x1013105c uses it), %f (SDL rounds
  halves differently: the original's digits are made exactly, rounded to 17 significant digits and then half up)
  and a negative zero printed without its sign. sprintf / vsprintf stay unbounded as the original (no buffer sizes
  at the call sites); _vsnprintf keeps its rules (-1 when cut, no closing 0 when full); fprintf / printf write the
  text with fwrite inside the original's temporary-buffer bracket.
- **Scanning.** sscanf (Sim_100fd710) is SDL_vsscanf. SDL3 hands that, and strtod, to the system's C library on
  Windows, Linux and Mac alike (SDL_string.c, HAVE_VSSCANF / HAVE_STRTOD): Microsoft's, glibc's or Apple's code.
  Ordinary numbers read the same; spellings like "inf" / "nan" or hexadecimal floats may differ between systems.
  fscanf (one use: object.dat, "%s") reads the rest of the stream and scans it (the stream is read to its end; the
  caller closes it right after). The scanning engine 0x10103ba0 and text-to-number 0x1010b7c0 are no longer used by
  our C (only the original scanning engine called 0x10108e20 / 0x10108da0).
- **Measured against the original** (native: the DLL's own sprintf 0x100fc780 run on this CPU, DLL mapped without
  DllMain, its float table set up by 0x100fc640, x87 control word 0x27f): 2,244,421 integer / character / text
  conversions (d i u x X o c s, h / l, every flag, width, precision): identical. 755,579 %f conversions (game-like
  values and any finite double): identical but 3, all numbers above 2^49 (about 5.6 * 10^14) whose 17th digit is an
  exact half, where the original's x87 conversion rounds either way. The game's space-flag string: identical.
- **Checker.** tools/verify/stand_in/hg_format.c stands in for hg_format.c (which needs SDL3) and calls the original
  engines, so the specs compare the code around them. The specs of the wrappers themselves (spec_w8z Crt_SPrintf,
  spec_w8A Crt_VSNPrintf / Crt_VSPrintf, spec_w7q Crt_FPrintf / Crt_Printf, spec_w6i Sim_100fe450) tested how they
  drove the engine and are superseded by the native comparison.
- **Specs** (crt_format_specs.txt, buildlogs/crt_format*): every affected entry passes apart from the superseded
  wrapper entries and the old failures listed under memory.
- **Build:** the Release build compiles and links HG.exe and HGVK.dll with all three groups; the game was not run.

### The rest

- **Now** (src/sim/crt/hg_rest.c, standard C; checker stand-in tools/verify/stand_in/hg_rest.c calls what the copies
  called before): time (Crt_Time 0x100ff500) is standard time(); ctime's local time (Sim_100ff4e0) is standard
  localtime (the original's 0x101071e0 no longer used), filled into the original's nine-int record; getenv
  (Crt_GetEnv 0x10100130) is standard getenv; exit's last step (Proc_DoExit 0x100ff410) is standard exit (was
  ExitProcess) or _Exit (was TerminateProcess), after the original's own exit routines as before.
- **assert** (the original 0x100ff860, 42 call sites, reached through their old addresses): hooked to Crt_Assert
  (conv_w6i.inc). The same text as the original's box ("Assertion failed!", Program / File / Line / Expression, the
  Visual C++ lines; names over 60 characters as "..." and their end, the text capped at 0x21c bytes) under the same
  title, in an SDL3 message box (Plat_AssertBox, plat_dialog.c) with Abort / Retry / Ignore: Abort leaves with code 3
  (Proc_ExitQuick, as the original's raise(SIGABRT) without a handler), Retry stops in the debugger (SDL's
  breakpoint, the original's DebugBreak), Ignore returns. The program name comes from Plat_ProgramPath
  (GetModuleFileNameA on Windows; Linux / Mac still to do: "<program name unknown>" until then).
- **Joystick maker name** (Joy_GetOemName 0x10057186): no registry read, no name (joysticks are not supported,
  Diego 2026-11-02); spec_w7n's entry is superseded.
- **Differences (recorded):** time and local time follow the system's own clock and time-zone rules (the original
  used Windows' GetLocalTime / GetTimeZoneInformation and its own daylight flag cache; the original's TZ variable
  handling is no longer used by these); no local time before 1970 (as the original).
- **Not replaced, with the DLL:** signal / raise (Crt_Abort), the maths error path (Math_ReportError,
  Math_RaiseFpError, Math_DomainError, Math_FrExp and the x87 control-word helpers they call), the Pentium FDIV
  check, the runtime-error message box (Sim_10107840: LoadLibraryA user32), the locale copies (CompareString /
  LCMapString / GetStringType: only with a non-"C" locale, which the game never sets), the start-up stdio and heap
  set-up. None is reachable from the game's C (static call graph: no caller outside src/sim/unsorted_100f /
  unsorted_1010); they serve the DLL's own code through their hooks.
- **Specs** (crt_rest_specs.txt, buildlogs/crt_rest): the 13 entries reaching hg_rest.c pass.
- **Build:** the Release build compiles and links HG.exe and HGVK.dll; the game was not run.

## Dropped features: CD audio, joystick, DirectDraw / DISPDIB (Diego, 2026-11-02: "we no longer have cd audio, joystick and should not have directdarw"; "Let's delete code first, and then we analyse again.")

Deleted C conversions only. While HGSIM.DLL is loaded, an address whose conversion is gone runs the original code
again, so the game is unchanged where nothing reaches it, and /D3D (option 3) still gets the original DirectDraw
driver.

- **CD audio.** Music comes from the MUSIC folder (round 33). `HG_MUSIC_FILES` is now always on: its `#if` branches
  are resolved in w4u_cd.c, w5a_misc.c, w6d_calls.c, w7l_misc.c, w9d_misc.c, w5c_calls.c (the checker's
  `W5C_MUTATE12` negative check stays) and w4w_win.c. The define is gone from src/launcher/CMakeLists.txt.
  Deleted: src/sim/win/w4u_cdaudio.c, w6g_cd.c, w6g.h (the MCI wrappers Cd_* 0x1009dec0..0x1009eaa2,
  Aux_FindCdDevice, Sim_1009e441) and Cd_PollDisc (w7o_modes.c). Also deleted: the thunks Sim_1009e5a2 / e5ac / e5b9
  / e5c3 / eb24 / eb2e, which only called them.
  - Sim_1001f415 now sets the "CD open" pair [0x1017d060] / [0x1017d064] to 0 itself.
  - Sim_1001f47d no longer calls Cd_Shutdown.
  - hooks_sim.c's HookedCdAudioInit stays: the original-reference (noconv) runs need it.
- **Joystick** (Diego, 2026-11-02: "we don't support joystick"). Deleted the joystick driver's functions, which only
  the joystick driver table 0x101426f0 points to: Sim_1005621b, Sim_100567cb, Sim_10056d04, Sim_10056df3,
  Sim_10056dfa, Sim_100570e7, Sim_100570f1 and Joy_GetOemName. Deleted files: w27_joystick.c, w4v_joystick.c, w27.h
  and conv_w27.inc. Win_JoyGetNumDevs stays and returns 0, so the game finds no device and never calls the driver.
- **DirectDraw (display modes 0..3) and DISPDIB (mode 4).** Deleted the conversions that only those drivers' tables
  (0x1014a138, 0x10149e58) or their own original code reach. I checked each one against the DLL's pointers and
  direct calls, including the incremental-link jump stubs:
  - DisplayDibWindowEnd / Draw / SetFmt (w4u_gdi.c);
  - Sim_1007ef27, Sim_1007efb4, Sim_1007f0c8 (w4w_display.c, and its unused statics);
  - Sim_1007f0b4;
  - Sim_1007f950 and Sim_1007f9da (unsorted_1007/w11_ddraw.c and w11.h deleted);
  - Sim_10080471;
  - Bits_Count (view/objlist.c).
  Sim_1003775f was deleted by mistake and is restored, because the kept Sim_1003772b calls it.
- **Kept: the GDI window driver (mode 5).** SimMain always starts it first, before it switches to the renderer, so
  the Vulkan build uses it.
- **Not touched yet: the software-renderer branches.** The flag [0x1017b940] (renderer active) is tested in 178
  places in 87 C files. It is only set partway through SimMain, so code that runs earlier takes the `== 0` path even
  with Vulkan. Each site needs its own check before anything is deleted (next analysis).
- **Checker.**
  - difftest.py's build_c and w10_common.closure skip sources that no longer exist.
  - build_c links music_stub.c when Music_* symbols are missing.
  - spec_w7l / spec_w9d stub a symbol only if it exists.
  - Entries for deleted functions are commented out with a "deleted:" note. The three spec_w11_* files are deleted.
  - The 1.0 CD-version entries are superseded by spec_music_v12 (8 of 8 pass): Sim_1001f335 / f367 / f399 / f3ac /
    f26e / ee7c / edd0, Win_AuxSetVolume and Sim_1009ebeb.
  - The "no CD" entries of Sim_1001f4d3, Sim_1008b900 and Sim_1001f415 are retired.
- **Results.** 53 spec files, 1541 entries, 39 failures. Every failure also fails the same way on commit 9bec9e2
  (re-run there for the ones not already known: Sim_10047e1a, Sim_10041444, Sim_1004158c, Sim_100421b0,
  Sim_1005874a, Sim_10041b2d), so none comes from the deletions. The Release build compiles and links HG.exe and
  HGVK.dll. I did not run the game.

## KEYBOARD.DLL dropped; force feedback kept for rumble (Diego, 2026-11-02: "We can drop keyboard.dll and fforce.dll", then "No, don't drop it, I will add rumble later.")

- KEYBOARD.DLL (the language keyboard remap the sim loads in Sim_1005f6c6, with simtext.txt) is dropped: the keyboard
  stays the plain one. The code goes in the Windows-calls round (group 5, loading DLLs).
- Force feedback stays: the game's effect requests (TSFF_SIM_Effect and the 1.2 ports of rounds 33 / 35) are kept,
  for rumble later. Only the way they reach a device changes: FFORCE.DLL / TS_SFRCE.dll cannot load on Linux / Mac,
  so in the Windows-calls round the calls go to one small C interface of ours (no device for now), which the rumble
  work later connects to SDL3.

## Windows calls (Diego, 2026-11-02: "Start the changes and let me decide on vulkan later.")

The sim's C code called 96 Windows functions through HGSIM.DLL's import slots (SIM_API). The plan, by group (the
order agreed in the project thread): (1) dead code out, (2) memory, (6) timing, (7) debug output, (3) window /
keyboard / cursor / message boxes, (5) DLLs loaded at run time; (8), the C runtime's insides, goes with the DLL.
The Vulkan questions (HGVK built in, the loading screen through Vulkan, group 4) wait for Diego.

### Dead code (group 1)

- Deleted: Win_PumpMessage (0x10086b99), Win_PollMouse (0x1007d9d8), Win_CenterCursorAxis (0x1007d961) and
  Mouse_Unclip (0x1007d948): the SDL3 platform's versions (src/sim/platform, conv_w28.inc) take those addresses and
  these had their hooks commented out. Sim_101069c0 (VirtualAlloc, small-block heap): no hook, no caller.
- Win_AuxGetVolume (0x1009eb52, the CD drive's aux volume): no caller and no pointer anywhere in the DLL; deleted
  instead of turned into a music volume read.
- Their spec entries are commented out with a "deleted:" note.

### Memory (group 2)

- **Our heaps** (src/sim/crt/hg_winheap.c, standard C): Hg_HeapCreate / Alloc / ReAlloc / Free / Size / Validate /
  Destroy with Windows' meaning. Each block has a checked head and sits in its heap's list, so HeapDestroy frees the
  heap as a whole (SimMain makes seven heaps, [0x1017b8fc]..[0x1017b910], the game heap [0x1017b8f8] is one of them,
  and destroys them at its end).
- **The DLL shares them** (src/hooks/hooks_heap.c): HGSIM.DLL's import slots for the seven heap functions point at
  them, so the original code still in use (about four functions that allocate or free on the game heap, plus the
  DLL's own reads of HeapSize) and the C code use the same blocks. A heap Windows made before the slots changed (the
  DLL's C runtime heap [0x10ac3714], made while the DLL loads) is not ours: its calls go on to Windows'. "noconv"
  leaves the slots alone.
- **The C call sites** (37 files, plus the w8z / w8x / w4w_records macros and SimMain's SimPlat_Heap* wrappers) call
  Hg_Heap* directly. The C runtime's own heap code (w6i_mem, w6i_crt, w6j_heap, w6k_sbh, w7r_heap) keeps the slots:
  that heap is Windows' and goes with the DLL (group 8).
- **Differences (recorded):** a block never grows in place (HEAP_REALLOC_IN_PLACE_ONLY growth fails; Windows grows
  one when it has room after it; only the C runtime's realloc asks, on its Windows heap). A fixed-size heap
  (maximum != 0) caps the blocks' total without Windows' overhead; the game never makes one (all 10 HeapCreate calls
  pass 0). No lock, as the game's flag 1 (HEAP_NO_SERIALIZE) asks.
- **Checked natively** (tools/verify/winheap_native.c): against Windows' heap, 40 heaps, 800,000 random steps
  (alloc with and without zero fill, realloc, size, validate, free, destroy): no difference.
- **Checker:** tools/verify/stand_in/hg_winheap.c calls the slots as before, so the specs compare the code around
  the heap calls.
- **Specs** (the entries of every changed function, plus SimMain): 74 entries pass; Sim_1003dc19 in
  spec_w10_1003dc19.py fails as on commit fca1e91 (the same 1852 of 3000: a 1.0 spec of a function the network round
  moved to 1.2; its 1.2 spec spec_net_start_v12 passes). The Release build compiles and links; the game was not run.

### Timing, debug output, keyboard (groups 6 / 7 / part of 3)

- New platform functions (src/platform/plat.h, Windows backend in plat_win32.c; Linux / Mac still to do):
  Plat_TimeMs (timeGetTime), Plat_DebugText (OutputDebugStringA), Plat_KeyChar (MapVirtualKeyA type 2),
  Plat_KeyState (GetKeyState). On Windows each makes exactly the original call.
- Win_TimeGetTime, Debug_Printf (its "debugger" output) and Input_QueueKey use them. Specs: the three entries pass
  (the checker stand-ins in tools/verify/plat_stub.c make the original calls through the import slots).
- Not done, on purpose: the message boxes (Debug_Printf's box, Sim_1005573d, Win_ErrorBox, Sim_1008663f,
  Sim_100bc47e: with the screen group, Diego 2026-11-02 "do not do anything regardint the screen yet"); the
  QueryPerformanceCounter of the video-mode profiling (Sim_10037cab) and Sim_100381d0's timeGetTime (screen code);
  the debug lines of the force-feedback calls in w4w_net.c / w4v_dll.c (that code is replaced in the DLL group).

### DLLs loaded at run time (group 5): KEYBOARD.DLL, FFORCE.DLL

- **KEYBOARD.DLL** (Sim_1005f6c6): no longer loaded; the function takes the original's own path for a DLL that does
  not load (the key table [0x10143a00] keeps its built-in contents: no language-specific remap). Spec:
  Sim_1005f6c6_nokbd compares with the original given a missing DLL (its failing LoadLibraryA left out of the call
  comparison); 2000 trials pass. HGSHELL.DLL still loads KEYBOARD.DLL itself (the shell's own round).
- **FFORCE.DLL** (kept for rumble): the sim's three calls go to src/platform/force.h (Force_Present / Init / Effect /
  Exit, FFORCE.DLL's answers). force_none.c is the only version so far: no device, so the sim runs the original's
  path for a missing FFORCE.DLL: "Error loading TS_SFRCE.dll" as debug text, [0x1015ef3c] = 1 when the option is on
  (as before), every effect request answered 0x8004050f. The effect requests themselves (the 1.2 ports, the 1.2
  table W2k_FfTable12) are unchanged. The loader Sim_10045ec2 and the export lookup Sim_10045e5d are deleted (only
  each other and 0x10045d70 called them). Specs: Sim_10045d70_noff, Sim_10045db9_noff, Force_ExitSystem_noff pass
  (600 / 300 / 400 trials, against the original with FFORCE.DLL missing). A force-feedback joystick no longer
  rumbles on Windows (joysticks are not supported, Diego 2026-11-02).
- Release build compiles and links; the game was not run.

### The decompressors (group 5): pkware.dll / zlib.dll

- **What they were:** the game archive (HEAVYG.MAP / HEAVYG.PRJ, the same in 1.0 and 1.2) names its packing methods
  "pkware.dll" and "zlib.dll"; Sim_100bcc50 loaded each by that name and the pack reader (Sim_100bc70d) called their
  "Uncompress"(out, &size, packed, packedSize). Of 19,545 entries 15,824 are zlib streams, 1,419 PKWARE DCL
  "implode" data (all binary literals, 1 KB dictionary), 2,302 not packed.
- **Now** (src/sim/util/unpack.c): Unpack_Zlib calls zlib's uncompress and Unpack_Pkware calls blast() (zlib's
  contrib/blast, Mark Adler's PKWARE DCL decoder; it also reads the ASCII form, absent from the data), both from
  zlib 1.3.2's source in third_party/zlib (decompression files only, compiled into the program like OpenLibm;
  third_party/zlib/HG_VENDOR.md). Sim_100bcc50 puts
  them in the unpacker table by name (letter case aside), with a 0 library handle; a name that is neither gets an
  unpacker that always fails (the original retried LoadLibrary every 2 s for ever). Sim_100bd1b6 no longer frees the
  DLLs.
- **Checked natively** (tools/verify/unpack_native.c): all 17,243 packed entries give the DLLs' answer, size and
  bytes exactly; 12,930 damaged copies (cut short, one byte changed, one byte too little room) never make ours write
  past the room or crash.
- **Difference (recorded):** damaged data. The DLLs answer success for data cut short or bigger than the room (and
  can crash on it); ours answer a failure, so the pack reader takes its own failure path. The game's data is not
  damaged (every entry checked).
- **zlib + blast, bundled source** (Diego, 2026-11-02, decision card 19:40): the real zlib and its contrib/blast
  replace the decoders first written here. Considered: our own hand-written decoders (they also matched the DLLs on
  the whole archive, but are code nobody else maintains), miniz (one-file zlib work-alike; no DCL decoder),
  libdeflate (fast, but a bigger library for a one-shot load), StormLib's pklib (DCL only, pulls in a large
  library). zlib is the reference implementation of the format the archive names, and blast comes in the same
  release from the same author. Same native check as before, re-run on zlib 1.3.2: identical on all 17,243
  entries, no crash or overrun on the damaged copies.
- Specs Sim_100bcc50 and Sim_100bd1b6 are superseded (no DLL to compare with); the native check stands for them.
  HGSHELL.DLL is not touched (its own round).
- Release build compiles and links; the game was not run.

### The screen (group 3; Diego, 2026-11-02: "just focus on moving the code to sdl3 and keep the current setup. Only add Fullscreen or windowed and keep hud in 640*480 regardless of resolution. ignore vsync")

- **What moved to SDL3** (src/platform/plat_window.c, one implementation for every system; plat.h): the screen
  size (GetSystemMetrics), placing the window (SetWindowLongA / SetWindowPos / AdjustWindowRect, now
  Plat_WindowPlace: a framed window by its client rectangle, or borderless full screen without a display mode
  change), showing it (ShowWindow / UpdateWindow), the client rectangle (GetWindowRect / ClientToScreen), message boxes (MessageBoxA, GetFocus) and the video-mode timing clock
  (QueryPerformanceCounter). Call sites: Display_SetMode, Sim_100381d0, ToggleFullScreen, Sim_10037cab, SimMain,
  Sim_1005573d, Sim_100bc47e, Sim_10086c32, Debug_Printf, Sim_1008663f, Win_ClientRectToScreen; the w28 window
  procedure's Plat_WindowShow is SDL3's too. SDL3 wraps the game's window once (PlatSdl_Window), shared with the
  SDL3 input.
- **Kept the Windows way, on purpose:** the launcher still makes the window (CreateWindowExA) and routes its messages.
  The 1.2 shell, still the original HGSHELL.DLL, needs raw window messages; SDL creating the window would put its
  message handling in front of the shell's (the shell round). So SimMain's routing messages (SendMessageA 0x41e /
  0x41f / WM_ACTIVATEAPP) go through Plat_WindowSend and its queue flush through Plat_DropEvent, both in
  plat_win32.c with the window procedure's plumbing (AFTER_DLL.md).
- **Full screen / windowed (the one addition):** with the renderer (display class 3, Vulkan), Alt+Enter (the game's
  own key 0x40d, ToggleFullScreen) switches between borderless full screen and a framed window of the largest whole
  multiple of 640 x 480 that fits the screen, centred (1280 x 960 on a 2560 x 1440 screen), keeping the renderer.
  The original looked for a windowed display driver (the GDI one) and moved the whole display to it, which cannot
  show the 3D. The game starts as before (full screen). The HUD and the game's 2D stay laid out at 640 x 480 at any
  window size (HGVK scales the 4:3 picture, unchanged). No settings file, no vsync change.
- **Differences (recorded):**
  - The window rectangle the sim keeps ([0x109e3780..]) is the client rectangle, not the frame one (no
    AdjustWindowRect); only the position lock after the old DirectDraw driver read it, which the Vulkan game never
    sets.
  - A type-2 (windowed) display class whose rectangle covers the screen goes borderless full screen; the
    original's captioned window of the desktop's size looked the same.
  - The pointer goes through Plat_CursorVisible / Plat_CursorShown, but stays Windows' display count (ShowCursor,
    plat_win32.c) while the launcher's Windows window is the game window. A first version used SDL3's
    SDL_HideCursor: Diego's play test (2026-11-02) showed the pointer after Alt+Tab, because SDL's hiding holds only
    while SDL sees the mouse in the window and the game drops the mouse messages while the pointer is hidden
    (input_sdl3.c MessageAllowed), so Windows put the window class's arrow back. The count hides it whatever has
    the focus, as in 1.2. The loops become one call each ("show" / "hide"); SimMain's single ShowCursor(TRUE) at
    its end now shows it fully.
  - GetDeviceCaps(NUMCOLORS) == -1 (a true-colour screen) is always taken: no 256-colour palette device is left.
  - Debug_Printf's box, the fatal box (Sim_100bc47e: minimised, not "minimised without activating") and the error
    boxes are SDL3 message boxes with the same buttons and answers.
  - Unchanged on purpose (1.2's behaviour, "keep the current setup"): losing the focus still minimises the game
    window, also when Alt+Enter made it a framed window.
- **Deleted:** Win_ErrorBox (0x100d9fc5: MessageBeep, wvsprintfA, MessageBoxA), Sim_10083d5b (0x10083d5b, a GDI blit
  below the menu bar), Sim_1008375a (0x1008375a, the GDI
  driver's DIB section) and Sim_10083e4e (0x10083e4e, its second stretch): no caller and no pointer in the DLL.
- **Checker:** tools/verify/plat_stub.c stands in for the new platform calls with the original's Windows calls where
  there is one (screen size, show window, pointer loops, message box, send, peek, counter). The pointer loops stop
  after 64 calls: spec_w21_simmain's ShowCursor keeps answering "hidden", and SimMain's last ShowCursor(TRUE), one
  call in the original, is Plat_CursorVisible(1) now, whose stand-in loop otherwise never ended (a first run hung
  for an hour). The entries whose Windows
  calls changed shape are superseded with a note (Display_SetMode, Sim_100381d0, ToggleFullScreen, Sim_10037cab,
  Win_ClientRectToScreen, Sim_1005573d, Sim_100bc47e); Debug_Printf, Sim_1008663f, Sim_10086c32, Sim_100840dc,
  Sim_10055048 and the 1.2 window procedure (Sim_100867a2_v12) pass. SimMain passes, 4000 / 0
  (spec_w21_simmain.py: the original's GetDC / GetDeviceCaps / ReleaseDC leave its call log as they happen, so the
  stubs that answer by log position answer both sides alike; it is given a true-colour screen; runs of the same
  ShowCursor call count as one; the texture table's mapping is applied after that). The 1.0 window spec
  (spec_w28_window.py) builds again (it linked plat_stub.c twice) and differs from 1.0 only by 1.2's focus pause,
  which spec_w28_window_v12.py checks against 1.2.

### The loading screen through Vulkan (Diego, 2026-11-02: "include the loading screen in hgvk now")

- **What it was:** SimMain starts the windowed GDI display driver (mode 5) first for the loading screen, before the
  renderer: an 8-bit picture of the screen's size with a 6-bit palette, drawn with GDI (GetDC, CreatePalette /
  SelectPalette / SetPaletteEntries / RealizePalette, SetDIBitsToDevice, StretchDIBits, ReleaseDC, DeleteObject).
- **Now** (src/platform/picture.h, picture_hgvk.c; HGVK's new export HgvkPictureApi, src/plugins/hgvk/picture_api.h):
  the driver's functions keep all their own logic and data, and only their GDI calls go to the picture: Sim_10083530
  (start: Pic_Open, Vulkan started on the game window), Gdi_SetPalette (Pic_SetPalette), Sim_10083c45 /
  Gdi_BitBltRect / Sim_10083801's rolling blit (Pic_Blit), Sim_10083de1 (Pic_Stretch), Sim_10083692 (Pic_Close).
  The window's picture is kept in 32 bits on the sim's side (as the window kept what GDI drew) and each drawing call
  presents it through Vulkan, scaled to the window with the picture's own proportions (Vk_PresentPicture). Loading
  stays single-threaded, in the original's order. HGVK's FirstDevice keeps the Vulkan the loading screen started (one
  device, one swapchain on the window); LastDevice ends it as before.
- **Vulkan only:** when Vulkan cannot start, the player gets a message box ("Heavy Gear could not start Vulkan ...",
  see hgvk.log) and the program ends; there is no GDI fallback.
- **Checked natively** (tools/verify/picture_native.c): the three drawing calls, made with the original's exact GDI
  arguments (SetDIBitsToDevice of a rectangle with its bottom-up source y, of the whole picture, the rolling pair
  with a start scan line; StretchDIBits of a part over the whole) into a 32-bit GDI bitmap, against the picture code
  on random pictures and palettes (640 x 480, 320 x 200, 100 x 37, 36 x 64): 240 calls, the same pixels every time.
  The stretch takes the source pixel under each pixel's centre, as GDI does.
- **Differences (recorded):** the drawing never fails (the original's GetLastError reports are not reached, nor the
  driver's "start failed" path after a failed blit); the palette and device-context marks ([0x1017b3c0],
  [0x1017b3c4]) are 1, not GDI handles; a picture whose width is not a multiple of 4 (a 1366-wide desktop) shows its
  rows as the game packs them, where GDI read 8-bit rows rounded up to 4 bytes and sheared the picture (an original
  bug, fixed); each drawing call presents, so a palette fade waits for the screen's refresh per step (FIFO) where
  GDI drew at once.
- **Checker:** the driver's entries are superseded (Gdi_BitBltRect, Gdi_SetPalette, Sim_1008400d, Sim_10083c45,
  Sim_1008404b, Sim_10083530, Sim_10083692, Sim_10083801, Sim_10083de1); the native check stands for them.
  tools/verify/picture_stub.c links the picture calls in the checker's image.
- Release build compiles and links (HG.exe, HGVK.dll); the game was not run.

## The last original code (Diego, 2026-11-02: "okay, do that": steps 1-3 of the plan toward dropping HGSIM.DLL)

### Step 1: original code still called from the C

The count of "about 50 original functions" was of addresses the C calls through; checked one by one, most were
converted already (jump-table addresses of converted functions, or addresses our C takes over through other hook
files). What ran as original 1.0 code, and what changed:

- **Jump-table addresses of converted functions, now direct calls:** the three collision cell tests (0x100035df,
  0x10001e1f, 0x10002775 = Sim_1002d3ca, Sim_1002d479, Sim_1002cde1; in w3o_ray.c and w3s_cell.c) and the
  projector's clip-edge and vertex callbacks (0x1000110e, 0x10001a64, 0x1000222f = Sim_100a6009, Render_ClipEdgeFar,
  Sim_100a5d25; w2n_project.c). Sim_1002d3ca and Sim_1002cde1 now take the callers' shape (object, x, y, z, &out).
  - **Bug fixed on the way:** Sim_1002d3ca was declared as returning nothing; the original returns 0x10031f95's
    answer (its eax), which the callers test. It now returns it (spec_w5x compares the result).
  - **Left as they are:** 0x100020e5, 0x10002513 and 0x10002964 are not calls but callback addresses handed to the
    list walks (stored and compared as data, logged by the specs); they become plain C function pointers at the
    switch-over (AFTER_DLL.md).
- **The view-detail hooks (hooks_view.c) call our C:** the camera (0x100d1f90, 0x100d2306), the vertex projection
  (0x100a62c4), the texture options (0x100988fc, 0x100989a1), multiple lights (0x100585c4) and shadows (0x100ac603)
  ran the original 1.0 code behind trampolines although converted versions existed; the hooks now call those (no
  trampolines left). The texture-option wrappers return 1, the value the original leaves in eax. With "noconv" the
  detail hooks are now off (they would run converted code in the untouched run).
- **Converted:** AllocPolyBuffers 0x100a8816 (Sim_100a8816; the view hook calls it with its larger buffers, and
  conv_w5x.inc hooks it when the view hooks are off), the dissolve 0x100eda16 (Sim_100eda16, a hand-written assembly
  transition copying pixels in shift-register order) and the colour-remap copy 0x100e96a7 (Sim_100e96a7), both in
  src/sim/gfx/w37_draw.c with conv_w37.inc.
- **Exit:** the error boxes' exit (0x100ff3b0) calls the converted Proc_Exit directly.
- **Deleted:** Sim_1008663f (the original SimMain's __finally block, the only user of AbnormalTermination
  0x1010026a): only the original SimMain reaches it; the converted SimMain has its own W21_FINALLY.
- **Already ours, nothing to do:** the empty debug print 0x10083495 (the damage log's hook owns its address).
- **Not done here, recorded:**
  - 0x1008181e (DirectDraw: Unlock / IsLost / Restore / colour-fill Blt of the back buffer) and the software
    renderer's polygon routines 0x100e4780 / 0x100e5477: reached only without the Vulkan renderer (SimMain's two
    calls need [0x1017b940] set with [0x1017b944] clear or the DirectDraw display class; w22_poly.c's under
    [0x1017b940] == 0). There is nothing to convert them to (DirectDraw and the software renderer are dropped);
    removing their branches goes with the software-renderer branch clean-up (above: "Not touched yet: the
    software-renderer branches", each site checked on its own).
  - 0x10103ad0 (the C runtime's x87 control word) and 0x101069c0 (its small-block allocator): inside the 1997 C
    runtime's math-error and realloc paths, with step 3.
- **Checked:** Sim_1002d3ca 300 / 0, Sim_1002cde1 400 / 0, Sim_1002db2d 1500 / 0, Sim_1002d961 500 / 0,
  Render_ClipPolygon 1500 / 0, Render_ProjectVertex 1500 / 0, Sim_100d1f90 1500 / 0, Sim_100d2306 800 / 0 and
  3000 / 0, Sim_100988fc 400 / 0, Sim_100989a1 300 / 0, Sim_100585c4 400 / 0, Sim_100ac603 400 / 0, Sim_100a8816
  500 / 0 (new), Sim_100eda16 3000 / 0 (new; 177 trials end in the instruction limit on both sides: the original
  loops for ever when no pixel lands inside both panes), Sim_100e96a7 500 / 0 (new). spec_w3s / spec_w3o stub the
  three cell tests at their C entries as at the jump addresses. Release build compiles and links.

### Step 2: the small Windows calls

- **New platform functions** (src/platform/plat_system.c, standard C and SDL3, every system; plat.h):
  Plat_BlockAlloc / Plat_BlockSize / Plat_BlockFree for SimMain's GlobalAlloc(GMEM_FIXED, 0x249f0) / GlobalSize /
  GlobalFree (the pack reader's read buffer [0x1017b958], its size [0x1017b95c]; a malloc block with its size in
  front), Plat_MemoryStatus for GlobalMemoryStatus (the RAM that sizes the first game heap; SDL_GetSystemRAM, capped
  at 4 GB - 1 as Windows caps it for a 32-bit program; the other fields, which the game does not read, filled
  plainly) and Plat_SystemTime for GetSystemTime (UTC, SDL3's calendar). sim_platform_iat.c keeps only the game
  heaps.
- **Deleted:**
  - SimMain's SetEnvironmentVariableA("FX_GLIDE_NO_SPLASH", "1"): only 3dfx's Glide library read it (Vulkan only).
  - The display shutdown's (Sim_100b8938) timeKillEvent / timeEndPeriod of [0x1017d5cc]: only the original Windows
    loading animation started that timer; ours (w28_anim.c) never does.
- **Not done, a question for Diego:** CoUninitialize (0x10ac58a4) is in the converted DirectSound shutdown
  (w11_sound.c Sim_1003a548), which runs only with the hgrc.cfg word "dsound" (the DirectSound sound kept for
  comparison; the game's sound is SDL3's SndSdl_1003a548). Deleting it alone would leave that path's COM
  initialisation open; it goes with the DirectSound path when that is dropped.
- **Checker:** tools/verify/plat_stub.c stands in with the original's calls (GlobalAlloc / GlobalSize / GlobalFree /
  GlobalMemoryStatus / GetSystemTime through the import slots); spec_w21_simmain.py takes the original's
  SetEnvironmentVariableA out of its call log as it happens (as GetDC); spec_w8y.py gives Sim_100b8938 no timer.
  SimMain 4000 / 0, Sim_100b8938 300 / 0. Release build compiles and links.

### Step 3: the C runtime's Windows calls (checked, nothing to port before the switch-over)

The 51 Windows imports still called through the DLL after steps 1 and 2 (49 after them), read again call site by
call site:

- **The 1997 C runtime's insides (about 40):** CompareString / LCMapString / GetStringType / MultiByteToWideChar /
  WideCharToMultiByte / GetCPInfo / GetACP / GetOEMCP (locale and multibyte tables), Get / FreeEnvironmentStrings,
  SetEnvironmentVariableA (putenv), GetModuleFileNameA (argv), GetStdHandle / SetStdHandle / GetFileType /
  GetStartupInfoA / SetHandleCount / WriteFile / WriteConsoleA (start-up stdio and the runtime-error text), its own
  heap (HeapCreate / HeapAlloc / HeapReAlloc / HeapFree / HeapValidate / HeapDestroy, VirtualAlloc / VirtualFree for
  the small-block heap, with the small-block allocator 0x101069c0), GetTimeZoneInformation, FileTimeToSystemTime /
  FileTimeToLocalFileTime, RaiseException and the x87 control word 0x10103ad0 (the maths error path),
  GetModuleHandleA / GetProcAddress (the FDIV check), LoadLibraryA / GetProcAddress (the runtime-error box's
  user32), GetLastError. As round 37 found ("The rest", above), none of these is reachable from the game's C: they
  serve only the DLL's own C runtime start-up and the original code's library calls, and the game's C uses our C
  runtime (src/sim/crt, OpenLibm, plat_*.c). They need no port: they go with the DLL at the switch-over. Earlier
  wording of the plan ("about 40 Windows calls go to our C runtime") was wrong; corrected to Diego.
- **The plugins and the network library (6 call sites):** LoadLibraryA / GetProcAddress / FreeLibrary /
  GetModuleHandleA for the renderer plugin (w6e_misc.c, w7m_misc.c, w4u_heap.c) and ANET2.DLL (w5y_misc.c,
  w4a_heap.c, w4w_records.c, the 55 names of w11_sound.c): Diego's decisions 4 (HGVK built in) and 5 (network).
- **Render set-up** (Sim_10034cca): GetVersion and VirtualProtect make a range of the DLL's own code writable for the
  original code's run-time patches; it goes with the DLL.
- **The DirectSound comparison path:** CoUninitialize (step 2, question to Diego).
- **Checked again function by function** (Diego: "you deleted what was not needed in step 3?"): every C function of
  ours that still makes one of these calls, its hook and its C callers. Most are hooked over the DLL's own C runtime
  routines and called only by the DLL's code (they stay until the switch-over, AFTER_DLL.md). Three were not as
  stated above:
  - **Time_FromFileTime** (0x100fe6e0) is reached from the game's C (Crt_FindFirst / FindNext, the file times):
    ported. Its FileTimeToLocalFileTime + FileTimeToSystemTime are Plat_FileTimeToLocal (plat_system.c, standard
    C localtime: the system's own local-time rules for each date, where Windows applied the current bias to every
    date). Spec 600 / 0 (plat_stub.c makes the original's two calls).
  - **Heap_Check** (0x101000d0, _heapchk: HeapValidate / GetLastError of the DLL C runtime's heap) is reached from
    the game's debug heap check (w7n_misc.c). That heap holds only the DLL's start-up blocks (the game's go to
    crt/hg_heap.c), so it stays until the switch-over, when it checks our heap instead (AFTER_DLL.md).
  - **Sim_10106660** (a new small-block region) was neither hooked nor called from C: deleted (spec entry
    commented; spec_w7s skips stubs of symbols that no longer exist). Specs that build w6k_sbh.c / w7s still pass
    (Sim_10106960 400 / 0, Sim_10104970 600 / 0).
- Release build compiles and links.

## HGVK built into HG.EXE (Diego, 2026-11-03: "build vulkan inside hg.exe and drop directsound")

- **What:** the Vulkan renderer's sources (src/plugins/hgvk: hgvk.c, vk.c, hud2d.c) are compiled into HG.EXE;
  HGVK.DLL, its CMakeLists.txt and hgvk.def are gone, and tools/dev/dev.cmd no longer copies it (Diego allowed that
  one edit).
- **How the sim finds it:** unchanged on the sim's side. It still loads "plugin\HGVK.DLL" (DLLFILE.INI, or every
  plugin\*.dll) through the platform layer (w28_plugin.c Sim_1003650d). The platform keeps a table of libraries built
  into the program (Plat_AddBuiltinLibrary, plat_win32.c): a name whose file part is "HGVK.DLL" (letter case aside)
  gives the built-in renderer, whose entry points Hgvk_Function answers by name; any other name still loads a real
  DLL. HG.EXE registers it at start (Hgvk_RegisterBuiltin). The entry points are static functions of hgvk.c (no
  exports, no name clash with the game's symbols).
- **Through the platform now** (were LoadLibraryA / GetProcAddress / FreeLibrary / GetModuleHandleA): the two plugin
  probes (Plugin_ProbeRaster 0x10036027, Sim_100361d8), the plugin release (Sim_1003693c), the HUD hand-over
  (hooks_hud.c) and the loading screen (picture_hgvk.c, already).
- **Vulkan through SDL3:** SDL3 loads the Vulkan loader and gives the instance extensions and the window surface
  (SDL_Vulkan_GetVkGetInstanceProcAddr, SDL_Vulkan_GetInstanceExtensions, SDL_Vulkan_CreateSurface) and the window
  size in pixels (SDL_GetWindowSizeInPixels, Vk_WindowSize) in place of vulkan-1.dll by LoadLibraryA, the Win32
  surface and GetClientRect. SDL3's wrap of the game window (plat_window.c PlatSdl_Window) is made with SDL's Vulkan
  flag; without a Vulkan loader it is made without it (input and the window still work) and the renderer reports why
  in hgvk.log, then the game ends with its "could not start Vulkan" box.
- **hgvk.log** is opened by its first line (the DLL opened it when loaded), so only the process that uses the
  renderer writes it.
- **Kept as they were (Windows-only, AFTER_DLL.md):** in hgvk.c the F12 dump key (GetAsyncKeyState), the texture
  read checks (IsBadReadPtr), the 2D buffer's height from GetSystemMetrics(SM_CYVIRTUALSCREEN) and the GDI fallback
  (PresentGdi) for a missing Vulkan.
- **Checker:** plat_stub.c's library stand-ins now make the original's LoadLibraryA / GetProcAddress / FreeLibrary
  (the round-28 specs hook these symbols). Plugin_ProbeRaster 500 / 0, Sim_100361d8 600 / 0, Sim_1003693c 300 / 0,
  Sim_1003650d 8000 / 0, Sim_1003695b 3000 / 0 (219 trials end in the instruction limit on both sides, as before).
- Release build compiles and links (HG.EXE only); the game was not run.

## DirectSound dropped (Diego, 2026-11-03)

- The hgrc.cfg word "dsound" (the converted game with the original DirectSound sound, for comparison) is gone:
  sound is SDL3's in every converted run (HooksAudio_SdlSound: "sdlaudio", or anything but "noconv"). The original
  DirectSound runs only inside the untouched original ("noconv").
- **Deleted:** the converted DirectSound versions that only that path used: Sim_10039708 (pitch), Sim_1003a548
  (shutdown, with CoUninitialize), Sim_10039931 (3D voice position), Sim_100397e7 (3D listener) in w11_sound.c, their
  conv_w10.inc entries, hooks_conv.c's choice between them and the SDL3 versions, and their spec files
  (spec_w11_10039708 / 100397e7 / 10039931 / 1003a548.py; the line in crt_math_specs.txt). sim/sound/sdl_sound.c is
  unchanged. Release build compiles and links.

## Software and Direct3D sides removed (round 42; Diego, 2026-11-03: "Remove them")

- **The renderer flag:** [0x1017b940] (1.0: set by "/d3d", or by SimMain for "/3dfx" once the display is set up) is
  now set together with [0x1017b944] when CmdLine_Parse reads "/3dfx" (our Vulkan renderer's slot; the launcher
  always passes it). "/d3d" is still recognised but does nothing; the display switch "Z=" (Z=H: renderer, else
  software) is ignored. SimMain still sets the flag with the "3dfx" display set-up, as 1.0 does.
- **SimMain:** the renderer plugin is always the "3dfx" slot (S(0x1014b1f4), 2), where 1.0 picked the D3D slot for
  [0x1017b940] and a software one without either flag; the test that cleared [0x1017b940] for a plugin whose record
  +0x44 is not 1, the software display set-up, the two 0x1008181e screen calls, the driver-less loop and the 8-bit
  line / clear calls are gone.
- **Everywhere else** (137 flag tests in 96 converted functions, through the macros W28_FITS and DISPLAY16 too):
  only the renderer side is left. The 8-bit routines (lines 0x100e86a1, shapes 0x100e91dc / 0x100e96c6, palettes
  0x100e96a7, text 0x100ecea6, pixels 0x100e84f0, polygons 0x100e4780 / 0x100e5477 / 0x100f9fd4, the clear
  0x100eb070, the fade 0x100eda16) are no longer called by the C on those paths; the 8-bit HUD palettes
  (0x10178b60..), the software vertices (0x101ce4a0) and the window procedure's small-screen minimise handling go
  with them. Other 8-bit calls that the original made in every mode (for instance the single-point plot in
  Sim_100a539e) are unchanged.
- **Deleted converted functions** that only those paths reached (nothing else in the C, HGSHELL or the DLL's data
  refers to them): Clip_Polygon2D (0x100a2d91, file w9e_clip.c with its header), the GIF screenshot chain
  Sim_1004ec60, Sim_1004ecb8, Sim_1004ed37, Sim_1004edb6, Gif_Finish (0x1004e838), Gif_CloseFile (0x1004e886),
  Sim_1004de34, the refresh-mode test step Sim_10037cab with Sim_10037e8f, and 1.2's software-mode spark updates
  Sim12_1007c996 / Sim12_1007ca8b / Sim12_1007cb80; their CONV lines and spec entries too. Screenshots are TGA only.
- **Launcher:** ChooseRenderer picks hgvk for every converted run; glide / d3d / soft stay only with the hgrc.cfg
  word "noconv" (the reference runs of the original code: dx5noconv, ref12). dev.cmd (Diego, 01:39): mode dx5 (the
  converted game with Direct3D) is gone; menu option 3 is now dx5noconv.
- **Checker:** difftest.py sets [0x1017b940] and [0x1017b944] to 1 for every trial (after the generator); a spec
  with 'renderer_free' keeps its generator's values. SimMain's spec is one: it starts as after "/3dfx" (1.0:
  [0x1017b944] only), since its command line is a stub. CmdLine_Parse's spec gives "Z=" the letter H only (1.0's
  other letters would clear the flag the C no longer touches). spec_w32_explode always has the 3D mode.
- **Not changed:** the header comments that describe the original's [0x1017b940] tests still describe the
  original (both sides); the C follows the set side.
- **Checks** (16 processes): the 102 changed functions' entries. Pass: SimMain 4000 / 0, CmdLine_Parse 2500 / 0,
  Screenshot_Save 400 / 0, the 7 particle functions (1.2) 3000 / 0 each, Render_SubmitClipped_100ac738 and
  Sim_100ad1f3 (1.2) 3000 / 0, Sim_100352d5 (1.2) 3000 / 0, Sim_100cd70f (1.2) 3000 / 0, Sim_10058c5e (1.2)
  5000 / 0, Sim_100867a2 (1.2) 20000 / 0, Sim_100a0880 4000 / 0, Sim_10034ec9 1000 / 0, and every other entry of
  the list. Not passing, as before this round: the 1.0 specs of functions ported to 1.2 (Sim_100cd70f in spec_w9g
  and spec_w21_wrap_cd70f, Sim_10058c5e, Sim_100867a2: their 1.2 specs above pass) and the long-known Sim_100a0063
  1952 and Sim_100a0def 248 (the same counts as recorded in the maths round).
- **Spec changes for the checks:** spec_w8y's Sim_100a4ff0 / Sim_100a512a leave out g_w33Light1Cam /
  g_w33Light2Cam (C-only: 1.2's light directions in the camera frame, written by Sim_1005874a during the object
  walk; checked against 1.2 in spec_w3p_light_v12): a -0.0 there was the only difference (6 of 500 trials).
  spec_w22_100a0880 stubs 0x100a88db by address (the C no longer calls Sim_100a88db). tools/verify/w9e_shim.c's
  bodies are weak, so spec_w9e links again when the checker adds a real file defining the same function (it had
  not linked since the memory round).

## Platform layer off Windows, rounds P1 / P2 (Diego, 2026-11-03: "do changes"; recommended choices: 1 ms clock steps, assert box folder + "HG")

- **Moved to SDL3** (src/platform/plat_system.c; were Windows calls in plat_win32.c):
  - Plat_TicksMs: SDL_GetTicks cut to 32 bits (was GetTickCount: since Windows started, about 15.6 ms steps; now
    since SDL started, 1 ms steps). Its callers only take differences. Plat_TimeMs stays timeGetTime (round P3).
  - Plat_DebugText: SDL's log (on Windows still the debugger); one trailing line end of the text is dropped, since
    SDL adds its own.
  - Plat_LoadLibrary / Plat_LibraryFunction / Plat_FreeLibrary: the built-in table first (unchanged; the name
    compared with SDL_strcasecmp), then SDL_LoadObject / SDL_LoadFunction / SDL_UnloadObject. Freeing NULL does
    nothing.
  - Plat_CurrentDir: SDL_GetCurrentDirectory without its separator at the end (a drive's root keeps "C:\"), the
    size needed when the buffer is too small, as GetCurrentDirectoryA. On Linux / Mac "C:" is put in front (the
    loading animation, w28_anim, takes the first character as a drive letter; plat_file.c drops "X:" again):
    compiled there only, untested until a Linux / Mac build exists. Difference: a folder name with non-English
    characters comes as UTF-8 (Windows: the ANSI code page); the game's folders are plain ASCII.
  - Plat_ProgramPath: SDL_GetBasePath + "HG" (was the full path of HG.EXE); only the C runtime's assert box shows it.
- **Moved to SDL3** (src/platform/input_sdl3.c, next to the key table):
  - Plat_KeyChar (was MapVirtualKeyA type 2): the sim's key code back to its SDL scancode (the first one in
    input_sdl3.c's table), SDL_GetKeyFromScancode without modifiers, the English letters upper case and the rest as
    Windows-1252 (Windows keeps ñ / ç lower case); the keypad's codes give '0'..'9' * + - . / as Windows does.
    **Difference (a fix):** input_sdl3.c sends the US position's key code for a physical key, which the Windows
    lookup then read with the user's layout's meaning: on a Spanish keyboard 8 punctuation keys typed another
    key's character (the Ñ key typed `). The new lookup answers for the physical key, as the original game did with
    Windows' own key codes. US keyboards: no change.
  - Plat_KeyState (was GetKeyState): SDL's modifier state in GetKeyState's form: 0xFF80 while Shift / Ctrl / Alt is
    held, bit 0 for Caps / Num Lock toggled. Measured: Windows answers 0xFF80, so the game's 0x1000 test in
    Input_QueueKey does see the modifiers (they are added to the typed character, as in the original).
  - input_sdl3.c defines the Windows key-code numbers it uses itself (when windows.h is not there).
- **Check** (tools/verify/plat_native.c, native on Windows, old against new): 164 checks, 0 differences: the current
  folder, a drive's root, a too-small buffer, the program folder, libraries (kernel32, winmm, a missing function and
  library, the built-in renderer through a path), the clock over a 1 s sleep (1000 / 1000 ms), the debug text; every
  key code our input produces against Windows' character of the same physical key under the US and the Spanish
  layout (loaded for the test only, unloaded after), 2 Spanish dead keys; GetKeyState against the new state for
  nothing / Shift / Ctrl / Alt held and the locks toggled. No game file changed (the checker stands in for the
  platform layer), so no game spec was re-run. Release build compiles clean.
