# HGSIM.DLL: renderer-plugin usage, resolution and pixel-format assumptions

Source: `sim.asm` (objdump -d -M intel of HGSIM.DLL, image base 0x10000000), strings read with pefile.
All addresses are absolute VAs. **VERIFIED** = read directly in the listing/data. **INFERRED** = interpretation of verified code.
Helper names used below (mine, not in the binary): `HWFLAG` = dword 0x1017b940, `W/H` = dwords 0x1017b918/0x1017b91c.

## 0. Big picture

- HGSIM has TWO independent layers that both assume 640x480 and 8/16-bit pixels:
  1. a **display layer** (window, DirectDraw/DISPDIB/GDI/"plugin LFB" back-ends, an 8-bit palettised or 16-bit framebuffer) that the sim owns itself, and
  2. the **renderer plugin** (RASTER type) which receives already-projected screen-space polygons (36-byte vertices with x,y,z,1/z, flags, RGB) via `Render`.
- The 3D projection is done in the sim (VERIFIED: vertex block built at 0x10006609..0x10006730 computes `1.0/z` into the vertex, stride 0x24). The plugin only rasterises. So the sim, not the plugin, decides view window, centre, focal scale.
- The sim's 2D HUD/text is drawn by a statically linked Miles-VFX-style library (VERIFIED: `VFX_WINDOW/PANE` access pattern: window {buffer,x_max,y_max}, pane {window*,x0,y0,x1,y1}), with an **8-bit set** (0x100e84d0..0x100edc90) and a **16-bit clone** (0x100ef545..0x100f2f01). 244 `cmp HWFLAG,0` sites in 94 functions pick between them.

## 1. Plugin loading and every slot call site

### 1.1 Loading (VERIFIED)
- Plugin list: `LoadPlugins` 0x10036876 (thunk 0x1000135c) opens `dllfile.ini` (string 0x1014b204, call at 0x10085434), one DLL per line, builds `plugin\%s` (0x1013a208) and calls `LoadOnePlugin` 0x1003650d per line. Install's DLLFILE.INI contains only `3DFXGEAR.DLL`.
- Selection: `SetPluginType` 0x1003695b (thunk 0x100031b1) is called from SimMain with `("RASTER", NULL, id)`: id=1 if HWFLAG (`/d3d`) at 0x10085395, id=2 if `1017b944` (`/3dfx`) at 0x100853b1, id=0 otherwise at 0x100853c4. Stored in 0x1015e578 (wanted), 0x1015e57c (name), 0x1015e5bc (id).
- 0x1003650d: LoadLibraryA (0x1003651a), `GetFuncDesc` (0x1013a0ec) called with `(buf, &id, 0)`, must return the string `RASTER` (0x1013a0f8); id must equal the requested id (0x100365d0..0x100365dc) or name must match (0x100365ab). Then `CheckFunc` (0x1013a100) and 0x10036820 (accepts ids 0,1,2 only). Then `GetProcAddress` x16 at 0x10036664..0x100367cf.
- After load, SimMain 0x1008543f..0x10085451: if HWFLAG and plugin id (info+0x44) != 1, HWFLAG is cleared. 0x10085470..0x10085488: if the plugin file name contains `3dfx` sets `0x1017d0ac`=1.

### 1.2 Slot table (VERIFIED, stores at 0x1003665e..0x100367cf)
| addr | export | | addr | export |
|---|---|---|---|---|
| 0x109e3d20 | plugin id (type) | | 0x109e3d48 | LockDisplay |
| 0x109e3d24 | SetLumaTable | | 0x109e3d4c | UnlockDisplay |
| 0x109e3d28 | FirstDevice | | 0x109e3d50 | RefreshDisplay |
| 0x109e3d2c | LastDevice | | 0x109e3d54 | PreloadTexture |
| 0x109e3d30 | Render | | 0x109e3d58 | RenderNoClip |
| 0x109e3d34 | RenderRefresh | | 0x109e3d5c | RestoreDevice |
| 0x109e3d38 | (never filled) | | 0x109e3d60 | LostDeviceDisplay |
| 0x109e3d3c | GetSocketCaps | | 0x109e3d64 | UpdateTexture |
| 0x109e3d40 | SetState | | 0x109e3d44 | SetTexturePalette |

