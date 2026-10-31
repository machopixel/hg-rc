# 1.2's particle effects (rounds C-F): analysis notes

Read-only analysis of GameRip_12/hgsim.dll (1.2) against GameRip/HGSIM.DLL (1.0), round 32. Addresses are each
version's own. "conv" = converted in our C (file).

## What 1.2 did

1.2 replaced 1.0's whole effect system with particle effects. 1.0's effects live in an effect-object pool
(records 0x57e bytes, update slot +0x552 word / +0x55a callback) used by about 45 functions (w6f / w7o / w8x
effects, w22 items, w10, w6h, w8z); 1.2 keeps the pool mechanism but with particle records of 0x406 bytes (16
particles of 0x32 bytes from +0x?; update callback +0x336, draw / free callback +0x33a, type +0x402) and 58 new
functions in 0x10077a07..0x10086750.

## Families: spawn -> (update / draw callback, free callback)

1.0 (spawn functions with direct callers; alloc 0x10090c7d):
- 0x10091085 -> 0x10091071, 0x10090f71
- 0x1009201f -> 0x100918e6, 0x100917ed
- 0x10092956 -> 0x1009221d, 0x10092117
- 0x10092f9d -> 0x10092bbb, 0x10092a71
- 0x100939a3 -> 0x1009353e, 0x10093295
- 0x100941c3 -> 0x10093e5b, 0x10093ce7
- 0x100949e8 -> 0x100945b8, 0x10094423
- 0x10094fcb -> 0x10094d7f, 0x10094c2a
- 0x10095e0a -> 0x1009596a, 0x1009589c
- 0x10096ac3 -> 0x10096869, 0x100967cc

1.2 (alloc 0x10079c54):
- 0x1007a0a6 -> 0x1007a092, 0x10079f7d
- 0x1007acf3 -> 0x1007a379, 0x1007a26c
- 0x1007b956 -> 0x1007afdc, 0x1007aec2
- 0x1007c1a1 -> 0x1007bca5, 0x1007bb48
- 0x1007d342 -> 0x1007cd78 (one callback)
- 0x1007e25c -> 0x1007dd89, 0x1007dbe7
- 0x1007f19f, 0x1007f8a9 -> 0x1007eb46, 0x1007e99d (shared)
- 0x100805dc -> 0x1008011d, 0x1007ffb3
- 0x1008139b -> 0x10080f53, 0x10080d38
- 0x10082556 -> 0x10081bdc, 0x10081afb
- 0x10083679 -> 0x10082da9, 0x10082cc8
- 0x10084362 -> 0x10083e8a, 0x10083dde
- 0x1008514d -> 0x10084b83, 0x10084a87 (grenade bounce dust)
- 0x10086007 -> 0x10085b34, 0x10085992

## Who spawns them (1.2 caller -> its 1.0 counterpart, conversion)

- 0x100191d7 grenade shot (1.0 0x100225c7, conv w10_shot.c): 0x100805dc, 0x10082556, 0x10084362, 0x1008514d
- 0x10019f1d main shot (1.0 0x100232d7, conv w29_shot.c): the three explosions (via thunks)
- 0x1000b8db (1.0 0x1001274b, conv w6d_calls.c): 0x10082556
- 0x1001ef21 (1.0 0x10029421, conv w8u_debris.c): 0x1007f8a9
- 0x1005ab68 (1.0 0x1006ad98, conv w10_misc.c): 0x1007c1a1
- 0x1005bfca (1.0 0x1006bf33, conv w9c_event.c): 0x1007d342
- 0x1005d75c, 11.5 KB (1.0 0x1006e74b / 0x1006e22c, NOT converted): 0x1007e25c, 0x1007f8a9, 0x100805dc
- 0x100bb425 (1.0 0x100d5485, NOT converted): 0x1007f19f
- 0x100baf31 (1.0 0x100d5191 / 0x100d4fd6 / 0x100d4f71, NOT converted): 0x1007a0a6, 0x1007acf3, 0x1007b956
- 0x1003fbb7 (no 1.0 counterpart found): 0x10086007
- 0x100be504 / 0x100beafe (1.0 0x100d9064 / 0x100d965e, conv w6h_calls.c / w10_misc.c): alloc 0x10079c54
- 1.2 sim main 0x1006fae1 (1.0 0x10084feb, conv w21_simmain.c): setup 0x1007986e, per-frame 0x10079dfa

## Pool functions (1.0 -> 1.2)

- 0x100903e0 -> 0x1007939b (same), 0x1009044e -> 0x10079409 (same), 0x100905e8 -> 0x100795a3 (same)
- 0x10090099 -> 0x10078a59 (layout only), 0x10090d15 -> 0x10079cec (record 0x57e -> 0x406, +0x552 -> +0x332),
  0x10090f17 -> 0x10079eee (record size, +0x55a -> +0x33a, sets bit 4 of the object's +0x10 when the type has a
  texture record, and a new flag from a call at its start)
- the other 1.0 pool users (0x10090513, 0x10090581, 0x100908b3, 0x10090c7d, 0x10090cc8, 0x10090e23) to pair

## Setup and data

- 0x1007986e: up to 1000 systems (0x3e8): each an engine object (0x1000387c = 1.0 0x10008f1c) with a quad mesh
  (0x10003a96 / 0x10003b57 / 0x10003bd8 / ... = 1.0 0x10009136 / 0x100091f7 / 0x10009278 / ..., all unchanged);
  pointers at 0x1018ffe0 (1000 x 4), records at 0x10191060 (1000 x 0x406), counts 0x1014b590 (created),
  0x1014b594 (active), list heads 0x1014b598 / 0x1014b59c, last index 0x1018ef30; then 0x100795a3, 0x100790be.
- 0x10079dfa (per frame): the active list -> array 0x1018f040 / count 0x10191040, each record's +0x336 callback,
  a record whose callback returns 0 freed (0x100794ce), the active count lowered.
- particle type table 0x1011e700 (22 x 0x1e; ported: src/sim/fx/w32_partspec.c), PartSpec.ini from the current
  folder (root copy), 1.2's heap sizes equal 1.0's.
- draw 0x10078c29: per particle a 4-vertex quad (corners +-size, UVs from 0x1011e998), sorted (0x10078be0), drawn
  through the driver slot [0x1011fa54] (1.0 [0x1014cb4c]) with the flags left in eax by 0x1000204c.
