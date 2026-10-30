# HGSIM.DLL: detail options, LOD and draw distance

Source: `sim.asm` (objdump of HGSIM.DLL, md5 9980c2a5ae57094151d31c19564fd8f5, image base 0x10000000) plus capstone/pefile reads of the image.
All addresses are absolute VAs at the preferred base; RVA = VA - 0x10000000.
**VERIFIED** = read directly in the disassembly or data. **INFERRED** = interpretation of verified code, or not checked at runtime.
Names in `CamelCase` are mine, not from the binary.

Goal: always render at maximum detail with a much larger draw distance, game logic unchanged.

## 0. Summary

| What | Where | Default | What limits it now |
|---|---|---|---|
| LOD quality ("Display detail", key `TOGGLE_LOD_QUALITY`) | `0x10152418` int, 1 = High, 2 = Low | 1 | Scales the LOD switch distance by 1/1 or 1/2 |
| LOD switch scale | camera `+0xb8` = halfwidth * zoom / (LODquality * 160.0) | 2.0 at 640 wide | Per-model thresholds |
| Far plane | main camera `0x1014ca00 + 0x40` = `0x1014ca40` (float); per-frame copy `0x10185bf8` | 150000.0, or the mission's BWD `VIEW` chunk | Object cull, polygon clip, fog end |
| Near plane | camera `+0x3c`, recomputed as halfwidth*0.5+1.0 | 161 at 640 wide | (not a distance limit) |
| Object/terrain grid | 9x9 cells of 55562 units, cell list built by `VisibleCells` 0x10087d83 | camera cell + 5 or 8 neighbours | **This is the real terrain/object range now (about 55k to 111k ahead)** |
| Sub-object (child) range | `0x10185b60` = (far-near)/2, plus projected radius > 10 px | 74968 | Children of hierarchical objects |
| Fog | linear from SKYGND.PAR `density` Z (`0x10143fc8`) to the far plane | per mission | Fully fogged at the far plane |
| Polygon sort list | 3000 entries (`0x1017d17c`), 1 MiB arena, soft limit 2900 (`0x1017d140`) | | Drops further objects when full |

The frame dump fits this (VERIFIED in `hgvk.log`): terrain polygons (Render op 1) come in rows of exactly 7937 units in z (1642, 9579, ... 49264, 57201), and 7 x 7937 = 55562 = one grid cell. The farthest row (57201) is the far edge of the neighbour cell in front of the camera, well inside the 150000 far plane. So the grid cell list, not the far plane, is what cuts the terrain off today (INFERRED, but consistent with the cell code in section 3.3).

## 1. Detail option variables

All options are fields of the 108-byte **HGSND.CFG** record (not hgprm.cfg). SimMain reads it at 0x10085222 (`push 0x1015d1b8; push "hgsnd.cfg"; call 0x10001afa`); the pointer to it is `[0x1015d1b8]`; if the file is missing the defaults at 0x101321b0 (27 dwords) are copied (0x10085253..0x10085277). VERIFIED.
The in-game "COMBAT VARIABLES" menu table is at 0x1014bb90 (title 0x101c47b0, 5 items at stride 0x14: Particles, Shadows, Multiple Lights, Joystick Auto Center, exit). Its first item has an on-enter function `ApplySimOptions` 0x10098721 (thunk 0x100013a7, also listed in a pointer table at 0x10149aa8) that pushes the cfg values into the game (VERIFIED code; when the menu engine calls it is INFERRED: on opening that menu). The other option descriptors (Textures 0x1014ba28, Terrain textmaps 0x1014ba50, Display detail 0x1014ba78, Object density 0x1014baa0, Explosion chunks 0x1014bac8) are not referenced by any menu (VERIFIED: no pointer to them in the image). Descriptor layout: `{2, 0, choices*, 0, 0, getter, 0, setter, 0, 0}`; choice lists 0x10144358 = {Off, On}, 0x101443e8 = {Low, High}.

String globals (from the loader at 0x100aef47..): Textures 0x101c4840, Terrain textmaps 0x101c45ec, Display detail 0x101c4580, Object density 0x101c484c, Explosion chunks 0x101c4640, Particles 0x101c4750, Multiple Lights 0x101c4db8, Shadows 0x101c4f64, Joystick 0x101c510c, Affine 0x101c5124, Perspective 0x101c4698. "Affine/Perspective" is copied into a choice list at 0x1014b9e8 that nothing references (dead, VERIFIED).

