# Heavy Gear: who owns the screen, and how a frame reaches the monitor

Scope: HGSIM.DLL (sim), PLUGIN\DX5DRAW.DLL (Direct3D plugin), with 3DFXGEAR.DLL / RENDDRAW.DLL used for comparison, and HGSHELL.DLL where it matters.
Method: objdump listings (image base 0x10000000 for every module, so VAs are module-relative to that DLL) plus pefile reads of data tables. No dynamic testing.

Legend
- VERIFIED = read directly from instructions or data bytes at the VA cited. COM method names are decoded from vtable offsets against the DirectDraw/Direct3D (DX5/DX6 era, IDirectDraw2 / IDirect3D2 / IDirect3DDevice2) interface layouts; the offsets themselves are VERIFIED.
- INFERRED = a conclusion drawn from several verified facts, or a naming/intent guess.
- "HGSIM x" means a VA inside HGSIM.DLL; "DX5 x" a VA inside DX5DRAW.DLL; "SHELL x" inside HGSHELL.DLL.

## 0. Answer in one paragraph

There are two completely different display contracts between the sim and a 3D plugin, selected by the command line the shell passes to SimMain.
1. `/d3d` (DX5DRAW): the SIM owns the screen. The sim creates the DirectDraw object, the exclusive-fullscreen 640x480x16 flip chain (primary + 1 back buffer, DDSCAPS_3DDEVICE), and does ALL presentation (Lock, CPU 2D overlay, Unlock, Flip). It hands its IDirectDraw pointer and its back-buffer IDirectDrawSurface pointer to the plugin in the FirstDevice argument block. DX5DRAW creates only a Direct3D2 HAL device on that back buffer (no Z-buffer in practice) and rasterises pre-transformed triangles into it. DX5DRAW never flips, blits, locks or clears the render target, and it creates no primary surface. It cannot present.
2. `/3dfx` (3DFXGEAR, RENDDRAW): the PLUGIN owns the screen. The sim creates no DirectDraw/GDI objects at all; it calls the plugin's LockDisplay / UnlockDisplay / RefreshDisplay for its 2D drawing and for the swap.
A Vulkan replacement should implement contract 2 (recommended, section 6), because it removes the sim's DirectDraw entirely and the contract is already proven by two shipping plugins.

---

## 1. HGSIM display code

### 1.1 Structure: two tables of driver objects (VERIFIED)

HGSIM contains a statically linked 2D display library with pluggable "mode" objects. All tables are in .data, read with pefile.

Table A ("window mode"), pointer array at HGSIM 0x1013a298, struct stride 0x28: `{id, kind, windowStyle, fn0..fn5}`.

| A | VA | kind | style | meaning | evidence |
|---|---|---|---|---|---|
| A0 | 0x1014a138 | 1 | 0x80000000 (WS_POPUP) | DirectDraw exclusive fullscreen | fn0 thunk 0x10002239 -> 0x1007fe6f = DDRAW_Init |
| A1 | 0x10149e58 | 1 | WS_POPUP | DISPDIB (DisplayDibWindow), 320x200 only | fn0 -> 0x1007ea30, class "DisplayDibWindow" 0x1014a110, "DISPDIB.DLL" 0x1014a124; size test 0x1007ea36/0x1007ea3f |
| A2 | 0x1014af28 | 2 | 0x00ca0000 (WS_CAPTION\|SYSMENU) | GDI windowed (CreateDIBSection/StretchDIBits) | fn0 -> 0x10083530, strings "GDI ..." 0x1014af78.. |
| A3 | 0x1014b838 | 1 | WS_POPUP | PLUGIN-OWNED display (LFB style) | fn5 -> 0x10087a0c calls plugin LockDisplay |

Table B ("surface/flip driver"), pointer array at HGSIM 0x1013a2a8 (7 entries, stride 0x28): `{id, A-index, flag, 0, fn0 init, fn1 release, fn2 flip, fn3, fn4}`.

| B | VA | A | init (+0x10) | release (+0x14) | flip (+0x18) |
|---|---|---|---|---|---|
| B0 | 0x1014a160 | A0 | 0x1007fe6f | 0x1007f9da | 0x10080471 DDRAW_Flip |
| B1 | 0x1014a188 | A0 | 0x1007fe6f | 0x1007f9da | 0x100805d6 DDRAW_BlitFlip |
| B2 | 0x1014a1b0 | A0 | 0x1007fe6f | 0x1007f9da | 0x10080c57 DDRAW_Blit |
| B3 | 0x1014a1d8 | A0 | 0x1007fe6f | 0x1007f9da | 0x10080c57 DDRAW_Blit |
| B4 | 0x10149e80 | A1 | 0x1007ea30 | 0x1007eda5 | DISPDIB |
| B5 | 0x1014af50 | A2 | 0x10083530 | 0x10083692 | GDI |
| B6 | 0x1014b860 | A3 | 0x100877bc (nop) | 0x100877a5 | 0x100877c3 (UnlockDisplay+RefreshDisplay) |

Current objects: HGSIM 0x1015e688 = A, 0x1015e68c = B. Selection/fallback function: `SetDisplay` 0x1003795b (thunk 0x10001ee2). It picks B from the table (default arg -1 -> B1, 0x10037972..0x10037978), sets the window style/position through 0x100381d0 (SetWindowLongA GWL_STYLE, SetWindowPos), calls B.init `[B+0x10](screenStruct,w,h)` at 0x10037bd2, and on failure logs and falls to the next higher B id until id 6 (loop 0x10037bb5..0x10037c25). All VERIFIED.

