# Heavy Gear renderer plugin API (reverse-engineered from PLUGIN\DX5DRAW.DLL)

Image base 0x10000000 for all modules. VA = virtual address. "DX5" = DX5DRAW.DLL, "SIM" = HGSIM.DLL, "SHELL" = HGSHELL.DLL.
Tags: **VERIFIED** = read in the disassembly. **INFERRED** = deduced, not directly confirmed.
All exports are `cdecl` thunks (VC5) that forward to `thiscall` methods of one device object (size 0x1b38, global pointer at 0x10072be4). Offsets written `dev+X` are into that object.

## 1. Discovery and loading (VERIFIED)

SIM loader at 0x1003650d, per candidate `plugin\*.dll`:
1. `LoadLibrary`.
2. `GetFuncDesc(char kind[0x100], int *id, char *extra)`. kind must equal "RASTER" (string at SIM 0x1013a0f8).
3. `CheckFunc(const char *ddrawDriverName_or_NULL)`. A return of 0 rejects the plugin.
4. Validator 0x10036820(result, id): accepts only id 0, 1 or 2. The id is stored at SIM 0x109e3d20.
5. `GetProcAddress` for the export names, filling the slot table (SIM 0x109e3d24..0x109e3d64):

| slot | export | slot | export |
|---|---|---|---|
| d24 | SetLumaTable | d4c | UnlockDisplay (optional) |
| d28 | FirstDevice | d50 | RefreshDisplay (optional) |
| d2c | LastDevice | d54 | PreloadTexture |
| d30 | Render | d58 | RenderNoClip |
| d34 | RenderRefresh | d5c | RestoreDevice |
| d38 | unused (GetNumDevice not loaded by SIM) | d60 | LostDeviceDisplay |
| d3c | GetSocketCaps | d64 | UpdateTexture |
| d40 | SetState | d44 | SetTexturePalette |
| d48 | LockDisplay (optional) | | |

SHELL has its own table at 0x102723a4..0x102723e4 in the same order.

Plugin ids: DX5DRAW returns 1, RENDDRAW and 3DFXGEAR return 2, I76DRAW returns 0 (stores at 0x1000622f).

DX5DRAW does NOT export LockDisplay, UnlockDisplay or RefreshDisplay, so those slots are NULL. SIM calls them without a null check in its software display functions (0x100877c3.., d48 at 0x10087842/0x10087a1e, d50 at 0x100877d5). Those paths are only valid for software/Glide plugins. In the hardware path SIM uses DirectDraw surfaces directly (Flip vtable +0x2c, Blt +0x14, Lock +0x64, Unlock +0x80, code about 0x1007f9xx..0x10082800). A replacement exporting these three would be safer.
LastDevice is null-checked by SIM (0x1003550c).

## 2. Export reference

### GetFuncDesc  0x10001000  `int GetFuncDesc(char *kind, int *id, char *extra)` -> 0 (VERIFIED)
Copies "RASTER" (0x1000f03c) into kind. If extra is non-NULL copies "Direct3D" (0x1000f030). Writes *id = 1 (0x10001058).

### CheckFunc  0x10001060  `int CheckFunc(const char *ddrawDriverName /*may be NULL*/)` -> 1 usable / 0 not (VERIFIED)
Stores name at 0x10012850, clears result 0x10012854. `DirectDrawEnumerateA` with callback 0x10001150: with a name, strcmp against driver name then `DirectDrawCreate`; with NULL, skips the primary (NULL GUID) entry and takes the first device whose HAL caps has DDCAPS_3D. Falls back to `DirectDrawCreate(NULL)`. QueryInterface IDirect3D, `EnumDevices` callback 0x10001120 sets result=1 if the HW device desc dcmColorModel (+8) is nonzero. The probe objects are separate from the game's DirectDraw.

### GetNumDevice  0x10001390  `int GetNumDevice(void *buf)` (VERIFIED)
Enumerates DirectDraw drivers (callback 0x100013c0); for each with DDCAPS_3D writes a 0x140-byte entry at buf+n*0x140: +0x00 description string, +0x100 driver name string. Returns the count (0x10012858), or 0 if enumeration failed. buf must not be NULL. SIM calls it (0x1009a26a, 0x100363af) to build the options device list; the returned driver name is what is later fed to CheckFunc. Not in SIM's slot table.

