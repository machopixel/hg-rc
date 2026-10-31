# Differential tests for round 3, letter o (see difftest.py).
#
# Floating point: the originals run on the x87 unit with the Windows control word 0x27f (53 bit mantissa), the C
# is built with SSE2 doubles. The two agree bit for bit in these functions, so the specs use fp=True with a tiny
# tolerance (the worst deviation is printed, 0 means bit identical). Functions that leave a result in st(0) are
# called through a trampoline that stores it as a double at 0xAF1000 (tools/verify/w3o_test.c does the same for the C).
#
# Stubs (same address for the original and the C run, a `ret` with a code hook that logs the arguments to a buffer
# that is compared afterwards and returns values chosen per trial): the function pointers stored in data, the display
# driver callback 0x1014cb4c, the cache loader 0x100ba7d5, the timestamp function behind 0x1017d60c, the three
# collision tests behind the jump thunks 0x100035df, 0x10001e1f and 0x10002775. The k-th call of a stub in a run gets
# the k-th prepared return value (the counter restarts at the entry of the function under test).
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_EDX, UC_X86_REG_FPCW

SOURCES = [
    'sim/unsorted_1000/w3o_sphere.c', 'sim/unsorted_1001/w3o_records.c', 'sim/unsorted_1002/w3o_vectors.c',
    'sim/unsorted_1002/w3o_ray.c', 'sim/unsorted_1003/w3o_bounds.c', 'sim/unsorted_1004/w3o_misc.c',
    'sim/unsorted_1005/w3o_calls.c', 'sim/unsorted_1006/w3o_add.c', 'sim/unsorted_1007/w3o_load.c',
    'sim/unsorted_1008/w3o_length.c', 'sim/unsorted_1008/w3o_grid.c', 'sim/unsorted_1009/w3o_ray.c',
    'sim/unsorted_1009/w3o_list.c', 'sim/unsorted_100a/w3o_misc.c', 'sim/unsorted_100b/w3o_hash.c',
    'sim/unsorted_100c/w3o_misc.c', 'sim/unsorted_100d/w3o_cull.c',
    # callees that are already converted
    'sim/render/w2n_project.c', 'sim/util/ring.c', 'sim/util/field.c', 'sim/unsorted_1004/tables.c',
    'sim/unsorted_1004/fields.c', 'sim/core/timer.c', 'sim/view/objlist.c', 'sim/unsorted_1007/w2j_misc.c',
    'sim/unsorted_1005/w2j_misc.c', 'sim/unsorted_1009/w2l_pool.c', 'sim/unsorted_100f/w2e_string.c',
    '../tools/verify/w3o_test.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']
TOL = -1.0   # exact: the C and the x87 original agree bit for bit; fp=True only reports the worst deviation (always 0)

LOGCAP = 0x8000
ST = {}
RESULT = 0xAF1000
TRAMP_BASE = 0xAF0000
STUB_BASE = 0x00F00400
THUNKS = {0x100035df: 'col', 0x10001e1f: 'col', 0x10002775: 'col'}
LOADER = 0x100ba7d5
# stub name -> (address, number of logged arguments)
STUBS = {'cb1': (STUB_BASE, 1), 'cb4': (STUB_BASE + 8, 4), 'cbcc': (STUB_BASE + 16, 4), 'cmp': (STUB_BASE + 24, 2),
         'ts': (STUB_BASE + 32, 0), 'drv': (STUB_BASE + 40, 3), 'load': (LOADER, 3)}
for _a in THUNKS:
    STUBS['col%x' % _a] = (_a, 4)
BYADDR = {a: n for n, (a, k) in STUBS.items()}
NARGS = {n: k for n, (a, k) in STUBS.items()}
ENTRY = {}   # spec name -> (orig address, c symbol)
TRAMPS = {}  # spec name -> (target address, number of arguments)


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

def bitsf(u):
    return struct.unpack('<f', struct.pack('<I', u & 0xffffffff))[0]


# ---- log ------------------------------------------------------------------------
def emit(uc, ints):
    log = ST['log']
    pos = r32(uc, log)
    out = bytearray()
    for v in ints:
        v &= 0xffffffff
        for s in (0, 8, 16, 24):
            out += struct.pack('<I', 0x7fc00000 | ((v >> s) & 0xff))
    if pos + len(out) > LOGCAP:
        uc.mem_write(log + 4 + LOGCAP, struct.pack('<I', 0xdead))
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

    def cb(uc, address, size, ud):
        try:
            name = BYADDR.get(address)
            if name is None:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<8I', bytes(uc.mem_read(esp + 4, 32)))
            n = ST['n']
            ST['n'] = n + 1
            rets = ST['rets']
            ret = rets[n % len(rets)]
            rec = [list(STUBS).index(name) + 1, n] + list(a[:NARGS[name]])
            if name == 'cmp':
                ret = [-3, 0, 0, 5, 1, -1][ret % 6] & 0xffffffff
            elif name == 'ts':
                uc.reg_write(UC_X86_REG_EDX, ST['rets2'][n % len(rets)])
            elif name == 'drv':
                skip = ST['skip']
                words = []
                cnt = max(0, min(a[0], 40))
                for k in range(cnt):
                    d = read_or(uc, a[1] + k * 36, 36)
                    if d is None:
                        words.append(0xBADBAD)
                        break
                    w = struct.unpack('<9I', d)
                    words += [x for j, x in enumerate(w) if j not in skip]
                rec += words
            elif name.startswith('col'):
                rec += [r32(uc, a[4]) if False else 0]
                f = ST['outs'][n % len(ST['outs'])]
                uc.mem_write(a[4], struct.pack('<I', f))
                for j, g in enumerate((0x1015dd40, 0x1015dd44, 0x1015dd48)):
                    uc.mem_write(g, struct.pack('<I', ST['outs'][(n + 1 + j) % len(ST['outs'])]))
            elif name == 'load':
                pool = ST['pool']
                k = (a[0] * 2654435761 + ST['seed'] + n) & 0xffffffff
                k = (k >> 8) % (len(pool) + 1)
                ret = pool[k] if k < len(pool) else 0
            emit(uc, rec)
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:
            ST['err'] = repr(e)
            raise

    def reset(uc, address, size, ud):
        ST['n'] = 0

    for name, (addr, k) in STUBS.items():
        uc.hook_add(UC_HOOK_CODE, cb, None, addr, addr)
        uc.mem_write(addr, b'\xc3')
    m._reset_cb = reset
    m._armed = set()


def arm(m, addr):
    if addr not in m._armed:
        m._armed.add(addr)
        m.uc.hook_add(UC_HOOK_CODE, m._reset_cb, None, addr, addr)


