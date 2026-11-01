# Differential tests for round 8, letter u (see difftest.py and docs/checker.md).
#
# Helpers are replaced in the emulator by stubs where their own world would be large: a code hook at the helper's
# address appends ('stub_<address>', arguments) to the checker's call log (compared between the original and the
# C run like the Windows calls), may write the helper's results through its pointer arguments (the same bytes for
# both runs), and returns to the caller with a value chosen by the generator (code bytes are not patched).
#  * Helpers that are not converted (called with SIM_FN by the C code) are stubbed at their own address, which
#    both runs reach.
#  * Converted helpers (and the functions of this list that others here call) are stubbed at their original
#    address (reached by the original code) AND at their C function in the test image (reached by the C code),
#    under the same log name. Each is proven by its own spec. Stubs are active only in the trials that ask for
#    them, so a function of this list is tested for real in its own spec and stubbed as a callee elsewhere.
#  * Helpers that return in st(0) answer through a small trampoline (`fld dword/qword [slot]; ret`).
# Stack addresses are logged as 'stack'; texts are logged by content (SARGS), small records by their bytes (DEREF),
# printf-style calls by the formatted text, and 16-bit arguments masked (MASK). Text handed to the Windows calls
# OutputDebugStringA / MessageBoxA (a stack buffer) is logged by content by an extra hook on their import stubs.
# Everything else runs for real on both sides: sprintf / _vsnprintf / fabs / the 0x10084766 table lookup of the
# DLL (x87 control word 0x27f, as in the game), and the converted string, copy and matrix helpers.
# All floating point here is exact: no float tolerance is used.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

