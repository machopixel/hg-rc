# Differential tests for round 8, letter t (see difftest.py and docs/checker.md).
#
# Every helper a tested function calls is replaced in the emulator by a stub while that function is tested (the
# stubs are switched on per trial, so a function of this list can be stubbed as the helper of another one and
# tested itself): a code hook at the helper's address appends ('stub_<address>', arguments) to the checker's call
# log (compared between the original and the C run like the Windows calls), may write results through pointer
# arguments (the same bytes for both runs), and returns to the caller with eax set from the trial's list, or for a
# helper whose result is a float in st(0) through a small routine that loads the chosen float or double.
#  * Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their own address.
#  * Converted helpers (called by name) are stubbed at their original address (reached by the original code) and
#    at the address of their C function in the test image (reached by the C code), under the same log name. Both
#    versions are proven equal by their own specs.
# Stack addresses are logged as 'stack'; pointer arguments to small records are logged by the bytes they point to
# (DEREF), texts by content (SARGS); arguments the helper reads as 16 bits are masked (MASK). The handler tables of
# the DLL's data that the code calls through (0x10130e58, 0x10130e98, 0x10130eb8) get logging routines.
# Run for real on both sides: the C library sprintf 0x100fc780, fmod 0x100fcb30, fabs 0x100fcb70, 0x100fcc74, the
# table lookups 0x10084766 / 0x1008486e (control word 0x27f, as in the game) and the converted string helpers.
# All floating point here is exact: no float tolerance is used. Float inputs are never signalling NaNs.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

MINE = [
    'sim/unsorted_1000/w8t_mono.c', 'sim/unsorted_1000/w8t_sight.c', 'sim/unsorted_1000/w8t_sound.c',
    'sim/unsorted_1001/w8t_actions.c', 'sim/unsorted_1001/w8t_misc.c', 'sim/unsorted_1001/w8t_queue.c',
    'sim/unsorted_1002/w8t_misc.c', '../tools/verify/w8t_test.c',
]
# existing converted code the functions above call (and what that calls)
OTHERS = [
    "sim/core/timer.c", "sim/core/w2a_timers.c", "sim/render/w2n_polygon.c", "sim/render/w2n_project.c",
    "sim/render/w3q_texture.c", "sim/unsorted_1000/w2a_fields.c", "sim/unsorted_1000/w2e_mono.c",
    "sim/unsorted_1000/w2e_names.c", "sim/unsorted_1000/w2f_records.c", "sim/unsorted_1000/w2g_fields.c",
    "sim/unsorted_1000/w2h_access.c", "sim/unsorted_1000/w2h_reset.c", "sim/unsorted_1000/w2j_misc.c",
    "sim/unsorted_1000/w2j_slots.c", "sim/unsorted_1000/w2k_misc.c", "sim/unsorted_1000/w2l_fields.c",
    "sim/unsorted_1000/w2l_objects.c", "sim/unsorted_1000/w3o_sphere.c", "sim/unsorted_1000/w3q_misc.c",
    "sim/unsorted_1000/w3s_model.c", "sim/unsorted_1000/w3t_records.c", "sim/unsorted_1000/w5a_misc.c",
    "sim/unsorted_1000/w5b_calls.c", "sim/unsorted_1000/w5c_calls.c", "sim/unsorted_1000/w5x_calls.c",
    "sim/unsorted_1000/w5y_misc.c", "sim/unsorted_1000/w5z_calls.c", "sim/unsorted_1000/w6d_calls.c",
    "sim/unsorted_1000/w6d_select.c", "sim/unsorted_1000/w7l_misc.c", "sim/unsorted_1001/w2a_records.c",
    "sim/unsorted_1001/w2e_mono.c", "sim/unsorted_1001/w2e_names.c", "sim/unsorted_1001/w2e_tables.c",
    "sim/unsorted_1001/w2f_misc.c", "sim/unsorted_1001/w2g_fields.c", "sim/unsorted_1001/w2h_objects.c",
    "sim/unsorted_1001/w2j_misc.c", "sim/unsorted_1001/w2k_misc.c", "sim/unsorted_1001/w2l_misc.c",
    "sim/unsorted_1001/w3t_loader.c", "sim/unsorted_1001/w3t_misc.c", "sim/unsorted_1001/w4u_cd.c",
    "sim/unsorted_1001/w5a_misc.c", "sim/unsorted_1001/w5b_calls.c", "sim/unsorted_1001/w5c_calls.c",
    "sim/unsorted_1001/w5c_math.c", "sim/unsorted_1001/w5x_calls.c", "sim/unsorted_1001/w5y_misc.c",
    "sim/unsorted_1001/w6d_calls.c", "sim/unsorted_1001/w6d_slots.c", "sim/unsorted_1001/w7l_misc.c",
    "sim/unsorted_1002/w2e_object.c", "sim/unsorted_1002/w2e_tables.c", "sim/unsorted_1002/w2f_misc.c",
    "sim/unsorted_1002/w2g_records.c", "sim/unsorted_1002/w2h_misc.c", "sim/unsorted_1002/w2i_misc.c",
    "sim/unsorted_1002/w2j_nodes.c", "sim/unsorted_1002/w2k_misc.c", "sim/unsorted_1002/w2l_tables.c",
    "sim/unsorted_1002/w3o_ray.c", "sim/unsorted_1002/w3o_vectors.c", "sim/unsorted_1002/w4u_heap.c",
    "sim/unsorted_1002/w4v_misc.c", "sim/unsorted_1002/w5a_misc.c", "sim/unsorted_1002/w5x_calls.c",
    "sim/unsorted_1002/w5y_misc.c", "sim/unsorted_1002/w5z_calls.c", "sim/unsorted_1002/w6d_calls.c",
    "sim/unsorted_1002/w7l_collide.c", "sim/unsorted_1002/w7l_misc.c", "sim/unsorted_1003/entry.c",
    "sim/unsorted_1003/records.c", "sim/unsorted_1003/setters.c", "sim/unsorted_1003/slots.c",
    "sim/unsorted_1003/w2b_misc.c", "sim/unsorted_1003/w2e_entries.c", "sim/unsorted_1003/w2e_misc.c",
    "sim/unsorted_1003/w2e_mono.c", "sim/unsorted_1003/w2g_misc.c", "sim/unsorted_1003/w2l_records.c",
    "sim/unsorted_1003/w3r_misc.c", "sim/unsorted_1003/w3s_entry.c", "sim/unsorted_1003/w4u_heap.c",
    "sim/unsorted_1003/w5a_misc.c", "sim/unsorted_1003/w5c_calls.c", "sim/unsorted_1004/fields.c",
    "sim/unsorted_1004/tables.c", "sim/unsorted_1004/w2e_strings.c", "sim/unsorted_1004/w2h_record.c",
    "sim/unsorted_1004/w2i_misc.c", "sim/unsorted_1004/w2k_flags.c", "sim/unsorted_1004/w3o_misc.c",
    "sim/unsorted_1004/w3p_net.c", "sim/unsorted_1004/w4u_heap.c", "sim/unsorted_1004/w4w_net.c",
    "sim/unsorted_1004/w5y_misc.c", "sim/unsorted_1005/w2e_lists.c", "sim/unsorted_1005/w2g_pool.c",
    "sim/unsorted_1005/w2h_tables.c", "sim/unsorted_1005/w3o_calls.c", "sim/unsorted_1005/w3q_misc.c",
    "sim/unsorted_1005/w4v_misc.c", "sim/unsorted_1005/w5b_calls.c", "sim/unsorted_1006/w3q_quat.c",
    "sim/unsorted_1006/w5a_math.c", "sim/unsorted_1006/w5b_calls.c", "sim/unsorted_1006/w5c_math.c",
    "sim/unsorted_1006/w5y_misc.c", "sim/unsorted_1006/w6e_misc.c", "sim/unsorted_1007/w2e_state.c",
    "sim/unsorted_1007/w2f_misc.c", "sim/unsorted_1007/w2g_stubs.c", "sim/unsorted_1007/w3p_net.c",
    "sim/unsorted_1007/w5a_misc.c", "sim/unsorted_1008/cell_link.c", "sim/unsorted_1008/w2g_rect.c",
    "sim/unsorted_1008/w2i_rect.c", "sim/unsorted_1008/w3o_grid.c", "sim/unsorted_1008/w3o_length.c",
    "sim/unsorted_1008/w3s_grid.c", "sim/unsorted_1008/w4v_fade.c", "sim/unsorted_1009/w2e_copy.c",
    "sim/unsorted_1009/w2g_objects.c", "sim/unsorted_1009/w2h_misc.c", "sim/unsorted_1009/w2l_pool.c",
    "sim/unsorted_1009/w3o_ray.c", "sim/unsorted_1009/w3q_misc.c", "sim/unsorted_1009/w6g_segment.c",
    "sim/unsorted_100a/w2f_links.c", "sim/unsorted_100a/w2i_misc.c", "sim/unsorted_100a/w2k_list.c",
    "sim/unsorted_100a/w3t_misc.c", "sim/unsorted_100a/w4u_misc.c", "sim/unsorted_100a/w5a_misc.c",
    "sim/unsorted_100a/w5b_calls.c", "sim/unsorted_100a/w5c_calls.c", "sim/unsorted_100a/w5x_calls.c",
    "sim/unsorted_100a/w5y_misc.c", "sim/unsorted_100a/w5z_calls.c", "sim/unsorted_100a/w6g_nodes.c",
    "sim/unsorted_100b/fields.c", "sim/unsorted_100b/w2e_object.c", "sim/unsorted_100b/w2e_strings.c",
    "sim/unsorted_100b/w2h_records.c", "sim/unsorted_100b/w2j_misc.c", "sim/unsorted_100b/w2l_records.c",
    "sim/unsorted_100b/w3o_hash.c", "sim/unsorted_100b/w3s_array.c", "sim/unsorted_100b/w4v_wrappers.c",
    "sim/unsorted_100b/w5c_calls.c", "sim/unsorted_100b/w5z_calls.c", "sim/unsorted_100b/w6g_misc.c",
    "sim/unsorted_100c/w2e_tables.c", "sim/unsorted_100c/w2h_misc.c", "sim/unsorted_100c/w2i_misc.c",
    "sim/unsorted_100c/w2l_objects.c", "sim/unsorted_100c/w3o_misc.c", "sim/unsorted_100c/w4v_timers.c",
    "sim/unsorted_100c/w5a_misc.c", "sim/unsorted_100c/w5c_math.c", "sim/unsorted_100c/w6h_calls.c",
    "sim/unsorted_100d/nodes.c", "sim/unsorted_100d/state.c", "sim/unsorted_100d/w2d_state.c",
    "sim/unsorted_100d/w2f_records.c", "sim/unsorted_100d/w2g_misc.c", "sim/unsorted_100d/w2i_select.c",
    "sim/unsorted_100d/w2j_nodes.c", "sim/unsorted_100d/w3q_misc.c", "sim/unsorted_100d/w4v_misc.c",
    "sim/unsorted_100d/w5c_math.c", "sim/unsorted_100d/w6h_calls.c", "sim/unsorted_100d/w7q_names.c",
    "sim/unsorted_100f/w2e_string.c", "sim/unsorted_100f/w2g_crt.c", "sim/unsorted_100f/w2j_str.c",
    "sim/unsorted_100f/w2k_string.c", "sim/unsorted_100f/w6h_crt.c", "sim/unsorted_100f/w6i_misc.c",
    "sim/unsorted_100f/w6i_stream.c", "sim/unsorted_100f/w7q_stream.c", "sim/unsorted_1010/w2i_crt.c",
    "sim/unsorted_1010/w2k_crt.c", "sim/unsorted_1010/w6j_stream.c", "sim/unsorted_1010/w6k_text.c",
    "sim/util/field.c", "sim/util/lookup.c", "sim/util/record.c", "sim/util/ring.c", "sim/view/matrix.c",
    "sim/view/objlist.c", "sim/view/viewrect.c", "sim/view/w2d_view.c", "sim/view/w2l_lists.c",
    "sim/view/w3r_matrix.c", "sim/win/w4a_heap.c", "sim/win/w4u_cdaudio.c", "sim/win/w4v_dll.c",
    "sim/win/w4v_heap.c",
]
SOURCES = MINE + OTHERS
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
OUTF = 0x00AFF000       # st(0) results are stored here (by the trampolines and the C test wrappers)
TRAMP = 0x00AF0000      # machine-code trampolines for the original side
CODE = 0x00AF8000       # targets of indirect calls (a `ret`, with a logging hook)
FRET = 0x00AFA000       # routines `fld [value]; ret` that stubs with a float result jump to
FVAL = 0x00AFE000       # the values those routines load

