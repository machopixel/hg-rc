# Differential tests for round 9, letter b (see difftest.py and docs/checker.md).
#
# Callees are replaced by stubs: a code hook at the callee's first byte (no code bytes patched) that appends
# ('stub_<address>', arguments, extra data) to the checker's call log, which must be identical for the original and
# the C run (stack addresses are logged as 'stack'; some word arguments only by their low 16 bits, because the
# original pushes them with garbage in the top half), writes out data where the callee has some, answers with a
# value that depends only on the trial and the call position, and returns. Converted callees are stubbed at their
# original address (reached by the original code through its thunk) and at their C symbol; the stubs of this
# letter's own functions (`mine`) are only active in the specs of their callers, so every function is checked on
# its own logic. The two DirectPlay function pointers (send 0x1015eab4, receive 0x1015eab8) point at `ret` bytes
# with logging hooks; the receive one delivers the trial's messages into the receive buffer.
# Run for real on both sides: Str_CompareNoCase, Sim_1003db56, Sim_1003dbc0, Sim_10015c61, Str_Copy, Mem_Move,
# Crt_StrStr, Crt_SPrintf (with the runtime's _output 0x10100fd0), Sim_1007e984 / Net_DpSend_1007e920 (converted,
# they only call the pointer slots), Sim_100610ae and Quat_Multiply (with the sine table 0x10084766 and fabs).
# Every float result (Net_SendStateMessage, Quat_FromAxisAngles) is checked bit for bit; the x87 runs with the
# game's control word 0x27f.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    # this letter
    'sim/unsorted_1001/w9b_records.c', 'sim/unsorted_1003/w9b_net.c', 'sim/unsorted_1004/w9b_net.c',
    'sim/unsorted_1005/w9b_misc.c', 'sim/unsorted_1006/w9b_quat.c', 'sim/unsorted_1006/w9b_tally.c',
    'sim/unsorted_1008/w9b_cells.c', 'sim/unsorted_100c/w9b_hud.c',
    # converted callees and what they need to link
    'sim/unsorted_1001/w5c_calls.c', 'sim/unsorted_1004/w3p_net.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1003/w3p_net.c', 'sim/core/w2a_timers.c', 'sim/core/w2b_timers.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_1003/records.c', 'sim/unsorted_1004/w2j_misc.c', 'sim/unsorted_1004/w8u_camera.c',
    'sim/unsorted_100f/w8z_crt.c', 'sim/unsorted_1007/w2e_state.c', 'sim/unsorted_1004/w6e_misc.c',
    'sim/unsorted_1004/w5y_misc.c', 'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_1007/w3s_calls.c',
    'sim/unsorted_1004/w5a_misc.c', 'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1004/w7m_misc.c',
    'sim/unsorted_100f/w6i_misc.c', 'sim/unsorted_1004/w2b_checks.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_1004/w2i_misc.c', 'sim/unsorted_1006/w8w_event.c',
    'sim/unsorted_1004/w2h_record.c', 'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1007/w3p_net.c',
    'sim/unsorted_1001/w8t_actions.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_1006/w5a_math.c',
    'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1001/w7l_misc.c', 'sim/unsorted_1002/w5a_misc.c',
    'sim/unsorted_1001/w3s_group.c', 'sim/unsorted_1005/w5b_calls.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1003/w8u_misc.c', 'sim/unsorted_1001/w2l_misc.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1002/w8t_misc.c', 'sim/util/lookup.c', 'sim/unsorted_1007/w2f_misc.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/view/w2l_lists.c', 'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/unsorted_100b/w6g_misc.c',
    'sim/unsorted_1004/w4u_heap.c', 'sim/unsorted_1001/w3t_loader.c', 'sim/core/timer.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1004/w3s_records.c',
    'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1004/state.c', 'sim/unsorted_1004/w2e_strings.c',
    'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_100a/w7o_nodes.c', 'sim/unsorted_100f/w6i_mem.c',
    'sim/unsorted_1010/w7s_stdio.c', 'sim/unsorted_100f/w7q_stream.c', 'sim/unsorted_1010/w6j_stream.c',
    'sim/unsorted_1010/w6j_double.c', 'sim/unsorted_1000/w5x_calls.c', 'sim/view/w3r_matrix.c',
    'sim/unsorted_1002/w2i_state.c', 'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_1010/w2i_crt.c', 'sim/view/objlist.c', 'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_100c/w2i_misc.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100d/w2i_select.c', 'sim/unsorted_1003/w7m_state.c', 'sim/util/ring.c',
    'sim/unsorted_1006/w2f_records.c', 'sim/unsorted_1001/w2h_objects.c', 'sim/unsorted_1003/w3r_misc.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_1001/w2e_tables.c', 'sim/unsorted_1001/w6d_slots.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_1001/w6d_calls.c', 'sim/unsorted_1001/w5y_misc.c',
    'sim/unsorted_1001/w5a_misc.c', 'sim/unsorted_100b/w2e_object.c', 'sim/unsorted_1001/w2k_misc.c',
    'sim/unsorted_1001/w2f_misc.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1001/w2g_fields.c', 'sim/unsorted_1000/w3t_records.c', 'sim/unsorted_1001/w5x_calls.c',
    'sim/unsorted_100f/w2k_string.c', 'sim/unsorted_100d/state.c', 'sim/win/w4u_cdaudio.c', 'sim/util/field.c',
    'sim/unsorted_1000/w2h_access.c', 'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1005/w2h_tables.c',
    'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_100f/w2g_crt.c',
    'sim/unsorted_1009/w2l_pool.c', 'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w2i_misc.c',
    'sim/unsorted_1003/w4u_heap.c', 'sim/unsorted_1005/w3s_groups.c', 'sim/win/w4a_heap.c', 'sim/unsorted_1001/w2e_names.c',
    'sim/unsorted_100a/w5c_calls.c', 'sim/win/w4v_heap.c', 'sim/unsorted_1000/w2i_misc.c', 'sim/unsorted_1003/w2h_misc.c',
    'sim/unsorted_1003/slots.c', 'sim/unsorted_1003/w2j_misc.c', 'sim/unsorted_1002/w5x_calls.c',
    'sim/unsorted_100a/w2f_links.c', 'sim/unsorted_1002/w6d_calls.c', 'sim/unsorted_1002/w5z_calls.c',
    'sim/unsorted_1002/w7l_misc.c', 'sim/unsorted_100d/w6h_calls.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_100d/w7q_names.c', 'sim/unsorted_1001/w8t_queue.c', 'sim/unsorted_100d/w4v_misc.c',
    'sim/unsorted_1005/w2e_lists.c', 'sim/util/record.c', 'sim/unsorted_1005/w4v_misc.c', 'sim/unsorted_1002/w2e_tables.c',
    'sim/unsorted_100d/w2g_misc.c', 'sim/view/viewrect.c', 'sim/view/matrix.c', 'sim/unsorted_100b/w5z_calls.c',
    'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/unsorted_100b/w3s_array.c',
    'sim/unsorted_100b/w5c_calls.c', 'sim/unsorted_1002/w3s_cell.c', 'sim/unsorted_1006/w6e_misc.c',
    'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_100f/w2h_mem.c', 'sim/unsorted_1010/w6k_stream.c', 'sim/unsorted_1010/w7s_locale.c',
    'sim/unsorted_100f/w6i_stream.c', 'sim/unsorted_1010/w2k_crt.c', 'sim/unsorted_1003/w2l_records.c',
    'sim/unsorted_100a/w2i_misc.c', 'sim/render/w2n_polygon.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1003/w6e_misc.c', 'sim/unsorted_1003/w2f_misc.c',
    'sim/unsorted_1007/w2g_stubs.c', 'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_100a/w3t_misc.c',
    'sim/unsorted_1003/w4v_misc.c', 'sim/unsorted_1009/w2h_misc.c', 'sim/unsorted_1001/w2j_misc.c',
    'sim/unsorted_1001/w5b_calls.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1003/w2e_misc.c',
    'sim/unsorted_1005/w2g_pool.c', 'sim/unsorted_100b/w2j_misc.c', 'sim/unsorted_1004/w4w_net.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1002/w4v_misc.c', 'sim/unsorted_100c/w2h_misc.c',
    'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1003/w2b_misc.c', 'sim/unsorted_1002/w2f_misc.c',
    'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_1006/w5y_misc.c', 'sim/unsorted_100d/nodes.c',
    'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_100d/w2d_state.c', 'sim/unsorted_1005/w3q_misc.c',
    'sim/unsorted_1008/w4v_fade.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_100f/w2j_str.c',
    'sim/unsorted_1010/w6k_text.c', 'sim/unsorted_1000/w2h_reset.c', 'sim/render/w3q_texture.c',
    'sim/unsorted_1000/w8t_sound.c', 'sim/unsorted_1003/w2e_entries.c', 'sim/unsorted_1000/w2e_mono.c',
    'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_1000/w2l_objects.c',
    'sim/unsorted_1008/w2g_rect.c', 'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100b/w2e_strings.c',
    'sim/win/w4v_dll.c', 'sim/unsorted_1004/w3o_misc.c', 'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1006/w5b_calls.c',
    'sim/unsorted_1004/w2k_flags.c', 'sim/unsorted_1009/w3o_ray.c', 'sim/unsorted_1008/w3o_grid.c',
    'sim/unsorted_1010/w6k_misc.c', 'sim/render/w2n_project.c', 'sim/unsorted_100d/w5b_calls.c',
    'sim/unsorted_100d/w5z_calls.c', 'sim/unsorted_1007/node_flag.c', 'sim/unsorted_1008/w5y_misc.c',
    'sim/unsorted_1008/w3q_place.c', 'sim/unsorted_100a/w2g_misc.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/unsorted_100d/w2i_table.c', 'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1000/w2j_misc.c',
    'sim/unsorted_1000/w5z_calls.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1010/w2f_mantissa.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1010/w2f_ld12.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
