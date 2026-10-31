# Differential tests for round 8, letter x (see difftest.py and docs/checker.md).
#
# Callees are either run for real on both sides (the original callee in the original run, its C conversion in the
# C run; both are verified already) or replaced, for the trials that ask for it, by a stub: an emulator code hook
# that appends ('stub_<address>', argument words) to the checker's call log (compared between the two runs like the
# Windows calls; stack addresses shown as 'stack'), may write results through pointer arguments (the same bytes
# for both runs, computed from what it is given), answers with a value chosen by the generator and returns to the
# caller (EIP/ESP set; no code bytes are patched).
#  * Callees that are not converted (or that another hook file patches) are called by both versions at their
#    original address, where the stub sits.
#  * Converted callees with a large world of their own are stubbed at their original address (reached by the
#    original code through its jump thunk) AND at the address of their C function in the test image (reached by
#    the C code), under the same log name.
# Pointer arguments to small records are logged by the bytes they point to (DEREF), texts by their content (SARGS),
# arguments read as 16 / 8 bits are masked (MASK). Indirect calls (the display routines 0x1014cb40 / 0x1014cb4c,
# the display driver's table entries, the callbacks of views and panes) go to `ret`s in a code area with a logging
# hook; the polygon routine 0x1014cb4c writes the count, the flags and the vertex fields the caller sets into a log
# buffer in argument memory (compared like all memory).
# Running for real: the C library fmod 0x100fcb30 and sqrt 0x100fca04, the table lookups 0x10084766 / 0x1008486e,
# Sim_1008e211 (with 0x100fcb70), Sim_1008e489, Sim_10084a94, the random ring Sim_1009c929, srand / rand, the
# string helpers, Sim_100a9cfb, Sim_100a9d21, Sim_1009a6b3 and the converted functions of this letter that call
# each other (Sim_100a4845 recursion, Sim_1009db70 recursion, Sim_10091223 .. on the ring).
# Every FPU spec here is checked bit for bit (no float tolerance); the x87 runs with the game's control word 0x27f.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import (UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW,
                               UC_X86_REG_FPTAG)

SOURCES = [
    'sim/unsorted_1008/w8x_cells.c', 'sim/unsorted_1008/w8x_hud.c', 'sim/unsorted_1009/w8x_effects.c',
    'sim/unsorted_1009/w8x_views.c', 'sim/unsorted_1009/w8x_needles.c', 'sim/unsorted_1009/w8x_radar.c', 'sim/unsorted_1009/w8x_misc.c',
    'sim/unsorted_100a/w8x_scene.c',
    # converted callees and what they need
    'sim/core/timer.c', 'sim/core/w2a_timers.c', 'sim/render/w2n_polygon.c', 'sim/render/w2n_project.c',
    'sim/render/w3p_view.c', 'sim/render/w3q_vertex.c', 'sim/render/w3r_screen.c', 'sim/render/w7o_draw.c',
    'sim/unsorted_1000/w2a_fields.c', 'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1000/w2e_names.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2i_misc.c',
    'sim/unsorted_1000/w2j_misc.c', 'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1000/w2k_misc.c',
    'sim/unsorted_1000/w2l_fields.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w4w_blocks.c', 'sim/unsorted_1000/w5a_misc.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/unsorted_1000/w5y_misc.c', 'sim/unsorted_1001/w2e_names.c', 'sim/unsorted_1001/w2e_tables.c',
    'sim/unsorted_1001/w2h_tables.c', 'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w3t_misc.c',
    'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w5x_calls.c', 'sim/unsorted_1001/w5y_misc.c',
    'sim/unsorted_1002/w2e_object.c', 'sim/unsorted_1002/w2g_records.c', 'sim/unsorted_1002/w2h_misc.c',
    'sim/unsorted_1002/w2i_misc.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w2l_tables.c',
    'sim/unsorted_1002/w4u_heap.c', 'sim/unsorted_1002/w5a_misc.c', 'sim/unsorted_1002/w5y_misc.c',
    'sim/unsorted_1003/bitmap.c', 'sim/unsorted_1003/setters.c', 'sim/unsorted_1003/w2b_misc.c',
    'sim/unsorted_1003/w2e_misc.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_1003/w2g_misc.c',
    'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/w4u_heap.c',
    'sim/unsorted_1003/w5a_misc.c', 'sim/unsorted_1003/w5c_calls.c', 'sim/unsorted_1004/fields.c',
    'sim/unsorted_1004/state.c', 'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/w2l_values.c',
    'sim/unsorted_1005/w2h_tables.c', 'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1005/w2k_misc.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w5a_math.c', 'sim/unsorted_1007/w2e_state.c',
    'sim/unsorted_1007/w5a_misc.c', 'sim/unsorted_1008/cell_link.c', 'sim/unsorted_1008/w2g_rect.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/w3q_place.c', 'sim/unsorted_1008/w3r_grid.c', 'sim/unsorted_1008/w3s_grid.c',
    'sim/unsorted_1008/w4u_misc.c', 'sim/unsorted_1008/w4w_display.c', 'sim/unsorted_1008/w5a_misc.c',
    'sim/unsorted_1008/w5b_calls.c', 'sim/unsorted_1008/w5x_calls.c', 'sim/unsorted_1008/w5y_misc.c',
    'sim/unsorted_1008/w6f_camera.c', 'sim/unsorted_1008/w6f_display.c', 'sim/unsorted_1008/w7n_misc.c',
    'sim/unsorted_1009/w2f_misc.c', 'sim/unsorted_1009/w2g_objects.c', 'sim/unsorted_1009/w2j_misc.c',
    'sim/unsorted_1009/w2j_tables.c', 'sim/unsorted_1009/w5y_misc.c', 'sim/unsorted_1009/w6f_modes.c',
    'sim/unsorted_100a/w2h_misc.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w5b_calls.c',
    'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w5z_calls.c',
    'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2e_strings.c',
    'sim/unsorted_100b/w2h_records.c', 'sim/unsorted_100b/w2l_records.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100b/w5c_calls.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/w2e_tables.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w3o_misc.c',
    'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w5a_misc.c', 'sim/unsorted_100d/state.c',
    'sim/unsorted_100d/w2f_records.c', 'sim/unsorted_100d/w2g_misc.c', 'sim/unsorted_100d/w2j_nodes.c',
    'sim/unsorted_100d/w5c_calls.c', 'sim/unsorted_100f/w2e_string.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_1010/w2f_misc.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w6i_crt.c', 'sim/unsorted_1010/w6k_text.c', 'sim/util/field.c', 'sim/util/lookup.c',
    'sim/util/record.c', 'sim/util/ring.c', 'sim/view/matrix.c', 'sim/view/objlist.c', 'sim/view/viewrect.c',
    'sim/view/w2d_view.c', 'sim/view/w2l_lists.c', 'sim/win/w4a_heap.c', 'sim/win/w4u_cdaudio.c',
    'sim/win/w4u_gdi.c', 'sim/win/w4v_dll.c', 'sim/win/w4v_heap.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
STACK_TOP = 0x00800000
CODE = 0x00AF8000       # targets of indirect calls (a `ret` each, with a logging hook); slot 0 = polygon routine
DLOG_CAP = 0x4000

# not converted (or patched elsewhere): address -> number of argument words logged
STUBS = {
    0x100ba7d5: 3, 0x100f0205: 5, 0x100e91dc: 5, 0x100ef727: 7, 0x100e86a1: 7, 0x100ecce0: 1, 0x100eccf3: 2,
    0x100ed743: 2, 0x100e84f0: 4, 0x10057f83: 0, 0x100766ca: 1, 0x100282d4: 4, 0x10055048: 0, 0x100eb6e8: 6,
    0x100eba29: 6, 0x1008ec46: 5, 0x10098f8c: 5, 0x100d3510: 3, 0x1003b534: 0, 0x1009a137: 1, 0x1009b03d: 1,
    0x100eb070: 2, 0x10048287: 4, 0x100cbc6b: 7, 0x100d2669: 3, 0x10087d83: 1, 0x100a4700: 2, 0x100a539e: 1,
}
# converted, stubbed on both sides: C name -> (original address, number of argument words logged)
DSTUBS = {
    'Sim_10088804': (0x10088804, 1), 'Sim_1008a8f5': (0x1008a8f5, 2), 'List_InsertBefore': (0x10088c9f, 2),
    'View_ScaleRect': (0x1008ca50, 3), 'Sim_1008d2d0': (0x1008d2d0, 4), 'Sim_1008b8de': (0x1008b8de, 0),
    'Sim_1000edce': (0x1000edce, 0), 'Sim_1000e983': (0x1000e983, 4), 'Sim_1001f454': (0x1001f454, 0),
    'Sim_1008b900': (0x1008b900, 0), 'Sim_100f2cec': (0x100f2cec, 2), 'Rect_ScalePoint': (0x1008cdc9, 3),
    'Sim_10008f1c': (0x10008f1c, 4), 'Sim_10009136': (0x10009136, 12), 'Sim_100091f7': (0x100091f7, 3),
    'Sim_10009278': (0x10009278, 3), 'Sim_100a9830': (0x100a9830, 2), 'Sim_100a9b4c': (0x100a9b4c, 2),
    'Sim_100096f9': (0x100096f9, 2), 'Sim_10009408': (0x10009408, 2), 'Sim_100094ba': (0x100094ba, 1),
    'Sim_100a9d47': (0x100a9d47, 4), 'Sim_100aa742': (0x100aa742, 1), 'Sim_10089e91': (0x10089e91, 1),
    'Sim_100aa351': (0x100aa351, 1), 'Sim_100aa406': (0x100aa406, 1), 'Sim_100905e8': (0x100905e8, 0),
    'Sim_10090c7d': (0x10090c7d, 4), 'Sim_100aa39f': (0x100aa39f, 1), 'Sim_100aa799': (0x100aa799, 1),
    'Sim_100999fc': (0x100999fc, 0), 'Sim_1009bba2': (0x1009bba2, 0), 'Sim_10099c59': (0x10099c59, 0),
    'Sim_1008fb20': (0x1008fb20, 4), 'Sim_100448ff': (0x100448ff, 0), 'Sim_100448cd': (0x100448cd, 1),
    'Sim_1008fce3': (0x1008fce3, 1), 'Sim_1008fd5a': (0x1008fd5a, 0), 'Sim_1009ace7': (0x1009ace7, 2),
    'Sim_1009ae96': (0x1009ae96, 3), 'Sim_1008f630': (0x1008f630, 1), 'Sim_1009b8d8': (0x1009b8d8, 4),
    'Sim_1009ba57': (0x1009ba57, 0), 'Sim_10090099': (0x10090099, 1), 'Sim_100bb2b0': (0x100bb2b0, 2),
    'Sim_1009a282': (0x1009a282, 8), 'Sim_100b53f0': (0x100b53f0, 4), 'Sim_10052be7': (0x10052be7, 1),
    'Sim_10052c02': (0x10052c02, 1), 'Sim_100a9ccf': (0x100a9ccf, 4), 'Sim_100a4510': (0x100a4510, 2),
    'Sim_100a88db': (0x100a88db, 0), 'Sim_100a4933': (0x100a4933, 1),
}
MASK = {(0x100091f7, 1): 0xffff, (0x10009278, 2): 0xff, (0x10009408, 1): 0xffff}
SARGS = {0x100ba7d5: (0, 1), 0x1000e983: (0,), 0x100bb2b0: (0,), 0x1009a282: (4,)}
DEREF = {0x100eba29: {0: 20}, 0x1008ec46: {1: 8}, 0x1008fb20: {0: 24}, 0x1008cdc9: {1: 8}, 0x10090099: {0: 12}}
EFFECT = {}
ST = {}


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def ur32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def uw32(uc, addr, *vals):
    uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def bitsf(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xffffffff))[0]


