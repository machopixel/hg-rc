# HGSIM functions left original because they read uninitialised stack (35)

Read-only survey, 2026-10-26. Sources: `tools/round10/pending_all.txt`, DECISIONS.md and the Ghidra output
`build/ghidra/HGSIM.c`. The size is the distance to the next function. "Does" is inferred from strings, callees and
callers, and is marked "?" where it is a guess. The Effect column has three values:

- **harmless**: only reached with bad or missing data or on an error path, or the unset bytes are probably never used.
- **matters**: the garbage value changes what the game does on a path it can really take.
- **unclear**: needs a closer look.

The "zero?" column says whether "start the local at zero" would be a safe decision:

- **yes**: zero is a sensible stand-in for a value that is really unset.
- **check**: probably yes, but first confirm that the consumer ignores those bytes.
- **no**: zero changes the behaviour, or the intended value is something else.

| Address | Size | Does | Read unset, and on which path | Effect | zero? |
|---|---|---|---|---|---|
| 0x1000aa9e | 866 | ? geometry / hit lookup (calls 1002dda9, 100c2851, 100cbc6b) | [ebp-0x64] and [ebp-4] are passed to 0x100cbc6b; the three differences all go to [ebp-0x60] | unclear | check |
| 0x10019fe1 | 605 | reads `%f %f %f %f` records from a text file | four locals, only when a line has fewer than 4 numbers | harmless | yes |
| 0x1001a23e | 700 | reads `%f %f %d %d` records | locals that sscanf leaves unset, only when a line is short | harmless | yes |
| 0x1001a4fa | 524 | parses `%d ... %d` text separated by `;` | 100-byte buffer [ebp-0x78], when there is no `;` or sscanf stops early | harmless | yes (empty string) |
| 0x10020055 | 72 | zeroes a sound slot's volume when [0x1015d52c] is set | index [ebp-4] is never written on any path, so it writes at a garbage slot | matters | no (the intended index is probably [0x1015d52c]-related) |
| 0x10021e8c | 1851 | ? camera / view target (calls 100246b3) | [ebp-0x8c], [ebp-0x94] and [ebp-8] are never written, then passed as the camera target | matters | no |
| 0x100232d7 | 3655 | ? camera / target tracking (larger sibling of the above) | miss path reads [ebp-0x2c]; 0x1009cf67 gets the never-built [ebp-0xdc] | matters (miss path) | no |
| 0x10055048 | 1658 | input reading (device [0x1015fdf4]) | word [ebp-4], written only by the device's +0x18 routine, is read when the device is not open | matters (probably every run without that device) | check (zero = no input) |
| 0x1005ab44 | 609 | recursive search of node names | [ebp-0x10] is passed to the recursive call when a node has no name at this level | unclear | check |
| 0x1005bb7a | 4560 | POF model loader ("the POF seg %s is missing") | [ebp-0xc0] and [ebp-0xa8] are set in only one switch case; [ebp-0x84] only by the 0x1004d330 cases | matters for models without those chunks | check |
| 0x10064364 | 1216 | HUD heading text with a shape | [ebp-0xc] goes through _ftol into sprintf, only when the first shape fails to load | harmless (error path) | yes |
| 0x100648a0 | 1125 | draws unit markers (palette chosen by kind) | palette index [ebp-0x14], for a kind other than 0..2 | harmless if kinds are always 0..2 | yes |
| 0x10064d05 | 1068 | same, second marker routine | same | harmless if kinds are always 0..2 | yes |
| 0x10067ec6 | 535 | kill handling (passes the killer to 1006c161) | killer [ebp-4], when both units or neither are kind 9 | matters (who is credited) | check (zero = no killer) |
| 0x100687a3 | 256 | "within radius" test against a point | [ebp-0x1c], [ebp-0x20] and [ebp-8] are compared with 0.0 (0x1012b600) before they are written, on every call | matters | no: zero turns the test off; in practice garbage is not 0.0, so "skip the check" matches |
| 0x1007581a | 3349 | draws and wraps text labels | [ebp-0x104] has no NUL after strncpy of a label longer than the width; strchr(' ') then reads on, if the label has no space | harmless with shipped labels | yes |
| 0x10089066 | 703 | recursive node-tree bounds / centre | the leaf path returns [ebp-4] | matters if callers use the result | check |
| 0x1008950e | 697 | same, second variant | same | matters if callers use the result | check |
| 0x100918e6 | 1849 | builds stack vertices for the renderer [0x1014cb4c] | vertex fields +8, +0x14, +0x18 are never set | probably harmless (unused by this draw mode?) | check |
| 0x1009221d | 1849 | same (vertices at ebp-0x90) | same | probably harmless | check |
| 0x1009353e | 1125 | same | +8 and +0x18 | probably harmless | check |
| 0x100945b8 | 1072 | same (four 0x24-byte vertices) | +8, +0x18, and +0x14 of three vertices | probably harmless | check |
| 0x1009596a | 1184 | same (four vertices) | +8, +0x14, +0x18 | probably harmless | check |
| 0x1009fada | 1113 | fills a 0x1200-byte vertex buffer for the renderer [0x109e3d30] | byte +0x17 and others | probably harmless | check |
| 0x100a0063 | 726 | reads skygnd.par (`mission %4s globalintensity %d`) | sscanf locals, only when a line is short | harmless | yes |
| 0x100a0880 | 1391 | builds four 0x58-byte clip vertices (0x100a0512 / 0x100a0339) | +0..+8 and +0x31..+0x57 | probably harmless | check |
| 0x100a65e7 | 2301 | ? terrain / landscape (0x100a5c10 group) | [ebp-0x78] is copied to [ebp-0x2c] at 0x100a695f before it is ever written | unclear | check |
| 0x100ac9ab | 2120 | builds 0x58-byte clip vertices in a 0x57b0-byte frame buffer (0x100ac738) | same fields as 0x100a0880 | probably harmless | check |
| 0x100b66a0 | 816 | HUD drawing with a loaded shape | tests [ebp-0x10] on the first loop pass, and divides by it | matters | no |
| 0x100b8d6c | 452 | resource name lookup (strncat, 0x100ba7d5) | stores word [ebp-0x10] through its first argument on a path that never writes it | unclear | check |
| 0x100b9dde | 205 | reads lang.ini (`language=`) | 256-byte buffer when fgets fails; [ebp-4] when strstr fails | harmless (missing or bad lang.ini only) | yes |
| 0x100c0158 | 545 | selection handling (kinds 0x200 / 0x400) | [ebp-0x14] is passed to 0x100aa863, for any other kind | unclear | check |
| 0x100c2ae5 | 1607 | ? line-of-sight / hit test (32 callers) | [ebp-0xc], when the finders find nothing but 0x100c292c says hit | matters | no |
| 0x100c7120 | 3275 | ? HUD range readout (`%2.2fk`, `%3ldm`) | type 0x100 path never writes [ebp-0x28] or [ebp-0x70..-0x7c]; the 0x400 path likewise when 0x100cba68 returns 0 | unclear | check |
| 0x100e2aa3 | 4353 | WTB model / material loader | [ebp-0x94] has no NUL after strncpy of a 16-character name; strupr and fprintf %s read on | harmless with shipped data (only exactly-16-character names) | yes |

Summary:

- **Zero-init = yes (10):** 0x10019fe1, 0x1001a23e, 0x1001a4fa, 0x10064364, 0x100648a0, 0x10064d05, 0x1007581a, 0x100a0063,
  0x100b9dde, 0x100e2aa3. For 0x100648a0 / 0x10064d05 this assumes unit kinds are always 0..2.
- **Check first (19):** mostly the renderer and clipper vertices, where one look at how the renderer reads those fields
  probably clears all 8 at once.
- **No (6):** 0x10020055, 0x10021e8c, 0x100232d7, 0x100687a3, 0x100b66a0 and 0x100c2ae5. Each needs a decision on the
  value the code meant to use.