# Stubbed helpers: log name (original address) -> (C function name or None, number of dword arguments logged,
# result kind: 'i' eax, 'f' float in st(0), 'd' double in st(0)).
HELPERS = {}


def helper(addr, cname, nargs, kind='i'):
    HELPERS[addr] = (cname, nargs, kind)


# not converted (SIM_FN)
for _a, _n, _k in [(0x1007e29a, 5, 'i'), (0x100cbc6b, 7, 'i'), (0x1000c1ea, 4, 'i'), (0x1000e0d9, 7, 'i'),
                   (0x1000f096, 1, 'i'), (0x100282d4, 4, 'i'), (0x10010fbf, 1, 'd'), (0x10017263, 1, 'i'),
                   (0x1001484e, 2, 'i'), (0x100bfa33, 1, 'i'), (0x10011c28, 0, 'i'), (0x10012af0, 1, 'i'),
                   (0x100fd760, 1, 'i'), (0x1001d0fa, 0, 'i'), (0x1001b902, 1, 'i'), (0x100ba7d5, 3, 'i'),
                   (0x100232d7, 1, 'i'), (0x100225c7, 1, 'i'), (0x10021e8c, 1, 'i'), (0x100d3510, 3, 'i'),
                   (0x100255de, 7, 'i'), (0x100278c0, 2, 'i'), (0x10027b69, 1, 'i'), (0x100bc280, 2, 'i'),
                   (0x100fc780, 2, 'i')]:
    helper(_a, None, _n, _k)
# converted (called by name), and the functions of this list when another one calls them
for _a, _c, _n, _k in [
        (0x1000799a, 'Sim_1000799a', 0, 'i'), (0x10008317, 'Mono_TickSpinner', 0, 'i'),
        (0x10007ae3, 'Mono_ShowFrameRate', 0, 'i'), (0x10007c45, 'Sim_10007c45', 0, 'i'),
        (0x10007c9e, 'Sim_10007c9e', 0, 'i'), (0x10007ed5, 'Sim_10007ed5', 0, 'i'),
        (0x10007fba, 'Mono_ShowPosition', 1, 'i'), (0x10008123, 'Sim_10008123', 0, 'i'),
        (0x10008221, 'Mono_ShowMemory', 0, 'i'), (0x100082be, 'Sim_100082be', 0, 'i'),
        (0x10008197, 'Cache_ShowItemCount', 0, 'i'), (0x100081dd, 'Cache_ClearItemCount', 0, 'i'),
        (0x1002850e, 'Sim_1002850e', 0, 'i'), (0x100346ad, 'Mono_PutText', 4, 'i'),
        (0x1007df6c, 'Sim_1007df6c', 4, 'i'), (0x10008390, 'Mono_PutTopLine', 1, 'i'),
        (0x100d6f16, 'Sim_100d6f16', 2, 'i'), (0x1009cd36, 'Sim_1009cd36', 2, 'i'),
        (0x1002dda9, 'Sim_1002dda9', 3, 'i'), (0x100c2851, 'Sim_100c2851', 4, 'i'),
        (0x1002d961, 'Sim_1002d961', 3, 'd'), (0x1009c929, 'Sim_1009c929', 1, 'i'),
        (0x1000bf73, 'Sim_1000bf73', 2, 'i'), (0x1000bba7, 'Sim_1000bba7', 2, 'i'),
        (0x1000a6f0, 'Sim_1000a6f0', 2, 'i'), (0x1000c172, 'Sim_1000c172', 1, 'i'),
        (0x1001448d, 'Sim_1001448d', 3, 'f'), (0x100144bb, 'Sim_100144bb', 3, 'f'),
        (0x1000ed25, 'Sim_1000ed25', 7, 'i'), (0x100a9cfb, 'Sim_100a9cfb', 4, 'i'),
        (0x1000f029, 'Sim_1000f029', 1, 'i'), (0x1002828e, 'Sim_1002828e', 2, 'i'),
        (0x1001150d, 'Sim_1001150d', 1, 'i'), (0x10052aaf, 'Sim_10052aaf', 1, 'i'),
        (0x10016a6b, 'Sim_10016a6b', 4, 'i'), (0x10020456, 'Sim_10020456', 2, 'i'),
        (0x1001139e, 'Sim_1001139e', 1, 'i'), (0x100bea42, 'Sim_100bea42', 1, 'i'),
        (0x10017a05, 'Sim_10017a05', 1, 'i'), (0x10052be7, 'Sim_10052be7', 1, 'i'),
        (0x10015afd, 'Sim_10015afd', 2, 'i'), (0x100117f8, 'Sim_100117f8', 1, 'i'),
        (0x1001403a, 'Sim_1001403a', 2, 'i'), (0x100125a3, 'Sim_100125a3', 1, 'i'),
        (0x100123bf, 'Sim_100123bf', 1, 'i'), (0x10013fe1, 'Sim_10013fe1', 4, 'i'),
        (0x10015630, 'Sim_10015630', 4, 'i'), (0x10013f2a, 'Sim_10013f2a', 1, 'i'),
        (0x100135d4, 'Sim_100135d4', 5, 'i'), (0x10016cb7, 'Sim_10016cb7', 3, 'i'),
        (0x10014061, 'Sim_10014061', 1, 'f'), (0x10013297, 'Sim_10013297', 3, 'i'),
        (0x10017852, 'Sim_10017852', 1, 'i'), (0x100159cf, 'Sim_100159cf', 2, 'i'),
        (0x1001570f, 'Sim_1001570f', 1, 'i'), (0x100157ca, 'Sim_100157ca', 1, 'i'),
        (0x10015018, 'Sim_10015018', 3, 'i'), (0x100152fc, 'Sim_100152fc', 3, 'i'),
        (0x10011be7, 'Mono_BlankRows', 0, 'i'), (0x10012970, 'Sim_10012970', 0, 'i'),
        (0x10015266, 'Sim_10015266', 1, 'f'), (0x100140df, 'Sim_100140df', 1, 'i'),
        (0x10020256, 'Sim_10020256', 2, 'i'), (0x100ca417, 'Sim_100ca417', 2, 'i'),
        (0x100155b7, 'Sim_100155b7', 2, 'i'), (0x10013e3c, 'Sim_10013e3c', 2, 'i'),
        (0x10013e20, 'Sim_10013e20', 2, 'i'), (0x1001457f, 'Sim_1001457f', 1, 'i'),
        (0x10012a72, 'Sim_10012a72', 2, 'i'), (0x100119a7, 'Sim_100119a7', 1, 'i'),
        (0x100fd850, 'Sim_100fd850', 2, 'i'), (0x100fd7e0, 'Crt_FPrintf', 2, 'i'),
        (0x1000df9c, 'Sim_1000df9c', 1, 'i'), (0x1001fc18, 'Sim_1001fc18', 1, 'i'),
        (0x100bb2b0, 'Sim_100bb2b0', 2, 'i'), (0x1000e737, 'Sim_1000e737', 12, 'i'),
        (0x100200d1, 'Sim_100200d1', 2, 'i'), (0x1001f790, 'Sim_1001f790', 3, 'i'),
        (0x10084c95, 'Sim_10084c95', 3, 'i'), (0x100b53f0, 'Sim_100b53f0', 4, 'i'),
        (0x10084a94, 'Sim_10084a94', 3, 'd'), (0x100aa0da, 'Sim_100aa0da', 5, 'i'),
        (0x100a9d47, 'Sim_100a9d47', 4, 'i'), (0x100257ec, 'Sim_100257ec', 6, 'i'),
        (0x100d3a67, 'Sim_100d3a67', 0, 'i'), (0x100aa7e7, 'Sim_100aa7e7', 1, 'i'),
        (0x100aa8cb, 'Sim_100aa8cb', 1, 'i'), (0x100aa351, 'Sim_100aa351', 1, 'i'),
        (0x1000711c, 'Sim_1000711c', 2, 'i'), (0x1000716a, 'Sim_1000716a', 2, 'i'),
        (0x10021be0, 'Sim_10021be0', 1, 'i'), (0x10024616, 'Sim_10024616', 8, 'i'),
        (0x1002597b, 'Sim_1002597b', 1, 'i'), (0x100273a1, 'Sim_100273a1', 1, 'i'),
        (0x10027654, 'Sim_10027654', 0, 'i'), (0x10027a8f, 'Sim_10027a8f', 1, 'i'),
        (0x10053132, 'Sim_10053132', 0, 'i'), (0x100db351, 'Sim_100db351', 0, 'i'),
        (0x100279f1, 'Sim_100279f1', 1, 'i'), (0x100530a9, 'Sim_100530a9', 1, 'i'),
        (0x10053010, 'Sim_10053010', 1, 'i'), (0x100da098, 'Sim_100da098', 1, 'i'),
        (0x100dafa8, 'Sim_100dafa8', 2, 'i'), (0x100db259, 'Sim_100db259', 1, 'i')]:
    helper(_a, _c, _n, _k)

