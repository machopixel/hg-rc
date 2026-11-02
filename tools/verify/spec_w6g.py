# Differential tests for round 6, letter g (see difftest.py and docs/checker.md).
#
# The functions call helpers of the DLL that are not converted yet (through their original address, SIM_FN) and
# converted C helpers that in turn call original code. Helpers that need a world the generators cannot build
# (file and memory functions of the C library, the CD init, error reporting, renderer and matrix code) are replaced
# in the emulator by a stub at the helper's own address, so the original and the C run reach the same stub: a code
# hook that appends ('stub_<address>', argument words) to the checker's call log (compared between the two runs,
# stack addresses shown as 'stack'), answers with a value that depends only on the trial and the position in the
# log, optionally writes dwords through some of its pointer arguments (the out parameters of the real helper), and
# returns to the caller (cdecl). Code bytes are not patched. The C library square root (0x100fca04), the function
# 0x100fcb70 and the float helper 0x10047bcd run for real, with the Windows control word 0x27f (53-bit precision).
# The drawing function pointer 0x1014cb4c and the callback argument of Sim_100bd69c point at two more stubs; the
# drawing stub logs the vertex fields the caller sets.
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EIP, UC_X86_REG_EAX, UC_X86_REG_FPCW, UC_X86_REG_FPSW, UC_X86_REG_FPTAG

SOURCES = [
    'sim/unsorted_1009/w6g_segment.c', 'sim/win/w6g_cd.c', 'sim/unsorted_100a/w6g_misc.c', 'sim/unsorted_100a/w6g_nodes.c',
    'sim/unsorted_100a/w6g_records.c', 'sim/unsorted_100b/w6g_misc.c', 'sim/unsorted_100c/w6g_calls.c',
    # converted code the functions above reach
    'sim/win/w4u_cdaudio.c', 'sim/win/w4v_heap.c', 'sim/view/matrix.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_1000/w2k_misc.c', 'sim/util/field.c', 'sim/unsorted_1000/w5b_calls.c',
    'sim/unsorted_1001/w5y_misc.c', 'sim/unsorted_1001/w5c_math.c', 'sim/unsorted_1001/w3t_misc.c', 'sim/unsorted_1002/w5b_calls.c',
    'sim/unsorted_1002/w2g_records.c', 'sim/core/w2a_timers.c', 'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1003/w5c_calls.c',
    'sim/unsorted_1003/w2l_records.c', 'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w5a_math.c', 'sim/view/objlist.c',
    'sim/unsorted_1008/w2i_rect.c', 'sim/view/w2l_lists.c', 'sim/unsorted_1008/w3s_grid.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_100a/w2e_search.c', 'sim/unsorted_100a/w5z_calls.c', 'sim/unsorted_100a/w5x_calls.c',
    'sim/unsorted_100a/w5b_calls.c', 'sim/unsorted_100a/w5y_misc.c', 'sim/unsorted_100a/w2i_misc.c',
    'sim/unsorted_100a/w2j_lists.c', 'sim/render/w2n_polygon.c', 'sim/unsorted_100b/w2h_records.c',
    'sim/unsorted_100b/w5z_calls.c', 'sim/unsorted_100b/w3o_hash.c', 'sim/unsorted_100b/w5c_calls.c',
    'sim/unsorted_100b/w3s_array.c', 'sim/unsorted_100b/fields.c', 'sim/unsorted_100b/w2l_records.c',
    'sim/unsorted_100c/w5c_math.c', 'sim/unsorted_100c/w2l_objects.c', 'sim/unsorted_100c/w5a_misc.c',
    'sim/unsorted_100c/w5x_calls.c', 'sim/unsorted_100d/w2j_nodes.c', 'sim/win/w4w_win.c', 'sim/win/w4a_heap.c',
    'sim/unsorted_1000/w2e_mono.c', 'sim/unsorted_1003/w2e_mono.c', 'sim/unsorted_100b/w2e_strings.c', 'sim/view/viewrect.c',
    'sim/render/w2n_project.c', 'sim/unsorted_1000/w2l_objects.c', 'sim/unsorted_1000/w3s_model.c',
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1000/w2g_fields.c', 'sim/unsorted_1000/w2l_fields.c',
    'sim/unsorted_1001/w2j_misc.c', 'sim/unsorted_1001/w4u_cd.c', 'sim/unsorted_1001/w2e_tables.c',
    'sim/unsorted_1002/w5y_misc.c', 'sim/unsorted_1002/w2j_nodes.c', 'sim/unsorted_1002/w3q_collide.c',
    'sim/unsorted_1002/w2h_misc.c', 'sim/unsorted_1002/w2k_misc.c', 'sim/unsorted_1003/w3r_misc.c', 'sim/unsorted_1003/setters.c',
    'sim/unsorted_1003/w2g_misc.c', 'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3o_grid.c',
    'sim/unsorted_1008/cell_link.c', 'sim/view/w2d_view.c', 'sim/unsorted_1008/w2g_rect.c', 'sim/util/ring.c',
    'sim/unsorted_100a/w4u_misc.c', 'sim/unsorted_100a/w2l_small.c', 'sim/unsorted_100a/w2f_links.c',
    'sim/unsorted_100a/w2k_list.c', 'sim/unsorted_100b/w4v_wrappers.c', 'sim/win/w4v_dll.c', 'sim/unsorted_100c/w3o_misc.c',
    'sim/unsorted_100c/w2i_misc.c', 'sim/unsorted_100d/w2f_records.c', 'sim/core/timer.c', 'sim/unsorted_1002/w4u_heap.c',
    'sim/unsorted_1000/w2j_slots.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_100c/w2e_tables.c', 'sim/unsorted_100c/w4v_timers.c', 'sim/util/lookup.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STACK_LO, STACK_HI = 0x00700000, 0x00800000
DRAW = 0x00F00200       # target of the drawing function pointer 0x1014cb4c
CALLBACK = 0x00F00300   # callback argument of Sim_100bd69c
ARENA = 0x00A80000      # blocks handed out by the HeapAlloc stub

# helper address -> (number of argument words logged, argument indices the stub writes a dword through)
STUBS = {
    0x1009dfa0: (0, ()),            # CD audio init (patched by hooks_sim.c)
    0x100d1f90: (1, ()), 0x10087d83: (1, ()),       # patched by hooks_view.c
    0x100585c4: (1, ()),            # lights query (patched by hooks_view.c)
    0x100bc280: (2, ()),            # system error report (patched by hooks_sim.c)
    0x100a4f03: (1, ()), 0x100c6e27: (1, ()), 0x100a7e55: (1, ()), 0x100aaa08: (2, ()), 0x10060d2e: (2, ()),
    0x100fefe0: (1, ()), 0x100ff3b0: (1, ()), 0x1010d180: (1, ()), 0x100c2ae5: (1, ()), 0x100fd760: (1, ()),
    0x10027fb8: (1, ()), 0x100fc780: (4, ()), 0x1005f9aa: (1, ()),
    0x1006123b: (5, ()), 0x100aa742: (1, ()), 0x10088a2d: (1, ()), 0x100a9c25: (1, ()), 0x100a9ba7: (1, ()),
    0x10060681: (4, (1, 2, 3)),     # writes three dwords through its pointer arguments
    0x10033eb3: (5, (4,)),          # writes a dword through its last argument
    0x1000b070: (2, ()), 0x1000b587: (2, ()), 0x100cb4fb: (1, ()), 0x10015630: (4, ()),
    CALLBACK: (1, ()),
}
EXTRA = {}          # further stubs added below as (nargs, outs)
# Argument words of which the stubbed helper only reads the low 16 bits (every read of that argument in the helper is
# 16 bits wide): 0x10015630 gets a word pushed with whatever the caller had in the top half (Sim_100bee33).
MASK = {(0x10015630, 1): 0xffff, (0x10015630, 2): 0xffff}
ST = {}


# ---- helpers -------------------------------------------------------------------------------------------
def w32(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(vals), *[v & 0xffffffff for v in vals]))


