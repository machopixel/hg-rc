# Differential tests for round 3, letter r (see difftest.py). Floating-point functions are built
# with SSE2 doubles like Diego's build and carry fp=True (worst deviation printed).
# Stubs: indirect-call targets (plugin Render/SetLumaTable slots, callbacks, driver methods, object
# methods, the polygon callback) are `ret` instructions at fixed addresses with a code hook that logs
# the arguments into a buffer in argument memory (compared afterwards, ints NaN-tagged so the float
# tolerance cannot hide a difference). The same stub addresses serve the original and the C run.
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

SOURCES = ['sim/view/w3r_matrix.c', 'sim/view/w3r_camera.c', 'sim/view/matrix.c',
           'sim/render/w3r_screen.c', 'sim/render/w3r_draw.c',
           'sim/unsorted_1001/w3r_misc.c', 'sim/core/w3r_wait.c', 'sim/core/timer.c',
           'sim/unsorted_1002/w3r_table.c', 'sim/unsorted_1002/w2i_misc.c',
           'sim/unsorted_1002/w2e_tables.c', 'sim/unsorted_100f/w2e_string.c',
           'sim/unsorted_1003/w3r_mesh.c', 'sim/unsorted_1003/w3r_misc.c',
           'sim/unsorted_1005/w3r_misc.c', 'sim/unsorted_1007/w3r_objects.c',
           'sim/unsorted_1009/w2i_state.c', 'sim/unsorted_1008/w3r_grid.c',
           'sim/unsorted_1008/w2l_grid.c', 'sim/unsorted_100d/w3r_records.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STUB = 0x00F00400
NSTUB = 8
LOGCAP = 0x20000
ST = {}
TIMER_ORIG = 0x100e45e7


def w32(ctx, addr, v):
    ctx.m.uc.mem_write(addr, struct.pack('<I', v & 0xffffffff))

def wf(ctx, addr, f):
    ctx.m.uc.mem_write(addr, struct.pack('<f', f))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]

def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def rfloat(r, spread=100.0, special=0.03):
    c = r.random()
    if c < special / 3: return float('nan')
    if c < special * 2 / 3: return r.choice([float('inf'), float('-inf')])
    if c < special: return r.choice([0.0, -0.0, 1e30, -1e30, 1e-30])
    if c < 0.25: return r.choice([0.0, 1.0, -1.0, 0.5, 2.0, 16.0])
    return r.uniform(-spread, spread)

def f32w(ctx, addr, f):
    ctx.m.uc.mem_write(addr, struct.pack('<f', f) if not (abs(f) > 3.4e38 and f == f and abs(f) != float('inf')) else struct.pack('<f', math.copysign(3e38, f)))


def emit(uc, items):
    log = ST['log']
    pos = r32(uc, log)
    out = bytearray()
    for v in items:
        v &= 0xffffffff
        for s in (0, 8, 16, 24):
            out += struct.pack('<I', 0x7fc00000 | ((v >> s) & 0xff))
    if pos + len(out) > LOGCAP:
        return
    uc.mem_write(log + 4 + pos, bytes(out))
    uc.mem_write(log, struct.pack('<I', pos + len(out)))

def read_or(uc, addr, n):
    try:
        return bytes(uc.mem_read(addr, n))
    except Exception:
        return None


