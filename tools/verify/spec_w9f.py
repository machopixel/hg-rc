# Differential tests for round 9, letter f (see difftest.py and docs/checker.md).
#
# Helpers of a tested function are replaced in the emulator by stubs while that function is tested (switched on
# per trial by begin()), as in spec_w9g.py: a code hook at the helper's address appends ('stub_<address>',
# arguments) to the checker's call log (compared between the original and the C run like the Windows calls),
# may write results through pointer arguments (the same bytes for both runs) and returns to the caller with eax
# from the trial's list. Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their
# own address; converted helpers (called by name) at their original address (reached by the original code) and
# at the address of their C function in the test image (reached by the C code), under the same log name. The
# three functions of this letter that call each other (Sim_1007adfc, Sim_1007b1d0, Sim_1007b5db) are stubbed the
# same way while their callers are tested, so every function is checked on its own logic.
# Stack addresses are logged as 'stack'; texts by content (SARGS); small records by their bytes (DEREF).
# Run for real on both sides: the converted string and memory helpers (Str_Copy, Str_Cat, Str_NCat, Str_CSpan,
# Str_CopyN, Str_Length, Mem_Fill, Crt_StrChr, Crt_StrStr, Str_CompareNoCase, Crt_ToLower, Str_CollapseSpaces),
# the text lookup Sim_100b7834 (with the runtime's bsearch), the converted sprintf / vsprintf (Crt_SPrintf,
# Crt_VSPrintf, whose formatter 0x10100fd0 is the original code in the emulator; only integer, character and
# text conversions are used) and the converted sscanf Sim_100fd710 (its reader 0x10103ba0 is the original code).
# strtok (Str_NextToken_100fd560) runs for real in Text_FormatNumbered, where its position stays in argument
# memory; in GameKey_LoadMap and Plugin_LoadList it would leave a pointer into the tested function's own stack
# frame in [0x1017e884] (an address that depends on the frame layout), so there it is replaced by a stub with
# the same effect on the text that stores a marker instead and logs the tokens.
# Sim_1005e426 is the one function with x87 code (two fld/fadd and two _ftol): it is game logic (a score) and is
# checked bit for bit (no fp tolerance) with the control word 0x27f the game runs with; its callers' arguments
# and the ftol results are compared exactly.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, \
    UC_X86_REG_FPTAG

MINE = [
    'sim/unsorted_1001/w9f_select.c', 'sim/unsorted_1003/w9f_plugin.c', 'sim/unsorted_1004/w9f_screenshot.c',
    'sim/unsorted_1005/w9f_gamekey.c', 'sim/unsorted_1005/w9f_score.c', 'sim/unsorted_100a/w9f_misc.c',
    'sim/unsorted_100b/w9f_text.c', 'sim/unsorted_1007/w9f_panel.c', 'sim/unsorted_1007/w9f_sides.c',
    'sim/unsorted_1007/w9f_objectives.c',
]
# existing converted code the functions above call (and what that calls): the closure spec_w9g.py compiles,
# plus the files of the callees of this letter that it lacks
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
    # callees of this letter that the list above lacks
    'sim/unsorted_1003/w8u_plugin.c', 'sim/unsorted_1004/w2h_palette.c', 'sim/unsorted_1004/w2b_checks.c',
    'sim/unsorted_1007/w2i_map.c', 'sim/unsorted_1007/w2h_misc.c', 'sim/unsorted_1001/w5z_calls.c',
    'sim/unsorted_100b/w5y_misc.c', 'sim/unsorted_1010/w7r_heap.c', 'sim/unsorted_1010/w8A_text.c',
    'sim/unsorted_1007/w2k_misc.c', 'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_1003/w7m_misc.c',
    'sim/unsorted_1003/w2h_misc.c', 'sim/unsorted_100f/w8A_stream.c', 'sim/unsorted_100f/w7q_find.c',
    'sim/unsorted_1010/w6k_sbh.c', 'sim/unsorted_1010/w7r_fmt.c', 'sim/unsorted_1010/w6j_heap.c',
    'sim/unsorted_1010/w8A_lowio.c', 'sim/unsorted_1010/w2f_digits.c', 'sim/unsorted_1001/w3s_group.c',
    'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1003/w5b_calls.c', 'sim/unsorted_1010/w2f_misc.c',
    'sim/unsorted_100d/w5x_calls.c',
]
SOURCES = MINE + OTHERS
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000

PT = 0x109e8b40             # the pointer table
SIDES = 0x10473800          # the side table: one block of 0x4fc3a bytes per side, rows of 0x13f from +0x3a
SIDE_SIZE = 0x4fc3a
ROW_SIZE = 0x13f
TOKPOS = 0x1017e884         # strtok's position


def side_addr(a):
    return SIDES + a * SIDE_SIZE


def row_addr(a, b):
    return SIDES + 0x3a + a * SIDE_SIZE + b * ROW_SIZE


# Stubbed helpers: original address -> (C function name or None, number of dword arguments logged).
HELPERS = {}


def helper(addr, cname, nargs):
    HELPERS[addr] = (cname, nargs)


# not converted or hooked elsewhere (the C calls them with SIM_FN)
for _a, _n in [(0x1003650d, 1), (0x100bc280, 2), (0x100782a9, 1), (0x1007a005, 2), (0x1007a4af, 2),
               (0x1007a955, 2)]:
    helper(_a, None, _n)