def f32(x):
    return bitsf(fbits(max(-3.4e38, min(3.4e38, x))))


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


def rfl_nan(ctx, scale=1000.0):
    """As rfl, now and then a quiet NaN or an infinity (never a signalling NaN)."""
    r = ctx.rng
    if r.random() < 0.06:
        return r.choice([0x7fc00000, 0xffc00000, 0x7f800000, 0xff800000])
    return rfl(ctx, scale)


def sane_floats(ctx, addr, n, scale=1000.0):
    w32(ctx, addr, *[rfl(ctx, scale) for _ in range(n)])


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rb(ctx, extra))


def word(ctx, *picks):
    return ctx.rng.choice(picks) & 0xffff


# ---- stubs --------------------------------------------------------------------------------------
def _ustr(uc, addr, limit=64):
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
                v = _ustr(uc, v)
            elif k in DEREF.get(addr, {}):
                try:
                    v = bytes(uc.mem_read(v, DEREF[addr][k]))
                except Exception:
                    v = ('bad', v)
            args.append('stack' if isinstance(v, int) and STACK_LO <= v <= STACK_HI else v)
        k = _count(m, name)
        n = len(m.api_log)
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


def _driver_cb(m):
    """The polygon routine (code slot 0): count, flags and the set vertex fields into the log buffer."""
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        count, verts, flags = struct.unpack('<3I', bytes(uc.mem_read(esp + 4, 12)))
        log = ST['dlog']
        pos = ur32(uc, log)
        rec = [0xd0d0d0d0, count, flags]
        for i in range(min(count, 16)):
            try:
                v = struct.unpack('<9I', bytes(uc.mem_read(verts + 36 * i, 36)))
            except Exception:
                rec.append(0xbadbad)
                break
            rec += [v[k] for k in ST['dfields']]
        data = struct.pack('<%dI' % len(rec), *rec)
        if pos + len(data) <= DLOG_CAP:
            uc.mem_write(log + 4 + pos, data)
            uc.mem_write(log, struct.pack('<I', pos + len(data)))
        uc.reg_write(UC_X86_REG_EAX, mix(ST['seed'], pos))
    return cb


def _code_cb(m):
    def cb(uc, address, size, ud):
        k = (address - CODE) // 16
        if k == 0:
            return
        esp = uc.reg_read(UC_X86_REG_ESP)
        nargs = ST['code_nargs'].get(k, 0)
        raw = list(struct.unpack('<%dI' % nargs, bytes(uc.mem_read(esp + 4, 4 * nargs)))) if nargs else []
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v for v in raw)
        name = 'code_%d' % k
        j = _count(m, name)
        m.api_log.append((name, args))
        val = None
        if k in ST['code_eff']:
            val = ST['code_eff'][k](uc, j, raw)
        if val is None:
            rets = ST['code_rets'].get(k) or [mix(ST['seed'], k, j)]
            val = rets[j % len(rets)]
        uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
    return cb