def w16(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%dH' % len(vals), *[v & 0xffff for v in vals]))


def wf(ctx, addr, *vals):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(vals), *vals))


def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))


def r32(uc, addr):
    return struct.unpack('<I', bytes(uc.mem_read(addr, 4)))[0]


def rnd(ctx, n):
    return bytes(ctx.rng.getrandbits(8) for _ in range(n))


def newbuf(ctx, n):
    return ctx.buf(n, rnd(ctx, n))


def zbuf(ctx, n):
    return ctx.buf(n, bytes(n))


def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]


def rfl(ctx, lo=-1000.0, hi=1000.0):
    """A float as its bits: ordinary values, zeros, small and large magnitudes, both signs."""
    r = ctx.rng
    c = r.random()
    if c < 0.55:
        return fbits(r.uniform(lo, hi))
    if c < 0.75:
        return fbits(r.choice([0.0, -0.0, 1.0, -1.0, 0.5, 2.0, 100.0, 1e-3, 1e-7, 1e-8, -3.0]))
    if c < 0.95:
        return fbits(r.uniform(-1, 1) * 10 ** r.randint(-10, 10))
    return r.getrandbits(32) & 0xff7fffff


def cstr(ctx, data, extra=16):
    return ctx.buf(len(data) + 1 + extra, data + b'\0' + rnd(ctx, extra))


def hsh(n, salt=0):
    x = (ST['seed'] + n * 0x9E3779B1 + salt * 0x85EBCA6B) & 0xffffffff
    x ^= x >> 15
    x = (x * 0x2C1B3C6D) & 0xffffffff
    x ^= x >> 12
    x = (x * 0x297A2D39) & 0xffffffff
    x ^= x >> 15
    return x


# ---- stubs ---------------------------------------------------------------------------------------------
def stub_ret(addr, n):
    vals = ST['rets'].get(addr)
    if vals:
        return vals[n % len(vals)]
    h = hsh(n, addr & 0xffff)
    return [0, 1, 2, 0xffffffff, h][h % 5]


def _arg_word(uc, esp, k):
    return r32(uc, esp + 4 + 4 * k)


def _stub_cb(m):
    def cb(uc, address, size, ud):
        nargs, outs = STUBS.get(address) or EXTRA[address]
        esp = uc.reg_read(UC_X86_REG_ESP)
        raw = [_arg_word(uc, esp, k) for k in range(nargs)]
        args = tuple('stack' if STACK_LO <= v <= STACK_HI else v & MASK.get((address, k), 0xffffffff)
                     for k, v in enumerate(raw))
        n = len(m.api_log)
        m.api_log.append(('stub_%x' % address, args))
        for j, k in enumerate(outs):
            uc.mem_write(raw[k], struct.pack('<I', hsh(n, 0x100 + j)))
        uc.reg_write(UC_X86_REG_EAX, stub_ret(address, n) & 0xffffffff)
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def _draw_cb(m):
    """The drawing function: logs count and colour and, per vertex, the fields the callers set (x, y, +0x10, +0x18)."""
    def cb(uc, address, size, ud):
        esp = uc.reg_read(UC_X86_REG_ESP)
        count, verts, colour = (_arg_word(uc, esp, k) for k in range(3))
        fields = []
        for i in range(min(count, 8)):
            v = struct.unpack('<9I', bytes(uc.mem_read(verts + 0x24 * i, 0x24)))
            fields += [v[0], v[1], v[4], v[6]]
        n = len(m.api_log)
        m.api_log.append(('draw', count, colour, tuple(fields)))
        uc.reg_write(UC_X86_REG_EAX, hsh(n, 77))
        uc.reg_write(UC_X86_REG_EIP, r32(uc, esp))
        uc.reg_write(UC_X86_REG_ESP, esp + 4)
    return cb


