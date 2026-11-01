# What is left of 1.2 in HGSIM (full pass, 2026-11-01)

Read-only. Every function of 1.2's hgsim.dll (2997 starts found by tools/v12/survey.py: prologues and every call
target) against its 1.0 twin and our C, after the controls, graphics/sound sweep and the weapon fix of 2026-11-01.
Method: the survey's pairing (survey.json), a fresh diff of every non-identical pair with a wider layout filter
(record displacements and immediates 4 apart, frame size), the hook / conv tables and `FUNCTION: HGSIM` markers for
"converted", the v12 and other checker specs for "checked against 1.2", and, for the 1.2-only functions, their
callers, strings and imports. Scripts: the scratchpad's fullpass.py / repair.py / newinfo.py (not committed).
Caveat (tools/v12/survey caveats): the start finder also takes mid-function call targets; those rows are folded into
their host function below.

## Counts

| class | what | count |
|---|---|---|
| A | ported to 1.2's form and checked against 1.2 (v12 or round 31/32/34 specs) | 58 changed pairs (+ 22 layout pairs re-checked) |
| B | identical apart from addresses | 2092 |
| B | layout only (the 4-byte record shift, table strides our C already handles: 0x380 texture groups, 0x406 pool records) | 351 + 32 + 14 checked by reading |
| C | differs from 1.0 and our C does not have 1.2's change yet | 25 (21 network, 4 sound; the command line settled, not ported); window focus and cache file ported 2026-11-01 |
| D | new in 1.2, no 1.0 twin, not in our C | about 75 live functions in 6 groups, + 6 dead zlib helpers (list below) |
| E | 1.2 change inside code that still runs as the DLL's original code (C runtime, graphics library) | 19 + 64 new C-runtime |
| F | not classified with certainty | 0 - all 30 settled on 2026-11-01 (section F below) |

Converted 1.0 functions with no 1.2 twin at all: 62 - the old effect families (replaced by 1.2's particle systems in
particle mode, round 32), ported ones whose 1.2 version moved (the mount, the respawn, the HUD panels 15 / 16,
the side panel helpers, the sky reader), dead code and C runtime. None needs more work for 1.2 except as listed.

## C - 1.2 changes not yet in our C

### Network - done (network round, 2026-11-01; DECISIONS.md "Network round")

All of 1.2's network module is ported and checked against 1.2 (spec_net_*_v12.py). 0x1002fd99 / 0x1002fdf4 are
points inside 1.2's ANET2 name loader, whose changes touch only names the sim never calls: nothing to port. The
transport stays the original ANET2.DLL (Diego, 2026-11-01).

### Network (for the network round) - 21 (the original list, all done)
- 0x1003e291 (1.2 0x10031a86) the network message loop: 10 players, new messages ("DI" and others).
- 0x1003dc19 (0x10031389) network game start: 10 players, two more 0x100-byte per-player buffers.
- 0x1003db56 (0x100312c6) player-id lookup: 10 ids instead of 8.
- 0x10041e61 (0x10035c65) network player reset: only three debug prints ("InitExternalCtrl ..."). Corrected
  2026-11-01: the stores are the same with the record shift (both clear +0x160 / +0x164); the +0x18 clear of kind-3
  units is gone because 1.2's record has no +0x18 dword. No behaviour change.
- 0x1005b98c (0x1004e4cc) armour scaling: in a network game every gear x2 (1.0: x4 for the own and kind-1 gears).
- The "game is ending" flag (1.2 [0x101314c0]) checked first by 13 message handlers: 0x1003ff72, 0x1004044c,
  0x100405e6, 0x1004084a, 0x1004094e, 0x10040a37, 0x10040f2d, 0x100410ff, 0x10041247, 0x1004131b, 0x1004170e,
  0x10041a16, 0x10041b2d; with 0x10040191 (0x10033a8a: also the kill table, ending check), 0x100408bd (also sends the new "DI"
  message 0x10033c90: the 17 part records' first three dwords of the handed-over gear; "TC" receive 0x1004084a
  unpacks them, 0x10034342), 0x100421b0 (0x10035fe7: keeps the previous host id; its callback 0x10042125 = 0x10035f3e clears the pending
  request flag 1.0 0x1015f200 when the host changes).

### Command line - 1 (settled 2026-11-01, not ported)
- 0x1002ad50 (1.2 0x100201d0) the sim's command-line reader: settled 2026-11-01, not ported (Diego: "We want only
  16 bit, no need for switch"; DECISIONS.md "Command line"). 1.2's two new words "16bit" / "8bit" set the .TEX flag
  0x10105048 (1 / 0) and 0x1014a5e0; 1.2's sim main sets 0x10105048 again from the 3D mode before anything reads
  it, and nothing reads 0x1014a5e0, so they change nothing.