def begin(ctx, *active, fill=False):
    """Installs the hooks (once per machine), activates the given stubs (addresses; converted ones by their
    original address), the x87 state of the game (control word 0x27f), the random ring of Sim_1009c929, the
    polygon routine and its log. Must come first in a generator."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w8x', False):
        m._w8x = True
        for addr, nargs in STUBS.items():
            m.raw.hook_add(UC_HOOK_CODE, _stub_cb(m, addr, nargs), None, addr, addr)
        for cname, (addr, nargs) in DSTUBS.items():
            cb = _stub_cb(m, addr, nargs)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
            m.raw.hook_add(UC_HOOK_CODE, cb, None, m.syms[cname], m.syms[cname])
        m.raw.hook_add(UC_HOOK_CODE, _driver_cb(m), None, CODE, CODE)
        m.raw.hook_add(UC_HOOK_CODE, _code_cb(m), None, CODE + 16, CODE + 16 * 64 - 1)
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    ST['active'] = set(active)
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {a: [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)] for a in STUBS}
    for _, (a, _n) in DSTUBS.items():
        ST['rets'][a] = [r.choice([0, 1, 2, 0xffffffff, r.getrandbits(32)]) for _ in range(3)]
    ST['code_nargs'] = {}
    ST['code_rets'] = {}
    ST['code_eff'] = {}
    ST['dfields'] = (0, 1, 2, 3, 4, 7, 8)
    ST['dlog'] = zbuf(ctx, 4 + DLOG_CAP + 4)
    m.uc.mem_write(CODE, b'\xc3' * (16 * 64))
    w32(ctx, 0x1014cb4c, CODE)
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101cffc0, *[r.choice([r.randrange(0, 1000), r.getrandbits(31), r.randrange(0, 50)]) for _ in range(0x7f)])
    if fill:
        # the stack below the arguments is filled with one repeated byte, so locals the original leaves unset
        # (and hands to its callees) read the same in both runs
        pat = r.choice([0, 0x55, 0xa5, 0x3f, 0x41])
        m.uc.mem_write(STACK_TOP - 0x20000, bytes([pat]) * (0x20000 - 0x100))
    return r


def rets(addr, *values):
    ST['rets'][addr] = list(values)


def code(k, nargs, *values, eff=None):
    """Address of code slot k (a `ret` with a logging hook) for an indirect call with nargs argument words."""
    ST['code_nargs'][k] = nargs
    if values:
        ST['code_rets'][k] = list(values)
    if eff is not None:
        ST['code_eff'][k] = eff
    return CODE + 16 * k


def drv_table(ctx, k, off, nargs=0):
    """A display driver table whose entry `off` is code slot k."""
    t = newbuf(ctx, 0x40)
    w32(ctx, t + off, code(k, nargs))
    return t


# ---- effects of stubbed helpers ---------------------------------------------------------------------
def e_upper(uc, k, raw):
    """0x100ba7d5 upper-cases its two texts in place (as the original does)."""
    for a in raw[:2]:
        s = _ustr(uc, a)
        if isinstance(s, bytes):
            uc.mem_write(a, s.upper())
    return None


def e_width(uc, k, raw):
    return (mix(ST['seed'], raw[1] & 0xffff, 7) % 13) + ST.get('wbias', 0)


def e_project(uc, k, raw):
    """0x100d2669(&x, &y, &z) projects in place: new x, y (from the values it is given), z kept."""
    x = bitsf(ur32(uc, raw[0]))
    y = bitsf(ur32(uc, raw[1]))
    uc.mem_write(raw[0], struct.pack('<f', f32(x * 0.5 + 160.0)))
    uc.mem_write(raw[1], struct.pack('<f', f32(120.0 - y * 0.25)))
    return ST['vis'][k % len(ST['vis'])]


def e_screen(uc, k, raw):
    """Sim_10090099(v): v[0], v[1] become screen coordinates (floats), the answer is chosen by the generator."""
    v = ST['scr'][k % len(ST['scr'])]
    uc.mem_write(raw[0], struct.pack('<2I', v[0], v[1]))
    return ST['vis'][k % len(ST['vis'])]


def e_wait(uc, k, raw):
    if k + 1 >= ST['wait']:
        uc.mem_write(0x1015fd4c, struct.pack('<H', ST['waitval']))
    return None


def e_scalept(uc, k, raw):
    x, y = struct.unpack('<2i', bytes(uc.mem_read(raw[1], 8)))
    uw32(uc, raw[2], x * 3 + 1, y - 7)
    return None


def e_ec46(uc, k, raw):
    uw32(uc, raw[4], mix(ST['seed'], k, 1), mix(ST['seed'], k, 2))
    return None


def e_block(uc, k, raw):
    """Sim_10008f1c(a, b, c, &d): an object while the generator's count lasts, d written."""
    uw32(uc, raw[3], ST['d_vals'][k % len(ST['d_vals'])])
    return ST['objs'][k] if k < len(ST['objs']) else 0


def e_cbc6b(uc, k, raw):
    for j in range(4):
        uw32(uc, raw[3 + j], ST['cbc'][(k + j) % len(ST['cbc'])])
    return None


def e_b53f0(uc, k, raw):
    for j in range(3):
        uw32(uc, raw[1 + j], mix(ST['seed'], k, j))
    return None


EFFECT.update({0x100ba7d5: e_upper, 0x100eccf3: e_width, 0x100f2cec: e_width, 0x100d2669: e_project,
               0x10090099: e_screen, 0x10055048: e_wait, 0x1008cdc9: e_scalept, 0x1008ec46: e_ec46,
               0x10008f1c: e_block, 0x100cbc6b: e_cbc6b, 0x100b53f0: e_b53f0})

HUD_DRAW = (0x100f0205, 0x100e91dc, 0x100ef727, 0x100e86a1)


def hud_world(ctx):
    r = ctx.rng
    w32(ctx, 0x1017b940, r.choice([0, 1, 1, 3]))
    wb(ctx, 0x101659e0, rb(ctx, 0x200))
    w32(ctx, 0x101668e8, r.choice([0, 1]))
    w32(ctx, 0x1014cec0, cstr(ctx, r.choice([b'shp', b'Tlt', b'xel', b'gdf'])))


def rect(ctx, spread=700):
    r = ctx.rng
    x0, y0 = r.randrange(-50, 400), r.randrange(-50, 300)
    w, h = r.choice([r.randrange(1, spread), r.randrange(-5, 40), 640]), r.choice([r.randrange(1, spread), 480, 1])
    return ctx.buf(0x20, struct.pack('<5i', r.randrange(-10, 10), x0, y0, x0 + w - 1, y0 + h - 1) + rb(ctx, 12))


def camera(ctx, scale=3000.0):
    cam = ctx.buf(0x140, b''.join(struct.pack('<I', rfl(ctx, scale)) for _ in range(0x50)))
    w32(ctx, 0x1014cae0, cam)
    return cam


