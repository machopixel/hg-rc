# Differential tests for round 9, letter g (see difftest.py and docs/checker.md).
#
# Helpers of a tested function are replaced in the emulator by stubs while that function is tested (switched on
# per trial by begin()), as in spec_w8t.py: a code hook at the helper's address appends ('stub_<address>',
# arguments) to the checker's call log (compared between the original and the C run like the Windows calls),
# may write results through pointer arguments (the same bytes for both runs) and returns to the caller with eax
# from the trial's list. Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their
# own address; converted helpers (called by name) at their original address (reached by the original code) and
# at the address of their C function in the test image (reached by the C code), under the same log name.
# Stack addresses are logged as 'stack'; texts by content (SARGS); small records by their bytes (DEREF);
# arguments the helper reads as 16 bits are masked (MASK).
# Run for real on both sides: the converted string and memory helpers, Sim_10011b9e, Sim_10052aaf,
# Math_AbsInt_100fd100, Sim_1007652f, Crt_IsCType, Str_NextToken_100fd560 and the converted sprintf (Crt_SPrintf,
# whose formatter 0x10100fd0 is the original code in the emulator; only %d and %s conversions are used).
# strtok's saved position [0x1017e884] (Lang_ReadIni) points into the function's line buffer on the stack: at the
# Crt_FClose stub it is replaced by a marker and the text it points at is logged instead (e_fclose).
# No floating point is involved.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, \
    UC_X86_REG_FPTAG

MINE = [
    'sim/unsorted_1001/w9g_mono.c', 'sim/unsorted_1001/w9g_select.c', 'sim/unsorted_1002/w9g_misc.c',
    'sim/unsorted_1004/w9g_gif.c', 'sim/unsorted_1005/w9g_lang.c', 'sim/unsorted_1006/w9g_misc.c',
    'sim/unsorted_1007/w9g_misc.c', 'sim/unsorted_1008/w9g_nmsave.c', 'sim/unsorted_100c/w9g_select.c',
    'sim/unsorted_100c/w9g_text.c',
]
# existing converted code the functions above call (and what that calls): the closure the round-8 specs
# (w8t, w8v, w8w, w8z) compile, which covers every callee here
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
]
SOURCES = MINE + OTHERS
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000

# Stubbed helpers: original address -> (C function name or None, number of dword arguments logged).
HELPERS = {}


def helper(addr, cname, nargs):
    HELPERS[addr] = (cname, nargs)


# not converted (SIM_FN)
for _a, _n in [(0x100cada2, 3), (0x10064364, 2), (0x10062b7c, 4), (0x10063529, 1), (0x100ecce0, 1),
               (0x100ecea6, 6), (0x100f1e10, 6), (0x100f2f01, 6)]:
    helper(_a, None, _n)
# converted (called by name), and Sim_10062956 of this list when Sim_100622f0 calls it
for _a, _c, _n in [
        (0x10048287, 'Sim_10048287', 4), (0x1004e719, 'Sim_1004e719', 1),
        (0x100fefe0, 'Heap_FreeBlock', 1), (0x10011be7, 'Mono_BlankRows', 0), (0x100346ad, 'Mono_PutText', 4),
        (0x10016cb7, 'Sim_10016cb7', 3), (0x100c3668, 'Sim_100c3668', 1), (0x10015630, 'Sim_10015630', 4),
        (0x10062956, 'Sim_10062956', 0), (0x10065131, 'Sim_10065131', 0), (0x10062abd, 'Sim_10062abd', 1),
        (0x1009c929, 'Sim_1009c929', 1), (0x10062780, 'Sim_10062780', 0), (0x100aa351, 'Sim_100aa351', 1),
        (0x100aa39f, 'Sim_100aa39f', 1), (0x10086e27, 'Sim_10086e27', 2), (0x100cab7b, 'Sim_100cab7b', 3),
        (0x100cb299, 'Sim_100cb299', 3), (0x100ccb0d, 'Sim_100ccb0d', 2), (0x100fd850, 'Sim_100fd850', 2),
        (0x100fe770, 'Stream_ReadLine', 3), (0x100fd760, 'Crt_FClose', 1)]:
    helper(_a, _c, _n)