CB = 0x00F00D00            # callback `ret` bytes, 0x10 apart
CB_SEND, CB_RECV = CB, CB + 0x10
FONT = 0x00A84000          # what the font lookup stub answers (only passed on, never read)
PTRTAB = 0x109e8b40

ST = {'seed': 0}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def mix(*vals):
    x = 0x9e3779b1
    for v in vals:
        x = ((x ^ (v & 0xffffffff)) * 0x85ebca6b + 0x1234567) & 0xffffffff
        x ^= x >> 15
    return x


def pick(h, options):
    return options[h % len(options)]


def logged(v):
    return 'stack' if STACK_LO <= v <= STACK_HI else v


def f2b(f):
    try:
        return struct.unpack('<I', struct.pack('<f', f))[0]
    except OverflowError:
        return 0xff800000 if f < 0 else 0x7f800000


def fbits(ctx, scale=1000.0, special=True):
    """A float bit pattern: ordinary values, zeros, ones, tiny / huge values and now and then a quiet NaN or inf."""
    r = ctx.rng
    k = r.random()
    if special and k < 0.03:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    if k < 0.12:
        return r.choice([0, 0x80000000, 1, 0x80000001, 0x00800000, 0x7f7fffff, 0x3f800000, 0xbf800000, 0x4f000000,
                         0xcf000000, 0x4effffff, 0x5f000000])
    if k < 0.75:
        return f2b(r.uniform(-scale, scale))
    if k < 0.9:
        return f2b(r.choice([-1, 1]) * r.choice([0.5, 0.49999997, 1.5, 2.5, 32767.5, 65535.99, 0.0000152587890625]))
    return r.getrandbits(32) & 0xbfffffff


