# After HGSIM.DLL is gone

A running list of things done one way only because the converted code still runs inside the original HGSIM.DLL
(its fixed data addresses, remaining original functions, the byte-for-byte checker). Each entry: what, where, why it
exists, what to do once nothing original runs. Add to it every round.

**Order (Diego, 2026-11-01: "before the dll goes, I want to be sure to port all 1.2 to the C"):** every 1.2 change
is ported into the C and proven against 1.2 with the checker first; only then does the work of dropping HGSIM.DLL
start (the checker needs the DLL to compare against).

## Memory and data

- **Data at fixed DLL addresses.** `SIM_PTR` / `G32` / `U32` / `SIM_API` (src/sim/simmem.h) reach the game's data at
  its 1.0 addresses through `g_simBase`. Exists because original code still shares that data. After: turn each table
  and global into ordinary C data with names and types, and drop `g_simBase` / `SIM_PREFERRED_BASE`.
- **Layouts kept for original code.** Records keep their 1.0 shape (field offsets, padding, the 1.0 object record
  with its +0x18 dword) because original functions read them. After: reshape freely. 1.2's objects lack that dword
  (fields from +0x1c sit 4 bytes earlier); the 1.2 specs shift them for 1.2's run (w7l_order, w9h_group,
  w16_range and others), a mapping step that goes with the checker.
- **Texture table** (1.0 at 0x10a51ac0, 512 x 0x300; 1.2 layout 512 x 0x380, frames of 0x1c). Moved out of the DLL
  into our own memory with the 1.2 layout (round T), because the luma table at 0x10ab1ae0 sits right after it. The
  pointer the sim main loop keeps in it ([0x1015dafc], w21) goes with it. After: an ordinary table of ours in the
  cleanest shape; drop the checker's 1.0 <-> 1.2 mapping step (specs w2h, w2j, w2l, w2m, w2n, w3q, w7l, w7q, w21).

- **The 14-byte texture records** (0x10ac1b00) and everything else the texture code reads stay at 1.0 addresses.

- **HGCAR.CFG record split** (round 33, round 3; Diego: "option 1"): 1.2's record is one 0x21c-byte block (bytes
  0..0x14c, the 10 x 10 kill table at +0x14c, two dwords at +0x214). Ours stays split: 1.0's 0x1d4 bytes at
  0x101653e0 (the two dwords at +0x1cc / +0x1d0, used at those addresses by converted code) and the table in
  g_w34Kills. Sim_10025dac builds the 0x21c image to save it; the mission-start clears Sim_10021ac0 / Sim_10025d93
  clear both. After: one record in 1.2's 0x21c layout in memory, the save writing it as it is (no conversion).

- **Vulkan counted as 3DFX** (round 33; Diego: "keep the name"): sim main sets 1.2's 3DFX / PowerVR mode for a
  renderer path holding "HGVK", besides 1.2's "3DFX" / "POWERVR". After: the renderer choice is a setting of ours,
  not a test on a DLL name.

## Calls into original code

- **Original functions called by address.** `SIM_FN(type, 0x...)` calls still-original code (292 distinct addresses
  from 246 source files today), including the DLL's own C library copies (e.g. 0x100fcb08 float-to-int,
  0x100fd7e0 formatted output). After: every target converted or replaced by our own / the standard library.
- **Calling-convention tags.** Converted functions keep `__stdcall` (95 files) and exact cdecl argument shapes
  where original code calls them or the checker drives them. After: plain C signatures.

## Hook layer

- **Hook engine and tables.** src/hooks (hook.h, hooks_conv.c, 88 `conv_*.inc` tables) patches the DLL so its
  functions jump into our C. After: gone; our executable calls its own code directly.
- **noconv switch.** Runs the untouched original DLL for comparison. After: gone.
- **HUD handover** (src/hooks/hooks_hud.c): finds HGVK.DLL's HUD interface by name and hands it to the converted
  drawing library (w30_hud.c). After: the renderer's HUD interface linked in directly.
- **Shell focus after a movie** (round 33, src/hooks/hooks_ddraw.c): HGSHELL's DestroyWindow import is replaced;
  when 1.2's shell closes its movie popup the game window is given the focus back (the shell draws only while it
  has it), plus logging of its window creation / destruction and focus messages. After: whoever converts the shell
  owns its windows; the movie player gives the focus back itself.
