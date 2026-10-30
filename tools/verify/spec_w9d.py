# Differential tests for round 9, letter d (see difftest.py and docs/checker.md): the integer game code at
# 0x10016632, 0x10017b3e, 0x1001edd0, 0x1002ad50, 0x1003218b, 0x10047569, 0x10063037, 0x10065384, 0x1006561a,
# 0x100690d4, 0x1009a137, 0x100bfa33, 0x100e3c6a and 0x100e41cc (with its register-argument helpers 0x100e440b
# and 0x100e4463, which are static functions of the same C file and are proven through it).
#
# Callees are either run for real on both sides (the string helpers Str_Copy, Str_CopyN, Str_Length, the strtok
# Str_NextToken_100fd560, Str_CompareNoCase, Crt_ToUpper, Crt_StrChr, Sim_100fd290, Crt_IsCType, Crt_AToI: the
# original in the original run, the converted C in the C run, all verified already) or replaced, for the trials
# that ask for it, by a stub: an emulator code hook on the callee's first instruction that appends
# ('stub_<address>', argument words) to the checker's call log (compared between the two runs like the Windows
# calls; stack addresses shown as 'stack'), may write results through pointer arguments (the same bytes for both
# runs), answers with a value chosen by the generator and returns to the caller (no code bytes are patched).
#  * Callees that are not converted, or that another hook file patches (0x100ba7d5, 0x100f0205, 0x100bc280), are
#    called by both versions at their original address, where the stub sits.
#  * Converted callees are stubbed at their original address (reached by the original code through its thunk)
#    AND at the address of their C function in the test image (reached by the C code), under the same log name.
# Pointer arguments to texts are logged by content (SARGS), arguments read as 16 / 8 bits are masked (MASK).
# Floating point (0x10063037, 0x10065384, 0x1009a137: int <-> float conversions and one double product) is
# checked bit for bit; the x87 runs with the game's control word 0x27f. No float tolerance is used.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

MINE = [
    'sim/unsorted_1001/w9d_select.c', 'sim/unsorted_1001/w9d_misc.c', 'sim/unsorted_1002/w9d_cmdline.c',
    'sim/unsorted_1003/w9d_tree.c', 'sim/unsorted_1004/w9d_path.c', 'sim/unsorted_1006/w9d_hud.c',
    'sim/unsorted_1009/w9d_radar.c', 'sim/unsorted_100b/w9d_states.c', 'sim/unsorted_100e/w9d_decode.c',
]
# the converted callees of the functions above (by name in the C), and what those files need in turn
CALLEES = [
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1001/w3s_group.c', 'sim/util/lookup.c',
    'sim/unsorted_1001/w2f_misc.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1001/w5c_calls.c',
    'sim/unsorted_1001/w2e_tables.c', 'sim/win/w4u_cdaudio.c', 'sim/unsorted_100f/w7q_stream.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w6i_misc.c',
    'sim/unsorted_100f/w7q_parse.c', 'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w8A_stream.c',
    'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1003/w7m_misc.c',
    'sim/unsorted_1003/setters.c', 'sim/unsorted_100a/w6g_misc.c', 'sim/unsorted_100f/w8z_crt.c',
    'sim/unsorted_1006/w8w_hud.c', 'sim/unsorted_100c/w2f_objects.c', 'sim/unsorted_100c/w7q_objects.c',
    'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100d/w8z_project.c', 'sim/unsorted_1000/w5a_misc.c',
    'sim/unsorted_1006/w7n_misc.c', 'sim/unsorted_1009/w8x_radar.c', 'sim/util/field.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_1003/entry.c', 'sim/unsorted_100c/entries.c',
    'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_1000/w5z_calls.c',
    # what the files above need in turn (the link closure)
    'sim/unsorted_1010/w8A_lowio.c', 'sim/unsorted_1002/w5b_calls.c', 'sim/unsorted_1003/w5b_calls.c',
    'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1005/w3s_groups.c', 'sim/unsorted_1006/w5z_calls.c',
    'sim/unsorted_1006/w3p_box.c', 'sim/unsorted_1006/w3o_add.c', 'sim/unsorted_1006/w3t_misc.c',
    'sim/view/w2l_rects.c', 'sim/unsorted_100a/w2e_search.c', 'sim/unsorted_100b/w2f_tables.c',
    'sim/unsorted_100c/w5x_calls.c', 'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_1010/w6j_double.c',
    'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1002/w3q_collide.c',
    'sim/unsorted_100a/w2l_small.c', 'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_1010/w7s_locale.c',
    'sim/unsorted_1010/w6k_handles.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1010/w6k_misc.c',
    'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1010/w2f_ld12.c',
]
# the closed source sets of spec_w8t.py and spec_w8x.py (everything those files need), as the link base
BASE = [
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
    'sim/unsorted_1008/w8x_cells.c', 'sim/unsorted_1008/w8x_hud.c', 'sim/unsorted_1009/w8x_effects.c',
    'sim/unsorted_1009/w8x_views.c', 'sim/unsorted_1009/w8x_needles.c', 'sim/unsorted_1009/w8x_radar.c',
    'sim/unsorted_1009/w8x_misc.c', 'sim/unsorted_100a/w8x_scene.c',
    'sim/render/w3p_view.c', 'sim/render/w3q_vertex.c', 'sim/render/w3r_screen.c', 'sim/render/w7o_draw.c',
    'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1000/w4w_blocks.c', 'sim/unsorted_1001/w2h_tables.c',
    'sim/unsorted_1003/bitmap.c', 'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1005/w2k_misc.c', 'sim/unsorted_1008/w2l_grid.c',
    'sim/unsorted_1008/w3q_place.c', 'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w4u_misc.c',
    'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w5a_misc.c', 'sim/unsorted_1008/w5b_calls.c',
    'sim/unsorted_1008/w5x_calls.c', 'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w6f_camera.c',
    'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w7n_misc.c', 'sim/unsorted_1009/w2f_misc.c',
    'sim/unsorted_1009/w2j_misc.c', 'sim/unsorted_1009/w2j_tables.c', 'sim/unsorted_1009/w5y_misc.c',
    'sim/unsorted_1009/w6f_modes.c', 'sim/unsorted_100a/w2h_misc.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_1010/w2f_misc.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w6i_crt.c', 'sim/win/w4u_gdi.c',
]
SOURCES = MINE + sorted(set(CALLEES + BASE) - set(MINE))
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000