def install(m):
    if getattr(m, '_w6g', False):
        return
    m._w6g = True
    cb = _stub_cb(m)
    for a in list(STUBS) + list(EXTRA):
        m.raw.hook_add(UC_HOOK_CODE, cb, None, a, a)
    m.raw.hook_add(UC_HOOK_CODE, _draw_cb(m), None, DRAW, DRAW)
    # the text screen the mono print functions write to
    m.uc.mem_map(0xb0000, 0x10000)
    m.wr.append((0xb0000, 0x10000))
    m.snap0 = m.dump()
    m._index()


def begin(ctx):
    """Stubs (once per machine), x87 state like the Windows control word, per-trial answers. First call of every generator."""
    m = ctx.m
    install(m)
    r = ctx.rng
    ST.clear()
    ST['seed'] = r.getrandbits(32)
    ST['rets'] = {}
    ST['blk'] = [ARENA + 0x400 * i for i in range(4)]
    ST['mci_fail'] = r.choice([0, 0, 0, 10, 35])
    ST['cursor'] = r.choice([0, 0, 1, 2, 3])
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    m.uc.reg_write(UC_X86_REG_FPSW, 0)
    m.uc.reg_write(UC_X86_REG_FPTAG, 0xffff)
    wb(ctx, ARENA, rnd(ctx, 0x1000))
    w32(ctx, 0x1017b8f8, r.getrandbits(32) | 0x10000)       # heap handle
    w32(ctx, 0x10a3ade8, r.choice([0, 1]))                   # text screen output on/off
    return r


def rets(addr, *values):
    ST['rets'][addr] = [v & 0xffffffff for v in values]


# ---- Windows API answers ---------------------------------------------------------------------------------
def a_heapalloc(n, a):
    h = hsh(n, 3)
    return 0 if h % 7 == 0 else ST['blk'][h % len(ST['blk'])]


def a_mci(n, a, uc):
    msg, flags = a[1], a[2]
    ptr = r32(uc, uc.reg_read(UC_X86_REG_ESP) + 16)
    h = hsh(n)
    ret = 0x111 if h % 100 < ST['mci_fail'] else 0
    if msg == 0x814 and ret == 0:
        item = r32(uc, ptr + 8)
        if item == 4:
            modes = [0x20d, 0x20e, 0x211, 0x212, 0x20c, 0x20f, 0x210, 0x213, 1]
            v = modes[(h >> 4) % len(modes)]
        elif item == 3:
            v = (h >> 8) % 7
        elif flags & 0x10:
            v = h
        else:
            v = (h & 0xffffff00) | ((h >> 8) % 8)
        uc.mem_write(ptr + 4, struct.pack('<I', v))
    return ret


def a_showcursor(n, a):
    return 0xffffffff if n < ST['cursor'] + 3 and hsh(n, 5) % 3 else 1


API = {'HeapAlloc': a_heapalloc, 'mciSendCommandA': a_mci, 'ShowCursor': a_showcursor}


# ---- generators: segments -------------------------------------------------------------------------------
def seg(ctx):
    r = ctx.rng
    s = newbuf(ctx, 0x40)
    pts = [rfl(ctx) for _ in range(6)]
    if r.random() < 0.08:
        pts[3:6] = pts[0:3]             # zero length
    w32(ctx, s, *pts)
    w32(ctx, s + 0x18, *[rfl(ctx) for _ in range(3)])
    w32(ctx, s + 0x24, *[rfl(ctx, -1, 1) for _ in range(3)])
    w32(ctx, s + 0x30, r.choice([rfl(ctx, 0, 2000), fbits(0.0), fbits(1e-9), rfl(ctx)]))
    w32(ctx, s + 0x34, r.choice([0, 1, 2, 2, 3, 0xffffffff]))
    return s


def g_cc82(ctx):
    begin(ctx)
    return [seg(ctx)]


def g_cdd5(ctx):
    begin(ctx)
    s = seg(ctx)
    w32(ctx, s + 0x34, ctx.rng.choice([0, 0, 1, 2, 2, 5]))
    return [s, rfl(ctx, -50, 50)]


def g_ceab(ctx):
    begin(ctx)
    s = seg(ctx)
    w32(ctx, s + 0x34, ctx.rng.choice([0, 1, 1, 2, 2, 3, 0xffffffff]))
    return [s, rfl(ctx), rfl(ctx), rfl(ctx)]


# ---- generators: CD ----------------------------------------------------------------------------------------
def cd_env(ctx):
    r = begin(ctx)
    w32(ctx, 0x1014c9b0, r.choice([0, 1, 2, 0xffffffff, r.getrandbits(16)]))
    w32(ctx, 0x1014c9b4, r.choice([0, 1, 0xffffffff]))
    w32(ctx, 0x1017d08c, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1017d080, r.choice([0, 1]), r.randrange(0, 9), ctx.buf(0x80))
    w32(ctx, 0x1017d068, r.randrange(0, 9), r.getrandbits(8), r.getrandbits(6), r.getrandbits(5))
    w32(ctx, 0x1017d060, ctx.u32(), ctx.u32())
    w32(ctx, 0x1017d078, ctx.u32())
    w32(ctx, 0x101321bc, r.choice([0, 0x8000, 0xffff, 0x10000, ctx.u32()]))
    return r