M16 = 0xffff
MASK = {(0x100346ad, 1): M16, (0x100346ad, 2): M16, (0x100346ad, 3): M16, (0x1000e0d9, 2): M16,
        (0x1000ed25, 5): M16, (0x10016a6b, 1): M16, (0x10016a6b, 2): M16, (0x10016a6b, 3): M16,
        (0x1001403a, 1): M16, (0x10013fe1, 0): M16, (0x10013fe1, 1): M16, (0x10013fe1, 2): M16,
        (0x10013fe1, 3): M16, (0x10015630, 1): M16, (0x10015630, 2): M16, (0x100135d4, 1): M16,
        (0x100135d4, 2): M16, (0x10014061, 0): M16, (0x10013297, 1): M16, (0x10013297, 2): M16,
        (0x100159cf, 1): M16, (0x10015018, 1): M16, (0x10015018, 2): M16, (0x100152fc, 1): M16,
        (0x100155b7, 1): M16, (0x10012a72, 1): M16, (0x1001484e, 1): M16, (0x1000711c, 0): M16,
        (0x1000711c, 1): M16, (0x1000716a, 0): M16}
SARGS = {0x1007e29a: (0, 2), 0x100346ad: (0,), 0x1007df6c: (0,), 0x10008390: (0,), 0x100fd850: (0, 1),
         0x1000df9c: (0,), 0x100bb2b0: (0, 1), 0x100ba7d5: (0, 1)}
DEREF = {0x1009cd36: {0: 0x38}, 0x1002dda9: {0: 0x38}, 0x100c2851: {0: 0x38}, 0x10084c95: {0: 4, 1: 4, 2: 4},
         0x100278c0: {0: 0x10}, 0x100273a1: {0: 0x20}, 0x10027a8f: {0: 0x20}}
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax) that writes the helper's results
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def ustr(uc, addr, limit=200):
    out = b''
    while len(out) < limit:
        try:
            c = bytes(uc.mem_read(addr + len(out), 1))
        except Exception:
            return ('bad', addr)
        if c == b'\0':
            break
        out += c
    return out


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, spread=1000.0):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs, now and then a quiet NaN or an
    infinity (never a signalling NaN)."""
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        return fbits(r.uniform(-spread, spread))
    if c < 0.65:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, -1e-7, 5000.0]))
    if c < 0.88:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-10, 10))
    if c < 0.92:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    v = r.getrandbits(32)
    if (v & 0x7f800000) == 0x7f800000:
        v |= 0x00400000         # quiet
    return v


def cstr(ctx, data, extra=24):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


# ---- stubs --------------------------------------------------------------------------------------
def _fmt_args(uc, fmt, esp, first):
    """The arguments of a printf-style call: integers as dwords, %f as doubles, %s by content."""
    out, k, i = [], first, 0
    while i < len(fmt):
        if fmt[i:i + 1] != b'%':
            i += 1
            continue
        i += 1
        while i < len(fmt) and fmt[i:i + 1] in b'0123456789.-+ #lh':
            i += 1
        c = fmt[i:i + 1]
        i += 1
        if c == b'%':
            continue
        if c in (b'f', b'e', b'g'):
            out.append(bytes(uc.mem_read(esp + 4 + 4 * k, 8)))
            k += 2
        elif c == b's':
            out.append(ustr(uc, ur32(uc, esp + 4 + 4 * k)))
            k += 1
        else:
            out.append(ur32(uc, esp + 4 + 4 * k))
            k += 1
    return out


def _log_args(uc, addr, nargs, esp):
    raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
    args = []
    for k, v in enumerate(raw):
        if (addr, k) in MASK:
            v &= MASK[(addr, k)]
        if k in SARGS.get(addr, ()):
            v = ustr(uc, v)
        elif k in DEREF.get(addr, {}):
            try:
                v = ('deref', bytes(uc.mem_read(v, DEREF[addr][k])))
            except Exception:
                v = ('bad', v)
        args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
    if addr in (0x100fd7e0, 0x100fc780):    # fprintf / sprintf: the format by content and its arguments
        args = [args[0], ustr(uc, raw[1])] + _fmt_args(uc, ustr(uc, raw[1]), esp, 2)
    return raw, args


def _stub_cb(m, addr, slot):
    cname, nargs, kind = HELPERS[addr]

    def cb(uc, address, size, ud):
        if addr not in ST.get('active', ()):
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        if ST.get('log') is not m.api_log:     # a new run (the original or the C one): count the calls anew
            ST['log'] = m.api_log
            ST['count'] = {}
        raw, args = _log_args(uc, addr, nargs, esp)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        k = ST['count'].get(addr, 0)
        ST['count'][addr] = k + 1
        seq = ST['seq'].get(addr)
        if seq is not None:
            eax = seq[k] if k < len(seq) else ST['seq_end'].get(addr, 0)
        else:
            rets = ST['rets'].get(addr) or [0]
            eax = rets[n % len(rets)]
        eax &= 0xffffffff
        if addr in EFFECT:
            v = EFFECT[addr](uc, k, raw, eax)
            if v is not None:
                eax = v & 0xffffffff
        uc.reg_write(UC_X86_REG_EAX, eax)
        if kind == 'i':
            uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        else:
            vals = ST['fvals'].get(addr) or [0.0]
            val = vals[k % len(vals)]
            if kind == 'f':
                uc.mem_write(FVAL + 16 * slot, struct.pack('<I', val if isinstance(val, int) else fbits(val)))
            else:
                uc.mem_write(FVAL + 16 * slot, struct.pack('<d', val))
            uc.reg_write(UC_X86_REG_EIP, FRET + 16 * slot)
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = (address - CODE) // 16
        nargs, masks = ST['code_nargs'].get(k, (2, {}))
        raw = struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs))) if nargs else ()
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v & masks.get(i, 0xffffffff)
                     for i, v in enumerate(raw))
        n = len(m.api_log)
        m.api_log.append(('code_%d' % k, args))
        rets = ST['code_rets'].get(k) or [mix(ST['seed'], k, n)]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def begin(ctx, *active):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and switches on the
    stubs of the helpers given (original addresses) for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w8t', False):
        m._w8t = True
        for slot, (addr, (cname, nargs, kind)) in enumerate(sorted(HELPERS.items())):
            cb = _stub_cb(m, addr, slot)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            if cname is not None:
                m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
            if kind == 'f':
                m.uc.mem_write(FRET + 16 * slot, b'\xd9\x05' + struct.pack('<I', FVAL + 16 * slot) + b'\xc3')
            elif kind == 'd':
                m.uc.mem_write(FRET + 16 * slot, b'\xdd\x05' + struct.pack('<I', FVAL + 16 * slot) + b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 64 - 1)
        m.uc.mem_map(0xb0000, 0x10000)          # the text screen that the mono helpers write to
        m.wr.append((0xb0000, 0x10000))
        m.snap0 = m.dump()
        m._index()          # re-index now, before this trial writes anything
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    for a in active:
        assert a in HELPERS, hex(a)
    ST['seed'] = r.getrandbits(32)
    ST['code_nargs'] = {}
    ST['code_rets'] = {}
    ST['count'] = {}
    ST['seq'] = {}
    ST['seq_end'] = {}
    ST['fvals'] = {}
    ST.update(picks=[r.getrandbits(16), r.getrandbits(16)], flags=[r.choice([0, 1])], members=[], dists=[0],
              out=[0], hits=[0])
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def seq(addr, values, end=0):
    ST['seq'][addr] = list(values)
    ST['seq_end'][addr] = end


def fvals(addr, *values):
    ST['fvals'][addr] = list(values)


def code_target(ctx, k, nargs, masks=None, *values):
    """Address of a routine (`ret` with a logging hook) for an indirect call."""
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    ST['code_nargs'][k] = (nargs, masks or {})
    if values:
        ST['code_rets'][k] = list(values)
    return a


def tramp_st0(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and stores st(0) as a double at OUTF; returns its address.
    Every function has its own slot: the emulator may keep running an old translation of rewritten code."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\xdd\x1d' + struct.pack('<I', OUTF)       # fstp qword [OUTF]
    code += b'\x31\xc0\x5d\xc3'
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    return a


def tramp_ax(ctx, target, nargs, slot):
    """Machine code that calls target(args as dwords) and returns ax zero-extended (the callers only read ax)."""
    code = b'\x55\x89\xe5'
    for i in range(nargs - 1, -1, -1):
        code += b'\xff\x75' + bytes([8 + 4 * i])
    code += b'\xb8' + struct.pack('<I', target) + b'\xff\xd0'
    code += b'\x83\xc4' + bytes([4 * nargs])
    code += b'\x0f\xb7\xc0\x5d\xc3'                     # movzx eax, ax
    a = TRAMP + 0x100 * slot
    ctx.m.uc.mem_write(a, code)
    return a


def ptrtab(ctx, count, size=0x300):
    """count records in the pointer table 0x109e8b40, count in [0x1015dc64]."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        recs.append(rec)
    w32(ctx, 0x1015dc64, count)
    return recs


def rint(ctx, lo, hi):
    return ctx.rng.randint(lo, hi) & 0xffffffff


# ---- effects of stubbed helpers -----------------------------------------------------------------------
def put32(uc, addr, v):
    uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def put16(uc, addr, v):
    uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def e_floats(*idx):
    """Writes the trial's float values (ST['out'][k], cycling) through the pointer arguments idx."""
    def e(uc, n, raw, eax):
        vals = ST['out']
        for j, i in enumerate(idx):
            put32(uc, raw[i], vals[(n * len(idx) + j) % len(vals)])
    return e


def e_seg(uc, n, raw, eax):
    uc.mem_write(raw[1], struct.pack('<14I', *[mix(ST['seed'], n, k) for k in range(14)]))


def e_cd36(uc, n, raw, eax):
    put32(uc, raw[0] + 0x30, raw[1])


def e_hit(uc, n, raw, eax):
    hits = ST['hits']
    put32(uc, raw[1], hits[n % len(hits)])