# ---- 0x10089e91 / 0x100908b3 -----------------------------------------------------------------------
def g_9e91(ctx):
    r = begin(ctx, 0x10088804, 0x1008a8f5, 0x10088c9f)
    w32(ctx, 0x1017c8cc, r.choice([0x1017b988] * 6 + [0x1017b9d8, 0]))
    w32(ctx, 0x1017c8c4, r.choice([0, 1, 1, 1]))
    lists = [newbuf(ctx, 0x40) for _ in range(2)]
    rets(0x10088804, *lists)
    obj = newbuf(ctx, 0x100)
    w16(ctx, obj, r.choice([0, 0, 0x1000, 0x800, 0x1800, r.getrandbits(16)]))
    w32(ctx, obj + 0x34, r.choice([4, 0, 1, 3, 4, ctx.u32()]))
    x = r.choice([r.uniform(-254008, 246000), r.uniform(-300000, 300000), -254008.0, 0.0, 55562.0 * r.randrange(-6, 6)])
    z = r.choice([r.uniform(-246044, 254000), r.uniform(-300000, 300000), 0.0, 55562.0 * r.randrange(-6, 6) - 0.5])
    if r.random() < 0.5:
        node = newbuf(ctx, 0x80)
        wf(ctx, node + 0x70, x, r.uniform(-100, 100), z)
        w32(ctx, obj + 0x28, node)
    else:
        w32(ctx, obj + 0x28, 0)
        wf(ctx, obj + 0x44, x)
        wf(ctx, obj + 0x4c, z)
    if r.random() < 0.05:
        w32(ctx, obj + 0x44, rfl_nan(ctx, 1e9))
        w32(ctx, obj + 0x4c, rfl_nan(ctx, 1e9))
    col = int((x + 254008.0) / 55562.0 + 0.5)
    row = int((z + 246044.5) / 55562.0 + 0.5)
    for c in {(col, row), (r.randrange(9), r.randrange(9))}:
        if c[0] in range(9) and c[1] in range(9):
            w32(ctx, 0x1017ba38 + c[0] * 0x18c + c[1] * 0x2c, r.choice([0, newbuf(ctx, 0x20)]))
    outside = newbuf(ctx, 0x40)
    w32(ctx, outside + 0x10, r.choice([0, newbuf(ctx, 0x20)]))
    w32(ctx, 0x1017c8c8, outside)
    w32(ctx, 0x1017b998, r.choice([0, newbuf(ctx, 0x20)]))
    return [0 if r.random() < 0.03 else obj]


def g_08b3(ctx):
    r = begin(ctx, 0x10008f1c, 0x10009136, 0x100091f7, 0x10009278, 0x100a9830, 0x100a9b4c, 0x100096f9, 0x10009408,
              0x100094ba, 0x100a9d47, 0x100aa742, 0x10089e91, 0x100aa351, 0x100aa406, 0x100905e8)
    n = r.choice([0, 1, 2, 5, 12, 30]) if r.random() < 0.97 else 1000
    ST['objs'] = [newbuf(ctx, 0x40) for _ in range(min(n, 4))]
    ST['objs'] = [ST['objs'][i % len(ST['objs'])] for i in range(n)] if n else []
    ST['d_vals'] = [ctx.u32() for _ in range(3)]
    rets(0x100091f7, newbuf(ctx, 0x40), newbuf(ctx, 0x40))
    rets(0x100a9830, newbuf(ctx, 0x40), ctx.u32())
    w32(ctx, 0x1017cc20, r.choice([0, 3, ctx.u32()]))
    return []


# ---- 0x1008b6a0 .. 0x1008e9af -----------------------------------------------------------------------
def g_b6a0(ctx):
    r = begin(ctx, 0x100ba7d5, 0x1008ca50, 0x1008d2d0, *HUD_DRAW)
    hud_world(ctx)
    w32(ctx, 0x109e85c4, cstr(ctx, r.choice([b'title', b'Logo', b'A', b''])))
    w32(ctx, 0x1014b94c, r.choice([0, 1, 1, ctx.u32()]))
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    return []


def g_b78f(ctx):
    r = begin(ctx, 0x100766ca, 0x1008b8de, 0x1000edce, 0x1000e983, 0x100282d4, 0x1008ca50, 0x1008d2d0, 0x10055048,
              0x1001f454, 0x1008b900, *HUD_DRAW)
    hud_world(ctx)
    rets(0x100766ca, r.choice([0, 0, 0, 1, 5]))
    w32(ctx, 0x1015ecd0, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1014b94c, r.choice([0, 1, 1]))
    w32(ctx, 0x1014cf8c, ctx.u32())
    w32(ctx, 0x101c64f0, ctx.u32())
    w32(ctx, 0x1017b920, r.choice([0, 1]))
    w32(ctx, 0x1015e68c, drv_table(ctx, 1, 0x18))
    rets(0x100282d4, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    rets(0x1000edce, ctx.u32(), ctx.u32())
    ST['wait'] = r.choice([1, 1, 2, 3, 6])
    ST['waitval'] = r.choice([1, 0x8000, 0xffff, 0x1234])
    w16(ctx, 0x1015fd4c, r.getrandbits(16))
    return []


def g_d65d(ctx):
    r = begin(ctx, 0x100ecce0, *HUD_DRAW)
    hud_world(ctx)
    rets(0x100ecce0, r.randrange(0, 20), r.randrange(-5, 30))
    return [rect(ctx), ctx.u32(), r.randrange(-100, 500) & 0xffffffff, newbuf(ctx, 0x20), r.randrange(0, 256)]


def text(ctx, lines=False):
    r = ctx.rng
    n = r.choice([0, 1, 3, 8, 20])
    out = bytearray()
    for _ in range(n):
        c = r.random()
        if c < 0.5:
            out.append(r.randrange(0x20, 0x7f))
        elif c < 0.6 and lines:
            out.append(0x0a)
        else:
            out.append(r.choice([0x81, 0x85, 0x86, 0x90, 0x9f, 0xa0, 0xdf, 0xe0, 0xeb, 0xec, 0xee, 0xef, 0xf5, 0xf9,
                                 0xfa, 0xfc, 0xff, r.randrange(0x80, 0x100)]))
    return ctx.buf(n + 24, bytes(out) + bytes(24))


def g_d6f7(ctx):
    r = begin(ctx, 0x100ecce0, 0x100eccf3, 0x100f2cec, *HUD_DRAW)
    hud_world(ctx)
    rets(0x100ecce0, r.randrange(0, 20), r.randrange(-5, 30))
    ST['wbias'] = r.choice([0, 0, -3, 100])
    return [rect(ctx), text(ctx), r.randrange(-100, 600) & 0xffffffff, r.randrange(-100, 500) & 0xffffffff,
            newbuf(ctx, 0x20), r.randrange(0, 256)]


def g_de0a(ctx):
    r = begin(ctx, 0x100ecce0, 0x100eccf3, 0x100f2cec)
    hud_world(ctx)
    rets(0x100ecce0, r.randrange(0, 20), r.randrange(1, 30))
    ST['wbias'] = r.choice([0, 0, -3, 40])
    w32(ctx, 0x1014b9a8, r.choice([0x28f, 0x28f, 0, 0x8000, r.randrange(-0x8000, 0xff00)]))
    w32(ctx, 0x1014b9ac, r.choice([0x28f, 0x28f, 0, 0x4000, r.randrange(-0x8000, 0xff00)]))
    t = text(ctx, True) if r.random() < 0.95 else 0
    f = newbuf(ctx, 0x20) if r.random() < 0.95 else 0
    o = newbuf(ctx, 0x20) if r.random() < 0.95 else 0
    return [t, f, o]


def g_df64(ctx):
    r = begin(ctx, 0x1008cdc9, 0x100e84f0, 0x10057f83)
    w32(ctx, 0x1017b920, r.choice([0, 1]))
    w32(ctx, 0x1015e68c, drv_table(ctx, 1, 0x18))
    rets(0x10057f83, r.choice([0x1b, 0xffff001b, 0x1b1b, 0, 0x20, ctx.u32()]))
    a = newbuf(ctx, 0x10)
    w32(ctx, a, r.choice([newbuf(ctx, 0x20), newbuf(ctx, 0x20), 0]))
    return [a if r.random() < 0.95 else 0, rfl_nan(ctx, 70000), rfl_nan(ctx, 70000)]


def g_e0fe(ctx):
    r = begin(ctx, 0x100ed743, 0x100f0205, 0x100e91dc)
    hud_world(ctx)
    w, h = r.choice([1, 8, 16, 64, 200, 0x7fff]), r.choice([1, 8, 16, 64, 200, 0xffff])
    rets(0x100ed743, (w << 16) | h)
    pane = ctx.buf(0x20, struct.pack('<5i', 0, r.randrange(-20, 20), r.randrange(-20, 20), r.randrange(0, 300),
                                      r.randrange(0, 200)) + bytes(12))
    return [pane if r.random() < 0.95 else 0, newbuf(ctx, 0x10) if r.random() < 0.95 else 0, r.randrange(0, 40)]


def g_e320(ctx):
    r = begin(ctx, fill=True)
    cam = camera(ctx)
    w32(ctx, cam + 0x44, r.choice([fbits(1.0), fbits(0.75), fbits(1.3333), rfl(ctx, 3)]))
    angle = r.choice([fbits(r.uniform(-720, 720)), fbits(r.choice([0.0, 90.0, 180.0, 270.0, -90.0, 360.0, 89.999,
                                                                    90.001, 269.99, 180.0001])), rfl(ctx, 1e5)])
    return [rect(ctx), angle, newbuf(ctx, 0x10)]


def g_e707(ctx):
    r = begin(ctx, 0x100eb6e8, 0x100eba29)
    cam = camera(ctx)
    w32(ctx, cam + 0x44, r.choice([fbits(1.0), fbits(0.75), rfl(ctx, 3)]))
    return [rect(ctx), ctx.u32(), r.choice([fbits(r.uniform(0, 100)), fbits(100.0), rfl(ctx, 1000)])]


def g_e83b(ctx):
    r = begin(ctx, 0x100eba29)
    cam = camera(ctx)
    w32(ctx, cam + 0x44, r.choice([fbits(1.0), fbits(0.75), rfl(ctx, 3)]))
    box = ctx.buf(0x10, struct.pack('<4i', *[r.randrange(-300, 300) for _ in range(4)]))
    return [rect(ctx), box, ctx.u32()]


def g_e9af(ctx):
    r = begin(ctx, 0x1008ec46)
    cam = camera(ctx)
    w32(ctx, cam + 0x44, r.choice([fbits(1.0), fbits(0.75), fbits(-2.0), rfl(ctx, 3)]))
    rc = rect(ctx)
    cx = r.randrange(-100, 400)
    pt = ctx.buf(8, struct.pack('<2i', r.choice([cx, r.randrange(-1000, 1000)]), r.randrange(-1000, 1000)))
    return [rc, pt, newbuf(ctx, 0x10)]


# ---- effects ------------------------------------------------------------------------------------------
def node(ctx, scale=1000.0):
    n = newbuf(ctx, 0xa0)
    sane_floats(ctx, n + 0x70, 3, scale)
    return n


def particles(ctx, obj, scale=1000.0):
    for i in range(17):
        sane_floats(ctx, obj + 0x12 + i * 0x2a, 6, scale)


def effect(ctx):
    r = ctx.rng
    obj = newbuf(ctx, 0x580)
    particles(ctx, obj)
    w32(ctx, obj + 0x552, node(ctx))
    w32(ctx, obj + 0x566, ctx.u32())
    w32(ctx, obj + 0x56a, r.choice([0, 1, 2, 4, 8, -3, r.randrange(-100, 100), ctx.u32()]) & 0xffffffff)
    return obj


def g_effect(dfields, *extra):
    def g(ctx):
        r = begin(ctx, 0x100d2669, *extra, fill=True)
        ST['dfields'] = dfields
        ST['vis'] = [r.choice([0, 1, 1, 1, 5]) for _ in range(5)]
        w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
        obj = effect(ctx)
        if r.random() < 0.3:
            w32(ctx, U32(ctx, obj + 0x552) + 0x78, fbits(r.choice([10000.0, 9999.0, 10001.0, 20000.0, -5.0])))
        return [0 if r.random() < 0.04 else obj]
    return g


def U32(ctx, addr):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(addr, 4)))[0]


