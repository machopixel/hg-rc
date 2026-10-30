# Round 8B: bit-exact redo of the tolerance-only conversions

Diego's rule (2026-10-19): float code that is GAME LOGIC must match the original bit for bit; only pure DRAWING code may pass with the checker's float tolerance (`fp=True` with the default `tol` 4e-7, or a small `tol`). 81 functions converted in rounds 2-6 were only ever proven WITH a tolerance. This round classifies each one and makes every game-logic one exact, or unhooks it.

Read first: `tools/round7/README.md` (the full method, all addenda) and `docs/checker.md`. Disassembler: `python -I tools\round6\dis.py 0xADDR`. PATH must contain C:\Coding\Python315 and C:\msys64\mingw32\bin. NEVER install anything. Do not edit tools\dev\dev.cmd.

## Per function
1. Classify from its callers (grep the Ghidra output `build\ghidra\HGSIM.c` for `FUN_<addr>` / `thunk_FUN_<addr>` and the C sources for its name):
   - DRAWING: the result only affects what is drawn (clipping, projection to the screen, HUD art, gamma, particle/effect quads) and never feeds unit, physics, collision, AI, targeting, timers or network state.
   - GAME LOGIC: everything else (movement, hits, collision, AI, targeting, matrices/quaternions used by objects, trig tables, anything network-relevant). When unsure: GAME LOGIC.
2. GAME LOGIC: prove it EXACT. Write a NEW spec `tools/verify/spec_r8b_<helper>.py` (copy the generators you need from the old spec; do NOT edit the old spec files, other helpers read them) with the function WITHOUT `fp`/`tol` (exact compare), `CFLAGS = ['-msse2', '-mfpmath=sse']` as the real build, many trials (>= 1000, more for branchy code), and values near thresholds, signed zeros, NaN (quiet only: the emulator does not quiet signalling NaNs), infinities, denormals. If it mismatches, fix the C (typical causes: float vs double intermediate, missing float rounding where the original does `fstp dword`, evaluation order, contraction/fma, comparison of NaN, sign of zero, 80-bit x87 intermediates that a double cannot reproduce) until 0 mismatches. If it cannot be made exact (it really depends on 80-bit intermediates, or on fsin/fcos/fpatan 80-bit results used directly), UNHOOK it: comment out its CONV line in `src/hooks/conv_*.inc` as
   `// tolerance-only proof, game logic: the original runs; <one-line reason>` above `// CONV(...)`,
   and make every converted C function that calls it BY C NAME go to the original instead, with a `#define <Name>(...) SIM_FN(<type>, 0x<addr>)(...)` right after the header include in that caller's .c file, exactly as done for Mat3x4_Multiply (see `src/sim/unsorted_100a/w7o_nodes.c`). grep `<Name>(` in src/sim to find all callers.