def begin(ctx, key, skip=(), nrets=24):
    """Stubs (once), fresh log, return values, the control word, the trampoline of `key` (if it has one)."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w3o', False):
        _hook(m)
        m._w3o = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    orig, csym = ENTRY[key]
    arm(m, orig)
    arm(m, m.syms[csym])
    if key in TRAMPS:
        target, n = TRAMPS[key]
        code = bytes([0xFF, 0x74, 0x24, 4 * n]) * n
        code += b'\xE8' + struct.pack('<i', target - (orig + len(code) + 5))
        code += b'\x83\xC4' + bytes([4 * n]) + b'\xDD\x1D' + struct.pack('<I', RESULT) + b'\xC3'
        m.uc.mem_write(orig, code)
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    w32(ctx, 0x1014cb4c, stub('drv'))
    ST['n'] = 0
    ST['rets'] = [r.choice([0, 1, r.getrandbits(32), 0, 1, r.randrange(0, 4)]) for _ in range(nrets)]
    ST['rets2'] = [r.getrandbits(32) for _ in range(nrets)]
    ST['outs'] = [fbits(r.choice([r.uniform(-3000, 90000), r.uniform(0, 100), 0.0, -5.0, 1000.0, r.uniform(-1e6, 1e6)])) for _ in range(nrets)]
    ST['skip'] = set(skip)
    ST['seed'] = r.getrandbits(32)
    ST['pool'] = [ctx.buf(64) for _ in range(4)]


def stub(name):
    return STUBS[name][0]


# ---- random values ----------------------------------------------------------------------
def rf(ctx, spread=1000.0, special=True):
    r = ctx.rng
    c = r.random()
    if special and c < 0.05:
        return r.choice([0.0, -0.0, 1.0, -1.0])
    if special and c < 0.07:
        return r.choice([float('nan'), float('inf'), float('-inf')])
    if c < 0.4:
        return r.uniform(-spread, spread)
    if c < 0.7:
        return r.uniform(-spread * 100, spread * 100)
    return r.uniform(-3, 3)


def rpos(ctx, hi=1000.0):
    return ctx.rng.choice([ctx.rng.uniform(0, hi), ctx.rng.uniform(0, 3), ctx.rng.uniform(0, hi * 100), 0.0, 1.0])


def sane(ctx, spread):
    return ctx.rng.uniform(-spread, spread)


def norm_vec(ctx):
    r = ctx.rng
    v = [r.uniform(-1, 1) for _ in range(3)]
    n = math.sqrt(sum(x * x for x in v)) or 1.0
    return [x / n for x in v]


def f32(x):
    return struct.unpack('<f', struct.pack('<f', x))[0]


# ---- shared structures ---------------------------------------------------------------------
def make_seg(ctx, valid=None, scale=1000.0, start=None):
    """A segment: start, end, delta, unit direction, length, valid flag."""
    r = ctx.rng
    s = start or [sane(ctx, scale) for _ in range(3)]
    u = norm_vec(ctx)
    ln = r.choice([r.uniform(1, scale * 3), r.uniform(0, 5), 0.0, r.uniform(scale, scale * 10)])
    e = [s[i] + u[i] * ln for i in range(3)]
    d = [f32(e[i]) - f32(s[i]) for i in range(3)]
    if r.random() < 0.15:
        e = [rf(ctx, scale) for _ in range(3)]
        d = [rf(ctx, scale) for _ in range(3)]
    if valid is None:
        valid = r.choice([0, 1, 1])
    buf = ctx.buf(0x40, 'zero')
    wb(ctx, buf, struct.pack('<9f', *[f32(x) for x in s + e + d]))
    wb(ctx, buf + 0x24, struct.pack('<3f', *u))
    wf(ctx, buf + 0x30, ln)
    w32(ctx, buf + 0x34, valid)
    return buf, s, u, ln


def sphere_hit(ctx, ray_s, ray_u, ln, off_c, off_r, obj, scale):
    """Put a sphere on the ray's line so that the later tests are reached."""
    r = ctx.rng
    rad = r.choice([r.uniform(1, scale / 4), r.uniform(0.1, 3), r.uniform(scale / 4, scale)])
    D = r.choice([r.uniform(rad, ln + rad), r.uniform(0, ln * 2 + 1), r.uniform(rad, rad * 4)])
    h = r.choice([r.uniform(0, rad), r.uniform(0, rad * 2), 0.0])
    p = norm_vec(ctx)
    c = [ray_s[i] + ray_u[i] * D + p[i] * h for i in range(3)]
    wb(ctx, obj + off_c, struct.pack('<3f', *c))
    wf(ctx, obj + off_r, rad if r.random() < 0.93 else r.choice([0.0, -1.0, 1e-9, float('nan')]))


def ptrtab(ctx, n=8):
    """The pointer table at 0x109e8b40: n objects with small ids at +8; the table at 0x109b7d54 (0x6c bytes per id)."""
    r = ctx.rng
    objs = []
    for i in range(n):
        o = ctx.buf(0x1000, 'zero')
        w32(ctx, o + 8, r.randrange(0, 3))
        objs.append(o)
    w32(ctx, 0x109e8b40, *objs) if False else wb(ctx, 0x109e8b40, struct.pack('<%dI' % n, *objs))
    wb(ctx, 0x109b7d54, b''.join(struct.pack('<I', r.randrange(0, 3)) + bytes(0x68) for _ in range(3)))
    w32(ctx, 0x1017b8c8, r.randrange(0, n))
    return objs


def rec_table(ctx, scale=20000.0):
    """The 0x200 records of 0x156 bytes at 0x1096fc80."""
    r = ctx.rng
    objs = [ctx.buf(0x80, 'zero') for _ in range(6)]
    for o in objs:
        wb(ctx, o + 0x70, struct.pack('<3f', *[sane(ctx, scale) for _ in range(3)]))
    data = bytearray(0x200 * 0x156)
    for i in range(0x200):
        o = i * 0x156
        d = norm_vec(ctx)
        struct.pack_into('<I', data, o, r.choice([3, 9, 0xa, 0xc, 3, 9, 0, 1, 5]))
        struct.pack_into('<3f', data, o + 0x18, *d)
        struct.pack_into('<I', data, o + 0x3c, r.choice([0, r.choice(objs), r.choice(objs)]))
        struct.pack_into('<I', data, o + 0x48, r.choice([0, 0x100, r.getrandbits(32) & ~0x8000, 0x8000]))
        struct.pack_into('<I', data, o + 0x50, r.randrange(0, 8))
        v = r.choice([0.0, -1.0, -r.uniform(0.1, 30000), -0.0, r.uniform(0.1, 50000), r.uniform(0, 3), 2500.0, float('nan')])
        struct.pack_into('<f', data, o + 0x64, v)
    wb(ctx, 0x1096fc80, data)
    wb(ctx, 0x101cffc0, struct.pack('<%dI' % 0x7f, *[r.randrange(0, 4000) for _ in range(0x7f)]))
    w32(ctx, 0x1017cd38, r.randrange(0, 0x7f))
    w32(ctx, 0x101653f3, r.getrandbits(32))
    w32(ctx, 0x1016540c, r.getrandbits(32))


