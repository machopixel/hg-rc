# Differential tests for round 9, letter e (see difftest.py and docs/checker.md).
#
# Callees are replaced in the emulator by a code hook at their first byte (the code bytes are not patched): the
# hook appends ('stub_<address>', arguments, extra data) to the checker's call log, which must be identical for
# the original and the C run (stack addresses are logged as 'stack'), writes the callee's out data where it has
# some, answers with a value that depends only on the trial and the call position and returns to the caller.
# Converted callees are stubbed at both their original address and their C symbol, so each function is checked
# on its own logic. Run for real on both sides: Mem_Move, Str_* and the pure bit-field helpers of
# src/sim/unsorted_1004 (fields.c, w2k_flags.c, w2h_misc.c, w2l_values.c).
# Every function here is integer code and must match bit for bit (no float tolerance).
# The callees of the four big functions (cache release, frame driver, HUD message block, input map reader) that
# are stubbed on both sides get test-only bodies from tools/verify/w9e_shim.c (never executed: the hook sits on
# the symbol), because their real files would pull most of the DLL into the test image. PtrArray_Insert is
# stubbed with a Python copy of its sorted insertion (comparator 0x100b9748: the 64-bit stamp at +0x10), so the
# freeing order of the cache release is exercised. Input_ReadLine and sprintf are stubbed with the trial's
# script of lines and a %s-only formatter; the text routines log the text and the colour byte 0x109b992e.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX

SOURCES = [
    # this letter
    'sim/unsorted_1001/w9e_misc.c', 'sim/unsorted_1007/w9e_misc.c', 'sim/unsorted_100d/w9e_misc.c',
    'sim/unsorted_1009/w9e_misc.c', 'sim/unsorted_1005/w9e_hud.c', 'sim/unsorted_1005/w9e_gear.c',
    'sim/unsorted_100b/w9e_links.c', 'sim/unsorted_100b/w9e_cache.c',
    'sim/unsorted_1003/w9e_frame.c', 'sim/unsorted_100c/w9e_hud.c', 'sim/unsorted_1005/w9e_input.c',
    # callees of the four above that run for real (Crt_StrUpper, Array_Clear, Crt_StrStr, sscanf, Str_Cat) or
    # are stubbed on both sides and link on their own
    'sim/unsorted_100b/w2f_tables.c', 'sim/unsorted_1010/w7s_locale.c', 'sim/unsorted_1010/w2h_misc.c',
    'sim/unsorted_1010/w6k_misc.c', 'sim/unsorted_1010/w2i_crt.c', 'sim/unsorted_1010/w2f_mantissa.c',
    'sim/unsorted_1010/w2f_ld12.c', 'sim/unsorted_100f/w6i_mem.c', 'sim/unsorted_100f/w2h_mem.c',
    'sim/unsorted_1000/w2i_misc.c',
    'sim/unsorted_100d/state.c', 'sim/unsorted_100d/w5c_calls.c',
    'sim/unsorted_1005/w5y_misc.c', 'sim/unsorted_100f/w6h_crt.c', 'sim/unsorted_100f/w2k_string.c',
    'sim/unsorted_100f/w2j_str.c',
    # test-only bodies for the callees stubbed on both sides whose real files would pull in most of the DLL
    '../tools/verify/w9e_shim.c',
    # existing converted code the functions above call (stubbed or run for real), and what it needs to link
    'sim/unsorted_1004/tables.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_1004/w2k_flags.c',
    'sim/unsorted_1004/w2h_misc.c', 'sim/unsorted_1004/w2l_values.c', 'sim/unsorted_100a/w3o_misc.c',
    'sim/unsorted_100b/w4v_wrappers.c', 'sim/unsorted_100a/w5x_calls.c', 'sim/unsorted_100a/w2g_misc.c',
    'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_100a/w5y_misc.c',
    'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100a/w7o_nodes.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/view/w3r_matrix.c', 'sim/unsorted_100f/w2g_crt.c', 'sim/unsorted_1010/w2g_crt.c', 'sim/util/field.c',
    'sim/unsorted_1000/w3s_model.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w3o_ray.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1006/w6e_misc.c', 'sim/view/w2l_lists.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/unsorted_100a/w5c_calls.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w6g_nodes.c', 'sim/unsorted_100a/w2k_list.c',
    'sim/unsorted_100c/w4v_timers.c', 'sim/unsorted_100c/w3o_misc.c', 'sim/win/w4v_heap.c', 'sim/view/objlist.c',
    'sim/view/matrix.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/view/viewrect.c', 'sim/unsorted_1000/w2f_records.c',
    'sim/unsorted_1000/w2k_misc.c', 'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w5x_calls.c',
    'sim/unsorted_1002/w5z_calls.c', 'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w2e_tables.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1004/w5y_misc.c',
    'sim/unsorted_1004/w2e_strings.c', 'sim/unsorted_1004/w3o_misc.c', 'sim/unsorted_1006/w5a_math.c',
    'sim/unsorted_1006/w5b_calls.c', 'sim/unsorted_1006/w5c_math.c', 'sim/unsorted_1008/w3o_length.c',
    'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1008/w3s_grid.c', 'sim/util/ring.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_100a/w5a_misc.c', 'sim/unsorted_100a/w2i_misc.c', 'sim/render/w2n_polygon.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/win/w4a_heap.c', 'sim/render/w2n_project.c',
    'sim/unsorted_1000/w2e_names.c', 'sim/unsorted_1002/w3o_vectors.c', 'sim/unsorted_1001/w2e_names.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
DATA = 0x00A84000           # fixed buffers the stubs hand out (matrices)

ST = {'seed': 0}


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<H', v & 0xffff))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def rb(ctx, n):
    return ctx.rng.getrandbits(8 * n).to_bytes(n, 'little') if n else b''


def newbuf(ctx, n):
    return ctx.buf(n, rb(ctx, n))


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


def cstring(uc, addr, maxn=0x60):
    out = b''
    while len(out) < maxn:
        c = bytes(uc.mem_read(addr + len(out), 1))
        if c == b'\0':
            break
        out += c
    return list(out)


# ---- stubs --------------------------------------------------------------------------------------------------
class Stub:
    def __init__(self, nargs, pol, csym=None):
        self.nargs, self.pol, self.csym = nargs, pol, csym


def pv(tag, options):
    """Answer from options by (trial, call number); a list in ST['v<tag>'] overrides."""
    return lambda uc, n, a: (pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or options), [])