### Sound (sdl_sound.c replaces the DirectSound code; see tools/v12/sound.md) - 4, ported 2026-11-01 (DECISIONS.md "Sound round")
- 0x1003a0e4 (0x1002e159) sound start: 1.2 sets a high-quality primary format and creates a DirectSound 3D listener
  (new 0x1002e2b1).
- 0x10039c00 (0x1002dc30) and 0x1003a052 (0x1002e0a6): sound buffer / WAV-music handling.
- 0x1000e0d9 (0x10007bf9) sound load: one added null check.

### Other - 2 (found when settling F, details there) - both ported 2026-11-01 with fixes (DECISIONS.md, "Focus pause and cache size 0")
- 0x100867a2 (0x10071397) the window procedure: pause the clock and sound on losing focus, resume on getting it back.
- 0x100ba304 (0x100a26e4) the cache file reader: no slot table for a size-0 line.

## D - new in 1.2, not in our C

- zlib inflate, 27 functions (1.2 0x10042e90..0x10046d10: "invalid block type", "incorrect header check"...).
  Settled 2026-11-01 (tiny-sweep end): dead code in 1.2. Its only entries are zlib's uncompress 0x100b7c40 and the
  "ZLIB" block reader 0x10026f40, and neither has a caller (no call, jump or pointer to them anywhere in the DLL;
  the few other hits into the range are bytes inside other instructions). No data file can reach it: nothing to port.
- WAV music player and wavmap.ini, about 34 (0x10015a38.., 0x10016130.., 0x1002e690.., 0x100b9bf8..): not ported,
  Diego's choice 1a (the MUSIC folder's Ogg player does its jobs).
- DirectPlay, about 8: done in the network round (2026-11-01). 0x100360c4 / 0x100362d2 / 0x10036590 are ported
  (the player update, "'%s' resurrected."). 0x1002fd99 / 0x1002fdf4 are points inside 1.2's ANET2 name loader:
  nothing to port.
- Full-screen toggle: corrected 2026-11-01, not new. 0x1002c5bf is inside 1.2 0x1002c5a1, which is identical to
  1.0 0x10037f71 (fdiff: 0 differences; same "ToggleFullScreen" texts, already converted in w4u_refresh.c), called
  by the window procedure as in 1.0. The "call from 0x1002bcb0" was bytes inside a `shr` instruction.
- Already ported or checked equal under other addresses: the HUD panel draw 0x100b2a4a (= 1.0 0x100ce983), the
  particle families of round 32, the footstep dust, the weapon mount.
- Dead in 1.2 (no caller): 0x100a3a92 (a Patch-folder loader variant), the missile 0x100baf31 / 0x100baf96 /
  0x100bb171, the particle families 0x10080f53 / 0x1008139b / 0x10082cc8 / 0x10082da9 / 0x10083679.

## E - original code (not ported; replaced when the DLL goes)

C runtime: 1.2 changed 19 of its routines (formatting, maths, unwinding) and has 64 new ones - Diego's choice 2a.
Graphics library and DirectDraw driver: no 1.2 change found that our renderers use.

## F - settled (2026-11-01, read-only)

Each item was checked in the code (rea / capstone, scratchpad scripts callalign.py, twin.py, stores.py, gtab.py,
gemu.py, gmatch.py, gbig.py). No earlier verdict was wrong. Three survey pairings were wrong (see the gear tables and
0x100a773b); the real twins are equal.

### Already ported or equal (A / B)
- 0x1001a0e2 (the 934-instruction helper): not a function. It is a point in the middle of 1.2's main shot 0x10019f1d
  (= 1.0 0x100232d7, ported and checked against 1.2: spec_w29_100232d7_v12). The order target point 0x1000f0f4
  (1.0 0x10015f5b) does not call it, so its layout-only verdict stands, and Sim_10015018 (order targeting, 4000 / 0
  against 1.2) is not affected.
- Start-finder artifacts inside functions that are equal or already ported: 0x10011a34 (host 0x100118fe = 1.0
  0x1001a23e), 0x10012bb2 (0x10012a2c = 0x1001b36c), 0x10051966 (0x10051940 = 0x1005fbb0), 0x100b816d (0x100b8120 =
  0x100d1f90), 0x100bfbad (0x100bfaf9 = 0x100dacf9), 0x100d0354 (0x100d029b = 0x100ef727), 0x100c6005 (inside the
  WTB reader), 0x100b3058 (inside HUD panel 16), 0x10078d03 (inside the particle draw 0x10078c29), 0x10058aab (inside
  0x100589ad = 1.0 0x100680dd). 0x10063952 and 0x1005a21a are not call targets at all: the "call" byte is inside
  another instruction (0x100557cc, 0x1005a20f).