# not converted (or patched elsewhere): address -> number of argument words logged
STUBS = {
    0x100bc280: 3, 0x100648a0: 2, 0x10064d05: 3, 0x100ba7d5: 3, 0x100ecce0: 1, 0x100ecea6: 6, 0x100f1e10: 6,
    0x100f2f01: 6, 0x100f0205: 5, 0x100e91dc: 5,
}
# converted, stubbed on both sides: C name -> (original address, number of argument words logged)
DSTUBS = {
    'Sim_10052aaf': (0x10052aaf, 1), 'Sim_10012843': (0x10012843, 1), 'Sim_10016cb7': (0x10016cb7, 3),
    'Sim_10016a4a': (0x10016a4a, 1), 'Sim_10016a6b': (0x10016a6b, 4), 'Sim_1001555c': (0x1001555c, 3),
    'Sim_10017852': (0x10017852, 1), 'Cd_StopWait': (0x1009e4ae, 0),
    'Crt_Printf': (0x100fdc80, 1), 'Mono_PutTopLine': (0x10008390, 1), 'Mono_Enable': (0x10007a8e, 0),
    'Sim_1003c5a0': (0x1003c5a0, 1), 'Crt_Remove': (0x100fdb30, 1), 'Sim_1003217c': (0x1003217c, 0),
    'Sim_10034855': (0x10034855, 1), 'Sim_100a7880': (0x100a7880, 1),
    'Crt_SPrintf': (0x100fc780, 4),
    'Sim_10052be7': (0x10052be7, 1), 'Sim_10052c02': (0x10052c02, 1), 'Sim_100cba68': (0x100cba68, 0),
    'Sim_100d2669': (0x100d2669, 3), 'Sim_10065216': (0x10065216, 4),
    'Sim_100ccb0d': (0x100ccb0d, 2), 'Sim_100bb2b0': (0x100bb2b0, 2), 'Sim_1000ebfb': (0x1000ebfb, 5),
    'Sim_10065384': (0x10065384, 0), 'Sim_10068e77': (0x10068e77, 5),
    'Sim_1009aac9': (0x1009aac9, 1), 'Sim_1009a3b3': (0x1009a3b3, 1), 'Sim_1009a6f3': (0x1009a6f3, 1),
    'Sim_1001570f': (0x1001570f, 1), 'Sim_100c274a': (0x100c274a, 2), 'Sim_100ca417': (0x100ca417, 2),
    'Sim_10033c42': (0x10033c42, 1), 'Sim_100335cf': (0x100335cf, 2), 'Sim_100c48c9': (0x100c48c9, 1),
    'Sim_100c48fa': (0x100c48fa, 2), 'Sim_100bf9a1': (0x100bf9a1, 1), 'Sim_10009741': (0x10009741, 1),
}
M16 = 0xffff
MASK = {(0x10016a6b, 1): M16, (0x10016a6b, 2): M16, (0x10016a6b, 3): M16, (0x1001555c, 1): M16,
        (0x1001555c, 2): M16, (0x100c274a, 1): 0xff, (0x100ca417, 1): M16}