def e_c1ea(uc, n, raw, eax):
    vals = ST['out']
    put32(uc, raw[1], vals[(2 * n) % len(vals)])
    put32(uc, raw[2], vals[(2 * n + 1) % len(vals)])


def e_flag(uc, n, raw, eax):
    put32(uc, raw[1], ST['flags'][n % len(ST['flags'])])


def e_sprintf(uc, n, raw, eax):
    """The stubbed sprintf writes a text made from its call number (the same for both runs)."""
    uc.mem_write(raw[0], b'text%08x\0' % mix(ST['seed'], n))


EFFECT.update({0x100fc780: e_sprintf, 0x100cbc6b: e_floats(3, 4, 5, 6), 0x100d6f16: e_seg, 0x1009cd36: e_cd36, 0x1002dda9: e_hit,
               0x100c2851: e_hit, 0x1000c1ea: e_c1ea, 0x1000bf73: e_flag, 0x1000bba7: e_flag,
               0x100a9cfb: e_floats(1, 2, 3)})


# ---- generators: 0x10007890 .. 0x1000ee23 -------------------------------------------------------------
def g_7890(ctx):
    r = begin(ctx, 0x1000799a, 0x10008317, 0x10007ae3, 0x10007c45, 0x10007c9e, 0x10007ed5, 0x10007fba,
              0x10008123, 0x10008221, 0x100082be, 0x10008197, 0x100081dd, 0x1002850e)
    for a in (0x1015d01c, 0x1012dc9c, 0x1015d02c, 0x1015d028, 0x1015d034, 0x1015d03c, 0x1015d040, 0x1015d048,
              0x1012dc98, 0x1015d050, 0x1015d054, 0x1015d058, 0x1015d068, 0x1015d060):
        w32(ctx, a, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1015d064, ctx.u32())
    w32(ctx, 0x1014cae0, ctx.u32())
    w32(ctx, 0x1014cf8c, ctx.u32())
    t = rint(ctx, -1000, 100000)
    w32(ctx, 0x1015dc78, t)
    w32(ctx, 0x1015d05c, r.choice([t, t - 1, t + 1, rint(ctx, -1000, 100000), ctx.u32()]))
    return []


def mono_common(ctx):
    r = ctx.rng
    w32(ctx, 0x10a3ade8, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x109b98c0, ctx.u32())
    for a in range(0x1012dc80, 0x1012dc98, 4):
        w32(ctx, a, ctx.u32())


def g_7ae3(ctx):
    r = begin(ctx, 0x1007e29a, 0x100346ad)
    mono_common(ctx)
    w32(ctx, 0x1015d030, r.choice([0, 1, 1, 1, ctx.u32()]))
    w32(ctx, 0x1015d07c, r.choice([8, 9, 9, 10, 20, rint(ctx, -5, 30), ctx.u32()]))
    w32(ctx, 0x1015d078, r.choice([0, rint(ctx, -50, 600), rint(ctx, 1, 50), ctx.u32()]))
    w32(ctx, 0x1015dc80, r.choice([0, rint(ctx, -20, 200), ctx.u32()]))
    w32(ctx, 0x1015d070, r.choice([0, -3, rint(ctx, 1, 200), ctx.u32()]))
    w32(ctx, 0x1015d074, rint(ctx, -10, 10))
    for a in (0x1015e54c, 0x101c672c, 0x101c673c):
        w32(ctx, a, rint(ctx, -100000, 100000))
    return []


def g_7fba(ctx):
    """sprintf is stubbed here: its float conversion goes through a pointer the C runtime sets up at start."""
    r = begin(ctx, 0x1007e29a, 0x100346ad, 0x100fc780)
    mono_common(ctx)
    w32(ctx, 0x1015d044, r.choice([0, 1, 1, ctx.u32()]))
    pos = ctx.buf(0x18, b''.join(struct.pack('<f', r.choice([r.uniform(-99999, 99999), r.uniform(-1, 1), 0.0,
                                                             -0.0, 12345.678]))
                                 for _ in range(6)))
    return [pos]


def g_8221(ctx):
    r = begin(ctx, 0x1007e29a, 0x100346ad)
    mono_common(ctx)
    w32(ctx, 0x1015d050, r.choice([0, 1, ctx.u32()]))
    w32(ctx, 0x1015d04c, r.choice([0, 1, ctx.u32()]))
    return []


NAMECHARS = b'abcXYZ019 _.'


def g_85b7(ctx):
    r = begin(ctx, 0x1007df6c, 0x10008390)
    mono_common(ctx)
    res = ctx.buf(8, struct.pack('<2I', rint(ctx, 0, 2000), rint(ctx, 0, 2000)))
    w32(ctx, 0x109e3da4, res)
    name = cstr(ctx, bytes(r.choice(NAMECHARS) for _ in range(r.randint(0, 20))))
    return [name, rint(ctx, -100000, 10000000)]


def g_a6f0(ctx):
    r = begin(ctx, 0x100d6f16, 0x1009cd36, 0x1002dda9, 0x100c2851)
    obj = newbuf(ctx, 0x100)
    wf(ctx, obj + 0x68, r.uniform(-400, 400))
    wf(ctx, obj + 0x74, r.uniform(-400, 400))
    wf(ctx, obj + 0xe8, r.uniform(-400, 400))
    if r.random() < 0.1:
        w32(ctx, obj + 0x68, rfl(ctx))
    w32(ctx, obj + 0xd8, rfl(ctx))
    w32(ctx, obj + 0xf0, r.choice([0x200, 0x400, 0x600, 0]) | r.randrange(4) | (ctx.u32() & ~0x6ff))
    hits = [0]
    for _ in range(2):
        h = newbuf(ctx, 0x40)
        w16(ctx, h + 2, r.choice([0x100, 0x200, 0x300, 0, ctx.u32()]))
        w16(ctx, h + 0x24, r.choice([0, 1, 2, 3, ctx.u32()]))
        hits.append(h)
    ST['hits'] = [r.choice(hits) for _ in range(2)]
    rets(0x1002dda9, r.choice([0, 1]))
    rets(0x100c2851, r.choice([0, 0, 1]))
    return [obj, r.choice([0, 1, 1, ctx.u32()])]


def g_a866(ctx):
    r = begin(ctx, 0x100cbc6b, 0x1002d961)
    obj = newbuf(ctx, 0x100)
    wf(ctx, obj + 0x58, r.uniform(-5000, 5000), r.uniform(-5000, 5000), r.uniform(-5000, 5000))
    ST['out'] = [fbits(r.uniform(-360, 360)), rfl(ctx),
                 fbits(r.choice([r.uniform(-1000, 20000), 0.0, 999.0, 1000.0, 5000.0])),
                 fbits(r.uniform(-360, 360))]
    fvals(0x1002d961, *[r.choice([r.uniform(-2000, 2000), r.uniform(-1001, -999), r.uniform(999, 1001),
                                  1000.0, -1000.0, 0.0]) for _ in range(30)])
    return [obj, fbits(r.uniform(-5000, 5000)), fbits(r.uniform(-5000, 5000)), fbits(r.uniform(-5000, 5000))]


def g_d0e2(ctx):
    r = begin(ctx, 0x1000c1ea, 0x1009c929, 0x1000bf73, 0x1000bba7, 0x1000a6f0, 0x1000c172, 0x1001448d,
              0x100144bb)
    recs = ptrtab(ctx, 4)
    for rec in recs:
        st = newbuf(ctx, 0x10)
        w32(ctx, rec + 0x54, st)
        w32(ctx, st + 8, r.choice([0, fbits(1.0), fbits(-2.0), 0x80000000]))
    obj = newbuf(ctx, 0x200)
    blk = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, blk)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x40))
    wf(ctx, blk + 0x1c, r.uniform(-50, 50) if r.random() < 0.3 else r.uniform(-5, 5))
    wf(ctx, obj + 0x64, r.uniform(-50, 50) if r.random() < 0.3 else r.uniform(-5, 5))
    a74 = r.uniform(-20, 20)
    wf(ctx, obj + 0x74, a74)
    w32(ctx, obj + 0xec, fbits(r.uniform(-20, 20)))
    t = rint(ctx, 0, 100000)
    w32(ctx, 0x1015dc78, t)
    w32(ctx, obj + 0x1be, r.choice([t, t - 5, t + 5, rint(ctx, 0, 100000)]))
    w8(ctx, obj + 0x1b6, r.choice([0, 1, 2, 0xff, ctx.u32()]))
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x201, 0x202, 0x203, 0x100, 0x1203, ctx.u32()]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 3]))
    w32(ctx, obj + 0x1c2, r.choice([0, 1]))
    ST['out'] = [fbits(r.choice([r.uniform(-15, 15), a74 + r.uniform(-1.5, 1.5)])),
                 fbits(r.choice([r.uniform(-30, 30), r.uniform(-5, 5)])), rfl(ctx, 20)]
    ST['flags'] = [r.choice([0, 1, ctx.u32()])]
    seq(0x1009c929, [r.choice([0, 0, 0, 1]), r.choice([0, 0, 1])])
    rets(0x1000bf73, r.choice([0, 1]))
    rets(0x1000bba7, r.choice([0, 1]))
    rets(0x1000a6f0, r.choice([0, 1]))
    fvals(0x1001448d, r.uniform(-1, 1))
    fvals(0x100144bb, r.uniform(-1, 1))
    return [obj, fbits(r.choice([r.uniform(-10, 10), r.uniform(-200, 200)]))]


def slot_table(ctx):
    r = ctx.rng
    t = newbuf(ctx, 0x400)
    w32(ctx, 0x1015d0a4, r.choice([t, t, t, t, 0]))
    w32(ctx, t, r.choice([1, 1, 1, 0]))
    for i in range(0x10):
        w32(ctx, t + 0x54 + 4 * i, r.choice([0, 0, 1]))
    return t


def g_e737(ctx):
    r = begin(ctx, 0x1000e0d9)
    slot_table(ctx)
    w32(ctx, 0x109e369c, r.choice([0, 1, 1, 1]))
    for i in range(0x40):
        w16(ctx, 0x109f1520 + 8 * i, r.choice([0xffff, 0x10, 0x8000, ctx.u32()]))
        w16(ctx, 0x109f1524 + 8 * i, ctx.u32())
    w32(ctx, 0x1015dc80, rint(ctx, -10, 100))
    w32(ctx, 0x1015dc78, ctx.u32())
    rets(0x1000e0d9, r.choice([-1, -3, 0, 1, 5, 0xf, r.randrange(16)]))
    vol = r.choice([r.randint(0, 20000), 10000, r.randint(-100000, 100000)])
    pan = r.choice([r.randint(0, 0x1ffff), 0xffff, r.randint(-100000, 100000)])
    return [r.choice([0, 5, 200, rint(ctx, -10, 300)]), ctx.u32(), r.choice([-1, 0, 1, r.randint(1, 0x3f)]),
            r.choice([0, 1, ctx.u32()]), vol, pan, ctx.u32(), ctx.u32(), r.choice([-1, 0, 5, ctx.u32()]),
            ctx.u32(), ctx.u32(), ctx.u32()]