# converted (called by name), and the three functions of this letter that their callers of this letter use
for _a, _c, _n in [
        (0x10015c61, 'Sim_10015c61', 1), (0x10020256, 'Sim_10020256', 2), (0x10052be7, 'Sim_10052be7', 1),
        (0x10015f5b, 'Sim_10015f5b', 2), (0x10040f5f, 'Sim_10040f5f', 4), (0x10015d30, 'Sim_10015d30', 4),
        (0x100fd850, 'Sim_100fd850', 2), (0x10036466, 'Plugin_LoadAll', 0), (0x100fe770, 'Stream_ReadLine', 3),
        (0x100fd760, 'Crt_FClose', 1), (0x100fd560, 'Str_NextToken_100fd560', 2),
        (0x100472af, 'Screenshot_WriteTga', 5), (0x1004ec60, 'Sim_1004ec60', 1), (0x1004ecb8, 'Sim_1004ecb8', 0),
        (0x1004ed37, 'Sim_1004ed37', 1), (0x1004edb6, 'Sim_1004edb6', 0), (0x10053b64, 'Input_ReadLine', 3),
        (0x1008f630, 'Sim_1008f630', 1), (0x1008f867, 'Sim_1008f867', 2), (0x1007e7b6, 'Sim_1007e7b6', 1),
        (0x10020441, 'Sim_10020441', 1), (0x10041c00, 'Sim_10041c00', 0), (0x1007b4d6, 'Sim_1007b4d6', 2),
        (0x10079e3b, 'Sim_10079e3b', 1), (0x10042908, 'Sim_10042908', 1), (0x10079ecd, 'Sim_10079ecd', 2),
        (0x10079c86, 'Sim_10079c86', 2), (0x100419e9, 'Sim_100419e9', 0), (0x1001661f, 'Sim_1001661f', 1),
        (0x100fef30, 'Heap_Malloc', 1), (0x101003e0, 'Heap_Realloc', 2), (0x100fefe0, 'Heap_FreeBlock', 1),
        (0x1007b1d0, 'Sim_1007b1d0', 2), (0x1007adfc, 'Sim_1007adfc', 3), (0x1007b5db, 'Sim_1007b5db', 2)]:
    helper(_a, _c, _n)

SARGS = {0x1003650d: (0,), 0x100fd850: (0, 1), 0x100472af: (0,), 0x1004ec60: (0,), 0x1007e7b6: (0,),
         0x10020441: (0,), 0x100fd560: (1,)}
# arguments the helper reads as 16 bits (Sim_10040f5f casts them to uint16_t / uint8_t, Sim_10015d30 takes
# int16_t): the original pushes them in registers whose upper halves hold leftovers
M16 = 0xffff
MASK = {(0x10040f5f, 1): M16, (0x10040f5f, 2): M16, (0x10040f5f, 3): M16, (0x10015d30, 1): M16,
        (0x10015d30, 2): M16, (0x10015d30, 3): M16}
DEREF = {0x1004ed37: {0: 20}, 0x1008f630: {0: 8}}
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax) that writes the helper's results
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def put32(uc, addr, v):
    uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


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


TEXTCHARS = b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 _.,!?-'
TOKCHARS = b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_.\\-'


def text(ctx, n, alphabet=TEXTCHARS):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


def put_text(ctx, addr, data):
    """A NUL-terminated text at addr (the bytes after it stay as they are)."""
    wb(ctx, addr, data + b'\0')


# ---- stubs --------------------------------------------------------------------------------------
def _log_args(uc, addr, nargs, esp):
    raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
    args = []
    for k, v in enumerate(raw):
        if (addr, k) in MASK:
            v &= MASK[(addr, k)]
        if k in SARGS.get(addr, ()):
            v = ustr(uc, v) if v else None
        elif k in DEREF.get(addr, {}):
            try:
                v = ('deref', bytes(uc.mem_read(v, DEREF[addr][k])))
            except Exception:
                v = ('bad', v)
        args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
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
            ST['alloc_n'] = 0
            ST['tok_pos'] = None
            ST['heap_next'] = ST.get('heap_base', 0)
        raw, args = _log_args(uc, addr, nargs, esp)
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % addr, tuple(args)))
        k = ST['count'].get(addr, 0)
        ST['count'][addr] = k + 1
        rets_ = ST['rets'].get(addr) or [0]
        eax = rets_[n % len(rets_)] & 0xffffffff
        if addr in EFFECT:
            v = EFFECT[addr](uc, k, raw, eax)
            if isinstance(v, tuple):        # (extra log entries, eax or None)
                extra, v = v
                m.api_log.extend(extra)
            if v is not None:
                eax = v & 0xffffffff
        uc.reg_write(UC_X86_REG_EAX, eax)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def begin(ctx, *active):
    """Installs the stubs (once per machine), the x87 state of the game (control word 0x27f) and switches on the
    stubs of the helpers given (original addresses) for this trial. Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w9f', False):
        m._w9f = True
        for addr, (cname, nargs) in sorted(HELPERS.items()):
            cb = _stub_cb(m, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            if cname is not None:
                m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    for a in active:
        assert a in HELPERS, hex(a)
    ST['count'] = {}
    ST['log'] = None
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    ST.update(lines=[], heap_base=0, heap_end=0, hsize={}, fail=set())
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def ptrtab(ctx, count=8, size=0x200):
    """count records in the pointer table 0x109e8b40."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, PT + 4 * i, rec)
        recs.append(rec)
    return recs


# ---- effects of stubbed helpers -----------------------------------------------------------------------
# words the game pushes after the format of 0x100bc280 (the callers always add a 0 after fewer than two values)
ERR_WORDS = {0x10141cb0: 1, 0x10141cd8: 2, 0x10141d08: 2, 0x10141d3c: 2, 0x10141d6c: 2}


