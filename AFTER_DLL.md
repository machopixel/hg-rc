# After HGSIM.DLL is gone

A running list of things done one way only because the converted code still runs inside the original HGSIM.DLL
(its fixed data addresses, remaining original functions, the byte-for-byte checker). Each entry: what, where, why it
exists, what to do once nothing original runs. Add to it every round.

## Memory and data

- **Data at fixed DLL addresses.** `SIM_PTR` / `G32` / `U32` / `SIM_API` (src/sim/simmem.h) reach the game's data at
  its 1.0 addresses through `g_simBase`. Exists because original code still shares that data. After: turn each table
  and global into ordinary C data with names and types, and drop `g_simBase` / `SIM_PREFERRED_BASE`.
- **Layouts kept for original code.** Records keep their 1.0 shape (field offsets, padding, the 1.0 object record
  with its +0x18 dword) because original functions read them. After: reshape freely.
- **Texture table** (1.0 at 0x10a51ac0, 512 x 0x300; 1.2 layout 512 x 0x380, frames of 0x1c). Moved out of the DLL
  into our own memory with the 1.2 layout (round T), because the luma table at 0x10ab1ae0 sits right after it. The
  pointer the sim main loop keeps in it ([0x1015dafc], w21) goes with it. After: an ordinary table of ours in the
  cleanest shape; drop the checker's 1.0 <-> 1.2 mapping step (specs w2h, w2j, w2l, w2m, w2n, w3q, w7l, w7q, w21).

- **The 14-byte texture records** (0x10ac1b00) and everything else the texture code reads stay at 1.0 addresses.

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

## Renderer and platform

- **HGVK as a plugin DLL.** Loaded through the original plugin table (0x109e3d24..0x109e3d64, names in
  docs/renderer/plugin-api.md) with its export interface. After: built into the executable, called directly.
- **Windows calls.** The platform layer (src/platform) is shaped for other systems; only Windows is filled. Windows
  calls left in original functions, the launcher, the hooks and HGVK are listed in DECISIONS.md (rounds 27-28).
  After: fill the other platforms.
- **Network code.** Parked, to be rewritten rather than converted (DECISIONS.md, network).

## Checker

- **The differential checker** (tools/verify) and every mapping step it needs exist only to prove the C against the
  original. After: replace it with ordinary tests.

## 1.2 ports

- **Particle mode as a switch** (round 32): 1.2's particle systems, its pool layout and its polygon changes run only
  with `W32_PARTICLES()` (Vulkan and 3D mode); the 1.0 effect code stays for option 3 / noconv. After: 1.2's only,
  the 1.0 effect families and the size / offset macros (`W32_REC_*`) dropped.
- **The particle draw sorts with the DLL's C library qsort** (0x100fdcc0, `W32_CALL_QSORT`). After: our own sort
  (the order of equal-depth particles must stay qsort's, or accept a different order).
- **1.2 force-feedback jolt** in the main shot (1.2 0x10019f1d calling 0x100382fb): does nothing without the
  force-feedback library. After: drop it, or wire it to a modern rumble API.

## Data files

- **Patch folder path** (round B): `Sim_100ba7d5` builds "Patch\name.ext" with a backslash, as 1.2 does. After:
  build paths through the platform layer.
- **Renderer-dependent data** (round B): the Patch folder (and later the .TEX textures) is used only with the
  Vulkan renderer so option 3 stays the 1.0 reference (`g_w8yPatch`). After: one data set for every renderer.
- **1.2 as a switch** (round T): 1.2's texture behaviour runs only with `g_w31Tex` (Vulkan), option 3 and noconv
  keep 1.0 (Diego's pick). After: one behaviour, 1.2's.

## Renderer

- **Specular colour** (round 32): 1.2's queued polygons carry a coloured light in the vertex's specular dword (+0x18
  low 24 bits); HGVK does not read +0x18 at all. After (lighting round): decide whether
  HGVK adds it, as D3D does with specular enabled.
- **Mode numbers:** our renderers keep 1.0's 19 render modes; 1.2's code numbers them differently (1.0's 5..18 are
  1.2's 7..20) and ported 1.2 code is mapped back. After: one numbering of our own.
- **SetState codes:** 1.2's are 1.0's minus 1 (and 1.2 has more states); ported 1.2 calls are sent in 1.0's
  numbering. After: named render states of our own.
- **Texture descriptions** keep the plugin layout {w, h, pixels, palette, key, +0x14, flag +0x18}; HGVK remembers
  alpha textures by pixel pointer. After: a texture object of the renderer's own.