def _hook(m):
    uc = m.uc
    csym = m.syms.get('Timer_Elapsed')

    def cb(uc, address, size, ud):
        try:
            esp = uc.reg_read(UC_X86_REG_ESP)
            if address == TIMER_ORIG or address == csym:
                step = ST.get('bump', 0)
                if step:
                    t = r32(uc, esp + 4)
                    ca = 0x10153380 if t & 0x80 else 0x10153384
                    uc.mem_write(ca, struct.pack('<I', (r32(uc, ca) + step) & 0xffffffff))
                return
            k = (address - STUB) // 4
            a = struct.unpack('<8I', bytes(uc.mem_read(esp + 4, 32)))
            ret = ST['ret'].get(k, 0)
            if k == 0:       # plugin Render (mode, surface, verts, count, tex**)
                items = [1, a[0], a[1], a[3]]
                n = max(0, min(a[3], 40))
                d = read_or(uc, a[2], n * 36)
                items += [0xBADBAD] if d is None else list(struct.unpack('<%dI' % (len(d) // 4), d))
                try:
                    h = r32(uc, a[4])
                    items += list(struct.unpack('<4I', bytes(uc.mem_read(h, 16))))   # width, height, pixels, palette (key not set by the caller)
                except Exception:
                    items += [0xBADBAD]
                emit(uc, items)
            else:
                emit(uc, [10 + k] + list(a[:ST.get('nargs', {}).get(k, 2)]))
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB, STUB + 4 * NSTUB)
    uc.hook_add(UC_HOOK_CODE, cb, None, TIMER_ORIG, TIMER_ORIG)
    if csym:
        uc.hook_add(UC_HOOK_CODE, cb, None, csym, csym)
    uc.mem_write(STUB, b'\xc3' * (4 * NSTUB))


def stub(k):
    return STUB + 4 * k


def begin(ctx, nargs=None):
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w3r', False):
        _hook(m)
        m._w3r = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    ST['ret'] = {k: r.choice([0, 1, r.getrandbits(32)]) for k in range(NSTUB)}
    ST['nargs'] = {1: 1}
    ST['nargs'].update(nargs or {})
    ST['bump'] = 0
    w32(ctx, 0x109e3d30, stub(0))
    w32(ctx, 0x109e3d24, stub(1))


def fill(ctx, addr, n):
    ctx.m.uc.mem_write(addr, bytes(ctx.rng.getrandbits(8) for _ in range(n)))


def setptab(ctx, entries):
    for i, v in enumerate(entries):
        w32(ctx, 0x109e8b40 + 4 * i, v)


# ---- generators -------------------------------------------------------------
def g_10019670(ctx):
    r = ctx.rng
    begin(ctx)
    n = 4
    idx = r.choice([0, 1, 2, 3, 3])
    w32(ctx, 0x1017b8c8, idx if r.random() < 0.97 else r.getrandbits(32))
    ents = []
    for i in range(n):
        arr = ctx.buf(8 * 0x128 + 16)
        data = ctx.buf(0x100)
        w32(ctx, data + 0x84, arr if r.random() < 0.95 else 0)
        gear = ctx.buf(0x40)
        w32(ctx, gear + 0x28, data if r.random() < 0.95 else r.choice([0, 0x1234]))
        ents.append(gear)
    setptab(ctx, ents)
    return []


def g_1002986c(ctx):
    r = ctx.rng
    begin(ctx, nargs={2: 1, 3: 1})
    fill(ctx, 0x109eeb00, 0x80 * 0x30)
    fill(ctx, 0x1015dc48, 20)
    for i in range(0x80):
        e = 0x109f09e0 + i * 0x14
        fill(ctx, e, 0x14)
        if r.random() < 0.5: w32(ctx, e + 4, 0)
        w32(ctx, e + 0x10, r.choice([0, stub(2), stub(2), stub(3)]))
    return []


def g_1002ab74(ctx):
    r = ctx.rng
    begin(ctx)
    ST['bump'] = r.choice([1, 7, 30, 100, 0x7fffffff, 0x90])
    t = r.choice([0, 1, 5, 62, 0x80, 0x81, 0x85, 0xbe, r.randrange(0, 0x40), r.randrange(0x80, 0xc0)])
    w32(ctx, 0x101354dc, t)
    for base in (0x10153180, 0x10153280):
        wb(ctx, base, struct.pack('<64I', *[r.getrandbits(32) for _ in range(64)]))
    w32(ctx, 0x10153380, r.getrandbits(32))
    w32(ctx, 0x10153384, r.getrandbits(32))
    return []


def vtx_block(ctx, n):
    buf = ctx.buf(0x18 + max(n, 1) * 0x58 + 16)
    for i in range(n):
        o = buf + 0x18 + i * 0x58
        for k in range(3):
            f32w(ctx, o + 0xc + 4 * k, rfloat(ctx.rng, 500.0, 0.05))
    return buf


def g_10030228(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 2, 3, 5, 8, 12])
    obj = vtx_block(ctx, 12)
    part = ctx.buf(0x40 + 16)
    off = r.choice([0x20, 0x20, 0x30])
    wb(ctx, part + 2, struct.pack('<H', n))
    w32(ctx, part + 4, off)
    wb(ctx, part + off, bytes(r.randrange(0, 12) if r.random() < 0.97 else r.getrandbits(8) for _ in range(16)))
    outs = [ctx.buf(8) for _ in range(9)]
    a = [obj, part] + outs
    if r.random() < 0.08:
        a[r.randrange(0, 11)] = 0
    return a


def g_10033744(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 2, 3, 5, 9, -2, 20])
    block = vtx_block(ctx, 20)
    wb(ctx, block + 4, struct.pack('<h', n))
    holder = ctx.buf(0x60)
    w32(ctx, holder + 0x30, block if r.random() < 0.95 else 0)
    obj = ctx.buf(0x60)
    w32(ctx, obj + 0x18, holder if r.random() < 0.95 else 0)
    for off in (4, 8, 0xc, 0x20, 0x24, 0x28):
        f32w(ctx, obj + off, rfloat(r, 100.0, 0.04))
    return [obj if r.random() < 0.97 else 0]