def g_cd_open(ctx):
    r = cd_env(ctx)
    rets(0x1009dfa0, r.choice([0, 0, 0, 0, 1, 5]))
    return []


def g_cd_door(ctx):
    cd_env(ctx)
    return []


# ---- generators: 0x100a ------------------------------------------------------------------------------------
def g_a2823(ctx):
    r = begin(ctx)
    w32(ctx, 0x1014cb4c, DRAW)
    vals = [r.choice([0, 1, -1, 320, 199, 0x7fffffff, -0x80000000, 0x1000001, r.randrange(-2000, 2000), r.getrandbits(32)])
            for _ in range(10)]
    return vals + [r.choice([0, 0xff, 0x12345678, ctx.u32()])]


def g_a5336(ctx):
    r = begin(ctx)
    k = r.choice([0, 1, 2, 3, 5, 8])
    w32(ctx, 0x1036f2a0, *([r.getrandbits(32) | 1 for _ in range(k)] + [0]))
    return [ctx.u32(), ctx.u32()]


def hash_table_1015dc30(ctx):
    r = ctx.rng
    if r.random() < 0.6:
        w32(ctx, 0x1015dc30, 0)
        return
    tab = zbuf(ctx, 0x3f1 * 4)
    for _ in range(r.choice([1, 2, 4])):
        node = newbuf(ctx, 0x20)
        b = r.randrange(0, 0x3f1)
        w16(ctx, node, r.choice([0xf000, 0xffff, 0x1234, 0xefff]))
        w32(ctx, node + 8, r32(ctx.m.uc, tab + 4 * b))
        w32(ctx, tab + 4 * b, node)
    w32(ctx, 0x1015dc30, tab)


def g_a7880(ctx):
    r = begin(ctx)
    hash_table_1015dc30(ctx)
    p = 0
    if r.random() < 0.6:
        p = newbuf(ctx, 0x20)
        w32(ctx, p, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, 0x1017d158, p)
    name = bytes(r.choice(b'abcXYZ019._\\') for _ in range(r.randrange(0, 40)))
    return [cstr(ctx, name)]


def g_a7f8c(ctx):
    r = begin(ctx)
    names_all = []
    for idx in range(4):
        n = r.choice([0, 1, 2, 3, 5])
        nrec = newbuf(ctx, 0xe + 0x18 * max(n, 1) + 8)
        w16(ctx, nrec + 0xc, n)
        names = []
        for e in range(n):
            nm = bytes(r.choice(b'abAB') for _ in range(4))
            wb(ctx, nrec + 0xe + 0x18 * e, nm)
            names.append(nm)
        rec = 0x101c65e6 + idx * 0x14a
        w32(ctx, rec, nrec)
        for e in range(8):
            vals = 0
            if r.random() < 0.85:
                vals = newbuf(ctx, 0x16 + 8 * 8 + 8)
                for s in range(8):
                    w32(ctx, vals + 0x16 + 8 * s + 4, r.choice([0, 1, 0xffff, 0x10000, ctx.u32(), r.randrange(0, 0x20000)]))
            w32(ctx, rec + 0x24 + 8 * e, vals)
        names_all.append(names)
    index = r.randrange(0, 4)
    names = names_all[index]
    key = r.choice(names) if names and r.random() < 0.8 else bytes(r.choice(b'abAB') for _ in range(4))
    rets(0x100a7e55, r.choice([0xffffffff, 0, 1]))
    slot = r.choice([0, 0x10000, 1, 2, 3, 7, 0x20005, r.getrandbits(32) & 0xffff0007])
    return [index, cstr(ctx, key), slot]


def node_obj(ctx):
    """A scene node of 0xa4 bytes with random content (pointer fields cleared)."""
    n = newbuf(ctx, 0xc0)
    w32(ctx, n, 0, 0, 0)
    w32(ctx, n + 0x8c, 0)
    return n