SARGS = {0x100bc280: (1, 2), 0x100ba7d5: (0,), 0x100bb2b0: (0,), 0x10065216: (2,), 0x1000ebfb: (0,),
         0x100fdc80: (0,), 0x10008390: (0,), 0x1003c5a0: (0,), 0x100fdb30: (0,), 0x100a7880: (0,),
         0x100fc780: (1, 2, 3)}
EFFECT = {}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def r32(ctx, addr):
    return ur32(ctx.m.uc, addr)


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def rfl(ctx, scale=1000.0):
    """A float bit pattern: ordinary values, zeros, small and big ones, both signs (no NaN, no infinity)."""
    r = ctx.rng
    c = r.random()
    if c < 0.55:
        return fbits(r.uniform(-scale, scale))
    if c < 0.75:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 16.0, 100.0, 256.0, -3.0, 5.0, 1e-7, 1e-30]))
    if c < 0.9:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-12, 12))
    return r.getrandbits(32) & 0xbf7fffff


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def rint(ctx, lo, hi):
    return ctx.rng.randint(lo, hi) & 0xffffffff


def ustr(uc, addr, limit=300):
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


# ---- stubs --------------------------------------------------------------------------------------
def _count(m, name):
    return sum(1 for e in m.api_log if e[0] == name)


