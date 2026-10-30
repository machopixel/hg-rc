SOURCES = [
    'sim/unsorted_1000/w3q_misc.c', 'sim/unsorted_1002/w3q_collide.c', 'sim/unsorted_1002/w3q_misc.c',
    'sim/unsorted_1002/w3q_widgets.c', 'sim/unsorted_1003/w3q_grid.c', 'sim/unsorted_1004/w3q_misc.c',
    'sim/unsorted_1005/w3q_misc.c', 'sim/unsorted_1006/w3q_quat.c', 'sim/unsorted_1007/w3q_misc.c',
    'sim/unsorted_1008/w3q_place.c', 'sim/unsorted_1009/w3q_misc.c', 'sim/unsorted_100a/w3q_orient.c',
    'sim/unsorted_100d/w3q_misc.c', 'sim/render/w3q_texture.c', 'sim/render/w3q_driver.c',
    'sim/render/w3q_vertex.c',
    # converted functions that the ones above call
    'sim/util/field.c', 'sim/util/ring.c', 'sim/unsorted_1004/fields.c', 'sim/unsorted_100b/fields.c',
    'sim/unsorted_100a/w2h_misc.c', 'sim/unsorted_1000/w2h_reset.c', 'sim/unsorted_100f/w2e_string.c',
    'sim/view/viewrect.c', '../tools/verify/w3q_test.c',
]
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

# Differential tests for round 3, letter q (see difftest.py).
#
# Floating-point functions are built with SSE2 doubles like Diego's build and compared with the x87 original
# (no fp tolerance is used: every spec is compared bit for bit, which holds because each step is a double operation).
# Calls the functions make through stored pointers (callbacks, widget methods, the display driver slot
# 0x1014cb4c, the cache loader 0x100ba7d5) go to stubs in the emulator: a `ret` at a fixed address with a code
# hook that appends the arguments to a log buffer in the argument memory (compared afterwards) and sets
# the return value; the same stub addresses serve the original and the C run. Pointers into the stack are
# never logged (the stack frames differ): the display driver's vertices are logged by content, and only
# the fields the functions set (the others are stack garbage in the original, zero in the C version).
import struct
import math
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

STUB = 0x00F00100
ST = {}
LOADER = 0x100ba7d5
TRAMP = 0x00F00C00
OUT_D = 0x00AFF000
LOGCAP = 0x4000
# stub kinds: address = STUB + 8 * index
KIND = {'cb': 0, 'px': 1, 'drv': 2, 'm10c': 3, 'm110': 4, 'm114': 5, 'm124': 6, 'm12c': 7, 'm130': 8, 'm11c': 9}
NARGS = {'cb': 1, 'px': 2, 'm10c': 2, 'm110': 2, 'm114': 2, 'm124': 2, 'm12c': 2, 'm130': 1, 'm11c': 2}
BYADDR = {STUB + 8 * i: k for k, i in KIND.items()}


def sa(kind):
    return STUB + 8 * KIND[kind]

def w32(ctx, addr, *v):
    ctx.m.uc.mem_write(addr, struct.pack('<%dI' % len(v), *[x & 0xffffffff for x in v]))

def wf(ctx, addr, *v):
    ctx.m.uc.mem_write(addr, struct.pack('<%df' % len(v), *v))

def wb(ctx, addr, data):
    ctx.m.uc.mem_write(addr, bytes(data))

def r32(uc, a):
    return struct.unpack('<I', bytes(uc.mem_read(a, 4)))[0]

def fbits(f):
    return struct.unpack('<I', struct.pack('<f', f))[0]

def f32(f):
    """f rounded to float32 (inf for overflow)."""
    try:
        return struct.unpack('<f', struct.pack('<f', f))[0]
    except OverflowError:
        return math.copysign(float('inf'), f)


def emit(uc, items):
    log = ST['log']
    pos = r32(uc, log)
    out = struct.pack('<%dI' % len(items), *[x & 0xffffffff for x in items])
    if pos + len(out) > LOGCAP:
        return
    uc.mem_write(log + 4 + pos, out)
    uc.mem_write(log, struct.pack('<I', pos + len(out)))

def cstr(uc, a, n=40):
    try:
        d = bytes(uc.mem_read(a, n))
    except Exception:
        return b'?'
    return d.split(b'\0')[0] + b'\0'