def g_2f9d(ctx):
    r = begin(ctx, 0x10090c7d, 0x100aa39f, 0x100a9d47, 0x100aa799)
    cam = camera(ctx)
    w32(ctx, cam + 0x40, r.choice([fbits(1000.0), fbits(5000.0), fbits(0.0), rfl(ctx, 3000)]))
    lim = r.randrange(0, 20)
    w32(ctx, 0x1017cc20, lim)
    w32(ctx, 0x1017cc24, r.choice([lim - 2, lim - 1, lim - 3, 0, 0xffffffff, r.randrange(0, 20)]))
    w32(ctx, 0x1015dc78, ctx.u32())
    e = newbuf(ctx, 0x580)
    nd = newbuf(ctx, 0xa0)
    w32(ctx, e + 0x552, nd)
    w32(ctx, nd + 0x8c, newbuf(ctx, 0x60))
    rets(0x10090c7d, e if r.random() < 0.9 else 0)
    cx, cy, cz = [bitsf(U32(ctx, cam + 4 * i)) for i in range(3)]
    pos = [f32(c + r.uniform(-800, 800)) for c in (cx, cy, cz)]
    args = [fbits(p) for p in pos] + [rfl(ctx, 300) for _ in range(3)]
    args.append(r.choice([0, 1, 2, 3, 5, 10, -1 & 0xffffffff]))
    return args


# ---- views ---------------------------------------------------------------------------------------------
def g_93a3(ctx):
    r = begin(ctx, 0x10098f8c, 0x100d3510)
    rets(0x10098f8c, r.choice([0, 60, 61, 100, -5 & 0xffffffff, ctx.u32()]))
    return [ctx.u32(), ctx.u32(), ctx.u32()]


def g_9c36(ctx):
    r = begin(ctx, 0x100999fc, 0x1009bba2, 0x10099c59)
    g = newbuf(ctx, 0x10)
    w16(ctx, g + 6, r.choice([0, 0, 1, 2, 0x100]))
    w32(ctx, 0x109e88e0, g)
    return []


def player(ctx):
    """The player's record in the pointer table (index [0x1017b8c8])."""
    r = ctx.rng
    i = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, i)
    me = newbuf(ctx, 0x200)
    b28 = newbuf(ctx, 0x20)
    w32(ctx, b28 + 0xc, rfl(ctx, 400))
    w32(ctx, me + 0x28, b28)
    w32(ctx, me + 0x68, rfl(ctx, 400))
    w32(ctx, me + 0x4c, r.choice([0, node(ctx, 3000)]))
    w16(ctx, me + 0x1c, r.choice([0, 0x16, 2, 0x100, r.getrandbits(16)]))
    w32(ctx, 0x109e8b40 + 4 * i, me)
    return me


def view_rec(ctx, pane=None):
    r = ctx.rng
    v = newbuf(ctx, 0xa0)
    w32(ctx, v, pane if pane is not None else newbuf(ctx, 0x20))
    w32(ctx, v + 8, r.randrange(0, 8))
    w32(ctx, v + 0x18, r.choice([0, 1, 100, 3000, -40 & 0xffffffff, ctx.u32()]))
    info = newbuf(ctx, 0x40)
    w32(ctx, info + 0x2c, r.randrange(0, 256))
    w32(ctx, v + 0x74, info)
    w32(ctx, v + 0x7c, r.choice([0, code(2, 2)]))
    return v


