# M4: Vulkan renderer

Goal: the 3D view runs at full resolution in 32-bit colour on Vulkan, with the game logic untouched.

Detailed reverse-engineering notes (all addresses are module-relative VAs, image base 0x10000000):

- [renderer/plugin-api.md](renderer/plugin-api.md): every export of the renderer plugin, the vertex format, render modes, texture cache.
- [renderer/sim-usage.md](renderer/sim-usage.md): how HGSIM.DLL calls the plugin and where it hard-codes 640x480 and 16-bit.
- [renderer/display-ownership.md](renderer/display-ownership.md): who owns the screen and how a frame reaches the monitor.
- [renderer/hud.md](renderer/hud.md): the 2D HUD drawn at the window's full resolution.

## What we learned

1. The sim projects to screen space itself (36-byte vertices with x, y, z, 1/z). The plugin only rasterises. There is no Z-buffer, ordering is painter's.
2. There are two display contracts, chosen by the sim command line:
   - `/d3d` (DX5DRAW): the sim owns the screen (DirectDraw 640x480x16) and the plugin draws into the sim's back buffer.
   - `/3dfx` (3DFXGEAR): the plugin owns the screen. The sim draws its 2D HUD through the plugin's `LockDisplay`, `UnlockDisplay`, `RefreshDisplay`.
3. The sim's own 2D drawing is 8-bit (software) or 16-bit RGB565 (hardware mode). It cannot produce 32-bit.

## Design

A new plugin, `HGVK.DLL`, replaces DX5DRAW. It implements the `/3dfx` contract (type id 2) and exports the same functions plus the three display functions.

- Window: SDL3 wraps the game window (HWND from `FirstDevice`, offset +0x4c) and Vulkan presents to it.
- 3D: `Render` batches triangles into a Vulkan pipeline. All render modes map to a handful of pipeline states (texture/no texture, flat/gouraud, colour key, alpha blend).
- 1.2's .TEX textures (round 31): a texture offered with the description's flag +0x18 is made with alpha from its palette's top byte (alpha (15 - top 4 bits) * 17, no colour key), as 1.2's DX5DRAW makes a 16-bit ARGB texture; it is made at `PreloadTexture` and keeps its alpha when made again. It blends in the blending modes and is solid in the others. The mode numbers stay 1.0's (1.2's renderers renumbered them; see DECISIONS.md, round 31).
- 1.2's particles (round 32): their squares carry no depth (z 1, 1/z 1 from the template), which 1.2's renderer did not need. The sim calls `SetState(0x48470001, 1)` before each particle square or beam quad and `SetState(0x48470001, 0)` after it (our own state code, sent from `src/sim/fx`). While it is on, `Render` marks its draws "painter": they use a second set of pipelines without depth test or depth write, so they show over what was drawn before them in the frame and are not hidden by the ground, as in 1.2. Everything else keeps the depth test.
- 2D: `LockDisplay` returns a 640x480 RGB565 buffer, `UnlockDisplay` uploads it, `RefreshDisplay` composites and presents. The HUD drawing calls are also repeated at full resolution (see hud.md).
- Output is a 32-bit swapchain. The 3D view renders at the window resolution.

## Steps

1. Plugin skeleton that loads, logs every call and draws nothing (confirms the contract works).
2. First image: clear colour plus triangles, flat colours.
3. Textures (8-bit indexed with palette, colour key) and the 2D overlay.
4. Remove the 640x480 limit in the sim (list in `renderer/sim-usage.md`, section 5) so the 3D view can be larger than the HUD.
5. Widen the sim's 2D layer to 32-bit if wanted.

Status: research done, step 1 next.