def _stub_cb(m, addr, nargs):
    name = 'stub_%x' % addr

    def cb(uc, address, size, ud):
        if addr not in ST.get('active', ()):
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        args = []
        for k, v in enumerate(raw):
            if (addr, k) in MASK:
                v &= MASK[(addr, k)]
            if k in SARGS.get(addr, ()):
                v = ustr(uc, v)
            args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
        k = _count(m, name)
        m.api_log.append((name, tuple(args)))
        val = None
        if addr in EFFECT:
            val = EFFECT[addr](uc, k, raw)
        if val is None:
            rets = ST['rets'].get(addr) or [0]
            val = rets[k % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def begin(ctx, *active):
    """Installs the hooks (once per machine), activates the given stubs (addresses; converted ones by their
    original address) and sets the x87 state of the game (control word 0x27f). Must come first in a generator."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w9d', False):
        m._w9d = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        for cname, (addr, nargs) in DSTUBS.items():
            cb = _stub_cb(m, addr, nargs)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    for a in active:
        assert a in STUBS or a in {v[0] for v in DSTUBS.values()}, hex(a)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in active}
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def put32(uc, addr, v):
    uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))


def ptrtab(ctx, count, size=0x300):
    """count records in the pointer table 0x109e8b40, count in [0x1015dc64]."""
    recs = []
    for i in range(count):
        rec = newbuf(ctx, size)
        w32(ctx, 0x109e8b40 + 4 * i, rec)
        recs.append(rec)
    w32(ctx, 0x1015dc64, count)
    return recs


# ---- 0x10016632 -----------------------------------------------------------------------------------------
def e_members(uc, n, raw):
    """Sim_10016cb7: writes the trial's member pointers to out[] (and the fifth slot) and returns their number."""
    mem = ST['members']
    for i, p in enumerate(mem):
        put32(uc, raw[1] + 4 * i, p)
    if ST['fifth'] is not None:
        put32(uc, raw[1] + 16, ST['fifth'])
    return len(mem)


EFFECT[0x10016cb7] = e_members


def g_6632(ctx):
    r = begin(ctx, 0x10052aaf, 0x10012843, 0x10016cb7, 0x10016a4a, 0x10016a6b, 0x1001555c, 0x10017852)
    n = r.choice([2, 4, 6])
    recs = ptrtab(ctx, n, 0x200)
    for rec in recs:
        w32(ctx, rec + 0x14, r.choice([0, 1, 2, 2, 3, 3, 5]))
        w32(ctx, rec + 4, r.choice([0, 1, 2, 3, ctx.u32()]))
        w32(ctx, rec + 0xc, r.choice([0, 1, 2, 3]))
        w32(ctx, rec + 0x1ae, r.choice([0, 3, 0x43, ctx.u32()]))
    index = r.choice([0, 1, 2, 3, 7, 15])
    me = r.randrange(n)
    w32(ctx, 0x1017b8c8, me)
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    w32(ctx, 0x1015d120 + 4 * index, r.choice([0, 0, 1, ctx.u32()]))
    entry = r.choice([-1, 0, 1, 3, 5])
    w32(ctx, 0x104727a0 + 4 * index, entry)
    base = 0x10473800 + index * 0x4fc3a + entry * 0x13f
    wb(ctx, base + 0x3c, rb(ctx, 2))
    if r.random() < 0.7:
        w16(ctx, base + 0x3c, r.choice([0x400, 0x800, 0x10, 0x200, 0x1, 0x8000, 0, 0x30]))
    w32(ctx, base + 0xb7, r.choice([0, 1, 2, 3, ctx.u32()]))
    idx1 = r.choice([-1, -1, me, r.randrange(n), r.randrange(n)])
    idx2 = r.choice([me, r.randrange(n), r.randrange(n)])
    rets(0x10052aaf, idx1, r.choice([me, r.randrange(n)]))
    rets(0x10012843, r.choice([-1, idx2, idx2]))
    rets(0x10016a4a, r.choice([0, 1, 2, 3]))
    members = [r.choice(recs) for _ in range(r.choice([0, 1, 1, 2, 3, 4]))]
    fifth = r.random() < 0.6
    ST['members'] = members
    ST['fifth'] = r.choice(recs) if fifth else None
    w32(ctx, 0x109b7d48 + index * 0x6c, r.choice([1, ctx.u32() | 1]) if fifth else 0)
    return [index]


# ---- 0x10017b3e, 0x1001edd0 ------------------------------------------------------------------------------
def g_7b3e(ctx):
    begin(ctx)
    obj = newbuf(ctx, 0x260)
    w32(ctx, obj + 0x28, newbuf(ctx, 0x600))
    return [obj, ctx.u32()]


def g_edd0(ctx):
    r = begin(ctx, 0x1009e4ae)
    for a in (0x101321b0, 0x101321b4, 0x101321b8, 0x101321bc, 0x101321c0):
        w32(ctx, a, ctx.u32())
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    return [r.choice([0, 1, 2, 3, 4, 2, 4, 5, -1, -2, 0x7fffffff, 0x80000000, 100])]


# ---- 0x1002ad50 ------------------------------------------------------------------------------------------
CMD_WORDS = [0x10135518, 0x1013551c, 0x10135524, 0x1013552c, 0x10135534, 0x10135540, 0x10135548, 0x1013555c,
             0x1013556c]


def flipcase(r, s):
    return bytes((c ^ 0x20) if (65 <= c <= 90 or 97 <= c <= 122) and r.random() < 0.5 else c for c in s)


def cmd_token(ctx, words):
    r = ctx.rng
    c = r.random()
    if c < 0.2:
        return flipcase(r, r.choice(words))
    if c < 0.3:
        return bytes(r.choice(b'abcXYZ019_.,') for _ in range(r.randint(1, 8)))
    letter = r.choice(b'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefgjlmnopqrstvxz' + b'0?@[`~ ')
    tail = r.random()
    if tail < 0.3:
        return bytes([letter])
    if tail < 0.4:
        return bytes([letter]) + bytes(r.choice(b'abcXYZ019') for _ in range(r.randint(1, 4)))
    if letter in b'Oo':
        second = r.choice(b'EFeGx')
        return bytes([letter, second]) + (b'=' if r.random() < 0.8 else b':') + bytes(r.choice(b'ab9') for _ in range(r.randint(0, 5)))
    if letter in b'Ff':
        return bytes([letter]) + b'=' + r.choice([b'1', b'2', b'5', b'10', b'20', b'-3', b'7x', b'181', b'-1'])
    if letter in b'Vv':
        second = r.choice([b'G', b'g', b'=', b'=', b'x'])
        return bytes([letter]) + second + r.choice([b'=vga.dll', b'3', b'9', b'x', b'=', b'1234567890123', b'-5', b'\xe9'])
    if letter in b'XxZz':
        return bytes([letter]) + r.choice([b'=', b'=', b'-']) + bytes([r.choice(b'FGMPSHfgmpshXx0')])
    return bytes([letter]) + r.choice([b'=', b'=', b'-', b'']) + bytes(r.choice(b'abcXYZ019 ') for _ in range(r.randint(0, 10)))


def g_ad50(ctx):
    r = begin(ctx, 0x100fdc80, 0x10008390, 0x10007a8e, 0x1003c5a0, 0x100fdb30, 0x1003217c, 0x10034855,
              0x100a7880, 0x100bc280)
    uc = ctx.m.uc
    words = [ustr(uc, a) for a in CMD_WORDS]
    delim = ustr(uc, 0x101354e4)
    sep = bytes([delim[0]]) if delim else b' '
    toks = [cmd_token(ctx, words) for _ in range(r.choice([0, 1, 2, 3, 5, 8]))]
    for i in range(len(toks)):
        if toks[i] not in words and not any(toks[i].lower() == w.lower() for w in words) or r.random() < 0.7:
            if r.random() < 0.9 and not toks[i][:1] in (b'/', b'-'):
                toks[i] = r.choice([b'/', b'-', b'/', b'/']) + toks[i]
    if r.random() < 0.3:
        toks = [bytes(r.choice(b'abcXYZ019_.') for _ in range(r.randint(1, 10)))] + toks
    line = sep.join(toks)
    if r.random() < 0.3:
        line = sep * r.randint(1, 3) + line + sep * r.randint(0, 2)
    cmd = ctx.buf(len(line) + 1 + 16, line + b'\0' + rb(ctx, 16))
    saved = cstr(ctx, sep.join(cmd_token(ctx, words) for _ in range(r.choice([0, 1, 2]))))
    w32(ctx, 0x1017e884, saved)
    for a in (0x1017b940, 0x1017b944, 0x1017c8c4, 0x1017c8c0, 0x1017c8e8, 0x1017b954, 0x1017d5dc, 0x1017d5e0,
              0x1015f1e8, 0x1015dc28, 0x1015dc94, 0x1017d154, 0x101668f4, 0x1015ec58, 0x1017e6b4, 0x1017e6b0,
              0x1015dc24, 0x1017b8d4, 0x1013a00c, 0x1017b3bc, 0x1015d06c, 0x109e89e0):
        w32(ctx, a, r.choice([0, 1, ctx.u32()]))
    wb(ctx, 0x109e89e4, rb(ctx, 0x10))
    wb(ctx, 0x109e89fe, rb(ctx, 0x104))
    w32(ctx, 0x1015965c, r.choice([0, 1, 2, 3]))
    ctype = newbuf(ctx, 0x300)
    for i in range(0x180):
        w16(ctx, ctype + 2 * i, r.choice([0, 4, 4, 1, 2, 0x10, r.getrandbits(16)]))
    w32(ctx, 0x10159450, ctype + 0x100)
    opts = newbuf(ctx, 8)
    rest = zbuf(ctx, 0x200)
    rets(0x1003c5a0, r.choice([0, 1, 0x400000]))
    return [r.choice([cmd, cmd, cmd, cmd, cmd, 0]), opts, rest]


# ---- 0x1003218b ------------------------------------------------------------------------------------------
def tree(ctx, depth):
    r = ctx.rng
    n = newbuf(ctx, 0x30)
    w32(ctx, n + 0x18, r.choice([0, 1, 5, 100, rint(ctx, -20, 20), ctx.u32()]))
    for i in range(4):
        w32(ctx, n + 0x1c + 4 * i, tree(ctx, depth - 1) if depth > 0 and r.random() < 0.55 else 0)
    return n


def g_218b(ctx):
    r = begin(ctx)
    return [tree(ctx, r.choice([0, 1, 2, 3, 4])) if r.random() < 0.95 else 0]


# ---- 0x10047569 ------------------------------------------------------------------------------------------
def e_sprintf(uc, n, raw):
    """The stubbed sprintf writes a text made from its call number (the same for both runs)."""
    uc.mem_write(raw[0], b'text%08x\0' % mix(ST['seed'], n))
    return 13


EFFECT[0x100fc780] = e_sprintf


def g_7569(ctx):
    r = begin(ctx, 0x100fc780)
    wb(ctx, 0x1015ef50, rb(ctx, 0x50))
    wb(ctx, 0x1015f050, r.choice([b'\0', b'\0', b'C:\\HG\0', b'data\0', bytes([r.randrange(1, 256)]) + b'\0',
                                  b'\xe9ab\0']) + rb(ctx, 4))
    name = cstr(ctx, bytes(r.choice(b'abcXYZ019_.\\') for _ in range(r.choice([0, 1, 5, 12, 40, 70]))))
    return [name]


# ---- 0x10063037 ------------------------------------------------------------------------------------------
def e_project(uc, n, raw):
    """Sim_100d2669: the projected point goes back through the three pointers."""
    for i in range(3):
        put32(uc, raw[i], ST['proj'][i])
    return None


EFFECT[0x100d2669] = e_project


def g_3037(ctx):
    r = begin(ctx, 0x10052be7, 0x100648a0, 0x10052c02, 0x100cba68, 0x10064d05, 0x100d2669, 0x10065216)
    recs = ptrtab(ctx, 6, 0x100)
    holder = newbuf(ctx, 0x10)
    obj = newbuf(ctx, 0x200)
    w32(ctx, holder, obj)
    idx = r.randrange(6)
    kind = r.choice([0x100, 0x100, 0x100, 0x200, 0x400, 0x300, 0x500, 0xf00, 0])
    target = r.choice([kind | idx, kind | idx, kind | idx | 0x1000, kind | idx | (r.getrandbits(16) << 16), 0])
    w32(ctx, obj + 0x118, target)
    wb(ctx, obj + 0x104, struct.pack('<3I', rfl(ctx, 5000), rfl(ctx, 5000), rfl(ctx, 5000)))
    w32(ctx, 0x10176954, r.choice([0, 1, 6, 7, 13, -1, -7, -8, rint(ctx, -50, 50), ctx.u32()]))
    for i in range(-6, 7):
        w32(ctx, 0x109e857c + 4 * i, cstr(ctx, bytes(r.choice(b'abcXYZ019 ') for _ in range(r.randint(0, 14)))))
    w16(ctx, 0x109a8664 + idx * 0x54, r.choice([0x20, 0x20, 0, 0x21, r.getrandbits(16)]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    ST['proj'] = [rfl(ctx, 3e3), rfl(ctx, 3e3), rfl(ctx, 1e4)]
    if r.random() < 0.2:
        ST['proj'] = [fbits(r.uniform(-4e9, 4e9)), fbits(r.uniform(-3e9, 3e9)), rfl(ctx)]
    rets(0x100d2669, r.choice([0, 1, 1, 2]))
    rets(0x10052be7, r.choice([0, 1, 2, 3]))
    rets(0x10052c02, r.choice([0, 1, 2]))
    rets(0x100cba68, ctx.u32())
    return [holder]


# ---- 0x10065384, 0x1006561a ------------------------------------------------------------------------------
def g_5384(ctx):
    r = begin(ctx, 0x100ba7d5, 0x100ccb0d, 0x100ecce0, 0x100ecea6, 0x100f1e10, 0x100f2f01, 0x100bb2b0)
    # [0x109b9900] is a pointer to the font name; 0x101658d8 holds the message text
    w32(ctx, 0x109b9900, cstr(ctx, bytes(r.choice(b'abcXYZ019 ') for _ in range(r.randint(0, 20)))))
    wb(ctx, 0x101658d8, bytes(r.choice(b'abcXYZ019 ') for _ in range(r.randint(0, 20))) + b'\0')
    w8(ctx, 0x109b992e, ctx.u32())
    w32(ctx, 0x1014cec4, ctx.u32())
    w32(ctx, 0x109e3e10, r.choice([640, 320, 1024, rint(ctx, -100, 2000), ctx.u32()]))
    w32(ctx, 0x109e3e18, r.choice([480, 240, 768, rint(ctx, -100, 2000), ctx.u32()]))
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20), ctx.u32()]))
    rets(0x100ccb0d, r.choice([0, 1, 8, 100, 640, rint(ctx, -5000, 5000), ctx.u32()]))
    rets(0x100ecce0, r.choice([0, 1, 8, 16, rint(ctx, -500, 500), ctx.u32()]))
    return []