MINE = [
    'sim/unsorted_1003/w8u_banner.c', 'sim/unsorted_1002/w8u_debug.c', 'sim/unsorted_1002/w8u_cache.c', 'sim/unsorted_1002/w8u_debris.c',
    'sim/unsorted_1002/w8u_collide.c', 'sim/unsorted_1003/w8u_collide.c', 'sim/unsorted_1003/w8u_debug.c',
    'sim/unsorted_1003/w8u_plugin.c', 'sim/unsorted_1003/w8u_display.c', 'sim/unsorted_1003/w8u_misc.c',
    'sim/unsorted_1004/w8u_camera.c', 'sim/unsorted_1004/w8u_files.c',
]
SOURCES = MINE + [
    # existing converted code the functions above call (and what that calls)
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/core/w2b_timers.c', 'sim/render/w2n_polygon.c',
    'sim/render/w2n_project.c', 'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2i_misc.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w2l_fields.c',
    'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/unsorted_1000/w5z_calls.c',
    'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w2h_objects.c',
    'sim/unsorted_1001/w2i_misc.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w4u_cd.c',
    'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2e_tables.c',
    'sim/unsorted_1002/w2f_misc.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2i_misc.c',
    'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2k_misc.c',
    'sim/unsorted_1002/w2l_tables.c', 'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1002/w3q_collide.c', 'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w6d_calls.c', 'sim/unsorted_1002/w7l_collide.c',
    'sim/unsorted_1002/w7l_misc.c', 'sim/unsorted_1003/records.c', 'sim/unsorted_1003/slots.c',
    'sim/unsorted_1003/w2b_misc.c', 'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1003/w2e_mono.c',
    'sim/unsorted_1003/w2f_misc.c', 'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1003/w2h_misc.c',
    'sim/unsorted_1003/w2j_misc.c', 'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w3q_grid.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1003/w4v_misc.c', 'sim/unsorted_1003/w5a_misc.c',
    'sim/unsorted_1003/w5b_calls.c', 'sim/unsorted_1003/w5x_calls.c', 'sim/unsorted_1003/w6d_calls.c',
    'sim/unsorted_1003/w6e_misc.c', 'sim/unsorted_1003/w6e_state.c', 'sim/unsorted_1003/w7m_misc.c',
    'sim/unsorted_1003/w7m_state.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/state.c',
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w2h_record.c',
    'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/w2k_flags.c',
    'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_1004/w3o_misc.c', 'sim/unsorted_1004/w3p_net.c',
    'sim/unsorted_1004/w3s_records.c', 'sim/unsorted_1004/w4w_net.c', 'sim/unsorted_1004/w5a_misc.c',
    'sim/unsorted_1004/w5c_calls.c', 'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_1004/w6e_misc.c',
    'sim/unsorted_1004/w7m_misc.c', 'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_1005/w2h_tables.c',
    'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1006/w5b_calls.c',
    'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_1006/w6e_misc.c',
    'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1007/w2f_misc.c',
    'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1007/w3p_net.c', 'sim/unsorted_1007/w5a_misc.c',
    'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w2l_grid.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3q_place.c',
    'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1008/w4v_fade.c',
    'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w5y_misc.c', 'sim/unsorted_1008/w6f_cells.c',
    'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w7o_cells.c', 'sim/unsorted_1009/w2e_copy.c',
    'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1009/w6g_segment.c', 'sim/unsorted_1009/w7o_misc.c',
    'sim/unsorted_100a/w2g_misc.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w3t_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100a/w7o_nodes.c',
    'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w2h_misc.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/nodes.c', 'sim/unsorted_100d/state.c',
    'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_100d/w5b_calls.c', 'sim/unsorted_100d/w5x_calls.c', 'sim/unsorted_100d/w5z_calls.c',
    'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_100f/w7q_find.c',
    'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1010/w6i_crt.c',
    'sim/unsorted_1010/w6j_stream.c', 'sim/unsorted_1010/w6k_handles.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c', 'sim/view/w2d_view.c',
    'sim/view/w2l_lists.c', 'sim/view/w3r_matrix.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c',
    'sim/win/w4v_heap.c', 'sim/unsorted_1004/w4u_heap.c', 'sim/unsorted_1008/w5c_calls.c',
    'sim/unsorted_100f/w2j_str.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
STUB_BASE = 0x00F20000  # difftest's Windows import stubs (16 bytes each, in import order)
FRET = 0x00AF7000       # trampolines: +0 fld dword [FSLOT]; ret   +0x10 fld qword [FSLOT]; ret
FSLOT = 0x00AF7F00
CODE = 0x00AF8000       # targets of indirect calls (a `ret`, with a logging hook)

# helpers that are not converted: address -> (number of dword arguments logged, kind: 'i' eax / 'f' float /
# 'd' double in st(0))
STUBS = {
    0x100fd760: (1, 'i'),       # fclose
    0x100ba7d5: (3, 'i'), 0x100bc280: (5, 'i'), 0x100a836e: (4, 'i'), 0x10029b1f: (1, 'i'), 0x1002f021: (4, 'i'),
    0x1003650d: (1, 'i'), 0x1003aecb: (3, 'i'), 0x100897c7: (1, 'i'),
    0x100feb20: (3, 'i'),       # fseek
    0x100fe830: (4, 'i'),       # fread
    0x101049c0: (1, 'i'),       # _close
    0x10105750: (3, 'i'),       # _write (the data is logged)
    0x10105b50: (3, 'i'),       # _read (fills the buffer)
    0x1006788c: (2, 'i'), 0x10044d0b: (4, 'i'), 0x100456fc: (0, 'i'), 0x100b8d6c: (6, 'i'),
    0x100ed491: (3, 'i'), 0x100ed6aa: (2, 'i'),
}
# converted helpers stubbed on both sides: C name -> (original address, number of dword arguments logged, kind)
DSTUBS = {
    'Sim_100fd850': (0x100fd850, 2, 'i'), 'Crt_FPrintf': (0x100fd7e0, 2, 'i'), 'Crt_FPuts': (0x100fe390, 2, 'i'),
    'Crt_FFlush': (0x100fe240, 1, 'i'), 'Crt_FTell': (0x100fe970, 1, 'i'), 'Crt_FindFirst': (0x100fe470, 2, 'i'),
    'Crt_FindNext': (0x100fe5a0, 2, 'i'), 'File_FindClose': (0x100fe6c0, 1, 'i'),
    'Mono_PrintText': (0x1000847d, 1, 'i'),
    'Plugin_ProbeRaster': (0x10036027, 2, 'i'), 'Sim_100361d8': (0x100361d8, 2, 'i'),
    'Sim_10034777': (0x10034777, 1, 'i'), 'Sim_10027f33': (0x10027f33, 2, 'i'), 'Sim_10027cce': (0x10027cce, 1, 'i'),
    'Sim_10027fb8': (0x10027fb8, 1, 'i'), 'Sim_1007df6c': (0x1007df6c, 4, 'i'), 'Sim_100a8068': (0x100a8068, 3, 'i'),
    'Sim_10028fb0': (0x10028fb0, 2, 'i'), 'Sim_100a9c25': (0x100a9c25, 1, 'i'), 'Sim_10009a53': (0x10009a53, 2, 'i'),
    'Sim_10088a2d': (0x10088a2d, 1, 'i'), 'Sim_1000946e': (0x1000946e, 4, 'f'), 'Sim_1002d961': (0x1002d961, 3, 'd'),
    'Sim_10038d65': (0x10038d65, 1, 'i'), 'Sim_1009c929': (0x1009c929, 1, 'i'), 'Sim_100a9d47': (0x100a9d47, 4, 'i'),
    'Sim_100aa127': (0x100aa127, 5, 'i'), 'Sim_1002990f': (0x1002990f, 1, 'i'), 'Sim_10009eb3': (0x10009eb3, 2, 'f'),
    'Sim_10009d7e': (0x10009d7e, 2, 'f'), 'Sim_1002e569': (0x1002e569, 3, 'i'), 'Sim_1009cbde': (0x1009cbde, 1, 'f'),
    'Sim_1002e9f5': (0x1002e9f5, 4, 'i'), 'Sim_1002ed0b': (0x1002ed0b, 4, 'i'), 'Sim_1009cfd4': (0x1009cfd4, 6, 'i'),
    'Sim_10031ed3': (0x10031ed3, 3, 'i'), 'Sim_1002e790': (0x1002e790, 3, 'i'), 'Sim_100381d0': (0x100381d0, 1, 'i'),
    'Sim_10038dd3': (0x10038dd3, 6, 'i'), 'Sim_100aae0f': (0x100aae0f, 2, 'i'), 'Sim_1003913b': (0x1003913b, 2, 'i'),
    'Sim_100aa351': (0x100aa351, 1, 'i'), 'Sim_100ab186': (0x100ab186, 2, 'i'), 'Sim_10009408': (0x10009408, 2, 'i'),
    'Sim_1000942e': (0x1000942e, 2, 'i'), 'Sim_1003b485': (0x1003b485, 1, 'i'), 'Sim_1003b449': (0x1003b449, 2, 'i'),
    'Sim_1003b45d': (0x1003b45d, 2, 'i'), 'Sim_1000ebfb': (0x1000ebfb, 5, 'i'), 'Sim_10041203': (0x10041203, 2, 'i'),
    'Heap_AllocRaw': (0x100b659a, 1, 'i'), 'Sim_1008a40a': (0x1008a40a, 1, 'i'), 'Sim_1008a37f': (0x1008a37f, 1, 'i'),
    'Sim_10084a94': (0x10084a94, 3, 'd'), 'Sim_1004409f': (0x1004409f, 1, 'i'), 'Sim_10045308': (0x10045308, 6, 'i'),
    'Sim_10047bcd': (0x10047bcd, 1, 'f'), 'Sim_10044909': (0x10044909, 1, 'i'), 'Sim_100448cd': (0x100448cd, 1, 'i'),
    'Sim_100448ff': (0x100448ff, 0, 'i'), 'Sim_10044c7a': (0x10044c7a, 0, 'i'), 'Sim_100453a3': (0x100453a3, 0, 'i'),
    'Sim_100a94fa': (0x100a94fa, 0, 'i'), 'Sim_10044255': (0x10044255, 0, 'i'), 'Sim_1004487c': (0x1004487c, 1, 'f'),
    'Sim_10109980': (0x10109980, 3, 'i'), 'File_GetLength': (0x10100730, 1, 'i'), 'Sim_1002828e': (0x1002828e, 2, 'i'),
    # functions of this list, stubbed where another one calls them
    'Debug_LogText': (0x10027b19, 1, 'i'), 'Cache_UseItem': (0x100282d4, 3, 'i'), 'Sim_10029421': (0x10029421, 1, 'i'),
    'Sim_100291e3': (0x100291e3, 1, 'i'), 'Debug_AppendLog': (0x100347d5, 1, 'i'),
    'Plugin_ProbeAll': (0x10035f90, 1, 'i'), 'Plugin_LoadAll': (0x10036466, 0, 'i'),
    'Sim_10043f1a': (0x10043f1a, 0, 'i'), 'Sim_100451a2': (0x100451a2, 0, 'i'), 'Sim_1004548f': (0x1004548f, 5, 'i'),
    'File_ReadAll': (0x10046ac4, 4, 'i'), 'File_LoadWhole': (0x100461e8, 2, 'i'),
    'Sim_1008d36c': (0x1008d36c, 3, 'i'), 'Heap_Free_100461d0': (0x100461d0, 1, 'i'),
    # verified with a float tolerance only (spec_w3r.py): stubbed so that the callers' own work is checked exactly
    'Mat3x4_Multiply': (0x10060008, 3, 'i'),
}
SARGS = {0x100ba7d5: (0,), 0x100bc280: (1, 2), 0x1003650d: (0,), 0x100b8d6c: (5,), 0x100fd850: (0, 1),
         0x100fe390: (0,), 0x100fe470: (0,), 0x1000847d: (0,), 0x10036027: (0,), 0x100361d8: (0,), 0x10034777: (0,),
         0x1007df6c: (0,), 0x100aae0f: (1,), 0x1000ebfb: (0,), 0x10027b19: (0,), 0x100347d5: (0,), 0x10109980: (0,),
         0x10046ac4: (0,), 0x100461e8: (0,)}
DEREF = {0x10009a53: {1: 0x30}, 0x1002d961: {}, 0x10009eb3: {1: 0x38}, 0x10009d7e: {1: 0x38}, 0x1002e569: {1: 0x38},
         0x1009cbde: {0: 0x38}, 0x10047bcd: {0: 0x10}, 0x1004487c: {0: 0x18}, 0x1002e9f5: {}, 0x10027f33: {1: 4},
         0x100a8068: {}, 0x100a836e: {1: 4}, 0x10027fb8: {}, 0x10060008: {0: 0x30, 1: 0x30},
         0x1008d36c: {0: 0x14}, 0x100ed491: {0: 0x14}}
MASK = {(0x100a836e, 2): 0xffff, (0x100a8068, 2): 0xffff, (0x10009408, 1): 0xffff, (0x1000942e, 1): 0xffff}
EFFECT = {}             # helper -> function(uc, call number, raw arguments, esp) -> (extra log items, eax or None)
ST = {'active': set(), 'rets': {}}


# ---- helpers ------------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def w8(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, bytes(v & 0xff for v in vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def r32(ctx, addr):
    return ur32(ctx.m.uc, addr)


def rf(ctx, addr):
    return struct.unpack('<f', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


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


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def bitsf(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xffffffff))[0]


def f32(x):
    try:
        return bitsf(fbits(x))
    except OverflowError:
        return float('inf') if x > 0 else float('-inf')


def wf(ctx, addr, *vals):
    w32(ctx, addr, *[fbits(v) for v in vals])


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def rfl(ctx, spread=1000.0, special=()):
    """A float bit pattern: ordinary values, the given values (and their neighbours), zeros, small and big ones,
    now and then a quiet NaN or an infinity (never a signalling NaN)."""
    r = ctx.rng
    c = r.random()
    if c < 0.45:
        return fbits(r.uniform(-spread, spread))
    if c < 0.6 and special:
        return (fbits(r.choice(special)) + r.choice([0, 0, 1, -1])) & 0xffffffff
    if c < 0.72:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 100.0, -100.0, 1e-7, -1e-7]))
    if c < 0.95:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-8, 8))
    return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])


def rname(ctx, maxlen, alpha=b'abcXYZ_09.'):
    r = ctx.rng
    return bytes(r.choice(alpha) for _ in range(r.choice([0, 1, 3, 5, r.randrange(0, maxlen + 1)])))