`GetNumDevice` is not requested. **GetSocketCaps, RenderNoClip, UpdateTexture have zero call sites** (VERIFIED: only the stores exist). The plugin id at 0x109e3d20 is only read by 0x10036850/0x10036859 (loader). Slots are never NULL-checked except LastDevice (0x1003550c); a plugin that omits Lock/Unlock/RefreshDisplay (e.g. DX5DRAW exports none) leaves them NULL, which is only safe because the class-3 display driver (section 4) is used only with `/3dfx`.

### 1.3 Call sites
| slot | call VA | containing function | args (cdecl) and sources |
|---|---|---|---|
| SetLumaTable | 0x100065fb | 0x1000656f | `(0x10ab1ae0)`: 32x256 byte table built from a cached file (0x10003413 with name 0x1014cf0c; rows reversed 31-r) |
| SetLumaTable | 0x100d1ebc | 0x100d1e10 | `(0x10185c60)` same construction, once (guard 0x1017e610) |
| FirstDevice | 0x10085e8f | SimMain 0x10084feb | `(&param)`, param block 0x10085dc0..0x10085e88, see 2.2 |
| LastDevice | 0x10035515 | 0x100354ff (shutdown) | no args; only if slot non-NULL |
| RenderRefresh | 0x10086477 | SimMain main loop | no args; once per frame, see 4 |
| SetState | 0x1004a6be | 0x1004954e | `(0x200000, 0)` |
| SetState | 0x1004a6d9 | 0x1004954e | `(0x200000, 1)` (guard 0x1017d108) |
| SetState | 0x1006b878 | 0x1006ad98 | `(0x200000, 1)` (guard 0x1017d108) |
| SetTexturePalette | 0x100d3130 | 0x100d3000 | `(0, 0x10185760)`: 256 dwords `0x00RRGGBB` = 6-bit VGA components x4, loaded from file name 0x10185620 |
| PreloadTexture | 0x10006fee | 0x10006e52 | `(&{...}, word[tex+0x12])` |
| PreloadTexture | 0x100a2af0, 0x100a2b81 | 0x100a2a66 | `(0x101cfc90 / 0x101cfcb8 descriptor, 0)` for two sprite sets (0x1017d0fc, 0x1017d100) |
| LockDisplay | 0x10087842 | 0x10087820 (class-3 rect fill) | `(&out)`, returns 16-bit pixel pointer, out used as row width |
| LockDisplay | 0x10087a1e | 0x10087a0c (class-3 set-mode) | `(&out)`; sets `0x1017b984`=1, writes `[0x1015e698]=ptr`, `[+4]=out-1` |
| UnlockDisplay | 0x100877cf, 0x100878af | 0x100877c3 (present), 0x10087820 | no args |
| RefreshDisplay | 0x100877d5, 0x100877e2, 0x100877ef, 0x100877fc, 0x10087809, 0x10087816 | class-3 driver methods 0x100877c3..0x10087813 | no args (all driver methods that "present" just call it) |
| RestoreDevice | 0x10086921 | SimWindowProc 0x100867a2, WM_ACTIVATEAPP (0x1c) active branch, if 0x1017b934 | no args |
| LostDeviceDisplay | 0x10086935 | same, inactive branch | no args |