def p_log(tag, ptr_i, nwords, ret=(0,)):
    """Logs nwords dwords read through argument ptr_i."""
    def pol(uc, n, a):
        extra = list(struct.unpack('<%dI' % nwords, bytes(uc.mem_read(a[ptr_i], 4 * nwords))))
        return pick(mix(ST['seed'], tag, n), ST.get('v%x' % tag) or list(ret)), extra
    return pol


def p_outs(tag, outs, ret=(0,)):
    """Writes a hash dword through each argument pointer in outs."""
    def pol(uc, n, a):
        for k, i in enumerate(outs):
            uc.mem_write(a[i], struct.pack('<I', mix(ST['seed'], tag, n, k)))
        return pick(mix(ST['seed'], tag, n, 98), ST.get('v%x' % tag) or list(ret)), []
    return pol


def p_err(uc, n, a):
    """SystemError(code, format, ...): the argument count follows the format (%d/%s words, then the 0 the game
    always appends); %s arguments are logged by their text."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    fmt = bytes(cstring(uc, a[1], 0x100))
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
    extra = list(fmt)
    for k, s in enumerate(specs):
        v = r32(uc, esp + 12 + 4 * k)
        extra += cstring(uc, v) + [-1] if s == 's' else [logged(v)]
    extra.append(logged(r32(uc, esp + 12 + 4 * len(specs))))
    return 0, extra


def p_clip(uc, n, a):
    """Sim_100a2c08(mode, value, cur, prev, out): logs both vertices, writes six dwords to out (x, y in the
    clipping range so the next planes have work)."""
    extra = list(struct.unpack('<6I', bytes(uc.mem_read(a[2], 24)))) + \
        list(struct.unpack('<6I', bytes(uc.mem_read(a[3], 24))))
    h = mix(ST['seed'], 0x2c08, n)
    vals = [(mix(h, k) % 0x6001) - 0x3000 for k in range(2)] + [mix(h, k) for k in range(2, 6)]
    if h & 0x10:
        vals[0] = pick(h >> 8, [0x2000, -0x2000, 0x2001, -0x2001])
    if h & 0x20:
        vals[1] = pick(h >> 12, [0x2000, -0x2000, 0x2001, -0x2001])
    uc.mem_write(a[4], struct.pack('<6I', *[v & 0xffffffff for v in vals]))
    return 0, extra


STUBS = {
    0x100bc280: Stub(2, p_err),
    # lines (hooks_hud.c patches 0x100ef727)
    0x100ef727: Stub(7, pv(0xf727, [0])),
    0x100e86a1: Stub(7, pv(0x86a1, [0])),
    # tables
    0x10052be7: Stub(1, pv(0x2be7, [0, 1, 2, 3, 0xffffffff, 5]), 'Sim_10052be7'),
    0x10052c02: Stub(1, pv(0x2c02, [0, 1, 2, 3, 0xffffffff, 5]), 'Sim_10052c02'),
    # clipping
    0x100a2c08: Stub(5, p_clip, 'Sim_100a2c08'),
    # links
    0x100b42aa: Stub(1, pv(0x42aa, [0]), 'Sim_100b42aa'),
    0x100aa863: Stub(1, pv(0xa863, [DATA, DATA + 0x40]), 'Sim_100aa863'),
    0x100aa807: Stub(1, pv(0xa807, [DATA + 0x80, DATA + 0xc0]), 'Sim_100aa807'),
    0x100a9ccf: Stub(4, p_outs(0x9ccf, (1, 2, 3)), 'Sim_100a9ccf'),
    0x10009787: Stub(1, pv(0x9787, [0]), 'Sim_10009787'),
    0x100b48de: Stub(2, pv(0x48de, [0, 1, 1])),
    0x100aa87a: Stub(2, p_log(0xa87a, 1, 12), 'Sim_100aa87a'),
    0x100aa812: Stub(2, p_log(0xa812, 1, 12), 'Sim_100aa812'),
    0x100aa0da: Stub(5, pv(0xa0da, [0]), 'Sim_100aa0da'),
    0x100b42d5: Stub(1, pv(0x42d5, [0]), 'Sim_100b42d5'),
    0x100aa64a: Stub(1, pv(0xa64a, [0]), 'Sim_100aa64a'),
}


def install(m):
    if getattr(m, '_w9e', False):
        return
    m._w9e = True

    def make(addr, st):
        name = 'stub_%x' % addr

        def cb(uc, address, size, ud):
            esp = uc.reg_read(UC_X86_REG_ESP)
            raw = list(struct.unpack('<%dI' % st.nargs, bytes(uc.mem_read(esp + 4, 4 * st.nargs)))) if st.nargs else []
            n = sum(1 for e in m.api_log if e[0] == name)
            val, extra = st.pol(uc, n, raw)
            m.api_log.append((name, tuple(logged(v) for v in raw), tuple(extra)))
            uc.reg_write(UC_X86_REG_EAX, val & 0xffffffff)
            uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
            uc.reg_write(UC_X86_REG_ESP, esp + 4)
        return cb

    for addr, st in STUBS.items():
        m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, addr, addr)
        if st.csym:
            c = m.syms[st.csym]
            m.raw.hook_add(UC_HOOK_CODE, make(addr, st), None, c, c)


def begin(ctx):
    install(ctx.m)
    ST.clear()
    ST['seed'] = ctx.rng.getrandbits(32)
    return ctx.rng


# ---- generators ---------------------------------------------------------------------------------------------
def g_dd66(ctx):
    r = begin(ctx)
    idx = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, idx)
    for i in range(8):
        w32(ctx, 0x109e8b40 + 4 * i, r.choice([0, newbuf(ctx, 0x60), newbuf(ctx, 0x60)]))
    return []


def g_5753(ctx):
    r = begin(ctx)
    code = r.choice([0, 9, 0xd, 0xd, 0xd, 0x1b, 0xc6, 0xc7, 0x209, 0xe, 0xc8, 0x20a, 8, 0xa, r.randrange(0, 0x300),
                     r.getrandbits(32)])
    sub = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 0xffffffff, r.getrandbits(32)])
    return [code, sub, newbuf(ctx, 4), newbuf(ctx, 4)]


def g_dc2d7(ctx):
    r = begin(ctx)
    w32(ctx, 0x1015dc68, r.choice([0, 1, 5, 0xfc, 0xfd, 0xfe, 0xff, 0x100, 0x7fffffff, 0x80000000, 0xffffffff]))
    return []


def g_9b516(ctx):
    r = begin(ctx)
    idx = r.randrange(0, 4)
    w32(ctx, 0x1015d1ac, idx)
    world = newbuf(ctx, 0x80)
    table = newbuf(ctx, 0x40)
    w32(ctx, world + 0x74, table)
    w32(ctx, 0x1014c4c8 + 4 * idx, r.choice([world] * 9 + [0]))
    w32(ctx, 0x1014cb28, r.choice([0, 1]))
    obj = newbuf(ctx, 0x40)
    w16(ctx, obj, r.choice([0x8000, 0x9000, 0x1000, 0x7000, 0, 0xa000, r.getrandbits(16)]) | r.getrandbits(12))
    rec = newbuf(ctx, 0x40)
    cls = r.choice([0x100, 0x200, 0x400, 0x800, 0, 0x300, 0xf00, 0x100, 0x200])
    sub = r.choice([0x10, 0x20, 0x40, 0x50, 0x60, 0x80, 0, 0x30, 0x70, 0x90, 0xf0, 0xa0])
    w16(ctx, rec + 2, cls | sub | r.choice([0, 0xf, 0x1000, 0xf00f]))
    w16(ctx, rec + 0x24, r.randrange(0, 0x20) | r.choice([0, 0, 0x8000, 0x7f00]))
    w32(ctx, obj + 0x24, rec)
    return [r.choice([obj] * 9 + [0]), r.choice([0, 1, 1, 1, 7]), ctx.u32()]


def g_1c90(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    wb(ctx, 0x101659c0, rb(ctx, 0x100))
    width = r.choice([-5, -1, 0, 1, 2, 5, 20, 100])
    height = r.choice([-7, -3, -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 13, 16, 40])
    colour = r.choice([0, 1, 2, 3, 7, 11, 0x1f, 0x20, 0x3e, r.randrange(0, 0x60), -1])
    return [ctx.u32(), r.randrange(-50, 700), r.randrange(-50, 500), width, height, colour & 0xffffffff]


def g_a1db(ctx):
    r = begin(ctx)
    gear = newbuf(ctx, 0x100)
    count = r.choice([0, 1, 1, 2, 3, 4, 6])
    items = newbuf(ctx, 0x6c * 6 + 0x10)
    w32(ctx, gear + 0xb0, count)
    w32(ctx, gear + 0xc8, items)
    for t in range(6):
        d = 0x10111688 + 0xaa * t
        w32(ctx, d, r.choice([0, 1, 5, 100, 0x7fffffff, 0xffffffff, r.randrange(0, 1000)]))
        w32(ctx, d + 4, r.choice([0, 1, 2, 3, 4, 5, 7, 10, 11, 0xffffffff, 0x7fffffff, 6]))
    for i in range(0x60):
        w32(ctx, 0x10124ea8 + 0x98 * i, r.choice([0, 1, 7, 50, 200, 0x7fffffff, 0xffffffff, r.randrange(0, 500)]))
    for a in (0x10126cf0, 0x101262d8, 0x10126370, 0x10126408, 0x101264a0, 0x10126538, 0x101265d0, 0x10126668,
              0x10126700, 0x10126798, 0x10126830, 0x101268c8):
        w32(ctx, a, r.choice([0, 1, 9, 60, 250, 0x7fffffff, 0xffffffff, r.randrange(0, 500)]))
    for i in range(count):
        it = items + 0x6c * i
        t = r.randrange(0, 6)
        w32(ctx, it + 4, t)
        kind = r32(ctx.m.uc, 0x10111688 + 0xaa * t + 4)
        if kind == 1:
            word = (r.choice([0, 1, 5, 0x30, 0x5f, 0xffff, 0xfffe]) << 16) | r.getrandbits(16)
        elif kind == 2:
            word = r.getrandbits(12) | r.choice([0, 0, 0xfff000, r.getrandbits(32) & 0xfff000])
        elif kind >= 5 and kind <= 10:
            word = r.choice([r.randrange(0, 0x60), r.getrandbits(16)]) | (r.getrandbits(16) << 16)
        else:
            word = r.getrandbits(32)
        w32(ctx, it + 0xc, word)
    return [gear]


def g_43ce(ctx):
    r = begin(ctx)
    wb(ctx, 0x101a2100, rb(ctx, 0x88 * 8))
    wb(ctx, 0x109e9ba0, rb(ctx, 0x50 * 8))
    wb(ctx, DATA, rb(ctx, 0x100))
    index = r.randrange(0, 8)
    recs = [0x101a2100 + 0x88 * i for i in range(8)]
    for rec in recs:
        w32(ctx, rec + 0x24, r.choice([0xffffffff] + list(range(8))))
        w32(ctx, rec + 0x20, r.choice([0xffffffff, 0xffffffff] + list(range(8))))
        w32(ctx, rec + 0x28, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
        w32(ctx, rec + 0x2c, r.choice([0, newbuf(ctx, 0x20), newbuf(ctx, 0x20)]))
    rec = recs[index]
    nxt = r32(ctx.m.uc, rec + 0x20)
    if nxt != 0xffffffff and (r32(ctx.m.uc, rec + 0x28) == 0 or r32(ctx.m.uc, rec + 0x2c) == 0):
        # the original would pass an uninitialised position to Sim_100aa0da here: keep that path out
        w32(ctx, recs[nxt] + 0x2c, 0)
    return [index]


# ---- 0x100b9203: cache release ----------------------------------------------------------------------------
EXTS = [b'WTB', b'XEL', b'WAV', b'3DI', b'MLT', b'TLT', b'GDF', b'BWD', b'SDF', b'ZZZ', b'wtb', b'gdf', b'']


def text(ctx, n, alpha=b'ABCXYZ019_'):
    return bytes(ctx.rng.choice(alpha) for _ in range(n))


def cstr(ctx, data, extra=8):
    return ctx.buf(len(data) + 1 + extra, bytes(data) + b'\0' + rb(ctx, extra))


def u64(uc, addr):
    return struct.unpack('<Q', bytes(uc.mem_read(addr, 8)))[0]


def p_insert(uc, n, a):
    """PtrArray_Insert(list, entry, 0x100b9748) as the original does it: the entry goes before the first one whose
    64-bit stamp (+0x10) is greater, 0 when the list is full (or, now and then, as if it were)."""
    lst, item = a[0], a[1]
    cap, cnt, arr = r32(uc, lst), r32(uc, lst + 4), r32(uc, lst + 8)
    if item == 0 or cnt >= cap or pick(mix(ST['seed'], 0xd76b, n), [1, 1, 1, 1, 1, 1, 0]) == 0:
        return 0, []
    items = [r32(uc, arr + 4 * i) for i in range(cnt)]
    i = 0
    while i < cnt and u64(uc, items[i] + 0x10) > u64(uc, item + 0x10):
        i += 1
    items.insert(i, item)
    uc.mem_write(arr, struct.pack('<%dI' % len(items), *items))
    uc.mem_write(lst + 4, struct.pack('<I', cnt + 1))
    return 1, []


def g_9203(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017e950, 0)         # "C" locale: strupr is the plain loop
    for a in (0x1017b904, 0x1017b908, 0x1017b90c, 0x1017b8fc, 0x1017b910):
        w32(ctx, a, r.choice([0, 0, 0x1234, ctx.u32()]))
    w32(ctx, 0x1017b914, ctx.u32())
    w32(ctx, 0x1017d604, newbuf(ctx, 0x10))
    nb = r.choice([1, 2, 3, 5, 8])
    table = newbuf(ctx, 0x30)
    buckets = ctx.buf(4 * nb, 'zero')
    w32(ctx, table, nb)
    w32(ctx, table + 0x10, buckets)
    w32(ctx, table + 0x20, r.choice([0, 0x10001234, ctx.u32()]))
    ST['table'] = table
    lst = newbuf(ctx, 0xc)
    w32(ctx, lst, 0x400, 0, ctx.buf(0x400 * 4, 'zero'))
    ST['list'] = lst
    keys = [text(ctx, 8) for _ in range(r.choice([1, 2, 3]))] + [text(ctx, 8, b'abcxyz01'), text(ctx, 4) + b'\0' + text(ctx, 3)]
    chains = {}
    total = 0
    for _ in range(r.choice([0, 1, 2, 4, 8, 12])):
        e = newbuf(ctx, 0x24)
        k = r.choice(keys)
        wb(ctx, e, k)
        w32(ctx, e + 8, r.choice([0, 0, 0, 0xffffffff, 1, 2]))
        size = r.choice([0, 1, 10, 50, 100, r.randrange(0, 1000)])
        w32(ctx, e + 0xc, size)
        w32(ctx, e + 0x10, r.choice([r.randrange(0, 100), ctx.u32()]), r.choice([0, 0, 1, ctx.u32()]))
        total += size
        d0, d1 = struct.unpack('<II', k)
        chains.setdefault((d0 ^ d1) % nb, []).append(e)
    for b, es in chains.items():
        w32(ctx, buckets + 4 * b, es[0])
        for i, e in enumerate(es):
            w32(ctx, e + 0x1c, es[i + 1] if i + 1 < len(es) else 0)
            w32(ctx, e + 0x20, es[i - 1] if i > 0 else 0)
    w32(ctx, table + 8, sum(len(es) for es in chains.values()) + r.choice([0, 0, 0, 3]))
    w32(ctx, table + 0xc, total + r.choice([0, 0, 0, 7, 0x100000]))
    w32(ctx, table + 4, r.choice([0, 1, total // 2, total - 1, total, total + 1, 0x7fffffff, total // 4, 0x80000000]))
    k = r.random()
    if k < 0.4:
        name = 0
    elif k < 0.8:
        name = cstr(ctx, r.choice(keys))
    else:
        name = cstr(ctx, text(ctx, r.choice([8, 3, 12]), b'abcXYZ01'))
    return [name, cstr(ctx, r.choice(EXTS))]


# ---- 0x10034ec9: the frame driver --------------------------------------------------------------------------
def p_scene(tag):
    """A scene pass: leaves its polygon count in 0x1017d138."""
    def pol(uc, n, a):
        uc.mem_write(0x1017d138, struct.pack('<I', pick(mix(ST['seed'], tag, n), [0, 1, 7, 0x100, 0xffffffff, 0x7fffffff])))
        return 0, []
    return pol


def g_4ec9(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 0, 1]))
    w32(ctx, 0x109e3e20, newbuf(ctx, 0x200))
    w32(ctx, 0x109e3ce8, r.choice([0, 1, 0x40, 0x200]))
    w32(ctx, 0x10138c70, r.choice([0, 7, 0xff, ctx.u32()]))
    w32(ctx, 0x1015e53c, r.choice([0, 0, 1, 2, 0xffffffff]))
    w32(ctx, 0x1013a014, r.choice([0, 1]))
    w32(ctx, 0x1014cae0, newbuf(ctx, 0x100))
    w32(ctx, 0x109e3e00, newbuf(ctx, 0x20))
    for a in range(0x109e3e04, 0x109e3e14, 4):
        w32(ctx, a, ctx.u32())
    for a in (0x1014cae8, 0x1014cb18, 0x1014cb1c, 0x1014cb04, 0x1014cb08, 0x1015e540, 0x1013a018, 0x1015ef0c,
              0x1013a008, 0x1015e52c):
        w32(ctx, a, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1013a01c, r.choice([0xffffffff, 0, 3, 0xffffffff]))
    w32(ctx, 0x1015e544, r.choice([0, newbuf(ctx, 0x40), newbuf(ctx, 0x40)]))
    w32(ctx, 0x1015e548, r.choice([0, newbuf(ctx, 0x40), newbuf(ctx, 0x40)]))
    w32(ctx, 0x1017d138, ctx.u32())
    w32(ctx, 0x1015e54c, ctx.u32())
    w32(ctx, 0x1014cb40, ctx.u32())
    n = r.choice([0, 1, 3, 6])
    for i in range(n):
        w32(ctx, 0x1036f2a0 + 4 * i, ctx.u32() | 1)
    w32(ctx, 0x1036f2a0 + 4 * n, 0)
    return []


# ---- 0x100ce983: the HUD message block ---------------------------------------------------------------------
def colour_byte(uc):
    return bytes(uc.mem_read(0x109b992e, 1))[0]


def p_draw(uc, n, a):
    """A text routine (pane, x, y, font, text, colours): logs the text and the colour byte of the moment."""
    return 0, cstring(uc, a[4]) + [-1, colour_byte(uc)]


def p_line(uc, n, a):
    """Sim_1008d6f7 / Sim_1008d8c0 (pane, text, x, y, font, colour)."""
    return 0, cstring(uc, a[1]) + [-1, colour_byte(uc)]


def p_width(uc, n, a):
    return pick(mix(ST['seed'], 0xcb0d, n), [0, 5, 40, 100, 300]), cstring(uc, a[0])


def p_sprintf(uc, n, a):
    """sprintf(buffer, format, ...) for formats made of %s and plain text: writes the text, logs it."""
    esp = uc.reg_read(UC_X86_REG_ESP)
    fmt = bytes(cstring(uc, a[1], 0x100))
    out = b''
    i = k = 0
    while i < len(fmt):
        if fmt[i:i + 2] == b'%s':
            out += bytes(cstring(uc, r32(uc, esp + 12 + 4 * k), 0x100))
            k += 1
            i += 2
        else:
            out += fmt[i:i + 1]
            i += 1
    uc.mem_write(a[0], out + b'\0')
    return len(out), list(out)


def g_e983(ctx):
    r = begin(ctx)
    hud = newbuf(ctx, 0x100)
    w16(ctx, hud + 4, r.choice([0, 1, 1, 1, 1, 0xffff, 0x8000]))
    w32(ctx, hud + 0xec, ctx.u32())
    pos = newbuf(ctx, 8)
    w32(ctx, pos, r.randrange(0, 600), r.randrange(0, 400))
    w32(ctx, hud + 0xf0, pos)
    mode = r.choice([0, 1, 1, 1, 2, 3, 4, 0xffffffff])
    w32(ctx, 0x1015f208, mode)
    own = r.randrange(0, 8)
    w32(ctx, 0x1017b8c8, own)
    w32(ctx, 0x10151d50, r.choice([-2, -2, -1, -1, -1, own, r.randrange(0, 8), r.randrange(0, 8), 8, 9, -3]))
    w32(ctx, 0x109b9900, cstr(ctx, text(ctx, r.randrange(1, 8))))
    w32(ctx, 0x1014cec4, cstr(ctx, r.choice([b'gdf', b'TLT'])))
    unit = r.randrange(0, 8)
    w32(ctx, 0x1015ef18, unit)
    for i in range(8):
        obj = newbuf(ctx, 0x140)
        w32(ctx, obj, r.choice([1, 1, 1, 0, 2]))
        w16(ctx, obj + 0x1c, r.choice([0, 2, 4, 6, 0x800, 0x4000, 0x4800, 1, 0x4806, r.getrandbits(16)]))
        wb(ctx, obj + 0x124, text(ctx, r.randrange(0, 7)) + b'\0')
        w32(ctx, 0x109e8b40 + 4 * i, 0 if r.random() < 0.15 and i != unit else obj)
    for a in (0x101c4c44, 0x101c4c00, 0x101c4c08, 0x101c4c10, 0x101c4c18, 0x101c4c0c, 0x101c4c24, 0x101c4c30,
              0x101c4bfc, 0x101c4c04, 0x101c4c20):
        w32(ctx, a, cstr(ctx, text(ctx, r.randrange(0, 8))))
    state = newbuf(ctx, 0x10)
    wb(ctx, state + 0xa, bytes([r.choice([0, 0, 1, 0x80])]))
    w32(ctx, 0x1017b8c4, state)
    wb(ctx, 0x1015dba0, text(ctx, r.randrange(0, 20)) + b'\0')
    wb(ctx, 0x1017e5f4, text(ctx, r.randrange(0, 6)) + b'\0')
    w32(ctx, 0x101668e8, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1]))
    w32(ctx, 0x10179ca0, ctx.u32(), ctx.u32())
    wb(ctx, 0x109b992e, bytes([r.getrandbits(8)]))
    return [hud]


# ---- 0x100543ee: the input mapping file --------------------------------------------------------------------
STREAM = 0x00A7F000      # the stream argument: only handed to the stubs
CHANNELS = [b'walk_forward', b'walk_backward', b'turn_left', b'menu_up', b'menu_x', b'throttle_plus',
            b'throttle_minus', b'torso_pan', b'fire', b'zz']


def p_readline(uc, n, a):
    """Input_ReadLine(buffer, n, stream): the trial's next line, 0 at the end."""
    if n >= len(ST['lines']):
        return 0, []
    line = ST['lines'][n][:max(a[1] - 1, 0)]
    uc.mem_write(a[0], line + b'\0')
    return 1, []


