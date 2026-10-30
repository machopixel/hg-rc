# Round 8B helper h3: EXACT (no fp/tol) re-proof of the game-logic float functions of spec_w3r.py.
# Generators copied from spec_w3r.py; rfloat also yields denormals, signed zeros, quiet NaNs, infinities.
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

SPECIALS = [0.0, -0.0, 1e30, -1e30, 1e-30, 1e-40, -1e-40, 1.4e-45, -1.4e-45, 1.17549435e-38, 3.4e38, -3.4e38]

def rfloat(r, spread=100.0, special=0.03):
    special *= 3          # round 8B: more special values
    c = r.random()
    if c < special / 3: return r.choice([float('nan'), -float('nan')])
    if c < special * 2 / 3: return r.choice([float('inf'), float('-inf')])
    if c < special: return r.choice(SPECIALS)
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


def g_10087b00(ctx):
    r = ctx.rng
    begin(ctx)
    f32w(ctx, 0x1014b8e0, 0.0)
    return [fbits(rfloat(r, 5000, 0.08)), fbits(rfloat(r, 5000, 0.08))]


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


def S(name, addr, gen, trials=3000, **kw):
    d = dict(name=name, orig=addr, c=name, gen=gen, trials=trials)
    d.update(kw)
    return d


SPECS = [
    S('Sim_10030228', 0x10030228, g_10030228, void=True),
    S('Sim_10033744', 0x10033744, g_10033744),
    S('Sim_1003c970', 0x1003c970, g_1003c970, void=True),
    S('Sim_1003cc73', 0x1003cc73, g_1003cc73, void=True),
    S('Sim_10054d98', 0x10054d98, g_10054d98, 5000),
    S('Sim_1005b98c', 0x1005b98c, g_1005b98c, void=True),
    S('Sim_10087b00', 0x10087b00, g_10087b00, void=True),
    S('Sim_100d867b', 0x100d867b, g_100d867b, void=True),
    S('Sim_100d8936', 0x100d8936, g_100d8936, void=True),
]