def scene_item(ctx):
    """The object a node keeps at +0x8c: flags word +0, cell links +4/+8, list links +0x14/+0x18 (and those of the
    second list) cleared, owner node at +0x28 (its floats +0x70..+0x78 are read); now and then linked after an anchor."""
    r = ctx.rng
    o = zbuf(ctx, 0x100)
    w16(ctx, o, r.choice([0, 0x800, 0x1000, 0x1800, 0x8000, 0xffff, r.getrandbits(16)]))
    w16(ctx, o + 2, r.choice([0, 0x70, 0x10, r.getrandbits(16)]))
    w16(ctx, o + 0x26, r.choice([0, 1, 2, 5]))
    w32(ctx, o + 0x34, r.choice([4, 0, 1]))
    owner = zbuf(ctx, 0xc0)
    w32(ctx, owner + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
    w32(ctx, o + 0x28, owner)
    if r.random() < 0.5:
        anchor = zbuf(ctx, 0x100)
        w32(ctx, anchor + 8, o)
        w32(ctx, o + 4, anchor)
    return o


def g_a8c24(ctx):
    r = begin(ctx)
    if r.random() < 0.05:
        return [0]
    obj = newbuf(ctx, 0x500)
    w32(ctx, obj + 4, r.randrange(-100, 100) & 0xffffffff, rfl(ctx), rfl(ctx), rfl(ctx, 0.5, 10))
    w32(ctx, obj + 0x14, r.randrange(-100, 100) & 0xffffffff, rfl(ctx), rfl(ctx), rfl(ctx, 0.5, 10))
    w32(ctx, 0x1015dc78, r.randrange(-200, 200))
    head = newbuf(ctx, 0x100)
    w32(ctx, head + 0x48, node_obj(ctx))
    w32(ctx, obj, head)
    w32(ctx, obj + 0x490, node_obj(ctx) if r.random() < 0.6 else 0)
    w32(ctx, obj + 0x48c, node_obj(ctx) if r.random() < 0.6 else 0)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return [obj]


def obj_tree(ctx, depth):
    """Nodes (first child +4, next sibling +8) with an object at +0x8c now and then."""
    r = ctx.rng
    node = node_obj(ctx)
    if r.random() < 0.6:
        w32(ctx, node + 0x8c, scene_item(ctx))
    kids = [obj_tree(ctx, depth - 1) for _ in range(r.choice([0, 1, 2, 3]))] if depth > 0 else []
    if kids:
        w32(ctx, node + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
        for k in kids:
            w32(ctx, k, node)
    return node


def g_a96d8(ctx):
    r = begin(ctx)
    w32(ctx, 0x1017d1b8, obj_tree(ctx, r.choice([0, 1, 2])) if r.random() < 0.85 else 0)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return [r.choice([0, 0, 1, 2, 0xffffffff])]


def parent_with_kids(ctx):
    r = ctx.rng
    if r.random() < 0.15:
        return 0
    p = node_obj(ctx)
    wb(ctx, p + 0x4c, rnd(ctx, 0x40))
    kids = [node_obj(ctx) for _ in range(r.choice([0, 0, 1, 2, 3]))]
    if kids:
        w32(ctx, p + 4, kids[0])
        for a, b in zip(kids, kids[1:]):
            w32(ctx, a + 8, b)
    return p


def g_a9830(ctx):
    r = begin(ctx)
    p = parent_with_kids(ctx)
    w32(ctx, 0x1014d1a8, r.choice([0, 0x40, 0x7f, 0xbf, ctx.u32()]))
    w32(ctx, 0x10134f88, ctx.u32())
    return [p, r.choice([0, 1, 2, 4, 6, 0x12, 0x14, 0x10, 0x16, ctx.u32()])]


def g_a9a1a(ctx):
    r = begin(ctx)
    p = parent_with_kids(ctx)
    w32(ctx, 0x1014d1a8, r.choice([0, 0x40, 0x7f, 0xbf, ctx.u32()]))
    return [p, newbuf(ctx, 0xc0)]


def g_a9e1a(ctx):
    begin(ctx)
    return [newbuf(ctx, 0xc0), newbuf(ctx, 0x40)]


def g_aa8cb(ctx):
    r = begin(ctx)
    p = parent_with_kids(ctx)
    node = node_obj(ctx)
    wb(ctx, node + 0x4c, rnd(ctx, 0x40))
    w32(ctx, node + 0x94, ctx.u32())
    if p and r.random() < 0.8:
        # link the node into the parent's list at a random place (or leave it out)
        kids = []
        k = r32(ctx.m.uc, p + 4)
        while k:
            kids.append(k)
            k = r32(ctx.m.uc, k + 8)
        pos = r.randrange(0, len(kids) + 1)
        kids.insert(pos, node)
        if r.random() < 0.1:
            kids.remove(node)
        w32(ctx, p + 4, kids[0] if kids else 0)
        for a, b in zip(kids, kids[1:] + [0]):
            w32(ctx, a + 8, b)
    w32(ctx, node, p)
    if r.random() < 0.6:
        w32(ctx, node + 0x8c, scene_item(ctx))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return [node]


def g_aae7b(ctx):
    r = begin(ctx)
    node = obj_tree(ctx, r.choice([0, 1, 2]))
    return [node, r.choice([-1, 0, 1, 2, 3, 5, 15, 16, 17, 8, 9]) & 0xffffffff, r.choice([0, 1, 2, 5, 0x10002])]


def g_aaf32(ctx):
    r = begin(ctx)
    if r.random() < 0.08:
        return [0]
    p = parent_with_kids(ctx)
    node = obj_tree(ctx, r.choice([0, 1]))
    w32(ctx, node, p)
    if p:
        w32(ctx, node + 8, r32(ctx.m.uc, p + 4))
        w32(ctx, p + 4, node)
    if r.random() < 0.15:
        w32(ctx, node + 0x8c, 0)
    elif r32(ctx.m.uc, node + 0x8c) == 0:
        w32(ctx, node + 0x8c, scene_item(ctx))
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    return [node]


def g_ab186(ctx):
    r = begin(ctx)
    a = node_obj(ctx) if r.random() < 0.9 else 0
    b = node_obj(ctx) if r.random() < 0.9 else 0
    return [a, b]


def scene_entry(ctx):
    """An entry for Sim_100abc5d's list with its node (+0x30) and the node's object (+0x8c)."""
    e = newbuf(ctx, 0x40)
    node = node_obj(ctx)
    o = newbuf(ctx, 0x100)
    w32(ctx, node + 0x8c, o)
    w32(ctx, e + 0x30, node)
    return e


def g_ac21a(ctx):
    r = begin(ctx)
    # Sim_100abc5d pops a record number from the free stack 0x101c56c0 (top index [0x1017d1c4], -1 when empty)
    # and appends record 0x101c57e0 + 0x34 * number to the list head/tail 0x1017d1d0/0x1017d1d4
    for i in range(4):
        rec = 0x101c57e0 + i * 0x34
        wb(ctx, rec, rnd(ctx, 0x34))
        e_node = node_obj(ctx)
        w32(ctx, e_node + 0x8c, scene_item(ctx))
        w32(ctx, rec + 0x30, e_node)
    w32(ctx, 0x1017d1c4, r.choice([0xffffffff, 0, 1, 2, 2]))
    w32(ctx, 0x101c56c0, *[r.randrange(0, 4) for _ in range(4)])
    if r.random() < 0.5:
        w32(ctx, 0x1017d1d0, 0, 0)
    else:
        w32(ctx, 0x1017d1d0, newbuf(ctx, 0x34), newbuf(ctx, 0x34))
    src = newbuf(ctx, 0x100) if r.random() < 0.9 else 0
    if src:
        o = newbuf(ctx, 0x100)
        w32(ctx, o + 0x70, rfl(ctx), rfl(ctx), rfl(ctx))
        w32(ctx, src + 0x3c, o)
    rets(0x100585c4, r.choice([0, 1, 1, 5]))
    kind = r.choice([1, 1, 2, 2, 6, 6, 0, -1, 3, 0x3f, 0x40, 5])
    return [kind & 0xffffffff, src]


def g_ac620(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 2, 3, 5])
    w32(ctx, 0x1017d1c8, count)
    for i in range(count):
        rec = 0x101c57e0 + i * 0x34
        wb(ctx, rec, rnd(ctx, 0x34))
        node = 0
        if r.random() < 0.85:
            node = node_obj(ctx)
            w32(ctx, node + 0x8c, scene_item(ctx) if r.random() < 0.8 else 0)
        w32(ctx, rec + 0x30, node)
    w32(ctx, 0x1017c8c4, r.choice([0, 1]))
    vals = newbuf(ctx, 0x14)
    return [r.choice([-1, 0, 0, 1, 2, 3, 4, 5, 0x7fffffff]) & 0xffffffff, r.choice([0, 1, 2, 3, 4, 5, 6, 7, ctx.u32()]), vals]


def g_ad87d(ctx):
    r = begin(ctx)

    def tree(depth):
        node = node_obj(ctx)
        if r.random() < 0.8:
            # mesh block: vertex count +4 (0x58-byte vertices from +0x18), part count +6, parts of 0x28 bytes at
            # mesh + dword +8. Sim_100aa307 (run when the object's +0x9c differs from the mesh's +0x10) transforms
            # them with the matrix at owner+0x4c; that matrix is minus the identity and the parts' points are
            # positive, so every part keeps bit 31 of its +0x1c set and Sim_100ad1f3 returns at once.
            obj = newbuf(ctx, 0x100)
            nverts = r.choice([0, 1, 2])
            nparts = r.choice([0, 1, 2, 3])
            mesh = newbuf(ctx, 0x18 + 0x58 * nverts + 0x28 * max(nparts, 1))
            parts = mesh + 0x18 + 0x58 * nverts
            for i in range(nparts):
                w32(ctx, parts + 0x28 * i + 0xc, fbits(r.uniform(1, 50)), fbits(r.uniform(1, 50)), fbits(r.uniform(1, 50)))
                w32(ctx, parts + 0x28 * i + 0x1c, r.getrandbits(32) | 0x80000000)
            w16(ctx, mesh + 4, nverts)
            w32(ctx, mesh + 8, parts - mesh)
            w16(ctx, mesh + 6, nparts if r.random() < 0.9 else r.choice([0, 0xffff, 0x8000]))
            w32(ctx, mesh + 0x10, ctx.u32())
            w32(ctx, obj + 0x30, mesh)
            w32(ctx, obj + 0x9c, r32(ctx.m.uc, mesh + 0x10) if r.random() < 0.5 else ctx.u32())
            owner = node_obj(ctx)
            w32(ctx, owner + 0x4c, *[fbits(v) for v in (-1, 0, 0, 0, -1, 0, 0, 0, -1, 0, 0, 0)])
            w32(ctx, obj + 0x28, owner)
            w16(ctx, obj + 2, r.choice([0, 0, 0x10, 0x40, 0x8f, 0xff0f]))
            w32(ctx, node + 0x8c, obj)
        kids = [tree(depth - 1) for _ in range(r.choice([0, 1, 2]))] if depth > 0 else []
        if kids:
            w32(ctx, node + 4, kids[0])
            for a, b in zip(kids, kids[1:]):
                w32(ctx, a + 8, b)
        return node
    ctxp = newbuf(ctx, 0x40) if r.random() < 0.95 else 0
    return [ctxp, tree(r.choice([0, 1, 2])) if r.random() < 0.95 else 0]


# ---- generators: 0x100b ------------------------------------------------------------------------------------
def g_b5ad7(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 2, 4, 6, 9])
    w32(ctx, 0x1017d218, count)
    for i in range(max(count, 1) + 1):
        rec = 0x101a2100 + i * 0x88
        wb(ctx, rec, rnd(ctx, 0x88))
        parent = r.randrange(0, i) if i and r.random() < 0.6 else r.choice([0xffffffff, 0x500, i + 1 + r.randrange(0, 3)])
        w32(ctx, rec + 0x14, parent)
        w32(ctx, rec + 0x24, r.choice([0, 1, 5, 0xfd, 0xfe, 0xffffffff, r.randrange(0, 0xfe)]))
    ident = r.choice([r.randrange(0, max(count, 1)), r.randrange(0, max(count, 1)), 0x401, 0x402, 0xffffffff, 0x500])
    if ident == 0xffffffff or ident >= max(count, 1) + 1:
        rec = 0x101a2100 + (ident & 0x3ff) * 0x88
        if ident < 0x402:
            wb(ctx, rec, rnd(ctx, 0x88))
            w32(ctx, rec + 0x24, r.choice([0, 5, 0xfe, 0xffffffff]))
    return [ident]


