# uninittrace, run 1 (2026-10-26, with the converted functions, sdlaudio)

The source is the game folder's debug.log, saved as `C:\Coding\HeavyGear\Project\debug_run1.log`.

The load line was present: "uninittrace 25 of 25 entry watcher(s) and 29 of 29 read watcher(s) installed". The sim
returned normally, so the summary was written.

**Lost data.** The launcher's DebugPrintf keeps only 255 characters per line, so 8 of the summary lines were cut
short. The lost parts are:

- 0x100232d7: both read sites.
- 0x1005ab44: the third site.
- 0x1005bb7a: sites 2 and 3.
- 0x10067ec6: the third site.
- 0x10089066 and 0x1008950e: the compare sites.
- 0x100a65e7: most values.
- 0x100c7120: its read sites.

Build 6 splits the summary into short lines. Run 2 uses it.

"Unset" below means the read saw the same value that was in the stack slot when the function started, i.e. the
local really was never written on that call.

| Function | Calls | Bad read ran (unset) | Values seen | Verdict |
|---|---|---|---|---|
| 0x10020055 sound slot volume | 0 | - | - | never reached |
| 0x10021e8c camera target | 0 | - | - | never reached |
| 0x100232d7 camera tracking | 9973 | lost | entry [ebp-0x2c]: 0 x9959, 7/8/9, 0x40044000, 0x40060000; [ebp-0xdc]: all different (0x46627cxx, float-like) | varied; site data lost |
| 0x100687a3 within-radius test | 0 | - | - | never reached |
| 0x100b66a0 HUD drawing | 9576 | first-pass test 67032 runs, unset 9576 (every call) | always 0x00000005 | **always the same value** (5); the fidiv read is never unset |
| 0x100c2ae5 hit test | 37102 | hit-without-find path: 0 | - | bad path never ran |
| 0x1000aa9e geometry lookup | 0 | - | - | never reached |
| 0x10055048 input device | 9940 | 9940 runs, unset 0 | entry word always 0x80 | bad path never ran (the device routine always writes it) |
| 0x1005ab44 node-name search | 3002 | sites 1-2: 1159 runs, unset 0; site 3 lost | - | not seen unset (site 3 unknown) |
| 0x1005bb7a POF loader | 6 | site 1: 6 runs, unset 0; sites 2-3 lost | - | not seen unset (sites 2-3 unknown) |
| 0x10067ec6 kill credit | 7 | site 2: 4 runs, unset 1 (value 0x40266c85); site 1: 0; site 3 lost | killer = 0x40266c85 (float-like garbage) | **bad path ran once**: garbage killer |
| 0x10089066 node-tree bounds | 563 | fld site: 284 runs, unset 6 | varied (0xc73d2a62, 0xc7a70087, ...) | varied; the cmp site was lost |
| 0x1008950e node-tree bounds 2 | 563 | fld site: 284 runs, unset 5 | varied (0xc73d29b9, ...) | varied; the cmp site was lost |
| 0x100918e6 / 0x1009221d renderer | 0 / 0 | - | - | never reached |
| 0x1009353e renderer | 971 | not watched | - | reached (calls only) |
| 0x100945b8 renderer | 82719 | not watched | - | reached |
| 0x1009596a renderer | 29926 | not watched | - | reached |
| 0x1009fada vertex buffer | 0 | - | - | never reached |
| 0x100a0880 clip vertices | 9940 | not watched | - | reached |
| 0x100ac9ab clip vertices | 8917 | not watched | - | reached |
| 0x100a65e7 terrain? | 36400983 | 11111330 runs, all unset | varied: 0x1014cdc0 x183036 most common (a DLL data address), rest lost | **varied**, runs constantly |
| 0x100b8d6c resource lookup | 0 | - | - | never reached |
| 0x100c0158 selection | 21 | 21 runs, unset 0 | - | bad path never ran |
| 0x100c7120 HUD range readout | 9576 | lost | entry values constant: [ebp-0x28] 0x1014c2e8, [ebp-0x70] 0x1014cfa0, [ebp-0x74] 0x101380b2, [ebp-0x78] 0x004748be, [ebp-0x7c] 0x0019f970 | entry always the same; read data lost |