def p_channel(uc, n, a):
    """Sim_10053620(name): the trial's entry of that name."""
    name = bytes(cstring(uc, a[0]))
    return ST['entries'].get(name, 0), list(name)


def p_text(tag, text_i, options):
    def pol(uc, n, a):
        return pick(mix(ST['seed'], tag, n), options), (cstring(uc, a[text_i]) if a[text_i] else [-2])
    return pol


def p_chord(uc, n, a):
    """Input_ReadChord(stream, count, out): writes a count and a first key."""
    h = mix(ST['seed'], 0x41e9, n)
    uc.mem_write(a[1], struct.pack('<I', h % 9))
    uc.mem_write(a[2], struct.pack('<3I', mix(h, 1), mix(h, 2), mix(h, 3)))
    return pick(h, [0, 1, 1, 1]), []


def g_43ee(ctx):
    r = begin(ctx)
    entries = {}
    for nm in CHANNELS + [b'throttle', b'throttle_delta']:
        if r.random() < 0.8:
            e = newbuf(ctx, 0x20)
            w32(ctx, e, cstr(ctx, nm))
            w32(ctx, e + 4, ctx.u32())
            w32(ctx, e + 8, r.choice([0, 1, 1, 2, 3]))
            entries[nm] = e
    ST['entries'] = entries
    lines = []
    for _ in range(r.choice([0, 1, 2, 3, 5, 8])):
        nm = r.choice(CHANNELS + [b'bogus'])
        if r.random() < 0.05:
            lines.append(r.choice([b'', b'   ', b'\t']))
            continue
        lines.append(r.choice([b'', b' ', b'\t']) + nm + r.choice([b' {', b'{', b' { ', b'']))
        e = entries.get(nm)
        if e is not None and r32(ctx.m.uc, e + 8) == 0:
            if r.random() < 0.1:
                break               # the file ends after the opening line: "unexpected EOF"
            lines.append(r.choice([b' + kbd a', b' - joy x', b'+ kbd 5', b' - kbd ctrl', b'bad', b'  + kbd', b' * kbd a',
                                   b'']))
            for _ in range(r.choice([0, 0, 1, 3])):
                lines.append(r.choice([b' + kbd b', b' x', b'  ']))
            lines.append(r.choice([b'}', b'} ', b'}x']))
    ST['lines'] = lines
    w32(ctx, 0x1015fdf0, r.choice([0, 1, 5, 50, 98, 99, 100, 101]))
    w32(ctx, 0x1015fdf8, r.randrange(0, 1000))
    w32(ctx, 0x10179ce4, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1015965c, 1)         # single-byte code page: isctype uses the table
    wb(ctx, 0x101620c0, rb(ctx, 0x6e * 103))
    wb(ctx, 0x1015fe00, rb(ctx, 0x80 * 0x32))
    return [STREAM]