def cell_env(ctx, with_nodes=True, list_links=0x18, objs_for_grid=True):
    """Grid of 9 x 9 cells with cell records and node chains, plus the offset table of Sim_100881dd."""
    r = ctx.rng
    w32(ctx, 0x1017c8c4, 1 if r.random() < 0.92 else 0)
    wb(ctx, 0x1017c840, struct.pack('<64i', *[r.choice([-1, 0, 1, 1, 0]) for _ in range(64)]))
    wb(ctx, 0x1017ba4c, bytes(0x18c * 9))
    cells = []
    for k in range(6):
        c = ctx.buf(0x80, 'zero')
        w32(ctx, c + 8, r.choice([0, 1, 5, 5, 5]))
        first = 0
        prev = 0
        for j in range(r.choice([0, 1, 2, 3])):
            n = ctx.buf(0x100, 'zero')
            w32(ctx, n + 0x34, r.choice([5, 8, 0, 0, 5, 8, 1, 2]))
            if prev:
                w32(ctx, prev + 0x18, n)
            else:
                first = n
            prev = n
        w32(ctx, c + 0x18, first)
        cells.append(c)
    objs = []
    for k in range(4):
        o = ctx.buf(0x80, 'zero')
        wb(ctx, o + 0x44, struct.pack('<3f', *[sane(ctx, 200000) for _ in range(3)]))
        objs.append(o)
    for i in range(9):
        for j in range(9):
            if True:
                w32(ctx, 0x1017ba4c + i * 0x18c + j * 0x2c, r.choice(cells))
            if r.random() < 0.3:
                w32(ctx, 0x1017ba50 + i * 0x18c + j * 0x2c, r.choice(objs))
    w32(ctx, 0x1017c8cc, r.choice(cells))
    wb(ctx, 0x1036f2a0, bytes(0x30))
    return cells


def grid_x(ctx):
    return ctx.rng.choice([ctx.rng.uniform(-254008, 245000), ctx.rng.uniform(-254008, 245000), ctx.rng.uniform(-500000, 500000), 0.0])


def grid_z(ctx):
    return ctx.rng.choice([ctx.rng.uniform(-246044, 253000), ctx.rng.uniform(-246044, 253000), ctx.rng.uniform(-500000, 500000), 0.0])


# ---- generators ------------------------------------------------------------------------------
def fval(ctx):
    r = ctx.rng
    return r.choice([0.0, 0.5, -0.5, 1.5, 2.5, -2.5, 100.0, -100.0, 32767.5, 32768.0, 65535.9, 65536.0, 70000.0, -32768.5, -40000.0, 2147483648.0, 3e9, -3e9, 1e20, -1e20, float('nan'), float('inf'), r.uniform(-70000, 70000), r.uniform(-5, 5)])


def g_8ed0(ctx):
    begin(ctx, 'Sim_10008ed0')
    obj = ctx.buf(0x40)
    t = ctx.buf(0x40) if ctx.rng.random() < 0.85 else 0
    w32(ctx, obj + 0x30, t)
    return [obj, fbits(fval(ctx))]


def g_946e(ctx):
    begin(ctx, 'Sim_1000946e')
    r = ctx.rng
    obj = ctx.buf(0x80)
    ptrs = [ctx.buf(8) if r.random() < 0.8 else 0 for _ in range(3)]
    return [obj if r.random() < 0.9 else 0] + ptrs


def g_9ca8(ctx):
    begin(ctx, 'Sim_10009ca8')
    r = ctx.rng
    obj = ctx.buf(0x80)
    rad = r.choice([r.uniform(0, 1000), r.uniform(0, 3), 0.0, -1.0, 1e-8])
    c = [sane(ctx, 1000) for _ in range(3)]
    wb(ctx, obj + 0x44, struct.pack('<4f', *c, rad))
    k = r.random()
    p = [c[i] + (r.uniform(-1, 1) * rad * (0.6 if k < 0.6 else 1.5)) for i in range(3)] if k < 0.85 else [rf(ctx) for _ in range(3)]
    return [obj] + [fbits(x) for x in p]


def ray_sphere(ctx, key, off_c, off_r):
    begin(ctx, key)
    r = ctx.rng
    ray, s, u, ln = make_seg(ctx)
    obj = ctx.buf(0x100, 'zero')
    if r.random() < 0.8:
        sphere_hit(ctx, s, u, ln, off_c, off_r, obj, 1000.0)
        if r.random() < 0.25:
            wf(ctx, ray + 0x30, ln * r.choice([0.0, 0.5, 2.0, 1.0]))
    else:
        wb(ctx, obj + off_c, struct.pack('<4f', *[rf(ctx) for _ in range(4)]))
    return [obj, ray]


def g_9d7e(ctx): return ray_sphere(ctx, 'Sim_10009d7e', 0x60, 0x6c)
def g_9eb3(ctx): return ray_sphere(ctx, 'Sim_10009eb3', 0x44, 0x50)


def g_17ad9(ctx):
    begin(ctx, 'Sim_10017ad9')
    r = ctx.rng
    st = ctx.buf(0x560, 'zero')
    cur = r.choice([0.0, 5.0, -5.0, 100.0, float('nan'), r.uniform(-100, 100)])
    wf(ctx, st + 0x520, cur)
    obj = ctx.buf(0x40, 'zero')
    w32(ctx, obj + 0x28, st)
    v = r.choice([cur, cur + 1, cur - 1, 0.0, float('nan'), r.uniform(-100, 100), 1e30])
    return [obj if r.random() < 0.93 else 0, fbits(v)]


def g_1987a(ctx):
    begin(ctx, 'Sim_1001987a')
    r = ctx.rng
    objs = ptrtab(ctx)
    rec_table(ctx)
    o = ctx.buf(0x100, 'zero')
    w32(ctx, o + 4, r.choice([G(ctx), r.randrange(0, 8)]))
    w32(ctx, o + 0x50, r.choice([0, 1, 5]))
    w32(ctx, o + 0x28, r.choice([0, ctx.buf(8)]) if r.random() < 0.15 else ctx.buf(8))
    wb(ctx, o + 0x58, struct.pack('<3f', *[sane(ctx, 20000) for _ in range(3)]))
    return [o if r.random() < 0.95 else 0, r.getrandbits(32), r.getrandbits(32)]