The B0..B3 differences are in CreateSurfaces 0x1007fa74 (thunk 0x100030ee), VERIFIED:
- B0/B1: primary with DDSCAPS_FLIP\|COMPLEX (`or 0x18`, 0x1007fc54) and BackBufferCount (flag 0x20, 0x1007fc42); count is **1 if `[0x1017b940]` (3D) else 3** (0x1007fc66 / 0x1007fc72). Back buffer fetched with GetAttachedSurface (+0x30, caps 4) at 0x1007fd4d..0x1007fd70.
- B0: work surface `[0x1017b39c]` = the back buffer itself (0x1007fdb0..0x1007fdb5). B1/B2/B3: work surface = separate offscreen surface, caps 0x40 plus 0x4000 (video) for B2 or 0x800 (system) for B1/B3 (0x1007fdbf..0x1007fe36).
- Primary caps = 0x4200 (PRIMARY\|VIDEOMEMORY) plus **0x2000 DDSCAPS_3DDEVICE when `[0x1017b940]`** (0x1007fc24..0x1007fc36).
- A separate 320x200 ModeX path (0x1007fa7f..0x1007fbe5) with 2 back buffers; not relevant to 3D.

### 1.2 What HGSIM creates (DirectDraw path, B0, the one `/d3d` uses)

All in DDRAW_Init `0x1007fe6f` (thunk 0x10002239) unless noted. VERIFIED.
- IDirectDraw: if `[0x1017b940]` (3D) it first calls DirectDrawEnumerate (thunk 0x1010ce70, callback 0x10080372); the callback does DirectDrawCreate per device, GetCaps (+0x2c) and keeps the first device with DDCAPS_3D (`and eax,1` on dwCaps, 0x10080438..) into `[0x1017b390]`. Otherwise / fallback `DirectDrawCreate(NULL)` (thunk 0x1010ce6a) at 0x1007fef3..0x1007fefc.
- `SetCooperativeLevel(hwnd=[0x1017b8f0], 0x51)` at 0x1007ff31..0x1007ff48 (0x51 = EXCLUSIVE 0x10 \| FULLSCREEN 1 \| ALLOWMODEX 0x40). **Exclusive fullscreen.**
- `SetDisplayMode(w, h, bpp)` (+0x54) at 0x1007ff8b..0x1007ffc6 with **bpp = 16 if `[0x1017b940]`, else 8** (0x1007ff8b push 0x10 / 0x1007ffad push 8). w,h are the SetDisplay arguments, 640x480 for 3D (see 1.4).
- GetCaps (+0x2c) at 0x10080038; DDCAPS_BANKSWITCHED (0x08000000) rejects B0 and B2 ("bank-switched video card", 0x10080045..0x10080062).
- CreateSurfaces (as above) at 0x10080083.
- GetPixelFormat (+0x54) on the primary at 0x100800c7 -> 0x10002342 (-> 0x1003772b) fills the 16-bit format descriptor `0x109e3880` (RGB mask bit counts). 0x100819e2 (thunk 0x1000203b) packs the popcounts as R<<16\|G<<8\|B; 565 = 0x50605. The sim therefore supports 565 and 555 in the DDraw path. INFERRED for the consumer; the producer is VERIFIED.
- GetSurfaceDesc (+0x58) on primary / work / back at 0x100801a3, 0x100801da, 0x1008021a.
- Palette: `CreatePalette(0x44 = DDPCAPS_8BIT\|ALLOW256, entries, &[0x1017b3a8])` (+0x14) at 0x100802ca and `primary->SetPalette` (+0x7c) at 0x10080319. In 16 bpp the SetPalette error is only logged (string 0x1014a42c), not fatal. INFERRED that it fails in 16 bpp; the non-fatal handling is VERIFIED.
- On 320x200 the stretch hook `[B... +0x20] = 0x1000219e` is installed (0x10080356). Not relevant.

Display state is exposed to the rest of the sim as a small struct `0x109e3e00`/`0x1015e698` = `{surface pointer, pitch-1, ...}`.

### 1.3 When each function runs