def views_world(ctx):
    r = ctx.rng
    w32(ctx, 0x1015da24, r.choice([0, 0, 1]))
    w32(ctx, 0x1017cc58, r.choice([0, 5, -3 & 0xffffffff, -100 & 0xffffffff]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x1017b920, r.choice([0, 1]))
    w32(ctx, 0x1015d1c8, r.choice([4, 4, 0, 2]))
    w32(ctx, 0x1017cc50, r.choice([0, 1, 2, 3]))
    w32(ctx, 0x1015e688, drv_table(ctx, 3, 0x20))
    w32(ctx, 0x10138c78, ctx.u32())
    camera(ctx)
    player(ctx)


VIEW_STUBS = (0x1008fb20, 0x100448ff, 0x100448cd, 0x1003b534, 0x1008fce3, 0x1009a137, 0x1008fd5a, 0x1009b03d)


def g_9c59(ctx):
    r = begin(ctx, 0x1009ace7, *VIEW_STUBS)
    views_world(ctx)
    mode = r.choice([0, 1, 2, 3, 4, 4, 4, 5, -1 & 0xffffffff])
    w32(ctx, 0x1015d1ac, mode)
    if mode < 8:
        w32(ctx, 0x1014c4c8 + 4 * mode, view_rec(ctx) if r.random() < 0.93 else 0)
    return []


def g_9ef3(ctx):
    r = begin(ctx, 0x1009ae96, 0x100eb070, *VIEW_STUBS)
    views_world(ctx)
    w32(ctx, 0x1015d1ac, r.choice([0, 1, 2, 4, 5]))
    pane = newbuf(ctx, 0x20)
    w32(ctx, 0x1014c4c8 + 16, view_rec(ctx, pane) if r.random() < 0.93 else 0)
    return [pane]


def e_needle(uc, k, raw):
    uw32(uc, raw[2], mix(ST['seed'], k, 1) % 700, mix(ST['seed'], k, 2) % 500)
    return None


def needle_view(ctx, pane):
    r = ctx.rng
    v = newbuf(ctx, 0xa0)
    w32(ctx, v, pane)
    info = newbuf(ctx, 0x40)
    w32(ctx, info + 0x2c, r.randrange(0, 256))
    w32(ctx, v + 0x74, info)
    w32(ctx, v + 0x88, r.choice([code(4, 3, eff=e_needle), code(4, 3, eff=e_needle), 0]))
    return v


def angle(ctx):
    r = ctx.rng
    return r.choice([fbits(r.uniform(-720, 720)), fbits(r.choice([0.0, 90.0, 360.0, -90.0, 450.0])), rfl(ctx, 1e5)])


def g_ace7(ctx):
    r = begin(ctx, 0x100ef727, 0x100e86a1)
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    wb(ctx, 0x101659e0, rb(ctx, 0x200))
    cam = camera(ctx)
    w32(ctx, cam + 0x18, r.choice([fbits(r.uniform(0, 90)), fbits(45.0), rfl(ctx, 100)]))
    return [needle_view(ctx, rect(ctx)), angle(ctx)]


def g_ae96(ctx):
    r = begin(ctx, 0x100ef727, 0x100e86a1)
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    wb(ctx, 0x101659e0, rb(ctx, 0x200))
    cam = camera(ctx)
    w32(ctx, cam + 0x18, r.choice([fbits(r.uniform(0, 90)), fbits(45.0), rfl(ctx, 100)]))
    pane = rect(ctx)
    return [pane, needle_view(ctx, newbuf(ctx, 0x20)), angle(ctx)]


def g_ba57(ctx):
    r = begin(ctx, 0x100766ca, 0x10099c59, 0x1008f630, 0x1009b8d8)
    mode = r.randrange(0, 6)
    w32(ctx, 0x1015d1ac, mode)
    v = newbuf(ctx, 0x40)
    m = r.choice([1, 2, 7, 100, 3000])
    d = newbuf(ctx, 0x10)
    w32(ctx, d, newbuf(ctx, 0x10))
    md = newbuf(ctx, 8)
    w32(ctx, md, m)
    w32(ctx, d + 4, md)
    w32(ctx, v + 0x14, d if r.random() < 0.95 else 0)
    w32(ctx, 0x1014c4c8 + 4 * mode, v if r.random() < 0.95 else 0)
    rets(0x100766ca, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1015da28, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1017cc50, r.choice([0, 1, 2, 2, 3]))
    now = r.randrange(0, 1 << 30)
    w32(ctx, 0x1015dc78, now)
    w32(ctx, 0x1017cc48, now + r.randrange(-50, 50))
    w32(ctx, 0x1017cc4c, now + r.randrange(-50, 50))
    w32(ctx, 0x1015dc80, r.randrange(0, m - 1) if m > 1 else 0xfffffffe)
    return []


def g_bba2(ctx):
    r = begin(ctx, 0x1009ba57, 0x10099c59, 0x10048287)
    mode = r.choice([0, 1, 2, 4, 5])
    w32(ctx, 0x1015d1ac, mode)
    v = newbuf(ctx, 0x80)
    t = newbuf(ctx, 0x10)
    w32(ctx, t + 8, r.choice([0xffffffff, 0, 3, ctx.u32()]))
    w32(ctx, v + 0x78, t)
    w32(ctx, v + 0x74, newbuf(ctx, 0x40))
    w32(ctx, v + 0x7c, r.choice([0, code(2, 2)]))
    w32(ctx, 0x1014c4c8 + 4 * mode, v if r.random() < 0.95 else 0)
    g = newbuf(ctx, 0x10)
    w16(ctx, g + 6, r.choice([0, 1, 1, 2, 3]))
    w32(ctx, 0x109e88e0, g)
    w32(ctx, 0x1017cca4, r.choice([0, 1, 5]))
    return []


# ---- markers ---------------------------------------------------------------------------------------------
MARK_STUBS = (0x100ba7d5, 0x10090099, 0x100bb2b0) + HUD_DRAW[:2]


def marker_obj(ctx, gaps=False):
    """A marker pane object: +0 pane, +0x70 table of label pointers (7 rows of 3; with gaps some are 0, which only
    Sim_1009a6f3 checks), +0x80 / +0x84 callbacks."""
    r = ctx.rng
    obj = newbuf(ctx, 0x100)
    w32(ctx, obj, newbuf(ctx, 0x20))
    tab = newbuf(ctx, 0x60)
    for i in range(0x18):
        w32(ctx, tab + 4 * i, r.choice([cstr(ctx, r.choice([b'', b'a', b'tgt', b'enemy01', b'friendly', b'longlabel12']))] * 4 + [0] * gaps))
    w32(ctx, obj + 0x70, tab)
    w32(ctx, obj + 0x80, r.choice([0, code(5, 3, 0, 1, 1)]))
    w32(ctx, obj + 0x84, r.choice([0, code(6, 3, eff=lambda uc, k, raw: uw32(uc, raw[1], 77 + k))]))
    return obj


def screen_answers(ctx):
    r = ctx.rng
    ST['vis'] = [r.choice([0, 1, 1, 1]) for _ in range(4)]
    ST['scr'] = [(fbits(r.choice([r.uniform(-50, 700), r.randrange(0, 640) + 0.0, -0.5, 1e10])),
                  fbits(r.uniform(-50, 500))) for _ in range(4)]


def g_a282(ctx):
    r = begin(ctx, *MARK_STUBS)
    hud_world(ctx)
    screen_answers(ctx)
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    w32(ctx, 0x1017cc5c, ctx.u32())
    for b in range(1, 5):
        wb(ctx, 0x1014be6d + b * 0x1d, struct.pack('<I', r.choice([0, 5, 100, 1000, 70000, ctx.u32()])))
    obj = marker_obj(ctx)
    name = cstr(ctx, r.choice([b'mark', b'Tgt', b'x']))
    return [obj, rfl(ctx), rfl(ctx), rfl(ctx), name, r.choice([0, 1, 1, 2]), r.choice([-1, 0, 0, 1, 2, 4, 5, 9]) & 0xffffffff,
            r.choice([fbits(r.uniform(0, 1e5)), fbits(r.uniform(0, 1e8)), rfl_nan(ctx, 1e6)])]


def unit_table(ctx, n):
    r = ctx.rng
    for i in range(n):
        o = newbuf(ctx, 0x100)
        w16(ctx, o + 0x1c, r.choice([0x400, 0x1000, 0x1400, 0x400 | 0x800, 0x1000 | 0x4000, 0x400 | 2, 0, r.getrandbits(16)]))
        sane_floats(ctx, o + 0x58, 3, 3000)
        b = newbuf(ctx, 0xc00)
        w32(ctx, o + 0x28, b)
        w32(ctx, 0x109e8b40 + 4 * i, o)
    w32(ctx, 0x1015dc64, n)


def g_a3b3(ctx):
    r = begin(ctx, 0x1009a282, 0x100b53f0, 0x10052be7, 0x10052c02, 0x100cbc6b)
    n = r.choice([0, 1, 3, 6])
    unit_table(ctx, n)
    me = r.randrange(0, max(n, 1))
    w32(ctx, 0x1017b8c8, me)
    if n == 0:
        o = newbuf(ctx, 0x100)
        w32(ctx, o + 0x28, newbuf(ctx, 0xc00))
        w32(ctx, 0x109e8b40, o)
    rets(0x10052be7, *[r.choice([0, 1, 1, 2, 3]) for _ in range(4)])
    rets(0x10052c02, *[r.choice([0, 1, 2, 3]) for _ in range(4)])
    ST['cbc'] = [rfl(ctx, 1e4) for _ in range(5)]
    k = r.choice([0, 1, 4])
    for i in range(k):
        e = 0x109e9ba0 + i * 0x50
        w16(ctx, e, r.choice([0x400, 0x1000, 0x400 | 2, 0, 0x1400]))
        w32(ctx, e + 8, ctx.u32())
    w32(ctx, 0x1015dc68, k)
    obj = marker_obj(ctx)
    return [obj]


def g_a6f3(ctx):
    r = begin(ctx, 0x10052be7, 0x10052c02, *MARK_STUBS)
    hud_world(ctx)
    screen_answers(ctx)
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]), newbuf(ctx, 0x20))
    rets(0x10052be7, r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    rets(0x10052c02, r.choice([0, 1, 2]), r.choice([0, 1, 2]))
    unit_table(ctx, 4)
    w32(ctx, 0x1017b8c8, r.randrange(0, 4))
    me = newbuf(ctx, 0x200)
    w32(ctx, 0x109e8b40 + 4 * U32(ctx, 0x1017b8c8), me)
    for i in range(4):
        e = 0x109e9ba0 + i * 0x50
        w16(ctx, e, r.choice([0x400, 0, 0x1000]))
    for i in range(4):
        e = 0x109a8640 + i * 0x54
        w16(ctx, e + 0x24, r.choice([0, 0x20, 0x21]))
        w16(ctx, e + 0x26, r.getrandbits(16))

    def target():
        return r.choice([0, 0x1000 | 0x200, 0x200 | r.randrange(4), 0x400 | r.randrange(4), 0x100, 0x100,
                         0x300, 0x800 | 1, 0x200 | r.randrange(4) | 0x2000])
    w32(ctx, me + 0xf0, target())
    w32(ctx, me + 0x118, r.choice([target(), r.randrange(0, 4)]))
    w32(ctx, 0x1013bb40, r.randrange(0, 40))
    w32(ctx, 0x1017cca0, r.choice([0, 0, newbuf(ctx, 0x20)]))
    return [marker_obj(ctx, True)]


def g_aac9(ctx):
    r = begin(ctx, *MARK_STUBS)
    hud_world(ctx)
    screen_answers(ctx)
    rets(0x100ba7d5, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]), newbuf(ctx, 0x20))
    side = r.randrange(0, 4)
    w32(ctx, 0x1013bb40, side)
    n = r.choice([0, 1, 3, 6])
    w32(ctx, 0x1017e5a8, n)
    for i in range(n):
        e = 0x109a8640 + i * 0x54
        wb(ctx, e, rb(ctx, 0x54))
        w32(ctx, e + 0xc, r.choice([side, side, side + 1]))
        w32(ctx, e, r.choice([1, 1, 0, ctx.u32()]))
        w16(ctx, e + 0x24, r.choice([0, 0x20, 0x21, 0x22]))
        w32(ctx, e + 4, r.choice([0, node(ctx, 3000)]))
        sane_floats(ctx, e + 0x18, 3, 3000)
    w32(ctx, 0x1017cca0, r.choice([0, 0, newbuf(ctx, 0x20)]))
    return [marker_obj(ctx)]