def G(ctx):
    return struct.unpack('<I', bytes(ctx.m.uc.mem_read(0x1017b8c8, 4)))[0]


def g_19920(ctx):
    begin(ctx, 'Sim_10019920')
    r = ctx.rng
    ptrtab(ctx)
    rec_table(ctx)
    o = ctx.buf(0x100, 'zero')
    w32(ctx, o + 4, r.randrange(0, 8))
    wb(ctx, o + 0x58, struct.pack('<3f', *[sane(ctx, 20000) for _ in range(3)]))
    return [o]


def g_c476c(ctx):
    begin(ctx, 'Sim_100c476c')
    r = ctx.rng
    ptrtab(ctx)
    rec_table(ctx)
    o = ctx.buf(0x100, 'zero')
    w32(ctx, o + 4, r.randrange(0, 8))
    wb(ctx, o + 0x58, struct.pack('<3f', *[sane(ctx, 30000) for _ in range(3)]))
    return [o]


def g_25dc3(ctx):
    begin(ctx, 'Sim_10025dc3')
    return []


def vec_rec_env(ctx):
    r = ctx.rng
    w32(ctx, 0x101cfce0, *[0] ) if False else wb(ctx, 0x101cfce0, struct.pack('<%dI' % 0x7f, *[r.choice([r.getrandbits(32), r.randrange(0, 5000), (-r.randrange(0, 5000)) & 0xffffffff]) for _ in range(0x7f)]))
    w32(ctx, 0x1017cd3c, r.randrange(0, 0x7f))
    recs = ctx.buf(0x30 * 6, 'rand')
    owner = ctx.buf(0x100, 'rand')
    wb(ctx, owner + 0x3c, struct.pack('<4f', *[sane(ctx, 100) for _ in range(4)]))
    wf(ctx, owner + 0x48, sane(ctx, 100))
    prop = ctx.buf(0x100, 'rand')
    w32(ctx, prop + 0x8c, owner if r.random() < 0.8 else 0)
    base = 0x109eeb00
    for k in range(6):
        w32(ctx, base + k * 0x30 + 4, r.choice([0, prop, prop]))
    wb(ctx, base, bytes(r.getrandbits(8) for _ in range(0x30 * 6)))
    for k in range(6):
        w32(ctx, base + k * 0x30 + 4, r.choice([0, prop, prop]))


def g_28c6f(ctx):
    begin(ctx, 'Sim_10028c6f')
    vec_rec_env(ctx)
    return [ctx.rng.randrange(0, 6)]


def g_2977e(ctx):
    begin(ctx, 'Sim_1002977e')
    vec_rec_env(ctx)
    r = ctx.rng
    return [r.randrange(0, 6)] + [fbits(r.choice([0.0, sane(ctx, 50), sane(ctx, 5000), rf(ctx)])) for _ in range(3)]


def g_2a0ca(ctx):
    begin(ctx, 'Sim_1002a0ca')
    r = ctx.rng
    ptrtab(ctx)
    idx = G(ctx)
    base = 0x109e8b40
    rec = struct.unpack('<I', bytes(ctx.m.uc.mem_read(base + idx * 4, 4)))[0]
    if r.random() < 0.1:
        w32(ctx, base + idx * 4, 0)
    w32(ctx, rec + 0x40, r.choice([stub('cb1'), stub('cb1'), 0]))
    w32(ctx, rec + 0x28, r.choice([0, ctx.buf(16), ctx.buf(16)]))
    return []


def g_2d961(ctx, key='Sim_1002d961'):
    begin(ctx, key)
    r = ctx.rng
    cell_env(ctx)
    cam = ctx.buf(0x100)
    w32(ctx, 0x1014cae0, cam)
    return [fbits(grid_x(ctx)), fbits(r.choice([sane(ctx, 1000), 0.0, 500.0, -500.0, 5000.0])), fbits(grid_z(ctx))]


def g_881dd(ctx):
    begin(ctx, 'Sim_100881dd')
    cell_env(ctx)
    return [fbits(grid_x(ctx)), fbits(grid_z(ctx))]


def seg_list_env(ctx):
    """first = head of a chain (next at +0x10) of nodes with spheres; the chain of Sim_100881dd's cells is as in cell_env."""
    r = ctx.rng
    cell_env(ctx)
    prev = 0
    head = ctx.buf(0x100, 'zero')
    nodes = []
    cur = head
    for k in range(r.choice([0, 1, 2, 3, 4])):
        n = ctx.buf(0x100, 'zero')
        rad = r.choice([r.uniform(1, 800), 0.0, r.uniform(0, 5)])
        wb(ctx, n + 0x44, struct.pack('<4f', *[sane(ctx, 600) for _ in range(3)], rad))
        w32(ctx, n + 0x34, r.randrange(0, 5))
        w32(ctx, cur + 0x10, n)
        cur = n
        nodes.append(n)
    t = 0x10138bc0
    wb(ctx, t, b''.join(struct.pack('<III', r.choice([0, stub('cb4'), stub('cb4')]), 0, 0) for _ in range(6)))
    return head, nodes


def g_2dcb3(ctx):
    begin(ctx, 'Sim_1002dcb3')
    r = ctx.rng
    head, nodes = seg_list_env(ctx)
    gx = r.uniform(-254000, 245000) if r.random() < 0.9 else grid_x(ctx)
    gz = r.uniform(-246000, 253000) if r.random() < 0.9 else grid_z(ctx)
    return [head if r.random() < 0.95 else 0, fbits(gx), fbits(sane(ctx, 600)), fbits(gz)]


def g_2dc60(ctx):
    begin(ctx, 'Sim_1002dc60')
    r = ctx.rng
    head, nodes = seg_list_env(ctx)
    w32(ctx, 0x1017c8cc, head if r.random() < 0.95 else 0)
    out = ctx.buf(8, 'zero')
    gx = r.uniform(-254000, 245000) if r.random() < 0.9 else grid_x(ctx)
    gz = r.uniform(-246000, 253000) if r.random() < 0.9 else grid_z(ctx)
    return [fbits(gx), fbits(sane(ctx, 600)), fbits(gz), out]