STUBS.update({
    # cache release
    0x100bda9f: Stub(3, lambda uc, n, a: (ST['table'], []), 'Sim_100bda9f'),
    0x100bd5fc: Stub(1, lambda uc, n, a: (ST['list'], []), 'PtrArray_CreateSized'),
    0x100bd76b: Stub(3, p_insert, 'PtrArray_Insert'),
    0x100b9675: Stub(2, pv(0x9675, [0]), 'Sim_100b9675'),
    0x100bd69c: Stub(2, pv(0xd69c, [0]), 'Sim_100bd69c'),
    0x100ff860: Stub(3, pv(0xf860, [0])),
    # frame driver (hooks_view.c patches 0x100d2306, 0x100d1f90 and 0x10087d83)
    0x100d32e5: Stub(1, pv(0x32e5, [0]), 'Sim_100d32e5'),
    0x100d2306: Stub(1, pv(0x2306, [0])),
    0x100d255e: Stub(1, pv(0x255e, [0]), 'Sim_100d255e'),
    0x100d1f90: Stub(1, pv(0x1f90, [0])),
    0x100eb070: Stub(2, pv(0xb070, [0])),
    0x100a0def: Stub(1, pv(0x0def, [0])),
    0x100b53f0: Stub(4, pv(0x53f0, [0]), 'Sim_100b53f0'),
    0x100d2545: Stub(2, pv(0x2545, [0]), 'Sim_100d2545'),
    0x100a4ff0: Stub(1, p_scene(0x4ff0), 'Sim_100a4ff0'),
    0x100a4d8e: Stub(0, p_scene(0x4d8e), 'Sim_100a4d8e'),
    0x100d252c: Stub(2, pv(0x252c, [0]), 'Sim_100d252c'),
    0x100a512a: Stub(1, p_scene(0x512a), 'Sim_100a512a'),
    0x10087d83: Stub(1, pv(0x7d83, [0])),
    0x100357db: Stub(1, pv(0x57db, [0])),
    0x100071f6: Stub(0, pv(0x71f6, [0]), 'Sim_100071f6'),
    # HUD message block (hooks_conv.c patches 0x100ba7d5)
    0x100ba7d5: Stub(3, pv(0xa7d5, [DATA, DATA + 0x10, 0])),
    0x100ecce0: Stub(1, pv(0xcce0, [0, 1, 2, 7, 8, 13, 16, 0xffffffff])),
    0x100ecea6: Stub(6, p_draw),
    0x100f1e10: Stub(6, p_draw),
    0x100f2f01: Stub(6, p_draw),
    0x100fc780: Stub(2, p_sprintf, 'Crt_SPrintf'),
    0x100bb2b0: Stub(2, pv(0xb2b0, [0, 1]), 'Sim_100bb2b0'),
    0x1008d6f7: Stub(6, p_line, 'Sim_1008d6f7'),
    0x1008d8c0: Stub(6, p_line, 'Sim_1008d8c0'),
    0x100ccb0d: Stub(2, p_width, 'Sim_100ccb0d'),
    # input mapping file
    0x10053b64: Stub(3, p_readline, 'Input_ReadLine'),
    0x10053620: Stub(1, p_channel, 'Sim_10053620'),
    0x1005388b: Stub(1, p_text(0x388b, 0, [0, 1, 2, 0xffffffff]), 'Input_GetDevice'),
    0x1005393a: Stub(2, p_text(0x393a, 1, [0xffffffff, 0xffffffff, 0, 1, 5, 31]), 'Sim_1005393a'),
    0x10053fe1: Stub(3, p_text(0x3fe1, 1, [0xffffffff, 0, 1, 7, 0x31]), 'Input_AddAnalog'),
    0x100541e9: Stub(3, p_chord, 'Input_ReadChord'),
})


def spec(name, orig, gen, trials, **kw):
    return dict(name=name, orig=orig, c=name, gen=gen, trials=trials, **kw)


SPECS = [
    spec('Sim_1001dd66', 0x1001dd66, g_dd66, 300, void=True),
    spec('Sim_10075753', 0x10075753, g_5753, 1000, void=True),
    spec('GameThing_NextIndex', 0x100dc2d7, g_dc2d7, 400),
    spec('Sim_1009b516', 0x1009b516, g_9b516, 1500),
    spec('Hud_DrawShadedBox', 0x10051c90, g_1c90, 1000, void=True),
    spec('Sim_1005a1db', 0x1005a1db, g_a1db, 1500),
    spec('Sim_100b43ce', 0x100b43ce, g_43ce, 1500),
    spec('Sim_100b9203', 0x100b9203, g_9203, 1500),
    spec('Sim_10034ec9', 0x10034ec9, g_4ec9, 1000, void=True),
    spec('Sim_100ce983', 0x100ce983, g_e983, 1500, void=True),
    spec('Input_ReadMap', 0x100543ee, g_43ee, 1500),
]