# ---- printf-style formatting of the logged texts ----------------------------------------------------------
def _fmt(uc, fmt, words):
    """A small printf for the formats of these functions (%s %c %d %u %x with flags and widths)."""
    if not isinstance(fmt, bytes):
        return ('bad format', fmt)
    out = b''
    i = k = 0
    while i < len(fmt):
        c = fmt[i:i + 1]
        if c != b'%':
            out += c
            i += 1
            continue
        j = i + 1
        while j < len(fmt) and fmt[j:j + 1] in b'0123456789l-.':
            j += 1
        spec = fmt[i + 1:j].replace(b'l', b'')
        conv = fmt[j:j + 1]
        if conv == b'%':
            out += b'%'
            i = j + 1
            continue
        v = words[k] if k < len(words) else 0
        k += 1
        if conv == b's':
            s = ustr(uc, v)
            if not isinstance(s, bytes):
                s = b'<bad %x>' % v
        elif conv == b'c':
            s = bytes([v & 0xff])
        elif conv == b'd':
            s = b'%d' % (v - (1 << 32) if v & 0x80000000 else v)
        elif conv == b'u':
            s = b'%d' % v
        else:
            s = b'%x' % v
        width = int(spec.split(b'.')[0]) if spec.split(b'.')[0].lstrip(b'-').isdigit() else 0
        out += s.ljust(-width) if width < 0 else s.rjust(width)
        i = j + 1
    return out


def e_fprintf(uc, n, raw, esp):
    words = list(struct.unpack('<8I', bytes(uc.mem_read(esp + 12, 32))))
    return [_fmt(uc, ustr(uc, raw[1]), words)], None


def e_bc280(uc, n, raw, esp):
    words = list(struct.unpack('<6I', bytes(uc.mem_read(esp + 12, 24))))
    return [_fmt(uc, ustr(uc, raw[1]), words)], None


def _find_at(uc, k, raw):
    names = ST['find']
    if k < len(names):
        uc.mem_write(raw[1] + 0x14, names[k] + b'\0')
        return [names[k]], True
    return [], False


def e_find(uc, n, raw, esp):
    """_findfirst: the first name of the plan into the record's name (+0x14) and the handle; -1 when none."""
    extra, ok = _find_at(uc, 0, raw)
    return extra, (ST['handle'] if ok else 0xffffffff)


def e_findnext(uc, n, raw, esp):
    """_findnext (call n): name n + 1 of the plan, 0; -1 when none is left."""
    extra, ok = _find_at(uc, n + 1, raw)
    return extra, (0 if ok else 0xffffffff)


def e_fill(uc, n, raw, esp):
    """fread(buffer, size, count, stream) / _read(handle, buffer, n): deterministic bytes; the planned count."""
    if len(raw) == 4:
        buf, size = raw[0], (raw[1] * raw[2]) & 0xffffffff
    else:
        buf, size = raw[1], raw[2]
    size = min(size, 0x400)
    try:
        uc.mem_write(buf, bytes((mix(ST['seed'], n, i // 4) >> (8 * (i % 4))) & 0xff for i in range(size)))
    except Exception:
        return [('unwritable', buf)], None
    return [], None


def e_write(uc, n, raw, esp):
    try:
        data = bytes(uc.mem_read(raw[1], min(raw[2], 0x400))).hex()
    except Exception:
        data = ('bad', raw[1])
    return [data], None


def e_open(uc, n, raw, esp):
    """_open(name, flags, mode): mode is only read with O_CREAT (0x100); log it only then."""
    if raw[1] & 0x100:
        return [], None
    return [('mode unused',)], None


def e_946e(uc, n, raw, esp):
    for k in range(3):
        uc.mem_write(raw[1 + k], struct.pack('<I', ST['pos'][k]))
    return [], None


def e_seg(uc, n, raw, esp):
    """Sim_1002e569 cuts the segment: new end and length, and the hit data at 0x1015dd40.."""
    h = mix(ST['seed'], n, 7)
    uc.mem_write(raw[1] + 0xc, struct.pack('<4I', *[mix(h, k) & 0x3fffffff for k in range(4)]))
    uc.mem_write(0x1015dd40, struct.pack('<3I', *[mix(h, k, 1) for k in range(3)]))
    return [], None


def e_cfd4(uc, n, raw, esp):
    pair = ST['cfd4'][n % len(ST['cfd4'])]
    uc.mem_write(raw[4], struct.pack('<I', pair[0]))
    uc.mem_write(raw[5], struct.pack('<I', pair[1]))
    return [], None


def e_dd3(uc, n, raw, esp):
    for k in range(3):
        uc.mem_write(raw[k], struct.pack('<I', ST['dd3'][k]))
    return [], None


def e_5308(uc, n, raw, esp):
    for k in range(6):
        uc.mem_write(raw[k], struct.pack('<I', mix(ST['seed'], n, k) & 0x47ffffff))
    return [], None


def e_readall(uc, n, raw, esp):
    uc.mem_write(raw[1], struct.pack('<I', ST['ra'][0]))
    uc.mem_write(raw[2], struct.pack('<I', ST['ra'][1]))
    return [], None


def e_fb8(uc, n, raw, esp):
    """Purging takes the head of the purge list: the next head of the plan."""
    heads = ST['purge']
    uc.mem_write(0x109f13f0, struct.pack('<I', heads[n] if n < len(heads) else 0))
    return [], None


def e_mat(uc, n, raw, esp):
    """Mat3x4_Multiply(a, b, out): reads both inputs first (out may be b), writes a product made from them."""
    h = mix(ST['seed'], n, *struct.unpack('<24I', bytes(uc.mem_read(raw[0], 0x30)) + bytes(uc.mem_read(raw[1], 0x30))))
    uc.mem_write(raw[2], struct.pack('<12I', *[mix(h, k) & 0xbfffffff for k in range(12)]))
    return [], None


def e_set(addr):
    def e(uc, n, raw, esp):
        uc.mem_write(addr, struct.pack('<I', mix(ST['seed'], n, addr)))
        return [], None
    return e


EFFECT.update({0x100fd7e0: e_fprintf, 0x100bc280: e_bc280, 0x100fe470: e_find, 0x100fe5a0: e_findnext,
               0x100fe830: e_fill, 0x10105b50: e_fill, 0x10105750: e_write,
               0x10109980: e_open, 0x1000946e: e_946e, 0x1002e569: e_seg, 0x1009cfd4: e_cfd4, 0x10038dd3: e_dd3,
               0x10045308: e_5308, 0x10046ac4: e_readall, 0x10027fb8: e_fb8, 0x10060008: e_mat,
               0x10036027: e_set(0x1015e56c), 0x100361d8: e_set(0x1015e574)})


# ---- the hooks ------------------------------------------------------------------------------------------
def _log_args(uc, key, nargs, esp):
    raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
    if key == 0x10109980 and not raw[1] & 0x100:
        raw = raw[:2]
    args = []
    for k, v in enumerate(raw):
        if (key, k) in MASK:
            v &= MASK[(key, k)]
        if k in SARGS.get(key, ()):
            v = ustr(uc, v)
        elif k in DEREF.get(key, {}):
            try:
                v = bytes(uc.mem_read(v, DEREF[key][k]))
            except Exception:
                v = ('bad', v)
        args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
    return raw, args


def _stub_cb(m, key, nargs, kind):
    def cb(uc, address, size, ud):
        if key not in ST['active']:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw, args = _log_args(uc, key, nargs, esp)
        name = 'stub_%x' % key
        n = sum(1 for e in m.api_log if e[0] == name)
        rets = ST['rets'].get(key) or [0]
        val = rets[n % len(rets)]
        if key in EFFECT:
            extra, v2 = EFFECT[key](m.uc, n, raw, esp)
            args += extra
            if v2 is not None:
                val = v2
        m.api_log.append((name, tuple(args)))
        if kind == 'f':
            m.uc.mem_write(FSLOT, struct.pack('<I', val & 0xffffffff))
            uc.reg_write(UC_X86_REG_EIP, FRET)
        elif kind == 'd':
            m.uc.mem_write(FSLOT, struct.pack('<d', val))
            uc.reg_write(UC_X86_REG_EIP, FRET + 0x10)
        else:
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, ur32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        k = (address - CODE) // 16
        nargs = ST['code_nargs'].get(k, 0)
        raw = struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs))) if nargs else ()
        n = sum(1 for e in m.api_log if e[0] == 'code_%d' % k)
        m.api_log.append(('code_%d' % k, tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)))
        rets = ST['code_rets'].get(k) or [0]
        uc.reg_write(UC_X86_REG_EAX, rets[n % len(rets)] & 0xffffffff)
    return cb