- **Movies without the CD** (round 33, src/hooks/hooks_cd.c): HGSHELL's drive search is answered with a phantom
  CD-ROM letter whose video\ is the game folder's. After: the converted shell reads the movies from the game folder.
- **Shell focus after its start flush** (round 33, src/hooks/hooks_cd.c): HGSHELL's PeekMessageA and GetFocus
  imports are replaced; after the shell's start routine drops its 10 waiting messages, WM_SETFOCUS is posted (or the
  focus given) so a menu restarted after a mission draws; its focus test is logged. After: the converted shell sets
  its focus flag itself when it starts.
- **Shell movies** (round 33): under the SDL3 display 1.2's DirectShow movies play with sound only (parked). After:
  the video round plays the movies ourselves from re-encoded copies.
- **HGVK as a plugin DLL.** Loaded through the original plugin table (0x109e3d24..0x109e3d64, names in
  docs/renderer/plugin-api.md) with its export interface. After: built into the executable, called directly.
- **Windows calls.** The platform layer (src/platform) is shaped for other systems; only Windows is filled. Windows
  calls left in original functions, the launcher, the hooks and HGVK are listed in DECISIONS.md (rounds 27-28).
  After: fill the other platforms.
- **Network code.** Parked, to be rewritten rather than converted (DECISIONS.md, network).
- **Network transport: our own, no DirectPlay** (Diego, 2026-11-01: "we want to make the multiplatform, can't use
  directplay"). The sim's network code keeps 1.2's message format byte for byte (the network round ports 1.2's
  sim side as is); the transport under it (today DirectPlay through ANET2.DLL, loaded by the launcher; HGSHELL's
  lobby uses it too) is replaced by our own, multiplatform. No play against the original 1.2 game. The options and
  the reach of DirectPlay: DECISIONS.md, network round.
  Diego's choice (2026-11-01): option 1, our own ANET2 stand-in (the ~35 dp functions the shell and the sim use) on
  plain OS sockets (UDP) behind the platform layer, HG.EXE swapping the shell's ANET2 imports as for DirectDraw, the
  original lobby unchanged; after the 1.2 network port, plan to Diego first.
  Deferred (Diego, 2026-11-01: "only do the changes for hgsim, I don't want to rewrite the network library yet"):
  the original ANET2.DLL stays for now; the read-only notes for the stand-in are tools/v12/anet2_shell.md.
  Multiplayer testing waits until the whole game is done (Diego, 2026-11-01: "I wont tests multiplayer until the
  whole game has been completed"): the network round's code (DECISIONS.md, network round) is so far only checked
  against 1.2 in the checker; test it in real play then.
  The stand-in's "player left" notice: the 2 bytes "d2" then the player's key word (the loop's NETLEFT_FIX
  reads the key after the tag; Diego's option B).
- **1.2 network changes to keep in the rewrite** (Diego: "do as 1.2 does, we can rework network later, but remember
  the change"):
  - Part destroyed (1.2 0x1005ab68, ported in round 32 in 1.2 mode): a kind-4 unit's part 2 destroyed by the local
    player in a network game sets off 16 shockwaves (6 at node +0x4c0, 10 at node +0x4c4) and scores the kill, without
    marking the part.
  - Kill tally and rating (round 33, ported; DECISIONS.md): 1.2's 10 x 10 kill table and ratings as C arrays
    (g_w34Kills, g_w34Rating), messages "KI" (0xc bytes) and the new "AK". Still to port with the rest of 1.2's
    message loop (1.2 0x10031a86): its other new messages ("DI" 0xd1 and more) and 10-player loops. Was: 1.2 drops
    1.0's write 0x1015d1b0[unit id] = 1; its per-player kill table is 10 x 10 (1.2 0x1028cc2c, row 0x14, killer < 10)
    against 1.0's 8 x 8 (0x1016552c, row 0x10, killer < 8); it also counts kills of the local player's kind-3 units
    and sends a new message (1.2 0x100352b7, message 0xe: that unit's column); 1.0's kill message (Sim_100413c7,
    message 0xc) becomes 1.2's 0x10035088, sending 10 entries. Part of 1.2's 10-player protocol.
  - Target selection (round 33, ported, Sim_10016632): in a network game the list entry whose number is
    [0x1014c99c] (the local player's) is never held back.
  - Tanks as infantry (round 33, ported, the world loader Sim_100dcc12): in a network game a GPS gear whose type name
    holds "tank" (any case) is loaded as "d01infnt" / "mmb00std" / "Infantry" / "GREL", its words +8 / +0xa zeroed.

- **The DLL's C runtime is 1.0's** (round 33; Diego, 2026-11-01: "We will replace them all at the end."): 1.2's
  copies (about 90 routines, from a newer compiler) are not ported. After: every C-runtime call replaced by the
  standard library (or our own where the game's results depend on the exact digits), each replacement checked
  against 1.2's C-runtime results (1.2's game ran on 1.2's copies).

## Checker

- **The differential checker** (tools/verify) and every mapping step it needs exist only to prove the C against the
  original. After: replace it with ordinary tests.

## 1.2 ports

- **Particle mode as a switch** (round 32): 1.2's particle systems, its pool layout and its polygon changes run only
  with `W32_PARTICLES()` (1.2's game, Vulkan or 1.2's Direct3D, and 3D mode); the 1.0 effect code stays for noconv. After: 1.2's only,
  the 1.0 effect families and the size / offset macros (`W32_REC_*`; round 4 added TIME / STATE / VEL / PT / NPT for the weather
  emitters) dropped.
- **The particle draw sorts with the DLL's C library qsort** (0x100fdcc0, `W32_CALL_QSORT`). After: our own sort
  (the order of equal-depth particles must stay qsort's, or accept a different order).
- **Blast direction fix as a switch** (round 4): `W10E_FIX_DIR` (w10_effects.c) exists so the checker can build 1.2's
  fixed angle 90. After: the fix only.
- **1.2 force feedback** (controls round): 1.2's request Sim_100382fb and its table (kept at 1.0's 0x1013af00, size
  set by the hook layer) go to 1.2's FFORCE.DLL, a DirectInput I-Force library; does nothing without a
  force-feedback stick. After: drop the library and map the requests to SDL3 haptics / gamepad rumble, or drop them.
- **1.2 sound** (sound round): `SNDSDL_RELEASE_ALL` (sdl_sound.c) keeps 1.2's "Release until the count is 0" loop
  shape around a single `Audio_VoiceDestroy`, and the `SND_MUTATE12*` / `W5C_MUTATE12` switches exist for the
  checker's negative checks. After: one destroy call, the switches dropped. The music close sits in Sim_1003a052
  only under `HG_MUSIC_FILES` (the checker builds keep 1.0's CD code); after: unconditional.
- **3D sound** (sound round; Diego, 2026-11-01: "will add support for modern alternatives onces the dlls are gone"):
  the shell's 3D sound choice (Off / Headphones / Mono / Quad / Stereo / Surround = HGSND.CFG +0x40) is ignored; every
  choice plays as Off. After: give it a modern 3D path, e.g. miniaudio (one C file, mixes into SDL3) or OpenAL Soft
  (HRTF and real surround). The game logic for it is kept: 0x100397e7 / 0x10039931 fill the listener / voice blocks
  0x109e36e0 / 0x109e3720, but only run with the 3D flag (block +0x10) on, which the sound start never sets now.

## Data files

- **Patch folder path** (round B): `Sim_100ba7d5` builds "Patch\name.ext" with a backslash, as 1.2 does. After:
  build paths through the platform layer.
- **Renderer-dependent data** (round B): the Patch folder (and later the .TEX textures) is used only with the
  renderers that run 1.2's game, Vulkan and (round 33) Direct3D on 1.2's dx5draw (`g_w8yPatch`); noconv keeps 1.0's
  data. After: one data set, 1.2's.
- **1.2 as a switch** (round T): 1.2's behaviour runs with `g_w31Tex` (Vulkan, and from round 33 Direct3D on 1.2's
  dx5draw); only noconv keeps 1.0. After: one behaviour, 1.2's (Diego, round 33: the whole game is 1.2).

## Renderer

- **Specular colour** (round 32): 1.2's queued polygons carry a coloured light in the vertex's specular dword (+0x18
  low 24 bits); HGVK does not read +0x18 at all. After (lighting round): decide whether
  HGVK adds it, as D3D does with specular enabled.
- **Mode numbers:** our renderers keep 1.0's 19 render modes; 1.2's code numbers them differently (1.0's 5..18 are
  1.2's 7..20) and ported 1.2 code is mapped back. After: one numbering of our own.
- **1.2's Direct3D renderer behind a translation** (round 33, src/sim/render/w33_dx12.c): the D3D plugin's Render,
  RenderNoClip and SetState slots are wrapped (modes +2 from 5, states -1, our painter state dropped). After: gone
  with one numbering of our own, or with the D3D path itself.
- **SetState codes:** 1.2's are 1.0's minus 1 (and 1.2 has more states); ported 1.2 calls are sent in 1.0's
  numbering. After: named render states of our own.
- **Texture descriptions** keep the plugin layout {w, h, pixels, palette, key, +0x14, flag +0x18}; HGVK remembers
  alpha textures by pixel pointer. After: a texture object of the renderer's own.

## Graphics library, Vulkan only (DLL removal step 1, DECISIONS.md)

- **Callers of the converted graphics library still use SIM_FN.** The 41 routines of src/sim/gfx/w37_*.c are reached
  from the C at their original addresses (SIM_FN / W..._CALL_ macros, e.g. 0x100ecea6, 0x100ecce0, 0x100e86a1,
  0x100e91dc, 0x100e9cd1), where conv_w37.inc jumps to the C. After: call Sim_100xxxxx directly (types: the C
  takes pointers where callers pass uint32_t DLL addresses) and drop conv_w37.inc.
- **Software-renderer-only call sites** (every one under `G32(0x1017b940) == 0`): the calls of 0x100e4780,
  0x100e5477, 0x100f9fd4, 0x100e96a7, 0x100eda16, and the 8-bit branches next to the 16-bit routines (e.g. the
  0x100e86a1 / 0x100ecea6 / 0x100e91dc branches of w13_marker.c, w21_simmain.c, w9d_hud.c, w10_misc.c files that
  choose by [0x1017b940]). Kept for now. After: delete those branches ([0x1017b940] is always set with Vulkan).
- **D3D option 3 and the sim's DirectDraw driver** (0x1007f35c..0x10082790 and 0x1008667c) are not part of the final
  program. After: remove their C and hooks; the reference for the look is the untouched 1.2 game (dev.cmd option H).
- **Tables read from the DLL image by the converted routines:** the quarter-wave table inside the code at 0x100ebd27
  (0x385 dwords, w37_math.c), the bit masks 0x10154e95 and interlace steps 0x10154e9e / 0x10154ea3 (w37_gif.c), the
  remap table 0x10154eac, the colour table 0x10156dcc. After: C arrays with these contents.
- **State the converted routines keep in DLL data:** the GIF row buffer 0x10153b95 and pane [0x10154ea8], the shape
  encoder's globals 0x1015365d..0x10153691, the rotated draw's vertex table 0x10154fac and step table 0x10154ffc.
  After: locals or file statics.
- **0x100f9eb7 / Sim_10034cca:** report and VirtualProtect the DLL's self-modifying code block 0x100f4fd4..0x100f9e9a
  (and read CS with inline assembly). After: drop both.
- **Duplicates:** 0x100ed7bc = 0x100f1f86 and 0x100ed788 = 0x100f1f52 (two copies each in the DLL, two C functions).
  After: one each.
- **0x100e86a1's callback mode** (mode above 1, register arguments) draws nothing in the C (no caller uses it).
  After: drop modes 1 and above.

## C runtime replacement: maths (DECISIONS.md)

- **Errno of the original C runtime:** round 10's interpreted wrappers (src/sim/util/w10_interp_body.h, W10i_Domain)
  still store EDOM in the DLL's errno 0x1017e888 for out-of-domain arguments, as the original's routines did. Nothing
  outside the C runtime reads it. After: drop the store (OpenLibm's NaN answer stays).
- **Checker-only stand-ins** in tools/verify/difftest.py: the exact fmod answer for the original's fmod, the jump of
  the original's pow / asin / sin / atan to OpenLibm, and the errno / scratch rule for trials that ran a maths
  routine. They exist only to compare against the DLL. After: drop with the checker's DLL side.
- **Specs that stub the maths at the original's address and at hg_*** (spec_w10_100225c7, spec_w29_100232d7,
  spec_w26_10021e8c, spec_w8z): after: nothing to compare against.