def g_eb12(ctx):
    r = begin(ctx, 0x100cbc6b, 0x1000f096, 0x1000ed25)
    name = r.choice([0, cstr(ctx, b''), cstr(ctx, b'boom'), cstr(ctx, b'SHOT1')])
    ST['out'] = [fbits(r.uniform(-1000, 1000)), fbits(r.choice([r.uniform(0, 50000), r.uniform(-100, 100)])),
                 rfl(ctx), rfl(ctx)]
    w32(ctx, 0x1015ebb4, r.choice([0, rint(ctx, 0, 600), ctx.u32()]))
    rets(0x1000f096, r.choice([rint(ctx, -12000, 100), -10000, -9001, 0, 1]))
    return [rfl(ctx), rfl(ctx), rfl(ctx), name, r.choice([0, 1]), ctx.u32(), ctx.u32()]


def g_ee23(ctx):
    r = begin(ctx, 0x100a9cfb, 0x100cbc6b, 0x1000f029, 0x100282d4, 0x1002828e, 0x1000f096)
    slot_table(ctx)
    cam = newbuf(ctx, 0x20)
    wf(ctx, cam, r.uniform(-1000, 1000), r.uniform(-1000, 1000), r.uniform(-1000, 1000))
    w32(ctx, 0x1014cae0, cam)
    w32(ctx, 0x1014ceb4, ctx.u32())
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj, r.choice([rint(ctx, 0, 3000), 1000]))
    w32(ctx, obj + 4, r.choice([-1, -1, 0xb, 0xc, 0xf]))
    w32(ctx, obj + 8, r.choice([0, 0, ctx.u32()]))
    w32(ctx, obj + 0x18, r.choice([0, 0, 0, 1]))
    w16(ctx, obj + 0x1c, r.choice([0xffff, 5, ctx.u32()]))
    if r.random() < 0.1:
        t = struct.unpack('<I', bytes(ctx.m.uc.mem_read(0x1015d0a4, 4)))[0]
        if t:
            w32(ctx, t + 0x54 + 4 * 0xb, 1, 1, 1, 1, 1)
    ST['out'] = [fbits(r.uniform(-1000, 1000)), fbits(r.uniform(-1000, 1000)), fbits(r.uniform(0, 3000)),
                 rfl(ctx)]
    rets(0x100282d4, r.choice([0, ctx.u32()]))
    rets(0x1000f096, r.choice([rint(ctx, -12000, 100), -10000, -9999]))
    return [obj]


SPECS = [
    dict(name='Sim_10007890', orig=0x10007890, c='Sim_10007890', gen=g_7890, trials=400, void=True),
    dict(name='Mono_ShowFrameRate', orig=0x10007ae3, c='Mono_ShowFrameRate', gen=g_7ae3, trials=500, void=True),
    dict(name='Mono_ShowPosition', orig=0x10007fba, c='Mono_ShowPosition', gen=g_7fba, trials=300, void=True),
    dict(name='Mono_ShowMemory', orig=0x10008221, c='Mono_ShowMemory', gen=g_8221, trials=300, void=True),
    dict(name='Mono_ShowVersion', orig=0x100085b7, c='Mono_ShowVersion', gen=g_85b7, trials=300, void=True),
    dict(name='Sim_1000a6f0', orig=0x1000a6f0, c='Sim_1000a6f0', gen=g_a6f0, trials=800),
    dict(name='Sim_1000a866', orig=0x1000a866, c='Sim_1000a866', gen=g_a866, trials=800),
    dict(name='Sim_1000d0e2', orig=0x1000d0e2, c='Sim_1000d0e2', gen=g_d0e2, trials=1500),
    dict(name='Sim_1000e737', orig=0x1000e737, c='Sim_1000e737', gen=g_e737, trials=800),
    dict(name='Sim_1000eb12', orig=0x1000eb12, c='Sim_1000eb12', gen=g_eb12, trials=800),
    dict(name='Sim_1000ee23', orig=0x1000ee23, c='Sim_1000ee23', gen=g_ee23, trials=800, void=True),
]


# ---- generators: 0x10011093 .. 0x1001fc18 -------------------------------------------------------------
def e_members(uc, n, raw, eax):
    """Sim_10016cb7: writes the trial's member pointers to out[] and returns their number."""
    mem = ST['members']
    for i, p in enumerate(mem):
        put32(uc, raw[1] + 4 * i, p)
    return len(mem)


def e_dist(uc, n, raw, eax):
    """Sim_1001403a sets the distances of obj: new floats at +0xd4 and +0xd8 for every call."""
    d = ST['dists']
    put32(uc, raw[0] + 0xd4, d[(2 * n) % len(d)])
    put32(uc, raw[0] + 0xd8, d[(2 * n + 1) % len(d)])


def e_pick(uc, n, raw, eax):
    """Sim_100135d4 (stubbed for its caller): writes the two picked words."""
    put16(uc, raw[3], ST['picks'][0])
    put16(uc, raw[4], ST['picks'][1])


EFFECT.update({0x10016cb7: e_members, 0x1001403a: e_dist, 0x100135d4: e_pick})


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def obj_rec(ctx, size=0x300):
    """An object record with a state block at +0x54 and a block at +0x28."""
    r = ctx.rng
    obj = newbuf(ctx, size)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x60))
    w32(ctx, obj + 0x28, newbuf(ctx, 0x700))
    return obj


def g_1093(ctx):
    r = begin(ctx, 0x10010fbf, 0x1001150d, 0x10052aaf, 0x10016a6b, 0x10020456, 0x1001139e, 0x100bea42)
    recs = ptrtab(ctx, 6)
    for rec in recs:
        w32(ctx, rec + 0x14, r.choice([0, 1, 2, 3]))
        w32(ctx, rec + 8, r.choice([1, 2, ctx.u32()]))
    w32(ctx, 0x1017b8c8, r.randrange(6))
    obj = obj_rec(ctx)
    w32(ctx, obj + 4, r.choice([0, 3, 100, ctx.u32()]))
    w32(ctx, obj + 8, r.choice([1, 2, ctx.u32()]))
    w32(ctx, obj + 0x14, r.choice([0, 1, 2, 3, 3, 2]))
    for off in (0x188, 0x190, 0x192, 0x194):
        w8(ctx, obj + off, r.choice([0, 0, 1, 0x80, ctx.u32()]))
    for off in (0x18a, 0x18c, 0x18e, 0x196):
        w16(ctx, obj + off, r.choice([0, 0, 1, 0xffff, ctx.u32()]))
    rets(0x10052aaf, r.randrange(6), r.randrange(6))
    fvals(0x10010fbf, r.choice([r.uniform(-1000, 1000), 0.1, 1e30, -3.3]))
    return [obj]


def g_1681(ctx):
    r = begin(ctx, 0x10017a05, 0x10052aaf, 0x10017263, 0x10052be7, 0x10015afd, 0x1000bf73, 0x100117f8,
              0x1001403a, 0x100125a3)
    recs = ptrtab(ctx, 4)
    obj = obj_rec(ctx)
    for i in range(16):
        w32(ctx, 0x10130e58 + 4 * i, code_target(ctx, i, 2, {1: 0xffff}))
    t = rint(ctx, 0, 100000)
    w32(ctx, 0x1015dc78, t)
    w32(ctx, obj + 0x19c, r.choice([t, t - 1, t - 100, t + 1]))
    w32(ctx, obj + 4, r.choice([0, 0, 1, 2]))
    w16(ctx, obj + 0x1a6, r.choice([0, 1, 2, 3, 12, 12, 15, r.randrange(16)]))
    w32(ctx, obj + 8, r.choice([0, 1, 2]))
    w32(ctx, obj + 0x14, r.choice([0, 1, 2, 2, 3, 3]))
    w32(ctx, obj + 0xc, r.choice([0, 1, 2]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2]))
    w32(ctx, obj + 0x84, r.choice([-1, 0, 1, 2, 3]))
    w32(ctx, r32(ctx, obj + 0x28) + 0x4f4, r.choice([0, 1]))
    w32(ctx, 0x1015d114, r.choice([0, 1]))
    rets(0x10017a05, r.choice([0, 0, 0, 1]))
    rets(0x10052aaf, r.choice([0, 1, 2]))
    rets(0x10052be7, r.choice([0, 1, 1, 2]))
    rets(0x100117f8, r.choice([0, 1]))
    ST['dists'] = [0]
    return [obj]


def action_list(ctx, n):
    """The action list of an object: n entries (kind word +0, codes +2, +4, +9, handler byte +6, action +7)."""
    r = ctx.rng
    lst = newbuf(ctx, 4 * (n + 1))
    for i in range(n):
        e = newbuf(ctx, 0x10)
        w16(ctx, e, r.choice([0, 1, 2, 3, 7, 8, 9, r.randrange(8)]))
        w16(ctx, e + 2, r.choice([0x40, 0x25, 0x2a, 5, ctx.u32()]))
        w16(ctx, e + 4, r.choice([0x40, 0x25, 0x2a, 5, ctx.u32()]))
        w8(ctx, e + 6, r.randrange(5))
        w16(ctx, e + 7, ctx.u32())
        w16(ctx, e + 9, r.choice([0x40, 0x25, 0x2a, 5, ctx.u32()]))
        w32(ctx, lst + 4 * i, e)
    w32(ctx, lst + 4 * n, 0)
    return lst


def g_17f8(ctx):
    r = begin(ctx, 0x100123bf, 0x10013fe1, 0x10015630)
    obj = obj_rec(ctx)
    w32(ctx, obj + 0x170, action_list(ctx, r.choice([0, 1, 3, 6])))
    w16(ctx, obj + 0x1a0, r.choice([0, 1, 2, 3, ctx.u32()]))
    w16(ctx, obj + 0x1a2, r.choice([0, 5, ctx.u32()]))
    for i in range(8):
        w32(ctx, 0x10130e98 + 4 * i, code_target(ctx, i, 3, {1: 0xffff, 2: 0xffff},
                                                 r.choice([0, 0, 1, 0x10000, ctx.u32()])))
    for i in range(5):
        w32(ctx, 0x10130eb8 + 4 * i, code_target(ctx, 8 + i, 2, {}, r.choice([0, 1, 1])))
    rets(0x10013fe1, *[r.choice([0, 5, 0x10000, ctx.u32()]) for _ in range(4)])
    return [obj]