| cfg off | Option | Values | Setter (thunk) -> what it writes | Variable(s) and default | Readers | Notes |
|---|---|---|---|---|---|---|
| +0x14 | Textures | 0 Off, 1 On | 0x100988fc (0x10002892): `SetRenderFlag(0x100,v)`, `(0x200,v)`, `(0x800,v)`, 0x100a96d8(v), `[0x1014c9fc]=v`, cfg+0x14/+0x18 = v | `RenderFlags` 0x1014cb38, bit set = feature **off**; default 0 | 0x10006bd2, 0x1002bb74, 0x10032f5a/0x10032fa2 (texture choice), 0x1005911c, 0x10074d54, 0x10076fd9, 0x100a0282 (fog colours), 0x100a1160, 0x100a25f9, 0x100a2770 (sky), 0x100dcfd7 | Applied at load by SimMain 0x10085282..0x100852b1 and by ApplySimOptions. `SetRenderFlag` 0x100332a3: v!=0 clears bit, v==0 sets bit. 0x100787f4 and 0x10078857 set it to 0xb00 temporarily for a special pass (they save/restore the 0x68-byte render state at 0x1014cae8). VERIFIED |
| +0x18 | Terrain textmaps | 0/1 | 0x100989a1 (0x10001cf3): `SetRenderFlag(0x800,v)`, 0x100a96d8(v), `[0x1014c9fc]=v`, cfg+0x18 | bit 0x800 of 0x1014cb38 | as above | Also key action `TOGGLE_TERRAIN_TEXT_MAPS`. VERIFIED |
| +0x1c | Display detail | 0 Low, 1 High | 0x10098a18 (0x10001497): `SetLodQuality` 0x100d2c86 (v ? 1 : 2) and `SetLowDetailFlag` 0x100332e9 (`[0x1014cb34] = (v==0)`) | `LodQuality` 0x10152418 int, default **1** (High); `LowDetailFlag` 0x1014cb34 default 0 | 0x10152418: 0x100d24fd..0x100d2522 (LOD scale, section 2), 0x10024c94 (effect throttle, below), getter 0x100d2c6f. 0x1014cb34: **no reader** except save/restore at 0x1009fa5e/0x100a0d0f | Only ApplySimOptions applies cfg+0x1c; SimMain does not. The key toggle at 0x1008bfc7..0x1008c00e flips 0x10152418 between 1 and 2 directly ("LOD Quality HIGH" 0x1014b980 / string 0x101c4838). VERIFIED |
| +0x20 | Object density | 0 Low, 1 High | 0x10098a5d (0x1000223e): `SetObjectDensity` 0x1008a94d(v) | none (state is per object: flag 0x1000 in word +0) | n/a | v=0 hides every object of class 0xc0 (`(word[obj+2] & 0xf0) == 0xc0`) by unlinking it from its grid cell (0x1008a40a, 0x1008a37f); v=1 re-inserts them (0x1008a46e, 0x1008a220). The only call is from that setter, which only the unused descriptor 0x1014baa0 and the table at 0x10149a48 point to, and ApplySimOptions skips +0x20. So in normal play all class-0xc0 objects stay in (VERIFIED code, "never called" INFERRED). Hiding them also removes them from the grid that gameplay queries use (0x1002db2d), so this option is not purely visual |
| +0x24 | Explosion chunks | 0/1 | 0x100b5bcb (0x10001546): `[0x1015084c] = 1` (ignores v), cfg+0x24 = v | 0x1015084c, default 1 | getter only (0x100b5bbb) | Dead option: always 1. VERIFIED |
| +0x28 | Particles | 0/1 | 0x10098826 (0x10002c3e): `Particles` 0x10090d15(v) | 0x1014b9b8, default 1 | 0x100905b0 (spawn gate) | v=0 also kills live particles (pool 0x10217c20, stride 0x57e). VERIFIED; visual-only INFERRED |
| +0x2c | Multiple Lights | 0/1 | 0x1009885e (0x100038af): 0x100585c4(v) | 0x10143fd8, default 1 | 0x10058582, 0x10058f20, 0x100593a6 (vertex lighting) | `0x100585c4(-1)` is the getter. VERIFIED |
| +0x30 | Shadows | 0/1 | 0x10098896 (0x10002379): 0x100ac603(v) | 0x1014d1d8, default 1 | 0x100ad9c1 (shadow pass 0x100ad1f3, Render op 0xf) | VERIFIED |
| +0x64 | Joystick auto center | | 0x100987fa | cfg+0x64 | 0x100553e2 | not graphics |

Installed HGSND.CFG (VERIFIED by reading the file): +0x14=1, +0x18=1, **+0x1c=0 (Low)**, **+0x20=0 (Low)**, +0x24=0, +0x28=1, +0x2c=1, +0x30=1. So with this file the LOD quality drops to Low as soon as ApplySimOptions runs.