def _hook(m):
    uc = m.uc

    def cb(uc, address, size, ud):
        try:
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<6I', bytes(uc.mem_read(esp + 4, 24)))
            ret = 0
            if address == LOADER:
                name = cstr(uc, a[0])
                emit(uc, [100, a[1], a[2], len(name)] + list(name))
                pool = ST['pool']
                ret = pool[(sum(name) + ST['seed']) % (len(pool) + 1)] if (sum(name) + ST['seed']) % (len(pool) + 1) < len(pool) else 0
            else:
                kind = BYADDR[address]
                ret = ST['ret'][kind]
                if kind == 'drv':
                    items = [200, a[0], a[2]]
                    for k in range(max(0, min(a[0], 8))):
                        d = struct.unpack('<9I', bytes(uc.mem_read(a[1] + 36 * k, 36)))
                        items += [d[0], d[1], d[4], d[6]]
                    emit(uc, items)
                else:
                    emit(uc, [300 + KIND[kind]] + list(a[:NARGS[kind]]))
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB, STUB + 0x80)
    uc.hook_add(UC_HOOK_CODE, cb, None, LOADER, LOADER)
    uc.mem_write(STUB, b'\xc3' * 0x80)
    uc.mem_write(LOADER, b'\xc3')
    # original Sim_100d6e99 returns in st(0): store it at OUT_D
    code = b'\x55\x8b\xec\xff\x75\x08\xe8' + struct.pack('<i', 0x100d6e99 - (TRAMP + 11)) + b'\x83\xc4\x04\xdd\x1d' + struct.pack('<I', OUT_D) + b'\x5d\xc3'
    uc.mem_write(TRAMP, code)


def begin(ctx):
    """Stubs (once), x87 control word as Windows leaves it, a fresh log, random stub results."""
    m = ctx.m
    if not getattr(m, '_w3q', False):
        _hook(m)
        m._w3q = True
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    r = ctx.rng
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    ST['ret'] = {k: r.choice([0, 1, r.getrandbits(32)]) for k in KIND}
    ST['seed'] = r.getrandbits(8)
    ST['pool'] = [ctx.buf(64, struct.pack('<hh', r.choice([0x100, 7, -3, 40, r.randrange(-50, 700)]), r.choice([256, 5, 17, r.randrange(-50, 700)])) + bytes(r.getrandbits(8) for _ in range(60))) for _ in range(4)]


SPECIAL = [0.0, -0.0, 1.0, -1.0, 0.5, -0.5, 2.0, 1e-7, -1e-7, 100.0, -100.0, 1e10, -1e10, 3.0e38, 1e-30]

def rfl(r, spread=100.0, special=0.1, nan=0.0):
    c = r.random()
    if c < nan:
        return r.choice([float('nan'), float('inf'), float('-inf')])
    if c < nan + special:
        return r.choice(SPECIAL)
    return r.uniform(-spread, spread)

def small(r):
    """Coordinates from a small grid so that equalities between vertices and the query happen often."""
    return float(r.choice([-2, -1, 0, 1, 2, 3])) if r.random() < 0.8 else r.uniform(-3, 3)


# ---- polygon / mesh builders -----------------------------------------------------------------
def make_verts(ctx, n, grid=True):
    r = ctx.rng
    buf = ctx.buf(max(n, 1) * 0x58 + 0x20)
    for i in range(n):
        o = buf + 0x18 + i * 0x58 if False else buf + i * 0x58
        if grid:
            xs = [small(r), small(r), small(r)]
        else:
            xs = [rfl(r), rfl(r), rfl(r)]
        wf(ctx, o + 0xc, *xs)
    return buf

def make_poly(ctx, nverts_total, verts_base, count=None, normal=None):
    """A polygon record (0x28 bytes + vertex list behind it); verts_base is the vertex array address."""
    r = ctx.rng
    n = count if count is not None else r.choice([0, 1, 2, 3, 3, 4, 4, 5, 6, 8])
    rec = ctx.buf(0x28 + 16)
    lst = rec + 0x28
    idx = [r.randrange(0, max(nverts_total, 1)) for _ in range(max(n, 0))]
    wb(ctx, lst, bytes(idx + [0]))
    wb(ctx, rec + 2, struct.pack('<H', n))
    w32(ctx, rec + 4, lst - rec)
    nz = normal if normal is not None else [rfl(r, 2.0, 0.2), r.choice([-1.0, 1.0, 0.5, -0.5, 0.0, rfl(r, 2.0, 0.1)]), rfl(r, 2.0, 0.2)]
    wf(ctx, rec + 0x18, *nz)
    return rec

def mesh_world(ctx, npolys=None, cluster=True):
    """A mesh buffer: short +6 = polygon count, dword +8 = offset of the 0x28-byte records, vertices at +0x18."""
    r = ctx.rng
    nv = r.choice([3, 4, 6, 9])
    npoly = npolys if npolys is not None else r.choice([1, 2, 3, 4, 5])
    mesh = ctx.buf(0x18 + nv * 0x58 + npoly * 0x28 + 64)
    voff = 0x18
    for i in range(nv):
        wf(ctx, mesh + voff + i * 0x58 + 0xc, small(r), small(r), small(r))
    polys = []
    roff = voff + nv * 0x58
    wb(ctx, mesh + 6, struct.pack('<h', npoly))
    w32(ctx, mesh + 8, roff)
    lists = mesh + roff + npoly * 0x28
    for p in range(npoly):
        rec = mesh + roff + p * 0x28
        n = r.choice([3, 3, 4, 4, 5, 6])
        idx = [r.randrange(0, nv) for _ in range(n)]
        lo = lists + p * 8
        wb(ctx, lo, bytes(idx[:8]) + b'\0')
        wb(ctx, rec + 2, struct.pack('<H', n))
        w32(ctx, rec + 4, lo - rec)
        wf(ctx, rec + 0x18, rfl(r, 2.0, 0.2), r.choice([-1.0, 1.0, 0.5, -0.5, 0.0, 1e-3, rfl(r, 2.0, 0.1)]), rfl(r, 2.0, 0.2))
        polys.append(rec)
    return mesh, polys