# ---- stubs --------------------------------------------------------------------------------------------------
class Stub:
    def __init__(self, nargs, pol=None, csym=None, mine=False, words=()):
        self.nargs, self.csym, self.mine, self.words = nargs, csym, mine, words
        self.pol = pol if pol is not None else pv(0, [0])


def pv(tag, options):
    """Answer from options (or the list ST['v<tag>']) by (trial, call number)."""
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), [])


def cstring(uc, addr, maxn=0x50):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return list(out)


def p_str(tag, options, which=(0,)):
    """Logs the texts the arguments `which` point to; answers like pv."""
    def pol(uc, n, a):
        extra = []
        for i in which:
            extra.append(tuple(cstring(uc, a[i])) if a[i] else ())
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), extra
    return pol


def p_bytes(tag, argi, size, ret=(0,)):
    """Writes `size` bytes from the hash through argument argi (an out record)."""
    def pol(uc, n, a):
        data = bytes(mix(ST['seed'], tag, n, k) & 0xff for k in range(size))
        uc.mem_write(a[argi], data)
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or list(ret)), []
    return pol


def p_log(tag, argi, size, ret=(0,)):
    """Logs `size` bytes read through argument argi."""
    def pol(uc, n, a):
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or list(ret)), list(bytes(uc.mem_read(a[argi], size)))
    return pol


def p_clock(uc, n, a):
    return (ST.get('t0', 0) + n * ST.get('tstep', 7)) & 0xffffffff, []


def p_font(uc, n, a):
    """0x100ba7d5(name, kind, 0): upper-cases both texts in place like the original; answers 0 or the font."""
    extra = []
    for i in (0, 1):
        s = bytes(cstring(uc, a[i])) if a[i] else b''
        if a[i]:
            uc.mem_write(a[i], s.upper())
        extra.append(tuple(s))
    return pick(mix(ST['seed'], 0xa7d5, n), ST.get('font') or [0, FONT, FONT]), extra


STUBS = {
    # ---- animation records
    0x1001bb1f: Stub(3, p_str(0xbb1f, [0], (0,)), 'Sim_1001bb1f'),
    # ---- the message pump and its handlers
    0x100420a0: Stub(0, pv(0x20a0, [0, 1, 1]), 'Sim_100420a0'),
    0x1002ac1b: Stub(0, p_clock, 'Sim_1002ac1b'),
    0x1006abf6: Stub(3, None, 'Sim_1006abf6'),
    0x1006a12d: Stub(2, None, 'Score_RecordKill', mine=True),
    0x100421b0: Stub(1, None, 'Sim_100421b0', words=(0,)),
    0x1007df6c: Stub(4, p_str(0xdf6c, [0, 1, 1], (0, 1)), 'Sim_1007df6c'),
    0x1003f3f8: Stub(3),
    0x100aa39f: Stub(1, None, 'Sim_100aa39f'),
    0x100aa454: Stub(1, None, 'Sim_100aa454'),
    0x100aa799: Stub(1, None, 'Sim_100aa799'),
    0x100aa351: Stub(1, None, 'Sim_100aa351'),
    0x100aa406: Stub(1, None, 'Sim_100aa406'),
    0x1004044c: Stub(1, None, 'Sim_1004044c'),
    0x10040191: Stub(1, None, 'Sim_10040191'),
    0x1004158c: Stub(2, None, 'Sim_1004158c'),
    0x1004170e: Stub(2, None, 'Sim_1004170e'),
    0x100418e3: Stub(2, None, 'Sim_100418e3'),
    0x10041b2d: Stub(2, None, 'Sim_10041b2d'),
    0x10041c4c: Stub(2, None, 'Sim_10041c4c'),
    0x10041a16: Stub(0, None, 'Sim_10041a16'),
    0x1003ff72: Stub(2, None, 'Sim_1003ff72'),
    0x100405e6: Stub(1, None, 'Sim_100405e6'),
    0x1004084a: Stub(1, None, 'Sim_1004084a'),
    0x1004094e: Stub(2, None, 'Sim_1004094e'),
    0x10040a37: Stub(1, None, 'Sim_10040a37'),
    0x10040af8: Stub(1),
    0x10040f2d: Stub(1, None, 'Sim_10040f2d'),
    0x100410ff: Stub(1, None, 'Sim_100410ff'),
    0x10041247: Stub(1, None, 'Sim_10041247'),
    0x1004131b: Stub(1, None, 'Sim_1004131b'),
    0x10040376: Stub(1, None, 'Sim_10040376'),
    0x1003eebc: Stub(2, None, 'Net_SendStateMessage', mine=True),
    0x10041a47: Stub(0, pv(0x1a47, [0, 1]), 'Sim_10041a47'),
    0x1004227f: Stub(0, None, 'Net_UpdatePlayers', mine=True),
    0x10040eb3: Stub(0, pv(0x0eb3, [0, 1]), 'Sim_10040eb3'),
    0x10042908: Stub(1, p_log(0x2908, 0, 2, ret=(0, 1, 1)), 'Sim_10042908'),
    0x100419e9: Stub(0, pv(0x19e9, [0, 1]), 'Sim_100419e9'),
    0x100408bd: Stub(1, None, 'Sim_100408bd'),
    0x1004185b: Stub(0, pv(0x185b, [0, 1]), 'Sim_1004185b'),
    0x10041888: Stub(0, pv(0x1888, [0, 1]), 'Sim_10041888'),
    0x10041997: Stub(0, pv(0x1997, [0, 1]), 'Sim_10041997'),
    0x1002abb0: Stub(0, pv(0xabb0, [0, 5, 100, 0xffffffff]), 'Sim_1002abb0'),
    0x1002abc3: Stub(0, None, 'Sim_1002abc3'),
    # ---- the player sync
    0x10040ab8: Stub(1, pv(0x0ab8, [0, 1]), 'Sim_10040ab8'),
    0x1009d200: Stub(4, pv(0xd200, [0, 1])),
    0x10011093: Stub(1, None, 'Sim_10011093'),
    0x10040564: Stub(1, None, 'Sim_10040564'),
    0x10040645: Stub(2, p_bytes(0x0645, 0, 0x1e), 'Sim_10040645'),
    0x1003ffe9: Stub(1, None, 'Sim_1003ffe9'),
    # ---- 0x1005
    0x1004ee50: Stub(1, pv(0xee50, [0, 0x3000, 0x2004, 0x10, 0x12345678, 0xffffffff]), 'Sim_1004ee50'),
    # ---- the kill tally
    0x1000ecb0: Stub(1, p_str(0xecb0, [0], (0,)), 'Sim_1000ecb0'),
    0x100413c7: Stub(0, None, 'Sim_100413c7'),
    0x1007b02d: Stub(0, None, 'Sim_1007b02d'),
    0x10052be7: Stub(1, pv(0x2be7, [0, 1, 1, 2, 3, 0xffffffff]), 'Sim_10052be7'),
    0x100203cf: Stub(2, None, 'Sim_100203cf'),
    0x1001150d: Stub(1, None, 'Sim_1001150d'),
    0x10052aaf: Stub(1, pv(0x2aaf, [0]), 'Sim_10052aaf'),
    0x10012843: Stub(1, pv(0x2843, [0]), 'Sim_10012843'),
    0x100128dd: Stub(3, None, 'Sim_100128dd'),
    0x1005f9aa: Stub(1, p_str(0xf9aa, [0, 1], (0,)), 'Sim_1005f9aa'),
    0x10020256: Stub(2, None, 'Sim_10020256'),
    0x1003ad51: Stub(2, pv(0xad51, [0, 1]), 'Gear_SetLod'),
    0x100ac58e: Stub(1, None, 'Sim_100ac58e'),
    0x100654a1: Stub(1, None, 'FlashText_Start', mine=True),
    # ---- the scene sweep
    0x1008a46e: Stub(1, None, 'Sim_1008a46e'),
    0x1008a220: Stub(1, None, 'Sim_1008a220'),
    0x1008a40a: Stub(1, None, 'Sim_1008a40a'),
    0x1008a37f: Stub(1, None, 'Sim_1008a37f'),
    # ---- the HUD panes
    0x100ba7d5: Stub(3, p_font),
    0x100f2f01: Stub(6, p_str(0x2f01, [0], (4,))),
    0x100f1e10: Stub(6, p_str(0x1e10, [0], (4,))),
    0x100ecea6: Stub(6, p_str(0xcea6, [0], (4,))),
    0x100bb2b0: Stub(2, p_str(0xb2b0, [0, 1], (1,)), 'Sim_100bb2b0'),
}