def g_23bf(ctx):
    r = begin(ctx, 0x10013f2a, 0x100135d4, 0x10015630, 0x10016cb7)
    obj = obj_rec(ctx)
    key = r.choice([0xffff, 5, 0x203, ctx.u32()]) | (r.getrandbits(16) << 16)
    rets(0x10013f2a, key)
    w32(ctx, obj + 0x1ae, r.choice([0, 0, 1, 0x40]))
    w16(ctx, obj + 0x1aa, r.choice([key, 0, 7]))
    w32(ctx, obj + 0x1d6, r.choice([key & 0xffff, 0, 7]))
    w16(ctx, obj + 0x1a6, r.choice([0, 2, 3, 8]))
    w32(ctx, obj + 0x1dc, r.choice([0, 1]))
    w32(ctx, obj + 4, r.choice([1, 2, 3]))
    w32(ctx, 0x1017b8c8, r.choice([1, 2, 3]))
    partner = obj_rec(ctx, 0x200)
    w32(ctx, partner + 0x1ae, r.choice([0, 0x40, 0x41]))
    rets(0x100135d4, r.choice([0, partner]))
    mem = []
    for _ in range(r.choice([0, 1, 3, 8])):
        m = newbuf(ctx, 0x200)
        w32(ctx, m + 4, r.choice([1, 2, 3]))
        w16(ctx, m + 0x1a6, r.choice([0, 1, 2, 3]))
        w16(ctx, m + 0x1aa, r.choice([key, 0, 9]))
        mem.append(r.choice([m, m, m, 0]))
    ST['members'] = mem
    return [obj]


def target_seq(ctx):
    r = ctx.rng
    return [r.choice([0x201, 0x202, 0x305, 0x3, 0x105, ctx.u32() & 0xfeff]) | (r.getrandbits(16) << 16)
            for _ in range(r.choice([0, 1, 2, 4, 6]))]


def g_305e(ctx):
    r = begin(ctx, 0x10014061, 0x10013297, 0x10017852, 0x100159cf, 0x1001570f, 0x100157ca)
    obj = obj_rec(ctx)
    key = r.choice([0x100, 0x105, 0x5, 0x305, ctx.u32()])
    sel = r.choice([0, 0, 0, 1, -3, ctx.u32()])
    w32(ctx, obj + 0x1d6, r.choice([key & 0xffff, 0, ctx.u32()]))
    w16(ctx, obj + 0x1a6, r.choice([7, 7, 0, 3]))
    w16(ctx, obj + 0x1aa, r.choice([0x400, 0x100, 0x200, 0x600, 0, ctx.u32()]))
    w16(ctx, obj + 0x118, r.choice([0x1000, 5, ctx.u32()]))
    w16(ctx, obj + 0xf0, r.choice([0x1000, 5, ctx.u32()]))
    w32(ctx, obj + 0x21c, r.choice([0, 1, 2, 3, 7, -1, ctx.u32()]))
    rets(0x10013297, r.choice([0, 0, 1, 0x10000, ctx.u32()]))
    fvals(0x10014061, r.choice([r.uniform(-20000, 20000), 150.0, 0.0, -1e9]))
    tramp_ax(ctx, 0x1001305e, 3, 1)
    return [obj, key, sel]


def g_3297(ctx, which=0x10013297, slot=2):
    r = begin(ctx, 0x10015018, 0x100152fc, 0x1001403a, 0x10014061)
    obj = obj_rec(ctx)
    seq(0x10015018, target_seq(ctx), 0xffff | (r.getrandbits(16) << 16))
    rets(0x100152fc, r.choice([0, 0, 1]), r.choice([0, 1]))
    rets(0x1001403a, r.choice([0, 1, 1]), r.choice([0, 1, 1]))
    ST['dists'] = [fbits(r.choice([r.uniform(-100, 100000), r.uniform(0, 3000), 500.0])) for _ in range(5)]
    for off in (0x1d2, 0x1ce):
        w32(ctx, obj + off, r.choice([rint(ctx, -100, 5000), 0, 1000]))
    fvals(0x10014061, r.choice([r.uniform(-20000, 20000), 150.0, 0.0, 3e9]))
    sel = r.choice([0, -3, -4, -2, 1, 5, 30, 1000, ctx.u32()])
    tramp_ax(ctx, which, 3, slot)
    return [obj, ctx.u32(), sel]


def g_349a(ctx):
    r = begin(ctx, 0x10011be7, 0x10011c28, 0x10012970, 0x10012af0)
    w32(ctx, 0x1015d06c, r.choice([0, 1, 1]))
    a = r.choice([-1, -2, 0, 3, ctx.u32()])
    w32(ctx, 0x101303d0, a)
    w32(ctx, 0x101303dc, r.choice([a, a, 5, ctx.u32()]))
    for x in (0x101303d4, 0x101303d8, 0x1015d108):
        w32(ctx, x, ctx.u32())
    return []


def group_block(ctx, group):
    """The group block of 0x10473800 (entry [0x104727a0 + 4 * group], 0x13f bytes per entry)."""
    r = ctx.rng
    entry = r.choice([-1, 0, 1, 5])
    w32(ctx, 0x104727a0 + 4 * group, entry)
    if entry >= 0:
        base = 0x10473800 + group * 0x4fc3a + entry * 0x13f
        ctx.m.uc.mem_write(base, rb(ctx, 0x13f))
        w16(ctx, base + 0x3c, r.choice([4, 0, 1, ctx.u32()]))
        w32(ctx, base + 0x3a + 2, r.choice([0x100, 0x100, 0, ctx.u32()]))
        w32(ctx, base + 0x3a + 0x6f, r.choice([0, 1, 50, -5]))
    return entry


def g_35d4(ctx):
    r = begin(ctx, 0x10016cb7, 0x10015018, 0x1001403a, 0x10013297)
    obj = obj_rec(ctx)
    group = r.randrange(4)
    w32(ctx, obj + 8, group)
    w32(ctx, obj + 4, r.choice([1, 2, 3]))
    group_block(ctx, group)
    targets = target_seq(ctx)
    seq(0x10015018, targets, 0xffff | (r.getrandbits(16) << 16))
    mem = []
    for _ in range(r.choice([0, 1, 3, 8])):
        m = newbuf(ctx, 0x200)
        w32(ctx, m + 4, r.choice([1, 2, 3]))
        w16(ctx, m + 0x1c, r.choice([0, 0, 2, 0x10]))
        w16(ctx, m + 0x1a6, r.choice([2, 3, 0]))
        w16(ctx, m + 0x1aa, r.choice(targets + [0, 0x1201]) if targets else 0)
        mem.append(m)
    ST['members'] = mem
    rets(0x1001403a, r.choice([0, 1, 1]), r.choice([1, 1, 0]))
    rets(0x10013297, r.choice([0, 1, 0x10000]), r.choice([0, 1]))
    ST['dists'] = [fbits(r.uniform(0, 3e8)) for _ in range(5)]
    other, near = newbuf(ctx, 2), newbuf(ctx, 2)
    return [obj, ctx.u32(), ctx.u32(), other, near]


def g_3848(ctx):
    r = begin(ctx, 0x10014061, 0x100135d4)
    obj = obj_rec(ctx)
    w32(ctx, obj + 0x1ae, r.choice([0, 0, 1, 2, 4]))
    w32(ctx, obj + 0x1ce, rint(ctx, -100000, 100000))
    w32(ctx, obj + 0x1ca, rint(ctx, -100000, 100000))
    w32(ctx, obj + 0x1dc, r.choice([0, 1]))
    ST['picks'] = [ctx.u32(), ctx.u32()]
    fvals(0x10014061, r.uniform(-1e6, 1e6))
    tramp_ax(ctx, 0x10013848, 3, 3)
    return [obj, ctx.u32(), r.choice([-2, -1, 0, 0, 5, ctx.u32()])]


def g_412d(ctx):
    r = begin(ctx, 0x10015266, 0x100140df)
    obj = obj_rec(ctx)
    blk = r32(ctx, obj + 0x28)
    st = r32(ctx, obj + 0x54)
    w32(ctx, obj, r.choice([1, 1, 1, 2, 8, 0]))
    w16(ctx, obj + 0x1a6, r.choice([5, 8, 1, 2, 3]))
    w32(ctx, obj + 0x224, r.choice([0x100, 0]))
    w32(ctx, obj + 0x228, r.choice([1, 0]))
    w16(ctx, blk + 0x562, r.choice([0, 1]))
    w32(ctx, blk + 0x607, r.choice([0, 0, 1]))
    wf(ctx, blk + 0x4e8, r.choice([60.0, 70.0, 65.0]))
    lim = r.uniform(0, 500)
    wf(ctx, obj + 0xd4, r.choice([lim * r.uniform(0, 6), lim * 4.5, lim * 3, lim * 1.5, lim * 0.5, r.uniform(0, 3000)]))
    wf(ctx, st + 0xc, r.choice([r.uniform(-100, 100), 30.0, 50.0, r.uniform(40, 100000)]))
    fvals(0x10015266, r.choice([r.uniform(-10, 10), 5.0, -5.0, 4.0, r.uniform(-100, 100)]))
    rets(0x100140df, r.choice([0, 1]))
    tramp_st0(ctx, 0x1001412d, 2, 4)
    return [obj, fbits(lim)]


def g_457f(ctx):
    r = begin(ctx, 0x100152fc, 0x10020256, 0x1001570f, 0x100bfa33, 0x100ca417)
    obj = obj_rec(ctx)
    group = r.randrange(4)
    w32(ctx, obj + 8, group)
    group_block(ctx, group)
    w32(ctx, obj, r.choice([1, 1, 0]))
    w16(ctx, obj + 0x1a6, r.choice(list(range(0, 17)) + [8, 8, 2, 3, 6, 7, 13, 15]))
    w32(ctx, obj + 0x1ae, ctx.u32())
    w16(ctx, obj + 0x1a8, r.choice([0x100, 0, ctx.u32()]))
    rets(0x100152fc, r.choice([0, 1]))
    return [obj]