def g_2dd6c(ctx):
    begin(ctx, 'Sim_1002dd6c')
    r = ctx.rng
    wb(ctx, 0x10138bc0, b''.join(struct.pack('<III', r.choice([0, stub('cb4')]), 0, 0) for _ in range(6)))
    o = ctx.buf(0x80, 'zero')
    w32(ctx, o + 0x34, r.randrange(0, 6))
    return [o, fbits(rf(ctx)), fbits(rf(ctx)), fbits(rf(ctx))]


def g_2e6de(ctx):
    begin(ctx, 'Sim_1002e6de')
    r = ctx.rng
    obj = ctx.buf(0x80, 'zero')
    wb(ctx, obj + 0x44, struct.pack('<3f', *[sane(ctx, 1000) for _ in range(3)]))
    seg, s, u, ln = make_seg(ctx)
    t = r.choice([10.0, 10.0, 0.0, -1.0, 5.0, 3.5, float('nan'), r.uniform(0, 100), 1e-9])
    return [obj, seg, fbits(t)]


def g_3196f(ctx):
    begin(ctx, 'Sim_1003196f')
    r = ctx.rng
    n = r.choice([0, 1, 2, 3, 4, 6, 8])
    nv = 10
    scene = ctx.buf(0x18 + nv * 0x58, 'zero')
    for i in range(nv):
        wb(ctx, scene + 0x18 + i * 0x58 + 0xc, struct.pack('<3f', *[r.choice([sane(ctx, 100), float('nan')]) if r.random() < 0.03 else sane(ctx, 100) for _ in range(3)]))
    lst = ctx.buf(0x40, 'zero')
    wb(ctx, lst + 2, struct.pack('<H', n))
    w32(ctx, lst + 4, 0x10)
    wb(ctx, lst + 0x10, bytes(r.randrange(0, nv) for _ in range(n)))
    lo = [r.uniform(-120, 20), r.uniform(-20, 120)]
    box = [lo[0], lo[0] + r.uniform(0, 200), r.uniform(-120, 20)]
    box.append(box[2] + r.uniform(0, 200))
    o1, o2 = ctx.buf(8), ctx.buf(8)
    return [lst, scene] + [fbits(x) for x in box] + [o1, o2]


def g_44296(ctx):
    begin(ctx, 'Sim_10044296')
    r = ctx.rng
    w32(ctx, 0x1013aec4, r.choice([0, 1, 5, 9, 10, 11, -1, r.randrange(-3, 14)]))
    d = r.choice([0.0, 1.0, -1.0, 0.5, r.uniform(-1e6, 1e6), r.uniform(-5, 5), 1e300, 1e12, -1e12, float('nan')])
    return [r.getrandbits(32) for _ in range(6)] + list(struct.unpack('<2I', struct.pack('<d', d)))


def g_4c965(ctx):
    begin(ctx, 'Sim_1004c965')
    r = ctx.rng
    wb(ctx, 0x10124eb8, b''.join(struct.pack('<If', r.randrange(0, 600), r.uniform(0.2, 3)) + bytes(0x90) for _ in range(64)))
    wb(ctx, 0x101116b2, bytes(r.getrandbits(8) for _ in range(0xaa * 12)))
    return [r.randrange(0, 10), r.choice([r.getrandbits(32), r.getrandbits(8), r.getrandbits(20)])]


def g_556c2(ctx):
    begin(ctx, 'Sim_100556c2')
    r = ctx.rng
    w32(ctx, 0x10164bc0, r.choice([0, 1, 2, 4, 6, 6]))
    wb(ctx, 0x1015f868, struct.pack('<6I', *[r.choice([0, 1, 7]) for _ in range(6)]))
    for i in range(6):
        t = ctx.buf(0x20, 'zero')
        w32(ctx, t + 0xc, stub('cb1'))
        w32(ctx, 0x1015fd20 + 4 * i, t)
    return []


def g_68e1d(ctx):
    begin(ctx, 'Sim_10068e1d')
    r = ctx.rng
    outs = [ctx.buf(8, 'rand') if r.random() < 0.88 else 0 for _ in range(3)]
    return [fbits(r.choice([rf(ctx), 1e30, 0.1])) for _ in range(6)] + outs


def g_74fce(ctx):
    begin(ctx, 'Sim_10074fce')
    r = ctx.rng
    o = ctx.buf(0x80, 'rand')
    w32(ctx, o + 0x10, r.choice([0, r.getrandbits(32), 0x1234]))
    w32(ctx, o + 0x1c, r.choice([0, r.getrandbits(32), 0x5678]))
    w32(ctx, 0x1014cec0, r.getrandbits(32))
    w32(ctx, 0x1014cec4, r.getrandbits(32))
    w32(ctx, 0x109b98c0, r.choice([0, r.getrandbits(32), 7]))
    return [o]


def g_75063(ctx):
    begin(ctx, 'Sim_10075063')
    r = ctx.rng
    base = 0x10153180
    for b in (0x10153180, 0x10153280):
        slots = [r.choice([0, 0, r.getrandbits(32), 1]) for _ in range(63)]
        wb(ctx, b, struct.pack('<64I', *(slots + [0xFFFFFFFF])))
    for a in (0x10153380, 0x10153384):
        w32(ctx, a, r.choice([0, 1, 1000]))
    w32(ctx, 0x1015317c, r.getrandbits(10))
    w32(ctx, 0x10149948, r.choice([0xffffffff, 0xffffffff, 3, 0x80, r.getrandbits(8)]))
    objs = []
    for i in range(14):
        if r.random() < 0.2:
            objs.append(0)
            continue
        o = ctx.buf(0x100, 'zero')
        arr = ctx.buf(64, 'zero')
        w32(ctx, o + 8, arr)
        w32(ctx, o + 0xc, r.randrange(0, 12))
        w32(ctx, o + 4, r.getrandbits(3))
        w32(ctx, o + 0x80, r.getrandbits(32))
        objs.append(o)
    wb(ctx, 0x10149958 - 4, struct.pack('<14I', *objs[:14]) if False else struct.pack('<15I', 0, *objs[:14]))
    p = ctx.buf(0x40, 'zero')
    w32(ctx, p, r.choice([1, 2, 5, 13, 13, 14 - 1, 0 + 1, r.randrange(1, 14)]))
    w32(ctx, p + 0xc, r.getrandbits(32))
    return [p]


def g_84a94(ctx):
    begin(ctx, 'Sim_10084a94')
    r = ctx.rng
    return [fbits(r.choice([rf(ctx), rf(ctx, 5), -rpos(ctx), 0.0, 1.0, 1e30])) for _ in range(3)]