| Phase | Function (VA) | Role (VERIFIED unless marked) |
|---|---|---|
| Boot | SimMain 0x10084feb (export `SimMain` 0x10002856); hwnd = 6th arg `[ebp+0x1c]` stored to `[0x1017b8f0]` at 0x1008504f; hInstance `[ebp+8]` to `[0x1017b8f4]` at 0x10085057 | Window is supplied by the caller; HGSIM has no RegisterClassA (import table has no RegisterClass). |
| Boot | cmdline parser 0x1002ad50 | `-d3d` / `/d3d` sets `[0x1017b940]` (0x1002adde); `/3dfx` sets `[0x1017b944]` (0x1002ae02). Strings 0x10135518 "d3d", 0x1013551c "3dfx". |
| Boot | 0x1008538c..0x100853d2 | Loads the RASTER plugin: class 1 if d3d, 2 if 3dfx, 0 otherwise (pushes at 0x10085395 / 0x100853b1 / 0x100853c4, call 0x100031b1). |
| Boot | 0x1008543f..0x10085458 | If 3D requested but dllfile.ini parsed record `[+0x44] != 1` then `[0x1017b940]=0` (3D off). |
| Boot | 0x100854ae | Placeholder `SetDisplay(5 = GDI driver, ..., w=-1, h=-1)`; GetSystemMetrics full-screen size via 0x100379c8..0x100379f4 (INFERRED: pre-game GDI surface). |
| Boot | 0x10085cc6..0x10085d18 | Final mode set. If `[0x1017b944]` (/3dfx): `[0x1017b940]=1`, driver index `[0x1013a00c]=6`, `[0x1015e688]=A3`. Else if `[0x1017b940]` (/d3d): driver 0 (0x10085cb2..0x10085cbc). Else default (data value -1 -> B1). Then `SetDisplay(driver, flag, 0x109e3e20, [0x1017b918], [0x1017b91c], 0)`. |
| Boot | 0x10085d35..0x10085e98 | Only if 3D **and 640x480**: builds the FirstDevice block and calls plugin `FirstDevice` (`[0x109e3d28]`, 0x10085e8f). Return value is stored to `[ebp-0xe4]` and **never read** (full scan of SimMain finds no other use). Then `PreloadTexture` loops 0x100a2a66 / 0x10006e52. |
| Per frame | LockBuffer = A.fn5 `[A+0x20]`; A0: 0x10082790 (thunk 0x10002b..) | Lock (+0x64, DDLOCK_WAIT) of work surface `[0x1017b39c]`; writes `*0x1015e698 = lpSurface`, `[+4] = pitch-1` (0x10082858..0x100828ac). Restore on DDERR_SURFACELOST (0x887601c2) first (0x100827a1..0x10082802). |
| Per frame | flush 0x10035478 (thunk 0x1000370b) | `[0x109e3dac]==0` and active (`[0x1017b920]`): `B.flip` (`[B+0x18]`, 0x100354f8). Else blit-region `[B+0x20]` (0x100354bb) and reset screen struct. |
| Per frame | B0 flip 0x10080471 | Unlock if locked (`[0x1017b3ac]`), IsLost/Restore, then `primary->Flip(NULL, DDFLIP_WAIT=1)` (+0x2c, 0x1008058c..0x1008059e). |
| Palette events | 0x10081a7a (WritePaletteEntries), 0x10081e1d (gamma), 0x100821b1 (fade) | With `[0x1017b940]` builds RGB565 `pal16[]` through 0x100036a2 -> 0x100f01cc (table at 0x10156dcc); formula `(r>>1)<<11 \| g<<5 \| (b>>1)` on 6-bit VGA components at 0x10081ac0..0x10081aff. Also SetEntries on the DD palette. |
| Activation | `SimWindowProc` export 0x10001622 -> 0x100867a2 | WM_ACTIVATEAPP (0x1c, 0x10086859): `[0x1017b920]=[0x1017b928]=wParam` (0x1008686a..0x10086873). Activate: ShowWindow, **plugin RestoreDevice `[0x109e3d5c]`** (0x10086921), palette rewrite via `[A+0x14]`. Deactivate: **plugin LostDeviceDisplay `[0x109e3d60]`** (0x10086935), ShowWindow(6). |
| Shutdown | 0x10035500 (end-of-session) | Plugin `LastDevice` `[0x109e3d2c]` (0x10035515); non-3D final fill+flip (0x10035536..0x10035556); then `ShutdownDisplay` 0x10037c6c (thunk 0x10002388, call 0x10035582) which calls `B.release` (0x10037c7d) and `A.shutdown` (0x10037c8f). |
| Shutdown | A0.fn1 0x1007f950 | `ReleaseSurfaces` (0x1007f9da), `SetCooperativeLevel(hwnd, DDSCL_NORMAL=8)` (+0x50), `RestoreDisplayMode` (+0x4c), release palette, `Release` DD (0x1007f966..0x1007f9d5). |

### 1.4 Resolution and bit depth in 3D (VERIFIED)
- 3D is initialised only when the sim is 640x480 and `[0x1017b940]` is set (0x10085d35..0x10085d5c). The 320x200 and 1024x768 variants (set by 0x10086da5 / 0x10086dd0 / 0x10086dfb) never call FirstDevice.
- 3D display mode = 640x480x16 (see 1.2). The sim's 2D library has parallel 8-bit and 16-bit routines selected by `[0x1017b940]`, e.g. HUD overlay 0x1007e0ff picks 0x10003788 (-> 0x100f0205, 16 bit) vs 0x10001e3d (-> 0x100e91dc, 8 bit) at 0x1007e19f..0x1007e1d4.
  Consequence (INFERRED but strongly supported): in `/d3d` mode there is no 8-bit screen at all. There is no offscreen-16-bit-plus-colour-key composition; everything lives in one 16-bit surface.

---

## 2. DX5DRAW.DLL

Exports (VERIFIED, pefile): CheckFunc 0x10001060, FirstDevice 0x10001290, GetFuncDesc 0x10001000, GetNumDevice 0x10001390, GetSocketCaps 0x100014e0, LastDevice 0x10001330, LostDeviceDisplay 0x1000ccf0, PreloadTexture 0x10001550, Render 0x10001370, RenderNoClip 0x10001530, RenderRefresh 0x10001360, RestoreDevice 0x100015c0, SetLumaTable 0x10001280, SetState 0x100014f0, SetTexturePalette 0x10001510, UpdateTexture 0x100015d0.
Imports: DDRAW.dll (DirectDrawCreate, DirectDrawEnumerateA) + KERNEL32 only. **No USER32/GDI32, so it cannot touch the window.** There is **no LockDisplay / UnlockDisplay / RefreshDisplay export**, so the sim's `[0x109e3d48/4c/50]` are NULL for this plugin and the A3/B6 driver must never be selected with it.