def e_err(uc, k, raw, eax):
    """0x100bc280 (code, format, ...): logs the format text and the words that follow it (texts for %s)."""
    if raw[1] == 0:
        return [], 0
    fmt = ustr(uc, raw[1])
    specs = []
    i = 0
    while i < len(fmt):
        if fmt[i:i + 1] == b'%':
            if fmt[i + 1:i + 2] == b'%':
                i += 2
                continue
            j = i + 1
            while j < len(fmt) and chr(fmt[j]) not in 'dsuxXc':
                j += 1
            specs.append(chr(fmt[j]) if j < len(fmt) else 'd')
            i = j + 1
        else:
            i += 1
    esp = uc.reg_read(UC_X86_REG_ESP)
    extra = [fmt]
    for k2 in range(ERR_WORDS.get(raw[1], len(specs))):
        v = ur32(uc, esp + 12 + 4 * k2)
        if k2 < len(specs) and specs[k2] == 's':
            extra.append(ustr(uc, v))
        else:
            extra.append('stack' if STACK_LO <= v <= STACK_HI else v)
    return [('err_args', tuple(extra))], 0


def e_readline(uc, k, raw, eax):
    """Stream_ReadLine / Input_ReadLine (fgets-like): the k-th line of the trial into the buffer, 0 at the end."""
    lines = ST['lines']
    if k < len(lines):
        uc.mem_write(raw[0], lines[k] + b'\0')
        return raw[0]
    return 0


def e_strtok(uc, k, raw, eax):
    """strtok with the real effect on the text, the position kept here and a marker in [0x1017e884] (the real
    position would point into the tested function's stack frame)."""
    delim = ustr(uc, raw[1])
    s = raw[0] if raw[0] != 0 else ST['tok_pos']
    if s is None:
        s = ur32(uc, TOKPOS)
    while True:
        c = bytes(uc.mem_read(s, 1))
        if c == b'\0' or c not in delim:
            break
        s += 1
    start = s
    if bytes(uc.mem_read(s, 1)) != b'\0':
        while True:
            c = bytes(uc.mem_read(s, 1))
            if c in delim:
                uc.mem_write(s, b'\0')
                s += 1
                break
            s += 1
            if bytes(uc.mem_read(s, 1)) == b'\0':
                break
    ST['tok_pos'] = s
    put32(uc, TOKPOS, 0x00DEAD00 + k)
    tok = ustr(uc, start) if start != s else None
    return [('token', tok)], (start if start != s else 0)


def e_alloc(size):
    """A block of the trial's heap area (0 when the trial's list says this allocation fails)."""
    n = ST['alloc_n']
    ST['alloc_n'] = n + 1
    if n in ST['fail']:
        return 0
    a = ST['heap_next']
    ST['heap_next'] = a + ((size + 31) & ~15)
    if ST['heap_next'] > ST['heap_end']:
        return 0
    ST['hsize'][a] = size
    return a


def e_malloc(uc, k, raw, eax):
    return e_alloc(raw[0])


def e_realloc(uc, k, raw, eax):
    old, size = raw
    a = e_alloc(size)
    if a and old:
        n = min(ST['hsize'].get(old, 0), size)
        if n:
            uc.mem_write(a, bytes(uc.mem_read(old, n)))
    return a


def e_free(uc, k, raw, eax):
    return None


def e_782a9(uc, k, raw, eax):
    """0x100782a9(obj): logs the entry 0x10185400 + obj[+0xc] * 0x14 and the record obj[+0xec] (the rectangle the
    caller swapped in for the call)."""
    obj = raw[0]
    entry = 0x10185400 + ur32(uc, obj + 0xc) * 0x14
    rec = ur32(uc, obj + 0xec)
    return [('swapped', bytes(uc.mem_read(entry, 20)), bytes(uc.mem_read(rec, 20)))], None


EFFECT.update({0x100bc280: e_err, 0x100fe770: e_readline, 0x10053b64: e_readline, 0x100fd560: e_strtok,
               0x100fef30: e_malloc, 0x101003e0: e_realloc, 0x100fefe0: e_free, 0x100782a9: e_782a9})


def e_queue(uc, k, raw, eax):
    """0x10020441 takes a 24-byte record, not a text: the name, a message text pointer at +0x10 and a flag at
    +0x14, which 0x1001f790 reads (round 9 crash: the C passed a bare 16-byte name). Logs the whole record."""
    a = raw[0]
    p, flag = struct.unpack('<II', bytes(uc.mem_read(a + 0x10, 8)))
    text = ustr(uc, p) if p else None
    return [('queue_rec', text, 'stack' if STACK_LO <= p <= STACK_HI else p, flag)], None


EFFECT[0x10020441] = e_queue