### FirstDevice  0x10001290  `int FirstDevice(struct P *p)` -> 0 success, 1 failure (VERIFIED)
Returns 1 if p->ddraw (P+0x20) is NULL. Otherwise `new` 0x1b38 bytes (a second call without LastDevice leaks the first), ctor 0x10002830, init-A 0x100028a0(p+0x20), init-B 0x10002c40(p). SIM stores the return at [ebp-0xe4] and never checks it. SIM calls it at 0x10085e8f only when display is 640x480 and [0x1017b940] (HW/hi-colour flag) is nonzero.
Init-A: QI the game's IDirectDraw for IDirectDraw2 (IID at 0x1000e038); `GetAvailableVidMem` -> flag at 0x10072bec set when total texture memory > 0x800000; QI IDirect3D2 (dev+0x10); EnumDevices callback 0x10002980 copies HW/HEL D3DDEVICEDESC (0xcc bytes) into dev+0x7c...
Init-B: render target = p+0x2c -> dev+0x20; dev+0x1b34 = 1.0/(float)p+0x44; `IDirect3D2::CreateDevice` (clsid 0x1000e0e8) -> dev+0x14; EnumTextureFormats callback 0x10003030 selects the texture format; CreateViewport/AddViewport/SetViewport2 (0,0,W,H, minZ 0, maxZ 1) -> dev+0x18; texture cache of capacity p+0x28 (0x10005c60, array at 0x10072be8); initial render states via 0x10002b10.

#### Parameter struct P (VERIFIED, built by SIM at 0x10085dc0..0x10085e88)
| off | size | meaning | read at |
|---|---|---|---|
| +0x00 | ptr | surface/rect descriptor at SIM 0x109e3dc0 ({1,w,h,...}); DX5 never reads it. Used by software plugins. | none in DX5 |
| +0x04 | int | preferred texture bpp, SIM passes 8. 8 means paletted textures when total VRAM <= 8 MB and the driver supports it | 0x10002ddc |
| +0x08 | uint | flags, see below | 0x10002cda.. |
| +0x0c | | unused | |
| +0x10 | int | fog mode (SIM: 3, linear) -> dev+0x54 | 0x10002fba.. |
| +0x14 | uint | fog colour ARGB -> dev+0x50 | |
| +0x18 | float | fog start -> dev+0x58 (SIM 1.0) | |
| +0x1c | float | fog end -> dev+0x5c (SIM 500000.0) | |
| +0x20 | IDirectDraw* | the game's DirectDraw object (SIM [0x1017b390]); must be nonzero | 0x10001290, init-A |
| +0x24 | | 0, unused | |
| +0x28 | int | texture cache capacity (0x200) | 0x10002c.. |
| +0x2c | IDirectDrawSurface* | render target surface (SIM [0x1017b39c]) -> dev+0x20 | init-B |
| +0x3c | int | width (640); becomes clip W (0x10012aa8) | |
| +0x40 | int | height (480); clip H (0x10012aa4) | |
| +0x44 | int | z mask (0xffff); dev+0x1b34 = 1/zmask (0x10002c70) | |
| +0x4c | HWND | window handle (SIM [0x1017b8f0]); DX5 ignores it, RENDDRAW reads it (0x1000234c) | none in DX5 |