# ---- random tables, tree copy -------------------------------------------------------------------------------
def g_c860(ctx):
    begin(ctx)
    return [ctx.u32()]


def tree(ctx, depth):
    r = ctx.rng
    n = newbuf(ctx, 0x40)
    for off in (4, 8):
        w32(ctx, n + off, tree(ctx, depth - 1) if depth > 0 and r.random() < 0.6 else 0)
    return n


def a_heap(n, args):
    k = n
    if k in ST.get('alloc_fail', ()):
        return 0
    return ST['heap'][k] if k < len(ST['heap']) else 0


def g_db70(ctx):
    r = begin(ctx, 0x100a9ccf)
    w32(ctx, 0x1017b8f8, ctx.u32())
    ST['heap'] = [newbuf(ctx, 0x28) for _ in range(40)]
    ST['alloc_fail'] = (r.randrange(0, 12),) if r.random() < 0.2 else ()
    return [tree(ctx, r.choice([0, 1, 2, 3, 4])) if r.random() < 0.95 else 0]


# ---- scene pass --------------------------------------------------------------------------------------------
def scene_obj(ctx, depth):
    r = ctx.rng
    o = newbuf(ctx, 0x100)
    w16(ctx, o, r.choice([0, 0, 0x1000, r.getrandbits(16)]))
    if depth > 0 and r.random() < 0.7:
        nd = newbuf(ctx, 0xa0)
        kids = []
        for _ in range(r.choice([0, 1, 2, 3])):
            c = newbuf(ctx, 0xa0)
            w32(ctx, c + 0x8c, scene_obj(ctx, depth - 1) if r.random() < 0.9 else 0)
            kids.append(c)
        w32(ctx, nd + 4, kids[0] if kids else 0)
        for a, b in zip(kids, kids[1:] + [0]):
            w32(ctx, a + 8, b)
        w32(ctx, o + 0x28, nd)
    else:
        w32(ctx, o + 0x28, 0)
    return o


def scene_world(ctx):
    r = ctx.rng
    ST['code_rets'] = {}
    w32(ctx, 0x1014cb40, code(7, 1, *[r.choice([0, 0, 1]) for _ in range(5)]))
    w32(ctx, 0x101c7aa0, rfl(ctx, 5000))
    w32(ctx, 0x101c9ea4, r.choice([1, 1, 1, 0]))
    w32(ctx, 0x1017d13c, r.randrange(0, 10))
    w32(ctx, 0x1017d140, r.randrange(0, 12))
    w32(ctx, 0x101cc2b4, ctx.u32())
    w32(ctx, 0x101c7a9c, ctx.u32())