- 0x1001fa46 = 1.0 0x1002a2f6 (Sim_1002a2f6, a record reset): every store compared, the same 62 stores with the
  4-byte record shift; layout only.
- 0x10059e9d = 1.0 0x1006a12d (Score_RecordKill): 1.2's kill tally, already ported in round 33 (spec_w34_rating).
- 0x10036cc0 (1.2 0x1002b4e0) and 0x100a89f0 (0x10094150): layout only. The 0.3 constant and the call order are the
  same in 1.0; only the record shift differs.
- The gear part tables (0x1004c221, 0x1004c297, 0x1004c4a1, 0x1004c4d1, 0x1004ca99, 0x1004cb29, 0x1004cbb9,
  0x1004cc49, 0x1004d106, 0x1005a1db): all ten give the same results as their 1.2 twins on the real tables (run
  side by side in an emulator, 2000-3000 cases each, return value and the gear / part records compared), and the
  table values are the same row by row. The survey had paired three of them wrongly: 1.0 0x1004ca99 = 1.2
  0x1003de76, 0x1004cb29 = 0x1003df06, 0x1004cbb9 = 0x1003df96.
- 0x100a773b: wrong pairing. Its real 1.2 twin 0x1009326b is identical (and has no caller in either version).

### Not ported, a real 1.2 change (C)
- Window focus (0x100867a2 = 1.2 0x10071397, our src/sim/platform/w28_window.c): when the game window loses focus
  1.2 pauses the game clock and the sound (the pause pair 1.0 already has, 0x1008b8de / 0x1008b900, used by the HUD
  menu); when it gets focus back it resumes them. Not in a network game. 1.0 already skips frames while the window
  is inactive (SimMain's loop, [0x1017b928] == 0), but its clock keeps running and its sounds keep playing.
- Cache file reader (0x100ba304 = 1.2 0x100a26e4): a line with size 0 gets no slot table (+0x10 = 0) instead of a
  zero-size block; plus the assert line numbers. Only matters for a cache file line with 0.

### The survey's mission / platform rows (tiny-sweep end, 2026-11-01)
- 0x100867a2 window procedure: ported (focus pause, above).
- 0x1009e1ef Cd_Open (1.2 0x1008b36f): its volume call goes through 1.2's music table slot [0x10108ca0] (the WAV
  volume 0x1001603f with wavmap.ini, else the CD's aux volume). Ours already calls Win_AuxSetVolume, which with
  HG_MUSIC_FILES is that WAV volume (Music_SetVolume). Nothing to port; and the CD audio start (0x1009dfa0) is
  hooked to "no CD" (hooks_sim.c), so this code does not run.
- 0x10099510 CD drive search (1.2 0x10087460): our GameRip 1.0 HGSIM is a patched copy (two jumps NOPed, FindClose
  in place of FindFirstFileA), so it always answers "found". 1.2 has the real search: the first CD-ROM drive (type 5)
  holding \video\B51MOVIE.AVI gives 1, else 0; the drive count [0x1017cc3c] counts the CD drives seen. In our build
  its callers are SimMain (skipped: the launcher passes 1.2's own "no CD" flag), the movie folder search 0x1007e66c
  (same result either way: 1.2's answer 1 is not a drive letter, so its CD path never finds anything) and the CD
  audio start (hooked off). Not ported: Diego's standing rule "NO CD EVER" (DECISIONS.md, tiny-sweep end).
- Also re-read: 0x10024548 missile release (1.2 0x1001b224) only adds a dead store (the player's unit into a local
  never read); 0x100c7120 target readout (1.2 0x100acc00) is layout only read side by side. Nothing to port.

### 1.2's own code, not ported (D / E)
- 0x1006b560 = 1.0 0x1007f950 (DirectDraw shutdown): 1.2 no longer sets the normal cooperative level before
  restoring the display mode. DirectDraw reference driver only (option 3); goes with the DLL.
- 0x10026f40, 0x100281c0, 0x10028531, 0x10028b3b, 0x100294f4, 0x100c70ed: a second zlib entry and its helpers
  (0x100c70ed is zlib's free, also used by the zlib group). 0x10026f40 has no caller in 1.2: dead code.