def g_10033e6f(ctx):
    r = ctx.rng
    begin(ctx)
    a, b = ctx.buf(16), ctx.buf(16)
    v = r.choice([0.0, 1.0, -1.0, 2.5, float('nan'), float('inf')])
    f32w(ctx, a + 4, v if r.random() < 0.4 else rfloat(r, 10, 0.05))
    f32w(ctx, b + 4, v if r.random() < 0.4 else rfloat(r, 10, 0.05))
    return [a, b]


def g_10037c6c(ctx):
    r = ctx.rng
    begin(ctx, nargs={2: 0, 3: 0})
    for g, off in ((0x1015e68c, 0x14), (0x1015e688, 0x10)):
        if r.random() < 0.8:
            o = ctx.buf(0x40)
            w32(ctx, o + off, stub(2 if off == 0x14 else 3))
            w32(ctx, g, o)
        else:
            w32(ctx, g, 0)
    w32(ctx, 0x1013a2c8, r.getrandbits(32))
    return []


def g_1003c970(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1013abc8, r.choice([0, 24, -24, 0x7fffffff, 0x80000000, r.getrandbits(32), r.randrange(-1000, 1000)]))
    w32(ctx, 0x1013abb8, r.choice([0xffffffff, 0, 1, 2, 3, 3, 0xffffffff]) if r.random() < 0.97 else r.getrandbits(32))
    fill(ctx, 0x1015eba0, 16)
    w32(ctx, 0x1013abd0, r.getrandbits(32))
    f32w(ctx, 0x1013abc0, rfloat(r, 1000, 0.05))
    return []


def g_1003cc73(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 3, 6, -2])
    ents = []
    for i in range(6):
        data = ctx.buf(0x520)
        f32w(ctx, data + 0x4ec, rfloat(r, 100, 0.08))
        gear = ctx.buf(0x40)
        w32(ctx, gear + 0x28, data)
        ents.append(gear)
    setptab(ctx, ents)
    w32(ctx, 0x1015dc64, n)
    return []


def proj_globals(ctx):
    r = ctx.rng
    for a in (0x10185bd4, 0x10185bf4, 0x10185bd0, 0x10185bec, 0x10185bf0, 0x10185be0):
        w32(ctx, a, r.choice([0, 160, 240, 320, 639, 479, r.randrange(-100, 700), r.getrandbits(32)]) if r.random() < 0.3 else r.randrange(-50, 700))
    for a in (0x10185be4, 0x10185be8):
        f32w(ctx, a, r.choice([0.0, 1.0, 160.0, 240.0, -3.0, r.uniform(0.1, 400.0)]))


