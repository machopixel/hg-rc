# Differential tests for round 9, letter h (see difftest.py and docs/checker.md).
#
# Helpers of a tested function are replaced in the emulator by stubs while that function is tested (switched on
# per trial by begin()), as in spec_w9g.py / spec_w8t.py: a code hook at the helper's address appends
# ('stub_<address>', arguments) to the checker's call log (compared between the original and the C run like the
# Windows calls), may write results through pointer arguments (the same bytes for both runs) and returns to the
# caller with eax from the trial's list. Helpers that are not converted or hooked elsewhere (called with SIM_FN by
# the C code) are stubbed at their own address; converted helpers (called by name) at their original address
# (reached by the original code) and at the address of their C function in the test image (reached by the C
# code), under the same log name. Stack addresses are logged as 'stack'; texts by content (SARGS); arguments the
# helper reads as 16 bits are masked (MASK: the original pushes them with a stale upper half); LOGFN adds the
# contents a pointer argument points at (the quad of Sim_100352d5, whose unset fields are stack garbage and left
# out).
# Run for real on both sides: Sim_10052aaf, Sim_10052be7, Sim_10052c02, Sim_100b5386, Str_Length, Mem_Move,
# Crt_StrChr and the converted sprintf (Crt_SPrintf; its formatter 0x10100fd0 is the original code in the
# emulator, only %d and %s conversions are used).
# Floating point: Sim_10032a10 and Sim_100e1398 truncate x87 results to int (_ftol, written as Render_Ftol); the
# only outputs are ints, so they are compared exactly (no fp/tol).
import atexit
import os
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, \
    UC_X86_REG_FPTAG