3. DRAWING: the tolerance may stay. Still run it once without the tolerance and record the worst deviation (or "exact").
4. Fixing a C function in an existing file is allowed in this round (only the functions assigned to you; other helpers own the other files). Keep comments and style. After a fix, the callers' specs need not be rerun.
5. Mat3x4_Multiply (0x10060008) is already unhooked (conv_w3r.inc, plus #defines in w7o_nodes.c, w8u_debris.c, w8y_boxes.c). Helper 1 tries to make it exact; ONLY if proven exact, re-hook it (uncomment the CONV line) and remove those three #defines.
6. Run difftest only on your own new spec, `-j2`, foreground. Compile-check every edited .c with `i686-w64-mingw32-gcc -std=c17 -c`.

## Final report (your last message), compact
Table: address | name | class (DRAWING/LOGIC) + one-line reason | result (already exact / fixed: what / unhooked: why / drawing: worst deviation) | trials. Then the list of files you edited or created.

## Assignment
- h1 (matrices; files view/w3r_matrix.c, view/w3r_camera.c): Mat3x4_TransformPoint, Mat3_CrossColumn, Mat3_Multiply, Mat3x4_Multiply, Mat3x4_InvertRigid, Sim_100b403d, Sim_100d2306 (CameraViewport: also patched by hooks_view.c; classify and check only, it is not in any conv .inc), Sim_100d280d. Old spec: spec_w3r.py.
- h2 (render + misc; files render/w2n_*.c, render/w3r_screen.c, unsorted_1010/w6i_fdiv.c, w6g files): Render_ClipPolygon, Render_SubmitClipped_100ac738, Sim_100ad1f3 (spec_w2n.py); Sim_10052e60, Sim_10052f3a, Sim_10090099 (spec_w3r.py); Fpu_HasFdivBug, Fpu_CheckFdivBug (spec_w6i.py); Sim_1009cc82, Sim_1009cdd5, Sim_1009ceab, Sim_100a8c24, Sim_100ac21a, Sim_100c0423, Sim_100c0471, Sim_100c0ac4, Sim_100c0fc4 (spec_w6g.py).
- h3 (files unsorted_1003/w3r_*.c, unsorted_1005/w3r_misc.c, unsorted_1008/w3r_grid.c, unsorted_100d/w3r_records.c): Sim_10030228, Sim_10033744, Sim_1003c970, Sim_1003cc73, Sim_10054d98, Sim_1005b98c, Sim_10087b00, Sim_100d867b, Sim_100d8936 (spec_w3r.py).
- h4 (w5a and w5z files): Sim_1000ed77, Sim_10060882, Sim_100610ae, Sim_10068c48, Sim_10069f68, Trig_InitTables, Sim_100c34b7 (spec_w5a.py, tol 1e-9); Sim_100aa307 (spec_w5z.py).
- h5 (w5b files): Sim_10047e1a, Sim_10062a3a, Sim_100631ef, Sim_10068bb2, Sim_1008e687, Sim_100c0558, Sim_100c0d40, Sim_100c3892 (spec_w5b.py).
- h6 (w6d files): Sim_1001274b, Sim_100127f0, Sim_1001448d, Sim_1001655f, Sim_100157ca, Sim_100257ec, Sim_100299c8, Sim_1002fba5, Sim_1002fd4c, Sim_10031ed3, Sim_10034cca, Sim_1000bba7 (spec_w6d.py).
- h7 (w6e files): Sim_10038eea, Sim_10044b2c, Sim_100453a3, Sim_10047bcd, Sim_100529a2, Sim_100607d5, Sim_10061ecd, Sim_10061f8e, Sim_10068d76 (spec_w6e.py).
- h8 (w6h files): Sim_100c17b7, Sim_100c25af, Sim_100c2851, Sim_100c3372, Sim_100c3419, Sim_100d6f92, Sim_100d8d0a, Sim_100d9306, Sim_100d9064, Sim_100e2776 (spec_w6h.py).

## Classification table (filled in by the lead from the reports)
| address | name | class | result |
|---|---|---|---|
| 0x1005fbb0 | Mat3x4_TransformPoint | LOGIC | already exact (h1) |
| 0x1005fcdc | Mat3_CrossColumn | LOGIC | already exact (h1) |
| 0x1005fe48 | Mat3_Multiply | LOGIC | already exact (h1) |
| 0x10060008 | Mat3x4_Multiply | LOGIC | already exact (incl. out == a / out == b); RE-HOOKED, caller #defines removed (h1) |
| 0x10060101 | Mat3x4_InvertRigid | LOGIC | already exact (h1) |
| 0x100b403d | Sim_100b403d | LOGIC | already exact (h1) |
| 0x100d2306 | Sim_100d2306 (CameraViewport, patched by hooks_view.c) | DRAWING | exact, deviation 0 (h1) |
| 0x100d280d | Sim_100d280d | DRAWING | exact, deviation 0 (h1) |
| 0x100a0512 | Render_ClipPolygon | DRAWING | exact, deviation 0 (h2) |
| 0x100ac738 | Render_SubmitClipped_100ac738 | DRAWING | exact, deviation 0 (h2) |
| 0x100ad1f3 | Sim_100ad1f3 | DRAWING | exact, deviation 0 (h2) |
| 0x10052e60 | Sim_10052e60 | DRAWING | exact (h2) |
| 0x10052f3a | Sim_10052f3a | DRAWING | exact (h2) |
| 0x10090099 | Sim_10090099 | DRAWING | exact (h2) |
| 0x10100a10 | Fpu_HasFdivBug | LOGIC | already exact (h2) |
| 0x10100a60 | Fpu_CheckFdivBug | LOGIC | already exact (h2) |
| 0x1009cc82 | Sim_1009cc82 | LOGIC | already exact (h2) |
| 0x1009cdd5 | Sim_1009cdd5 | LOGIC | already exact (h2) |
| 0x1009ceab | Sim_1009ceab | LOGIC | already exact (h2) |
| 0x100a8c24 | Sim_100a8c24 | LOGIC | already exact (h2) |
| 0x100ac21a | Sim_100ac21a | LOGIC | already exact (h2) |
| 0x100c0423 | Sim_100c0423 | LOGIC | already exact (h2) |
| 0x100c0471 | Sim_100c0471 | LOGIC | already exact (h2) |
| 0x100c0ac4 | Sim_100c0ac4 | LOGIC | already exact (h2) |
| 0x100c0fc4 | Sim_100c0fc4 | LOGIC | already exact (h2) |
| 0x10030228 | Sim_10030228 | LOGIC | already exact (h3) |
| 0x10033744 | Sim_10033744 | LOGIC | already exact (h3) |
| 0x1003c970 | Sim_1003c970 | LOGIC | already exact (h3) |
| 0x1003cc73 | Sim_1003cc73 | LOGIC | already exact (h3) |
| 0x10054d98 | Sim_10054d98 | LOGIC | already exact (h3) |
| 0x1005b98c | Sim_1005b98c | LOGIC | already exact (h3) |
| 0x10087b00 | Sim_10087b00 | LOGIC | already exact (h3) |
| 0x100d867b | Sim_100d867b | LOGIC | already exact (h3) |
| 0x100d8936 | Sim_100d8936 | LOGIC | already exact (h3) |
| 0x1000ed77 | Sim_1000ed77 | LOGIC | already exact (h4) |
| 0x10060882 | Sim_10060882 | LOGIC | already exact (h4) |
| 0x100610ae | Sim_100610ae | LOGIC | already exact (h4) |
| 0x10068c48 | Sim_10068c48 | LOGIC | FIXED: negated product kept through a volatile double so the NaN sign of fchs is kept (h4) |
| 0x10069f68 | Sim_10069f68 | LOGIC | already exact (h4) |
| 0x100846e0 | Trig_InitTables | LOGIC | UNHOOKED: uses 80-bit fsin / fpatan results directly; not reproducible in C nor provable in the emulator (h4) |
| 0x100c34b7 | Sim_100c34b7 | LOGIC | already exact (h4) |
| 0x100aa307 | Sim_100aa307 | LOGIC | already exact (h4) |
| 0x10047e1a | Sim_10047e1a | LOGIC | already exact (h5) |
| 0x10062a3a | Sim_10062a3a | DRAWING | exact, deviation 0 (h5) |
| 0x100631ef | Sim_100631ef | LOGIC | already exact (h5) |
| 0x10068bb2 | Sim_10068bb2 | LOGIC | already exact (h5) |
| 0x1008e687 | Sim_1008e687 | DRAWING | exact, deviation 0 (h5) |
| 0x100c0558 | Sim_100c0558 | LOGIC | already exact (h5) |
| 0x100c0d40 | Sim_100c0d40 | LOGIC | already exact (h5) |
| 0x100c3892 | Sim_100c3892 | LOGIC | already exact (h5) |
| 0x1001274b | Sim_1001274b | LOGIC | already exact (h6) |
| 0x100127f0 | Sim_100127f0 | LOGIC | already exact (h6) |
| 0x1001448d | Sim_1001448d | LOGIC | already exact (h6) |
| 0x1001655f | Sim_1001655f | LOGIC | already exact (h6) |
| 0x100157ca | Sim_100157ca | LOGIC | already exact (h6) |
| 0x100257ec | Sim_100257ec | LOGIC | already exact (h6) |
| 0x100299c8 | Sim_100299c8 | LOGIC | already exact (h6) |
| 0x1002fba5 | Sim_1002fba5 | LOGIC | already exact (h6) |
| 0x1002fd4c | Sim_1002fd4c | LOGIC | already exact (h6) |
| 0x10031ed3 | Sim_10031ed3 | LOGIC | already exact (h6) |
| 0x10034cca | Sim_10034cca | LOGIC | already exact (h6) |
| 0x1000bba7 | Sim_1000bba7 | LOGIC | already exact (h6) |
| 0x10038eea | Sim_10038eea | LOGIC | already exact (h7) |
| 0x10044b2c | Sim_10044b2c | LOGIC | already exact (h7) |
| 0x100453a3 | Sim_100453a3 | LOGIC | already exact (h7) |
| 0x10047bcd | Sim_10047bcd | LOGIC | already exact (h7) |
| 0x100529a2 | Sim_100529a2 | LOGIC | already exact (h7) |
| 0x100607d5 | Sim_100607d5 | LOGIC | already exact (h7) |
| 0x10061ecd | Sim_10061ecd | LOGIC | already exact (h7) |
| 0x10061f8e | Sim_10061f8e | LOGIC | already exact (h7) |
| 0x10068d76 | Sim_10068d76 | LOGIC | exact with the Sim_10068c48 fix (h7) |
| 0x100c17b7 | Sim_100c17b7 | LOGIC | already exact (h8) |
| 0x100c25af | Sim_100c25af | LOGIC | already exact (h8) |
| 0x100c2851 | Sim_100c2851 | LOGIC | already exact (h8) |
| 0x100c3372 | Sim_100c3372 | LOGIC | already exact (h8) |
| 0x100c3419 | Sim_100c3419 | LOGIC | already exact (h8) |
| 0x100d6f92 | Sim_100d6f92 | LOGIC | already exact (h8) |
| 0x100d8d0a | Sim_100d8d0a | LOGIC | already exact (h8) |
| 0x100d9306 | Sim_100d9306 | LOGIC | already exact (h8) |
| 0x100d9064 | Sim_100d9064 | LOGIC | already exact (h8) |
| 0x100e2776 | Sim_100e2776 | LOGIC | already exact (h8) |

Summary: 81 functions, 70 LOGIC / 11 DRAWING. 79 already bit-exact without tolerance, 1 fixed (Sim_10068c48), 1 unhooked (Trig_InitTables); every DRAWING one is exact too (deviation 0). Mat3x4_Multiply re-hooked. New specs: tools/verify/spec_r8b_h1..h8.py (spec_r8b_h2.py runs one group per run, env R8B_GROUP = w2n | w3r | w6i | w6g).
Known limits of every proof (also in docs/checker.md): the x87 is assumed at 53-bit precision (control word 0x27f); and when two different NaNs meet in one operation, real x87 and SSE keep different payloads, which the emulator cannot show.