# ---- generators ---------------------------------------------------------------------------------------
def g_6117(ctx):
    r = begin(ctx, 0x10015c61, 0x10020256, 0x10052be7, 0x10015f5b, 0x10040f5f, 0x10015d30)
    recs = ptrtab(ctx, 8, 0x200)
    c = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, c)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    idx = r.choice([-1, 0, 1, 2, 3, 4, 5, 6, 7, c, c, c])
    rets(0x10015c61, idx)
    rets(0x10052be7, r.choice([0, 1]))
    rets(0x10015f5b, r.choice([0x1000, 0x1000, 0x205, 0x200 | (idx & 0xff), 0x100, 1, 0x7fff, 0xffff]))
    rets(0x10015d30, r.getrandbits(32))
    cur = recs[c]
    sel = r.choice([0, 0, 0xffff, 0x200 | (idx & 0xff), 0x200 | r.randrange(0, 8), 0x300 | r.randrange(0, 8),
                    0x100, 0x1000, 0x1200 | (idx & 0xff), r.getrandbits(16), 0x8000 | r.getrandbits(15), 1])
    w32(ctx, cur + 0xf0, sel | ((r.getrandbits(16) << 16) if r.random() < 0.3 else 0))
    w32(ctx, cur + 0x118, r.choice([0x100 | r.randrange(0, 0x100), 0x100 | r.randrange(0, 0x100), 0, 0xffffffff,
                                    0x7fffffff, 0x80000100, r.randrange(0, 0x100), r.getrandbits(32)]))
    for rec in recs:
        w32(ctx, rec + 0x14, r.choice([3, 3, 0, 1, 4]))
        w32(ctx, rec + 0xc, r.choice([c, c, r.randrange(0, 8)]))
    kind = r.choice([2, 3, 5, 7, 8, 11, 13, 14, 15, 2, 3, 7, 13, 14, 15, 2, 3, 7,
                     r.choice([0, 1, 4, 6, 9, 10, 12, 16, -1, 100, 0x8002])])
    if r.random() < 0.3:
        kind = (kind & 0xffff) | (r.getrandbits(16) << 16)
    return [r.choice([r.randrange(0, 16), ctx.u32()]), kind & 0xffffffff]


def lines_plugin(ctx):
    r = ctx.rng
    seps = b' \t\r\n'
    lines = []
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        k = r.random()
        if k < 0.1:
            line = b''
        elif k < 0.2:
            line = bytes(r.choice(seps) for _ in range(r.randrange(1, 5)))
        else:
            line = bytes(r.choice(seps) for _ in range(r.randrange(0, 3))) + text(ctx, r.randrange(1, 30), TOKCHARS)
            line += bytes(r.choice(seps) for _ in range(r.randrange(0, 3)))
            if r.random() < 0.3:
                line += text(ctx, r.randrange(1, 10), TOKCHARS)
            line += r.choice([b'', b'\n', b'\r\n'])
        lines.append(line[:0xfe])
    return lines


def g_6876(ctx):
    r = begin(ctx, 0x100fd850, 0x10036466, 0x100fe770, 0x100fd560, 0x1003650d, 0x100fd760)
    name = cstr(ctx, text(ctx, r.randrange(1, 20), TOKCHARS))
    stream = newbuf(ctx, 0x20)
    rets(0x100fd850, r.choice([0, stream, stream, stream]))
    rets(0x10036466, r.getrandbits(32))
    rets(0x100fd760, 0)
    ST['lines'] = lines_plugin(ctx)
    return [name]


def g_73e4(ctx):
    r = begin(ctx, 0x100472af, 0x1004ec60, 0x1004ecb8, 0x1004ed37, 0x1004edb6)
    # the number goes into "hg%04d.gif" in a 16-byte name: nothing below -99999, which would overflow the frame
    # (the original's as much as the C's, differently laid out)
    w32(ctx, 0x1015f150, r.choice([0, 1, 7, 42, 999, 999, 1000, 1001, 5000, 0xffffffff, -12345, 0x7fffffff,
                                   r.randrange(0, 1200)]))
    w32(ctx, 0x1017b940, r.choice([0, 1, 1, 2]))
    for a in (0x109e3e14, 0x109e3de0, 0x109e3e24, 0x109e3e20):
        w32(ctx, a, ctx.u32())
    return []


def g_b8b0(ctx):
    r = begin(ctx)
    return [r.choice([r.randrange(0, 0x5f), r.randrange(0, 0x5f), r.randrange(0, 0x70), 0x5e, 0x5f, 0xffffffff,
                      0x80000000, ctx.u32()])]


KEY_NAMES = [b'ALT', b'CTRL', b'SHIFT', b'F1', b'F2', b'F3', b'F4', b'F5', b'F6', b'F7', b'F8', b'F9', b'F10', b'F11',
             b'F12', b'TAB', b'INSERT', b'ESC', b'BSP', b'ENTER', b'PAUSE', b'POUND', b'PLUS', b'GREY SLASH',
             b'GREY ASTERICK', b'GREY MINUS', b'GREY PLUS']


def action_names(ctx):
    """The 196 action names of the table at 0x1013ff00 (read from the DLL once per machine)."""
    m = ctx.m
    names = getattr(m, 'w9f_actions', None)
    if names is None:
        names = []
        for i in range(0xc4):
            p = r32(ctx, 0x1013ff00 + 5 * i)
            names.append(ustr(m.uc, p))
        m.w9f_actions = names
    return names


def mixcase(r, s):
    return bytes((c ^ 0x20) if r.random() < 0.3 and 0x41 <= (c & 0xdf) <= 0x5a else c for c in s)