def grid_world(ctx, polys):
    """A 7 x 7 cell grid (see unsorted_1003/w3q.h) around the origin, cells holding lists of the polygons."""
    r = ctx.rng
    g = ctx.buf(0x24 + 7 * 0xe0 + 16)
    wf(ctx, g, r.choice([-30000.0, -30000.0, -4000.0, 0.0]), r.choice([30000.0, 30000.0, 4000.0]),
       r.choice([-1000.0, -5.0, -1e9]), r.choice([1000.0, 5.0, 1e9]),
       r.choice([-30000.0, -30000.0, -4000.0]), r.choice([30000.0, 30000.0, 4000.0]))
    wf(ctx, g + 0x18, r.choice([0.0, 0.0, 500.0]))
    wf(ctx, g + 0x20, r.choice([0.0, 0.0, -500.0]))
    ox = struct.unpack('<f', bytes(ctx.m.uc.mem_read(g + 0x18, 4)))[0]
    oz = struct.unpack('<f', bytes(ctx.m.uc.mem_read(g + 0x20, 4)))[0]
    for ix in range(7):
        for iz in range(7):
            c = g + 0x24 + ix * 0xe0 + iz * 0x20
            x0 = ox - 28000.0 + ix * 8000.0
            z0 = oz - 28000.0 + iz * 8000.0
            jit = r.choice([0.0, 0.0, 0.0, 3000.0, -3000.0])
            wf(ctx, c, x0 + jit, x0 + 8000.0 + jit, 0, 0, z0, z0 + 8000.0)
            # list of polygons
            head = 0
            for p in reversed(r.sample(polys, r.randrange(0, len(polys) + 1))):
                node = ctx.buf(8)
                w32(ctx, node, head, p if r.random() > 0.05 else 0)
                head = node
            w32(ctx, c + 0x18, r.choice([1, 1, 1, 0]), head)
    return g

def query(ctx):
    r = ctx.rng
    return [fbits(small(r)), fbits(small(r)), fbits(small(r))]

def far_query(ctx):
    r = ctx.rng
    c = r.random()
    if c < 0.3:
        return [fbits(r.uniform(-35000, 35000)), fbits(small(r)), fbits(r.uniform(-35000, 35000))]
    if c < 0.4:
        return [fbits(r.choice([-28000.0, 28000.0, 0.0, -20000.0, 4000.0, 3999.0, -4000.0, 36000.0, 8000.0, -12000.0, float('nan')])), fbits(small(r)), fbits(r.choice([0.0, 4000.0, -4000.0, 28000.0, float('nan')]))]
    return query(ctx)