**Render** is called as `Render(op, ctx, a3, a4, a5)` (5 dwords, caller cleans 0x14). `ctx` is `0x109e3dc0` except at 0x100d1f11/0x100d1f2e where it is the caller's first parameter. Ops seen: 1, 2, 4(in a5 slot only), 6, 0xd, 0xf, 0x10.
| call VA | function | op | a3 / a4 / a5 |
|---|---|---|---|
| 0x10006732 | 0x10006609 | 2 | vertex array [ebp+0x10], count [ebp+0xc] (>=3), &desc |
| 0x1000687b / 0x100068a9 / 0x100068c7 | 0x10006744 | 1 if [ebp+0x14]!=0, else 6 (HWFLAG) / 2 | same shape |
| 0x100069ff / 0x10006a1d | 0x100068d9 | 6 if HWFLAG else 2 | same shape |
| 0x1009f198, 0x1009f660, 0x1009f9a0 | 0x1009ef50 | 0xd | [ebp+0xc], [ebp+8], &[ebp-0x3c] |
| 0x1009f778 | 0x1009ef50 | 0x10 | same |
| 0x1009fe44 | 0x1009fada | 0x10 | &[ebp-0x1210], 8, &[ebp-0x1234] |
| 0x100a04df / 0x100a0500 | 0x100a0339 | 2 / 6 (HWFLAG) | [ebp+0x10], [ebp+8], &[ebp-0x23e8] |
| 0x100a2798 | 0x100a0def | 2 | &[ebp-0x178], 4, 0x101cfc90 |
| 0x100ac898 | 0x100ac738 | 0xf | &[ebp-0x23f4], [ebp+0xc], &[ebp-0x10] |
| 0x100ad4d6 | 0x100ad1f3 | 0xf | &[ebp-0x9f94], [ebp-0x14], &[ebp-0x7bac] |
| 0x100d1f11 / 0x100d1f2e | 0x100d1e10 | 6 (HWFLAG) / 2 | [ebp+0x1c], [ebp+0x18], &[ebp-0x2c] |
Op meanings are INFERRED only (2 = software-flavour polygon batch, 6 = same for HWFLAG, 1 = textured variant, 0xd/0xf/0x10 = other prim types). The DX5DRAW `Render` (0x10001370) ignores a2 (ctx) and dispatches `op<=0x12` through the jump table at 0x10004b64 (VERIFIED).

## 2. Where resolution is fixed

### 2.1 How W/H get their value (VERIFIED)
- Command line parser `0x1002ad50` (thunk 0x10001041), called at 0x1008534a. Switches: `d3d` (0x1002adde sets HWFLAG), `3dfx` (0x1002ae02 sets 0x1017b944), `high` (0x1002aea1), `low` (0x1002aee5), also grid/nogrid/griddraw/onegrid/lutdump/hgdebug etc.
- `/high` copies the string `vesa480.dll` to `0x109e89e4` (0x1002aec7) and sets bit0 of `0x109e89e0`; `/low` copies `mcga.dll` (0x1002af0b). Without either, the name comes from **HGSND.CFG**: struct read at 0x10085222 (file `hgsnd.cfg`, 108 bytes) into 0x1015d1b8, field +0x48 (here the ASCII `vesa480.dll`, field +0x44 = 8) copied at 0x10085324 BEFORE the cmd line is parsed, so `/high` `/low` override the file.
- `0x10086d8d` (thunk 0x10002fb3, called at 0x10085419 with 0x109e89e4) maps the driver name to W/H: `MCGA.DLL` -> 320x200 (0x10086da5), `VESA480.DLL` -> 640x480 (0x10086dd0), `VESA768.DLL` -> 1024x768 (0x10086dfb), anything else -> 320x200 (0x10086e11). **The `VFX\*.DLL` files are never loaded by HGSIM**: only the names are string-compared. (`vfx`, `vfxjk`, `vfxhd` strings at 0x1013ac60..0x1013ac78 are control-device file extensions, not video.)
- The driver name strings are written by HGSHELL (strings `mcga.dll`/`vesa480.dll`/`Software 640`/` /high /d3d`/` /high /3dfx`/`hgsnd.cfg`/`hgprm.cfg` appear in HGSHELL.DLL via strings(1); HG.EXE contains `hgprm.cfg`). HGPRM.CFG in the install is `RAN1SCN1 /high /3dfx` = the sim command line. HGSIM itself does not reference hgprm.cfg or CHARINFO. CACHE.INI (via `FirstResource`, 0x100856af) only holds cache sizes; HG.INI is the SYSERR text table. VERIFIED by content/absence in HGSIM string scan.
- Net: with `/high` the sim runs 640x480; the only other reachable sizes are 320x200 and 1024x768, and only the software path is expected to use them.