def g_b65b7(ctx):
    r = begin(ctx)
    count = r.choice([0, 1, 3, 8, 0x1f, 0x20, 0x21, 0xffffffff])
    w32(ctx, 0x1017d4fc, count)
    keys = []
    for i in range(min(count, 0x21)):
        k = r.choice([r.randrange(0, 8), ctx.u32()])
        keys.append(k)
        w32(ctx, 0x1017d278 + i * 0x14, k, r.choice([0, 1, 0xffdc, 0xffdd, 0xffd0, ctx.u32()]))
    key = r.choice(keys) if keys and r.random() < 0.6 else ctx.u32()
    amount = r.choice([0, 1, -1, 5, 0x20, 0xffdc, 0x10000, ctx.s32small()])
    return [amount & 0xffffffff, key]


def name_tables(ctx):
    """The sorted table list at [0x1017d604] for Sim_100bda9f; each hash table: name at +0x18, bucket count at +0,
    bucket array at +0x10, entries of 0x20 bytes (8-byte key, count at +8, next at +0x1c). Returns [(name, table, keys)]."""
    r = ctx.rng
    names = sorted(set(bytes(r.choice(b'ABCD') for _ in range(4)) for _ in range(r.choice([1, 2, 3, 4]))))
    tables = []
    for nm in names:
        nb = r.choice([1, 2, 3, 5, 7])
        t = newbuf(ctx, 0x40)
        w32(ctx, t, nb)
        wb(ctx, t + 0x18, nm)
        buckets = zbuf(ctx, 4 * nb)
        w32(ctx, t + 0x10, buckets)
        keys = []
        for _ in range(r.choice([0, 1, 3, 6])):
            key = struct.pack('<2I', r.randrange(0, 50), r.randrange(0, 50))
            e = newbuf(ctx, 0x20)
            wb(ctx, e, key)
            k0, k1 = struct.unpack('<2i', key)
            v = k0 ^ k1
            b = abs(v) % nb * (1 if v >= 0 else -1)
            if b < 0:
                continue
            w32(ctx, e + 0x1c, r32(ctx.m.uc, buckets + 4 * b))
            w32(ctx, buckets + 4 * b, e)
            keys.append(key)
        tables.append((nm, t, keys))
    arr = zbuf(ctx, 4 * len(tables))
    w32(ctx, arr, *[t for _, t, _ in tables])
    lst = zbuf(ctx, 0x10)
    w32(ctx, lst + 4, len(tables))
    w32(ctx, lst + 8, arr)
    w32(ctx, 0x1017d604, lst)
    return tables