MINE = [
    'sim/unsorted_1001/w9h_group.c', 'sim/unsorted_1002/w9h_misc.c', 'sim/unsorted_1003/w9h_view.c',
    'sim/unsorted_1004/w9h_misc.c', 'sim/unsorted_1005/w9h_misc.c', 'sim/unsorted_1008/w9h_keys.c',
    'sim/unsorted_1009/w9h_misc.c', 'sim/unsorted_100b/w9h_text.c', 'sim/unsorted_100e/w9h_misc.c',
]
# existing converted code the functions above call (and what that calls): the closure spec_w9g.py compiles
# plus the three files of Sim_10012843, Sim_1003327f and Cache_DumpLog
OTHERS = [
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/core/w2b_timers.c', 'sim/render/w2n_polygon.c',
    'sim/render/w2n_project.c', 'sim/render/w3p_view.c', 'sim/render/w3q_texture.c', 'sim/render/w3q_vertex.c',
    'sim/render/w3r_draw.c', 'sim/render/w7o_draw.c', 'sim/unsorted_1000/w2a_fields.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2h_access.c', 'sim/unsorted_1000/w2h_reset.c',
    'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1000/w2j_misc.c', 'sim/unsorted_1000/w2j_slots.c',
    'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w3q_misc.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w3t_records.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1000/w5c_calls.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1000/w5y_misc.c',
    'sim/unsorted_1000/w5z_calls.c', 'sim/unsorted_1000/w6d_calls.c', 'sim/unsorted_1000/w6d_select.c',
    'sim/unsorted_1000/w7l_misc.c', 'sim/unsorted_1000/w8t_mono.c', 'sim/unsorted_1000/w8t_sight.c',
    'sim/unsorted_1000/w8t_sound.c', 'sim/unsorted_1001/w2a_records.c', 'sim/unsorted_1001/w2e_mono.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2f_misc.c',
    'sim/unsorted_1001/w2g_fields.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w2h_tables.c',
    'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w2k_misc.c', 'sim/unsorted_1001/w2l_misc.c',
    'sim/unsorted_1001/w3t_loader.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w5a_misc.c', 'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1001/w5c_calls.c',
    'sim/unsorted_1001/w5c_math.c', 'sim/unsorted_1001/w5x_calls.c', 'sim/unsorted_1001/w5y_misc.c',
    'sim/unsorted_1001/w6d_calls.c', 'sim/unsorted_1001/w6d_slots.c', 'sim/unsorted_1001/w7l_misc.c',
    'sim/unsorted_1001/w8t_actions.c', 'sim/unsorted_1001/w8t_misc.c', 'sim/unsorted_1001/w8t_queue.c',
    'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_1002/w2f_misc.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2i_misc.c',
    'sim/unsorted_1002/w2i_setup.c', 'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1002/w2j_nodes.c',
    'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w3o_ray.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5x_calls.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w6d_calls.c',
    'sim/unsorted_1002/w7l_collide.c', 'sim/unsorted_1002/w7l_misc.c', 'sim/unsorted_1002/w8t_misc.c',
    'sim/unsorted_1003/bitmap.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_1003/records.c',
    'sim/unsorted_1003/scramble.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/slots.c',
    'sim/unsorted_1003/w2b_misc.c', 'sim/unsorted_1003/w2e_entries.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2f_misc.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_1003/w2k_misc.c', 'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w3p_net.c',
    'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/w3s_entry.c', 'sim/unsorted_1003/w4u_heap.c',
    'sim/unsorted_1003/w4v_misc.c', 'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c',
    'sim/unsorted_1003/w6e_misc.c', 'sim/unsorted_1003/w7m_state.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1004/state.c', 'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2e_strings.c',
    'sim/unsorted_1004/w2h_misc.c', 'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1004/w2i_misc.c',
    'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/w2j_strtab.c', 'sim/unsorted_1004/w2k_flags.c',
    'sim/unsorted_1004/w2k_hash.c', 'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_1004/w4u_heap.c',
    'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/w5a_misc.c', 'sim/unsorted_1004/w5b_calls.c',
    'sim/unsorted_1004/w5c_calls.c', 'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1004/w5z_calls.c',
    'sim/unsorted_1004/w6e_misc.c', 'sim/unsorted_1004/w6e_stream.c', 'sim/unsorted_1004/w7m_gif.c',
    'sim/unsorted_1004/w8v_files.c', 'sim/unsorted_1004/w8v_misc.c', 'sim/unsorted_1004/w8v_shots.c',
    'sim/unsorted_1005/w2e_lists.c', 'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c',
    'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1005/w2k_misc.c', 'sim/unsorted_1005/w3o_calls.c',
    'sim/unsorted_1005/w3q_misc.c', 'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1005/w5b_calls.c',
    'sim/unsorted_1005/w5y_misc.c', 'sim/unsorted_1005/w7m_misc.c', 'sim/unsorted_1005/w7n_input.c',
    'sim/unsorted_1005/w8v_hud.c', 'sim/unsorted_1005/w8v_ini.c', 'sim/unsorted_1005/w8v_input.c',
    'sim/unsorted_1005/w8v_misc.c', 'sim/unsorted_1006/w2f_records.c', 'sim/unsorted_1006/w3o_add.c',
    'sim/unsorted_1006/w3p_box.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1006/w3t_misc.c',
    'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c',
    'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1006/w5z_calls.c', 'sim/unsorted_1006/w6e_misc.c',
    'sim/unsorted_1006/w7n_misc.c', 'sim/unsorted_1006/w8v_misc.c', 'sim/unsorted_1006/w8v_quat.c',
    'sim/unsorted_1006/w8w_event.c', 'sim/unsorted_1006/w8w_hud.c', 'sim/unsorted_1006/w8w_misc.c',
    'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1007/w2j_misc.c', 'sim/unsorted_1007/w2l_fields.c',
    'sim/unsorted_1007/w3o_load.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1007/w6e_misc.c', 'sim/unsorted_1007/w7n_misc.c', 'sim/unsorted_1007/w8w_misc.c',
    'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_1008/w2i_rect.c',
    'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/w3q_place.c', 'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w5b_calls.c',
    'sim/unsorted_1008/w5x_calls.c', 'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w6f_cells.c',
    'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w7n_misc.c', 'sim/unsorted_1008/w7o_cells.c',
    'sim/unsorted_1008/w8w_math.c', 'sim/unsorted_1008/w8w_nodes.c', 'sim/unsorted_1009/w2e_copy.c',
    'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_1009/w2l_pool.c',
    'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1009/w3p_box.c', 'sim/unsorted_1009/w3q_misc.c',
    'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_100a/w2g_misc.c',
    'sim/unsorted_100a/w2h_misc.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w2l_small.c', 'sim/unsorted_100a/w3o_misc.c', 'sim/unsorted_100a/w3t_misc.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100a/w7o_misc.c',
    'sim/unsorted_100a/w7o_nodes.c', 'sim/unsorted_100a/w7p_files.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_100b/w2e_object.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/unsorted_100b/w2f_tables.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100b/w2l_records.c',
    'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c',
    'sim/unsorted_100b/w4w_buffers.c', 'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_100b/w5z_calls.c',
    'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/entries.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w2f_objects.c', 'sim/unsorted_100c/w2h_misc.c', 'sim/unsorted_100c/w2i_misc.c',
    'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100c/w3p_misc.c',
    'sim/unsorted_100c/w3s_entries.c', 'sim/unsorted_100c/w3t_misc.c', 'sim/unsorted_100c/w4v_timers.c',
    'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_100c/w5b_calls.c', 'sim/unsorted_100c/w5c_calls.c',
    'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w5x_calls.c', 'sim/unsorted_100c/w5z_calls.c',
    'sim/unsorted_100c/w6h_calls.c', 'sim/unsorted_100c/w7p_fall.c', 'sim/unsorted_100c/w7p_misc.c',
    'sim/unsorted_100c/w7p_states.c', 'sim/unsorted_100c/w7q_relative.c', 'sim/unsorted_100c/w8z_misc.c',
    'sim/unsorted_100c/w8z_targets.c', 'sim/unsorted_100c/w8z_units.c', 'sim/unsorted_100d/nodes.c',
    'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w2d_state.c', 'sim/unsorted_100d/w2e_names.c',
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2g_misc.c', 'sim/unsorted_100d/w2h_misc.c',
    'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_100d/w3q_misc.c', 'sim/unsorted_100d/w4v_misc.c', 'sim/unsorted_100d/w5b_calls.c',
    'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_100d/w5c_math.c', 'sim/unsorted_100d/w5z_calls.c',
    'sim/unsorted_100d/w6h_calls.c', 'sim/unsorted_100d/w7q_names.c', 'sim/unsorted_100d/w8z_effects.c',
    'sim/unsorted_100d/w8z_lists.c', 'sim/unsorted_100d/w8z_palette.c', 'sim/unsorted_100d/w8z_project.c',
    'sim/unsorted_100e/w3p_misc.c', 'sim/unsorted_100e/w8z_load.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_100f/w7q_parse.c',
    'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_100f/w7r_ctype.c', 'sim/unsorted_100f/w8z_crt.c',
    'sim/unsorted_1010/w2f_ld12.c', 'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2g_crt.c',
    'sim/unsorted_1010/w2h_misc.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2k_crt.c',
    'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1010/w6j_stream.c',
    'sim/unsorted_1010/w6k_handles.c', 'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w6k_stream.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1010/w7s_locale.c', 'sim/unsorted_1010/w7s_stdio.c',
    'sim/util/field.c', 'sim/util/lookup.c', 'sim/util/record.c', 'sim/util/ring.c', 'sim/view/matrix.c',
    'sim/view/objlist.c', 'sim/view/viewrect.c', 'sim/view/w2a_points.c', 'sim/view/w2d_view.c',
    'sim/view/w2l_lists.c', 'sim/view/w2l_rects.c', 'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c',
    'sim/win/w4u_cdaudio.c', 'sim/win/w4u_gdi.c', 'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
    'sim/unsorted_1006/w2l_slots.c', 'sim/unsorted_100c/w7q_objects.c',
    'sim/unsorted_1001/w3s_group.c', 'sim/unsorted_1003/flags.c', 'sim/unsorted_1002/w8u_cache.c',
    'sim/unsorted_1001/w5z_calls.c', 'sim/unsorted_1005/w6e_misc.c', 'sim/unsorted_1005/w3r_misc.c',
    'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_1005/w5a_misc.c',
]
SOURCES = MINE + OTHERS
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
DRAW = 0x00F00100           # the display driver's polygon routine (slot 0x1014cb4c): a stub in the return page