M16 = 0xffff
MASK = {(0x100346ad, 1): M16, (0x100346ad, 2): M16, (0x100346ad, 3): M16, (0x10015630, 1): M16,
        (0x10015630, 2): M16}
SARGS = {0x100346ad: (0,), 0x100fd850: (0, 1), 0x10086e27: (0,), 0x100ccb0d: (0,), 0x100ecea6: (4,),
         0x100f1e10: (4,), 0x100f2f01: (4,)}
DEREF = {0x1004e719: {0: 1}}
EFFECT = {}         # helper -> function(uc, call number, raw arguments, eax) that writes the helper's results
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
    if not getattr(m, '_w9g', False):
        m._w9g = True
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
    ST['seq'] = {}
    ST['seq_end'] = {}
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    ST.update(members=[], fifth=None, lines=[], rec=b'')
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def seq(addr, values, end=0):
    ST['seq'][addr] = list(values)
    ST['seq_end'][addr] = end


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
    number (the real helper writes out[4] only for a group whose dword +8 is set and whose fifth record passes
    its mask; the caller reads out[4] under the same condition, so it is always written here)."""
    for i, p in enumerate(ST['members']):
        put32(uc, raw[1] + 4 * i, p)
    if ST['fifth'] is not None:
        put32(uc, raw[1] + 16, ST['fifth'])
    return len(ST['members'])


def e_readline(uc, n, raw, eax):
    """Stream_ReadLine (fgets): the n-th line of the trial into the buffer, 0 at the end."""
    lines = ST['lines']
    if n < len(lines):
        uc.mem_write(raw[0], lines[n] + b'\0')
        return raw[0]
    return 0


def e_nmsave(uc, n, raw, eax):
    """Sim_10086e27: the trial's 0x66-byte record into the caller's buffer."""
    uc.mem_write(raw[1], ST['rec'])


def e_fclose(uc, n, raw, eax):
    """Crt_FClose, at the end of Lang_ReadIni: strtok's position [0x1017e884] is left pointing into the
    function's line buffer on the stack, whose address depends on the frame layout (the original's or the C
    compiler's). As spec_w8y.py does: like the stack addresses in the call log, such a pointer is replaced by a
    marker and the text it points at is logged instead, so the position in the line is still compared."""
    p = ur32(uc, 0x1017e884)
    if STACK_LO <= p <= STACK_HI:
        uc.mem_write(0x1017e884, struct.pack('<I', 0x00DEAD00))
        return [('strtok_pos', ustr(uc, p))], None
    return [], None


EFFECT.update({0x10016cb7: e_members, 0x100fe770: e_readline, 0x10086e27: e_nmsave, 0x100fd760: e_fclose})


# ---- generators -------------------------------------------------------------------------------------
STAR_BASE = 0x10473800
STAR_SIZE = 0x4fc3a
ENTRY = 0x13f


def entry(star, i):
    return STAR_BASE + star * STAR_SIZE + 0x3a + i * ENTRY


def g_2af0(ctx):
    """Mono_ShowMissions: a star table with 40 entries (the row window may reach past the count), the window
    globals around every branch of the window logic. The headline buffer is only printed when the state fills
    it (1 and above 4 leave it as the stack had it, which the checker cannot compare)."""
    r = begin(ctx, 0x100346ad, 0x10011be7)
    star = r.choice([0, 0, 1, 2, 3, 5])
    p = STAR_BASE + star * STAR_SIZE
    count = r.choice([0, 1, 2, 5, 10, 0x15, 0x16, 30, 40])
    w32(ctx, p, count, rint(ctx, -100, 100000), rint(ctx, -100, 100000),
        r.choice([0, 0, -5, rint(ctx, 1, 100000)]))
    state = r.choice([0, 0, 1, 2, 3, 4, 5, 0xff])
    w8(ctx, p + 0x38, state)
    w32(ctx, 0x10a3ade8, 0 if (state == 1 or state > 4) else r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1037305c, rint(ctx, 0, 200000))
    w32(ctx, 0x101303d8, r.choice([-1, -1, -1, 0, 0, 3, r.randrange(0, 40)]))
    w32(ctx, 0x101303d4, r.choice([-1, 0, 5, 0x14, 0x14, r.randrange(0, 40)]))
    w32(ctx, 0x1015d108, r.choice([0, 0, -1, -3, 1, 2, 0x14, 0x15, r.randrange(0, 40), r.randrange(0, 40)]))
    keys = [struct.unpack('<I', bytes(ctx.m.uc.mem_read(0x10130678 + 8 * i + 4, 4)))[0] for i in range(0x13)]
    for i in range(40):
        e = entry(star, i)
        w8(ctx, e, r.choice([0, 1, 2, 3, 4, 5, 5, 6, 6, 7, 0x40]))
        w8(ctx, e + 1, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 0x80]))
        w16(ctx, e + 2, r.choice(keys + [r.getrandbits(16), 0xffff]))
        w32(ctx, e + 0x67, rint(ctx, 0, 200000))
        w32(ctx, e + 0x6b, rint(ctx, -100, 100000))
        w32(ctx, e + 0x6f, r.choice([0, -1, rint(ctx, 1, 100000), rint(ctx, 1, 100000)]))
        w8(ctx, e + 0x73, r.choice([0, 1, 1]))
        s = text(ctx, r.choice([0, 1, r.randrange(2, 12), r.randrange(12, 40)]))
        ctx.m.uc.mem_write(e + 0xad, s + b'\0')
    return [star]


def g_6de7(ctx):
    r = begin(ctx, 0x10016cb7, 0x100c3668, 0x10015630)
    kind = r.randrange(0, 16)
    units = ptrtab(ctx, 8)
    for i, u in enumerate(units):
        w32(ctx, u + 4, r.choice([i, i, i, r.randrange(0, 8)]))
        w32(ctx, u + 0x1ae, r.choice([0, 0, 0, 1, 2, 3, 0x20, 0x40, 0x60, ctx.u32() & ~3, ctx.u32()]))
        w16(ctx, u + 0x1a6, r.choice([0, 0, 1, 2, 3, 4, 0xffff]))
        w32(ctx, u + 0x1b2, r.choice([0, 1, 1, 1, ctx.u32()]))
        w16(ctx, u + 0x1ac, r.choice([r.getrandbits(16), r.randrange(0, 100), r.randrange(0, 100), 0x7fff, 0x8000]))
        w32(ctx, u + 0x28, r.choice([0, newbuf(ctx, 0x20)]))
    key = r.choice([r.getrandbits(16), r.choice([0x200, 0x200, 0x100, 0x300]) | r.randrange(0, 8),
                    r.choice([0x200, 0x200, 0x100]) | r.randrange(0, 8), r.getrandbits(32), r.getrandbits(12)])
    group = 0x109b7d40 + kind * 0x6c
    w32(ctx, group, r.choice([-1, r.randrange(0, 8), r.randrange(0, 8), r.randrange(0, 8)]))
    w32(ctx, group + 8, r.choice([0, 1]))
    sel = r.randrange(0, 4)
    w32(ctx, 0x104727a0 + kind * 4, sel)
    e = entry(kind, sel)
    w32(ctx, e + 2, r.choice([4, 4, 0, 5, ctx.u32()]))
    w32(ctx, e + 0x79, r.choice([0, 0, 1, 3, 10, -1]))
    n = r.choice([0, 1, 2, 3, 3, 4, 4, 5])
    ST['members'] = [units[r.randrange(0, 8)] for _ in range(n)]
    ST['fifth'] = units[r.randrange(0, 8)]
    rets(0x100c3668, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    return [kind, key]


def g_c8cb(ctx):
    begin(ctx, 0x10048287)
    return [ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32()]


VALCHARS = b'abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789._ \\/-'


def ini_line(ctx, key, high):
    """key, a gap, then a value with at least one letter (class 0x103 is the CRT's alpha class: a digit does
    not stop the skip, and the original keeps skipping past the line's end into the stack without a letter);
    high bytes only when the table class is used (the isctype path reads an uninitialised word for them)."""
    r = ctx.rng
    lead = r.choice([b'', b'', b'', b' ', b'  ', b';', b'#'])
    gap = r.choice([b'', b' ', b' ', b'  ', b' \t', b'\t', b'=', b' = '])
    val = text(ctx, r.randrange(0, 12), VALCHARS)
    pre = bytes(r.choice(b' .-_019\x80\xff\xc3' if high else b' .-_019') for _ in range(r.randrange(0, 3)))
    core = text(ctx, 1, b'abcXYZ')
    tail = text(ctx, r.randrange(0, 10), VALCHARS + (b'\x80\xfe' if high else b''))
    return lead + key + gap + pre + core + val + tail + r.choice([b'\n', b'\n', b'\r\n', b''])


def g_f4e0(ctx):
    r = begin(ctx, 0x100fd850, 0x100fe770, 0x100fd760)
    dll = newbuf(ctx, 0x120)
    simf = newbuf(ctx, 0x120)
    mb = r.choice([1, 1, 2])
    w32(ctx, 0x1015965c, mb)
    rets(0x100fd850, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20), 0x12345678]))
    lines = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 8])):
        c = r.random()
        if c < 0.35:
            lines.append(ini_line(ctx, b'dll =', mb == 1))
        elif c < 0.7:
            lines.append(ini_line(ctx, b'sim file =', mb == 1))
        else:
            lines.append(r.choice([b'junk', b'dll', b'sim file', b'x = y', b'', b'dll=abc', b'[lang]',
                                   text(ctx, r.randrange(0, 60), VALCHARS)]) + r.choice([b'\n', b'']))
    ST['lines'] = [ln[:0xfe] for ln in lines]
    return [dll, simf]


def g_22f0(ctx):
    r = begin(ctx, 0x10062956, 0x10065131, 0x10064364, 0x10062b7c, 0x10062abd, 0x10063529)
    for a in (0x10144fa0, 0x10176938, 0x10144fac, 0x10144fa4, 0x10176950, 0x10144fa8):
        w32(ctx, a, r.choice([0, 1, 1, ctx.u32()]))
    obj = newbuf(ctx, 0xd00)
    w32(ctx, obj + 0xc4d, r.choice([0, 1, ctx.u32()]))
    return [obj] + [ctx.u32() for _ in range(6)]


def current_unit(ctx):
    r = ctx.rng
    recs = ptrtab(ctx, 3)
    who = r.randrange(0, 3)
    w32(ctx, 0x1017b8c8, who)
    return recs[who]


def g_2956(ctx):
    r = begin(ctx, 0x1009c929, 0x10062780)
    u = current_unit(ctx)
    blk = newbuf(ctx, 0xd00)
    w32(ctx, u + 0x28, blk)
    w32(ctx, blk + 0xc4d, r.choice([0, 1, 2, 3, 3, 4, ctx.u32()]))
    w32(ctx, 0x10165be0, ctx.u32())
    w32(ctx, 0x10165be4, ctx.u32())
    first = r.choice([r.randrange(0, 100), 9, 10, 49, 50, 99, 100, -1])
    seq(0x1009c929, [first] + [r.randrange(0, 10) for _ in range(12)], end=r.randrange(0, 10))
    return []


def g_ca00(ctx):
    r = begin(ctx, 0x100aa351, 0x100aa39f)
    u = current_unit(ctx)
    w32(ctx, u + 0x48, ctx.u32())
    w32(ctx, 0x10179ca8, r.choice([0, 0, 1, 2, ctx.u32()]))
    return []


def g_3041(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x28, newbuf(ctx, 0x600))
    return [obj, ctx.u32()]


def g_66ca(ctx):
    r = begin(ctx)
    nodes, keys = [], []
    for _ in range(r.choice([0, 1, 2, 4, 6])):
        nd = newbuf(ctx, 0x20)
        k = r.choice([r.randrange(0, 10), r.randrange(0, 10), ctx.u32()])
        w32(ctx, nd, k)
        nodes.append(nd)
        keys.append(k)
    for i, nd in enumerate(nodes):
        w32(ctx, nd + 0x14, nodes[i + 1] if i + 1 < len(nodes) else 0)
    w32(ctx, 0x1015dc14, nodes[0] if nodes else 0)
    return [r.choice(keys + [r.randrange(0, 10), 5, ctx.u32()])]


def g_6ebf(ctx):
    r = begin(ctx, 0x10086e27)
    obj = newbuf(ctx, 0x200)
    words = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x16, r.choice([0, words, words, words, words]))
    count = r.choice([0, 1, 2, 5, 8])
    items = newbuf(ctx, max(count, 1) * 0x1e + 0x20)
    rec = bytearray(rb(ctx, 0x66))
    rec[4:8] = struct.pack('<I', count)
    rec[8:12] = struct.pack('<I', r.choice([0, r.randrange(0, 8), r.randrange(0, 32)]))
    rec[0x10:0x14] = struct.pack('<I', items)
    s = text(ctx, r.randrange(0, 0x3f))
    rec[0x26:0x26 + len(s) + 1] = s + b'\0'
    ST['rec'] = bytes(rec)
    rets(0x10086e27, r.choice([0, 1, 1]))
    return [obj]


def g_a670(ctx):
    r = begin(ctx, 0x100cab7b, 0x100cada2, 0x100cb299)
    obj = newbuf(ctx, 0x300)
    blk = newbuf(ctx, 0x600)
    rec = newbuf(ctx, 0x60)
    w32(ctx, obj + 0x28, blk)
    w32(ctx, obj + 0x54, rec)
    w32(ctx, blk + 0x50c, r.choice([0, 1, 1]))
    who = r.randrange(0, 4)
    w32(ctx, obj + 4, who)
    w32(ctx, 0x1017b8c8, r.choice([who, who, r.randrange(0, 4)]))

    def sel():
        v = r.choice([0x100, 0x200, 0x400, 0x200, 0x800, 0]) | r.randrange(0, 6)
        if r.random() < 0.2:
            v |= 0x1000
        if r.random() < 0.1:
            v |= ctx.u32() & 0xffffe000
        return v
    w32(ctx, obj + 0x118, r.choice([-1, -1, sel(), sel(), sel()]))
    w32(ctx, obj + 0xf0, sel())
    flags = r.choice([1, 1, 1, 2, 4, 8, 0x20008, 0x40008, 0x10008, ctx.u32()])
    delta = r.choice([-1, 0, 1, 1, 2, -2])
    if flags == 1 and r32(ctx, obj + 0x118) == 0xffffffff:
        delta = 0           # the original goes on with an unset local otherwise (not provable)
    for a in (0x1015dc64, 0x1015dc68, 0x1017e5a8):
        w32(ctx, a, r.choice([0, 1, 3, 5]))
    for h in (0x100cab7b, 0x100cada2, 0x100cb299):
        rets(h, *[r.choice([-2, -1, 1, 0, 0, 5, -3]) for _ in range(7)])
    return [obj, delta, flags]


def g_d70f(ctx):
    r = begin(ctx, 0x100ecce0, 0x100ecea6, 0x100f1e10, 0x100f2f01, 0x100ccb0d)
    star = r.choice([0, 1, 2, 3])
    w32(ctx, 0x1013bb40, star)
    base = STAR_BASE + star * STAR_SIZE
    count = r.choice([0, 1, 2, 3, 5, 8])
    w32(ctx, base, count)
    k = r.choice([0, 1, 2])
    w32(ctx, 0x101c64f0, k)
    w32(ctx, 0x10151d60, 19, 20, 12)
    cut = r.choice([8, 12, 19, 20, 30, 60])
    w32(ctx, 0x10151d60 + 4 * k, cut)
    for i in range(count):
        e = entry(star, i)
        w8(ctx, e, r.choice([0, 1, 2, 3, 4, 5, 6, 7]))
        w8(ctx, e + 1, r.choice([0, 1, 2, 2, 3, 4, 8, 9, 0x40]))
        w8(ctx, e + 0x74, r.choice([0, 1, 1, 1]))
        w32(ctx, e + 0x75, r.choice([0, 1, 1, ctx.u32()]))
        n = min(0x90, r.choice([0, 1, r.randrange(1, cut), cut - 1, cut, cut + 1, cut + 2,
                                r.randrange(cut, 0x90)]))
        s = bytearray(text(ctx, n))
        if n >= cut:
            s[r.randrange(0, min(n, cut + 2))] = 0x20 if r.random() < 0.9 else s[0]
            if r.random() < 0.3 and n > cut + 1:
                s[cut + 1] = 0x20
        ctx.m.uc.mem_write(e + 0xad, bytes(s) + b'\0' * (0x92 - n))
    w32(ctx, 0x1015ec58, r.choice([0, 1, 1]))
    hb = newbuf(ctx, 0x20)
    w8(ctx, hb + 0xa, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b8c4, hb)
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    for a in (0x101c4994, 0x101c498c, 0x101c4988, 0x101c49a8):
        w32(ctx, a, cstr(ctx, text(ctx, r.randrange(0, 12))))
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj + 0xb4, rint(ctx, -100, 600))
    pos = ctx.buf(8, struct.pack('<2i', r.randint(-100, 600), r.randint(-100, 400)))
    seq(0x100ecce0, [r.choice([8, 10, 12, 16, 0, -4])])
    rets(0x100ccb0d, *[r.randrange(0, 200) for _ in range(4)])
    return [obj, pos, r.choice([0x10, newbuf(ctx, 0x20), ctx.u32()])]


SPECS = [
    dict(name='Mono_ShowMissions', orig=0x10012af0, c='Mono_ShowMissions', gen=g_2af0, trials=800, void=True),
    dict(name='Sim_10016de7', orig=0x10016de7, c='Sim_10016de7', gen=g_6de7, trials=1500),
    dict(name='Sim_1002c8cb', orig=0x1002c8cb, c='Sim_1002c8cb', gen=g_c8cb, trials=300, void=True),
    dict(name='Lang_ReadIni', orig=0x1005f4e0, c='Lang_ReadIni', gen=g_f4e0, trials=1000, void=True),
    dict(name='Sim_100622f0', orig=0x100622f0, c='Sim_100622f0', gen=g_22f0, trials=800, void=True),
    dict(name='Sim_10062956', orig=0x10062956, c='Sim_10062956', gen=g_2956, trials=800, void=True),
    dict(name='Sim_1006ca00', orig=0x1006ca00, c='Sim_1006ca00', gen=g_ca00, trials=300),
    dict(name='Sim_10073041', orig=0x10073041, c='Sim_10073041', gen=g_3041, trials=300, void=True),
    dict(name='Sim_100766ca', orig=0x100766ca, c='Sim_100766ca', gen=g_66ca, trials=500),
    dict(name='Sim_10086ebf', orig=0x10086ebf, c='Sim_10086ebf', gen=g_6ebf, trials=600, void=True),
    dict(name='Sim_100ca670', orig=0x100ca670, c='Sim_100ca670', gen=g_a670, trials=2000, void=True),
    dict(name='Sim_100cd70f', orig=0x100cd70f, c='Sim_100cd70f', gen=g_d70f, trials=1500, void=True),
]