def lines_gamekey(ctx):
    r = ctx.rng
    acts = action_names(ctx)
    lines = []
    for _ in range(r.choice([0, 1, 1, 2, 2, 3, 4, 6, 10])):
        k = r.random()
        if k < 0.06:
            line = b''                                              # sscanf gives -1
        elif k < 0.12:
            line = text(ctx, r.randrange(1, 20), TOKCHARS)          # one token: sscanf gives 1
        else:
            act = mixcase(r, r.choice(acts)) if r.random() < 0.9 else text(ctx, r.randrange(1, 20), TOKCHARS)
            parts = []
            for _ in range(r.choice([1, 1, 2, 2, 3, 4])):
                k2 = r.random()
                if k2 < 0.5:
                    parts.append(mixcase(r, r.choice(KEY_NAMES)).replace(b' ', b'_'))
                elif k2 < 0.8:
                    parts.append(bytes([r.choice(b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789')]))
                elif k2 < 0.9:
                    parts.append(bytes([r.randrange(0x80, 0x100)]) + text(ctx, r.randrange(0, 4), TOKCHARS))
                else:
                    parts.append(text(ctx, r.randrange(1, 8), TOKCHARS))
            seq = b'+'.join(parts)
            if r.random() < 0.05:
                seq = r.choice([b'+', b'++'])                       # no token in the key part
            line = act + r.choice([b' ', b'  ', b'\t']) + seq + r.choice([b'', b'\n', b' \n', b'\r\n'])
        lines.append(line[:0xfe])
    return lines


def g_4a79(ctx):
    r = begin(ctx, 0x100fd850, 0x100bc280, 0x10053b64, 0x100fd560, 0x100fd760)
    stream = newbuf(ctx, 0x20)
    rets(0x100fd850, r.choice([0, stream, stream, stream, stream]))
    rets(0x100fd760, 0)
    w32(ctx, 0x1015fdf8, r.choice([0, 0, 3, 77, 0xffffffff]))
    # the key -> action table (0x10164bc8 +- the 16-bit sum): mostly empty, sometimes a few entries
    if r.random() < 0.4:
        for _ in range(r.randrange(1, 60)):
            w8(ctx, 0x10164bc8 + r.choice([r.randrange(-0x8000, 0x8000), r.randrange(0, 0x200), r.randrange(0, 0x80)]),
               r.randrange(0, 256))
    ST['lines'] = lines_gamekey(ctx)
    return []


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def score_float(r):
    k = r.random()
    if k < 0.4:
        return r.uniform(-100000.0, 100000.0)
    if k < 0.6:
        return float(r.randrange(-5000, 5000)) + r.choice([0.0, 0.5, -0.5, 0.25, 0.999, 0.001])
    if k < 0.75:
        return r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 1.5, -1.5, 65535.0, 65536.0, -65536.0, 2147483647.0,
                         2147483648.0, -2147483648.0, -2147483649.0, 4294967296.0, 1e10, -1e10])
    if k < 0.9:
        return r.uniform(-1, 1) * 10 ** r.randint(-8, 12)
    if k < 0.95:
        return float('nan')
    return r.choice([float('inf'), -float('inf'), 1e30, -1e30, 9.3e18, -9.3e18])


def g_e426(ctx):
    r = begin(ctx)
    p = newbuf(ctx, 0x20)
    w32(ctx, p, r.choice([r.randrange(0, 0x10000), r.randrange(0, 0x10000), r.getrandbits(32), 0, 1, 0xffff,
                          0x7fff, 0x8000]))
    w32(ctx, p + 0x10, r.choice([0, 1, 2, 3, 5, 8, 8, 9, 20, 0xffffffff]))
    recs = newbuf(ctx, 17 * 0x3c + 0x10)
    for i in range(17):
        rec = recs + 0x3c * i
        for off in (0, 4, 8):
            w32(ctx, rec + off, fbits(score_float(r)))
        # the words start at +0xc and the count word is at +0x34 (word 20): counts above 20 read the count itself
        # and the bytes after it, as the original would; the count stays small so the loops end
        for j in range(24):
            w16(ctx, rec + 0xc + 2 * j, r.choice([r.randrange(0x1b00, 0x2000), r.randrange(0x1b00, 0x2000),
                                                  r.randrange(0, 0x10000), 0x1b58, 0x1b59, 0x1f40, 0x1f41, 0x1e14,
                                                  0x1e15, 0x7fff, 0x8000, 0xffff, 0]))
        w16(ctx, rec + 0x34, r.choice([0, 0, 1, 2, 3, 5, 8, 12, 20, 24, 0xffff, 0x8000]))
    lst = newbuf(ctx, 8 * 9)
    for i in range(9):
        w32(ctx, lst + 8 * i, r.choice([r.randrange(0, 30000), r.randrange(-30000, 30000), 0, 99, 100, 101, -1,
                                        -100, -101, r.randrange(-2000000, 2000000)]))
    return [p, recs, lst]


def g_8a7f(ctx):
    r = begin(ctx, 0x1008f630, 0x1008f867, 0x100782a9)
    obj = newbuf(ctx, 0x100)
    w16(ctx, obj + 4, r.choice([0, 1, 1, 0xffff, r.getrandbits(16)]))
    w32(ctx, obj + 0x30, r.choice([0, 1, 2, 3, 3, 3, 4, 7, r.getrandbits(32)]))
    pair = newbuf(ctx, 8)
    w32(ctx, obj + 0xf4, r.choice([0, pair, pair, pair]))
    w32(ctx, obj + 0xf8, r.choice([0, 1, 2, 3, 4, 5, 6, r.getrandbits(32)]))
    w32(ctx, obj + 0xc, r.randrange(0, 8))
    w32(ctx, obj + 0xec, newbuf(ctx, 0x20))
    wb(ctx, 0x10185400, rb(ctx, 0x14 * 8))
    w32(ctx, 0x109e89d8, ctx.u32())
    rect = newbuf(ctx, 0x20)
    rets(0x1008f867, r.choice([0, rect, rect, rect]))
    return [obj]


def fill_side(ctx, side, count, rows=None):
    """Random bytes in the header and the first `rows` rows of a side block, count in its dword +0."""
    rows = count if rows is None else rows
    wb(ctx, side_addr(side), rb(ctx, 0x3a))
    for i in range(rows):
        wb(ctx, row_addr(side, i), rb(ctx, ROW_SIZE))
    w32(ctx, side_addr(side), count)


def g_8f60(ctx):
    r = begin(ctx, 0x100bc280)
    side = r.randrange(0, 4)
    n = r.choice([0, 1, 1, 2, 3, 5])
    p = newbuf(ctx, 0x26 + 0x97 * n + 0x40)
    w32(ctx, p + 4, 0x26 + 0x97 * n + r.choice([0, 0, 0, r.randrange(1, 0x97)]))
    w32(ctx, p + 8, side)
    put_text(ctx, p + 0x12, text(ctx, r.randrange(0, 11)))
    put_text(ctx, p + 0x1d, text(ctx, r.randrange(0, 9)))
    for i in range(n):
        m = p + 0x26 + 0x97 * i
        w8(ctx, m + 4, r.choice([0x56, 0x56, 0x58, r.getrandbits(8)]))
        w8(ctx, m + 0x2f, r.choice([0x4d, 0x4d, 0x58, r.getrandbits(8)]))
        put_text(ctx, m + 0x34, text(ctx, r.randrange(0, 11)))
        put_text(ctx, m + 0x3f, text(ctx, r.randrange(0, 20)))
        put_text(ctx, m + 0x57, text(ctx, r.randrange(0, 64)))
        for c in range(8):
            w8(ctx, m + 0xa + c * 4, r.choice([0x43, 0x53, 0x46, 0x58, r.getrandbits(8)]))
    w32(ctx, p + 0x26, r.choice([0x10, 0x10, 0x10, 0, 0x11, r.getrandbits(32)]))
    w8(ctx, 0x109b7d4c + side * 0x6c, r.getrandbits(8))
    w32(ctx, 0x104737e0, r.choice([0, side, side + 1, side + 2, 4, 0xffffffff]))
    w32(ctx, 0x104727a0 + 4 * side, ctx.u32())
    fill_side(ctx, side, r.getrandbits(32), max(n, 1))
    return [p]


def name_text(ctx, limit):
    r = ctx.rng
    k = r.random()
    if k < 0.5:
        base = r.choice([b'Xavon', b'avos1', b'avonx', b'savos', b'avon', b'avos', b'Avon', b'xxavonyy'])
    else:
        base = text(ctx, r.randrange(0, limit + 1))
    return base[:limit]


def current_block(ctx):
    """The current record PTRTAB([0x1017b8c8]) with a block at +0x28 whose dword +0x613 is 0, 1 or 2."""
    r = ctx.rng
    c = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, c)
    recs = ptrtab(ctx, 8, 0x40)
    block = newbuf(ctx, 0x640)
    w32(ctx, recs[c] + 0x28, block)
    w32(ctx, block + 0x613, r.choice([0, 1, 1, 2, 0]))


def g_adfc(ctx):
    r = begin(ctx, 0x1007e7b6, 0x10020441)
    own = r.randrange(0, 4)
    side = r.choice([own, own, own, r.randrange(0, 4)])
    j = r.randrange(0, 4)
    w32(ctx, 0x1013bb40, own)
    w32(ctx, 0x104727e0 + 4 * j, r.choice([0, 0, 0, 1, 7]))
    current_block(ctx)
    fill_side(ctx, side, 4)
    row = row_addr(side, j)
    # both names are copied into a 16-byte local: at most 15 characters (longer ones overflow the frame)
    put_text(ctx, row + 0x89, name_text(ctx, 15))
    put_text(ctx, row + 0x99, name_text(ctx, 15))
    put_text(ctx, row + 0xad, r.choice([b'', b' ', text(ctx, r.randrange(1, 40)), b'a  b   c ', b' lead']))
    w8(ctx, row + 0x74, r.choice([0, 1, 1]))
    for a in (0x101c469c, 0x101c468c):
        w32(ctx, a, cstr(ctx, r.choice([b'', text(ctx, r.randrange(1, 30)), b'  two  spaces  '])))
    if r.random() < 0.5:
        put_text(ctx, 0x10179d34, r.choice([b'', b'a  b ', b'   ', b'x']))
    return [side, j, r.choice([5, 6])]


def g_b1d0(ctx):
    r = begin(ctx, 0x1007e7b6, 0x10041c00, 0x10020441)
    w32(ctx, 0x10179d30, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    own = r.randrange(0, 4)
    side = r.choice([own, own, own, r.randrange(0, 4)])
    w32(ctx, 0x1013bb40, own)
    current_block(ctx)
    fill_side(ctx, side, 2)
    put_text(ctx, side_addr(side) + 0x18, name_text(ctx, 15))
    put_text(ctx, side_addr(side) + 0x28, name_text(ctx, 15))
    for a in (0x101c46f0, 0x101c4688, 0x101c4684):
        w32(ctx, a, cstr(ctx, r.choice([b'', text(ctx, r.randrange(1, 60)), b'100%% done', b'x'])))
    return [side, r.choice([2, 3, 4])]


TYPES = [1, 2, 4, 8, 0x20, 0x100, 0x2000, 0x4000, 0x8000, 0x8001, 0x8002, 0x10000, 0x20000, 0x40000, 0x80000,
         0x100000, 0x200000, 0x400000, 0, 0x200, 0x400, 0x800]


def random_rows(ctx, side, count, now):
    r = ctx.rng
    fill_side(ctx, side, count, 4)
    for i in range(4):
        row = row_addr(side, i)
        w8(ctx, row, r.choice([3, 3, 0, 5, 6, 8]))
        w32(ctx, row + 2, r.choice(TYPES + [0x10, 0x10, r.getrandbits(32)]))
        w8(ctx, row + 0x74, r.choice([0, 1]))
        w32(ctx, row + 0x75, r.choice([0, 1, 1, 0x100, 0x12345600, r.getrandbits(32)]))
        w32(ctx, row + 0x67, r.choice([0xffffffff, now - r.randrange(0, 500), r.getrandbits(32)]))
        w32(ctx, row + 0x6b, r.choice([0xffffffff, now - r.randrange(0, 500), r.getrandbits(32)]))
        w8(ctx, row + 0xee, r.choice([0, 1, 2, 3, 4]))
        for k in range(4):
            w8(ctx, row + 0xef + 2 * k, r.randrange(0, 8), r.choice([2, 2, 4, 4, 0, 1, 3, 8]))


def g_b5db(ctx):
    r = begin(ctx, 0x1007b4d6, 0x10079e3b, 0x10042908, 0x10079ecd, 0x10079c86, 0x1007a005, 0x1007a4af, 0x1007a955,
              0x1007b1d0, 0x1007adfc)
    for a in (0x1007b4d6, 0x10079e3b, 0x10042908, 0x10079ecd, 0x10079c86, 0x1007a005, 0x1007a4af, 0x1007a955):
        rets(a, *[r.choice([0, 1, 1, 1, 2, 0xffffffff]) for _ in range(5)])
    rets(0x1007b4d6, *[r.choice([0, 1, 1, 1, 1, 1, 1]) for _ in range(5)])
    nsides = r.randrange(1, 4)
    w32(ctx, 0x104737e0, nsides)
    own = r.randrange(0, nsides)
    w32(ctx, 0x1013bb40, own)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x101655ac, r.choice([0xffffffff, 0, 5]))
    now = r.randrange(100, 100000)
    w32(ctx, 0x1037305c, now)
    for s in range(nsides + 2):
        random_rows(ctx, s, r.randrange(0, 5), now)
    side = r.randrange(0, nsides)
    j = r.randrange(0, 4)
    row = row_addr(side, j)
    w32(ctx, row + 2, r.choice(TYPES + [r.getrandbits(32), 0x10000, 0x20000, 0x40000, 0x80000, 0x100000, 0x200000,
                                        0x400000, 4, 4, 0x100, 0x2000, 0x4000, 1, 2, 0x20, 8]))
    w8(ctx, row, r.choice([3, 3, 3, 3, 0, 5, 6, 8]))
    w32(ctx, row + 0x67, r.choice([0xffffffff, 0xffffffff, now - r.randrange(0, 500), now - 1000, now,
                                   r.getrandbits(32)]))
    w32(ctx, row + 0x6f, r.choice([0xffffffff, 0, 10, 200, 200, 1000, 5000, r.getrandbits(32)]))
    w8(ctx, row + 0x73, r.choice([0, 1]))
    w16(ctx, row + 0xa9, r.choice([0, 1, 2, nsides, nsides - 1, nsides + 1, 0xffff]))
    w16(ctx, row + 0xab, r.choice([0, 1, 2, 3, 5, 0xffff]))
    for k in range(4):
        w32(ctx, 0x104727e0 + 4 * k, r.choice([0, 1]))
    g = r.randrange(0, 8)
    w32(ctx, 0x109b7d40 + side * 0x6c, g)
    recs = ptrtab(ctx, 8, 0x40)
    for rec in recs:
        w16(ctx, rec + 0x1c, r.choice([2, 2, 0, 3, 0xffff, r.getrandbits(16)]))
    if r.random() < 0.2:
        w32(ctx, PT + 4 * g, 0)
    return [side, j]


def g_c42f(ctx):
    r = begin(ctx, 0x1007b5db, 0x1007b4d6, 0x1007b1d0, 0x100419e9, 0x1001661f)
    rets(0x1007b4d6, *[r.choice([0, 0, 1, 1, 1]) for _ in range(5)])
    nsides = r.choice([0, 1, 1, 2, 2, 3])
    w32(ctx, 0x104737e0, nsides)
    own = r.randrange(0, max(nsides, 1))
    w32(ctx, 0x1013bb40, own)
    net = r.choice([0, 1])
    w32(ctx, 0x1015ec58, net)
    w32(ctx, 0x101655ac, r.choice([0xffffffff, 0, 5]))
    w32(ctx, 0x1015dc78, r.choice([r.randrange(0, 0x100000), r.getrandbits(32), 0, 0xb4, 0xb5]))
    now = (struct.unpack('<i', struct.pack('<I', r32(ctx, 0x1015dc78)))[0]) // 0xb5
    for a, vals in ((0x10179d2c, [0, 0, 0, 1]), (0x1015f1e8, [0, 1]), (0x1015d528, [0, 0, 1]),
                    (0x1015f1fc, [0, 0, 1]), (0x1015f1e4, [0, 7]), (0x1015f1d0, [0, 9])):
        w32(ctx, a, r.choice(vals))
    for s in range(max(nsides, 1) + 1):
        count = r.choice([0, 1, 2, 3, 3])
        random_rows(ctx, s, count, now)
        block = side_addr(s)
        w8(ctx, block + 0x38, r.choice([0, 0, 0, 2, 3, 4, 1]))
        w32(ctx, block + 4, r.choice([0, now - r.randrange(0, 1000), now, 0xffffffff]))
        w32(ctx, block + 0xc, r.choice([0, 0xffffffff, 10, 500, 5000, r.getrandbits(32)]))
        w32(ctx, 0x104727a0 + 4 * s, r.choice([0xffffffff, 0xffffffff, 0, 1, 2, 3]))
        for i in range(4):
            row = row_addr(s, i)
            w32(ctx, row + 2, r.choice([0x4000, 0x4000, 0x4000, 0x10, 0x20, 1, 0x10000, 0x12340010, r.getrandbits(32)]))
    ptrtab(ctx, 8, 0x200)
    wb(ctx, 0x109e9bec, rb(ctx, 0x50 * 8 + 4))
    return []


def g_cbcb(ctx):
    r = begin(ctx)
    side = r.randrange(0, 4)
    count = r.choice([0, 1, 2, 3, 3, 6, 0xffffffff, 0x80000000])
    fill_side(ctx, side, count, 6)
    for i in range(6):
        w32(ctx, row_addr(side, i) + 2, r.choice([0x20, 0x20, 7, 0, 0x21, r.getrandbits(32)]))
    return [side]


def g_ab1f2(ctx):
    begin(ctx)
    return []


SPEC_LIST = ['d', 'd', 's', 's', 'x', 'u', 'c', '5d', '-8s', '04x', 'ld', 'i', 'X', '3c']


def g_78d0(ctx):
    r = begin(ctx, 0x100fef30, 0x101003e0, 0x100fefe0)
    heap = ctx.buf(0x3000, 'rand')
    ST['heap_base'], ST['heap_end'] = heap, heap + 0x3000
    ST['fail'] = set(r.sample(range(12), r.choice([0, 0, 0, 0, 0, 0, 0, 1, 1, 2])))
    nslots = r.choice([0, 1, 1, 2, 2, 3, 3, 4])
    used = list(range(1, nslots + 1))
    r.shuffle(used)
    if r.random() < 0.2 and nslots > 1:
        used = used[:-1]                                    # a missing number: an empty slot
    specs = {}
    pieces = []
    for n in used:
        if r.random() < 0.15:
            specs[n] = r.choice(SPEC_LIST)                  # the same number twice: the later spec wins
            pieces.append(b'%%%d:%s' % (n, specs[n].encode()))
        specs[n] = r.choice(SPEC_LIST)
        pieces.append(b'%%%d:%s' % (n, specs[n].encode()))
    parts = []
    for p in pieces:
        if r.random() < 0.7:
            parts.append(r.choice([text(ctx, r.randrange(1, 12)), b'%%', b' - ', b'%%%%']))
        parts.append(p)
    if r.random() < 0.7:
        parts.append(r.choice([text(ctx, r.randrange(1, 12)), b'%%', b'.']))
    if r.random() < 0.08:
        parts.insert(r.randrange(0, len(parts) + 1), r.choice([b'%q', b'%', b'%:', b'% 1:d']))     # no number
    fmt = b''.join(parts)
    # the text lookup: the key text is the format itself (no table or no entry) or an entry naming another one
    key = cstr(ctx, fmt)
    k = r.random()
    if k < 0.3:
        w32(ctx, 0x1017d594, 0)
    else:
        names = sorted({fmt, text(ctx, r.randrange(1, 8)), text(ctx, r.randrange(1, 8)), b'a'})
        if k < 0.5:
            names.remove(fmt)
        arr = ctx.buf(8 * len(names), 'zero')
        for i, nm in enumerate(names):
            np_ = key if nm == fmt and r.random() < 0.5 else cstr(ctx, nm)
            val = np_
            if nm == fmt and r.random() < 0.7:
                val = cstr(ctx, fmt.replace(b'-8s', b'7s') if r.random() < 0.5 else fmt)
            w32(ctx, arr + 8 * i, np_, val)
        w32(ctx, 0x1017d594, ctx.buf(8, struct.pack('<2I', len(names), arr)))
    args = []
    for n in sorted(specs):
        sp = specs[n]
        if sp.endswith('s'):
            args.append(cstr(ctx, r.choice([text(ctx, r.randrange(1, 16)), text(ctx, r.randrange(1, 16)), b'',
                                            b'x', b'with space'])))
        elif sp.endswith('c'):
            args.append(r.choice([0x41, 0x7a, 0x30, 0x20, 0x7e, 0x141]))
        else:
            args.append(r.choice([0, 1, 7, 42, 1234567, 0xffffffff, 0x80000000, 0x7fffffff, r.getrandbits(32)]))
    out = ctx.buf(0x200, 'rand')
    return [out, key] + args


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Sim_10016117', 0x10016117, g_6117, 1500),
    spec('Plugin_LoadList', 0x10036876, g_6876, 500),
    spec('Screenshot_Save', 0x100473e4, g_73e4, 400, void=True),
    spec('Sim_1004b8b0', 0x1004b8b0, g_b8b0, 400),
    spec('GameKey_LoadMap', 0x10054a79, g_4a79, 1000),
    spec('Sim_1005e426', 0x1005e426, g_e426, 1500),
    spec('Sim_10078a7f', 0x10078a7f, g_8a7f, 800, void=True),
    spec('Sim_10078f60', 0x10078f60, g_8f60, 600, void=True),
    spec('Sim_1007adfc', 0x1007adfc, g_adfc, 800),
    spec('Sim_1007b1d0', 0x1007b1d0, g_b1d0, 600),
    spec('Sim_1007b5db', 0x1007b5db, g_b5db, 2500, void=True),
    spec('Sim_1007c42f', 0x1007c42f, g_c42f, 1500, void=True),
    spec('Sim_1007cbcb', 0x1007cbcb, g_cbcb, 400),
    spec('Sim_100ab1f2', 0x100ab1f2, g_ab1f2, 300),
    spec('Text_FormatNumbered', 0x100b78d0, g_78d0, 1200),
]