def g_5630(ctx):
    r = begin(ctx, 0x100155b7, 0x10013e3c, 0x10013e20, 0x1001457f, 0x10052aaf, 0x10012a72, 0x1001484e, 0x100119a7)
    obj = obj_rec(ctx)
    w32(ctx, obj + 4, r.choice([0, 1]))
    rets(0x100155b7, r.choice([0, 1, 1]))
    rets(0x10052aaf, r.choice([0, 1]))
    return [obj, r.choice([-1, 5, 5, 2, ctx.u32()]), r.choice([0x2201, 0x2201, 3, ctx.u32()]), r.choice([0, 1])]


def g_5d30(ctx):
    r = begin(ctx, 0x10015018, 0x100155b7, 0x10015630)
    recs = ptrtab(ctx, 8)
    for rec in recs:
        w32(ctx, rec + 0x10, r.choice([4, 0, 1]))
        w32(ctx, rec + 4, r.choice([0, 1, 2]))
        w16(ctx, rec + 0x1a6, r.choice([0, 12, 1]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2]))
    targets = [r.choice([0x200, 0x0]) | r.randrange(8) | (r.getrandbits(16) << 16) for _ in range(r.choice([0, 1, 3, 6]))]
    seq(0x10015018, targets, 0xffff)
    rets(0x100155b7, r.choice([0, 1, 1]), r.choice([1, 1, 0]))
    key = r.choice([0x4200, 0x4200, 5, ctx.u32()])
    other = r.choice(targets + [5]) if targets else 5
    tramp_ax(ctx, 0x10015d30, 4, 5)
    return [newbuf(ctx, 0x40), key, r.choice([2, 3, 5, 7, 1]), other]


def g_b36c(ctx):
    r = begin(ctx, 0x100fd850, 0x100fd7e0, 0x100fd760)
    rets(0x100fd850, r.choice([0, 0x5000, 0x5000]))
    name = r.choice([0, cstr(ctx, b'anim.txt')])
    model = newbuf(ctx, 0x400)
    ctx.m.uc.mem_write(model + 4, b'model\0')
    hdr = newbuf(ctx, 0x10)
    ntracks = r.choice([0, 1, 3])
    w32(ctx, hdr + 0xc, ntracks)
    w32(ctx, model + 0x14, hdr)
    for i in range(ntracks):
        t = model + 0x1c + 0xc * i
        w32(ctx, t, cstr(ctx, b'track%d' % i))
        for off, size in ((4, 0x10), (8, 0x14)):
            n = r.choice([0, 1, 3])
            keys = newbuf(ctx, 4 + size * n)
            w32(ctx, keys, n)
            for j in range(n):
                for f in range(4, size, 4):
                    wf(ctx, keys + 4 + size * j + f, r.uniform(-1000, 1000))
            w32(ctx, t + off, keys)
    nctl = r.choice([0, 1, 3])
    ctl = newbuf(ctx, 4 + 0x39 * nctl)
    w32(ctx, ctl, nctl)
    for j in range(nctl):
        e = ctl + 4 + 0x39 * j
        ctx.m.uc.mem_write(e + 4, r.choice([b'\0', b'step\0']))
    w32(ctx, model + 0x31c, ctl)
    return [name, r.choice([model, model, model, 0])]


def g_db6f(ctx):
    r = begin(ctx, 0x1001d0fa, 0x1001b902)
    for a in (0x1013b380, 0x1013b38c, 0x1013b398, 0x1013b434, 0x1013b3d4, 0x1013b404):
        w32(ctx, a, newbuf(ctx, 0x40))
    w32(ctx, 0x1017b8c8, r.randrange(25))
    recs = [0] + [newbuf(ctx, 8) for _ in range(3)]
    rets(0x1001b902, *[r.choice(recs) for _ in range(7)])
    return []


def sound_rec(ctx):
    """A sound request record: name at +0, text pointer +0x10, number +0x14."""
    r = ctx.rng
    rec = newbuf(ctx, 0x18)
    ctx.m.uc.mem_write(rec, r.choice([b'\0', b'gun\0', b'boom\0', b'v\0']))
    w32(ctx, rec + 0x10, r.choice([0, cstr(ctx, b''), cstr(ctx, b'Hello'), cstr(ctx, b'a much longer text here')]))
    w32(ctx, rec + 0x14, r.choice([0, 0, 1]))
    return rec


def g_f790(ctx):
    r = begin(ctx, 0x1000df9c, 0x1001fc18)
    w32(ctx, 0x1015d530, r.choice([0, 0, 0, 1]))
    for i in range(8):
        w32(ctx, 0x1015d1d0 + 0x6b * i, r.choice([0, 1, 1]))
    for i in range(0x10):
        w16(ctx, 0x109f1520 + 8 * i, r.choice([0, 5, 9, ctx.u32()]))
        w16(ctx, 0x109f1522 + 8 * i, r.choice([0xffff, 1, 2, 3]))
    head = 0
    if r.random() < 0.7:
        nodes = []
        for _ in range(r.choice([1, 2, 4])):
            nd = newbuf(ctx, 0x6b)
            ctx.m.uc.mem_write(nd + 4, r.choice([b'\0', b'GUN\0', b'gun\0', b'boom\0']))
            ctx.m.uc.mem_write(nd + 0x14, r.choice([b'\0', b'gun\0', b'x\0']))
            w32(ctx, nd + 0x24, r.choice([0, 0, 1]))
            w32(ctx, nd + 0x30, r.choice([0, 5, 9, 100]))
            nodes.append(nd)
        for a, b in zip(nodes, nodes[1:] + [0]):
            w32(ctx, a + 0x67, b)
        head = nodes[0]
    w32(ctx, 0x1015d528, head)
    rets(0x1000df9c, r.randrange(0x10))
    rets(0x1001fc18, r.choice([-1, 0, 3]))
    return [r.choice([sound_rec(ctx), sound_rec(ctx), 0]), r.choice([sound_rec(ctx), 0]),
            r.choice([-1, -1, 0, 5, 50])]


def g_fc18(ctx):
    r = begin(ctx, 0x1007df6c, 0x100ba7d5, 0x100bb2b0, 0x1000df9c, 0x1000e737)
    w32(ctx, 0x1015d0a4, r.choice([0, 1, 1]))
    w32(ctx, 0x101321c0, r.choice([0, 2, 2, 3]))
    w32(ctx, 0x1014ceb4, cstr(ctx, b'WAV'))
    w32(ctx, 0x101321b8, ctx.u32())
    w32(ctx, 0x1015dc78, ctx.u32())
    slot = newbuf(ctx, 0x6b)
    a = r.choice([b'\0', b'gun\0', b'gun\0'])
    b = r.choice([b'\0', b'boom\0'])
    if a == b'\0' and r.random() < 0.7:
        b = b'\0'           # (only b named: the original reads through a null pointer, both fault)
    ctx.m.uc.mem_write(slot + 4, a)
    ctx.m.uc.mem_write(slot + 0x14, b)
    ctx.m.uc.mem_write(slot + 0x34, r.choice([b'\0', b'text\0']))
    w32(ctx, slot + 0x24, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, 1, 1]), r.choice([0, 1, 1]))
    rets(0x1000df9c, r.randrange(0x10))
    rets(0x1000e737, r.choice([-1, 0, 3]))
    return [slot]


SPECS += [
    dict(name='Sim_10011093', orig=0x10011093, c='Sim_10011093', gen=g_1093, trials=600, void=True),
    dict(name='Sim_10011681', orig=0x10011681, c='Sim_10011681', gen=g_1681, trials=1000, void=True),
    dict(name='Sim_100117f8', orig=0x100117f8, c='Sim_100117f8', gen=g_17f8, trials=1000),
    dict(name='Sim_100123bf', orig=0x100123bf, c='Sim_100123bf', gen=g_23bf, trials=1000, void=True),
    dict(name='Sim_1001305e', orig=TRAMP + 0x100, c='Test_1001305e', gen=g_305e, trials=1000),
    dict(name='Sim_10013297', orig=TRAMP + 0x200, c='Test_10013297', gen=g_3297, trials=1000),
    dict(name='Sim_10013372', orig=TRAMP + 0x600, c='Test_10013372',
         gen=lambda c: g_3297(c, 0x10013372, 6), trials=1000),
    dict(name='Sim_1001349a', orig=0x1001349a, c='Sim_1001349a', gen=g_349a, trials=300, void=True),
    dict(name='Sim_100135d4', orig=0x100135d4, c='Sim_100135d4', gen=g_35d4, trials=1500),
    dict(name='Sim_10013848', orig=TRAMP + 0x300, c='Test_10013848', gen=g_3848, trials=600),
    dict(name='Sim_1001412d', orig=TRAMP + 0x400, c='Test_1001412d', gen=g_412d, trials=1500),
    dict(name='Sim_1001457f', orig=0x1001457f, c='Sim_1001457f', gen=g_457f, trials=1000, void=True),
    dict(name='Sim_10015630', orig=0x10015630, c='Sim_10015630', gen=g_5630, trials=600, void=True),
    dict(name='Sim_10015d30', orig=TRAMP + 0x500, c='Test_10015d30', gen=g_5d30, trials=1000),
    dict(name='Anim_WriteText', orig=0x1001b36c, c='Anim_WriteText', gen=g_b36c, trials=400, void=True),
    dict(name='Sim_1001db6f', orig=0x1001db6f, c='Sim_1001db6f', gen=g_db6f, trials=100, void=True),
    dict(name='Sim_1001f790', orig=0x1001f790, c='Sim_1001f790', gen=g_f790, trials=1500),
    dict(name='Sim_1001fc18', orig=0x1001fc18, c='Sim_1001fc18', gen=g_fc18, trials=800),
]


# ---- generators: 0x10020256 .. 0x100273a1 -------------------------------------------------------------
def e_res(uc, n, raw, eax):
    """0x100278c0: fills the 0x20-byte record out and returns it, or returns 0 (not found)."""
    if not ST['found']:
        return 0
    uc.mem_write(raw[1], struct.pack('<8I', *[mix(ST['seed'], n, k) for k in range(8)]))
    return raw[1]


EFFECT.update({0x10084c95: e_floats(0, 1, 2), 0x100b53f0: e_floats(1, 2, 3), 0x100278c0: e_res})


def g_0256(ctx):
    r = begin(ctx, 0x100200d1, 0x1001f790)
    return [r.choice(list(range(-3, 15)) + [ctx.u32()]), r.choice([-1, 0, 1, 2, 5, ctx.u32()])]