### 2.2 FirstDevice gate and parameter block (VERIFIED, SimMain)
- 0x10085d35 `cmp W,0x280`, 0x10085d45 `cmp H,0x1e0`, 0x10085d55 `cmp HWFLAG,0`: FirstDevice (and the whole block 0x10085d62..0x10085e98) runs **only if 640x480 and HWFLAG**. At any other size the HW plugin is never initialised.
- Block: `ctx` 0x109e3dc0 = {1, width=[canvas.window+4]+1, height=[+8]+1, 0, clip 0,0,[0x109e3e0c],[0x109e3e10]} (0x10085d62..0x10085dc3), and `[window+8] -= 1` (0x10085d85..0x10085d96, bottom row dropped).
- Param struct at ebp-0x88: +0x00 &ctx, +0x04 = 8, +0x08 flags 0x80 (|0x200 if hgsnd.cfg+0x3c, 0x10085e2b; |2 if 0x1017d108, 0x10085e3a), +0x10 = 3, +0x14 = fog RGBA from 0x1014c9ec/0x1017d0f0/f4/f8, +0x18 = 1.0f, +0x1c = 500000.0f (0x48f42400), +0x20 = **0x1017b390 (the sim's IDirectDraw*)**, +0x24 = 0, +0x28 = 0x200, +0x2c = **0x1017b39c (DDraw back surface)**, **+0x3c = 0x280, +0x40 = 0x1e0** (0x10085df1/0x10085df8), +0x44 = 0xffff, +0x4c = HWND (0x1017b8f0). DX5DRAW FirstDevice (0x10001290) fails if +0x20 is NULL; 3DFXGEAR FirstDevice (0x10001230) reads +4 (==8), +8 bit7/bit8, +0x28, +0x3c, +0x40 as floats. Meaning of +0x28/+0x44 is INFERRED/unknown.

### 2.3 Other hard-coded 640/480 (VERIFIED immediates)
- Frame-end border fills: 0x10086349..0x100863d6 (`0x27f`,`0x1df`, colour 0xf0 in 8-bit).
- Black full-screen quad in float: 0x441fc000 (639.0) / 0x43ef8000 (479.0) at 0x10035035..0x10035066, 0x1003536a..0x100353ab (functions 0x10034ec9 and 0x100352d5).
- DDraw/GDI back-ends: 0x100805d6 (mode 1 set-mode) and 0x10080c57 (modes 2/3): `0x27f`,`0x1e0`,`imul 0x280` at 0x100807d7..0x10080b03, 0x10080e4b..0x1008116f; GDI/DIB 640-stride 0x10083801 (`imul 0x280` at 0x10083a46..0x10083ba9). 320x200 special cases: 0x1007fa3f/0x1007fa4b/0x1007fa7f/0x10080344/0x1008034d.
- Window proc: 0x1008688b/0x10086897 and 0x1008693b/0x1008694b compare desktop size 0x1015e9c4/0x1015e9c8 with 640x480 (ShowWindow choice); `ToggleFullScreen` error strings at 0x1013a398/0x1013a414.
- 0x100b82a0, 0x100b8834, 0x100b8938 (`add ..,0x1df/0x27f`, `sub ..,0x140` at 0x100b8514), 0x100b9b3f (`push 0x2ff`), 0x100be0f0 (`imul 0x1c2`), 0x100bcd61 (`push 0x140`): not examined in context (INFERRED to be UI/menu or HUD code).
- Also `0x1008ce40..0x1008d0b5` (`mov ebx,0xc7`), 0x10075371 (`[0x1015dc18]=0xc7`), 0x100d3142 (`[0x10185410]=0xc7`) = 199 (320x200 height-1) in 2D code.

### 2.4 Resolution-class index and HUD layout tables (VERIFIED unless stated)
- `0x100ab89a` (thunk 0x10001de8) picks `0x101c64f0` = nearest of three presets in table 0x1014d1b8 = {(319,199),(639,479),(1023,767)} by Manhattan distance to (w-1,h-1). Called from canvas init `0x10034c11` (0x10034c31) with the {w,h} block 0x109e3da4 built by 0x10034b30 from W/H. 1920x1080 would give index 2.
- `0x100ab870` (thunk 0x10003a21) computes camera aspect `h*1.3333334/w` into 0x101c64e0 and camera(0x1014cae0)+0x44 (0x100ab876..0x100ab895). It bakes a 4:3 pixel-aspect assumption: 640x480 -> 1.0, 1920x1080 -> 0.75.
- About 30 consumers of 0x101c64f0: e.g. 0x100355e1 (resource name `sbannr` + suffix from 0x1014d1b0 = "", "6", "k" + `.gif`), 0x1002ba2c..0x1002bc2f (`InitHudTables` 0x1002ba28, thunk 0x10003783, arg 0/1), 0x1004824f, 0x10062820, 0x1006a1c1, 0x100760a9, 0x1007dede (font), 0x1008b7d6, 0x100c780e, 0x100cda5a..0x100ce780, 0x100d3efb.
- `InitHudTables` copies, per `(arg + idx*2)`: 30 panes x 0x14 bytes (`{window*=0x109e3e20,x0,y0,x1,y1}`) from 0x101355c0 stride 600 to 0x109e8660; 30x8 bytes from 0x10135f20 stride 0xf0 to 0x109e8240; 119 x 16-byte labels from 0x101362e0 stride 0x770 to 0x109e8480; plus 0x1017e5e4 from 0x10151d58 = {-90,-195,19}; 0x109b98c0/0x109b9900 from 0x101380a0/0x101380a9 stride 0x12; fixed rects 0x10145290.. = (0,0,0xeb,0xbe),(0x17e,0,0x102,0xbe). **The three big tables are contiguous and sized for only 4 sets** (0x101355c0+4*600=0x10135f20; +4*0xf0=0x101362e0; +4*0x770=0x101380a0), i.e. only idx 0 and 1 (320x200, 640x480). idx 2 (1024x768, and thus any big screen) reads out of table (VERIFIED arithmetic). The pane coordinates in the 640x480 set are all <= 319 wide (0x10135a70..): how the 640x480 HUD is composed is UNKNOWN.
- 3D view window: camera viewport maths in `0x100d2306` (thunk 0x10003170) is resolution-generic: from camera rect fields +0x2c..+0x38 it derives centre (+0x8c,+0x90), half sizes (+0x84,+0x88), focal factors (+0xb0,+0x94,+0x98,+0xa8,+0xac) and +0xb8 = halfwidth*zoom / (detail * 160.0) (160.0 at 0x1012bcc0; detail = 0x10152418, 1=high 2=low LOD toggle from the `LOD Quality HIGH/LOW` menu code 0x1008bfc7..0x1008bfda, 0x100b20bf). The rect itself comes from mission data (BWD keyword `window`, string 0x10134ba8) and the HUD pane tables; the handler was not located.

### 2.5 /high, low detail
- `/high` = VESA480 (640x480), `/low` = MCGA (320x200) in the sense above (VERIFIED); neither changes any other quality setting. The in-game "LOD Quality" toggle changes only 0x10152418 (texture/LOD scale), not resolution (INFERRED from 0x100d2510..0x100d2522). `generate_gammas()` (0x10085744) and `Monitor Brightness` (0x1014d624, `hgsnd.cfg+0x44` -> 0x1013b990/0x1013b98c) are brightness only.

## 3. Pixel format assumptions

- **Software/default**: 8-bit palettised. DirectDraw back-end `SetDisplayMode(w,h,8)` (0x1007ffad..0x1007ffc6), `CreatePalette(0x44 = 8BIT|ALLOW256)` + `SetPalette(primary)` (0x100802ac..0x10080319), palette writers `DDRAW_WritePaletteEntries/Gamma/PaletteFade` strings 0x1014a73c..0x1014a8f8. Startup check at 0x10085ea0: `GetDeviceCaps(hdc, NUMCOLORS=0x18)`, `== -1` (true-colour desktop) takes the `StartPalettes()` path (string 0x1014b5d0) without installing a palette; else palette loaded from file 0x10185720 and set through display-driver method +0x14 (`call [eax+0x14]` at 0x10085f2b).
- **HWFLAG (d3d/3dfx)**: **16-bit**. `SetDisplayMode(w,h,16)` at 0x1007ff8b..0x1007ffa3; the 16-bit VFX clone (pitch = (x_max+1)<<1, e.g. 0x100ef753) is used; colours come from palette->RGB16 LUT 0x10156dcc (set by 0x100f01cc, read by 0x100f01ea) copied into 0x101659e0 (0x100d3118) while in SW mode that table is the identity (0x100d310f). Palette write functions convert 6-bit VGA palette entries to RGB565 (`(r>>1)<<11 | g<<5 | b>>1`, e.g. 0x100878bb..0x10087925) or RGB555 (`<<10`, 0x10081b27..0x10081b6d) after `GetPixelFormat` (0x100819e2, popcount of the primary surface masks, packed `r<<16|g<<8|b`, 0x50605 = 565). Class-3 driver reports 565 unconditionally (0x10087a43, `mov eax,0x50605`). So **only 16bpp (565/555) is supported; no 24/32-bit path exists in the sim**.
- **Where the sim writes pixels itself (VERIFIED by the VFX 8/16-bit pairs; semantic labels INFERRED)**: HUD gauges, target info, timers, text (`"%3.0f m"`, `"%2.2d:%2.2d:%05.2f"`, `VTGT_NP`): functions 0x100c7120..0x100d0600 (76 gating sites); menu/cockpit/overlay code 0x10050000..0x100a0000; text 0x1007df6c (thunk 0x100032d8, 56 callers); borders 0x10086349; clear 0x10001f87 (pane wipe, 16 callers). Top shared primitives (thunk, 8-bit impl / 16-bit impl, callers): line/rect 0x10002b1c/0x10001d70 (76/76), 0x10002c48/0x10002ea5 (65/64) and 0x100022ed (63), 28 x 0x10001e3d, pane wipe 0x10001f87 (0x100eb070) / 0x10002da1 (0x100f0865).
- **Plugin draws**: all 3D polygons through Render; the HUD is NOT drawn by the plugin. In D3D mode both write into the same DirectDraw back surface (sim passes `0x1017b390`/`0x1017b39c` to FirstDevice, 2.2). In 3dfx mode the sim obtains the frame buffer pointer via LockDisplay (class-3 driver, section 4) and writes 16-bit pixels itself.
- Plugin texture palette is 32-bit RGB (SetTexturePalette) built from the same 6-bit palette (0x100d3086..0x100d30ee).

## 4. Frame end, presentation, window ownership

- **Window**: owned by HG.EXE. SimMain's entry args hold the HWND (`[ebp+0x1c]` -> 0x1017b8f0 at 0x1008504c) and the cmd line (`This program must be launched from HG.EXE`, string at 0x101354e8 is printed if no command line, 0x1002ad63). HGSIM restyles and positions it in `0x100381d0` (`SetWindowLong` GWL_STYLE 0x100381fc/0x1003821f, `SetWindowPos` 0x10038290) and `0x1003795b` (thunk 0x10001ee2, "InitDrawMode(mode, allowFallback, &bitmap, w, h, ...)": if requested size >= desktop (GetSystemMetrics) -> WS_POPUP 0x80000000, else caption style 0xca0000 + `AdjustWindowRect` (0x10037a67) centred). Its WndProc is `SimWindowProc` 0x100867a2. `CreateWindowExA` is called once, for the DISPDIB child window at 0x1007eb53.
- **Display back-ends** (table 0x1013a298 classes, mode table 0x1013a2a8; VERIFIED structure, class names from the strings in each class): mode ids 0-3 -> class 0 = DirectDraw (0x1007f9da.., fullscreen exclusive, `SetCooperativeLevel(hwnd,0x51)` 0x1007ff41; primary + back + optional stretch/offscreen surfaces), mode 4 -> class 1 = DISPDIB (0x1007ea30), mode 5 -> class 2 = GDI DIB (0x10083530..; `SetDIBitsToDevice`/`StretchDIBits`), mode 6 -> class 3 (0x1014b838) = **plugin-owned display** (LockDisplay/UnlockDisplay/RefreshDisplay, 0x100877bc..0x10087a4c).
- **Mode selection** (VERIFIED, SimMain): software = mode -1 with fallback chain (0x10085d18 call with `[0x1013a00c]=-1,[0x1013a010]=1`), `/d3d` = mode 0, no fallback (0x10085cb2..0x10085cbc), `/3dfx` = mode 6 with driver pointer 0x1015e688 = 0x1014b838 preset (0x10085ccf..0x10085ced).
- **D3D**: sim creates the IDirectDraw (DirectDrawEnumerateA via 0x1010ce70 with callback 0x10080372 when HWFLAG, 0x1007fed6..0x1007fee5), sets the 16bpp display mode, creates the flip chain, then hands object and back surface to the plugin.
- **Per-frame (VERIFIED order in SimMain main loop 0x1008605f..)**: world/render (0x10086278 `call 0x10002dd3`); if software, display-driver method `[0x1015e688+0x20]` decides redraw (0x1008628f) and the 640x480 border fills run (0x10086328..0x100863e5); then `0x1000370b` (0x10086463, thunk to 0x10035478: `[0x1015e68c+0x20]`/`+0x18` driver call = present; copies saved pane back from 0x1015e518), `0x10002b94` (0x10086472, thunk to 0x100352d5: in HWFLAG mode draws the black 639x479 quad through callback 0x1014cb4c and refreshes the camera, clears the pane), then **`RenderRefresh` at 0x10086477**. The per-frame ctx refresh from the canvas pane is at 0x10034f39..0x10034f9d. Which of these actually flips the D3D back buffer vs. the plugin's `RenderRefresh` is INFERRED (RenderRefresh is the plugin hook; DX5DRAW forwards it to its device object).
- The sim's own `DDRAW_*` code (Flip/Blit/StretchBlit/BlitRect/Fill/Palette* strings 0x1014a44c..0x1014a93c) is the software-mode presenter; it is bypassed in `/3dfx` (class 3) and shared with the plugin in `/d3d`.

## 5. Minimum change list to run the 3D view at an arbitrary size (e.g. 1920x1080), ranked

Assumption: a new Vulkan plugin owns its swapchain and uses the `/3dfx`-style class-3 display path for the sim's own 2D drawing.
1. **Mode/driver selection** `0x10085ca9..0x10085ced` and cmd-line `0x1002adfb..0x1002ae0c`: add a switch that behaves like `/3dfx` (HWFLAG=1, mode 6, driver 0x1014b838, plugin id) so the sim creates no DirectDraw. Also `0x10085395..0x100853cd` (plugin id for SetPluginType).
2. **W/H source** `0x10086d8d` (`0x10086dd0/0x10086dda` etc.) and callers `0x10085419`, `0x1002aea1`: allow arbitrary W/H (cmd-line or INI) in `0x1017b918/0x1017b91c`.
3. **FirstDevice gate and params** `0x10085d35..0x10085d5c` (remove 640x480 test) and `0x10085df1/0x10085df8` (use W/H instead of 0x280/0x1e0).
4. **Resolution class / HUD tables** `0x100ab89a` (limit/extend index; idx 2 reads out of table) and `InitHudTables` `0x1002ba28`; `0x100ab870` aspect formula (assumes 4:3) and camera rect source (`window` keyword, `0x100d2306` is already generic).
5. **Border/clear constants** `0x10086349..0x100863d6`, `0x10035035..0x100353ab` (639.0f/479.0f), `0x10083a46..`, `0x10080e4b..`, `0x100807d7..` (only needed if those back-ends stay in use), `0x100b82a0/0x100b8834/0x100b8938`.
6. **Class-3 display driver** `0x100877bc..0x10087a4c` and `0x10087a0c` (LockDisplay out value is used as `x_max+1`; 3DFXGEAR returns strideInBytes): make the plugin return a CPU-visible 16bpp (565) buffer and width, or change the VFX 16-bit window creation.
7. Pixel depth: nothing in the sim can output 32-bit. A 32-bit swapchain must be fed from the 565 buffer (plugin-side upload) or the 16-bit VFX clone (0x100ef545..0x100f2f01) and palette->RGB16 LUT (0x100f01cc/0x100f01ea, 0x100d3118, writers at 0x10081aff.., 0x1008791b..) must be widened.

## 6. Not determined
- Source of the 3D view rectangle (handler of BWD `window`, camera fields +0x2c..+0x38) and how the 640x480 HUD panes (<=319 wide) are used.
- Behaviour of software mode with the RENDDRAW/I76DRAW plugins (FirstDevice is not called there; ctx initialisation in that case unclear) and whether HGSHELL/HG.EXE impose their own resolution limits (only strings checked).
- Exact semantics of Render ops (1,2,6,0xd,0xf,0x10), SetState id 0x200000, FirstDevice fields +0x28/+0x44; LockDisplay stride units (pixels vs bytes).
