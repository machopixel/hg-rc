# uninittrace, run 2 (2026-10-26, noconv = the original code everywhere, sdlaudio)

## Recommendation per function (from runs 1 and 2)

"Zero OK" rests on two play sessions. A situation neither session reached could still take the bad path.

The 6 that statically change behaviour:

| Function | Recommendation | Evidence |
|---|---|---|
| 0x10020055 sound slot volume | needs a design decision and a test that reaches it | never called in either run |
| 0x10021e8c camera target | needs a design decision and a test that reaches it | never called |
| 0x100232d7 camera tracking | needs a design decision (what 0x1009cf67 does with an empty segment) | the never-built segment was passed once in run 2; [ebp-0x2c] never unset |
| 0x100687a3 within-radius test | needs a design decision ("skip the 0.0 checks" matches what garbage does) and a test that reaches it | never called |
| 0x100b66a0 HUD drawing | **start at 1** | unset on every call; always 1 (original) or 5 (C), only tested against 0 |
| 0x100c2ae5 hit test | **zero OK** | 75k calls; the bad path ran 146 times but the value was always written |

The other 19:

| Function | Recommendation | Evidence |
|---|---|---|
| 0x1000aa9e | needs a test that reaches it | never called |
| 0x10055048 input | **zero OK** | 20.7k calls, never unset |
| 0x1005ab44 node-name search | needs a design decision | site 3 unset 317 times, values vary |
| 0x1005bb7a POF loader | **zero OK** | 12 calls, never unset (the shipped models) |
| 0x10067ec6 kill credit | needs a design decision (zero = no killer?) | garbage killer once in run 1 |
| 0x10089066 node-tree bounds | needs a design decision | garbage enters fld arithmetic 6 times per run; the compare site is unset often |
| 0x1008950e node-tree bounds 2 | needs a design decision | the same |
| 0x100918e6, 0x1009221d renderer | needs a test that reaches it, plus a renderer check | never called |
| 0x1009353e, 0x100945b8, 0x1009596a, 0x1009fada renderer | needs a static renderer check (does it read those fields?) | called often; the watchers can't see inside the renderer |
| 0x100a0880, 0x100ac9ab clip vertices | needs a static clipper check | the same |
| 0x100a65e7 terrain? | needs a design decision | 11 million unset reads per session, varying leftover pointers |
| 0x100b8d6c resource lookup | needs a test that reaches it | never called |
| 0x100c0158 selection | **zero OK** | 42 calls, never unset |
| 0x100c7120 HUD range readout | **zero OK** | 20k calls, the bad reads never ran |

**Decidable now: 6 of 25.**

- Zero OK: 0x100c2ae5, 0x10055048, 0x1005bb7a, 0x100c0158 and 0x100c7120.
- Start at 1: 0x100b66a0.

The other 19:

- 7 were never reached.
- 6 vary.
- 6 are renderer or clipper functions, settled by reading the renderer code.

## Run 2 data

Source: the game folder's debug.log, the second uninittrace block. Both expected lines are there: the load line
("25 of 25 entry / 29 of 29 read watcher(s) installed") and the summary written at sim exit. No "converted helper"
line follows the load, as expected with noconv.

This run used the short-line output from build 6, so nothing was cut. "Unset" means the read saw the value that was
in the slot when the function started.

| Function | Calls | Bad read: ran / unset | Unset values | Run 1 (with C) |
|---|---|---|---|---|
| 0x10020055 sound slot volume | 0 | - | - | not reached |
| 0x10021e8c camera target | 0 | - | - | not reached |
| 0x100232d7 camera tracking | 6108 | 0x10023d35: 0 / 0; 0x10023ad6 (the never-built segment [ebp-0xdc] handed to 0x1009cf67): 1 / 1 | 0x04641868 (the buffer's contents are what matters, not watched) | site data lost |
| 0x100687a3 within-radius test | 0 | - | - | not reached |
| 0x100b66a0 HUD drawing | 10208 | first-pass test 71456 / 10208 (every call) | **always 0x00000001** | always 5 |
| 0x100c2ae5 hit test | 37895 | hit path 1: 146 / 0; hit path 2: 0 / 0 | - | 0 / 0 |
| 0x1000aa9e geometry lookup | 0 | - | - | not reached |
| 0x10055048 input device | 10732 | 10732 / 0 | - | 9940 / 0 |
| 0x1005ab44 node-name search | 3002 | sites 1-2: 1159 / 0; **site 3 (0x1005ad95): 317 / 317** | varied: 0x1013221c x252, 0 x26, 0x04522bec x5, ... | site 3 lost |
| 0x1005bb7a POF loader | 6 | 6 / 0, 17 / 0, 6 / 0 | - | partly lost, 0 seen |
| 0x10067ec6 kill credit | 2 | 0 / 0, 1 / 0, 0 / 0 | - | 1 unset (garbage killer) |
| 0x10089066 node-tree bounds | 559 | fld: 281 / 6; **cmp 0x1008930f: 458 / 183** | varied: 0x04643a64 x39, 0x048f9504 x3, float-like garbage | fld 284 / 6 |
| 0x1008950e node-tree bounds 2 | 559 | fld: 281 / 5; **cmp 0x100897b1: 417 / 141** | varied: 0x04643a64 x39, ... | fld 284 / 5 |
| 0x100918e6 / 0x1009221d renderer | 0 / 0 | - | - | not reached |
| 0x1009353e renderer | 1042 | not watched | - | 971 |
| 0x100945b8 renderer | 33301 | not watched | - | 82719 |
| 0x1009596a renderer | 19383 | not watched | - | 29926 |
| 0x1009fada vertex buffer | 40 | not watched | - | 0 |
| 0x100a0880 clip vertices | 10732 | not watched | - | 9940 |
| 0x100ac9ab clip vertices | 6833 | not watched | - | 8917 |
| 0x100a65e7 terrain? | 39910915 | **11295972 / 11295972** | varied: 0x1014cd60, 0x1014cdc0, 0x1014cd48, 0x1014cda8, 0x1014cd78 (DLL data addresses, 24 bytes apart), +10.5M other | same pattern |
| 0x100b8d6c resource lookup | 0 | - | - | not reached |
| 0x100c0158 selection | 21 | 21 / 0 | - | 21 / 0 |
| 0x100c7120 HUD range readout | 10208 | 0 / 0 on both sites | - | lost |

What both runs show:

- **Bad path never seen in either run:** 0x100c2ae5, 0x10055048, 0x1005bb7a, 0x100c0158, 0x100c7120.
  Starting their locals at zero would have changed nothing in these sessions.
- **Same branch with any non-zero value:** 0x100b66a0 tests its unset local against 0 on every call. The local was
  always 5 with the C and always 1 with the original, so the "non-zero" branch is taken every time. **Zero would
  flip that branch.** The faithful choice is a non-zero start value, for example 1.
- **Really varies, needs a code-level decision:**
  - 0x100a65e7: millions of reads of leftover pointers into DLL data.
  - 0x1005ab44, site 3: 317 reads.
  - 0x10089066 / 0x1008950e compare sites: about 1 read in 3. They compare against 0x7f7fc99e, which the garbage almost
    never equals, so they probably take the same branch.
  - 0x10067ec6: one garbage killer in run 1.
  - 0x100232d7: one never-built segment in run 2.
- **Never reached in either run:** 0x10020055, 0x10021e8c, 0x100687a3, 0x1000aa9e, 0x100b8d6c, 0x100918e6, 0x1009221d.