Other users of `LodQuality` (VERIFIED, 0x10024b88..0x10024ca6): when an effect is spawned into the 256-slot effect table 0x109a5a40 (stride 0x2c), the code counts active effects of the same type within 500.0 units (0x10111248); with LodQuality==1 up to 2 such neighbours are allowed, with 2 the new effect is dropped at the first one. If those effects can deal damage, LodQuality would touch gameplay (INFERRED risk). Leaving LodQuality at its default 1 keeps the original behaviour.

## 2. LOD selection

### 2.1 LOD scale (VERIFIED, `CameraViewport` 0x100d2306, thunk 0x10003170)
- halfwidth `hw = max(1, (x1 - x0 + 1) >> 1)` from camera rect +0x2c/+0x30, stored +0x84; halfheight likewise +0x88.
- zoom = camera +0x18 clamped to [0.5, 16.0] (0x1012bcbc, 0x1012bcb8); +0xb0 = zoom*f(+0x44)*hw/hh.
- `+0x94 = hw*zoom` (focal scale in pixels), `+0x98 = hw*zoom*aspect`.
- **near plane** `+0x3c = hw*0.5 + 1.0` (0x100d24d0..0x100d24e2; 0x1012bcbc = 0.5, 0x1012bcac = 1.0). At 640 wide hw=320, near = 161, which matches the nearest terrain z in the dump.
- 0x100d24fd: if LodQuality <= 0 it is forced to 1.
- **`+0xb8 = hw*zoom / (LodQuality * 160.0)`** (0x100d2510..0x100d2522; 160.0 at 0x1012bcc0, only use of that constant). At 640 wide and zoom 1: 2.0 (High), 1.0 (Low).
- This function runs when `[0x1013a014]` is set (0x10034f0e) and from 0x100352fb, 0x10061a1a/0x10061b26, 0x1008fc89/0x1008fda4 (map view), 0x100ab9c8, 0x100d25d5. Nothing else writes camera +0xb8 (the other `+0xb8` stores at 0x100d87fb/0x100d8b10 are a different struct).

### 2.2 Choosing the LOD (VERIFIED, `RenderObject(obj, depth)` 0x100a4510, thunk 0x100017ee)
```
node = obj->lodList (+0x2c)            // linked list, node+0 = float threshold, node+0xc = next
if (node->thr == 0.0) chosen = node    // single-LOD model (0x1012ba68 = 0.0)
else for (prev = node; node; prev = node, node = node->next)
        if (cam->b8 * node->thr > depth) { chosen = node; break; }
     if (!chosen) chosen = prev        // past the last threshold: last (coarsest) node
obj->+0x30 = chosen; then each polygon of chosen goes to DrawModelPolygon 0x100a65e7 (call at 0x100a46c5)
```
- `depth` is the view-space z of the object centre, written to `0x101c7aa0` by the cull callback (0x100d2912 / 0x100d2b03) and passed by all five callers (0x100a48b3, 0x100a49a4, 0x100a4b21, 0x100a4bde, 0x100a5302).
- So a model's LOD k is used while `depth < b8 * thr_k`; the thresholds are per model data (from the PRJ/model files), not code constants. Nodes are ordered finest first (INFERRED: a larger b8, i.e. High quality, keeps earlier nodes to larger distances).
- Effective switch distance = `thr * hw * zoom / (160 * LodQuality)`: it grows with the 3D view width, so a wider sim viewport alone raises detail (INFERRED from the formula).

### 2.4 Where the game marks the finest LOD (VERIFIED from code and model files)

Static models (names `o...`, `t...`): the loader 0x100b48de / 0x100db78b reads an SDF with up to 10 WTB names at `+0x0c + 32*i` and one switch distance per name at `+0x14c + 4*i`, passes it through 0x1017e858 to ReadWTB 0x100e2aa3, and AddLodNode 0x10008cbc inserts nodes into `obj+0x2c` sorted by ascending threshold. Example SDFs: `o_1artya/o_2artya/o_3artya` with 10 / 25000 / 40000. So node 0 is the `_1` file, the finest, and a threshold of 0 marks a single-LOD model.

Gears: each GDF part (0x438 bytes) has six 32-byte names at `+0x7c`, e.g. `ga11trso ga12trso ga13trso ga14trso`. The gear builder 0x1005ada5 skips slot 0 when the name starts with `g` and makes levels 0..2 from `ga12`, `ga13`, `ga14` (`ga11` and `ga12` have the same geometry, 44 vertices and 25 faces for the Cheetah torso). Each level is its own single-node object in the gear model table 0x109b9e20 (stride 0x7e, `+0x50 + 4*level`), loaded on demand by 0x1003aecb. So gears do not use RenderObject's thresholds; `UpdateGearLods` 0x1003b534 picks the level every frame:

- player gear in cockpit view, or with flag 2 in `+0x1c`: 0; other gears with flag 2: 1;
- otherwise by distance with `T = (int)(b8 * {1500, 13000, 21200, 30000})`: 0 only for the single nearest gear within T0, 1 for the next two nearest (and up to six within T1), then 2 within T2, else 3.