def cb_recv(uc, n, a):
    """The DirectPlay receive (session, &from, &to, a, buffer, &size): delivers message n of the trial into
    the buffer (its type word into `from`, its extra word into `size`) and answers 0; after the last one 1."""
    msgs = ST.get('msgs') or []
    if n < len(msgs):
        typ, extra, data = msgs[n]
        uc.mem_write(a[4], data)
        uc.mem_write(a[1], struct.pack('<I', typ & 0xffffffff))
        uc.mem_write(a[5], struct.pack('<I', extra & 0xffffffff))
        return 0
    return 1


CALLBACKS = {CB_SEND: (6, None), CB_RECV: (6, cb_recv)}


def install(m):
    if getattr(m, '_w9b', False):
        return
    m._w9b = True

    def make(addr, st):
        name = 'stub_%x' % addr

        def cb(uc, address, size, ud):
            if st.mine and addr not in ST.get('active', ()):
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % st.nargs, bytes(uc.mem_read(esp + 4, 4 * st.nargs)))) if st.nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = st.pol(uc, n, raw)
            args = tuple((v & 0xffff) if i in st.words else logged(v) for i, v in enumerate(raw))
            m.api_log.append((name, args, tuple(extra)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for addr, st in STUBS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, addr, addr)
        if st.csym:
            c = m.syms[st.csym]
            m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, c, c)

    def make_cb(addr, nargs, eff):
        name = 'cb_%x' % addr

        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            m.api_log.append((name, tuple(logged(v) for v in raw), ()))
            val = eff(uc, n, raw) if eff else mix(ST['seed'], addr, n) & 0x7fffffff
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
        return cb

    for addr, (nargs, eff) in CALLBACKS.items():
        m.raw.mem_write(addr, b'\xc3')
        m.raw.hook_add(UC_HOOK_CODE, make_cb(addr, nargs, eff), None, addr, addr)