P flags (+0x08), VERIFIED in DX5:
- 0x01: create 16-bit Z-buffer in VRAM and attach (0x10002cda). SIM never sets it.
- 0x02: fog; needs device cap 0x40000 (0x10002fba); uses +0x10/+0x14/+0x18/+0x1c. SIM sets it when [0x1017d108] != 0; colour = [0x1014c9ec]<<24|[0x1017d0f0]<<16|[0x1017d0f4]<<8|[0x1017d0f8].
- 0x04: antialias (dev+0x48 = 1).
- 0x08: dither (if raster cap present).
- 0x10: specular.
- 0x80: 0x10072bc8 = (flags&0xff)>>7, "share one DDraw palette". SIM always sets it.
- 0x100: 0x10072bf0, palette entries are 16-bit 565. SIM never sets it.
- 0x200: SetState(4, 0x20000) at end of init (bilinear). SIM sets it if [[0x1015d1b8]+0x3c] != 0.
SHELL builds its own P (0x1016ff6c..0x10070046): +4=8, flags=2, fog mode 3, colour 0x40ead5c1, start 1.0, end 2.0, cache 0x200, 640x480; +0x44 and +0x4c are left uninitialised (DX5 uses +0x44! INFERRED: the shell's value is whatever is in the stack; a replacement should default to 0xffff if it looks bogus).

### LastDevice  0x10001330  `void LastDevice(void)` (VERIFIED)
Cleanup 0x10005c00: releases the shared palette (0x10072bf4), frees cache (releasing each record's surface via 0x10005b80), releases D3DDevice (dev+0x14) and IDirect3D2 (dev+0x10), `delete this`, global=0. Does not release the game's DirectDraw or the render surface.

### GetSocketCaps  0x100014e0  `unsigned GetSocketCaps(void)` (VERIFIED)
No args. Returns dev+0x924: bitmask of features actually enabled: 1 zbuffer, 2 fog, 4 antialias, 8 dither, 0x10 specular. No SIM or SHELL call sites (VERIFIED by search), so unused.

### SetState  0x100014f0 -> 0x100033e0  `int SetState(unsigned code, unsigned value)` (VERIFIED)
Jump table 0x1000367c, valid code 1..7; others return 1 with no effect. Valid paths return 0.
| code | effect |
|---|---|
| 1 | value&0x10000: gouraud (SHADEMODE 2), else flat (1) |
| 2 | fog; needs dev caps 0x40000; value&0x200000 -> FOGENABLE (RS 28) |
| 3 | value&0x80000: texture address CLAMP(3) else WRAP(1); needs dev caps &3 |
| 4 | value&0x20000: bilinear (MAG/MIN filter 2) else nearest (1); needs dev caps &2 |
| 5 | value&0x40000: alpha blend on (ALPHABLEND, SRC=5 SRCALPHA, DEST=6 INVSRCALPHA) else off |
| 6 | store value at 0x10072944 (default 0x99); no reader in DX5. RENDDRAW/3DFX store a byte |
| 7 | value&0x400000: COLORKEYENABLE (RS 0x29) |
The flag bits (0x10000, 0x20000, ...) look like the game's own render flags. The plugin itself calls SetState(1/3/5/7,...) per Render mode.
SIM's three call sites (0x1004a6be, 0x1004a6d9, 0x1006b878) pass arg1 = 0x200000, arg2 = 0 or 1. DX5 rejects 0x200000 as an invalid code (no effect). VERIFIED that SIM passes this; INFERRED that it is a game-side quirk (the argument roles may differ for other plugins).

### Render  0x10001370  `int Render(int mode, void *surfDesc, VERTEX *verts, int nverts, TEX **tex)` cdecl, 5 args (VERIFIED)
arg2 is ignored by DX5 (software plugins use it); DX5 forwards (mode, verts, nverts, tex) to 0x10004080. Returns 0 OK; 1 when the polygon is fully clipped or the texture could not be obtained; BeginScene HRESULT on failure. SIM ignores the result. BeginScene is called lazily on the first Render of a frame (flag 0x10072be0).

Modes 0..18 (jump table 0x10004b64); "key" = colour-keyed texture:
| mode | texture | shade | address | alpha | colourkey |
|---|---|---|---|---|---|
| 0, 1 | yes, key 0 | gouraud | clamp | off | off |
| 2 | key 0 | gouraud | wrap | off | off |
| 3 | key 1 | gouraud | clamp | off | on |
| 4 | key 1 | gouraud | wrap | off | on |
| 5 | key 1 | gouraud | clamp | on | on |
| 6 | key 1 | gouraud | wrap | on | on |
| 7 | key 0 | flat | clamp | off | off |
| 8 | key 1 | flat | clamp | off | on |
| 9 | key 1 | flat | wrap | off | on |
| 10 | key 1 | flat | clamp | on | inherited (SetState 7 not issued) |
| 11 | key 1 | flat | wrap | on | on |
| 12 | none | flat | | off | |
| 13 | none | gouraud | | off | |
| 14 | none | flat | | on | |
| 15 | none | gouraud | | on | |
| 16 | none | LINELIST closed outline, flat | | | |
| 17 | none | LINELIST closed outline, gouraud | | | |
| 18 | none | POINTLIST, one per clipped vertex | | | |
| >18 | no state change, draws a fan from the stale clip buffer 0x10012860 (INFERRED: unintended) |
Modes SIM uses: 1, 2, 6, 0xd, 0xf, 0x10. SHELL: 1, 2, 6, 0xd, 0x10. Because 0/1 and odd/even groupings match the 3DFX byte table (0x10001978: 0,0,0,1,1,2,2,3,4,4,5,5,6,7,8,8,9,9,10), VERIFIED that modes group the same way in 3DFX.

Path: textured: clip via 0x100015f0(clipRect, out=0x10012860, verts, n); texture handle via 0x10003e00(tex, keyflag); bind via 0x10003370 (flushes the batch on change). Polygons are convex fans: (n-2) triangles (v0, vi, vi+1) emitted at 0x100048fb; culling off. Batch buffer 0x10012ab0 (pointer 0x10072bd4, count 0x10072bd0, primitive type 0x10072bcc: 4 tri, 2 line, 1 point). Flushed with DrawPrimitive (vtable +0x74, flags 4 = DONOTCLIP, D3DTLVERTEX) when the mode changes (0x10072940) and at RenderRefresh.

#### Input vertex, 36 bytes (stride 0x24) (VERIFIED)
| off | type | meaning |
|---|---|---|
| +0x00 | float | x, screen pixels |
| +0x04 | float | y, screen pixels |
| +0x08 | float | z, camera depth; D3D sz = z * (1/zmask) |
| +0x0c | float | w = 1/z (SIM computes 1.0/z); D3D rhw |
| +0x10 | int | light intensity I, 0..255 (SIM: 0xff in hi-colour, 0x80 otherwise in some paths) |
| +0x14 | u8 x4 | R, G, B (0x14, 0x15, 0x16), A (0x17) |
| +0x18 | u32 | top byte = per-vertex fog byte, forwarded verbatim as D3D specular (out+0x14) |
| +0x1c | float | u (normalised, passed through unscaled) |
| +0x20 | float | v |
Output D3DTLVERTEX (32 bytes): sx, sy, sz, rhw, color = A<<24 | ((R*I)>>8)<<16 | ((G*I)>>8)<<8 | ((B*I)>>8), specular, tu, tv. The multiply is not clamped. Untextured/line/point paths (0x10003ec0, 0x10003f90) write specular 0xff000000 and no input alpha. Texture blend is D3D default MODULATE (texel x vertex colour).
Clipper (VERIFIED): rect x in [0,W), y in [0,H) (globals 0x10012aa0..aac). u,v are premultiplied by w, x,y,w,u*w,v*w,fog byte and I are interpolated against the planes, then z=1/w and u,v recovered by dividing by w (0x1000169c, 0x10001f10). All outcodes zero -> no clipping; AND of outcodes nonzero -> rejected. The no-uv clipper is 0x10001f60.

Fog: INFERRED that the fog byte uses 255 = no fog, 0 = fully fogged (SIM's fog function 0x10058ac2 returns 1.0 near/0 far, scaled by 255 at 0x10143fcc, which matches D3D specular-alpha fog). 3DFXGEAR forwards 255 - (v+0x18>>24) as Glide alpha, consistent with that. Unresolved: SIM writes 0xff000000 to the first two vertices and 0 to the rest at 0x100a0339 (a special case).
Z: sz = z/65535 requires z < 65535 for in-range depth; INFERRED only. SIM never enables the Z buffer, so depth order comes from painter ordering by the game.

### RenderNoClip  0x10001530  same args as Render, forwards to 0x10004bb0 (VERIFIED)
Same modes and states, but no clipping: vertices are read directly from arg3, count arg4. SIM and SHELL never call it.

### Texture argument TEX** and texture cache (VERIFIED)
`tex` points to a pointer to T (SIM builds T on the stack per call):
| off | meaning |
|---|---|
| +0 | int width |
| +4 | int height |
| +8 | pointer to 8-bit indexed pixels, row-major, width bytes per row |
| +0x0c | pointer to 256 DWORD palette 0x00RRGGBB |
| +0x10 | int colour-key palette index (0xff when keyed) |
Cache key is the pixel pointer T+8 (comparator 0x10003eb0, sorted array 0x10005e80), NOT the address of T. Record, 0x1c bytes: +0 key, +4 w, +8 h, +0xc keyflag, +0x10 IDirectDrawSurface, +0x14 D3DTEXTUREHANDLE, +0x18 last-used frame (compared to counter 0x10072bf8, incremented by RenderRefresh). On a miss it makes a VRAM texture (caps 0x4005000) and a sysmem one (0x1800), loads the data. If creation fails it evicts an LRU record of same size/keyflag and re-uploads (0x10003cf0/0x10003730). VRAM accounting at 0x10072bd8.
Formats: if dev+0x1b14 != 0 (8-bit paletted chosen) the index bytes are copied and the shared DDraw palette attached. Otherwise the palette is applied on the CPU and baked to 16-bit 565/555 with shifts dev+0x1b28/0x1b2c/0x1b30; later palette changes do not alter already-cached 16-bit textures. Colour key (keyflag 1): palette[T+0x10] converted to the surface format (or the index itself in 8-bit mode), SetColorKey(DDCKEY_SRCBLT=8). Texture contents must be treated as immutable while their pixel pointer is cached (use UpdateTexture to invalidate).

### PreloadTexture  0x10001550  `int PreloadTexture(TEX **tex, int keyflag)` (VERIFIED)
Free texture VRAM via 0x10005ac0; computes free - w*h*(1 or 2 bytes). If not greater than the threshold (0x40000 in 8-bit mode, 0x80000 in 16-bit mode) returns 1 without loading. Otherwise returns the result of 0x10003e00 (nonzero texture handle, 0 on failure). SIM sites: 0x10006fee (keyflag from [rec+0x12]), 0x100a2af0, 0x100a2b81 (0x101cfc90/0x101cfcb8, key 0). SHELL: 0x100360bf, 0x1004452e, 0x100445bf. INFERRED: callers do not depend on the return value.

### UpdateTexture  0x100015d0 -> 0x10005bb0  `int UpdateTexture(TEX **tex, int keyflag)` -> 0 (VERIFIED)
Finds the record by pixel pointer, releases/removes it (0x10005b80), re-creates via 0x10003e00. No call sites in SIM or SHELL.

### SetTexturePalette  0x10001510 -> 0x100032c0  `int SetTexturePalette(int unused, const DWORD pal[256])` (VERIFIED)
arg1 unused (SIM passes 0). Returns 0 immediately if dev+0x1b14 == 0. Otherwise builds PALETTEENTRY[256] (R,G,B, flags 4) and `SetEntries(0,0,256)` on the shared palette 0x10072bf4 if 0x10072bc8 and the palette are nonzero. SIM site 0x100d3130 (palette 0x10185760 built 0x100d3000..0x100d30ee: VGA 6-bit x4, index 255 forced 0x808080); SHELL site 0x10055840 (0x102541a0).
INFERRED: a replacement must apply palette changes immediately to 8-bit paletted textures (all textures share one palette). Where textures are baked to RGB at upload, texture creation after SetTexturePalette must use the palette in T+0x0c.

### SetLumaTable  0x10001280  `void SetLumaTable(const BYTE tbl[8192])` (VERIFIED)
A bare `ret` in DX5 (lighting is RGB x I). SIM builds the 32x256 table at 0x10ab1ae0 (0x100065fb) and 0x10185c60 (0x100d1ebc) with rows reversed (dst[(31-i)*256+j]=src[i*256+j]); SHELL calls at 0x100356fb, 0x1004d0ec. Called before FirstDevice. Software/Glide plugins consume it (not analysed in detail).

### RenderRefresh  0x10001360 -> 0x100036a0  `int RenderRefresh(void)` -> 0 (VERIFIED)
In order: frame counter 0x10072bf8++; flush pending batch; reset batch globals and BeginScene flag; EndScene (vtable +0x2c); if dev+0x24 (Z buffer) is set, viewport Clear (+0x30) with flags 2 (ZBUFFER) over rect 0x10072930. No colour clear, no flip, no present. SIM calls it every frame at 0x10086477 after scene render 0x100352d5; SHELL at three sites.

### RestoreDevice  0x100015c0 -> 0x10005b30  `int RestoreDevice(void)` -> 0 (VERIFIED)
Calls Restore on the Z-buffer surface if present, releases and frees every cached texture, resets VRAM accounting 0x10072bd8, empties the cache. Textures are re-uploaded lazily.

### LostDeviceDisplay  0x1000ccf0  `int LostDeviceDisplay(void)` -> 0 (VERIFIED)
`xor eax,eax; ret`. No-op in DX5.

## 3. Lifetime and ordering
- VERIFIED: GetFuncDesc, CheckFunc at load; SetLumaTable before FirstDevice; FirstDevice once after the game has created its DirectDraw object, display mode and surfaces; SetTexturePalette after FirstDevice; PreloadTexture/Render between frames; RenderRefresh once at the end of each frame; LastDevice at shutdown.
- Window loss (VERIFIED): SIM WndProc WM_ACTIVATEAPP (0x1c) near 0x10086859. Activation calls RestoreDevice (d5c at 0x10086921); deactivation calls LostDeviceDisplay (d60 at 0x10086935); both only if [0x1017b934] (set at 0x10085fde after FirstDevice) is nonzero. The game itself restores its DDraw surfaces.
- INFERRED: Render/PreloadTexture before FirstDevice or after LastDevice dereference a null device (no null check seen in the exports) and crash.

## 4. Display ownership (VERIFIED)
The game creates the primary surface [0x1017b394] (caps 0x4200, plus 0x2000 3DDEVICE when [0x1017b940]; one back buffer in flip mode), back buffer [0x1017b398] and render surface [0x1017b39c] (code from 0x1007fa74), and passes the DirectDraw and surface pointers in P+0x20/+0x2c. The game draws its HUD, blits and flips via DDraw. The plugin only draws into the given surface between BeginScene and EndScene.

## 5. Comparison with other plugins
- I76DRAW (VERIFIED): most exports are the stub 0x10006250 (`xor eax,eax; ret`); Render at 0x10006260 software-dispatches modes 0..0x11 and uses arg2 (the surface descriptor).
- RENDDRAW (software, id 2): exports LockDisplay/RefreshDisplay/UnlockDisplay; FirstDevice (0x10002310) reads P+4, +8, +0x3c, +0x40, +0x44, +0x4c; SetState 0x10002750 (codes 2, 4, 6); GetSocketCaps 0x10002740.
- 3DFXGEAR (Glide, id 2): LockDisplay at 0x100027f0 returns pitch through an out pointer; SetState codes 2 and 6; mode grouping per the byte table above; fog byte inverted into Glide alpha.
- Semantics of Render modes, vertex layout, SetState code numbers agree across the plugins (VERIFIED for the mode grouping and code numbers; the per-mode software semantics in I76DRAW/RENDDRAW were not traced in detail: INFERRED equal).

## 6. Open questions
1. Fog byte polarity and the 0xff000000/0 pattern at SIM 0x100a0339.
2. Z range and units (z/65535).
3. SetState(0x200000, x) from SIM is rejected by DX5; the intended behaviour is unknown.
4. SetState code 6 value (0x99) has no reader in DX5.
5. Mode 10 colour-key inheritance; modes >18.
6. Exact error HRESULT paths.
7. Replacement plugin must obtain/simulate the game's DDraw render surface (see section 4).