def g_1d65(ctx):
    r = begin(ctx, 0x100232d7, 0x100225c7, 0x10021e8c)
    w32(ctx, 0x1017694c, r.choice([0, newbuf(ctx, 8)]))
    w32(ctx, 0x1015dc80, rint(ctx, -10, 200))
    w32(ctx, 0x1017b92c, r.choice([0, 0, 1]))
    for base, count in ((0x1096fc80, 0x200), (0x109a3f80, 0x14)):
        for _ in range(r.choice([0, 3, 10])):
            e = base + 0x156 * r.randrange(count)
            w32(ctx, e + 0x30, ctx.u32(), ctx.u32())
            w32(ctx, e + 0x3c, r.choice([0, 1, ctx.u32()]))
            w32(ctx, e + 0x48, r.choice([0, 1, 1, ctx.u32()]), r.choice([0, 3, 1, ctx.u32()]))
    return []


def g_411e(ctx):
    r = begin(ctx, 0x10084c95)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x18, rfl(ctx, 100))
    w32(ctx, obj + 0x20, rfl(ctx, 100))
    wf(ctx, obj + 0x38, r.choice([r.uniform(-1000, 1000), r.uniform(-360, 360), 359.9, 0.0]))
    w32(ctx, 0x1015dc80, rint(ctx, -10, 200))
    ST['out'] = [rfl(ctx, 10) for _ in range(3)]
    bufs = [ctx.buf(4, struct.pack('<I', rfl(ctx, 1000))) for _ in range(3)]
    return [obj] + bufs


def g_41ed(ctx):
    r = begin(ctx, 0x100b53f0, 0x10084a94, 0x10084c95, 0x100aa0da)
    recs = ptrtab(ctx, 4)
    for rec in recs:
        w16(ctx, rec + 0x1c, r.choice([0, 0, 2, 4, 8]))
        wf(ctx, rec + 0x58, r.uniform(-5000, 5000), r.uniform(-5000, 5000), r.uniform(-5000, 5000))
    for i in range(4):
        e = 0x109e9ba0 + 0x50 * i
        w16(ctx, e, r.choice([0, 0, 4, 1]))
        w32(ctx, e + 8, ctx.u32())
    obj = newbuf(ctx, 0x90)
    w32(ctx, obj + 0x40, r.choice([-1, 0, 1, 2, 3]))
    w32(ctx, obj + 0x44, r.choice([0x200, 0x400, 0x400, 0x200, 0]))
    w32(ctx, obj + 4, ctx.u32())
    w32(ctx, obj + 0x7e, r.choice([0, 1, 0x20, 0x21, ctx.u32()]))
    for off in (0x18, 0x1c, 0x20, 0x24, 0x28, 0x2c):
        w32(ctx, obj + off, rfl(ctx, 100))
    w32(ctx, 0x1015dc80, rint(ctx, -10, 200))
    ST['out'] = [fbits(r.uniform(-1000, 1000)), fbits(r.uniform(-1, 1)), fbits(r.uniform(-1000, 1000))]
    fvals(0x10084a94, r.choice([r.uniform(0, 5000), 100.0, 3000.0, 50.0, 0.0]))
    p, q, rr = [ctx.buf(4, struct.pack('<I', rfl(ctx))) for _ in range(3)]
    return [obj, fbits(r.uniform(-5000, 5000)), fbits(r.uniform(-5000, 5000)), fbits(r.uniform(-5000, 5000)),
            p, q, rr]


def g_528f(ctx):
    r = begin(ctx, 0x100a9cfb, 0x100a9d47, 0x100d3510, 0x100255de, 0x100257ec, 0x100d3a67, 0x100aa7e7,
              0x100aa8cb, 0x100aa351, 0x1000711c, 0x1000716a, 0x10021be0)
    w32(ctx, 0x1015dc80, rint(ctx, 0, 100))
    flags = newbuf(ctx, 8)
    w32(ctx, 0x1017b8c4, flags)
    w8(ctx, flags + 2, r.choice([0, 1]))
    cam = newbuf(ctx, 0x40)
    w32(ctx, 0x1014cae0, cam)
    wf(ctx, 0x109f14ec, r.choice([r.uniform(-70000, 70000), 12.7, -3.5]))
    for a in (0x109f14e0, 0x109f14e4, 0x109f14e8, 0x109f14f0, 0x109f14f4):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x1015dc04, r.choice([0, 1]))
    picked = []
    for _ in range(r.choice([1, 3, 6])):
        i = r.randrange(0x100)
        picked.append(i)
        e = 0x109a5a40 + 0x2c * i
        kind = r.choice([0x16, 0x2c, 0x2d, 5, 0x16, 0x2c])
        d = 0x10147b80 + 0x3c * kind
        w32(ctx, d, rint(ctx, 0, 300))
        w32(ctx, d + 0x18, r.choice([0, 1, 1]))
        w32(ctx, d + 0x20, rfl(ctx), rfl(ctx))
        w32(ctx, d + 0x28, r.choice([0, 1]))
        obj = 0
        if r.random() < 0.8 or kind not in (0x16, 0x2c):     # the others read obj[0x8c]
            obj = newbuf(ctx, 0x90)
            sub = newbuf(ctx, 0x60)
            w32(ctx, obj + 0x8c, sub)
        w32(ctx, e, obj)
        w32(ctx, e + 4, r.choice([0, ctx.u32()]))
        w32(ctx, e + 8, rfl(ctx), rfl(ctx), rfl(ctx))
        w32(ctx, e + 0x14, r.choice([rint(ctx, -100, 300), 0, 5]))
        w32(ctx, e + 0x18, r.choice([0, 1, 1, 1]))
        w32(ctx, e + 0x1c, r.choice([0, 5, -1, ctx.u32()]))
        w32(ctx, e + 0x20, r.choice([0, 1]))
        w32(ctx, e + 0x24, kind)
        w32(ctx, e + 0x28, ctx.u32())
    w32(ctx, 0x10134888, r.choice(picked + [-1]))
    ST['out'] = [rfl(ctx) for _ in range(3)]
    rets(0x100d3a67, r.choice([0, 1]))
    rets(0x100aa7e7, r.choice([0, 1]))
    return []


def g_5aee(ctx):
    r = begin(ctx, 0x10024616, 0x1002597b)
    hit = newbuf(ctx, 0x30)
    idx = r.randrange(16)
    w16(ctx, hit + 0x24, idx)
    w16(ctx, hit + 2, r.choice([0xb0, 0x10, 0xb5, ctx.u32()]))
    e = 0x109e9ba0 + 0x50 * idx
    w16(ctx, e, r.choice([0, 0, 4]))
    w32(ctx, e + 2, r.choice([0, 1, 2, 3, ctx.u32()]) & ~4 | r.choice([0, 0, 4]))
    w32(ctx, e + 0xc, r.choice([0, 10, 100, rint(ctx, -5, 500)]))
    w32(ctx, e + 0x4c, ctx.u32())
    dmg = r.choice([r.uniform(-10, 200), 9.99, 100.0, -0.5])
    return [ctx.u32(), r.choice([hit, hit, hit, 0]), fbits(dmg), ctx.u32(), ctx.u32(), ctx.u32()]


def g_72cb(ctx):
    r = begin(ctx, 0x100278c0, 0x100273a1, 0x10027654, 0x10027a8f, 0x10053132, 0x100db351)
    w32(ctx, 0x1015965c, r.choice([1, 1, 1, 2]))
    ST['found'] = r.choice([0, 1, 1])
    rets(0x100273a1, r.choice([0, 1]))
    rets(0x10027654, ctx.u32())
    name = r.choice([b'12abc', b'abc', b'7', b'x', b'', b'longname_over_12chars', b'-5', b'0099'])
    return [cstr(ctx, name)]


def g_73a1(ctx):
    r = begin(ctx, 0x100279f1, 0x100bc280, 0x100530a9, 0x10053010, 0x10027b69, 0x100da098, 0x100dafa8,
              0x100db259)
    tags = [0x48454144, 0x434e544c, 0x4f424a53, 0x4d4f444c, 0x54455854]
    for a, t in zip((0x101349ec, 0x101349f0, 0x10134a14, 0x10134ab0, 0x10134ab8), tags):
        w32(ctx, a, t)
    w32(ctx, 0x101348b0, r.choice([0x01000000, 0x02000000, 0x00010000]))
    recs = []
    hdr = newbuf(ctx, 0x20)
    w32(ctx, hdr, r.choice([tags[0], tags[0], tags[0], 7]))
    w32(ctx, hdr + 8, r.choice([0x01000000, 0x02000000, 0x00020000, 0]))
    recs.append(r.choice([hdr, hdr, hdr, 0]))
    cnt = newbuf(ctx, 0x20)
    w32(ctx, cnt, r.choice([tags[1], tags[1], tags[1], 7]))
    recs.append(r.choice([cnt, cnt, cnt, 0]))
    for _ in range(r.choice([0, 1, 3, 5])):
        x = newbuf(ctx, 0x20)
        w32(ctx, x, r.choice(tags + [9]))
        recs.append(x)
    seq(0x100279f1, recs, 0)
    w32(ctx, 0x1015dc28, r.choice([0, 1]))
    for a in range(0x109f1448, 0x109f145c, 4):
        w32(ctx, a, ctx.u32())
    rets(0x100530a9, r.choice([0, 1]))
    for a in (0x100da098, 0x100dafa8, 0x100db259):
        rets(a, r.choice([1, 1, 0]), r.choice([1, 1, 0]))
    return [newbuf(ctx, 0x40)]


SPECS += [
    dict(name='Sim_10020256', orig=0x10020256, c='Sim_10020256', gen=g_0256, trials=500, void=True),
    dict(name='Sim_10021d65', orig=0x10021d65, c='Sim_10021d65', gen=g_1d65, trials=300, void=True),
    dict(name='Sim_1002411e', orig=0x1002411e, c='Sim_1002411e', gen=g_411e, trials=800, void=True),
    dict(name='Sim_100241ed', orig=0x100241ed, c='Sim_100241ed', gen=g_41ed, trials=1500, void=True),
    dict(name='Sim_1002528f', orig=0x1002528f, c='Sim_1002528f', gen=g_528f, trials=600, void=True),
    dict(name='Sim_10025aee', orig=0x10025aee, c='Sim_10025aee', gen=g_5aee, trials=800, void=True),
    dict(name='Sim_100272cb', orig=0x100272cb, c='Sim_100272cb', gen=g_72cb, trials=500),
    dict(name='Sim_100273a1', orig=0x100273a1, c='Sim_100273a1', gen=g_73a1, trials=1000),
]