Setting b8 to 1e20 makes those `_ftol` conversions overflow to INT_MIN, so every gear dropped to level 3; that was the "enemy gears look wrong" result. The hooks now set b8 = 1e20 (static models) and replace UpdateGearLods to give every gear level 0, keeping its flag-2 rule.

### 2.3 Forcing the finest LOD
Either of:
- **Hook `CameraViewport` 0x100d2306** (RVA 0xd2306, prologue `55 8B EC 83 EC 48`, 6 bytes, no relative branches) with a trampoline: call the original, then `*(float*)(cam + 0xb8) = 1.0e20f`. Every multi-LOD model then picks node 0 at any distance. LodQuality stays as it is, so the effect throttle is unchanged. Risk: more polygons per far object (see section 4).
- Or a 2-byte code patch at **0x100a4591**: `75 08` (jne 0x100a459b) -> `90 90`: the first node is always taken. Needs a verified byte-patch facility in the hook layer (it only does function-entry jumps today).
- Or write the .rdata float at 0x1012bcc0 (160.0, `00 00 20 43`) to e.g. 1.0e-6f before the first viewport computation (needs VirtualProtect).

Locking LodQuality itself to 1 (optional, keeps the effect throttle at its High setting): hook `SetLodQuality` 0x100d2c86 (RVA 0xd2c86, `55 8B EC 8B 45 08`) to always store 1, and re-write `[0x10152418] = 1` each frame because the key toggle at 0x1008bfd0/0x1008c007 writes it directly.

### 2.4 Sub-objects (VERIFIED, `DrawObjectTree` 0x100a4933 / 0x100a4845)
For objects with a child list (`obj+0x28`, children via `+0x4`/`+0x8`), the parent is drawn, then the children are drawn only if
`z < [0x10185b60]` (= (far - near) / 2, set in SetCurrentCamera 0x100d2111..0x100d2123) **and** projected radius `cam+0x94 * obj+0x88 / z > 10.0` (0x1012ba6c, its only use) (0x100a4b46..0x100a4b64).
To always draw children: set `[0x10185b60] = FLT_MAX` after each SetCurrentCamera (see 3.6), and optionally patch `0x100a4b64: 75 3B -> 90 90` (drop the 10 px test). Children are extra detail on top of the parent (INFERRED), so this only costs polygons.

## 3. Draw distance

### 3.1 Cameras (VERIFIED)
- The main camera is a static struct at **0x1014ca00**; the current camera pointer `[0x1014cae0]` is initialised to 0x1014ca00 (data) and set by `SetCurrentCamera` 0x100d1f90 (thunk 0x10002a04) every frame (0x10034fdd; also 0x10035323 in hardware mode).
- Static defaults: +0x18 zoom 1.0, +0x1c 1000.0, +0x20 10000.0, +0x24 -1000.0, rect +0x2c..+0x38 = (0, 319, 0, 199), **+0x3c near 64.0, +0x40 far 150000.0** (0x1014ca3c = `42800000`, 0x1014ca40 = `48127c00`).
- BWD chunk handlers (chunk ids at 0x101349f0.. and 0x10134ac8..): **`VIEW`** (id at 0x10134ad4) at 0x100dce59: `near = (float)int[chunk+8]` -> 0x1014ca3c, **`far = (float)int[chunk+0xc]` -> 0x1014ca40**. `VWST` (0x10134a0c) at 0x100dd481 sets position +0x0..+0x8 and angles/zoom +0xc..+0x18 (int * 1/65536). `VWSP` (0x10134a08) fills up to 9 viewport rects at 0x10185404 (stride 0x14). `TERR` (0x101349fc) is skipped. The mission files were not available, so the per-mission far value is UNKNOWN.
- Near is overwritten anyway by CameraViewport (2.1).
- `SetCurrentCamera` copies to globals (0x100d1f90..0x100d2164): near `+0x3c -> 0x10185c50`, **far `+0x40 -> 0x10185bf8`**, `(far-near)/2 -> 0x10185b60`, rect -> 0x10185bd0/0x10185bec/0x10185bf0/0x10185be0, centre 0x10185bd4/0x10185bf4, matrix rows 0x10185c20..0x10185c40, position 0x10185c44..0x10185c4c.
- Temporary changes, all restored by the caller: `SetCameraFar` 0x100d2545 (sky-object pass 0x10035178..0x100351c1 uses far = FLT_MAX `0x7f7fc99e`), `SetCameraNear` 0x100d252c (cockpit pass 0x10035210..0x10035258 uses near = 8.0), the hardware horizon clip 0x100a0512 swaps in near/far = (0x10185c50, 9.0e7 `0x4caba950`, pushed at 0x100a0d25), the map view 0x1008fb20 saves the camera (0x1036f0e0) and sets far from its argument (0x1008fc44).