def g_561a(ctx):
    r = begin(ctx, 0x1000ebfb, 0x10065384)
    w32(ctx, 0x101658c0, r.choice([0, 1, 0x389, ctx.u32()]))
    t = r.choice([0, 10, 0x5a, 100, rint(ctx, -200, 200), ctx.u32()])
    w32(ctx, 0x101658c4, t)
    w32(ctx, 0x101658c8, r.choice([0x5a, t, t + 1, t - 1, rint(ctx, -200, 200), ctx.u32()]))
    w32(ctx, 0x101658cc, r.choice([0, 1, 1, 2, 10, ctx.u32()]))
    w32(ctx, 0x101658d4, r.choice([0, 1, 1, ctx.u32()]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 60, rint(ctx, -100, 300), ctx.u32()]))
    return []


# ---- 0x100690d4 ------------------------------------------------------------------------------------------
def g_90d4(ctx):
    r = begin(ctx, 0x10068e77)
    rets(0x10068e77, r.choice([0, 1, 2]), r.choice([0, 1, 5, ctx.u32()]))
    return [newbuf(ctx, 0x40), newbuf(ctx, 0x40), newbuf(ctx, 4), newbuf(ctx, 4), newbuf(ctx, 4)]


# ---- 0x1009a137 ------------------------------------------------------------------------------------------
def g_a137(ctx):
    r = begin(ctx, 0x1009aac9, 0x100ba7d5, 0x100f0205, 0x100e91dc, 0x100bb2b0, 0x1009a3b3, 0x1009a6f3)
    ptrtab(ctx, 4, 0x100)
    w32(ctx, 0x1017b8c8, r.randrange(4))
    obj = newbuf(ctx, 0x100)
    pane = newbuf(ctx, 0x40)
    w32(ctx, obj, pane)
    for off in (4, 8, 0xc, 0x10):
        w32(ctx, pane + off, r.choice([0, 1, 100, 319, 639, rint(ctx, -2000, 2000), ctx.u32()]))
    texts = newbuf(ctx, 0x40)
    w32(ctx, obj + 0x70, texts)
    w32(ctx, texts, cstr(ctx, bytes(r.choice(b'abcXYZ019 ') for _ in range(r.choice([0, 1, 3, 7, 8, 9, 20])))))
    w32(ctx, 0x1015d1ac, r.choice([4, 0, 1, 4, ctx.u32()]))
    w32(ctx, 0x1014cec0, ctx.u32())
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    return [obj]