def begin(ctx, *active):
    """Installs the hooks (once per machine), activates the stubs of this letter's own functions given, the x87
    state of the game and the two DirectPlay slots. Must come first in a generator."""
    m = ctx.m
    install(m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    ST['active'] = set(active)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    w32(ctx, 0x1015eab4, CB_SEND)
    w32(ctx, 0x1015eab8, CB_RECV)
    w32(ctx, 0x1015ecd4, ctx.rng.getrandbits(32))
    w16(ctx, 0x1013acac, ctx.rng.getrandbits(16))
    return ctx.rng


def text(ctx, alphabet, n):
    return bytes(ctx.rng.choice(alphabet) for _ in range(n))


# ---- the player table -----------------------------------------------------------------------------------------
def entry(ctx, index, count):
    """A player entry (0x200 bytes) with its block (+0x28, 0xc40 bytes), name (+0x124), respawn byte (+0x168) and
    node (+0x48); registered in the pointer table."""
    r = ctx.rng
    e = newbuf(ctx, 0x200)
    blk = newbuf(ctx, 0xc40)
    w32(ctx, e + 0x28, blk)
    w32(ctx, blk, e)
    w32(ctx, e + 4, index)
    w32(ctx, e + 0x14, r.choice([3, 3, 3, 0, 1, 2]))
    w32(ctx, e + 0xc, r.choice([ctx.m._self, ctx.m._self, r.randrange(count), 0xffffffff]))
    w32(ctx, e + 0x164, r.choice([0, 1]))
    w16(ctx, e + 0x1c, r.choice([0, 2, 4, 6, 0x4000, 0x4002, 0x1400, 0x1000, 0x400, r.getrandbits(16)]))
    w32(ctx, e + 0x150, r.choice([index, r.randrange(8), 0xffffffff, 9, 0x8000, r.getrandbits(16)]))
    w32(ctx, e + 0x168, ctx.buf(4, bytes([r.choice([0, 1, 2, 3]), 0, 0, 0])))
    w32(ctx, e + 0x18, r.choice([0, 0, 1]))
    w32(ctx, e + 0x160, r.choice([0, 0, 1]))
    # kind (bits 8..15) and target player (bits 0..7); a kind-0x200 target is always a real player, as in the
    # game (a random one made both sides read through an empty or out-of-table slot)
    w32(ctx, e + 0xf0, r.choice([0x200 | r.randrange(count), 0x200 | r.randrange(count), 0x100 | r.randrange(count),
                                 0x400 | r.randrange(count), (r.getrandbits(8) << 8) | r.randrange(count)]))
    w32(ctx, e + 0x48, r.choice([0, newbuf(ctx, 0x40)]))
    w32(ctx, e + 0x15c, r.choice([0, 0, 1]))
    w32(ctx, e + 8, r.choice([0, 1, 2, r.randrange(40)]))
    w32(ctx, e + 0x10, r.choice([0, 1, 2, 3]))
    w32(ctx, e + 0x154, r.choice([0xffffffff, 3, r.getrandbits(32)]))
    wb(ctx, e + 0x124, text(ctx, b'abcXYZ09', r.randrange(1, 12)) + b'\0')
    w32(ctx, blk + 0x4f0, r.choice([2, 2, 3, 0, 4, 5, 6, 7, r.getrandbits(32)]))
    w32(ctx, blk + 0x50c, r.choice([0, 1, 2, 6, 3]))
    w32(ctx, blk + 0x613, r.choice([0, 1, 2]))
    w32(ctx, blk + 0xbdb, r.randrange(7))
    w32(ctx, PTRTAB + 4 * index, e)
    return e


def table(ctx, count=None):
    """The player table: [0x1015dc64] entries 0..count-1, self among them, the root record [0x1015d004] with
    the list of ids (+0x126, count rows of 0x1e with the id word at +0x1a) and the keys (+0x16)."""
    r = ctx.rng
    if count is None:
        count = r.choice([1, 2, 2, 3, 4, 8])
    self_ = r.randrange(count)
    ctx.m._self = self_
    w32(ctx, 0x1017b8c8, self_)
    w32(ctx, 0x1015dc64, count)
    entries = [entry(ctx, i, count) for i in range(count)]
    for i in range(count, 0x19):
        w32(ctx, PTRTAB + 4 * i, 0)
    root = zbuf(ctx, 0x200)
    w32(ctx, 0x1015d004, root)
    w32(ctx, root + 0x12, count)
    ids = list(range(count))
    r.shuffle(ids)
    lst = newbuf(ctx, 0x1e * count + 0x20)
    for j in range(count):
        w16(ctx, lst + j * 0x1e + 0x1a, ids[j])
    w32(ctx, root + 0x126, lst)
    keys = zbuf(ctx, 0x20)
    for j in range(8):
        w16(ctx, keys + 2 * j, 0x100 + j if j < count else 0xfa01)
    w32(ctx, root + 0x16, keys)
    w32(ctx, 0x1015ec84, r.choice([1, 1, 1, 0]))
    wb(ctx, 0x1015d1b0, rb(ctx, 0x20))
    ctx.m._ids = ids
    ctx.m._keys = [0x100 + j for j in range(count)]
    return count, self_, entries


def net_globals(ctx):
    r = ctx.rng
    t = r.choice([0, 100, 5000, 0x7ffffff0, r.getrandbits(31)])
    ST['t0'] = t
    ST['tstep'] = r.choice([0, 1, 7, 100])
    w32(ctx, 0x1015dc7c, t)
    w32(ctx, 0x1013ac9c, r.choice([0x38e, 100, 1]))
    w32(ctx, 0x1015ecec, t - r.choice([-5, -1, 0, 1, 100, 0x1000]))
    w32(ctx, 0x1015ecd0, r.choice([1, 1, 2, 0]))
    w32(ctx, 0x1017b8c4, ctx.buf(0x10, bytes([0, r.choice([0, 1]), 0, 0, 0, 0, 0, 0, r.choice([0, 1, 1]), 0,
                                               r.choice([0, 1]), 0, 0, 0, 0, 0])))
    w32(ctx, 0x101c4a04, cstr(ctx, r.choice([b'died', b'was killed', b'x'])))
    w32(ctx, 0x101c49f0, cstr(ctx, r.choice([b'killed by', b'by'])))
    w32(ctx, 0x101c4a00, cstr(ctx, r.choice([b'left', b'joined'])))
    w32(ctx, 0x109b98c0, cstr(ctx, r.choice([b'label', b'', b'A'])))
    w32(ctx, 0x1015ec78, r.randrange(G32(ctx, 0x1015dc64)))
    w32(ctx, 0x1015eca0, zbuf(ctx, 0x40))
    w32(ctx, 0x1015ec88, zbuf(ctx, 0x50))
    w32(ctx, 0x1015dc78, r.getrandbits(31))
    w32(ctx, 0x1015ec58, r.choice([2, 3, 8, 1, 0]))
    w32(ctx, 0x1015f1fc, r.choice([0, 0, 1]))


def G32(ctx, addr):
    return r32(ctx.m.uc, addr)


def free_player(ctx, entries):
    """Makes one player of the list free (no +0x160 mark, state 0 or 1), so the round-robin search of
    Net_UpdatePlayers ends."""
    r = ctx.rng
    e = entries[r.choice(ctx.m._ids)]
    w32(ctx, e + 0x160, 0)
    w32(ctx, e + 0x14, r.choice([0, 1]))


# ---- generators ------------------------------------------------------------------------------------------------
def g_record(ctx):
    r = begin(ctx)
    if r.random() < 0.7:
        row = r.choice([0, 4, 5, 9, 0xa, 0xe, 0x12, 0x13, 0x1a, 0x4d, r.randrange(256)])
        return [(r.choice([0, 0x1000, 0x2000, 0x3000, 0x4000, 0x5000, 0x6000, 0x7f000]) | (r.randrange(16) << 8) | row)]
    return [ctx.u32()]


def g_register(ctx):
    r = begin(ctx)
    for a in (0x1017e800, 0x1017e7ec, 0x1017e7f0, 0x1017e7f8, 0x1017e7fc, 0x1017e7f4):
        w32(ctx, a, r.choice([0, 1, 1, r.getrandbits(32)]))
    return []


def g_state(ctx):
    r = begin(ctx)
    count, self_, entries = table(ctx)
    net_globals(ctx)
    index = r.randrange(count) if r.random() < 0.95 else r.randrange(count, 0x19)
    if index < count:
        e = entries[index]
        blk = r32(ctx.m.uc, e + 0x28)
        if r.random() < 0.6:
            w32(ctx, e + 0xf0, 0x400 | r.choice([0, 1, 2, 7, r.randrange(256)]))
        kind = G32(ctx, e + 0xf0) & 0xff
        w16(ctx, 0x109e9ba0 + kind * 0x50, r.choice([0x20, 0x20, 0, 0xffff, r.getrandbits(16)]))
        w16(ctx, 0x109e9ba6 + kind * 0x50, r.choice([0xffff, 0x8000, r.getrandbits(16)]))
        w32(ctx, e + 8, r.choice([r.randrange(32), r.randrange(40), 31, 32, r.getrandbits(32)]))
        for off in (0x58, 0x5c, 0x60, 0x64, 0x68, 0x6c):
            w32(ctx, e + off, fbits(ctx, 1e5))
        for off in (0x520, 0x588, 0x2c, 0x7c, 0x38, 8, 0x18):
            w32(ctx, blk + off, fbits(ctx, r.choice([1e5, 100.0, 40000.0])))
        for off in (0xc1d, 0xc19, 0xc21, 0xc25):
            w32(ctx, blk + off, r.choice([0, 0, 1, 2, r.getrandbits(32)]))
        w32(ctx, blk + 0xc29, r.choice([0, 1, 2, 3]))
        w32(ctx, blk + 0xc2d, r.choice([0, 1]))
        w16(ctx, blk + 0x560, r.getrandbits(16))
        w16(ctx, blk + 0x562, r.getrandbits(16))
        obj = newbuf(ctx, 0x60)
        rec = newbuf(ctx, 0x60)
        wb(ctx, rec + 0x59, bytes([r.choice([0, 0, 1, 0xff])]))
        w32(ctx, obj + 0x54, rec)
        w32(ctx, blk, obj)
        w32(ctx, blk + 0x510, r.choice([0xffffffff, 0, 1, 100]))
        w32(ctx, blk + 0x514, r.choice([0, 1]))
        w32(ctx, blk + 0x518, r.choice([0, 1]))
        if r.random() < 0.4:
            w32(ctx, blk + 0x4f0, 2)
    return [index, r.choice([0, 1, 2])]


def g_update(ctx):
    r = begin(ctx, 0x1003eebc)
    count, self_, entries = table(ctx)
    net_globals(ctx)
    if r.random() < 0.5:
        # often: a driven entry that is dead and not yet handled, with respawns left
        e = entries[r.randrange(count)]
        w32(ctx, e + 0x14, 3)
        w32(ctx, e + 0xc, self_)
        w32(ctx, e + 0x164, 0)
        w16(ctx, e + 0x1c, r.choice([2, 4, 6]))
    # last, so that the free player stays free (else the round-robin search never ends, on both sides)
    free_player(ctx, entries)
    return []


TAGS = [b'eD', b'DA', b'WE', b'DM', b'KI', b'CO', b'GO', b'SN', b'SU', b'EG', b'SS', b'CH', b'LA', b'CA', b'TC',
        b'AI', b'DS', b'RG', b'NR', b'OR', b'PU', b'DR', b'KA']
EXTRA = {b'WE': 5, b'DM': 0x16, b'KI': 0xa, b'CO': 0x1a, b'GO': 2, b'SU': 2, b'EG': 2, b'SS': 2}


def g_pump(ctx):
    r = begin(ctx, 0x1006a12d, 0x1003eebc, 0x1004227f)
    count, self_, entries = table(ctx)
    net_globals(ctx)
    w32(ctx, 0x1017b8b8, r.choice([0, 1]))
    w32(ctx, 0x1015ecdc, r.choice([0, 1]))
    w32(ctx, 0x1017b8bc, r.getrandbits(32))
    w32(ctx, 0x1015eccc, r.choice([1, 1, 1, 1, 0, 2]))
    w32(ctx, 0x1015ec58, r.choice([2, 3, 8, 8, 1, 0]))
    w32(ctx, 0x1015d008, zbuf(ctx, 0x60))
    w32(ctx, 0x1017b94c, r.choice([0, 1, 1, 2, 3, r.getrandbits(32)]))
    t = ST['t0']
    w32(ctx, 0x1015db00, *[t - r.choice([0, 1, 0x38e, 0x38f, 0x1000, -5]) for _ in range(8)])
    w32(ctx, 0x1015db40, *[r.choice([0, 1]) for _ in range(8)])
    w32(ctx, 0x1015d08c, r.choice([0x27, 2, r.getrandbits(8)]))
    w16(ctx, 0x1015ec7c, r.choice([0, 1, 1, 0xffff]))
    w32(ctx, 0x1015ece0, t - r.choice([-5, 0, 1, 100]))
    w32(ctx, 0x1015ece4, t - r.choice([-5, 0, 1, 100]))
    w32(ctx, 0x1015ec68, r.getrandbits(32))
    w32(ctx, 0x1013aca0, r.choice([0, 1, 2]))
    w32(ctx, 0x1013aca4, r.choice([0, 1, 2]))
    root = G32(ctx, 0x1015d004)
    wb(ctx, root + 0x137, bytes([r.choice([0, 1, 2]), r.choice([0, 1, 2])]))
    w32(ctx, 0x1015ec54, r.choice([0, 1]))
    wb(ctx, 0x1015d090, rb(ctx, 8))
    w32(ctx, 0x1015dc84, r.choice([2, 2, 0, 1]))
    w32(ctx, 0x1015ec70, r.getrandbits(32))
    w32(ctx, 0x1015ec6c, r.getrandbits(32))
    w16(ctx, 0x1015ec60, r.getrandbits(16))
    # the CRT's locale handle for LC_CTYPE: 0 (the C locale, what the game runs with) keeps Str_CompareNoCase on its
    # ASCII path; any other value makes it lower-case through the LCMapStringA import, whose stub never writes
    # the result, so the compare would depend on stack garbage
    w32(ctx, 0x1017e950, 0)
    w32(ctx, 0x10372d98, r.getrandbits(32))
    free_player(ctx, entries)
    msgs = []
    for _ in range(r.choice([0, 1, 1, 2, 3, 5])):
        tag = r.choice(TAGS + [b'xx', b'da', b'Da', b'ed', b'ss'])
        key = r.choice(ctx.m._keys + [0xfa01, 0x1234, ctx.m._keys[self_]])
        if tag.lower() == b'ed':
            typ = r.choice([0, 0, 0, key])
        else:
            typ = r.choice([key, key, 0, r.getrandbits(16)])
        extra = r.choice([EXTRA.get(tag.upper(), 2), EXTRA.get(tag.upper(), 2), 5, 0x27, r.getrandbits(8)])
        data = bytearray(rb(ctx, 0x40))
        data[0:2] = tag
        if typ == 0:
            data[2:4] = struct.pack('<H', key)
        msgs.append((typ, extra, bytes(data)))
    ST['msgs'] = msgs
    return []


def g_reset(ctx):
    r = begin(ctx)
    e = newbuf(ctx, 0x100)
    w32(ctx, e + 4, r.randrange(25))
    return [e]


JOY_NAMES = [b'Tracker', b'Microsoft SideWinder', b'CH Flightstick Pro', b'Thrustmaster FCS', b'Gravis GamePad',
             b'', b'sidewinder', b'TrackerFlightstick Pro', b'xThrustmaster']


def g_joy(ctx):
    r = begin(ctx)
    dev = zbuf(ctx, 0x80)
    name = r.choice(JOY_NAMES) if r.random() < 0.85 else text(ctx, b'TrackSideWFlightPo', r.randrange(0, 30))
    wb(ctx, dev + 0xc, name + b'\0')
    caps = newbuf(ctx, 0x70)
    w16(ctx, caps + 0x60, r.choice([0x33, 0x33, 0x31, 0x32, 0x3, 0, 0xffff, 0x30, r.getrandbits(16)]))
    w32(ctx, caps + 0x3c, r.choice([4, 4, 8, 8, 2, 0, r.getrandbits(32)]))
    return [dev, caps]


def angle(ctx):
    r = ctx.rng
    k = r.random()
    if k < 0.6:
        return f2b(r.uniform(-720.0, 720.0))
    if k < 0.8:
        return f2b(r.choice([0.0, -0.0, 90.0, 180.0, 360.0, -90.0, 45.0, 1e-5, 1e5]))
    return fbits(ctx, 1000.0, special=False)


def g_quat(ctx):
    r = begin(ctx)
    return [angle(ctx), angle(ctx), angle(ctx), r.choice([0, 1, 2, 2, 1, 0, 3, 0xffffffff, 5]), newbuf(ctx, 0x10)]


def g_flash(ctx):
    r = begin(ctx)
    w32(ctx, 0x10179ce0, r.choice([0, 1, 1, 1]))
    for a in (0x101c4eb0, 0x101c4e7c, 0x101c4e94, 0x101c4e58, 0x101c4e64, 0x101c4e40, 0x101c4e48, 0x101c4560):
        w32(ctx, a, cstr(ctx, text(ctx, b'abcXYZ ', r.randrange(0, 20))))
    wb(ctx, 0x101658c0, rb(ctx, 0x18))
    return [r.choice([0, 1, 2, 3, 4, 5, 0xa, 0x14, 0x15, 0x16, 0x100, r.getrandbits(32)])]


def g_kill(ctx):
    r = begin(ctx)
    count, self_, entries = table(ctx)
    net_globals(ctx)
    v = r.randrange(count)
    e = entries[v]
    blk = r32(ctx.m.uc, e + 0x28)
    w32(ctx, e, r.choice([1, 1, 1, 3, 4, 5, 8, 10, 2, 0, 11]))
    for x in entries:
        w32(ctx, x + 8, r.choice([0, 1, 2]))
        w32(ctx, x + 0x10, r.choice([0, 1, 2, 3]))
    w32(ctx, blk + 0x4f0, r.choice([4, 5, 5, 7, 2, r.getrandbits(32)]))
    w32(ctx, blk + 0xbdf, r.getrandbits(32))
    w32(ctx, r32(ctx.m.uc, entries[self_] + 0x28) + 0xbe3, G32(ctx, blk + 0xbdf) - r.choice([0, 3, 5, 6, 7, 100, -2]))
    w32(ctx, 0x101c64f0, r.choice([0, 1]))
    w32(ctx, 0x1017e678, r.choice([0, 0, newbuf(ctx, 0x30)]))
    if G32(ctx, 0x1017e678) != 0:
        w16(ctx, G32(ctx, 0x1017e678) + 2, r.choice([0x100, 0x100, 0]))
        w16(ctx, G32(ctx, 0x1017e678) + 0x24, r.choice([v, r.randrange(8)]))
    w32(ctx, 0x1015ebbc, r.choice([0, 1]))
    w32(ctx, 0x10179cac, r.getrandbits(32))
    w32(ctx, 0x10165512, r.choice([0, 1]))
    w32(ctx, 0x101c42a8, cstr(ctx, b'new leader'))
    wb(ctx, 0x101653e7, rb(ctx, 0x150))
    ST['v2aaf'] = [v, self_, 0, 7, r.randrange(count)]
    ST['v2843'] = [self_, 0, 3, 0xffffffff, r.randrange(count)]
    killer = r.choice([0xfffffffe, 0xffffffff, self_, self_, r.randrange(count), r.randrange(8), 9, 0x8000])
    return [killer, blk]


def chain(ctx, n):
    r = ctx.rng
    head = 0
    for _ in range(n):
        node = newbuf(ctx, 0x10)
        w16(ctx, node + 2, r.choice([0xc0, 0xc5, 0xcf, 0x40, 0x70, 0xc0 | (r.getrandbits(16) & 0xff0f), r.getrandbits(16)]))
        w32(ctx, node + 8, head)
        head = node
    return head


def g_sweep(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b9e0, chain(ctx, r.choice([0, 1, 3, 6])))
    w32(ctx, 0x1017b990, chain(ctx, r.choice([0, 1, 3, 6])))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    for c in range(9):
        for rr in range(9):
            w32(ctx, 0x1017ba30 + c * 0x18c + rr * 0x2c, chain(ctx, r.choice([0, 0, 0, 0, 1, 2])) if r.random() < 0.3 else 0)
    return [r.choice([0, 1, 1, 5])]


def pane(ctx):
    r = ctx.rng
    p = zbuf(ctx, 0x100)
    w16(ctx, p + 4, r.choice([0, 1, 1, 0xffff, 5]))
    w32(ctx, p + 0xf0, newbuf(ctx, 0x10))
    w32(ctx, p + 0xec, r.getrandbits(32))
    w32(ctx, 0x109b9900, cstr(ctx, r.choice([b'font1', b'Hud', b''])))
    w32(ctx, 0x1014cec4, cstr(ctx, r.choice([b'fnt', b'Xel'])))
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    ST['font'] = [r.choice([0, FONT, FONT])]
    return p


def g_score(ctx):
    r = begin(ctx)
    p = pane(ctx)
    w32(ctx, 0x1015ec58, r.choice([0, 1, 2, 8]))
    w32(ctx, 0x1015f208, r.choice([0, 0, 1]))
    w32(ctx, 0x10179cd4, r.choice([0, 0, 1]))
    w32(ctx, 0x1017e5e0, r.choice([0, 0, 1]))
    w32(ctx, 0x101c4c2c, cstr(ctx, r.choice([b'Score: ', b'', b'S'])))
    w32(ctx, 0x10179cac, r.choice([0, 5, 0xfffffffd, 12345, r.getrandbits(32)]))
    return [p]


def g_status(ctx):
    r = begin(ctx, 0x100654a1)
    count, self_, entries = table(ctx)
    p = pane(ctx)
    blk = r32(ctx.m.uc, entries[self_] + 0x28)
    w32(ctx, blk + 0x50c, r.choice([1, 2, 6, 6, 0, 3, r.getrandbits(32)]))
    w32(ctx, 0x101c4c48, cstr(ctx, r.choice([b'Ready', b'', b'\x80x'])))
    wb(ctx, 0x1017e5f8, r.choice([b'\0', b'ab\0', b'X\0', b'\xff\0']))
    wb(ctx, 0x109b992e, bytes([r.getrandbits(8)]))
    return [p]


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Anim_RecordOfId', 0x1001b902, g_record, 600),
    spec('Anim_RegisterNames', 0x1001d0fa, g_register, 300, void=True),
    spec('Net_ProcessMessages', 0x1003e291, g_pump, 1500),
    spec('Net_SendStateMessage', 0x1003eebc, g_state, 1200, void=True),
    spec('Net_UpdatePlayers', 0x1004227f, g_update, 1500, void=True),
    spec('Sim_100500ab', 0x100500ab, g_reset, 300),
    spec('Joy_PickProfileName', 0x100572c9, g_joy, 600),
    spec('Quat_FromAxisAngles', 0x1006123b, g_quat, 1000, void=True),
    spec('FlashText_Start', 0x100654a1, g_flash, 400, void=True),
    spec('Score_RecordKill', 0x1006a12d, g_kill, 1500, void=True),
    spec('Sim_1008a94d', 0x1008a94d, g_sweep, 500, void=True),
    spec('Hud_DrawScore', 0x100cf69a, g_score, 500, void=True),
    spec('Hud_DrawStatusText', 0x100cf7e3, g_status, 600, void=True),
]