def g_4845(ctx):
    r = begin(ctx, 0x100a4510)
    scene_world(ctx)
    return [scene_obj(ctx, r.choice([0, 1, 2, 3])) if r.random() < 0.95 else 0]


def g_4933(ctx):
    r = begin(ctx, 0x100a4510)
    scene_world(ctx)
    o = scene_obj(ctx, r.choice([0, 1, 2, 3]))
    me = r.randrange(0, 4)
    w32(ctx, 0x1017b8c8, me)
    w16(ctx, o + 2, r.choice([0, 0x100, 0x100, r.getrandbits(16)]))
    w16(ctx, o + 0x24, r.choice([me, me, me + 1]))
    w32(ctx, 0x1015ef0c, r.choice([0, 1, 1]))
    w32(ctx, 0x1015d1ac, r.choice([0, 4, 1, 2]))
    camera(ctx)
    w32(ctx, 0x10185c50, rfl(ctx, 10))
    p = newbuf(ctx, 0x40)
    b = newbuf(ctx, 0x500)
    w32(ctx, p + 0x28, b)
    own = newbuf(ctx, 0xa0)
    w32(ctx, own + 0x8c, scene_obj(ctx, 1))
    w32(ctx, b + 0x490, r.choice([0, own, own]))
    w32(ctx, 0x109e8b40 + 4 * me, p)
    if r.random() < 0.3:
        nd = newbuf(ctx, 0x10)
        w32(ctx, nd + 4, 0)
        w32(ctx, o + 0x28, r.choice([0, nd]))
    return [o if r.random() < 0.97 else 0]


def e_4933(uc, k, raw):
    """Sim_100a4933 adds a few draw list entries ([0x1017d138]) and sometimes says stop."""
    v = ur32(uc, 0x1017d138)
    uw32(uc, 0x1017d138, v + ST['adds'][k % len(ST['adds'])])
    return None


EFFECT[0x100a4933] = e_4933


def g_4d8e(ctx):
    r = begin(ctx, 0x100a88db, 0x100a4933, 0x10087d83, 0x100a4700, 0x100a539e)
    ST['adds'] = [r.choice([0, 0, 1, 2]) for _ in range(5)]
    rets(0x100a4933, *[r.choice([0, 0, 0, 1]) for _ in range(5)])
    w32(ctx, 0x1017d134, r.choice([0, 0, 1, 3]))
    w32(ctx, 0x1017d168, newbuf(ctx, 0x200))
    w32(ctx, 0x1017b940, r.choice([0, 0, 1]))
    camera(ctx)
    n = r.choice([0, 1, 2, 4])
    for i in range(n):
        lst = newbuf(ctx, 0x40)
        a = ctx.u32() | 1
        w32(ctx, lst + 4, a)
        w32(ctx, lst + 8, r.choice([a ^ 2, a ^ 0x100, a ^ 6]))       # non-zero and not +4 (else the original retries for ever)
        objs = [newbuf(ctx, 0x40) for _ in range(r.choice([0, 1, 2, 4]))]
        w32(ctx, lst + 0x20, objs[0] if objs else 0)
        for x, y in zip(objs, objs[1:] + [0]):
            w32(ctx, x + 0x20, y)
        w32(ctx, 0x1036f2a0 + 4 * i, lst)
    w32(ctx, 0x1036f2a0 + 4 * n, 0)
    return []


def spec(name, addr, gen, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


API = {'HeapAlloc': a_heap}

SPECS = [
    spec('Sim_10089e91', 0x10089e91, g_9e91, 800, void=True),
    spec('Sim_100908b3', 0x100908b3, g_08b3, 300, void=True),
    spec('Sim_1008b6a0', 0x1008b6a0, g_b6a0, 400),
    spec('Sim_1008b78f', 0x1008b78f, g_b78f, 400, void=True),
    spec('Sim_1008d65d', 0x1008d65d, g_d65d, 400),
    spec('Sim_1008d6f7', 0x1008d6f7, g_d6f7, 600),
    spec('Sim_1008d8c0', 0x1008d8c0, g_d6f7, 600),
    spec('Sim_1008de0a', 0x1008de0a, g_de0a, 600),
    spec('Sim_1008df64', 0x1008df64, g_df64, 600),
    spec('Sim_1008e0fe', 0x1008e0fe, g_e0fe, 400, void=True),
    spec('Sim_1008e320', 0x1008e320, g_e320, 1500),
    spec('Sim_1008e707', 0x1008e707, g_e707, 600),
    spec('Sim_1008e7a1', 0x1008e7a1, g_e707, 600),
    spec('Sim_1008e83b', 0x1008e83b, g_e83b, 600),
    spec('Sim_1008e9af', 0x1008e9af, g_e9af, 1000),
    spec('Sim_10091223', 0x10091223, g_effect((0, 1, 2, 3, 4, 7, 8)), 400),
    spec('Sim_10092bbb', 0x10092bbb, g_effect((0, 1, 2, 3, 4, 7, 8), 0x100e84f0), 500),
    spec('Sim_10093e5b', 0x10093e5b, g_effect((0, 1, 3, 4, 7, 8), 0x100e84f0), 500),
    spec('Sim_10096163', 0x10096163, g_effect((0, 1, 3, 4, 7, 8)), 500),
    spec('Sim_10096869', 0x10096869, g_effect((0, 1, 3, 4, 7, 8), 0x100e84f0), 500),
    spec('Sim_10092f9d', 0x10092f9d, g_2f9d, 800, void=True),
    spec('Sim_100993a3', 0x100993a3, g_93a3, 300),
    spec('Sim_10099c36', 0x10099c36, g_9c36, 300, void=True),
    spec('Sim_10099c59', 0x10099c59, g_9c59, 800, void=True),
    spec('Sim_10099ef3', 0x10099ef3, g_9ef3, 600, void=True),
    spec('Sim_1009ace7', 0x1009ace7, g_ace7, 800, void=True),
    spec('Sim_1009ae96', 0x1009ae96, g_ae96, 800, void=True),
    spec('Sim_1009ba57', 0x1009ba57, g_ba57, 800, void=True),
    spec('Sim_1009bba2', 0x1009bba2, g_bba2, 600, void=True),
    spec('Sim_1009a282', 0x1009a282, g_a282, 1000, void=True),
    spec('Sim_1009a3b3', 0x1009a3b3, g_a3b3, 600, void=True),
    spec('Sim_1009a6f3', 0x1009a6f3, g_a6f3, 1000, void=True),
    spec('Sim_1009aac9', 0x1009aac9, g_aac9, 600, void=True),
    spec('Sim_1009c860', 0x1009c860, g_c860, 40, void=True),
    spec('Sim_1009db70', 0x1009db70, g_db70, 500, api=API),
    spec('Sim_100a4845', 0x100a4845, g_4845, 600),
    spec('Sim_100a4933', 0x100a4933, g_4933, 800),
    spec('Sim_100a4d8e', 0x100a4d8e, g_4d8e, 500, void=True),
]

# round 32: the pool layout follows W31_TexMode() (off here: the 1.0 layout)
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', 'sim/fx/w32_partspec.c', '../tools/verify/plat_stub.c',
                                 'sim/render/w3q_texture.c', 'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_100f/w2e_string.c',
                                 'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_1004/state.c'] if x not in SOURCES]

# round 32: the 1.0 effect spawns call 1.2's in 1.2 mode (off here): stand-ins, and the mode switch
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', '../tools/verify/w32_smoke_stub.c'] if x not in SOURCES]