# Stubbed helpers: original address -> (C function name or None, number of dword arguments logged).
HELPERS = {}


def helper(addr, cname, nargs):
    HELPERS[addr] = (cname, nargs)


# not converted or hooked elsewhere (SIM_FN in the C), and the driver slot
for _a, _n in [(0x100591ff, 4), (0x10058c5e, 4), (0x100d2306, 1), (0x100d1f90, 1), (0x100a0def, 1), (DRAW, 3)]:
    helper(_a, None, _n)
# converted (called by name)
for _a, _c, _n in [
        (0x10016d83, 'Sim_10016d83', 2), (0x10016cb7, 'Sim_10016cb7', 3), (0x100152fc, 'Sim_100152fc', 3),
        (0x100c3668, 'Sim_100c3668', 1), (0x10016a6b, 'Sim_10016a6b', 4), (0x10016a4a, 'Sim_10016a4a', 1),
        (0x10025f6b, 'Sim_10025f6b', 1), (0x1001555c, 'Sim_1001555c', 3),
        (0x100aa351, 'Sim_100aa351', 1), (0x100aa406, 'Sim_100aa406', 1), (0x100aa799, 'Sim_100aa799', 1),
        (0x1009c929, 'Sim_1009c929', 1),
        (0x100d255e, 'Sim_100d255e', 1), (0x100a4c45, 'Sim_100a4c45', 0), (0x100071f6, 'Sim_100071f6', 0),
        (0x100d32e5, 'Sim_100d32e5', 1),
        (0x10010f70, 'Sim_10010f70', 0), (0x10012843, 'Sim_10012843', 1), (0x1003db56, 'Sim_1003db56', 1),
        (0x100a9d47, 'Sim_100a9d47', 4), (0x100aa0da, 'Sim_100aa0da', 5), (0x100529a2, 'Sim_100529a2', 4),
        (0x100aa863, 'Sim_100aa863', 1),
        (0x100448cd, 'Sim_100448cd', 1), (0x10044a69, 'Sim_10044a69', 2), (0x1007df6c, 'Sim_1007df6c', 4),
        (0x1003327f, 'Sim_1003327f', 1), (0x1002812f, 'Cache_DumpLog', 0),
        (0x100fd9a0, 'Crt_IsCType', 2), (0x100fd710, 'Sim_100fd710', 3),
        (0x100fef30, 'Heap_Malloc', 1), (0x100fefe0, 'Heap_FreeBlock', 1),
        (0x1005b98c, 'Sim_1005b98c', 1)]:
    helper(_a, _c, _n)

M16 = 0xffff
# 16-bit arguments: Sim_100152fc's b, Sim_10016a6b's slot/sel/other, Sim_1001555c's a/b, Sim_1003db56's key
MASK = {(0x100152fc, 1): M16, (0x10016a6b, 1): M16, (0x10016a6b, 2): M16, (0x10016a6b, 3): M16,
        (0x1001555c, 1): M16, (0x1001555c, 2): M16, (0x1003db56, 0): M16}
SARGS = {0x1007df6c: (0, 1), 0x100fd710: (0, 1)}
DEREF = {}
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax) that writes the helper's results
LOGFN = {}          # helper -> function(uc, raw arguments) -> extra data appended to the log entry
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def ustr(uc, addr, limit=400):
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


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def rint(ctx, lo, hi):
    return ctx.rng.randint(lo, hi) & 0xffffffff


TEXTCHARS = b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _.,!?-'


def text(ctx, n, alphabet=TEXTCHARS):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


# ---- stubs --------------------------------------------------------------------------------------
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
    if addr in LOGFN:
        args.append(LOGFN[addr](uc, raw))
    return raw, args