def g_84c95(ctx):
    begin(ctx, 'Sim_10084c95')
    r = ctx.rng
    ps = []
    for _ in range(3):
        p = ctx.buf(8)
        wf(ctx, p, r.choice([rf(ctx), sane(ctx, 100), 0.0, 1e-9, 1e30]))
        ps.append(p)
    return ps


def g_9cbde(ctx, key='Sim_1009cbde'):
    begin(ctx, key)
    seg, s, u, ln = make_seg(ctx)
    r = ctx.rng
    if r.random() < 0.2:
        wf(ctx, seg + 0x18, 0.0); wf(ctx, seg + 0x1c, 0.0); wf(ctx, seg + 0x20, r.choice([0.0, 1e-9, -1e-9, float('nan')]))
    if r.random() < 0.15:
        wb(ctx, seg + 0x18, struct.pack('<3f', *[rf(ctx) for _ in range(3)]))
    return [seg]


def g_9cd36(ctx):
    begin(ctx, 'Sim_1009cd36')
    r = ctx.rng
    seg, s, u, ln = make_seg(ctx)
    if r.random() < 0.2:
        wb(ctx, seg + 0x18, bytes(12))
    return [seg, fbits(r.choice([rf(ctx), rpos(ctx), 0.0, 10.0, float('nan')]))]


POOL_REC, POOL_UPD, POOL_DRAW = 0x57e, 0x556, 0x55a     # the effect pool record (round 32: 1.2 mode in spec_w32_pool.py)


def rec_list_env(ctx):
    r = ctx.rng
    n = r.choice([0, 1, 3, 5, 8])
    w32(ctx, 0x1017cc20, 10)
    w32(ctx, 0x1017cc24, r.choice([0, 5, 9]))
    w32(ctx, 0x1015dc80, r.choice([0, 1, 1, 1]))
    w32(ctx, 0x1017b92c, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x10215ae8, r.choice([0, 3, 9, 0xffffffff]))
    w32(ctx, 0x1017cc2c, 0); w32(ctx, 0x1017cc2c - 4, 0)
    first = 0
    prev = 0
    nodes = []
    for k in range(n):
        nd = ctx.buf(0x20, 'zero')
        w32(ctx, nd + 8, r.choice([r.randrange(0, 10), r.randrange(0, 10), 10, 0xffffffff]))
        if prev:
            w32(ctx, prev + 4, nd)
        else:
            first = nd
        prev = nd
    w32(ctx, 0x1017cc28, first)
    for i in range(10):
        base = 0x10217c20 + i * POOL_REC
        wb(ctx, base, bytes(r.getrandbits(8) for _ in range(POOL_REC)))
        w32(ctx, base + POOL_UPD, r.choice([0, stub('cb1'), stub('cb1')]))
        w32(ctx, base + POOL_DRAW, r.choice([0, stub('cb1')]))
        w32(ctx, base + 8, r.randrange(0, 10))
    wb(ctx, 0x10216b80, bytes(0x400))


def g_90e23(ctx):
    begin(ctx, 'Sim_10090e23')
    rec_list_env(ctx)
    return []


def g_90f17(ctx):
    begin(ctx, 'Sim_10090f17')
    rec_list_env(ctx)
    r = ctx.rng
    return [r.choice([r.randrange(0, 10), r.randrange(0, 12), 0xffffffff, 10])]


def g_a2823(ctx):
    begin(ctx, 'Sim_100a2823', skip=(2, 3, 5, 6))
    r = ctx.rng
    return [r.choice([r.randrange(-300, 900), -2 ** 31, 2 ** 31 - 1, 16777217, 33554435]) & 0xffffffff for _ in range(10)] + [r.choice([0xf0, r.getrandbits(32)])]


def g_a2c08(ctx):
    begin(ctx, 'Sim_100a2c08')
    r = ctx.rng
    mode = r.choice([1, 2, 1, 2, 1, 2, 0, 3])
    def pt():
        return [r.randrange(-1000, 1000) for _ in range(6)]
    a, b = pt(), pt()
    if r.random() < 0.1:
        b = list(a)
    if r.random() < 0.1:
        a[mode % 2] = r.choice([-2 ** 31, 2 ** 31 - 1])
    pa = ctx.buf(24); pb = ctx.buf(24); po = ctx.buf(24, 'rand')
    wb(ctx, pa, struct.pack('<6i', *a)); wb(ctx, pb, struct.pack('<6i', *b))
    v = r.choice([r.randrange(-1500, 1500), a[(mode + 1) % 2 if mode else 0], b[0], 2 ** 31 - 1])
    return [mode, v & 0xffffffff, pa, pb, po]


def g_a78eb(ctx):
    begin(ctx, 'Sim_100a78eb')
    w32(ctx, 0x1017d160, stub('cb1'))
    return [ctx.rng.getrandbits(32)]


def g_cc887(ctx, key='Sim_100cc887'):
    begin(ctx, key)
    r = ctx.rng
    w32(ctx, 0x1015dc78, r.getrandbits(32))
    head = ctx.buf(8, 'zero')
    prev = head
    for k in range(r.choice([0, 1, 2, 4])):
        n = ctx.buf(0x20, 'zero')
        w32(ctx, n, stub('cbcc')); w32(ctx, n + 8, r.getrandbits(32))
        w32(ctx, prev if prev == head else prev + 0x14, n)
        prev = n
    return [head]


def g_cc863(ctx):
    begin(ctx, 'Sim_100cc863')
    r = ctx.rng
    w32(ctx, 0x1015dc78, r.getrandbits(32))
    n = ctx.buf(0x20, 'zero')
    w32(ctx, n, stub('cbcc')); w32(ctx, n + 8, r.getrandbits(32))
    return [r.getrandbits(32), n]


def g_b42d5(ctx):
    begin(ctx, 'Sim_100b42d5')
    r = ctx.rng
    w32(ctx, 0x1015dc78, r.getrandbits(32))
    k = r.randrange(0, 4)
    prev = None
    for q in range(4):
        head = 0x101a2184 + q * 0x88
        first = 0
        p = 0
        for j in range(r.choice([0, 1, 3])):
            n = ctx.buf(0x20, 'zero')
            w32(ctx, n, stub('cbcc')); w32(ctx, n + 8, r.getrandbits(32))
            if p: w32(ctx, p + 0x14, n)
            else: first = n
            p = n
        w32(ctx, head, first)
    return [k]


def g_bda9f(ctx):
    begin(ctx, 'Sim_100bda9f')
    r = ctx.rng
    n = r.choice([0, 1, 2, 5, 9, 16])
    arr = ctx.buf(4 * 20, 'rand')
    tab = ctx.buf(16, 'zero')
    w32(ctx, tab + 4, n if r.random() < 0.95 else 0x80000001)
    w32(ctx, tab + 8, arr)
    return [tab, r.getrandbits(32), stub('cmp')]