### 3.2 Where the far plane is used (VERIFIED)
- Object sphere cull `CullObject` 0x100d280d (= callback `[0x1014cb40]`, installed at 0x10034d61): view depth `z` -> 0x101c7aa0; `z + r < near` -> 4; **`z - r > far` (0x10185bf8) -> 5**; side planes -> 6/7; 0 = visible. 0x100d2a7b is the same without the near radius test (used for the sky list).
- Polygon clipping against near/far: 0x100a5d38 / 0x100a6022 (0x100a611c, 0x100a61e2, 0x100a6288) and per-vertex outcodes in DrawModelPolygon 0x100a685b (near) / 0x100a6876 (far).
- Fog (3.5) and 0x1001c8b9, 0x1008fe92 (other tests against far).
- The D3D plugin parameter block passes a z range of 1.0 and 500000.0 at +0x18/+0x1c (sim-usage.md 2.2). A far plane above 500000 may saturate a plugin that scales depth by that value (INFERRED; check HGVK's own depth mapping).

### 3.3 Object/terrain grid: the real range limit (VERIFIED)
- World grid: 9 x 9 cells, cell size **55562.0** (0x1012b8e0), origin offsets x + 254008.0 (0x1014b8e0), z + 246044.5 (0x1014b8e4). Cell `[row][col]` is a 0x2c-byte pseudo-object at `0x1017ba28 + row*0x18c + col*0x2c`; `+0x24` (0x1017ba4c) points to itself, `+0x8` = first object (0 = empty), `+0x28` (0x1017ba50) = bounding volume used for the cell cull. Built by 0x100883a2 (0x100886d8..0x10088755 also set `[0x1017c8cc]=0x1017b988` (no-grid list), `[0x1017c8c8]=0x1017b9b0` (objects outside the grid), `[0x1017c8d4]=0x1017ba00`). Objects are filed into cells by 0x1008a46e (same cell formula).
- `[0x1017c8c4]` = grid on (SimMain sets 1 at 0x1008503d; command-line `/grid` `/nogrid` 0x1002ae11..0x1002ae4a; `/griddraw` -> 0x1017c8c0, `/onegrid` -> 0x1017c8e8).
- **`VisibleCells(cam)` 0x10087d83** (thunk 0x10001019) fills the NULL-terminated pointer list **0x1036f2a0** that the object pass walks (0x100a4e0a). Algorithm:
  1. If grid off or cam NULL: list = { `[0x1017c8cc]` }.
  2. Octant = ftol(fmod(heading(+0xc, made positive by +360) + 22.0, 360.0) / 45.0) (0x10087dc8..0x10087e11; 22.0 0x1012b8d8, 45.0 0x1012b8dc, 360.0 0x1012b8d4). Even octant: 2 diagonal + 3 side neighbours; odd: 4 + 4 (`[0x1036f408]`, `[0x1036f40c]`), start indices per octant from the jump table at 0x100881bd (`0x1036f3f0`, `0x1036f3f4`). Offset tables: diagonals 0x1014b898 = (-1,1),(1,1),(1,-1),(-1,-1); sides 0x1014b8b8 = (-1,0),(0,1),(1,0),(0,-1).
  3. Camera cell `cx = ftol(ftol((x + 254008)/55562) + 0.5)` (0x1036f3f8, row), `cz` from cam+0x8 (0x1036f400, column). If outside 0..8 -> `AllCells()` 0x10087c86.
  4. Unless `/onegrid`: for each chosen neighbour inside the grid and non-empty, test its bounding volume with `[0x1014cb40]`; add if visible. Then add the camera cell **last** (0x10088163).
- `AllCells()` 0x10087c86 (thunk 0x10002c93): list = `[0x1017c8c8]` (if non-empty) then all 81 non-empty cells in row order, NULL-terminated. Already used by the game when the camera is outside the grid and at mission load (0x10034de2).
- So objects (and terrain, which is drawn as objects in the cells; INFERRED from the 7937 rows = 55562/7) are only drawn in the camera's cell and the neighbours in front: about 55k..111k units ahead depending on where the camera is in its cell, nothing behind/beside except as listed.
- List capacity: 0x1036f2a0 holds 83 dwords (82 entries + NULL), the next variable is 0x1036f3ec. AllCells writes at most 1 + 81 + NULL = 83. Any replacement must not exceed 82 entries.
- Other users of VisibleCells: world render 0x1003527a (the `[0x1015e52c]` pass), **0x1004570f** (`FaceCamera` 0x100456fc: turns objects of class 0x10/0x20/0x60 in the visible cells towards the camera, INFERRED billboards, visual), 0x10061a84 (second view), 0x100a4dfb (main object pass), 0x100a5352. Gameplay spatial queries (0x1002d98d, 0x1002dce3, 0x1002de2e, 0x1002e3e7, 0x10067567) fill the same array with their own builder 0x1000186b first, so they do not depend on VisibleCells. **Do not use `/nogrid`**: queries like 0x1002db2d then search the global list instead of one cell, which can change results (INFERRED gameplay risk).

### 3.4 Object pass and its fixed buffers (VERIFIED)
- World render `RenderWorld` 0x10034ec9 (callback `[0x1014cb3c]`, from the main loop 0x1008620e): CameraViewport if dirty, camera update 0x100d255e, SetCurrentCamera, sky/horizon 0x100a0def (if `[0x1014cb04]`/`[0x1014cb08]`), sky-object list `[0x1015e544]` (class 0x90 objects, far = FLT_MAX, 0x100a4ff0), main objects `DrawObjects` 0x100a4d8e, cockpit list `[0x1015e548]` with near 8.0 (0x100a512a). In hardware mode drawing is deferred: entries are kept and drawn later by 0x100a4c45 (via 0x100352d5 before RenderRefresh).
- `AllocPolyBuffers(K, N)` 0x100a8816 (thunk 0x10001e5b), called once from `RenderInit` 0x10034cca (from SimMain 0x1008579f) as `push 0xbb8; push 0x400` at **0x10034d36/0x10034d3b**: one malloc of `K<<10 + N*16` bytes = 1 MiB polygon/vertex arena (`0x1017d170`.., polygon records 0x40 bytes allocated downwards by 0x100a88ff, vertex blocks upwards by 0x100a8943) + sort array A `[0x1017d168]` (N x 8 bytes {record*, float depth}) + array B `[0x1017d16c]` (N x 8, never read: slack). `[0x1017d17c] = N = 3000`.
- **0x10034d48: `mov [0x1017d140], 0xb54`** = 2900 polygon soft limit.
- Overflow handling is graceful: arena full (top-0xc8 <= bottom) or `[0x1017d138] >= N` (0x100a6e4c) clears `[0x101c9ea4]`, the polygon is dropped, and the walkers stop the current list once `[0x101c9ea4]==0` or `[0x1017d13c] >= [0x1017d140]` (0x100a48bb.., 0x100a49ac.., 0x100a4b29.., 0x100a4be6.., 0x100a5277..). Nothing overflows, but **objects are silently not drawn**, and VisibleCells adds the camera's own cell last, so the nearest objects are the first to go.
- Hardware mode puts the sky-object list first (`[0x1017d134]` entries) and appends the main list after it, but the capacity check compares only the main count with N (0x100a6e46). The worst case overruns array A by `[0x1017d134]` entries into array B, which is unused, so it is harmless.
- Sort: `SortByDepth(first, last)` 0x100a4700 (thunk 0x10002739), recursive quicksort, first element as pivot, **descending** depth (far first), recursion on both halves (0x100a481b, 0x100a4839). Worst-case recursion depth = n; frame about 48 bytes -> 144 KiB at 3000, 576 KiB at 12000 (HG.EXE and HGSIM reserve 1 MiB stack). Inclusive range: callers pass (A, A + (n-1)*8).
- Per-polygon vertex list: 255 entries (`[0x101c9ea8]` vs 0xff at 0x100a63d7, array 0x101c76a0). Not affected by distance.

### 3.5 Fog (VERIFIED)
- SKYGND.PAR parser 0x100a00ac..0x100a032e (format string 0x1014cb64): `density=Y,M,Z` -> Y `0x10143fc4`, M `0x101668cc`, Z `0x10143fc8`; `fogswitch` -> `0x1017d108`; `fogcolor` -> 0x1017d0f0/f4/f8, 0x1014c9ec.
- `FogFactor(z, h)` 0x10058ac2 (thunk 0x10002414), called from the vertex lighting at 0x10058eb8 and from `FogByte` 0x10058c3e (result * `[0x10143fcc]`, 255 in hardware, 128 in software):
  - distance term `d`: if Z == 0 -> 1; else if z < Z -> 1; else if Z < far and z < far -> `1 - (z - Z)/(far - Z)`; else 0; clamped to [0,1]. **far is `[0x10185bf8]`** (0x10058b03, 0x10058b13, 0x10058b29).
  - height term: if Y != 0: `t = Y/h` (Y>0) or `-h/Y` (Y<0) for h > 0; if t < M: d *= M, else if t < 1: d *= t; for h <= 0 and Y < 0... d *= M (0x10058c18..0x10058c34).
- So the fog always reaches full fog colour exactly at the far plane. Raising the far plane spreads the same fog over a longer range (thinner fog at every distance). Keeping the original look while drawing farther is not useful: everything past the original far would be solid fog colour.
- `[0x10143fcc]` is also scaled by a camera-height factor from Y/M in SetCurrentCamera (0x100d2214..0x100d22fc).

### 3.6 Sky and horizon (VERIFIED, for completeness)
- Sky texture quad: Render op 2 at 0x100a2798 (texture descriptor 0x101cfc90), z = 1 in the dump.
- Hardware horizon: `DrawHorizonHW` 0x100a0339 (op 6, or 2 if `[0x1017d0ac]` = 3dfx plugin), clipped with near / 9.0e7. In the dump its z spans 500..40000. With a depth-buffered plugin, terrain farther than ~40000 sits behind this band; the plugin must draw it as a backdrop (no depth write), or extended terrain will be hidden (INFERRED from the dump; HGVK already counts "backdrops").

## 4. Patch proposals

The hook layer (docs/hooks.md) only patches function entries. P-items marked "byte patch" need a small addition: write bytes at an RVA after verifying the old bytes (same check as the hooks). Every hook below has a prologue without relative branches.

### H1. Larger object/terrain range: replace `VisibleCells`
- Function 0x10087d83, RVA **0x87d83**, verify `55 8B EC 83 EC 2C` (6 bytes), cdecl `void VisibleCells(Camera *cam)`, caller cleans.
- Simplest: replacement calls `AllCells` at RVA 0x87c86 (cdecl, no args; not hooked) and returns. Uses an existing game path, so it is low risk. Cost: every non-empty cell is walked each frame; the per-object cull (far plane) then decides. The order is row-major, so if the sort buffer fills, whichever cells come last are dropped: do H5 too.
- Better: a C replacement that writes 0x1036f2a0 as `{camera cell, ring 1, ring 2, ... ring R}` (Chebyshev rings around (cx, cz), within 0..8, non-empty cells only, optional cell cull through `(*(int(**)(void*))0x1014cb40)(*(void**)(cell+0x28)) == 0`), NULL-terminated, **at most 82 entries**. Keep the original fall-backs: grid off or cam NULL -> `{[0x1017c8cc], NULL}`; camera outside the grid -> AllCells. Nearest-first order means that if the buffer fills, the far cells are dropped instead of the near ones. Compute cx/cz exactly as 3.3 step 3 (`(int)((float)(int)((x+254008.0f)/55562.0f) + 0.5f)`, truncating) so the camera cell matches the original.
- Gameplay: not affected (VisibleCells only feeds rendering and the billboard turn 0x100456fc; INFERRED that the latter is visual).
- Risk: performance (whole map within the far plane), more FaceCamera work, buffer limits (H5), horizon depth issue (3.6).

### H2. Far plane, children range and buffer limit: trampoline on `SetCurrentCamera`
- Function 0x100d1f90, RVA **0xd1f90**, verify `55 8B EC 83 EC 08` (6 bytes), cdecl `void SetCurrentCamera(Camera *cam)`, needs the trampoline.
- Before calling the original: if `cam == base + 0x14ca00` (main camera) set `*(float*)(cam+0x40) = FAR` (for example 400000.0f; keep it at or below 500000 unless the plugin's depth range is known to cope). The original then copies it to 0x10185bf8 and recomputes 0x10185b60.
- After the original: `*(float*)(base+0x185b60) = 3.4e38f` (always draw sub-objects); `*(int*)(base+0x17d140) = N_new - 100` (see H5; 0x10034d48 rewrites 2900 at init, so set it here); optionally `*(int*)(base+0x152418) = 1`.
- Writing 0x1014ca40 directly once after mission load also works (it is reread each frame), but the hook avoids having to know when the BWD `VIEW` chunk has been parsed.
- Risk: the map/second views call SetCurrentCamera too; the `cam ==` test keeps them unchanged. Fog thins (3.5); combine with H4 if a specific fog look is wanted.

### H3. Finest LOD always: trampoline on `CameraViewport`
- Function 0x100d2306, RVA **0xd2306**, verify `55 8B EC 83 EC 48` (6 bytes), cdecl `void CameraViewport(Camera *cam)`.
- After the original: `*(float*)(cam+0xb8) = 1.0e20f`.
- Alternatives: byte patch 0x100a4591 `75 08` -> `90 90`; or .rdata 0x1012bcc0 160.0f -> 1.0e-6f.
- Risk: many more polygons for far objects and terrain, so the sort buffer fills sooner (H5).

### H4 (optional). Fog independent of the far plane: replace `FogFactor`
- Function 0x10058ac2, RVA **0x58ac2**, verify `55 8B EC 83 EC 08` (6 bytes), cdecl `float FogFactor(float z, float h)`, result returned in ST(0) (an `__cdecl float` function in MSVC does that).
- Reimplement 3.5 exactly, with `fogEnd = origFar * k` (origFar = the far plane value before H2's override, or a per-mission constant) instead of `[0x10185bf8]`. k = 1 keeps the original look (and makes extra distance useless under fog), k = FAR/origFar gives the same thinning as H2 alone, anything between is a choice. Fog off entirely: `fogswitch` 0 per mission in SKYGND.PAR, or return 1.0f.
- Byte-patch alternative: point the three disp32 operands at 0x10058b05, 0x10058b15, 0x10058b2b (each `f8 5b 18 10`) to a private float.

### H5. Bigger polygon buffers
- Byte patch (preferred, before SimMain runs; RenderInit runs once at 0x1008579f): **0x10034d37** `b8 0b 00 00` -> N (e.g. 0x2ee0 = 12000); **0x10034d3c** `00 04 00 00` -> K (KiB of arena, e.g. 0x1000 = 4 MiB); **0x10034d4e** `54 0b 00 00` -> N-100.
- Hook alternative: `AllocPolyBuffers` 0x100a8816, RVA 0xa8816, verify `55 8B EC 51 8B 45 0C` (7 bytes), trampoline, call original with (K', N'); set `[0x1017d140]` in H2 because 0x10034d48 overwrites it right after the call.
- **Also replace `SortByDepth`** when N grows: 0x100a4700, RVA 0xa4700, verify `55 8B EC 83 EC 20` (6 bytes), cdecl `void SortByDepth(Entry *first, Entry *last)` (inclusive), `Entry = {void *rec; float depth;}`, sort descending by depth (an iterative or `qsort` sort; the original is not stable, so any order of equal keys is fine). This removes the O(n) recursion depth risk.
- Memory: 4 MiB + 12000*16 = about 4.2 MiB; the malloc failure path is SystemError 0x18.

### Detail options: data overrides
- Textures / terrain textmaps: keep `RenderFlags` 0x1014cb38 bits 0x100/0x200/0x800 clear. They come from HGSND.CFG +0x14/+0x18 (default 1 = on). Do not force the whole dword to 0 each frame: 0x100787f4/0x10078857 set 0xb00 on purpose for a special pass and restore it. Simplest is to hook the two setters 0x100988fc (RVA 0x988fc, `55 8B EC 8B 45 08`) and 0x100989a1 (RVA 0x989a1, `55 8B EC 8B 45 08`) and pass v=1 to the trampoline.
  Both setters take two arguments `(unused, v)`; v is the SECOND one ([ebp+0xC]). A one-argument hook leaves the caller's return address in v, and the game then saves that address into HGSND.CFG +0x14/+0x18 (seen as 0x10098758 / 0x1009876e). hooks_view.c passes `(unused, 1)`.
- Display detail: H3 makes the cfg value irrelevant for LOD. To also keep LodQuality at 1, hook 0x100d2c86 as in 2.3.
- Particles 0x1014b9b8, Multiple Lights 0x10143fd8, Shadows 0x1014d1d8: defaults are already 1; they change only through ApplySimOptions/menu. To lock them, hook the setters 0x10090d15 (RVA 0x90d15, `55 8B EC 83 EC 08`), 0x100585c4 (RVA 0x585c4, `55 8B EC 83 7D 08 FF`), 0x100ac603 (RVA 0xac603, `55 8B EC 83 7D 08 FF`) and replace any argument other than -1 (the query) with 1.
- Explosion chunks: already always 1. Object density: leave alone (never applied; hiding objects would change gameplay queries).
- Alternative without hooks: ship an HGSND.CFG with +0x1c..+0x30 = 1.

### Recommended set
1. H5 (byte patch or hook) + SortByDepth replacement.
2. H1 with the nearest-first ring builder (R = 8 covers the whole grid).
3. H2 with FAR around 300000..500000, children always on.
4. H3.
5. Option setters forced to "on" (or a fixed HGSND.CFG).
6. Decide the fog policy (H4) after looking at it in game; check that HGVK draws the hardware horizon (op 6, texture 0x101cfcb8) without writing depth.

## 5. Open points
- Per-mission far plane from the BWD `VIEW` chunk: values unknown (no mission files here). A tap on SetCurrentCamera that logs `cam->far` once per mission would answer it.
- Terrain being ordinary grid-cell objects is INFERRED (7937-unit rows = 55562/7, all polygons go through RenderObject/DrawModelPolygon); no separate terrain renderer was found.
- LOD node order is VERIFIED, see 2.4.
- When the menu engine calls ApplySimOptions (0x10098721) and who uses the function table at 0x101499a4..0x10149aa8 was not traced.
- Whether the effect throttle (0x10024c94) touches damage-dealing effects is unknown; LodQuality is left at its default 1 to avoid the question.