def _stub_cb(m, addr):
    cname, nargs = HELPERS[addr]

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
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


# Code ranges of the original functions (start, end of the code before the jump tables / padding), for the
# coverage count switched on with W9H_COV=1 (or the file tools/round9/w9h_cov.flag, seen from the repo root, where
# the checker runs): every executed instruction
# of the ORIGINAL is noted and, at the end, the executed share of each function's code bytes is printed (a rough
# measure of the branches reached).
RANGES = {0x10017263: 0x10017852, 0x10029b1f: 0x10029b4e, 0x10032a10: 0x1003324b, 0x100352d5: 0x10035444,
          0x10042a11: 0x10042b10, 0x1004d344: 0x1004d358, 0x100527fa: 0x100529a2, 0x1008b922: 0x1008c0f7,
          0x1009bd47: 0x1009bd60, 0x100b6f6a: 0x100b737b, 0x100b7430: 0x100b7722, 0x100e1398: 0x100e14f0,
          0x100e6b29: 0x100e6b49}
COV = {}


def _cov_cb(uc, address, size, ud):
    for lo, hi in RANGES.items():
        if lo <= address < hi:
            COV.setdefault(lo, set()).update(range(address, address + size))
            break


def _cov_report():
    for lo in sorted(COV):
        hi = RANGES[lo]
        missing = sorted(set(range(lo, hi)) - COV[lo])
        gaps = []
        for a in missing:
            if gaps and gaps[-1][1] == a:
                gaps[-1][1] = a + 1
            else:
                gaps.append([a, a + 1])
        print('  coverage %x: %d of %d code bytes executed%s' % (
            lo, len(COV[lo]), hi - lo, '' if not gaps else ', not reached: ' + ' '.join('%x-%x' % (a, b) for a, b in gaps)))


def begin(ctx, *active):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and switches on the
    stubs of the helpers given (original addresses) for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w9h', False):
        m._w9h = True
        for addr, (cname, nargs) in sorted(HELPERS.items()):
            cb = _stub_cb(m, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            if cname is not None:
                m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
        if os.environ.get('W9H_COV') or os.path.exists(os.path.join('tools', 'round9', 'w9h_cov.flag')):
            for lo, hi in RANGES.items():
                m.raw.hook_add(UC_HOOK_CODE, _cov_cb, None, lo, hi - 1)
            atexit.register(_cov_report)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    for a in active:
        assert a in HELPERS, hex(a)
    ST['count'] = {}
    ST['seq'] = {}
    ST['seq_end'] = {}
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    ST.update(members=[], fifth=None, shades=[], nverts=0, pts=[], blocks=[])
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def ptrtab(ctx, count, size=0x300):
    """count records in the pointer table 0x109e8b40, count in [0x1015dc64]."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        recs.append(rec)
    w32(ctx, 0x1015dc64, count)
    return recs


# ---- effects of stubbed helpers -----------------------------------------------------------------------
def put32(uc, addr, v):
    uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def e_members(uc, n, raw, eax):
    """Sim_10016cb7: writes the trial's member pointers to out[] (and the fifth one to out[4]), returns the
    number."""
    for i, p in enumerate(ST['members']):
        put32(uc, raw[1] + 4 * i, p)
    if ST['fifth'] is not None:
        put32(uc, raw[1] + 16, ST['fifth'])
    return len(ST['members'])


def e_shade(uc, n, raw, eax):
    """0x10058c5e: writes the trial's shade into +0x3c of every vertex of the list (at least the first one, which
    the caller always reads)."""
    for i in range(max(ST['nverts'], 1)):
        v = ur32(uc, raw[2] + 4 * i)
        put32(uc, v + 0x3c, ST['shades'][i])


def e_point(uc, n, raw, eax):
    """Sim_100529a2: the trial's n-th point into *x, *y, *z."""
    x, y, z = ST['pts'][n % len(ST['pts'])]
    put32(uc, raw[1], x)
    put32(uc, raw[2], y)
    put32(uc, raw[3], z)
    return 1


# Character classes of the C runtime's table: 4 digit, 0x80 hex digit, 1 upper, 2 lower, 8 space, 0x10 punct.
def ctype_class(c):
    if 0x30 <= c <= 0x39:
        return 0x84
    if 0x41 <= c <= 0x5a:
        return 0x81 if c <= 0x46 else 1
    if 0x61 <= c <= 0x7a:
        return 0x82 if c <= 0x66 else 2
    if c == 0x20 or 9 <= c <= 13:
        return 8
    if 0x21 <= c <= 0x7e:
        return 0x10
    return 0


def e_isctype(uc, n, raw, eax):
    c = raw[0]
    return (ctype_class(c) & raw[1]) if c < 0x100 else 0


def e_sscanf(uc, n, raw, eax):
    """Sim_100fd710 (sscanf) with "%o" or "%x": the longest valid prefix into *out; 1 when there was one."""
    s = ustr(uc, raw[0])
    fmt = ustr(uc, raw[1])
    digits = b'01234567' if fmt == b'%o' else b'0123456789abcdefABCDEF'
    k = 0
    while k < len(s) and s[k:k + 1] in digits and s[k:k + 1] != b'':
        k += 1
    if k == 0:
        return 0
    put32(uc, raw[2], int(s[:k], 8 if fmt == b'%o' else 16))
    return 1