def g_10052e60(ctx):
    r = ctx.rng
    begin(ctx)
    proj_globals(ctx)
    p = ctx.buf(0x80)
    for off in (0x58, 0x64, 0x70):
        f32w(ctx, p + off, rfloat(r, 50, 0.04))
    return [r.randrange(-100, 700), r.randrange(-100, 500), p]


def g_10052f3a(ctx):
    r = ctx.rng
    begin(ctx)
    proj_globals(ctx)
    p = ctx.buf(0x80)
    for off in (0x58, 0x64, 0x70):
        f32w(ctx, p + off, rfloat(r, 50, 0.04))
    return [r.choice([r.randrange(-100, 500), r.getrandbits(32)]), p]


def cam_globals(ctx):
    r = ctx.rng
    for a, v in ((0x10185c48, 0), (0x10185c44, 0), (0x10185c4c, 0)):
        f32w(ctx, a, r.choice([0.0, r.uniform(-50, 50)]))
    m = [r.uniform(-1, 1) for _ in range(9)] if r.random() < 0.5 else [1, 0, 0, 0, 1, 0, 0, 0, 1]
    wb(ctx, 0x10185bfc, struct.pack('<9f', *m))


def g_10090099(ctx):
    r = ctx.rng
    begin(ctx)
    v = ctx.buf(16)
    if r.random() < 0.7:        # a plain camera so that many points land inside the viewport
        for a, val in ((0x10185bd4, 320), (0x10185bf4, 240), (0x10185bd0, 0), (0x10185bec, 639), (0x10185bf0, 0), (0x10185be0, 479)):
            w32(ctx, a, val)
        for a in (0x10185c48, 0x10185c44, 0x10185c4c):
            f32w(ctx, a, r.choice([0.0, r.uniform(-20, 20)]))
        wb(ctx, 0x10185bfc, struct.pack('<9f', 1, 0, 0, 0, 1, 0, 0, 0, r.choice([1, 1, -1])))
        f32w(ctx, 0x1036f1d8, r.choice([1.0, 2.0, 0.5, r.uniform(0.5, 4)]))
        f32w(ctx, v, r.uniform(-400, 400))
        f32w(ctx, v + 4, r.uniform(-300, 300))
        f32w(ctx, v + 8, r.choice([r.uniform(1, 500), r.uniform(-50, 500)]))
        return [v]
    proj_globals(ctx)
    cam_globals(ctx)
    f32w(ctx, 0x1036f1d8, r.choice([0.0, 1.0, -1.0, r.uniform(0.01, 200.0)]))
    for k in range(3):
        f32w(ctx, v + 4 * k, rfloat(r, r.choice([10, 300, 5000]), 0.04))
    return [v]


def g_10087b00(ctx):
    r = ctx.rng
    begin(ctx)
    f32w(ctx, 0x1014b8e0, 0.0)
    return [fbits(rfloat(r, 5000, 0.08)), fbits(rfloat(r, 5000, 0.08))]


def g_100883a2(ctx):
    begin(ctx)
    fill(ctx, 0x1017b900, 0x1000)
    fill(ctx, 0x1036f2a0, 0x170)
    return []


def g_100a78d9(ctx):
    r = ctx.rng
    begin(ctx, nargs={2: 1})
    w32(ctx, 0x1017d15c, stub(2))
    return [r.getrandbits(32)]