# ---- generators ----------------------------------------------------------------
def g_d8be(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([3, 4, 5])
    verts = make_verts(ctx, n, grid=r.random() < 0.8)
    poly = make_poly(ctx, n, verts)
    out = ctx.buf(8)
    return [poly, verts] + query(ctx) + [out]

def g_e9f5(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([3, 4, 5, 6])
    verts = make_verts(ctx, n)
    poly = make_poly(ctx, n, verts)
    if r.random() < 0.05:
        wb(ctx, poly + 2, struct.pack('<H', r.choice([0, 1, 2])))
    a, b = fbits(small(r)), fbits(small(r))
    if r.random() < 0.06:
        a = fbits(float('nan'))
    if r.random() < 0.06:
        b = fbits(float('nan'))
    return [poly, verts, a, b]

def g_obj_mesh(ctx, grid=False, far=False):
    r = ctx.rng
    begin(ctx)
    mesh, polys = mesh_world(ctx)
    obj = ctx.buf(0x100)
    w32(ctx, obj + 0x30, mesh)
    w32(ctx, obj + 0x2c, mesh)
    gridp = grid_world(ctx, polys) if grid else 0
    w32(ctx, obj + 0x98, gridp if grid and r.random() < 0.9 else 0)
    return obj, mesh, gridp

def g_d276(ctx):
    obj, mesh, g = g_obj_mesh(ctx)
    return [obj] + query(ctx)

def g_d3f8(ctx):
    r = ctx.rng
    obj, mesh, g = g_obj_mesh(ctx, grid=r.random() < 0.7)
    return [obj] + (far_query(ctx) if r.random() < 0.7 else query(ctx))

def g_310e2(ctx):
    r = ctx.rng
    begin(ctx)
    mesh, polys = mesh_world(ctx)
    g = grid_world(ctx, polys)
    q = far_query(ctx) if r.random() < 0.5 else query(ctx)
    return [g if r.random() > 0.04 else 0, mesh if r.random() > 0.04 else 0] + q

def g_309aa(ctx):
    r = ctx.rng
    begin(ctx)
    mesh, polys = mesh_world(ctx)
    g = grid_world(ctx, polys)
    q = far_query(ctx) if r.random() < 0.5 else query(ctx)
    out = ctx.buf(8, b'\xaa' * 8)
    return [g if r.random() > 0.04 else 0, mesh if r.random() > 0.04 else 0] + q + [out if r.random() > 0.03 else 0]

def g_d479(ctx):
    r = ctx.rng
    obj, mesh, g = g_obj_mesh(ctx, grid=True)
    out = ctx.buf(8, b'\xaa' * 8)
    return [obj] + (far_query(ctx) if r.random() < 0.6 else query(ctx)) + [out]

def g_ftol_fn(ctx):
    begin(ctx)
    return [ctx.u32()]

def g_5897b(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([3, 4, 6])
    verts = make_verts(ctx, n, grid=False)
    for i in range(n):
        wf(ctx, verts + i * 0x58 + 0xc, r.choice([-3.0, 0.0, 1.0, 5.0, rfl(r, 8)]), r.choice([-3.0, 0.0, 1.0, 5.0, rfl(r, 8)]), r.choice([-3.0, 0.0, 1.0, 5.0, rfl(r, 8)]))
    poly = make_poly(ctx, n, verts)
    box = ctx.buf(0x60)
    wf(ctx, box + 0x14, r.choice([0.0, 2.0, rfl(r, 8)]), r.choice([0.0, 2.0, rfl(r, 8)]), r.choice([0.0, 2.0, rfl(r, 8)]))
    wf(ctx, box + 0x3c, r.choice([1.0, 2.0, 0.0, 3.0, rfl(r, 5, 0.0)]))
    return [poly if r.random() > 0.05 else 0, verts if r.random() > 0.05 else 0, box]

def g_fc4f(ctx):
    r = ctx.rng
    begin(ctx)
    m = ctx.buf(48)
    wf(ctx, m, *[rfl(r, 4.0, 0.2, 0.02) for _ in range(9)])
    v = ctx.buf(16)
    wf(ctx, v, *[rfl(r, 50.0, 0.2, 0.02) for _ in range(3)])
    ptrs = [v, v + 4, v + 8]
    if r.random() < 0.1:
        ptrs = [v, v, v + 8]
    if r.random() < 0.03:
        ptrs = [m + 12, m + 16, m + 20]
    return [m] + ptrs

def g_2edc(ctx):
    r = ctx.rng
    begin(ctx)
    cam = ctx.buf(0x80)
    wf(ctx, cam + 0x58, rfl(r, 5, 0.1))
    wf(ctx, cam + 0x64, r.choice([1.0, 2.0, -1.0, 0.0, rfl(r, 5, 0.1)]))
    wf(ctx, cam + 0x70, rfl(r, 5, 0.1))
    w32(ctx, 0x10185bd4, r.choice([320, 0, ctx.u32(), r.randrange(-50, 700)]))
    w32(ctx, 0x10185bf4, r.choice([240, 0, ctx.u32(), r.randrange(-50, 500)]))
    wf(ctx, 0x10185be4, r.choice([1.0, 8.0, 0.0, 320.0, rfl(r, 50, 0.1)]))
    wf(ctx, 0x10185be8, r.choice([1.0, 8.0, 0.0, 240.0, rfl(r, 50, 0.1), 1e30]))
    return [r.choice([r.randrange(-100, 700), ctx.u32(), 0x80000000]), cam]

def g_quat(ctx):
    r = ctx.rng
    begin(ctx)
    a = ctx.buf(16)
    b = ctx.buf(16)
    out = ctx.buf(16)
    q = lambda: [rfl(r, 2.0, 0.3, 0.02) for _ in range(4)]
    wf(ctx, a, *q()); wf(ctx, b, *q())
    c = r.random()
    if c < 0.1:
        return [a, b, a]
    if c < 0.2:
        return [a, b, b]
    return [a, b, out]

def g_orient(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0xb0)
    wf(ctx, obj + 0x3c, *[rfl(r, 2.0, 0.3) for _ in range(4)])
    wf(ctx, obj + 0x7c, *[rfl(r, 2.0, 0.3) for _ in range(4)])
    w32(ctx, obj + 0x94, ctx.u32())
    q = ctx.buf(16)
    wf(ctx, q, *[rfl(r, 2.0, 0.3) for _ in range(4)])
    return [obj, q]

def g_c565(ctx):
    r = ctx.rng
    begin(ctx)
    return [r.randrange(0, 40), ctx.u32()]

def g_e6a2(ctx):
    r = ctx.rng
    begin(ctx)
    rows = r.choice([0, 1, 3, 5, 0x7fffffff, 0xffffffff])
    w32(ctx, 0x1015f224, r.choice([0, 1, 2, 4, 5, rows if rows < 100 else 0, 0xffffffff]))
    w32(ctx, 0x1015f330, rows)
    w32(ctx, 0x1015f334, sa('px'))
    w32(ctx, 0x1015f338, ctx.u32())
    w32(ctx, 0x1015f220, ctx.u32())
    cols = r.choice([1, 2, 3, 7, 0, 0xffffffff])
    w32(ctx, 0x1015f33c, cols)
    w32(ctx, 0x1015f228, r.choice([0, 1, 2, 6, cols - 1 if cols else 0, 0xffffffff, ctx.u32()]))
    return []

def g_81e0(ctx):
    r = ctx.rng
    begin(ctx)
    for oa, va in [(0x109e8928, 0x1096fc08), (0x109e8938, 0x1096fc0c), (0x109e890c, 0x1096fc04), (0x109e88e8, 0x1096fc00),
                   (0x109e894c, 0x1096fc10), (0x109e8950, 0x1096fc14), (0x109e8954, 0x1096fc18)]:
        o = ctx.buf(0x140)
        w32(ctx, o + 0x11c, sa('m11c'))
        w32(ctx, oa, o)
        w32(ctx, va, ctx.u32())
    return []

def g_97f2(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 5, 20, 40, -3])
    w32(ctx, 0x104737e0, n)
    wb(ctx, 0x1017ee40, struct.pack('<64I', *[r.choice([0, 0, 1, r.getrandbits(32)]) for _ in range(64)]))
    return []

def g_e489(ctx):
    r = ctx.rng
    begin(ctx)
    size = ctx.buf(8)
    w32(ctx, size, r.choice([0, 1, 100, 640, 7, ctx.u32(), r.randrange(-50, 50)]), r.choice([0, 1, 100, 480, 5, ctx.u32(), r.randrange(-50, 50)]))
    out = ctx.buf(8)
    mode = r.choice([0, 1, 2, 3, 4, 5, 6, 7, 8, 100, ctx.u32()]) if r.random() < 0.9 else r.randrange(0, 8)
    scale = r.choice([0.5, 1.0, 1.33, 0.75, 2.0, -0.5, -1.0, -2.0, 0.0, -0.0, 1e-30, 1e30, float('nan'), float('inf'), rfl(r, 4, 0.0)])
    sz = struct.unpack('<2i', bytes(ctx.m.uc.mem_read(size, 8)))
    if sz[0] and r.random() < 0.3:
        scale = r.choice([sz[1] / sz[0], -sz[1] / sz[0]])     # exactly on the comparison
    return [size, mode, fbits(scale), out]

def g_98be0(ctx):
    r = ctx.rng
    begin(ctx)
    n = r.choice([0, 1, 3, 6, 16, 20])
    w32(ctx, 0x1015dc68, n)
    for i in range(24):
        w32(ctx, 0x109e9ba8 + i * 0x50, r.choice([-1, 0, 1, 2, 3, r.randrange(0, 12), ctx.u32()]), r.choice([0, 1, 2, 3, 0xfffffffe, ctx.u32()]))
    w32(ctx, 0x1014bd88, ctx.buf(0x60))
    cnt = r.choice([0, 3, 8])
    w32(ctx, 0x1017d218, cnt)
    for i in range(12):
        blk = ctx.buf(0x60) if r.random() < 0.5 else 0
        obj = ctx.buf(0x90) if r.random() < 0.7 else 0
        w32(ctx, 0x101a2100 + i * 0x88 + 0x28, blk, obj)
        for base in (blk, obj):
            if base:
                wf(ctx, base + 0x44, *[rfl(r, 5, 0.2) for _ in range(3)])
                wf(ctx, base + 0x70, *[rfl(r, 5, 0.2) for _ in range(3)])
    wf(ctx, 0x1012b9ac, 0.0) if False else None
    return []

def g_b876(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0x40)
    w32(ctx, obj + 0x18, r.choice([0, 100, ctx.u32(), r.randrange(-200, 400)]))
    w32(ctx, 0x1017cc58, r.choice([0, 5, ctx.u32(), r.randrange(-50, 50)]))
    w32(ctx, 0x1014bf00, r.choice([16, 100, 1000, 0, 1, -7, 0x7fffffff, ctx.u32()]))
    return [obj, fbits(r.choice([0.0, 0.5, -0.5, 1.5, 10.0, -10.0, 100.0, 1e10, -1e10, float('nan'), float('inf'), rfl(r, 300, 0.0)]))]

def g_cb40(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0x40)
    return [obj] + [fbits(r.choice([rfl(r, 10, 0.1, 0.02), rfl(r, 1e6, 0.0), 16777217.0, 1e-8, 3.0])) for _ in range(7)]

def g_d6e99(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0xc0)
    w32(ctx, obj + 0xb4, r.choice([0, 1, -1, 0x7fffffff, 0x80000000, ctx.u32()]))
    return [obj]

def g_1000951c(ctx):
    r = ctx.rng
    begin(ctx)
    lst = ctx.buf(0x20)
    head = 0
    for _ in range(r.choice([0, 1, 2, 3, 5])):
        node = ctx.buf(0x20)
        w32(ctx, node + 8, head)
        head = node
    w32(ctx, lst + 8, head)
    return [lst if r.random() > 0.1 else 0, sa('cb')]

def g_8e46(ctx):
    r = ctx.rng
    begin(ctx)
    obj = ctx.buf(0x40)
    head = 0
    lim = r.choice([0.0, 1.0, -1.0, 5.0, float('nan')])
    for _ in range(r.choice([0, 1, 2, 4, 5])):
        node = ctx.buf(0x20)
        wf(ctx, node, r.choice([lim, 0.0, 1.0, 2.0, -2.0, 7.0, rfl(r, 5, 0.1, 0.05)]))
        w32(ctx, node + 0xc, head)
        head = node
    w32(ctx, obj + 0x2c, head)
    w32(ctx, obj + 0x30, 0x12345678)
    return [obj, fbits(lim)]

def g_ca63(ctx):
    r = ctx.rng
    begin(ctx)
    cnt = r.choice([0, 1, 2, 4, 6])
    w32(ctx, 0x1015d0a0, cnt)
    kinds = [r.randrange(0, 6) for _ in range(6)]
    for i in range(6):
        base = 0x10a3a600 + i * 0xa8
        th = sorted(r.choice([0.0, 1.0, 2.0, 5.0, rfl(r, 20, 0.0)]) for _ in range(20))
        w32(ctx, base, kinds[i])
        wf(ctx, base + 8, *th)
        w32(ctx, base + 0x58, *[r.choice([fbits(float(r.randrange(0, 60))), fbits(r.uniform(0, 100)), fbits(r.uniform(-10, 100)), ctx.u32()]) for _ in range(20)])
    for k in range(6):
        w32(ctx, 0x10145518 + k * 0xd6, r.choice([0x45, 0x45, 0, 5, 0x44]))
    w32(ctx, 0x1017cd38, r.randrange(0, 127))
    w32(ctx, 0x101cffc0, *[r.choice([0, 3, 7, 12345, r.randrange(-1000, 1000), ctx.u32()]) for _ in range(127)])
    kind = r.choice(kinds + [9, 0, 1])
    if r.random() < 0.05:
        kind = r.choice([7, -1, 0x7fffffff])
    out = ctx.buf(8, b'\xaa' * 8)
    ths = [1.0, 2.0, 5.0, 0.0, -1.0, 100.0, 1e9, float('nan')]
    return [kind, fbits(r.choice(ths + [rfl(r, 20, 0.0)])), out]

def g_6ba7(ctx):
    r = ctx.rng
    begin(ctx)
    name = bytes(r.choice(b'abxXwWG01_') for _ in range(r.randrange(1, 14))) + b'\0'
    if r.random() < 0.4:
        name = bytes([r.choice(b'Ww'), r.choice(b'Xx')]) + name
    nm = ctx.buf(0x20, name.ljust(0x20, b'\0'))
    w32(ctx, 0x1014cb38, r.choice([0, 1, 1]))
    w32(ctx, 0x1012da30, r.choice([-2, 0, 0, 5, 0x200]))
    w32(ctx, 0x1015cfe0, r.choice([0, 1, 5, 0x1fe, 0x1ff, r.randrange(0, 0x200)]))
    w32(ctx, 0x1015cfe4, r.choice([0, 1]))
    w32(ctx, 0x1015cfe8, ctx.u32())
    w32(ctx, 0x1015ec58, r.choice([0, 0, 0, 1]))
    w32(ctx, 0x1017b954, r.choice([0, 1, 1]))
    w32(ctx, 0x1015cff4, ctx.u32())
    w32(ctx, 0x1014ceb8, ctx.u32())
    group = r.choice([-1, -1, 0, 3, 0x1ff, 0x200, -5, r.randrange(0, 0x200)])
    ptr = 0x10a51ac0
    for g in set([group & 0x3ff, 0, 1, 3, 4, 5, 6, 0x1fe, 0x1ff]):
        if 0 <= g < 0x200:
            fill = r.choice(['empty', 'one', 'some', 'full'])
            nused = {'empty': 0, 'one': 1, 'some': r.randrange(1, 31), 'full': 32}[fill]
            for k in range(32):
                ent = bytearray(0x18)
                if k < nused:
                    ent[0] = r.choice([0x41, 0x61, 0x57, 0x7f, 0x80, 0xff])
                    ent[1] = 0x31
                wb(ctx, ptr + g * 0x300 + k * 0x18, ent)
    return [nm if r.random() > 0.0 else 0, group, r.choice([0, 1, 0x7fff, 0xffff, ctx.u32()])]

def g_quad(ctx, n):
    r = ctx.rng
    begin(ctx)
    w32(ctx, 0x1014cb4c, sa('drv'))
    c = lambda: r.choice([0, 1, -1, 320, 639, 16777217, 33554435, -2 ** 31, 2 ** 31 - 1, r.randrange(-500, 1500)])
    return [c() for _ in range(2 * n)] + [ctx.u32()]

def g_vtx(ctx):
    r = ctx.rng
    begin(ctx)
    top = ctx.buf(0x40 * 40)
    w32(ctx, 0x1017d174, top + 0x40 * 40)
    w32(ctx, 0x1017d178, top + r.choice([0, 0x40, 0x40 * 10, 0x40 * 36]))
    w32(ctx, 0x101c9ea4, ctx.u32())
    obj = ctx.buf(0x80)
    w32(ctx, obj + 0x2c, r.choice([0, 0, ctx.buf(0x40)]))
    scr = ctx.buf(8)
    return [obj, scr if r.random() < 0.5 else 0]

def g_vtx1(ctx):
    a = g_vtx(ctx)
    return [a[0]]

def widgets(ctx, methods):
    r = ctx.rng
    area = ctx.buf(20, struct.pack('<5i', 0, r.choice([0, 10, -5]), r.choice([0, 20, 7]), r.choice([640, 100, 1 << 20, 17]), r.choice([480, 77, 1 << 20, 3])))
    for n in range(30):
        o = ctx.buf(0x150)
        for off in methods:
            w32(ctx, o + off, sa('m%x' % off))
        w32(ctx, o + 0xec, area)
        w32(ctx, 0x109e88e0 + n * 4, o)
        w32(ctx, o + 0xd4, ctx.u32(), ctx.u32(), ctx.u32(), ctx.u32())
        w32(ctx, o + 0xe4, ctx.u32(), ctx.u32())
    w32(ctx, 0x101c64f0, r.choice([0, 1]))

def g_b68f(ctx):
    r = ctx.rng
    begin(ctx)
    widgets(ctx, [0x110, 0x114])
    owner = ctx.buf(0x100)
    recs = ctx.buf(8 * 0x128)
    for i in range(8):
        w32(ctx, recs + i * 0x128, r.choice([0, 0, 1, ctx.u32()]), r.choice([0, 1, 5, 0xffffffff, ctx.u32(), r.randrange(0, 12)]))
    w32(ctx, owner + 0x84, recs)
    return [owner]

def g_c113(ctx):
    r = ctx.rng
    begin(ctx)
    widgets(ctx, [0x10c, 0x110, 0x114, 0x124, 0x12c, 0x130])
    owner = ctx.buf(0x100)
    recs = ctx.buf(8 * 0x128)
    for i in range(8):
        w32(ctx, recs + i * 0x128, r.choice([0, 0, 1, ctx.u32()]), r.choice([0, 1, 5, 0xffffffff, ctx.u32(), r.randrange(0, 12)]))
    w32(ctx, owner + 0x84, recs)
    entry = ctx.buf(0x40)
    w32(ctx, entry + 0x28, owner)
    idx = r.randrange(0, 200)
    w32(ctx, 0x1017b8c8, idx)
    w32(ctx, 0x109e8b40 + idx * 4, entry)
    w32(ctx, 0x101381d0, *[ctx.u32() for _ in range(30)])
    w32(ctx, 0x10138158, *[ctx.u32() for _ in range(30)])
    w32(ctx, 0x109e8960, *[r.choice([0, 1, ctx.u32()]) for _ in range(30)])
    return []

def g_349b1(ctx):
    r = ctx.rng
    m = ctx.m
    if not getattr(m, '_mda', False):
        m.uc.mem_map(0xB0000, 0x2000)
        m.wr.append((0xB0000, 0x2000))
        m.snap0 = m.dump()
        m._mda = True
    begin(ctx)
    return [ctx.u32(), r.choice([0, 1, 5, 0x19, 0x1a, 0x1b, 0xffffffff, r.randrange(0, 0x1a)])]


SPECS = [
    dict(name='Sim_100219a0', orig=0x100219a0, c='Sim_100219a0', gen=lambda c: (begin(c), [0])[1], trials=300),
    dict(name='Sim_1000951c', orig=0x1000951c, c='Sim_1000951c', gen=g_1000951c, trials=400, void=True),
    dict(name='Sim_10008e46', orig=0x10008e46, c='Sim_10008e46', gen=g_8e46, trials=400, void=True),
    dict(name='Sim_1000ca63', orig=0x1000ca63, c='Sim_1000ca63', gen=g_ca63, trials=600),
    dict(name='Sim_1002d8be', orig=0x1002d8be, c='Sim_1002d8be', gen=g_d8be, trials=600),
    dict(name='Sim_1002e9f5', orig=0x1002e9f5, c='Sim_1002e9f5', gen=g_e9f5, trials=3000),
    dict(name='Sim_1002ed0b', orig=0x1002ed0b, c='Sim_1002ed0b', gen=g_e9f5, trials=3000),
    dict(name='Sim_1002d276', orig=0x1002d276, c='Sim_1002d276', gen=g_d276, trials=4000),
    dict(name='Sim_1002d4a7', orig=0x1002d4a7, c='Sim_1002d4a7', gen=g_d276, trials=5000),
    dict(name='Sim_1002d3f8', orig=0x1002d3f8, c='Sim_1002d3f8', gen=g_d3f8, trials=4000),
    dict(name='Sim_1002d479', orig=0x1002d479, c='Sim_1002d479', gen=g_d479, trials=1000),
    dict(name='Sim_100310e2', orig=0x100310e2, c='Sim_100310e2', gen=g_310e2, trials=6000),
    dict(name='Sim_100309aa', orig=0x100309aa, c='Sim_100309aa', gen=g_309aa, trials=1500),
    dict(name='Sim_100349b1', orig=0x100349b1, c='Sim_100349b1', gen=g_349b1, trials=400, void=True),
    dict(name='Sim_1004c5c1', orig=0x1004c5c1, c='Sim_1004c5c1', gen=g_c565, trials=600),
    dict(name='Sim_1004e6a2', orig=0x1004e6a2, c='Sim_1004e6a2', gen=g_e6a2, trials=400),
    dict(name='Sim_10052edc', orig=0x10052edc, c='Sim_10052edc', gen=g_2edc, trials=800),
    dict(name='Sim_1005897b', orig=0x1005897b, c='Sim_1005897b', gen=g_5897b, trials=1000),
    dict(name='Sim_1005fc4f', orig=0x1005fc4f, c='Sim_1005fc4f', gen=g_fc4f, trials=800, void=True),
    dict(name='Quat_Multiply', orig=0x1006112e, c='Quat_Multiply', gen=g_quat, trials=1000, void=True),
    dict(name='Sim_100aa1de', orig=0x100aa1de, c='Sim_100aa1de', gen=g_orient, trials=500, void=True),
    dict(name='Sim_100aa227', orig=0x100aa227, c='Sim_100aa227', gen=g_orient, trials=500, void=True),
    dict(name='Sim_100aa2be', orig=0x100aa2be, c='Sim_100aa2be', gen=g_orient, trials=500, void=True),
    dict(name='Sim_100781e0', orig=0x100781e0, c='Sim_100781e0', gen=g_81e0, trials=300, void=True),
    dict(name='Sim_100797f2', orig=0x100797f2, c='Sim_100797f2', gen=g_97f2, trials=300, void=True),
    dict(name='Sim_1008e489', orig=0x1008e489, c='Sim_1008e489', gen=g_e489, trials=1500),
    dict(name='Sim_10098be0', orig=0x10098be0, c='Sim_10098be0', gen=g_98be0, trials=500, void=True),
    dict(name='Sim_1009b876', orig=0x1009b876, c='Sim_1009b876', gen=g_b876, trials=1500),
    dict(name='Sim_1009cb40', orig=0x1009cb40, c='Sim_1009cb40', gen=g_cb40, trials=800, void=True),
    dict(name='Sim_100d6e99', orig=TRAMP, c='Test_100d6e99', gen=g_d6e99, trials=300, void=True),
    dict(name='Sim_10006ba7', orig=0x10006ba7, c='Sim_10006ba7', gen=g_6ba7, trials=1500),
    dict(name='Sim_100a291d', orig=0x100a291d, c='Sim_100a291d', gen=lambda c: g_quad(c, 4), trials=500),
    dict(name='Sim_100a29da', orig=0x100a29da, c='Sim_100a29da', gen=lambda c: g_quad(c, 3), trials=500),
    dict(name='Sim_100a5c10', orig=0x100a5c10, c='Sim_100a5c10', gen=g_vtx, trials=600),
    dict(name='Sim_100a5d25', orig=0x100a5d25, c='Sim_100a5d25', gen=g_vtx1, trials=400),
    dict(name='Sim_1002b68f', orig=0x1002b68f, c='Sim_1002b68f', gen=g_b68f, trials=400, void=True),
    dict(name='Sim_1002c113', orig=0x1002c113, c='Sim_1002c113', gen=g_c113, trials=400, void=True),
]