def e_malloc(uc, n, raw, eax):
    blocks = ST['blocks']
    return blocks[n] if n < len(blocks) else 0


EFFECT.update({0x10016cb7: e_members, 0x10058c5e: e_shade, 0x100529a2: e_point, 0x100fd9a0: e_isctype,
               0x100fd710: e_sscanf, 0x100fef30: e_malloc})

# the fields of the 4 x 0x24-byte quad that Sim_100352d5 sets (the rest is stack garbage in the original)
QUAD_OFFS = [0x00, 0x04, 0x10, 0x18, 0x1c, 0x20, 0x24, 0x28, 0x34, 0x3c, 0x40, 0x44, 0x48, 0x4c, 0x58, 0x60,
             0x64, 0x68, 0x6c, 0x70, 0x7c, 0x84, 0x88, 0x8c]


def log_quad(uc, raw):
    data = bytes(uc.mem_read(raw[1], 0x90))
    return tuple(struct.unpack_from('<I', data, off)[0] for off in QUAD_OFFS)


LOGFN[DRAW] = log_quad


# ---- generators -------------------------------------------------------------------------------------
STAR_BASE = 0x10473800
STAR_SIZE = 0x4fc3a
ENTRY = 0x13f


def entry(star, i):
    return STAR_BASE + star * STAR_SIZE + 0x3a + i * ENTRY


def g_7263(ctx):
    """Sim_10017263: 8 unit records, the group table, the selected entry with its word list; every flag value
    the function tests, kinds that match and do not, leaders that are the player or not."""
    r = begin(ctx, 0x10016d83, 0x10016cb7, 0x100152fc, 0x100c3668, 0x10016a6b, 0x10016a4a, 0x10025f6b,
              0x1001555c)
    index = r.choice([r.randrange(0, 8)] * 3 + [r.randrange(8, 16)])
    n_units = 8
    units = ptrtab(ctx, n_units, 0x300)
    words = []
    for u in units:
        w16(ctx, u + 0x1a0, r.choice([0, 0, 0, 1, 7, 0x10, r.getrandbits(16)]))
        w32(ctx, u + 0x14, r.choice([0, 1, 2, 2, 3, 3, r.randrange(0, 10)]))
        wd = r.choice([r.randrange(1, 4)] * 4 + [0, r.getrandbits(16)])
        words.append(wd)
        w16(ctx, u + 0x1aa, wd)
        w16(ctx, u + 0x1a6, r.choice([0, 0, 2, 2, 3, 4, 5, 7, 8, 8, 1, 6, r.getrandbits(16)]))
        w32(ctx, u + 0x28, r.choice([0, newbuf(ctx, 0x20)]))
        w32(ctx, u + 0x1ae, ctx.u32())
    for g in range(16):
        w32(ctx, 0x109b7d40 + g * 0x6c, r.randrange(0, n_units))
    idx = r.choice([-1, r.randrange(0, n_units), r.randrange(0, n_units), r.randrange(0, n_units)])
    w32(ctx, 0x109b7d40 + index * 0x6c, idx)
    w32(ctx, 0x1017b8c8, r.choice([idx & 0xffffffff, r.randrange(0, n_units), r.randrange(0, n_units),
                                   r.randrange(0, n_units)]))
    sel = r.choice([-1, 0, 0, 1, 2, 3])
    w32(ctx, 0x104727a0 + index * 4, sel)
    e = entry(index, max(sel, 0))
    flags = r.choice([0, 1, 2, 4, 8, 0x20, 0x100, 0x200, 0x400, 0x1000, 0x2000, 0x4000, 0x10, 0x800, 0x8000,
                      0x3, 0x5, 0xc, 0x21, 0x101, 0x1001, 0x10000, ctx.u32()])
    w32(ctx, e + 2, flags)
    cnt = r.choice([0, 1, 1, 2, 3, 5])
    w8(ctx, e + 0xee, cnt)
    for i in range(cnt):
        w16(ctx, e + 0xef + 2 * i, r.choice(words + [0, r.getrandbits(16)]))
    w32(ctx, e + 0x79, r.choice([0, 0, 1, 2, 3, -1]))
    w32(ctx, e + 0x7d, ctx.u32())
    n = r.choice([0, 1, 2, 3, 4, 5, 8])
    ST['members'] = [units[r.randrange(0, n_units)] for _ in range(n)]
    ST['fifth'] = None if r.random() < 0.5 else units[r.randrange(0, n_units)]
    rets(0x100152fc, *[r.choice([0, 0, 0, 1]) for _ in range(6)])
    rets(0x100c3668, *[r.choice([0, 0, 1]) for _ in range(6)])
    rets(0x10016a4a, *[r.choice([0, 1, 0x80, r.getrandbits(8)]) for _ in range(3)])
    return [index]


def g_9b1f(ctx):
    begin(ctx, 0x100aa351, 0x100aa406, 0x100aa799)
    return [ctx.rng.choice([0, newbuf(ctx, 0x40), newbuf(ctx, 0x40)])]