def g_bb2b0(ctx):
    r = begin(ctx)
    tables = name_tables(ctx)
    nm, t, keys = r.choice(tables)
    if r.random() < 0.05:
        nm = b'ZZZZ'          # no table: the original divides by the dword at 0 (both fault)
    key = r.choice(keys) if keys and r.random() < 0.7 else struct.pack('<2I', r.randrange(0, 50), r.randrange(0, 50))
    return [ctx.buf(16, key + rnd(ctx, 8)), cstr(ctx, nm)]


def g_bc47e(ctx):
    r = begin(ctx)
    wb(ctx, 0x1017da18, bytes(r.choice(b'abc XYZ') for _ in range(r.randrange(0, 30))) + b'\0')
    w32(ctx, 0x1017b8f0, ctx.u32())
    return [ctx.u32()]


def g_bc535(ctx):
    r = begin(ctx)
    wb(ctx, 0x1017da18, bytes(r.choice(b'abc XYZ\n') for _ in range(r.randrange(0, 30))) + b'\0')
    w32(ctx, 0x1017de18, ctx.u32())
    return [ctx.u32()]


def g_bd69c(ctx):
    r = begin(ctx)
    if r.random() < 0.08:
        return [0, CALLBACK]
    obj = newbuf(ctx, 0x10)
    n = r.choice([0, 1, 2, 5, 8])
    arr = 0
    if r.random() < 0.9:
        arr = ctx.buf(4 * max(n, 1), struct.pack('<%dI' % max(n, 1), *[r.choice([0, ctx.u32() | 1]) for _ in range(max(n, 1))]))
    w32(ctx, obj + 4, n if r.random() < 0.9 else r.choice([0xffffffff, 0x80000000]))
    w32(ctx, obj + 8, arr)
    return [obj, r.choice([0, CALLBACK])]