def _text_api_cb(m, name, which):
    """Logs the texts a Windows call reads (the checker itself logs stack pointers as 'stack')."""
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        m.api_log.append((name + '_text', tuple(ustr(uc, ur32(uc, esp + 4 + 4 * k)) for k in which)))
    return cb


def begin(ctx, *active):
    """Installs the hooks (once per machine), activates the given stubs (addresses), chooses their answers and
    sets the x87 state of the game (control word 0x27f). Must be the first thing a generator does."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w8u', False):
        m._w8u = True
        for addr, (nargs, kind) in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs, kind), None, addr, addr)
        for cname, (addr, nargs, kind) in DSTUBS.items():
            cb = _stub_cb(m, addr, nargs, kind)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE, CODE + 16 * 64 - 1)
        for k, (key, _) in enumerate(m.api_names):
            nm = key.split('!')[1]
            if nm == 'OutputDebugStringA':
                m.raw.hook_add(UC_HOOK_CODE, _text_api_cb(m, nm, (0,)), None, STUB_BASE + 16 * k, STUB_BASE + 16 * k)
            if nm == 'MessageBoxA':
                m.raw.hook_add(UC_HOOK_CODE, _text_api_cb(m, nm, (1, 2)), None, STUB_BASE + 16 * k,
                               STUB_BASE + 16 * k)
    m.uc.mem_write(FRET, bytes([0xd9, 0x05]) + struct.pack('<I', FSLOT) + b'\xc3')
    m.uc.mem_write(FRET + 0x10, bytes([0xdd, 0x05]) + struct.pack('<I', FSLOT) + b'\xc3')
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {}
    for a, (nargs, kind) in list(STUBS.items()) + [(v[0], (v[1], v[2])) for v in DSTUBS.values()]:
        if kind == 'i':
            ST['rets'][a] = [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)]
        elif kind == 'f':
            ST['rets'][a] = [rfl(ctx) for _ in range(3)]
        else:
            ST['rets'][a] = [r.uniform(-100, 100) for _ in range(3)]
    ST['code_nargs'] = {}
    ST['code_rets'] = {}
    ST['find'] = []
    ST['find_n'] = 0
    ST['handle'] = 0x1234
    ST['pos'] = [0, 0, 0]
    ST['cfd4'] = [(0, 0)]
    ST['dd3'] = [0, 0, 0]
    ST['ra'] = [0, 0]
    ST['purge'] = []
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code(ctx, k, nargs, *values):
    """Address of a routine (`ret` with a logging hook) for an indirect call."""
    a = CODE + 16 * k
    ctx.m.uc.mem_write(a, b'\xc3')
    ST['code_nargs'][k] = nargs
    if values:
        ST['code_rets'][k] = list(values)
    return a


def heap_api(*values):
    vals = list(values)
    return lambda n, args: vals[n % len(vals)] if vals else 0


STREAM = 0x00A7F000     # a FILE pointer value handed out by the fopen stub (only ever passed on)


# ---- 0x10027b19 .. 0x100282d4: debug log and resource cache -----------------------------------------------
def g_logtext(ctx):
    r = begin(ctx, 0x100fd850, 0x100fd7e0, 0x100fd760, 0x1000847d)
    rets(0x100fd850, r.choice([STREAM, STREAM, 0]))
    return [cstr(ctx, rname(ctx, 60, b'abc %d\n:XY'))]


def g_logcode(ctx):
    r = begin(ctx, 0x10027b19)
    return [r.choice([0, 1, 5, 0x10, 0x80004005, 0x88760000 + r.randrange(0, 0x300), ctx.u32()])]


def g_278c0(ctx):
    r = begin(ctx, 0x10027b19, 0x100ba7d5, 0x100bc280)
    w32(ctx, 0x1015dc28, r.choice([0, 0, 1, 7]))
    w32(ctx, 0x1014cefc, r.choice([0x10134fb0, ctx.u32()]))
    name = rname(ctx, 8, b'ABCxyz019_')
    rec = ctx.buf(0x14, struct.pack('<H', (r.randrange(-40000, 40000) if r.random() < 0.3 else
                                           r.randrange(-5, 300)) & 0xffff) + (name + bytes(8))[:8] + rb(ctx, 10))
    if r.random() < 0.2:
        w8(ctx, rec + 2, 0)
    if r.random() < 0.2:
        wb(ctx, rec + 2, rb(ctx, 8))
    res = newbuf(ctx, 0x10)
    rets(0x100ba7d5, r.choice([res, res, 0]))
    return [rec, newbuf(ctx, 0x24)]


def a_heapsize(n, args):
    return mix(n, 77) & 0xfffff


def g_dumplog(ctx):
    r = begin(ctx, 0x100fd850, 0x100fd7e0, 0x100fd760)
    rets(0x100fd850, r.choice([STREAM, STREAM, 0]))
    w32(ctx, 0x1015dc34, r.choice([0, 1, 9, 123, ctx.u32()]))
    table = zbuf(ctx, 0x3f1 * 4)
    w32(ctx, 0x1015dc30, table)
    for _ in range(r.randrange(0, 6)):
        slot = r.choice([0, 1, 0x3f0, r.randrange(0, 0x3f1)])
        head = 0
        for _ in range(r.randrange(1, 4)):
            node = newbuf(ctx, 0x18)
            w32(ctx, node + 8, head)
            head = node
        w32(ctx, table + 4 * slot, head)
    head = 0
    for _ in range(r.randrange(0, 4)):
        node = newbuf(ctx, 0x18)
        w32(ctx, node + 0xc, head)
        head = node
    w32(ctx, 0x109f13f0, head)
    return []


def g_useitem(ctx):
    r = begin(ctx, 0x10027f33, 0x10027cce, 0x10027fb8, 0x1007df6c, 0x100a8068, 0x100a836e)
    found = newbuf(ctx, 0x30)
    rets(0x10027f33, r.choice([found, 0, 0, 0]))
    w32(ctx, 0x109f13e0, r.choice([0, 5, 999, 1000, 1001, 5000, 0xffffffff, 0x80000000]))
    w32(ctx, 0x1015f1e8, r.choice([0, 1]))
    w32(ctx, 0x109b98c0, ctx.u32())
    ST['purge'] = [newbuf(ctx, 0x20) if r.random() < 0.6 else 0 for _ in range(r.randrange(0, 4))]
    w32(ctx, 0x109f13f0, r.choice([0, newbuf(ctx, 0x20)]))
    rets(0x100a8068, r.choice([0, -1, 1, 0x20, 0x100, r.getrandbits(32)]) & 0xffffffff)
    rets(0x100a836e, r.choice([0, 0, 0xffffffff, 1]))
    table = zbuf(ctx, 0x3f1 * 4 + 0x1000)
    w32(ctx, 0x1015dc30, table + 0x800)
    blocks = [newbuf(ctx, 0x40) for _ in range(2)]
    ctx.heap = [r.choice([0, blocks[0], blocks[1], blocks[1]]) for _ in range(4)]
    key = r.choice([-1, -5, 0, 1, 0xd, 0x7fff, 0x12345, r.randrange(0, 1000), r.getrandbits(31), 0x7fffffff])
    name = ctx.buf(8, bytes(r.choice([0x41, 0x7a, 0x80, 0xff, 0x30, r.getrandbits(8)]) for _ in range(8)))
    return [ctx.u32(), key & 0xffffffff, name]


def a_useitem_heap(n, args):
    return API_STATE['heap'][n % len(API_STATE['heap'])] if API_STATE['heap'] else 0


API_STATE = {'heap': []}


def g_useitem_wrap(ctx):
    args = g_useitem(ctx)
    API_STATE['heap'] = ctx.heap
    return args


# ---- 0x10029067 .. 0x10029421: debris pieces -------------------------------------------------------------
def g_29067(ctx):
    r = begin(ctx, 0x10029421, 0x10028fb0)
    now = r.getrandbits(32)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x10179cd4, r.choice([0, 0, 0, 1]))
    wb(ctx, 0x1015dc48, rb(ctx, 0x14))
    for i in range(0x80):
        e = 0x109f09e0 + 0x14 * i
        state = r.choice([0, 1, 1, 2])
        age = r.choice([0, 0x712, 0x713, 0x714, 5000, r.getrandbits(32), r.randrange(0, 0x1000)])
        w32(ctx, e, state, ctx.u32(), (now - age) & 0xffffffff, ctx.u32(), ctx.u32())
    return []


def obj_tree(ctx, depth):
    r = ctx.rng
    obj = newbuf(ctx, 0xa0)
    w32(ctx, obj + 0x94, r.getrandbits(32) | (1 if r.random() < 0.85 else 0))
    w32(ctx, obj, r.choice([0, 0, 1]))
    for k in range(12):
        w32(ctx, obj + 0xc + 4 * k, rfl(ctx, 10.0, (1.0, 0.0, -1.0)))
    if r.random() < 0.75:
        pivot = newbuf(ctx, 0x50)
        for k in (0x38, 0x3c, 0x40):
            w32(ctx, pivot + k, rfl(ctx, 100.0))
        w32(ctx, obj + 0x8c, pivot)
    else:
        w32(ctx, obj + 0x8c, 0)
    head = 0
    if depth > 0:
        for _ in range(r.choice([0, 0, 1, 2])):
            c = obj_tree(ctx, depth - 1)
            w32(ctx, c + 8, head)
            head = c
    w32(ctx, obj + 4, head)
    return obj


def g_291e3(ctx):
    r = begin(ctx, 0x100a9c25, 0x10009a53, 0x10088a2d, 0x10060008)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return [obj_tree(ctx, 2)]


def g_29421(ctx):
    r = begin(ctx, 0x1000946e, 0x1002d961, 0x10038d65, 0x10029b1f, 0x1009c929, 0x100a9d47, 0x100aa127,
              0x100291e3, 0x1002990f)
    i = r.randrange(0, 0x80) if r.random() < 0.9 else r.randrange(0x80, 0x100)
    e = 0x109eeb00 + 0x30 * i
    T = r.choice([0, 1, 2, 3, 16, 33, 50, 100, -1, r.randrange(-1000, 1000)])
    w32(ctx, 0x1015dc80, T)
    w32(ctx, 0x1015dc40, ctx.u32())
    obj = newbuf(ctx, 0xa0)
    model = newbuf(ctx, 0x60)
    w32(ctx, model + 0x3c, rfl(ctx, 10.0))
    w32(ctx, obj + 0x8c, model)
    name = r.choice([0, cstr(ctx, b'xxWRFyy'), cstr(ctx, b'WBZ'), cstr(ctx, b'WR'), cstr(ctx, rname(ctx, 8, b'WRFBZ'))])
    w32(ctx, obj + 0x90, name)
    w32(ctx, e + 4, obj if r.random() < 0.95 else 0)
    if r.random() < 0.5:
        # falling onto the ground: positive speed before, below the threshold after this frame's gravity
        T = r.randrange(1, 50)
        w32(ctx, 0x1015dc80, T)
        vy = r.choice([1e-6, 0.01, 0.3, r.uniform(1e-7, 2.0)])
        g = -r.uniform(0.0, 0.5) - vy / T
    else:
        vy = r.choice([1e-7, f32(1e-7), 0.0, -0.5524861812591553, -0.6, -0.5, 1.0, -3.0, r.uniform(-5, 5)])
        g = r.choice([0.0, -0.01, -0.1, -1.0, 0.05, r.uniform(-1, 1)])
    for k in range(0x8, 0x2c + 1, 4):
        w32(ctx, e + k, rfl(ctx, 50.0))
    wf(ctx, e + 0xc, vy)
    wf(ctx, e + 0x20, g)
    if r.random() < 0.05:
        w32(ctx, e + 0xc, rfl(ctx))
    radius = r.choice([0.0, 1.0, 2.5, r.uniform(0, 10)])
    rets(0x1000946e, fbits(radius))
    ST['pos'] = [rfl(ctx), rfl(ctx), rfl(ctx)]
    y = rf(ctx, e + 0x28)
    try:
        yp = f32(y - radius / 2.0)
        gg = T * f32(g)
        newy = ((gg * 0.5 + vy) * T) + yp
    except (OverflowError, ValueError):
        newy = 0.0
    if newy != newy or abs(newy) > 1e30:
        newy = 0.0
    rets(0x1002d961, r.choice([newy, newy + 0.001, newy - 0.001, newy + r.uniform(-5, 5), -1e30, 1e30]))
    rets(0x1009c929, r.choice([0, 49, 50, 99]))
    return [i]


# ---- 0x1002dfbb / 0x1002e91e: collision walk ------------------------------------------------------------
def seg_buf(ctx):
    return ctx.buf(0x38, b''.join(struct.pack('<I', rfl(ctx, 100.0)) for _ in range(14)))


def node_tree(ctx, depth):
    r = ctx.rng
    node = newbuf(ctx, 0x90)
    w16(ctx, node, r.choice([0, 0x800, r.getrandbits(16)]))
    if depth > 0 and r.random() < 0.7:
        lst = newbuf(ctx, 0x10)
        head = 0
        for _ in range(r.choice([0, 1, 2, 3])):
            c = newbuf(ctx, 0x90)
            w32(ctx, c + 0x8c, node_tree(ctx, depth - 1) if r.random() < 0.9 else 0)
            w32(ctx, c + 8, head)
            head = c
        w32(ctx, lst + 4, head)
        w32(ctx, node + 0x28, lst)
    else:
        w32(ctx, node + 0x28, r.choice([0, 0, zbuf(ctx, 0x10)]))
    return node


def dists(ctx, base):
    r = ctx.rng
    vals = []
    for _ in range(4):
        c = r.random()
        if c < 0.4:
            vals.append(fbits(base * r.uniform(0.2, 0.99)))
        elif c < 0.7:
            vals.append(fbits(base * r.uniform(1.0, 3.0)))
        elif c < 0.8:
            vals.append(fbits(base))
        else:
            vals.append(rfl(ctx, 100.0))
    return vals


def g_2dfbb(ctx):
    r = begin(ctx, 0x10009eb3, 0x10009d7e, 0x1002e569, 0x1009cbde)
    base = r.choice([10.0, 50.0, 1.0, 1000.0])
    rets(0x10009eb3, *dists(ctx, base))
    rets(0x10009d7e, *dists(ctx, base))
    rets(0x1009cbde, *dists(ctx, base))
    rets(0x1002e569, *[r.choice([0, 1, 1]) for _ in range(3)])
    dist = ctx.buf(4, struct.pack('<I', fbits(base) if r.random() < 0.9 else rfl(ctx)))
    outs = [newbuf(ctx, 4) for _ in range(3)]
    args = [node_tree(ctx, 2), seg_buf(ctx), newbuf(ctx, 0x38), dist] + outs
    if r.random() < 0.1:
        args[r.choice([0, 1, 3, 4, 5, 6])] = 0
    return args


def g_2e91e(ctx):
    r = begin(ctx, 0x1002e9f5, 0x1002ed0b, 0x1002f021)
    face = newbuf(ctx, 0x30)
    w16(ctx, face + 2, r.choice([0, 1, 2, 3, 4, 7, 0x8000, 0xffff]))
    vals = [r.choice([1.0, -1.0, 0.5, -0.5, 0.0, r.uniform(-1, 1)]) for _ in range(3)]
    for k in range(3):
        wf(ctx, face + 0x18 + 4 * k, vals[r.randrange(0, 3)] if r.random() < 0.3 else vals[k])
    if r.random() < 0.08:
        w32(ctx, face + 0x18 + 4 * r.randrange(0, 3), rfl(ctx))
    return [face, newbuf(ctx, 0x40), rfl(ctx), rfl(ctx), rfl(ctx)]


# ---- 0x10031ce0: box test ---------------------------------------------------------------------------------
def g_31ce0(ctx):
    r = begin(ctx, 0x1009cfd4, 0x1009cbde, 0x10031ed3, 0x1002e790)
    pool = [0.0, -0.0, 1.0, 2.0, 5.0, -1.0, 10.0, 0.5, 3.0]
    if r.random() < 0.6:
        # overlapping slabs: near values below the far ones, a long enough segment
        ST['cfd4'] = [(fbits(r.choice([-1.0, -0.0, 0.0, 0.5, 1.0, r.uniform(-2, 2)])),
                       fbits(r.choice([3.0, 4.0, 10.0, r.uniform(2, 20)]))) for _ in range(3)]
        rets(0x1009cfd4, *[r.choice([0, 0, 0, 0, 0, 0, 1]) for _ in range(3)])
        rets(0x1009cbde, fbits(r.choice([0.25, 1.0, 2.5, 5.0, 30.0, r.uniform(0, 20)])))
    else:
        ST['cfd4'] = [(fbits(r.choice(pool)), fbits(r.choice(pool))) if r.random() < 0.95 else (rfl(ctx), rfl(ctx))
                      for _ in range(3)]
        rets(0x1009cfd4, *[r.choice([0, 0, 0, 0, 1]) for _ in range(3)])
        rets(0x1009cbde, fbits(r.choice(pool)))
    rets(0x10031ed3, r.choice([0, 1, 2, 0xffffffff]))
    rets(0x1002e790, *[r.choice([0, 0, 1]) for _ in range(3)])
    box = newbuf(ctx, 0x60)
    n = r.choice([0, 0, 1, 2, 4, -1])
    w32(ctx, box + 0x18, n)
    if r.random() < 0.05:
        box = 0
    return [box, newbuf(ctx, 0x40), seg_buf(ctx)]


# ---- 0x100347d5 / 0x100348d8: debug output ---------------------------------------------------------------
def g_appendlog(ctx):
    r = begin(ctx, 0x100fd850, 0x100fe390, 0x100fe240, 0x100fd760)
    w32(ctx, 0x1015e500, r.choice([0, 0, STREAM, 0x00A7E000]))
    rets(0x100fd850, r.choice([STREAM, 0]))
    return [cstr(ctx, rname(ctx, 40))]


def g_printf(ctx):
    r = begin(ctx, 0x10034777, 0x100347d5)
    w32(ctx, 0x1015e4f8, r.choice([0, 1, 2, 3, 4, 5, 0xffffffff]))
    w32(ctx, 0x1015e4fc, r.choice([0, 0x10203, ctx.u32()]))
    w32(ctx, 0x10139e50, ctx.u32())
    fmt = r.choice([b'%d', b'x=%d y=%s', b'%x-%u', b'plain text', b'%d%s%s', b'%5d|%-4s|'])
    text = cstr(ctx, rname(ctx, 30))
    if r.random() < 0.15:
        text = cstr(ctx, b'Z' * 300)
    return [cstr(ctx, fmt), r.choice([5, -7, 0x1234]) & 0xffffffff, text, text, 3, 4]


# ---- 0x10035f90 .. 0x10036876: plugins -----------------------------------------------------------------
def find_plan(ctx):
    r = ctx.rng
    ST['find'] = [rname(ctx, 12, b'abcXY.dl_') for _ in range(r.choice([0, 1, 2, 3, 4]))]
    ST['handle'] = r.choice([0x1234, 0, 7])


def g_probeall(ctx):
    r = begin(ctx, 0x100fe470, 0x100fe5a0, 0x100fe6c0, 0x10036027)
    find_plan(ctx)
    w32(ctx, 0x1015e56c, ctx.u32())
    return [ctx.u32()]


def g_36108(ctx):
    r = begin(ctx, 0x100fe470, 0x100fe5a0, 0x100fe6c0, 0x100361d8)
    find_plan(ctx)
    w32(ctx, 0x1015e574, ctx.u32())
    return [newbuf(ctx, 0x20)]


def g_loadall(ctx):
    r = begin(ctx, 0x100fe470, 0x100fe5a0, 0x100fe6c0, 0x1003650d)
    find_plan(ctx)
    w32(ctx, 0x1015e570, ctx.u32())
    return []


# ---- 0x100355ad: banner ----------------------------------------------------------------------------
def g_355ad(ctx):
    r = begin(ctx, 0x100461e8, 0x1008d36c, 0x100ed491, 0x100ed6aa, 0x100461d0)
    w32(ctx, 0x1015e530, r.choice([0, cstr(ctx, b''), cstr(ctx, rname(ctx, 10))]))
    w32(ctx, 0x101c64f0, r.choice([0, 1, 2, 3]))
    wb(ctx, 0x109e3e00, rb(ctx, 0x14))
    rets(0x100461e8, r.choice([newbuf(ctx, 0x40), newbuf(ctx, 0x40), 0]))
    drv = newbuf(ctx, 0x30)
    w32(ctx, drv + 0x14, code(ctx, 20, 4))
    w32(ctx, drv + 0x1c, code(ctx, 21, 2))
    w32(ctx, drv + 0x20, code(ctx, 22, 0, r.choice([0, 0, 1, 0xffffffff])))
    w32(ctx, 0x1015e688, drv)
    old = newbuf(ctx, 0x30)
    w32(ctx, old + 0x18, code(ctx, 23, 0))
    w32(ctx, 0x1015e68c, old)
    w32(ctx, 0x1017b920, r.choice([0, 1, 1]))
    API_STATE['heap'] = [r.choice([newbuf(ctx, 0x40), newbuf(ctx, 0x40), 0]) for _ in range(2)]
    return []


# ---- 0x1003795b: display mode --------------------------------------------------------------------
def a_gsm(n, args):
    return API_STATE['gsm'][n % len(API_STATE['gsm'])]


def a_adjust(n, args, uc):
    rect = args[0]
    if isinstance(rect, int):
        v = struct.unpack('<4i', bytes(uc.mem_read(rect, 16)))
        uc.mem_write(rect, struct.pack('<4i', v[0] - 3, v[1] - 22, v[2] + 3, v[3] + 3))
    return 1


DISPLAY_API = {'GetSystemMetrics': a_gsm, 'AdjustWindowRect': a_adjust}


def g_display(ctx):
    r = begin(ctx, 0x100381d0)
    API_STATE['gsm'] = [r.choice([640, 800, 1024, 320, 0]) if k % 2 == 0 else r.choice([480, 600, 768, 200, 0])
                        for k in range(4)]
    tables = []
    for k in range(4):
        t = newbuf(ctx, 0x10)
        w32(ctx, t, r.choice([0, 1, 2, 3]), r.choice([0, 1, 1, 2]))
        tables.append(t)
        w32(ctx, 0x1013a298 + 4 * k, t)
    drivers = []
    for k in range(7):
        d = newbuf(ctx, 0x20)
        w32(ctx, d, r.choice([k, k, 6, r.randrange(0, 7)]), r.randrange(0, 4), r.choice([0, 1, 1, 1]))
        w32(ctx, d + 0x10, code(ctx, 2 * k, 3, *[r.choice([0, 0, 1]) for _ in range(3)]), code(ctx, 2 * k + 1, 0))
        drivers.append(d)
    for k in range(7):
        w32(ctx, 0x1013a2ac + 4 * k, drivers[k])
    w32(ctx, 0x1013a2a8, drivers[0])
    w32(ctx, 0x1013a2c8, r.choice([0, 1]))
    w32(ctx, 0x1015e68c, r.choice(drivers))
    w32(ctx, 0x109e3d14, r.choice([640, 800, 0]))
    w32(ctx, 0x109e3d18, r.choice([480, 600, 0]))
    w32(ctx, 0x1017b8f0, ctx.u32())
    mode = r.choice([-1, 0, 1, 2, 3, 6, 7, -2])
    a = r.choice([0, 1, 0, 0])
    w = r.choice([0, -1, 320, 640, 800, 1024, 2000])
    h = r.choice([0, -1, 200, 480, 600, 768, 2000])
    return [mode & 0xffffffff, a, ctx.u32(), w & 0xffffffff, h & 0xffffffff, r.choice([0, 1])]


# ---- 0x10038913 .. 0x1003ad51: unit parts --------------------------------------------------------------
def g_38913(ctx):
    r = begin(ctx, 0x10038dd3, 0x100aae0f, 0x1003913b, 0x100aa351, 0x100ab186, 0x10009408, 0x1000942e,
              0x1003b485, 0x1003b449, 0x1003b45d, 0x1003aecb, 0x1000ebfb, 0x10041203)
    unit = newbuf(ctx, 0x60)
    w32(ctx, unit + 4, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    rec = newbuf(ctx, 0xc50)
    w32(ctx, rec, unit)
    w32(ctx, rec + 0xc45, r.choice([0, 0, 0, 1]))
    data = newbuf(ctx, 0x120)
    part = newbuf(ctx, 0x90)
    ST['dd3'] = [data, ctx.u32(), r.choice([part, part, 0])]
    rets(0x100aae0f, r.choice([newbuf(ctx, 0x10), 0, 0x5555]))
    rets(0x1003b485, r.choice([0, 1, 5, 0xffffffff, 0x80000000]), r.choice([0, 3, 0xffffffff]))
    return [rec, ctx.u32(), ctx.u32()]


def unit_entries(ctx, units):
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 6])
    w32(ctx, 0x1015ea80, n)
    for i in range(n):
        e = 0x109b9e20 + 0x7e * i
        wb(ctx, e, rb(ctx, 0x7e))
        w32(ctx, e, r.choice(units))
    return n


def g_3a990(ctx):
    r = begin(ctx, 0x100b659a, 0x1003aecb)
    unit = newbuf(ctx, 0x10)
    u = r.choice([0, 1, 2])
    w32(ctx, unit + 4, u)
    unit_entries(ctx, [u, u, 5])
    w32(ctx, 0x10134f88, ctx.u32())
    rets(0x100b659a, r.choice([newbuf(ctx, 0x40), 0x00A90000, 0]))
    return [unit]


def g_3ad51(ctx):
    r = begin(ctx, 0x1003aecb, 0x1008a40a, 0x1008a37f, 0x100897c7)
    units = [0, 1, 2]
    for u in range(3):
        p = newbuf(ctx, 0x50)
        w16(ctx, p + 0x1c, r.getrandbits(16))
        w32(ctx, p + 0x48, r.choice([0, newbuf(ctx, 0x90)]))
        w32(ctx, 0x109e8b40 + 4 * u, p)
    n = unit_entries(ctx, units)
    for i in range(n):
        e = 0x109b9e20 + 0x7e * i
        w32(ctx, e + 0x4c, r.choice([0, 0, 1]))
        for k in range(4):
            if r.random() < 0.6:
                item = newbuf(ctx, 0x30)
                view = newbuf(ctx, 8)
                w32(ctx, view, r.choice([0, 1]))
                w32(ctx, item + 0x28, view)
                w32(ctx, e + 0x50 + 4 * k, item)
            else:
                w32(ctx, e + 0x50 + 4 * k, 0)
    rets(0x1003aecb, r.choice([0, 1, 1]), r.choice([0, 1, 1]))
    return [r.choice(units), r.choice([0xffffffff, 0, 1, 2, 3])]


# ---- 0x1004170e .. 0x1004548f: position packet and camera ----------------------------------------------
def g_4170e(ctx):
    r = begin(ctx, 0x10084a94, 0x1000ebfb, 0x1006788c)
    own = r.choice([0, 1, 2])
    w32(ctx, 0x1017b8c8, own)
    for u in range(4):
        if u == own or r.random() < 0.9:
            p = newbuf(ctx, 0x30)
            w32(ctx, p + 0x28, newbuf(ctx, 0x560))
            w32(ctx, 0x109e8b40 + 4 * u, p)
        else:
            w32(ctx, 0x109e8b40 + 4 * u, 0)
    rets(0x10084a94, r.choice([0.0, 1.0, 24.9, 25.0, 25.000001, 26.0, 1e6, -3.0, 2.5,
                               r.uniform(0, 100), float('nan')]))
    pkt = ctx.buf(0x20, b''.join(struct.pack('<i', r.choice([0, 1, -1, 65536, -65536, r.getrandbits(31),
                                                              -r.getrandbits(31), r.randrange(-10 ** 6, 10 ** 6)]))
                                 for _ in range(8)))
    return [pkt, r.choice([0, 1, 2, 3])]


def cam_buf(ctx):
    cam = ctx.buf(0x40, b''.join(struct.pack('<I', rfl(ctx, 200.0, (90.0, -90.0))) for _ in range(16)))
    w32(ctx, 0x1014cae0, cam)
    return cam


def g_43f1a(ctx):
    r = begin(ctx, 0x1004409f, 0x10045308, 0x10047bcd)
    cam_buf(ctx)
    w32(ctx, 0x1013aec4, r.choice([0, 1, 3, 10]))
    w32(ctx, 0x1015ed20, r.choice([-1, 0, 1, 2, 3, 9, 10]) & 0xffffffff)
    w32(ctx, 0x1015ee98, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1015ed38, r.choice([0, 10, 100, r.randrange(0, 2000)]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 33, 100]))
    for k in range(12):
        w32(ctx, 0x1015ed88 + 0x1c * k, r.choice([0, 50, 100, 1000, r.randrange(0, 3000)]))
    rets(0x10047bcd, *[rfl(ctx, 100.0) for _ in range(6)])
    return []


def camera_floats(ctx):
    r = ctx.rng
    for a in (0x1015fda0, 0x1015fda4, 0x1015fda8, 0x1015fdac, 0x1015fdb0):
        w32(ctx, a, rfl(ctx, 5.0, (1e-7, 0.0, -0.0)))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 16, 33, 100, -5]) & 0xffffffff)


def g_445d6(ctx):
    r = begin(ctx, 0x10044909, 0x100448cd, 0x100448ff, 0x10044c7a, 0x100453a3, 0x100a94fa, 0x100451a2,
              0x1004548f, 0x10044d0b, 0x100456fc)
    cam_buf(ctx)
    camera_floats(ctx)
    w32(ctx, 0x1015fdb4, ctx.u32())
    w32(ctx, 0x1015dc64, r.choice([0, 1, 1]))
    rets(0x100448ff, r.choice([0, 1, 2, 3, 4, 5, 6, 7, 0xffffffff]))
    w32(ctx, 0x1017b8c8, r.choice([0, 1]))
    w32(ctx, 0x109e8b40, r.choice([0, 0x1234]))
    w32(ctx, 0x109e8b44, r.choice([0, 0x1234]))
    w32(ctx, 0x1015ef14, r.choice([0, 0x55]))
    w32(ctx, 0x1013aed4, r.choice([0, 1, 2]))
    w32(ctx, 0x1015eefc, ctx.u32())
    w32(ctx, 0x10179cd4, r.choice([0, 1]))
    w32(ctx, 0x1015ef10, r.choice([0, 1]))
    return []


def g_451a2(ctx):
    r = begin(ctx, 0x10044255, 0x10044909, 0x10043f1a, 0x10045308)
    cam_buf(ctx)
    w32(ctx, 0x1013aed4, r.choice([0, 1, 2]))
    w32(ctx, 0x1015dc78, ctx.u32())
    for a in (0x1015fdbe, 0x1015fdbf, 0x1015fdc0, 0x1015fdc1, 0x1015ef2c):
        w8(ctx, a, r.choice([0, 0, 0, 1, 0x80]))
    rets(0x10043f1a, r.choice([0, 1]))
    return []


def g_4548f(ctx):
    r = begin(ctx, 0x10044909, 0x10047bcd, 0x1004487c)
    cam_buf(ctx)
    w32(ctx, 0x1013aed4, r.choice([0, 1, 2, 2]))
    w32(ctx, 0x10179cd4, r.choice([0, 1]))
    w32(ctx, 0x1015eeec, rfl(ctx, 10.0))
    w32(ctx, 0x1015dc78, ctx.u32())
    rets(0x10047bcd, r.choice([fbits(32.0), fbits(-32.0), fbits(31.9), fbits(-40.0), rfl(ctx, 100.0)]))
    rets(0x1004487c, rfl(ctx, 200.0))
    return [rfl(ctx, 5.0) for _ in range(5)]


# ---- 0x100461e8 .. 0x10046fe5: data files --------------------------------------------------------------
def g_loadwhole(ctx):
    r = begin(ctx, 0x100fd850, 0x100feb20, 0x100fe970, 0x100fe830, 0x100fd760)
    rets(0x100fd850, r.choice([STREAM, STREAM, 0]))
    rets(0x100fe970, r.choice([0, 1, 0x20, 0x100, 0xffffffff]))
    buf = newbuf(ctx, 0x120)
    API_STATE['heap'] = [r.choice([newbuf(ctx, 0x120), 0])]
    return [cstr(ctx, rname(ctx, 12)), r.choice([0, 0, buf])]


def g_463bf(ctx):
    r = begin(ctx, 0x100b8d6c, 0x100b659a)
    n = r.choice([0, 1, 2, 5, 32, r.randrange(0, 33)])
    m = r.choice([0, 1, 3, 4])
    words = [n, m]
    for _ in range(n):
        words += [r.choice([0, 1, 5, 0x1f, 0x20, -1, r.randrange(0, 0x20)]), r.choice([0, 1, 2, 3, 4, -1])]
        words += [r.choice([0, 1, -1, 65536, r.getrandbits(31), -r.getrandbits(31)]) for _ in range(m)]
    words += [ctx.u32() for _ in range(4)]
    data = ctx.buf(4 * len(words), b''.join(struct.pack('<I', w & 0xffffffff) for w in words))
    rets(0x100b8d6c, r.choice([data, data, data, 0]))
    w32(ctx, 0x1015d184, r.choice([0, 1, 0x300, 0x310, r.randrange(0, 0x320)]))
    w32(ctx, 0x1014cee4, ctx.u32())
    w32(ctx, 0x1014cf50, ctx.u32())
    blocks = [newbuf(ctx, 0x14) for _ in range(4)]
    rets(0x100b659a, *[r.choice(blocks + blocks + [0]) for _ in range(5)])
    name = ctx.buf(8, struct.pack('<h', r.choice([-1, 0, 5, -2])) + rb(ctx, 6))
    return [name]


def g_469f2(ctx):
    r = begin(ctx, 0x10109980, 0x100282d4, 0x101049c0, 0x10105750, 0x1002828e)
    rets(0x10109980, r.choice([3, 3, 0xffffffff]))
    rets(0x100282d4, r.choice([newbuf(ctx, 0x20), newbuf(ctx, 0x20), 0]))
    w32(ctx, 0x109e3e20, newbuf(ctx, 0x40))
    w32(ctx, 0x109e3de4, r.choice([0, 4, 0x40]))
    w32(ctx, 0x1014ced8, ctx.u32())
    w32(ctx, 0x1014cf8c, ctx.u32())
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 0x300)]


def g_readall(ctx):
    r = begin(ctx, 0x10109980, 0x10100730, 0x100b659a, 0x10105b50, 0x101049c0)
    rets(0x10109980, r.choice([5, 5, 5, 0xffffffff]))
    size = r.choice([0, 1, 0x10, 0x100])
    rets(0x10100730, size)
    rets(0x100b659a, r.choice([newbuf(ctx, 0x110), 0]))
    rets(0x10105b50, r.choice([size, size, size - 1, 0xffffffff]) & 0xffffffff)
    API_STATE['heap'] = [r.choice([newbuf(ctx, 0x110), newbuf(ctx, 0x110), 0])]
    alloc = r.choice([0, 0, ctx.buf(4, struct.pack('<I', ctx.u32()))])
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 4), newbuf(ctx, 4), alloc]


def g_46ba1(ctx):
    r = begin(ctx, 0x10046ac4, 0x101049c0)
    rets(0x10046ac4, r.choice([5, 0xffffffff]))
    ST['ra'] = [ctx.u32(), ctx.u32()]
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 4)]


def g_stats(ctx):
    r = begin(ctx, 0x100fd850, 0x100fd7e0, 0x100fd760)
    rets(0x100fd850, r.choice([STREAM, STREAM, 0]))
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 0x140)]


def g_46f7f(ctx):
    r = begin(ctx, 0x10109980, 0x10105750, 0x101049c0)
    rets(0x10109980, r.choice([3, 3, 0xffffffff]))
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 0x1e0)]


def g_46fe5(ctx):
    r = begin(ctx, 0x10046ac4, 0x101049c0)
    rets(0x10046ac4, r.choice([5, 5, 0xffffffff]))
    ST['ra'] = [r.choice([0, 1, 0x17, 0x10]), newbuf(ctx, 0x20)]
    API_STATE['heap'] = [zbuf(ctx, 0x20) if r.random() < 0.9 else 0]
    w32(ctx, 0x1015ec58, r.choice([0, 1]))
    return [cstr(ctx, rname(ctx, 12)), newbuf(ctx, 4)]


def a_heap(n, args):
    h = API_STATE['heap']
    return h[n % len(h)] if h else 0


HEAP_API = {'HeapAlloc': a_heap}


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Debug_LogText', 0x10027b19, g_logtext, 300, void=True),
    spec('Debug_LogCodeText', 0x10027b69, g_logcode, 300, void=True),
    spec('Sim_100278c0', 0x100278c0, g_278c0, 500),
    spec('Cache_DumpLog', 0x1002812f, g_dumplog, 300, void=True, api={'HeapSize': a_heapsize}),
    spec('Cache_UseItem', 0x100282d4, g_useitem_wrap, 1000, api={'HeapAlloc': a_useitem_heap}),
    spec('Sim_10029067', 0x10029067, g_29067, 300, void=True),
    spec('Sim_100291e3', 0x100291e3, g_291e3, 800, void=True),
    spec('Sim_10029421', 0x10029421, g_29421, 1500, void=True),
    spec('Sim_1002dfbb', 0x1002dfbb, g_2dfbb, 1500),
    spec('Sim_1002e91e', 0x1002e91e, g_2e91e, 800),
    spec('Sim_10031ce0', 0x10031ce0, g_31ce0, 1500),
    spec('Debug_AppendLog', 0x100347d5, g_appendlog, 300, void=True),
    spec('Debug_Printf', 0x100348d8, g_printf, 500, void=True),
    spec('Plugin_ProbeAll', 0x10035f90, g_probeall, 400),
    spec('Sim_10036108', 0x10036108, g_36108, 400),
    spec('Plugin_LoadAll', 0x10036466, g_loadall, 400),
    spec('Sim_100355ad', 0x100355ad, g_355ad, 500, void=True, api=HEAP_API),
    spec('Display_SetMode', 0x1003795b, g_display, 1500, api=DISPLAY_API),
    spec('Sim_10038913', 0x10038913, g_38913, 800),
    spec('Sim_1003a990', 0x1003a990, g_3a990, 600),
    spec('Gear_SetLod', 0x1003ad51, g_3ad51, 800),
    spec('Sim_1004170e', 0x1004170e, g_4170e, 1000, void=True),
    spec('Sim_10043f1a', 0x10043f1a, g_43f1a, 1000),
    spec('Sim_100445d6', 0x100445d6, g_445d6, 2000, void=True),
    spec('Sim_100451a2', 0x100451a2, g_451a2, 600, void=True),
    spec('Sim_1004548f', 0x1004548f, g_4548f, 1500, void=True),
    spec('File_LoadWhole', 0x100461e8, g_loadwhole, 400, api=HEAP_API),
    spec('Sim_100463bf', 0x100463bf, g_463bf, 800),
    spec('Sim_100469f2', 0x100469f2, g_469f2, 400),
    spec('File_ReadAll', 0x10046ac4, g_readall, 600, api=HEAP_API),
    spec('Sim_10046ba1', 0x10046ba1, g_46ba1, 300),
    spec('Stats_WriteKills', 0x10046bf3, g_stats, 300),
    spec('Sim_10046f7f', 0x10046f7f, g_46f7f, 300),
    spec('Sim_10046fe5', 0x10046fe5, g_46fe5, 400, api=HEAP_API),
]

# round 32: the debris update calls 1.2's smoke in 1.2 mode (off here): a stand-in, and the mode switch
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', 'sim/render/w33_dx12.c', '../tools/verify/w32_smoke_stub.c'] if x not in SOURCES]
