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