def game_obj(ctx):
    """An object for the 0x100beced/0x100c0xxx family with the fields they and their helpers read."""
    r = ctx.rng
    obj = newbuf(ctx, 0x600)
    own = newbuf(ctx, 0xc00)
    w32(ctx, obj, r.choice([1, 1, 1, 1, 0, 2, 9]))
    w32(ctx, obj + 0x28, own)
    w32(ctx, own, obj)
    w32(ctx, own + 0xbdb, r.choice([5, 0, 0, 1, 3, ctx.u32()]))
    rets(0x10033eb3, r.choice([0, 0, 1, 3]))      # Sim_100c4da3: 1 when this count and the out dword are non-zero
    w32(ctx, own + 0x4f4, r.choice([0, 0, 1, ctx.u32()]))
    w32(ctx, own + 0x514, r.choice([0, 0, 1, 5, 0x13, 0x15, 0xffffffff]))
    w32(ctx, obj + 0x264, r.choice([0, 0, 1]))
    w16(ctx, obj + 0x1f8, r.choice([0, 10, 0xffff, 3]))
    kind = r32(ctx.m.uc, obj)
    w32(ctx, obj + 0x27c, r.choice([0x10151b40, 0x10151ae0 + 8 * kind, 0x10151ae0 + 8 * r.randrange(0, 12),
                                    0x10151ae0 + 8 * r.randrange(0, 12)]))
    w16(ctx, obj + 0x1fa, r.choice([0xffff, 0xffff, 0xffff, 0, 1, 3]))
    w16(ctx, obj + 0x1aa, r.choice([0x200 | r.randrange(0, 8), 0x200 | r.randrange(0, 8), r.randrange(0, 8), r.getrandbits(16)]))
    w16(ctx, obj + 0x1a8, r.getrandbits(16))
    for i in range(256):
        if i < 8 or r.random() < 0.3:
            ent = newbuf(ctx, 0x40)
            w32(ctx, ent, r.choice([0, 1, 2, 3, 5, 9, 12, ctx.u32()]))
            w32(ctx, ent + 0x28, newbuf(ctx, 0x600))
            w32(ctx, 0x109e8b40 + 4 * i, ent)
        else:
            w32(ctx, 0x109e8b40 + 4 * i, 0)
    blk = newbuf(ctx, 0x80)
    wb(ctx, blk + 0x59, bytes([r.choice([0, 0, 1, 0x80])]))
    w32(ctx, obj + 0x54, blk)
    w32(ctx, obj + 0x1fc, r.choice([0xffffffff, 0xffffffff, 0xfffffffe, 0x1234, 0]))
    w32(ctx, obj + 0xd4, r.choice([rfl(ctx, -2000, 2000), rfl(ctx, 4000, 9000), fbits(5000.0)]))
    # angles: Sim_10015266 returns +0xe8 (or +0x110 for the player's object) minus +0x68, wrapped into -180..180
    a = r.uniform(-400, 400)
    w32(ctx, obj + 0x68, fbits(a))
    for off in (0xe8, 0x110):
        w32(ctx, obj + off, r.choice([fbits(a + r.uniform(-1.5, 1.5)), fbits(a), fbits(r.uniform(-400, 400)), rfl(ctx)]))
    # the random ring of Sim_1009c929
    w32(ctx, 0x101cffc0, *[r.getrandbits(31) for _ in range(0x80)])
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    return obj


def g_beced(ctx):
    begin(ctx)
    return [game_obj(ctx)]


SPECS_B = []


# ---- generators: 0x100c ------------------------------------------------------------------------------------
def g_c0(ctx):
    begin(ctx)
    return [game_obj(ctx)]


def g_c0w(ctx):
    begin(ctx)
    return [game_obj(ctx), ctx.rng.choice([0, 1, -1, 0x100, 0x7fff, ctx.u32()]) & 0xffffffff]


def spec(name, orig, gen, trials, **kw):
    d = dict(name=name, orig=orig, c=name, gen=gen, trials=trials, api=API)
    d.update(kw)
    return d


SPECS = [
    spec('Sim_1009cc82', 0x1009cc82, g_cc82, 500, void=True, fp=True),
    spec('Sim_1009cdd5', 0x1009cdd5, g_cdd5, 600, void=True, fp=True),
    spec('Sim_1009ceab', 0x1009ceab, g_ceab, 600, void=True, fp=True),
    # spec('Cd_Open', 0x1009e1ef, g_cd_open, 600),   # deleted: no CD audio (DECISIONS.md "dropped features")
    # spec('Cd_OpenDoor', 0x1009e39d, g_cd_door, 300, void=True),   # deleted: no CD audio (DECISIONS.md "dropped features")
    spec('Sim_100a2823', 0x100a2823, g_a2823, 400),
    spec('Sim_100a5336', 0x100a5336, g_a5336, 300),
    spec('Sim_100a7880', 0x100a7880, g_a7880, 400),
    spec('Sim_100a7f8c', 0x100a7f8c, g_a7f8c, 800),
    spec('Sim_100a8c24', 0x100a8c24, g_a8c24, 400, void=True, fp=True),
    spec('Sim_100a96d8', 0x100a96d8, g_a96d8, 400, void=True),
    spec('Sim_100a9830', 0x100a9830, g_a9830, 600),
    spec('Sim_100a9a1a', 0x100a9a1a, g_a9a1a, 400),
    spec('Sim_100a9e1a', 0x100a9e1a, g_a9e1a, 300, void=True),
    spec('Sim_100aa8cb', 0x100aa8cb, g_aa8cb, 600, void=True),
    spec('Sim_100aae7b', 0x100aae7b, g_aae7b, 600, void=True),
    spec('Sim_100aaf32', 0x100aaf32, g_aaf32, 500, void=True),
    spec('Sim_100ab186', 0x100ab186, g_ab186, 400, void=True),
    spec('Sim_100ac21a', 0x100ac21a, g_ac21a, 800, fp=True),
    spec('Sim_100ac620', 0x100ac620, g_ac620, 600, void=True),
    spec('Sim_100ad87d', 0x100ad87d, g_ad87d, 500, void=True),
    spec('Sim_100b5ad7', 0x100b5ad7, g_b5ad7, 600),
    spec('Sim_100b65b7', 0x100b65b7, g_b65b7, 600),
    spec('Sim_100bb2b0', 0x100bb2b0, g_bb2b0, 600),
    spec('Sim_100bb350', 0x100bb350, g_bb2b0, 600),
    spec('Sim_100bc47e', 0x100bc47e, g_bc47e, 300, void=True),
    spec('Sim_100bc535', 0x100bc535, g_bc535, 300),
    spec('Sim_100bd69c', 0x100bd69c, g_bd69c, 400, void=True),
    spec('Sim_100beced', 0x100beced, g_beced, 1500),
    spec('Sim_100bf186', 0x100bf186, g_beced, 800),
    spec('Sim_100c0423', 0x100c0423, g_c0, 500, fp=True),
    spec('Sim_100c0471', 0x100c0471, g_c0, 600, fp=True),
    spec('Sim_100c0ac4', 0x100c0ac4, g_c0w, 500, fp=True),
    spec('Sim_100c0fc4', 0x100c0fc4, g_c0w, 500, fp=True),
]