def objs_7(ctx, nargs_k=2):
    r = ctx.rng
    for a in (0x109e8928, 0x109e8938, 0x109e890c, 0x109e88e8, 0x109e894c, 0x109e8950, 0x109e8954):
        o = ctx.buf(0x140)
        w32(ctx, o + 0x11c, stub(2 + (a & 3)))
        w32(ctx, a, o)
    fill(ctx, 0x1096fc40, 0x20)
    fill(ctx, 0x1096fbec, 4)
    fill(ctx, 0x1096fc1c, 8)
    for a in (0x101355b8, 0x10144fac, 0x1015d1cc, 0x1015d098):
        w32(ctx, a, r.getrandbits(32))
    w32(ctx, 0x1015d1ac, r.choice([0, 1, 2, 3, 5, r.getrandbits(32)]))
    for a in range(0x1096fc40, 0x1096fc5c, 4):
        w32(ctx, a, r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))
    w32(ctx, 0x1096fc20, r.choice([0, 1, 2, 3, 4, 5, r.getrandbits(32)]))


def g_10077ecd(ctx):
    r = ctx.rng
    begin(ctx)
    objs_7(ctx)
    w32(ctx, 0x10179d20, r.choice([0, 1, 1, r.getrandbits(32)]))
    return []


def g_10077fff(ctx):
    begin(ctx)
    objs_7(ctx)
    return []


def g_100d1e10(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015cff0, ctx.buf(0x2000) if r.random() < 0.95 else 0)
    w32(ctx, 0x1017e610, r.choice([0, 0, 1]))
    w32(ctx, 0x1017b940, r.choice([0, 1, 3]))
    fill(ctx, 0x10185c60, 0x2000)
    n = r.choice([0, 1, 3, 4, 6, 12, 40, 100])
    verts = ctx.buf(max(n, 1) * 36 + 8)
    pixels = ctx.buf(0x400)
    w_ = r.choice([0x100, 0x80, 7, -3, 0x1ff, 0x10000 + 5, r.randrange(-50, 700)])
    h_ = r.choice([0x100, 0x200, 5, -4, 0x20, r.randrange(-50, 700), 0x18000 + 9])
    return [r.getrandbits(32), pixels, w_ & 0xffffffff, h_ & 0xffffffff, n, verts, r.getrandbits(32), r.getrandbits(32)]


def g_10054d98(ctx):
    r = ctx.rng
    begin(ctx, nargs={2: 2})
    slot = r.randrange(0, 4)
    p = ctx.buf(0x60)
    fill(ctx, p, 0x60)
    def flagptr():
        c = r.random()
        if c < 0.25: return 0
        return ctx.buf(4, bytes([r.choice([0, 1, 0xff, 0x80, r.getrandbits(8)]), 0, 0, 0]))
    def w32u(off, v): wb(ctx, p + off, struct.pack('<I', v & 0xffffffff))
    w32u(0x2a, flagptr()); w32u(0x2e, flagptr()); w32u(0x22, flagptr()); w32u(0x26, flagptr())
    acc = ctx.buf(8, struct.pack('<i', r.choice([0, 1, -1, 0x7ff0, -0x7ff0, 0x8000, -0x8000, r.randrange(-0x20000, 0x20000)])) + b'\0' * 4)
    w32u(4, acc)
    fl = ctx.buf(8)
    f32w(ctx, fl, rfloat(r, 40000.0, 0.03))
    w32u(0, fl)
    w32u(0x10, r.choice([0, r.getrandbits(32), r.randrange(-300, 300)]))
    w32u(0x1a, r.choice([0, r.getrandbits(32), 0xffff0000, 0x10000, -0x10000 + 1, r.randrange(-0x20000, 0x20000)]))
    w32u(0xc, r.choice([0, r.getrandbits(32), r.randrange(-3000, 3000)]))
    w32u(8, r.choice([0, 1, r.randrange(-3000, 3000), r.randrange(1, 100)]))
    w32u(0x1e, r.choice([0, 1, 2, 5, 31, 32, 33, r.getrandbits(32)]))
    wb(ctx, p + 0x14, bytes([r.choice([0, 1, 4, 16, 31, 32, 0xff, r.getrandbits(8)])]))
    wb(ctx, p + 0x15, bytes([r.choice([0, 0, 0, 1, 0xff, 5])]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 3, 100, 0x7fff, r.getrandbits(32), r.randrange(0, 5000)]))
    param = ctx.buf(0x30)
    wb(ctx, param, struct.pack('<6I', slot, r.getrandbits(32), r.choice([0, 1, 1]), 0, 0, p))
    for s in range(4):
        o = ctx.buf(0x40)
        w32(ctx, o + 0x10, stub(2))
        w32(ctx, 0x1015fd20 + 4 * s, o)
        w32(ctx, 0x1015f8f0 + 0x74 * s, r.getrandbits(32))
    return [param]