def g_2a10(ctx):
    """Sim_10032a10: every view mode and overlay, every colour kind, the level nibble and the flags around the
    thresholds, vertex lists of 0..6 entries."""
    r = begin(ctx, 0x100591ff, 0x10058c5e, 0x1009c929)
    obj = newbuf(ctx, 0x40)
    rec = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x24, rec)
    level = r.choice([0, 0, 1, 3, 7, 8, 15, r.randrange(0, 16)])
    w0 = (level << 4) | r.choice([0, 1, 2, 4, 0x100, 0x101, 0x102, 0x104, 0x103, r.getrandbits(4),
                                  r.getrandbits(16) & ~0xf0])
    w16(ctx, rec, w0)
    fl = 0
    for b in (0x10, 0x20, 0x40, 0x100, 0x200, 0x400):
        if r.random() < 0.3:
            fl |= b
    if r.random() < 0.1:
        fl = r.getrandbits(16)
    w16(ctx, rec + 2, fl)
    w32(ctx, rec + 0x34, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, ctx.u32()]))
    colour = (r.randrange(0, 16) << 12) | r.getrandbits(12)
    if r.random() < 0.1:
        colour |= r.getrandbits(16) << 16
    w32(ctx, 0x1014cb1c, r.choice([0, 0, 0, 1, 1, 2, 3, 3, 4, 4, 5]))
    w32(ctx, 0x1014cb20, r.choice([0, 0, 0, 1, 2, 3]))
    w32(ctx, 0x1014cb28, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb38, r.choice([0, 0, fl, r.getrandbits(16), 0x100]))
    w32(ctx, 0x10179ca4, r.choice([0, 1]))
    w32(ctx, 0x1015dd70, r.choice([0, 1]))
    nv = r.choice([0, 1, 2, 3, 6])
    w32(ctx, 0x101c9ea8, nv)
    shades = []
    for i in range(max(nv, 1)):
        v = newbuf(ctx, 0x48)
        w32(ctx, 0x101c76a0 + 4 * i, v)
        w32(ctx, v + 0x3c, r.choice([r.randrange(0, 16), r.randrange(0, 32), r.randrange(-40, 100), 0, 15]))
        shades.append(r.choice([r.randrange(0, 16), r.randrange(0, 32), r.randrange(-40, 100), 0, 15, 31]))
    ST['shades'] = shades
    ST['nverts'] = nv
    rets(0x100591ff, *[r.choice([0, 15, r.randrange(0, 16), r.randrange(-20, 300)]) for _ in range(3)])
    rets(0x1009c929, *[r.choice([0, 20, 21, 100, r.randrange(0, 101)]) for _ in range(3)])
    return [obj, ctx.u32(), colour, ctx.u32()]


def g_52d5(ctx):
    r = begin(ctx, 0x100d2306, 0x100d1f90, 0x100a0def, 0x100d255e, 0x100a4c45, 0x100071f6, 0x100d32e5, DRAW)
    w32(ctx, 0x1017b940, r.choice([0, 1, 1, 1, 1]))
    w32(ctx, 0x1013a014, r.choice([0, 1]))
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x80))
    w32(ctx, 0x1014cb18, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb1c, r.choice([0, 0, 1, 3]))
    w32(ctx, 0x1015d1ac, r.choice([0, 4, 4, 2]))
    w32(ctx, 0x1014cb04, r.choice([0, 0, 1]))
    w32(ctx, 0x1014cb08, r.choice([0, 0, 1]))
    w32(ctx, 0x10138c70, ctx.u32())
    w32(ctx, 0x1014cb4c, DRAW)
    return []


def g_2a11(ctx):
    """Sim_10042a11: records of every state, leaders found in the group table, by Sim_10012843 or neither."""
    r = begin(ctx, 0x10010f70, 0x10012843, 0x1003db56)
    n = r.choice([0, 1, 2, 4, 8])
    recs = ptrtab(ctx, max(n, 1), 0x40)
    w32(ctx, 0x1015dc64, n)
    for u in recs:
        w32(ctx, u + 0x14, r.choice([0, 1, 2, 3, 3, 3]))
        w32(ctx, u + 8, r.choice([r.randrange(0, 16), r.randrange(0, 16), r.randrange(16, 40)]))
    w32(ctx, 0x1017b8c8, r.randrange(0, max(n, 1)))
    for g in range(16):
        w32(ctx, 0x109b7d40 + g * 0x6c, r.choice([-1, -1, r.randrange(0, max(n, 1))]))
    w16(ctx, 0x1015ecd8, r.getrandbits(16))
    rets(0x10012843, *[r.choice([-1, r.randrange(0, max(n, 1))]) for _ in range(4)])
    rets(0x1003db56, *[r.choice([-1, r.randrange(0, 40), r.getrandbits(32)]) for _ in range(4)])
    return []


def g_d344(ctx):
    begin(ctx)
    return [ctx.rng.choice([0, 1, 2, ctx.rng.randrange(0, 200)])]


