# Independent audit: is any of 1.2's HGSIM.DLL still missing from our C? (2026-11-01, at commit 282bd3e)

Read-only. Nothing ported. Scripts in tools/v12/audit12/ (run from the repo root with `python -I`; they write their
json next to themselves; `rea12.json` is Ghidra's function list of GameRip_12/hgsim.dll; `cside.json` was built
from the CONV tables, `// FUNCTION: HGSIM` markers, "1.2 0x..." / HGSIM12 / Sim12_ citations and the v12 specs).

## Method

1. Function inventory, both DLLs: Ghidra's list (1.0: build/ghidra/HGSIM.idx.txt, 1.2: rea / Ghidra 12.1), the
   `push ebp / mov ebp, esp` starts, call targets found in decoded code only, targets of the 1.0 jmp thunks, and
   every relocated pointer into code (callback tables). Jump tables and their byte index tables are excluded; a
   start inside another function's body is folded into it; thunks dropped. 1.0: 2683 functions, 1.2: 2873.
   Every converted 1.0 function (CONV / FUNCTION markers, 2426) is in the 1.0 inventory except two C-runtime ones.
2. Pairing: identical listings (stack slots and absolute addresses blanked), then the call graph (aligned calls in
   paired functions name each other's callees), then similarity; a repair pass re-pairs functions whose listings
   and mapped data are identical. 2640 pairs, 233 1.2-only, 43 1.0-only.
3. Comparison with addresses mapped (the round-33 survey blanked every address, so a swapped callee, global or
   import was invisible): callees must be paired with each other, data operands must agree with the data map voted
   from all pairs, imports compared by name. 2114 pairs are identical this way.
4. The rest (199 changed pairs) sorted by an order-free instruction comparison (registers, numbers blanked) into
   layout only (20), numbers only (33: record strides, debug line numbers, render-mode renumbering) and logic (146),
   each then checked against our C: a 1.2 citation inside that function, a v12 emulator spec, DECISIONS.md, or by
   reading 1.0 / 1.2 side by side. Mispairs found on the way were corrected by hand (gear sums, 0x1004ee50 = 1.2
   0x1003f7b0, 0x10011093 = 0x1000a223, 0x100500ab = 0x10040ac4, 0x1006da46 = 0x1005cf76, 0x100d63fa = 0x100bc428).
5. Data: every directly read constant of the paired code (1913) compared byte for byte; indexed tables (168)
   compared over their extent; every string the paired code pushes compared.

## Missing (real 1.2 changes not in our C) - both ported 2026-11-01 (DECISIONS.md "1.2 audit")

- **Force feedback when firing**: Sim_100d4f71 (src/sim/unsorted_100d/w10_misc.c; 1.2 0x100baf31), the fire
  handler the weapon tables point at (1.2 slots 0x1011ae8a + n * 0xd6). 1.2 adds, for the own unit's shot and with
  the force-feedback library on ([0x10131718]), `Sim_100382fb(weapon +0xd2, 0, 0, 0)` right after the shot counter
  [0x101653f3] += 1. Nothing else differs (record shift only). The round-33 survey called 0x100baf31 dead: it has
  no call, only table pointers. The other 16 call sites of 1.2's 0x100382fb are all in our C.
- **Current-objective line wrap**: Sim_100cd70f (w9g_text.c; 1.2 0x100b1c8f). 1.2 copies the objective into its own
  terminated buffer, wraps only when it is longer than the width (1.0 also when equal: `jge` -> `jg`), terminates
  the first line before the space search and cuts at the objective's last space (strrchr on the whole text). Our C
  keeps 1.0's logic with Diego's buffer-zeroing deviation (DECISIONS.md, round 21). The F12 list (0x100ccc60 = 1.2
  0x100b15fb) kept 1.0's wrap; it only draws through 1.2's helper 0x100b1340 (settled in DECISIONS "HUD").

## Not ported, no effect in our build

- SimMain (1.2 0x1006fae1, after InitExternalCtrl): renderer SetState(7, 0) = 3dfxgear's mipmap mode off
  (grTexMipMapMode). Our renderers have no mipmaps.
- SimMain's exception handling: 1.2 uses __except (filter 0x1006facb returns 1; handler 0x10071235 shows the same
  "unknown fatal error" box, polls the network, frees the heaps and returns); ours keeps 1.0's __finally (the
  exception goes on). Only on a crash.

## Deliberate omissions (DECISIONS.md)

3D sound backend; the CD search 0x10087460 (and the "insert CD" text); the 16bit / 8bit switches (0x100201d0);
the WAV player and wavmap.ini (0x10015a38 loader, 0x10016130.., 0x1002e690.., 0x100b98c0.., the music-table slot
entries 0x1001569c / 0x1001582f / 0x10015a2e / 0x1008bd6c); the C runtime (about 20 changed routines, the new ones
from 0x100db900); the graphics library / DirectDraw driver (0x1006b560); ANET2 (1.2's name loader, its extra
loader 0x1002fd99 / 0x1002fdf4).

## Dead in 1.2 (no call and no pointer anywhere)

zlib (0x10026f40.., 0x10042e90..0x10046d10, 0x100b4450..0x100b7c40, 0x100c7050..0x100c70d8, 0x10004a70); the
particle families 0x10080d38 / 0x10080f53 / 0x1008139b / 0x10082cc8 / 0x10082da9 / 0x10083679; the Patch loader
0x100a3a92; 0x1004ccea, 0x10010c32, 0x100cb89a.

## Accepted from earlier work, not re-proved here

About 100 ported functions with a 1.2 citation: 73 have a v12 emulator spec; the others were read for their
residual differences (render 3DFX / .TEX branches, shot dust, network handlers, SimMain's CD / 3DFX / texture
bits), all present in the C. The gear-sum tables (0x1004c0xx..0x1004d3xx, 0x1005a1db): 1.2 re-packed them; the
row and column values used are equal (geartab.py; plus the emulator runs in remaining.md section F).