def g_1005b98c(ctx):
    r = ctx.rng
    begin(ctx)
    nent = 4
    index = r.randrange(0, nent)
    player = r.randrange(0, nent)
    pid = r.getrandbits(32)
    ents = []
    for i in range(nent):
        recs = ctx.buf(0x11 * 0x3c + 16)
        for k in range(0x11):
            o = recs + k * 0x3c
            f32w(ctx, o, rfloat(r, 20000.0, 0.03))
            f32w(ctx, o + 8, r.choice([rfloat(r, 20000.0, 0.03), r.uniform(0, 3000), 1.0]))
            w32(ctx, o + 0x38, r.getrandbits(32))
        sub = ctx.buf(0x40)
        w32(ctx, sub + 0x14, r.choice([0, 1, 1, 2]))
        data = ctx.buf(0x100)
        w32(ctx, data, sub)
        w32(ctx, data + 0x88, recs)
        gear = ctx.buf(0x40)
        w32(ctx, gear + 8, pid if (i in (index, player) and r.random() < 0.8) else r.getrandbits(32))
        w32(ctx, gear + 0x28, data)
        ents.append(gear)
    setptab(ctx, ents)
    w32(ctx, 0x1017b8c8, player)
    cfg = ctx.buf(16)
    wb(ctx, cfg + 5, bytes([r.choice([0, 1, 2])]))
    w32(ctx, 0x1017b8c4, cfg)
    w32(ctx, 0x1015ec58, r.choice([0, 0, 1]))
    return [index]


def mat(ctx, n=12, special=0.04):
    r = ctx.rng
    return ctx.buf(n * 4 + 16, struct.pack('<%df' % n, *[max(-3e38, min(3e38, x)) if x == x and abs(x) != float('inf') else x for x in [rfloat(r, 10, special) for _ in range(n)]]))


def g_5fbb0(ctx):
    r = ctx.rng
    begin(ctx)
    m = mat(ctx)
    p = mat(ctx, 3)
    if r.random() < 0.5:
        return [m, p, p + 4, p + 8]
    return [m, mat(ctx, 1), mat(ctx, 1), mat(ctx, 1)]


def g_5fcdc(ctx):
    r = ctx.rng
    begin(ctx)
    return [mat(ctx), r.choice([0, 1, 2, 0, 1, 2, 3, 0xffffffff, 7])]


def g_5fe48(ctx):
    r = ctx.rng
    begin(ctx)
    a, b = mat(ctx, 12), mat(ctx, 12)
    c = r.random()
    return [a, b, a if c < 0.15 else b if c < 0.3 else mat(ctx, 12)]


def g_60101(ctx):
    r = ctx.rng
    begin(ctx)
    a = mat(ctx)
    return [a, a if r.random() < 0.2 else mat(ctx)]


def g_100b403d(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x101507f8, r.choice([0xffffffff, 0, 1, 2, 5, 0xffffffff]))
    for i in range(6):
        wb(ctx, 0x101a11c8 + i * 0x7c, struct.pack('<12f', *[r.uniform(-5, 5) for _ in range(12)]))
    return [mat(ctx, 3)]