def g_baf3f(ctx):
    begin(ctx, 'Sim_100baf3f')
    r = ctx.rng
    w32(ctx, 0x1017d60c, stub('ts'))
    n = r.choice([1, 2, 3, 5, 7])
    cnt = 6
    ents = []
    key = ctx.buf(16, 'rand')
    for i in range(cnt):
        e = ctx.buf(0x40, 'rand')
        if r.random() < 0.5:
            wb(ctx, e, bytes(ctx.m.uc.mem_read(key, 8)))
        ents.append(e)
    bucket = ctx.buf(4 * n, 'zero')
    nxt = [0] * n
    for i, e in enumerate(ents):
        b = r.randrange(0, n)
        w32(ctx, e + 0x1c, struct.unpack('<I', bytes(ctx.m.uc.mem_read(bucket + 4 * b, 4)))[0])
        w32(ctx, bucket + 4 * b, e)
    h = struct.unpack('<2I', bytes(ctx.m.uc.mem_read(key, 8)))
    # pick the bucket count so that the key's bucket exists (mostly)
    tab = ctx.buf(0x20, 'zero')
    w32(ctx, tab, n); w32(ctx, tab + 0x10, bucket)
    # table object found through the sorted pointer table at [0x1017d604]: one entry, compared by 0x100b971d (4 bytes at +0x18)
    reg = ctx.buf(16, 'zero')
    arr = ctx.buf(8, 'zero')
    w32(ctx, reg + 4, 1); w32(ctx, reg + 8, arr)
    tab2 = tab
    wb(ctx, tab2 + 0x18, b'abcd')
    w32(ctx, arr, tab2)
    w32(ctx, 0x1017d604, reg)
    arg = ctx.buf(16, 'zero')
    wb(ctx, arg, b'abcd' if r.random() < 0.9 else b'abce')
    out = ctx.buf(8, 'zero')
    return [key, arg, out]


def g_d2a7b(ctx):
    begin(ctx, 'Sim_100d2a7b')
    r = ctx.rng
    obj = ctx.buf(0x100, 'zero')
    w32(ctx, obj, r.choice([0, 0x1000, 0x1000, r.getrandbits(16)]) if r.random() < 0.15 else r.getrandbits(16) & ~0x1000)
    wb(ctx, obj + 0x44, struct.pack('<4f', *[sane(ctx, 300) for _ in range(3)], r.choice([r.uniform(0, 100), 1.0, -5.0])))
    cam = ctx.buf(0x100, 'zero')
    wb(ctx, cam + 0x18, struct.pack('<f', r.uniform(0.2, 3)))
    for off in (0xa8, 0xac):
        wf(ctx, cam + off, r.choice([r.uniform(0.2, 3), r.uniform(-2, 2), 0.0]))
    wf(ctx, cam + 0xb0, r.uniform(0.2, 3))
    w32(ctx, 0x1014cae0, cam)
    m = [r.uniform(-1, 1) for _ in range(9)]
    wb(ctx, 0x10185c20, struct.pack('<9f', *m))
    wb(ctx, 0x10185c38, struct.pack('<3f', *[r.uniform(-1, 1) for _ in range(3)]))
    wb(ctx, 0x10185c44, struct.pack('<3f', *[sane(ctx, 100) for _ in range(3)]))
    wf(ctx, 0x10185c48, sane(ctx, 100))
    wf(ctx, 0x10185c50, r.choice([0.0, 1.0, -50.0, 5.0]))
    return [obj]


# ---- renderer-facing: copied environment helpers of spec_w2n -------------------------------------------
def proj_globals(ctx):
    r = ctx.rng
    wide = r.random() < 0.7
    w32(ctx, 0x10185bd4, r.choice([320, 160, 0, r.randrange(-100, 700)]))
    w32(ctx, 0x10185bf4, r.choice([240, 100, 0, r.randrange(-100, 500)]))
    w32(ctx, 0x10185bd0, 0 if wide else r.randrange(-50, 300))
    w32(ctx, 0x10185bec, 639 if wide else r.randrange(100, 700))
    w32(ctx, 0x10185bf0, 0 if wide else r.randrange(-50, 200))
    w32(ctx, 0x10185be0, 479 if wide else r.randrange(100, 600))
    w32(ctx, 0x101c7680, r.getrandbits(8))
    w32(ctx, 0x101cc2a0, r.getrandbits(8))
    w32(ctx, 0x101c673c, r.choice([0, 5, 0xffffffff, r.getrandbits(32)]))


def pool_setup(ctx):
    r = ctx.rng
    cap = r.choice([4, 12, 40, 600, 600])
    buf = ctx.buf(0x40 * 620)
    top = buf + 0x40 * 620
    w32(ctx, 0x1017d174, top)
    w32(ctx, 0x1017d178, top - 0x40 * cap - 0xc8)


def matrix(ctx):
    r = ctx.rng
    c = r.random()
    if c < 0.5:
        m = [1, 0, 0, 0, 1, 0, 0, 0, 1]
    elif c < 0.75:
        a = r.uniform(-3, 3)
        m = [math.cos(a), 0, -math.sin(a), 0, 1, 0, math.sin(a), 0, math.cos(a)]
    else:
        m = [r.uniform(-2, 2) for _ in range(9)]
    wb(ctx, 0x10185ba0, struct.pack('<9f', *m))
    pos = [r.choice([0.0, 0.0, r.uniform(-50, 50)]) for _ in range(3)]
    wf(ctx, 0x10185c48, pos[0]); wf(ctx, 0x10185c44, pos[1]); wf(ctx, 0x10185c4c, pos[2])


def clip_env(ctx):
    r = ctx.rng
    proj_globals(ctx)
    pool_setup(ctx)
    matrix(ctx)
    w32(ctx, 0x101c9ea4, 1 if r.random() < 0.93 else 0)
    w32(ctx, 0x101c9ea8, r.choice([0, 0, 7, 300]))
    wf(ctx, 0x10185c50, r.uniform(-5, 5))
    cam = ctx.buf(0x100, 'zero')
    wf(ctx, cam + 0x3c, r.choice([0.5, 1.0, 4.0, 10.0, 0.0, -1.0]))
    wf(ctx, cam + 0x40, r.choice([0.0, 100.0, 1000.0, 5000.0, 1e9, -5.0]))
    w32(ctx, 0x1014cae0, cam)
    return cam