GetFuncDesc 0x10001000 writes name "Direct3D" (0x1000f030), class "RASTER" (0x1000f03c) and out-param **1** (0x10001058). 3DFXGEAR writes 2 (3DFXGEAR 0x100011ef), RENDDRAW writes 2 (RENDDRAW 0x1000229a). The sim stores the type at `[0x109e3d20]` (0x1003665e) but only compares it inside a check that returns 1 on every path (0x10036850..0x10036870), so the type does not influence the driver choice.

### 2.1 FirstDevice -> init, step by step (VERIFIED)

`FirstDevice(S)` 0x10001290:
1. `S+0x20` (IDirectDraw*) must be non-NULL, else return 1 (0x100012aa..0x100012c5).
2. `operator new(0x1b38)`; ctor 0x10002830 (zeroes fields; Z-state defaults); global device pointer `[0x10072be4]` (0x100012ee).
3. `init(pDD)` 0x100028a0: stores `dev+0xc = pDD` (the **sim's** DirectDraw, no AddRef, 0x100028bf); QueryInterface IID_IDirectDraw2 (b3a6f3e0-..., 0x1000e038) then `GetAvailableVidMem` (+0x5c) to set `[0x10072bec]` = "video memory > 8 MB" (0x100028e2..0x10002914); QueryInterface IID_IDirect3D2 (6aae1ec1-..., 0x1000e0b8) into `dev+0x10`; `EnumDevices` (+0xc) with callback 0x10002980 (collects up to 6 devices, records HAL/HEL desc), then copies the **last enumerated** device description into `dev+0x7e4` (0x10002950..0x1000296c).
4. On init success (eax==0) `0x10002c40(S)` = SetupDevice runs (FirstDevice 0x10001305..0x10001311). That is where the real objects are built:
   - `dev+0x20 = S+0x2c` = the sim's **back-buffer IDirectDrawSurface** (render target), 0x10002c51..0x10002c55.
   - width `S+0x3c`, height `S+0x40`, Z scale from `S+0x44` (0xffff), 0x10002c58..0x10002c7f.
   - Optional 16-bit Z-buffer: `CreateSurface` (+0x18) desc flags 0x47, caps 0x24000 (ZBUFFER\|VIDEOMEMORY), ZBufferBitDepth 16, then `AddAttachedSurface` (+0xc) onto the render target (0x10002d31..0x10002d41). **Guarded by `test BYTE [S+8],1` (0x10002cda); the sim never sets bit 0** (S+8 is built from 0x80, \|0x200, \|0x2 at HGSIM 0x10085e1e / 0x10085e34 / 0x10085e46). Therefore **no depth buffer is created** and `dev+0x24` (the "Z enabled" flag, set only at 0x10002d4e) stays 0. INFERRED consequence: HG sorts polygons itself (painter's algorithm) and sends already-transformed vertices.
   - `IDirect3D2::CreateDevice(IID_IDirect3DHALDevice 84e63de0-..., 0x1000e0e8, pRenderTarget, &dev+0x14)` (+0x20) at 0x10002d69..0x10002d6f. **HAL only; the HEL/RGB/MMX devices are not requested.**
   - `IDirect3DDevice2::EnumTextureFormats` (+0x24, callback 0x10003030, 0x10002d92) fills a table of up to 32 formats at `dev+0x928` (stride 0x8c) and picks an index: DDPF_PALETTEINDEXED8 (flag 0x20) -> kind 8, PALETTEINDEXED4 (0x8) -> kind 4, otherwise RGB with per-channel bit counts (0x10003030..0x1000329e).
   - Viewport: `CreateViewport` (+0x18), `AddViewport` (+0x18), `SetViewport2` (+0x44, D3DVIEWPORT2 {0x2c, 0, 0, w, h, -1.0, ..., 2.0, ..., 0.0, 1.0}), `SetCurrentViewport` (+0x34) at 0x10002ea1..0x10002f4c.
   - Fog/dither/bilinear flags from `S+8` (0x2 fog, 0x200 bilinear -> `SetState(4, 0x20000)` at 0x10003004..0x10003015) and fog parameters from `S+0x10..0x1c`. Initial render states 0x10002b10 (SetRenderState +0x5c, SetLightState +0x64).

The FirstDevice block `S` as the sim builds it (HGSIM 0x10085d62..0x10085e88, `S = ebp-0x88`), VERIFIED:

| S offset | value | DX5DRAW use |
|---|---|---|
| +0x00 | &0x109e3dc0 (screen info {1, w+1, h+1,...}) | unused |
| +0x04 | 8 (art depth) | texture-format preference (0x10002ddf) |
| +0x08 | flags: 0x80; \|0x200 bilinear if `[[0x1015d1b8]+0x3c]`; \|0x2 fog if `[0x1017d108]` | bit7 -> `[0x10072bc8]` (palette support); bit0 never set -> no Z-buffer |
| +0x10..+0x1c | fog mode 3, colour, 1.0f, 500000.0f | fog states |
| **+0x20** | **IDirectDraw\* `[0x1017b390]`** | stored as `dev+0xc` |
| +0x28 | 0x200 | texture cache size argument (0x10002e2f -> 0x10005c60) |
| **+0x2c** | **IDirectDrawSurface\* `[0x1017b39c]` (back buffer in B0)** | `dev+0x20`, CreateDevice target |
| +0x3c/+0x40/+0x44 | 640 / 480 / 0xffff | viewport, z scale |
| +0x4c | hwnd `[0x1017b8f0]` | **unused by DX5DRAW** (no user32) |

### 2.2 Does it use its own DirectDraw object, does it present?

- Own DirectDraw object: **no for rendering.** DirectDrawCreate (IAT 0x10075134, thunk 0x10005f30) is called only from the enumeration helpers CheckFunc 0x10001060 / GetNumDevice 0x10001390 / probe at 0x100011ac, 0x100011eb, 0x100013d5, each of which Releases the temporary object. All render/texture work goes through `dev+0xc` = the sim's pDD. VERIFIED (all six call sites listed by grep).
- Does it present or touch the target? **No.** Surface-method scan of the whole .text (vtable offsets used and the functions using them): no Flip (+0x2c on a surface), no Blt/BltFast on the render target, no SetCooperativeLevel (+0x50), no SetDisplayMode (+0x54), no GetDC, no Lock on the target. Lock (+0x64) / Unlock (+0x80) / SetPalette (+0x7c) occur only in texture upload functions 0x10003951 / 0x10003985 / 0x100037fd / 0x100054a1. The only calls on the D3D device after setup are BeginScene (+0x28), DrawPrimitive (+0x74), EndScene (+0x2c), SetRenderState (+0x5c), SetLightState (+0x64) and viewport Clear (+0x30).
- Render path: `Render` 0x10004080 lazily calls `BeginScene` the first time per frame (flag `[0x10072be0]`, 0x10004080..0x100040a4), accumulates triangles in a vertex batch at 0x10012ab0 and flushes with `DrawPrimitive(primType, D3DVT_TLVERTEX=3, buf, count, D3DDP_DONOTCLIP=4)` (0x100040c6..0x100040dc; same sequence at 0x100036ba..0x100036d1). **Vertices are pre-transformed (TLVERTEX).** The geometry/clipping pipeline is the sim's.
- Texture formats: prefers 8-bit palettised (DDPF_PALETTEINDEXED8) when the HAL lists it; otherwise 16-bit RGB formats found by the callback. Textures are created via the sim's pDD: system-memory master (`flags 0x1007`, caps 0x1800 = TEXTURE\|SYSTEMMEMORY, 0x10003730..0x100037db) and video copy (caps 0x04005000 = TEXTURE\|VIDEOMEMORY\|ALLOCONLOAD, 0x10005380..0x1000541c). A shared 256-entry IDirectDrawPalette `[0x10072bf4]` is created (`CreatePalette` 0x44, 0x10003c81) and updated by `SetTexturePalette` 0x100032c0 (`SetEntries` +0x18 at 0x10003348). The exact rule that chooses 8-bit vs 16-bit texture format when VRAM <= 8 MB (0x10002dba..0x10002e22) is INFERRED (see unknowns).

### 2.3 RenderRefresh, LostDeviceDisplay, RestoreDevice, LastDevice

- `RenderRefresh` 0x10001360 -> 0x100036a0 (VERIFIED): `frame counter [0x10072bf8]++`; flush any pending batch (DrawPrimitive); reset batch state; `device->EndScene()` (+0x2c, 0x10003702); if Z-buffer enabled (`dev+0x24`, i.e. never in practice) `viewport->Clear(1, &rect, D3DCLEAR_ZBUFFER=2)` (+0x30, 0x10003722). It does **not** flip, blit, or clear colour.
- `LostDeviceDisplay` 0x1000ccf0: `xor eax,eax; ret`. No-op. VERIFIED.
- `RestoreDevice` 0x100015c0 -> 0x10005b30: if Z-buffer enabled, `Restore()` it (+0x6c); walk the texture cache `[0x10072be8]` releasing every texture (0x10005b80 -> 0x10006780), reset `[0x10072bd8]`, free the list (0x10005e10). It does **not** restore the render target (sim restores its surfaces itself in Flip/LockBuffer, 0x10080504..0x1008054f, 0x100827a1..0x10082802) and does not re-upload textures (they are re-created lazily on the next Render). INFERRED for the lazy re-creation; release behaviour VERIFIED.
- `LastDevice` 0x10001330 -> 0x10005c00: release palette `[0x10072bf4]`, texture cache, D3D device (`dev+0x14`), IDirect3D2 (`dev+0x10`), delete device. Does not release the sim's pDD or surface.

---

## 3. Per-frame sequence today, and the 3DFXGEAR comparison

### 3.1 `/d3d` (DX5DRAW) main loop (HGSIM SimMain loop head 0x1008605f, loop back 0x100864e6)

Order inside one iteration (all addresses VERIFIED as program order; the semantic labels are INFERRED where marked):

1. Game update/input/audio calls 0x1008611c..0x1008618c (INFERRED: simulation tick).
2. 0x10086278 `0x10002dd3`; then because `[0x1017b940]` is set the non-3D lock at 0x1008628f is skipped (0x1008627d jne 0x100862b7).
3. 0x100862c1 hook `[0x1014cb3c]`; **LockBuffer** `[A+0x20]` (0x100862e8) = DDRAW_LockBuffer 0x10082790: `Lock` the back buffer, expose the 16-bit pointer. On failure the iteration jumps straight to step 5 (0x10086304 jne 0x10086463).
4. **2D layer drawn by the CPU into the locked back buffer** (0x1008630a..0x100863fa): thunks 0x10002662 (-> 0x10089bad), 0x10001cbc (-> 0x1002a0ca), 0x10001a28 (-> 0x1007e0ff HUD/overlay with 16-bit branch), 0x1000354e (-> 0x10075585); then 16-bit 1-pixel lines on the bottom row and right column (0x1008635f call 0x10001d70 (-> 0x100ef727) with (0,479)-(639,479) and 0x100863b9 with (639,0)-(639,479); colour word `[0x10165bc0]` never written by a direct store, so 0 unless set through indexed access); optional debug text 0x100032d8.
5. **Present: flush 0x1000370b (-> 0x10035478)** at 0x10086463. With `[0x109e3dac]==0` and the app active it calls `B.flip` = DDRAW_Flip 0x10080471: Unlock the locked surface, then `Flip(DDFLIP_WAIT)`. This is the only present in a normal frame. (Other `[B+0x18]` call sites exist at 0x10035556, 0x10035776, 0x1003d45a/d503/d5e2/d786, 0x10086a21 and others; they are session-end, screenshot, fade/cutscene and paint paths and were not traced.)
6. **3D scene**: 0x10002b94 (-> 0x100352d5), which begins with `cmp [0x1017b940],0` (0x100352de) and runs the world render (0x100d2306, 0x100d255e, 0x100d1f90) and the plugin `Render` call sites (`[0x109e3d30]`, 22 call sites in 10 functions, e.g. 0x10006609, 0x10006744, 0x100068d9, 0x1009ef50..0x100ad1f3). A black full-screen quad is drawn through `[0x1014cb4c]` for some modes (0x1003534a..0x10035400). Rendering goes into the **new** back buffer, i.e. the one that was just un-flipped.
7. **RenderRefresh** `[0x109e3d34]` at 0x10086477 (EndScene). Immediately after, 0x100012cb, 0x100038b4, 0x100022c0, 0x1000128a (post-frame bookkeeping, not traced), then loop.

Consequences (INFERRED from the order above, but the order itself is VERIFIED):
- The final image shown at the flip in iteration N is: 3D scene rendered in iteration N-1 (steps 6..7), with the 2D layer of iteration N drawn over it by the CPU (steps 3..4). The 2D layer is therefore always composited **on top of** 3D by overwriting pixels in the same 16-bit surface; there is no colour key, no offscreen 16-bit surface and no 8-bit screen in `/d3d` mode. 3D is one frame older than 2D.
- DX5DRAW never clears colour, so the 3D pass starts on whatever the back buffer already held. Whether the sim fully repaints 640x480 every frame was not proven (unknown 4).
- Palette: the sim's 8-bit art is converted to RGB565 via `pal16[]` (0x10156dcc) and 16-bit blitters; DX5DRAW's 8-bit textures use its own DirectDraw palette from `SetTexturePalette` (single call site HGSIM 0x100d3000).

### 3.2 Non-3D (software) frame, for contrast (VERIFIED order)
`[A+0x20]` lock at 0x10086295 -> 8-bit draw -> 0x1000370b flush: B1 `BlitFlip` (0x100805d6: `BltFast` work -> back, then `Flip`) -> no plugin calls on this path. Default driver is B1 (SetDisplay arg -1), 640x480x8 or 320x200.

### 3.3 `/3dfx` (3DFXGEAR) and how the sim uses LockDisplay / UnlockDisplay / RefreshDisplay

Plugin functions bound in the sim: `[0x109e3d48]` Lock, `[0x109e3d4c]` Unlock, `[0x109e3d50]` Refresh (bind code HGSIM 0x1003671c..0x1003674a; others 0x109e3d24..0x109e3d64 bound in 0x10036664..0x100367bd).
The sim touches them only inside the plugin-owned driver (A3/B6) (list of call sites by function: Lock at 0x10087820 and 0x10087a0c; Unlock at 0x100877c3 and 0x10087820; Refresh at 0x100877c3, 0x100877df..0x10087813):
- `A3.fn5` 0x10087a0c (the "LockBuffer" slot `[A+0x20]`): `[0x1017b984]=1`, `LockDisplay(&pitch)` -> `*[0x1015e698] = pointer`, `[+4] = pitch-1` (0x10087a10..0x10087a3a). Executes at the same loop position as DDRAW_LockBuffer (step 3).
- `B6.flip` 0x100877c3: `if ([0x1017b984]) UnlockDisplay(); RefreshDisplay();` -> same loop position as step 5.
- 0x10087820: a 16-bit rectangle fill implemented as Lock/Unlock around direct stores.
- Palette slots 0x100878bb / 0x1008792c / 0x1008799c build RGB565 with the same formula and call 0x100036a2; pixel-format query 0x10087a43 returns the constant 0x50605 (565). The 565 layout is therefore hard-wired in this driver.
- The sim never calls DirectDraw/GDI in this driver (no DDraw call appears in 0x100877a0..0x10087a43) and, when `/3dfx` is set, skips the black DDRAW_Fill pair at 0x10086017/0x10086045.

3DFXGEAR implementation (3DFXGEAR.DLL, VERIFIED):
- `LockDisplay` 0x100027f0: `grLfbLock(type=1, buffer=1, writeMode=0, origin=0, pipeline=0, &info)` (thunk 0x10004386 -> `_grLfbLock@24`, IAT 0x101812b0); returns `info.lfbPtr`, stores `strideInBytes` to `*pitch`. (type 1 = GR_LFB_WRITE_ONLY, buffer 1 = back buffer, 565 write mode; the constants are the Glide 2 values, INFERRED from the argument shape, the call target is VERIFIED.)
- `UnlockDisplay` 0x10002830: `grLfbUnlock(1,1)` (0x1000438c).
- `RefreshDisplay` 0x10002840: `grBufferSwap(1)` (0x10004392) then `grBufferClear(0, 0, 0xffff)` (0x100042d8): swap **and clear the next buffer to black**.
- `FirstDevice` 0x10001230 opens its own Glide window with hWnd = 0 (`grSstWinOpen`, 0x10001321: first pushed argument is EDI = 0, res 7 = 640x480) so it ignores the sim's HWND entirely.
- `RENDDRAW` (Rendition) has the same trio (0x10002910 / 0x10002950 / 0x10002970, via function pointers `ds:0x10014c18/1c`). The shell has its own copy of the A3/B6 driver (SHELL 0x1017afa3 etc., plugin globals 0x102723a8..0x102723e4).

Key observation (INFERRED, important): because 3dfx LFB is locked write-only and the shipping game works in `/3dfx` mode, the 16-bit 2D routines used there do not need to read the destination. The 16-bit routine set is shared with `/d3d` (both set `[0x1017b940]`, 0x10085ccf), so the same assumption very likely holds in `/d3d`. Not proven by tracing the blitters (unknown 1).

### 3.4 How the shell decides `/d3d` vs `/3dfx` (HGSHELL 0x1012c3b0..0x1012c5dc)
It writes the chosen plugin name into dllfile.ini (fprintf "%s\n", e.g. SHELL 0x1012c427..0x1012c458) and appends " /high /d3d" (string 0x10233f3c, push at 0x1012c4f3) or " /high /3dfx" (0x10233f5c, push at 0x1012c598) to the sim command line. `/d3d` is used for selection value -3 with the table entry whose `+0x20 == 1` (the type returned by GetFuncDesc: DX5DRAW = 1); `/3dfx` is used for every other 3D selection (table index >= 0, entry name written). INFERRED mapping of the selection values; the strings and call order are VERIFIED. Practical meaning: any non-DX5DRAW 3D plugin is launched with `/3dfx`.

---

## 4. Summary table: who does what today

| Concern | `/d3d` + DX5DRAW | `/3dfx` + 3DFXGEAR |
|---|---|---|
| DirectDraw object | sim (HGSIM 0x1007fef3 / 0x10080372) | none |
| Display mode | sim: 640x480x16, exclusive (0x1007ff48..0x1007ffc6) | plugin (Voodoo, own window) |
| Front/back buffers | sim: primary + 1 back, 3DDEVICE caps | plugin (Glide) |
| 3D device | plugin: IDirect3DDevice2 HAL on sim's back buffer | plugin (Glide) |
| Z-buffer | none (flag bit 0 never set) | n/a |
| 2D overlay | sim CPU, Lock back buffer (0x10082790), 16-bit | sim CPU, LockDisplay LFB, 565 |
| Present | sim: Unlock + `Flip(WAIT)` (0x10080471) | plugin: RefreshDisplay = swap + clear |
| Colour key / extra surfaces | none | none |
| Window | caller's HWND, SetCooperativeLevel fullscreen | caller's HWND ignored |

---

## 5. What a replacement must preserve (hard contracts)

1. All plugin exports listed in section 2 with identical calling conventions. FirstDevice/LastDevice/Render/RenderRefresh/RestoreDevice/LostDeviceDisplay are called from fixed places (sections 1.3, 3.1).
2. The 640x480x16 / RGB565 2D buffer semantics: the sim writes 16-bit pixels with pitch from the lock, treating `*[0x1015e698]` as base and `[+4]` as pitch-1.
3. 3D draws come between the 2D lock/flip pair (step 6) and RenderRefresh (step 7); 2D for the **next** iteration is composited on the finished 3D image.
4. TLVERTEX input only (screen-space), painter's-algorithm order, no depth buffer, 8-bit paletted textures (texture palette via SetTexturePalette).
5. Resolution: plugin is initialised only at 640x480.

---

## 6. Options for taking over presentation, and recommendation

### Option A: keep the sim's DirectDraw (`/d3d`), Vulkan only rasterises (readback)
- Plugin keeps DX5DRAW's exports and contract. It renders to an offscreen VkImage; at RenderRefresh (EndScene analogue) it submits, waits, and copies the result into the sim's back buffer through `S+0x2c` (`IDirectDrawSurface::Lock`, convert to RGB565) before the next iteration's LockBuffer. The sim keeps presenting through DirectDraw.
- Pros: zero changes to the sim; every 2D effect (including any read-modify-write) keeps working because the 3D pixels are really in the surface the sim locks. Good bring-up path.
- Cons: still needs working 16-bit exclusive DirectDraw on modern Windows (compat layer), per-frame GPU->CPU readback (600 KB, plus sync), DirectDraw not Vulkan on the final path, no scaling/HD output, no vsync/swap control.
- Not recommended as the end state.

### Option B (recommended): plugin-owned display, the 3DFXGEAR/RENDDRAW contract (`/3dfx`)
- New plugin exports `LockDisplay(int *pitch) -> void*`, `UnlockDisplay()`, `RefreshDisplay()` in addition to the standard set, reports GetFuncDesc type 2, and is launched with `/3dfx` (the shell already does that for every non-type-1 plugin, section 3.4).
- The sim then creates no DirectDraw/GDI objects (A3/B6 driver, section 3.3), so Vulkan owns the HWND (`VkSurfaceKHR` + swapchain). FirstDevice's `S+0x20`/`S+0x2c` are NULL and should be ignored; `S+0x4c` is the HWND (verify it is non-NULL: the sim passes `[0x1017b8f0]`).
- Frame contract to implement:
  - 3D: Render calls draw into an offscreen 640x480 (or higher) colour target. `RenderRefresh` ends the pass.
  - `LockDisplay`: return a 640x480 RGB565 CPU buffer, pitch 1280, that **already holds the finished 3D image** (async copy started in RenderRefresh, completed here) so 2D draws on top exactly as with the 3dfx LFB or DirectDraw. Lowest risk.
  - `UnlockDisplay`: upload the CPU buffer to a texture.
  - `RefreshDisplay`: present (acquire/blit-or-draw/present), then **clear the next 3D target to black** (what 3DFXGEAR does with `grBufferClear`).
- Cheaper variant (B2, adopt only once unknown 1 is settled): `LockDisplay` hands out a CPU buffer pre-filled with a sentinel colour (e.g. 0xF81F), and `RefreshDisplay` composites it over the GPU 3D image with sentinel discard. This removes the GPU->CPU readback and lets the 3D pass run at a higher resolution while HUD stays crisp. It is only correct if no 2D routine reads the destination or blends with it and no legitimate 2D pixel equals the sentinel.
- Pros: no DirectDraw dependency at all, no mode switching, no exclusive fullscreen, no 16-bit display mode, full control of scaling/vsync/windowed, proven contract (two shipping plugins and a second consumer, the shell). Palette and pixel format (565) are hard-wired in the driver (0x10087a43).
- Cons: depends on the `/3dfx` flag (also sets `[0x1017b944]`, which changes asset loading at HGSIM 0x100a0208; and the dllfile.ini name field equal to "3dfx" sets `[0x1017d0ac]` (HGSIM 0x10085488) which changes paths at 0x100a04c0 / 0x100a25ec; effect not decoded). Avoid naming the plugin "3dfx" until decoded.

### Option C: ddraw.dll shim (intercept DirectDrawCreate/Enumerate) with Vulkan behind fake surfaces
- Gives the sim (and the shell, which has its own copy of the DDraw/DISPDIB/GDI layer, SHELL 0x10236cc4..) CPU-backed IDirectDrawSurface objects; `Flip` becomes a Vulkan present; the plugin sees the fake pDD/surface and renders to the same backing store.
- Pros: handles the shell's 8-bit DirectDraw menus and the sim without any binary change. Cons: must implement and test the full set of DirectDraw methods HG uses (CreateSurface, GetAttachedSurface, Lock/Unlock, Flip, BltFast, Blt, SetPalette/SetEntries, GetPixelFormat, GetCaps, Restore, IsLost, SetCooperativeLevel, SetDisplayMode, RestoreDisplayMode, EnumerateA ...) plus the lazy D3D2 path if kept. More work, more risk, no better than B for the sim.

### Recommendation
Use **Option B for the sim** (implement LockDisplay/UnlockDisplay/RefreshDisplay, type 2, launched `/3dfx`), build it first as the "LockDisplay returns 3D+copy" version, and use **Option A only as a quick first-triangle bring-up** if wanted. Keep **Option C in reserve for the shell**: the shell's own 8-bit menu display uses DDraw/GDI with the same layer, and DISPDIB.DLL does not exist on modern Windows (the GDI path B5/A2 is the DISPDIB-free fallback).

---

## 7. Open items (ranked)

1. **Does any 16-bit 2D routine read or blend with the destination?** (decides sentinel overlay vs readback; 3dfx write-only lock suggests no). Needs a trace of the 16-bit blitters (0x100e9xxx..0x100f2xxx, e.g. 0x100f0205, 0x100f2f01, 0x100f1e10, 0x100ef727).
2. **What `/3dfx` and a dllfile.ini name of "3dfx" change besides display selection** (`[0x1017b944]` at HGSIM 0x100a0208, `[0x1017d0ac]` at 0x100a04c0 / 0x100a25ec) and the exact shell selection-value mapping (SHELL 0x1012c3c1..0x1012c5dc).
3. **Shell/sim hand-off and window geometry.** The shell has its own copy of the display layer and plugin globals (SHELL 0x102723a8..); how it releases DirectDraw / the display mode before SimMain's DDRAW_Init, and how window size/style is chosen for desktops larger than 640x480 (HGSIM 0x1003795b..0x10037ac0, 0x100381d0), were not traced.
4. Whether the 3D pass fully repaints 640x480 every frame (DX5DRAW never clears colour; 3DFXGEAR clears to black after the swap, so a black clear at RefreshDisplay is the safe match).
5. Other `[B+0x18]` flip call sites (fades, cutscenes, menus) and the post-frame thunks 0x100012cb / 0x100038b4 / 0x100022c0 / 0x1000128a were not traced.
6. DX5DRAW's texture-format selection rule for VRAM <= 8 MB (0x10002dba..0x10002e22) is only partly decoded.

## 8. Reference: plugin function slots in HGSIM (VERIFIED, bind code 0x10036664..0x100367bd)

| Slot | Export | Slot | Export |
|---|---|---|---|
| 0x109e3d24 | SetLumaTable | 0x109e3d48 | LockDisplay |
| 0x109e3d28 | FirstDevice | 0x109e3d4c | UnlockDisplay |
| 0x109e3d2c | LastDevice | 0x109e3d50 | RefreshDisplay |
| 0x109e3d30 | Render | 0x109e3d54 | PreloadTexture |
| 0x109e3d34 | RenderRefresh | 0x109e3d58 | RenderNoClip |
| 0x109e3d3c | GetSocketCaps | 0x109e3d5c | RestoreDevice |
| 0x109e3d40 | SetState | 0x109e3d60 | LostDeviceDisplay |
| 0x109e3d44 | SetTexturePalette | 0x109e3d64 | UpdateTexture |