def g_100d2306(ctx):
    r = ctx.rng
    begin(ctx)
    cam = ctx.buf(0x100)
    f32w(ctx, cam + 0x18, r.choice([0.1, 0.5, 1.0, 2.0, 16.0, 17.0, 100.0, float('nan'), float('inf'), -3.0, 0.4999, 8.0, r.uniform(0, 30)]))
    f32w(ctx, cam + 0x44, r.choice([1.0, 0.75, 1.33, 100.0, -1.0, 0.0, r.uniform(0.1, 5)]))
    vp = r.choice([(0, 639, 0, 479), (0, 319, 0, 239), (10, 11, 3, 3), (5, 5, 7, 7), (r.randrange(-50, 700), r.randrange(-50, 700), r.randrange(-50, 500), r.randrange(-50, 500)), (0x7fffffff, 0x80000000, 0x7ffffff0, -0x7ffffff0)])
    wb(ctx, cam + 0x2c, struct.pack('<4i', *[(x + 2**31) % 2**32 - 2**31 for x in vp]))
    wb(ctx, 0x1036f860, struct.pack('<800f', *[r.uniform(0, 3) for _ in range(800)]))
    wb(ctx, 0x10370d20, struct.pack('<800f', *[r.uniform(-1, 1) for _ in range(800)]))
    w32(ctx, 0x10152418, r.choice([0, 1, -5, 3, 160, r.randrange(-3, 400)]))
    return [cam]


def g_100d280d(ctx):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1015ef0c, r.choice([0, 0, 1]))
    cam_ = ctx.buf(0x100)
    f32w(ctx, cam_ + 0x18, r.uniform(0.2, 3))
    f32w(ctx, cam_ + 0x40, r.choice([100.0, 1000.0, 50.0, r.uniform(1, 2000)]))
    f32w(ctx, cam_ + 0xa8, r.choice([1.0, 0.5, 2.0, 0.0, r.uniform(0.1, 3)]))
    f32w(ctx, cam_ + 0xac, r.choice([1.0, 0.5, 2.0, r.uniform(0.1, 3)]))
    f32w(ctx, cam_ + 0xb0, r.uniform(0.2, 3))
    w32(ctx, 0x1014cae0, cam_)
    for a in range(0x10185c20, 0x10185c54, 4):
        f32w(ctx, a, r.uniform(-1, 1) if r.random() < 0.9 else rfloat(r, 2, 0.5))
    f32w(ctx, 0x10185c50, r.choice([0.5, 1.0, 10.0, -5.0]))
    f32w(ctx, 0x10185bf8, r.choice([1000.0, 100.0, 5000.0, 10.0]))
    for a in (0x10185c48, 0x10185c44, 0x10185c4c):
        f32w(ctx, a, r.choice([0.0, r.uniform(-100, 100)]))
    obj = ctx.buf(0xc0)
    sp = r.choice([10.0, 200.0, 2000.0])
    for off in (0x44, 0x48, 0x4c):
        f32w(ctx, obj + off, rfloat(r, sp, 0.02))
    f32w(ctx, obj + 0x50, r.choice([0.0, 1.0, 5.0, 50.0, rfloat(r, 100, 0.03)]))
    wb(ctx, obj + 2, struct.pack('<H', r.choice([0xa0, 0xa5, 0x10, r.getrandbits(16)])))
    wb(ctx, obj + 0xa0, bytes([r.choice([0x57, 0x57, 0x41, r.getrandbits(8)])]))
    return [obj]