def g_27fa(ctx):
    """Sim_100527fa: group indices below 16 only (above, the original reads an uninitialised local as the
    leader index); leaders inside and outside the record count, members of the group among 12 records."""
    r = begin(ctx, 0x100a9d47, 0x100aa0da, 0x100aa799, 0x100529a2, 0x100aa863)
    index = r.randrange(0, 16)
    recs = ptrtab(ctx, 12, 0x80)
    n = r.choice([0, 1, 3, 6, 10, 12])
    w32(ctx, 0x1015dc64, n)
    for u in recs:
        w32(ctx, u + 8, r.choice([index, index, r.randrange(0, 16)]))
        w32(ctx, u + 0x48, newbuf(ctx, 0x40))
    w32(ctx, 0x109b7d40 + index * 0x6c, r.choice([r.randrange(0, 12), r.randrange(0, max(n, 1)),
                                                   r.randrange(0, max(n, 1))]))
    ST['pts'] = [(ctx.u32(), ctx.u32(), ctx.u32()) for _ in range(14)]
    rets(0x100aa863, *[newbuf(ctx, 0x30) for _ in range(3)])
    return [index, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


KEYS = [0x15, 0x17, 0x18] + list(range(0x79, 0x83)) + list(range(0x83, 0x8b)) + list(range(0x8b, 0x95)) + \
    [0x95, 0x96, 0x97, 0x98, 0x99, 0x9a, 0x9b, 0x9c, 0x9d, 0x9f, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa8, 0xa9, 0xaa,
     0xab, 0xac, 0xae, 0xaf, 0xb1, 0xb2, 0xb3, 0xb4]
TEXTPTRS = [0x101c4fa4, 0x101c4fa8, 0x101c4f98, 0x101c4fa0, 0x101c4fb8, 0x101c4fbc, 0x101c4fac, 0x101c4fb0,
            0x101c4fe0, 0x101c4824, 0x101c482c, 0x101c4828, 0x101c4834, 0x101c4830, 0x101c4838]
TOGGLES = [0x1014b930, 0x1014b934, 0x1012dc98, 0x1015d04c, 0x1015d02c, 0x1015d030, 0x1015d034, 0x1015d038,
           0x1015d040, 0x1015d044, 0x10152414, 0x1015d01c]


def g_b922(ctx):
    """Sim_1008b922: every key the switch handles (0x26, the debug break, left out) and keys it does not, the
    toggles in both states, the mission byte at 3 or not."""
    r = begin(ctx, 0x100448cd, 0x10044a69, 0x1007df6c, 0x1003327f, 0x1002812f)
    key = r.choice(KEYS + [r.randrange(0, 0x100), 0x14, 0x16, 0x9e, 0xa0, 0xa6, 0xa7, 0xad, 0xb0, 0xb5, 0xffff,
                           r.getrandbits(16)])
    if key == 0x26:
        key = 0x27
    if r.random() < 0.3:
        key |= r.getrandbits(16) << 16
    for a in TEXTPTRS:
        w32(ctx, a, cstr(ctx, text(ctx, r.randrange(0, 30))))
    w32(ctx, 0x109b98c0, cstr(ctx, text(ctx, r.randrange(0, 20))))
    for a in TOGGLES:
        w32(ctx, a, r.choice([0, 0, 1, 1, 5, ctx.u32()]))
    w32(ctx, 0x1014cb20, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1014cb1c, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x10152418, r.choice([0, 1, 2, 2]))
    star = r.choice([-1, -1, 0, 1, 2, 3])
    w32(ctx, 0x101303d0, star)
    w32(ctx, 0x104737e0, r.choice([0, 1, 3, 5, 10]))
    row = r.choice([0, 1, 5, r.randrange(0, 40)])
    w32(ctx, 0x1015d108, row)
    if star >= 0:
        w8(ctx, 0x1047383a + star * STAR_SIZE + row * ENTRY, r.choice([3, 3, 0, 5, 6, r.getrandbits(8)]))
    b = newbuf(ctx, 0x20)
    w8(ctx, b + 6, r.choice([0, 1]))
    w32(ctx, 0x109e88e0, b)
    b = newbuf(ctx, 0x20)
    for off in (2, 3, 4):
        w8(ctx, b + off, r.choice([0, 1, r.getrandbits(8)]))
    w32(ctx, 0x1017b8c4, b)
    return [key]


def g_bd47(ctx):
    begin(ctx)
    return []


ESC_ALPHA = b'\\\\\\\\\\\\abfnrtv\'"?x01234567012345678899gzA ' + bytes(range(10, 14)) + b'\x80\xe9\xff'


def ctype_table(ctx):
    """The C runtime's class table, indexed by a signed char: 0x200 bytes, the pointer at the middle."""
    tab = b''.join(struct.pack('<H', ctype_class(c & 0xff)) for c in list(range(-128, 0)) + list(range(0, 128)))
    return ctx.buf(0x200, tab) + 0x100


def g_6f6a(ctx):
    """Text_Unescape: texts dense in backslashes and the characters that follow them (every escape, octal and
    hex digit runs of 0..4, bare \\x, a backslash at the end, line breaks, high bytes), both class sources."""
    r = begin(ctx, 0x100fd9a0, 0x100fd710)
    w32(ctx, 0x10159450, ctype_table(ctx))
    w32(ctx, 0x1015965c, r.choice([0, 1, 1, 2, 2]))
    if r.random() < 0.04:
        return [0]
    n = r.choice([0, 1, 2, 3, r.randrange(0, 12), r.randrange(0, 60), r.randrange(0, 60)])
    s = bytearray(text(ctx, n, ESC_ALPHA))
    if r.random() < 0.15 and n:
        s[-1] = 0x5c
    return [cstr(ctx, bytes(s), 16)]


DECO_ALPHA = b'AEIOUaeiouy!?AEIOUaeiouy!?bcdBCD xz% \n\r\t\v\f\x80\xc4\xff'


def g_7430(ctx):
    """Sim_100b7430: every flag combination, empty and longer texts, allocation failures."""
    r = begin(ctx, 0x100fef30, 0x100fefe0)
    n = r.choice([0, 1, 2, 5, r.randrange(0, 40)])
    newbuf(ctx, 0x20)           # flag 2 on an empty text writes the byte BEFORE it: keep that byte mapped
    tb = cstr(ctx, text(ctx, n, DECO_ALPHA))
    pp = ctx.buf(16, struct.pack('<4I', tb, 0, 0, 0))
    flags = r.choice([1, 2, 4, 8, 3, 5, 6, 7, 9, 0xa, 0xc, 0xf, 0, r.getrandbits(8), r.getrandbits(32)])
    ST['blocks'] = [r.choice([0, newbuf(ctx, 3 * n + 64), newbuf(ctx, 3 * n + 64), newbuf(ctx, 3 * n + 64)])
                    for _ in range(2)]
    return [ctx.u32(), pp, flags, ctx.u32()]


def g_1398(ctx):
    """Sim_100e1398: records of every group kind, table entries of every kind, names with and without a block,
    floats of every size for the truncation."""
    r = begin(ctx, 0x1005b98c)
    n = r.choice([0, 1, 3, 6])
    recs = ptrtab(ctx, n, 0x40)
    for u in recs:
        w32(ctx, u + 8, r.randrange(0, 16))
    for g in range(16):
        w32(ctx, 0x109b7d54 + g * 0x6c, r.choice([0, 1, 2, 3, r.getrandbits(32)]))
    for i in range(4):
        w32(ctx, 0x109b8400 + 4 * i, r.choice([0, 1, 2, 3]))
    m = r.choice([0, 1, 2, 4, 6])
    w32(ctx, 0x1015dc68, m)
    cnt = r.choice([0, 2, 4, 6])
    w32(ctx, 0x1017d218, cnt)
    for i in range(6):
        rec = newbuf(ctx, 0xa0)
        if r.random() < 0.7:
            blk = newbuf(ctx, 0x60)
            f = r.choice([0.0, -0.0, 0.5, -0.5, 1.5, -2.5, 123456.75, -99.99, 2147483648.0, -2147483904.0,
                          3.0e9, 1e20, -1e20, r.uniform(-1e6, 1e6), r.uniform(-50, 50)])
            ctx.m.uc.mem_write(blk + 0x50, struct.pack('<f', f))
            w32(ctx, rec + 0x8c, blk)
        else:
            w32(ctx, rec + 0x8c, 0)
        w32(ctx, 0x101a212c + i * 0x88, r.choice([rec, rec, 0]))
    for i in range(m):
        e = 0x109e9ba8 + i * 0x50
        w32(ctx, e, r.choice([-1, r.randrange(0, 6), r.randrange(0, 6), 7]))
        w32(ctx, e + 8, r.choice([-1, 0, 1, 2, 3]))
        w32(ctx, e + 0xc, ctx.u32())
    for a in (0x1016541a, 0x1016541c, 0x1016541e, 0x10165420, 0x10165422, 0x10165424):
        w16(ctx, a, r.choice([0, 1, 0xffff, r.getrandbits(16)]))
    return []


def g_6b29(ctx):
    begin(ctx)
    return [newbuf(ctx, 0x100)]


SPECS = [
    dict(name='Sim_10017263', orig=0x10017263, c='Sim_10017263', gen=g_7263, trials=5000, void=True),
    dict(name='Sim_10029b1f', orig=0x10029b1f, c='Sim_10029b1f', gen=g_9b1f, trials=300, void=True),
    dict(name='Sim_10032a10', orig=0x10032a10, c='Sim_10032a10', gen=g_2a10, trials=3000),
    dict(name='Sim_100352d5', orig=0x100352d5, c='Sim_100352d5', gen=g_52d5, trials=500, void=True),
    dict(name='Sim_10042a11', orig=0x10042a11, c='Sim_10042a11', gen=g_2a11, trials=800, void=True),
    dict(name='Sim_1004d344', orig=0x1004d344, c='Sim_1004d344', gen=g_d344, trials=300),
    dict(name='Sim_100527fa', orig=0x100527fa, c='Sim_100527fa', gen=g_27fa, trials=800),
    dict(name='Sim_1008b922', orig=0x1008b922, c='Sim_1008b922', gen=g_b922, trials=2000, void=True),
    dict(name='Sim_1009bd47', orig=0x1009bd47, c='Sim_1009bd47', gen=g_bd47, trials=300, void=True),
    dict(name='Text_Unescape', orig=0x100b6f6a, c='Text_Unescape', gen=g_6f6a, trials=1500),
    dict(name='Sim_100b7430', orig=0x100b7430, c='Sim_100b7430', gen=g_7430, trials=1000, void=True),
    dict(name='Sim_100e1398', orig=0x100e1398, c='Sim_100e1398', gen=g_1398, trials=800, void=True),
    # push es / push ds / pop es / pop es: segment loads the emulator cannot run (no effect in the flat model)
    dict(name='Sim_100e6b29', orig=0x100e6b29, c='Sim_100e6b29', gen=g_6b29, trials=300, void=True,
         nop=[0x100e6b2f, 0x100e6b31, 0x100e6b32, 0x100e6b43]),
]