# ---- 0x100bfa33 ------------------------------------------------------------------------------------------
def g_fa33(ctx):
    r = begin(ctx, 0x1001570f, 0x100c274a, 0x100ca417, 0x10033c42, 0x100335cf, 0x100c48c9, 0x100c48fa,
              0x100bf9a1)
    ptrtab(ctx, 4, 0x300)
    obj = newbuf(ctx, 0x260)
    w32(ctx, obj + 0x54, newbuf(ctx, 0x80))
    w16(ctx, obj + 0x1f8, r.choice(list(range(0x1c)) + [1, 1, 0xe, 0x10, 0x11, 0x16, 0x17, 0xffff, 0x1c, 0x8000,
                                                         r.getrandbits(16)]))
    w16(ctx, obj + 0x1aa, r.choice([0x200, 0x201, 0x203, 0, 3, 0x1234, r.getrandbits(16) & 0xfff03]))
    w16(ctx, obj + 0x1a8, r.choice([0x100, 0x101, 0, 5, r.getrandbits(16)]))
    w32(ctx, obj + 0x218, r.choice([0, 1, 2, 3, 7, -1, -3, rint(ctx, -40, 40)]))
    w16(ctx, obj + 0x22c, r.choice([0xffff, 0, 1, r.getrandbits(16)]))
    w32(ctx, obj + 0x23a, r.choice([-1, 0, 1, 5, 0x7fffffff, ctx.u32()]))
    for a in (0x1017e2a8, 0x1017e2ac, 0x1017e2b0):
        w32(ctx, a, r.choice([0, 0, 1, ctx.u32()]))
    rets(0x10033c42, r.choice([0, newbuf(ctx, 0x40), newbuf(ctx, 0x40)]))
    return [obj]