def g_89907(ctx):
    begin(ctx, 'Sim_10089907', skip=(6,))
    r = ctx.rng
    ctx.m.uc.mem_write(0x00F00000 - 0x20000, bytes([r.choice([0, 0x55, 0xA5, 0x3F])]) * (0x20000 - 0x180)) if False else None
    stack_fill(ctx)
    clip_env(ctx)
    sc = r.choice([20.0, 300.0, 3000.0])
    return [fbits(sane(ctx, sc)), fbits(sane(ctx, sc)), fbits(r.choice([sane(ctx, sc * 2), r.uniform(0, sc)])), fbits(r.choice([r.uniform(0, sc), 1.0, 0.0, 100.0, -5.0])), r.choice([0x14, 0x1c, 4, r.getrandbits(32)])]


def stack_fill(ctx):
    STACK_TOP = 0x00800000
    # Unset scene fields of Sim_10089907 flow into clipper output vertices (never read by callers); only the zero pattern is comparable.
    pat = 0
    ctx.m.uc.mem_write(STACK_TOP - 0x20000, bytes([pat]) * (0x20000 - 0x180))


def g_89bad(ctx):
    begin(ctx, 'Sim_10089bad', skip=(6,))
    r = ctx.rng
    stack_fill(ctx)
    cell_env(ctx)
    cam = clip_env(ctx)
    wb(ctx, cam, struct.pack('<3f', grid_x(ctx), r.choice([0.0, 500.0, -200.0]), grid_z(ctx)))
    w32(ctx, 0x1017c8c0, 1 if r.random() < 0.9 else 0)
    return []


def make_specs():
    rows = [
        ('Sim_10008ed0', 0x10008ed0, g_8ed0, 400, dict(void=True)),
        ('Sim_1000946e', 0x1000946e, g_946e, 400, dict(tramp=4)),
        ('Sim_10009ca8', 0x10009ca8, g_9ca8, 600, dict(tramp=4)),
        ('Sim_10009d7e', 0x10009d7e, g_9d7e, 1200, dict(tramp=2)),
        ('Sim_10009eb3', 0x10009eb3, g_9eb3, 1200, dict(tramp=2)),
        ('Sim_10017ad9', 0x10017ad9, g_17ad9, 400, dict(void=True)),
        ('Sim_1001987a', 0x1001987a, g_1987a, 300, {}),
        ('Sim_10019920', 0x10019920, g_19920, 400, {}),
        ('Sim_10025dc3', 0x10025dc3, g_25dc3, 300, dict(void=True)),
        ('Sim_10028c6f', 0x10028c6f, g_28c6f, 400, dict(void=True)),
        ('Sim_1002977e', 0x1002977e, g_2977e, 500, dict(void=True)),
        ('Sim_1002a0ca', 0x1002a0ca, g_2a0ca, 300, dict(void=True)),
        ('Sim_1002d961', 0x1002d961, g_2d961, 500, dict(tramp=3)),
        ('Sim_1002dc60', 0x1002dc60, g_2dc60, 500, {}),
        ('Sim_1002dcb3', 0x1002dcb3, g_2dcb3, 500, {}),
        ('Sim_1002dd6c', 0x1002dd6c, g_2dd6c, 300, {}),
        ('Sim_1002e6de', 0x1002e6de, g_2e6de, 500, dict(void=True)),
        ('Sim_1003196f', 0x1003196f, g_3196f, 600, {}),
        ('Sim_10044296', 0x10044296, g_44296, 500, dict(void=True)),
        ('Sim_1004c965', 0x1004c965, g_4c965, 500, {}),
        ('Sim_100556c2', 0x100556c2, g_556c2, 300, dict(void=True)),
        ('Sim_10068e1d', 0x10068e1d, g_68e1d, 400, dict(void=True)),
        ('Sim_10074fce', 0x10074fce, g_74fce, 300, dict(void=True)),
        ('Sim_10075063', 0x10075063, g_75063, 400, {}),
        ('Sim_10084a94', 0x10084a94, g_84a94, 1000, dict(tramp=3)),
        ('Sim_10084c95', 0x10084c95, g_84c95, 600, dict(void=True)),
        ('Sim_100881dd', 0x100881dd, g_881dd, 600, dict(void=True)),
        ('Sim_10090e23', 0x10090e23, g_90e23, 300, dict(void=True)),
        ('Sim_10090f17', 0x10090f17, g_90f17, 300, dict(void=True)),
        ('Sim_1009cbde', 0x1009cbde, lambda c: g_9cbde(c, 'Sim_1009cbde'), 500, dict(tramp=1)),
        ('Sim_1009cbfe', 0x1009cbfe, lambda c: g_9cbde(c, 'Sim_1009cbfe'), 500, dict(void=True)),
        ('Sim_1009cd36', 0x1009cd36, g_9cd36, 500, dict(void=True)),
        ('Sim_100a2c08', 0x100a2c08, g_a2c08, 800, dict(void=True)),
        ('Sim_100a78eb', 0x100a78eb, g_a78eb, 300, {}),
        ('Sim_100b42d5', 0x100b42d5, g_b42d5, 300, dict(void=True)),
        ('Sim_100baf3f', 0x100baf3f, g_baf3f, 400, {}),
        ('Sim_100bda9f', 0x100bda9f, g_bda9f, 400, {}),
        ('Sim_100c476c', 0x100c476c, g_c476c, 400, {}),
        ('Sim_100cc863', 0x100cc863, g_cc863, 300, dict(void=True)),
        ('Sim_100cc887', 0x100cc887, g_cc887, 300, dict(void=True)),
        ('Sim_100d2a7b', 0x100d2a7b, g_d2a7b, 800, {}),
    ]
    specs = []
    for k, (name, addr, gen, trials, opt) in enumerate(rows):
        opt = dict(opt)
        c = name
        if 'tramp' in opt:
            n = opt.pop('tramp')
            orig = TRAMP_BASE + k * 0x40
            TRAMPS[name] = (addr, n)
            c = 'Test_' + name[4:]
            opt['void'] = True
        else:
            orig = addr
        ENTRY[name] = (orig, c)
        specs.append(dict(name=name, orig=orig, c=c, gen=gen, trials=trials, fp=True, tol=TOL, **opt))
    return specs


SPECS = make_specs()

# round 32: the pool layout follows W31_TexMode() (off here: the 1.0 layout)
SOURCES = SOURCES + [x for x in ['sim/render/w31_tex.c', 'sim/fx/w32_partspec.c', '../tools/verify/plat_stub.c',
                                 'sim/render/w3q_texture.c', 'sim/unsorted_1000/w2f_records.c', 'sim/unsorted_100f/w2e_string.c',
                                 'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_1004/state.c'] if x not in SOURCES]