def g_100d867b(ctx):
    r = ctx.rng
    begin(ctx)
    fill(ctx, 0x10180a00, 9 * 0x80c)
    src = ctx.buf(0x100)
    ints = [r.choice([0, 1, -1, 16777217, 0x7fffffff, 0x80000000, r.getrandbits(32), r.randrange(-1000, 1000)]) for _ in range(0x100 // 4)]
    wb(ctx, src, struct.pack('<%dI' % len(ints), *[x & 0xffffffff for x in ints]))
    w32(ctx, src + 4, r.getrandbits(11) if r.random() < 0.8 else r.choice([0x7ff, 0, 0x4]))
    return [src, r.choice([0, 1, 4, 8, r.randrange(-2, 11)])]


def g_100d8936(ctx):
    r = ctx.rng
    begin(ctx)
    fill(ctx, 0x10180a00, 9 * 0x80c)
    f32w(ctx, 0x1012bd64, 181.0)
    n = 0xb4 + 9 * 0xf4 + 16
    src = ctx.buf(n)
    ints = [r.choice([0, 1, -1, 16777217, 0x7fffffff, 0x80000000, r.getrandbits(32), r.randrange(-1000, 1000)]) for _ in range(n // 4)]
    wb(ctx, src, struct.pack('<%dI' % len(ints), *[x & 0xffffffff for x in ints]))
    flags = (r.getrandbits(9) << 16) | r.getrandbits(4) | (r.getrandbits(1) << 0)
    w32(ctx, src, r.choice([0, 1, 2, 2, 3]))
    w32(ctx, src + 4, flags)
    for k in range(9):      # descriptions for Sim_100d867b: flags dword
        w32(ctx, src + 0xb4 + k * 0xf4 + 4, r.getrandbits(11))
    return [src]


def g_floatconst(ctx):
    return g_100883a2(ctx)


def S(name, addr, gen, trials=400, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    S('Sim_10019670', 0x10019670, g_10019670, void=True),
    S('Sim_1002986c', 0x1002986c, g_1002986c, void=True),
    S('Sim_1002ab74', 0x1002ab74, g_1002ab74, void=True),
    S('Sim_10030228', 0x10030228, g_10030228, 600, void=True, fp=True),
    S('Sim_10033744', 0x10033744, g_10033744, 600, fp=True),
    S('Sim_10033e6f', 0x10033e6f, g_10033e6f),
    S('Sim_10037c6c', 0x10037c6c, g_10037c6c, void=True),
    S('Sim_1003c970', 0x1003c970, g_1003c970, void=True, fp=True),
    S('Sim_1003cc73', 0x1003cc73, g_1003cc73, void=True, fp=True),
    S('Sim_10052e60', 0x10052e60, g_10052e60, 600, fp=True),
    S('Sim_10052f3a', 0x10052f3a, g_10052f3a, 600, fp=True),
    S('Sim_10054d98', 0x10054d98, g_10054d98, 1500, fp=True),
    S('Sim_1005b98c', 0x1005b98c, g_1005b98c, 600, void=True, fp=True),
    S('Mat3x4_TransformPoint', 0x1005fbb0, g_5fbb0, 600, void=True, fp=True),
    S('Mat3_CrossColumn', 0x1005fcdc, g_5fcdc, 600, void=True, fp=True),
    S('Mat3_Multiply', 0x1005fe48, g_5fe48, 600, void=True, fp=True),
    S('Mat3x4_Multiply', 0x10060008, g_5fe48, 600, void=True, fp=True),
    S('Mat3x4_InvertRigid', 0x10060101, g_60101, 600, void=True, fp=True),
    S('Sim_10077ecd', 0x10077ecd, g_10077ecd),
    S('Sim_10077fff', 0x10077fff, g_10077fff, void=True),
    S('Sim_10087b00', 0x10087b00, g_10087b00, void=True, fp=True),
    S('Sim_100883a2', 0x100883a2, g_100883a2, 300, void=True),
    S('Sim_10090099', 0x10090099, g_10090099, 800, fp=True),
    S('Sim_100a78d9', 0x100a78d9, g_100a78d9),
    S('Sim_100b403d', 0x100b403d, g_100b403d, void=True, fp=True),
    S('Render_SubmitVerts_100d1e10', 0x100d1e10, g_100d1e10, 500, void=True),
    S('Sim_100d2306', 0x100d2306, g_100d2306, 800, void=True, fp=True),
    S('Sim_100d280d', 0x100d280d, g_100d280d, 1500, fp=True),
    S('Sim_100d867b', 0x100d867b, g_100d867b, 600, void=True, fp=True),
    S('Sim_100d8936', 0x100d8936, g_100d8936, 600, void=True, fp=True),
]