# ---- 0x100e3c6a ------------------------------------------------------------------------------------------
def g_3c6a(ctx):
    begin(ctx, 0x10009741)
    return [ctx.u32()]


# ---- 0x100e41cc ------------------------------------------------------------------------------------------
def block(ctx, length):
    """One packed block for the decoder: header, delta entries, codes."""
    r = ctx.rng
    rate = r.choice([0, 1, 2, 3, 1, 2])
    n = length >> {1: 1, 2: 2}.get(rate, 0)
    coding = r.choice([0, 1, 2, 3, 4, 5, 2, 3, 4, 5, 7, 15])
    if n == 0 and coding == 5:
        coding = r.choice([0, 2, 3, 4])
    if r.random() < 0.97 and coding > 5:
        coding = r.choice([2, 3, 4, 5])
    head = bytes([(rate << 6) | (r.getrandbits(2) << 4) | coding])
    if coding == 2:
        body = rb(ctx, 2) + rb(ctx, (n + 7) // 8)
    elif coding == 3:
        body = rb(ctx, 4) + rb(ctx, (n + 3) // 4)
    elif coding == 4:
        body = rb(ctx, 16) + rb(ctx, (n + 1) // 2)
    elif coding == 5:
        body = rb(ctx, n)
    else:
        body = b''
    return head + body, coding > 5


def g_41cc(ctx):
    r = begin(ctx)
    # lengths above 0x400 make the expansion scratch (0x10152794) overlap the block (0x10152b94), and blocks
    # above 0x401 bytes overwrite the delta entries (0x10152f95) while decoding; both are kept as the original
    length = r.choice([1, 2, 3, 4, 7, 8, 16, 17, 64, 100, 255, 256, 0x400, r.randint(1, 0x400), 0x401, 0x402,
                       0x404, 0x555, 0x558, 0x800, 0x801, 0xa00, 0x1000, r.randint(0x400, 0x1000)])
    count = r.choice([1, 1, 2, 3, 4])
    data = b''
    for _ in range(count):
        b, bad = block(ctx, length)
        data += b
        if bad:
            break
    src = ctx.buf(len(data) + 32, data + rb(ctx, 32))
    dst = newbuf(ctx, count * length + 16)
    state = ctx.buf(4, struct.pack('<i', r.choice([0, 0, 5, -5, 127, -127, 0x7f, r.randint(-200, 200),
                                                   r.randint(-2 ** 31, 2 ** 31 - 1)])))
    wb(ctx, 0x10152794, rb(ctx, 0x400 + 0x401 + 0x40))
    return [src, dst, count, length, state]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Sim_10016632', 0x10016632, g_6632, 1500, void=True),
    spec('Sim_10017b3e', 0x10017b3e, g_7b3e, 300, void=True),
    spec('Sim_1001edd0', 0x1001edd0, g_edd0, 500),
    spec('CmdLine_Parse', 0x1002ad50, g_ad50, 2500),
    spec('Sim_1003218b', 0x1003218b, g_218b, 500),
    spec('Sim_10047569', 0x10047569, g_7569, 400),
    spec('Sim_10063037', 0x10063037, g_3037, 1200, void=True),
    spec('Sim_10065384', 0x10065384, g_5384, 800, void=True),
    spec('Sim_1006561a', 0x1006561a, g_561a, 800, void=True),
    spec('Sim_100690d4', 0x100690d4, g_90d4, 300),
    spec('Sim_1009a137', 0x1009a137, g_a137, 800, void=True),
    spec('Sim_100bfa33', 0x100bfa33, g_fa33, 1500, void=True),
    spec('Sim_100e3c6a', 0x100e3c6a, g_3c6a, 200, void=True),
    # the original's push ds / push es / push ds / pop es ... pop es / pop ds (no-ops in flat 32-bit mode) cannot
    # run in the emulator (the ES load makes every es: write fault); they are nop'd like Timer_Step's in spec_core
    spec('Sim_100e41cc', 0x100e41cc, g_41cc, 2000,
         nop=[0x100e41d2, 0x100e41d3, 0x100e41d7, 0x100e41d8, 0x100e4407, 0x100e4408]),
]
