# Differential tests for wave 3B (x87 renderer path, src/sim/render/w2n_*.c, see difftest.py).
#
# The C code is built with SSE2 doubles like Diego's build, so the float words that come out of the
# matrix transform (Render_ClipPolygon and its two callers) are compared with a tolerance (fp=True,
# worst deviation printed); everything else is bit-exact (the other functions only convert, divide
# floats singly, or truncate, which gives the same bits on x87 and SSE).
#
# Stubs in the emulator (same address for the original and the C run): the plugin slots Render
# (0x109e3d30), SetLumaTable (0x109e3d24) and the display driver slot 0x1014cb4c get a `ret` plus a
# code hook that appends a record of the arguments to a log buffer (compared afterwards), the cache
# loader 0x100ba7d5 and the software polygon routine 0x100f9fd4 are replaced the same way. Integers
# are written to the log as four NaN-tagged dwords each so that the float tolerance of the checker
# can never hide a difference in an integer; floats are written raw. Fields the original leaves
# unset (stack garbage: the texture key of some Render calls, the fog dword of the software vertices,
# the first dword of the 8-byte op-0xf descriptor) are not logged. The stack below the argument area
# is filled with one repeated byte per trial, so the uninitialised scene-vertex fields that the
# original passes to its (unconverted) helpers read the same in both runs.
import math
import struct
from unicorn import UC_HOOK_CODE
from unicorn.x86_const import UC_X86_REG_ESP, UC_X86_REG_EAX, UC_X86_REG_FPCW

SOURCES = ['sim/render/w2m_texture.c', 'sim/render/w2n_texture.c', 'sim/render/w2n_project.c',
           'sim/render/w2n_polygon.c', '../tools/verify/w2n_test.c', 'sim/render/w33_dx12.c']
CFLAGS = ['-msse2', '-mfpmath=sse', '-mno-stack-arg-probe']

STUB = 0x00F00400
STUBS = {STUB: 'render', STUB + 4: 'luma', STUB + 8: 'drv'}
SLOT_RENDER, SLOT_LUMA, SLOT_DRV = 0x109e3d30, 0x109e3d24, 0x1014cb4c
LOADER, SWPOLY = 0x100ba7d5, 0x100f9fd4
TRAMP = 0x00F00800
LOGCAP = 0x30000
STACK_TOP = 0x00800000
ST = {}


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

def clampf(f):
    return max(-3e38, min(3e38, f))


# ---- log ----------------------------------------------------------------
def emit(uc, items):
    log = ST['log']
    pos = r32(uc, log)
    out = bytearray()
    for kind, v in items:
        if kind == 'i':
            v &= 0xffffffff
            for s in (0, 8, 16, 24):
                out += struct.pack('<I', 0x7fc00000 | ((v >> s) & 0xff))
        else:
            out += struct.pack('<I', v & 0xffffffff)
    if pos + len(out) > LOGCAP:
        uc.mem_write(log + 4 + LOGCAP, struct.pack('<I', 0xdead))
        return
    uc.mem_write(log + 4 + pos, bytes(out))
    uc.mem_write(log, struct.pack('<I', pos + len(out)))

def I(v): return ('i', v)
def F(v): return ('f', v)

def read_or(uc, addr, n):
    try:
        return bytes(uc.mem_read(addr, n))
    except Exception:
        return None

VERT_KINDS = 'ffffiiiffx'   # per dword of a plugin vertex: f float, i int; x = left out when `skip18`

def log_vertices(uc, addr, count, skip18=False):
    items = []
    count = max(0, min(count, 300))
    for k in range(count):
        d = read_or(uc, addr + k * 36, 36)
        if d is None:
            items.append(I(0xBADBAD)); break
        w = struct.unpack('<9I', d)
        for j, x in enumerate(w):
            if j in (4, 5, 6):
                if j == 6 and skip18:
                    continue
                items.append(I(x))
            else:
                items.append(F(x))
    return items


def _hook(m):
    uc = m.uc
    lo, hi = m.base, m.base + m.dll_size

    def cb(uc, address, size, ud):
        try:
            stub = STUBS.get(address)
            if address == LOADER:
                stub = 'load'
            elif address == SWPOLY:
                stub = 'sw'
            if stub is None:
                return
            esp = uc.reg_read(UC_X86_REG_ESP)
            a = struct.unpack('<8I', bytes(uc.mem_read(esp + 4, 32)))
            ret = ST['ret'].get(stub, 0)
            if stub == 'render':
                items = [I(1), I(a[0]), I(a[1]), I(a[3])]
                if a[0] == 0xf:
                    items += [I(r32(uc, a[4] + 4))]
                    items += log_vertices(uc, a[2], a[3])
                else:
                    items += log_vertices(uc, a[2], a[3])
                    try:
                        handle = r32(uc, a[4])
                        d = struct.unpack('<4I', bytes(uc.mem_read(handle, 16)))
                        items += [I(handle if False else 0)] + [I(x) for x in d]
                        if a[0] in ST['keyops']:
                            items += [I(r32(uc, handle + 16))]
                    except Exception:
                        items += [I(0xBADBAD)]
                emit(uc, items)
            elif stub == 'luma':
                emit(uc, [I(2), I(a[0])])
            elif stub == 'drv':
                emit(uc, [I(3), I(a[0]), I(a[2])] + log_vertices(uc, a[1], a[0], skip18=True))
            elif stub == 'load':
                name = _w31_addr(a[0])      # round 31: a name in our table as the 1.0 table's address
                emit(uc, [I(4), I(name), I(a[1]), I(a[2])])
                pool = ST['pool']
                h = ((name * 2654435761) + ST['seed']) & 0xffffffff
                k = (h >> 8) % (len(pool) + 2)
                ret = pool[k] if k < len(pool) else 0
            elif stub == 'sw':
                items = [I(5), I(a[0]), I(a[2])] + [I(x) for x in a[3:8]]
                d = read_or(uc, a[1], max(0, min(a[2], 300)) * 24)
                if d is None:
                    items.append(I(0xBADBAD))
                else:
                    items += [I(x) for x in struct.unpack('<%dI' % (len(d) // 4), d)]
                emit(uc, items)
            uc.reg_write(UC_X86_REG_EAX, ret & 0xffffffff)
        except Exception as e:
            ST['err'] = repr(e)
            raise
    uc.hook_add(UC_HOOK_CODE, cb, None, STUB, STUB + 0x10)
    uc.hook_add(UC_HOOK_CODE, cb, None, LOADER, LOADER)
    uc.hook_add(UC_HOOK_CODE, cb, None, SWPOLY, SWPOLY)
    uc.mem_write(STUB, b'\xc3' * 0x10)
    uc.mem_write(LOADER, b'\xc3')
    uc.mem_write(SWPOLY, b'\xc3')
    # float-to-int helper wrapper: fld qword [esp+4]; call 0x100fcb08; ret
    code = b'\xdd\x44\x24\x04\xe8' + struct.pack('<i', 0x100fcb08 - (TRAMP + 9)) + b'\xc3'
    uc.mem_write(TRAMP, code)


def begin(ctx, keyops=()):
    """Stubs (once), a fresh log, the slots, a uniform stack fill, the loader's image pool."""
    m = ctx.m
    r = ctx.rng
    if not getattr(m, '_w2n', False):
        _hook(m)
        m._w2n = True
    # Unicorn starts with control word 0 (24-bit precision, exceptions unmasked): use the Windows default 0x27f
    m.uc.reg_write(UC_X86_REG_FPCW, 0x27f)
    ST['log'] = ctx.buf(4 + LOGCAP + 8, 'zero')
    ST['ret'] = {s: r.choice([0, 1, r.getrandbits(32)]) for s in ('render', 'luma', 'drv', 'sw')}
    ST['keyops'] = set(keyops)
    ST['seed'] = r.getrandbits(32)
    ST['pool'] = [mk_image(ctx) for _ in range(5)]
    w32(ctx, SLOT_RENDER, STUB)
    w32(ctx, SLOT_LUMA, STUB + 4)
    w32(ctx, SLOT_DRV, STUB + 8)
    pat = r.choice([0, 0, 0x55, 0xA5, 0x3F, 0x7B])
    m.uc.mem_write(STACK_TOP - 0x20000, bytes([pat]) * (0x20000 - 0x180))
    w32(ctx, 0x10143fdc, r.getrandbits(32))
    w32(ctx, 0x10143fe0, r.getrandbits(32))
    w32(ctx, 0x10143fe4, r.getrandbits(32))
    w32(ctx, 0x101cfc8c, r.getrandbits(8))


def mk_image(ctx):
    r = ctx.rng
    return ctx.buf(8300, struct.pack('<hh', r.choice([0x100, 0x40, 7, -3, r.randrange(-50, 700)]), r.choice([256, 32, 5, r.randrange(-50, 700)])) + bytes(r.getrandbits(8) for _ in range(8296)))


def camera(ctx, near=None, far=None):
    r = ctx.rng
    cam = ctx.buf(0x100)
    wf(ctx, cam + 0x3c, near if near is not None else r.choice([0.5, 1.0, 4.0, 10.0, 0.0, -1.0]))
    wf(ctx, cam + 0x40, far if far is not None else r.choice([0.0, 100.0, 1000.0, 5000.0, 1e9, -5.0]))
    wf(ctx, cam + 0x94, r.choice([1.0, 320.0, 0.5, 1000.0, -2.0]))
    w32(ctx, 0x1014cae0, cam)
    return cam


def fog_globals(ctx):
    r = ctx.rng
    wf(ctx, 0x10143fc8, r.choice([0.0, 1.0, 50.0, 100.0, 1e-3]))
    wf(ctx, 0x10143fc4, r.choice([0.0, 1.0, 0.5, 400.0, -1.0, 1e5]))
    wf(ctx, 0x10185bf8, r.choice([0.0, 100.0, 1000.0, 5000.0, 20.0]))
    wf(ctx, 0x101668cc, r.choice([0.0, 0.25, 0.5, 1.0, 2.0]))


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


def pool_setup(ctx, cap=None):
    r = ctx.rng
    cap = cap if cap is not None else r.choice([4, 12, 40, 600, 600])
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
    wf(ctx, 0x10185c48, pos[0])
    wf(ctx, 0x10185c44, pos[1])
    wf(ctx, 0x10185c4c, pos[2])


def clip_env(ctx, near=None, far=None):
    r = ctx.rng
    proj_globals(ctx)
    pool_setup(ctx)
    matrix(ctx)
    w32(ctx, 0x101c9ea4, 1 if r.random() < 0.93 else 0)
    w32(ctx, 0x101c9ea8, r.choice([0, 0, 7, 300]))
    wf(ctx, 0x10185c50, r.uniform(-5, 5))


def scene_verts(ctx, n):
    r = ctx.rng
    buf = ctx.buf(max(1, n) * 0x58)
    spread = r.choice([50.0, 300.0, 3000.0])
    zr = r.choice([(-20.0, 60.0), (-5.0, 1500.0), (0.0, 120.0), (500.0, 3000.0)])
    for i in range(n):
        o = buf + i * 0x58
        xs = [r.uniform(-spread, spread), r.uniform(-spread, spread), r.uniform(*zr)]
        if r.random() < 0.1:
            xs[2] = r.choice([0.0, 1.0, 4.0, 10.0, 100.0, 1000.0])
        wb(ctx, o + 0xc, struct.pack('<3f', *xs))
        w32(ctx, o + 0x2c, 0)
        # the callees read floats from the rest of the record: keep them sane
        wb(ctx, o + 0x18, struct.pack('<2f', r.uniform(0, 1), r.uniform(0, 1)))
        wb(ctx, o + 0x40, struct.pack('<3f', r.uniform(0, 1), r.uniform(0, 1), r.uniform(0, 1)))
    return buf


# ---- generators ------------------------------------------------------------
def g_fog(ctx):
    r = ctx.rng
    begin(ctx)
    fog_globals(ctx)
    def flt():
        c = r.random()
        if c < 0.06: return fbits(float('nan'))
        if c < 0.1: return fbits(float('inf'))
        if c < 0.15: return fbits(float('-inf'))
        return fbits(r.choice([0.0, 1.0, 5.0, 50.0, 100.0, 400.0, 1000.0, 20.0, -3.0, r.uniform(-10, 6000), r.uniform(0, 200), 1e30, -1e-5]))
    return [flt(), flt()]


def g_ftol(ctx):
    r = ctx.rng
    begin(ctx)
    c = r.random()
    if c < 0.15:
        d = r.choice([0.5, -0.5, 1.5, -1.5, 2.5, -2.5, 0.0, -0.0, 0.99999999, -0.99999999, 1e-300, -1e-300, 2147483647.5, -2147483648.5, 2147483648.0, 4294967295.5, 4294967296.0, 9.2e18, -9.2e18, 9223372036854775807.0, -9223372036854775808.0, 1.8446744073709552e19, 1e300, -1e300, float('inf'), float('-inf'), float('nan')])
    elif c < 0.4:
        d = r.uniform(-4, 4)
    elif c < 0.7:
        d = r.uniform(-3e9, 3e9)
    elif c < 0.85:
        d = r.uniform(-1e19, 1e19)
    else:
        d = r.choice([-1, 1]) * 2.0 ** r.uniform(0, 70)
    b = struct.pack('<d', d)
    return list(struct.unpack('<2I', b))


def g_project(ctx):
    r = ctx.rng
    begin(ctx)
    proj_globals(ctx)
    w32(ctx, 0x101c9ea4, r.choice([0, 1]))
    w32(ctx, 0x101c9ea8, r.choice([0, 1, 100, 254, 255, 256, 0x7fffffff]) if r.random() < 0.3 else r.randrange(0, 30))
    v = ctx.buf(0x40)
    def f(spread):
        c = r.random()
        if c < 0.04: return float('nan')
        if c < 0.07: return r.choice([float('inf'), float('-inf')])
        return r.choice([0.0, spread * r.random(), -spread * r.random(), r.uniform(-1e30, 1e30) if c < 0.12 else 1.0])
    z = r.choice([r.uniform(0.01, 5000.0), r.uniform(-100, 100), 0.0, -0.0, 1e-30, 1e-38, 1e30, r.uniform(0.5, 50)])
    x = f(r.choice([100, 3000, 1e9]))
    y = f(r.choice([100, 3000, 1e9]))
    wb(ctx, v, struct.pack('<3f', clampf(x) if x == x and abs(x) != float('inf') else x, clampf(y) if y == y and abs(y) != float('inf') else y, z))
    wb(ctx, v + 0x28, bytes([r.getrandbits(8), r.choice([0, 1, 1, 2, 3, r.getrandbits(8)])]))
    ST['dbg'] = (x, y, z)
    return [v]


def g_clip(ctx):
    r = ctx.rng
    begin(ctx)
    clip_env(ctx)
    n = r.choice([0, 1, 2, 3, 3, 3, 4, 4, 5, 6, 8, 12])
    verts = scene_verts(ctx, n)
    near = fbits(r.choice([0.5, 1.0, 4.0, 10.0, 0.0, -1.0, 100.0]))
    far = fbits(r.choice([0.0, 100.0, 1000.0, 5000.0, 1e9, -5.0, 60.0]))
    return [verts, n, near, far]


def tex_tables(ctx):
    """Texture records, name tables and caches as seen by Render_GetTextureImage."""
    r = ctx.rng
    ngroups = 3
    recs = bytearray(0x200 * 14)
    for i in range(0x200):
        flag = r.choice([1, 1, 1, 0, 3, 0x8001, 0xfffe])
        w6 = r.choice([1, 1, 1, 0, 0x100, r.getrandbits(16)])
        # round 31: frame -1 only after group 0 (it is the last frame of the group before; before group 0 it is
        # memory next to the table, other data once the table is ours: w31_map.py)
        grp = r.randrange(0, ngroups)
        frm = r.choice([r.randrange(0, 32), r.randrange(0, 32), -1 if grp > 0 else 0, 31])
        struct.pack_into('<hhhHhhh', recs, i * 14, grp, frm, 0, w6, r.choice([r.randrange(0, 5), -2, -1]), 0, 0)
    wb(ctx, 0x10ac1b00, recs)
    for g in range(ngroups):
        names = bytearray(0x300)
        for k in range(32):
            ent = bytearray(0x18)
            ent[0] = r.choice([0x41, 0x61, 0x57, 0x01, 0x7f, 0x00, 0x80, 0xff])
            ent[1:8] = bytes(r.choice(b'abc012') for _ in range(7))
            if r.random() < 0.3:
                struct.pack_into('<I', ent, 0x14, r.choice(ST['pool']))
            names[k * 0x18:(k + 1) * 0x18] = ent
        wb(ctx, 0x10a51ac0 + g * 0x300, names)
    w32(ctx, 0x1014ceb8, ctx.u32())
    use = r.random() < 0.6
    w32(ctx, 0x1015cfec, 1 if use else r.choice([0, 0]))
    w32(ctx, 0x1012da38, r.choice([0, 1, 2, 5, 0x20, -1, r.randrange(-3, 40)]))
    w32(ctx, 0x1015cffc, r.choice([0, 0, r.choice(ST['pool'])]))
    w32(ctx, 0x1015d000, r.choice([0, 0, r.choice(ST['pool'])]))
    w32(ctx, 0x1015cff8, r.getrandbits(8))
    w32(ctx, 0x1012da34, r.choice([0, 1, 1, 0x80]))
    w32(ctx, 0x1015cff0, r.choice([0, r.choice(ST['pool'])]))
    wb(ctx, 0x10a3ae80, bytes(r.getrandbits(8) for _ in range(40)))
    w32(ctx, 0x1014cf0c, ctx.u32())


def index_for(ctx, base):
    r = ctx.rng
    return r.choice([r.randrange(base, base + 0x20), r.randrange(base, base + 0x20), r.randrange(base, base + 0x100), base, base + 0x200 - 1 - base if False else r.randrange(base, base + 0x100)])


def g_getimage(ctx):
    r = ctx.rng
    begin(ctx)
    tex_tables(ctx)
    wh = ctx.buf(8)
    # round 31: no negative record numbers (a record before the table names a group outside the texture table)
    idx = r.choice([r.randrange(0, 0x30), r.randrange(0, 0x200), r.randrange(0x100, 0x120), 0x1ff, 0x200, 0x300, 0xff, 0x100])
    return [idx, wh, wh + 4]


def verts36(ctx, n, ztypes=True):
    r = ctx.rng
    buf = ctx.buf(max(1, n) * 36)
    for i in range(n):
        z = r.choice([r.uniform(0.1, 3000.0), r.uniform(-50.0, 50.0), 0.0, -0.0, 1e-30, 1e30, 1.0])
        wb(ctx, buf + i * 36 + 8, struct.pack('<f', z))
    return buf


def g_sub1(ctx):
    r = ctx.rng
    begin(ctx)
    tex_tables(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 3]))
    n = r.choice([0, 1, 2, 3, 3, 3, 4, 5, 8, 20])
    return [r.randrange(-0x100, 0x100) if r.random() < 0.8 else r.randrange(-0x100, 0x140), n, verts36(ctx, n)]


def g_sub2(ctx):
    ST_keys = (6,)
    r = ctx.rng
    begin(ctx, keyops=ST_keys)
    tex_tables(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 3]))
    n = r.choice([0, 1, 2, 3, 3, 3, 4, 5, 8, 20])
    return [r.randrange(-0x100, 0x100) if r.random() < 0.8 else r.randrange(-0x100, 0x140), n, verts36(ctx, n), r.choice([0, 0, 1, 0x100, -1])]


def g_sub3(ctx):
    r = ctx.rng
    begin(ctx, keyops=(6, 2))
    tex_tables(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 3]))
    n = r.choice([0, 1, 2, 3, 3, 3, 4, 5, 8, 20])
    return [r.randrange(0, 0x200) if r.random() < 0.8 else r.randrange(0, 0x230), n, verts36(ctx, n)]


def g_ptrs(ctx):
    r = ctx.rng
    begin(ctx, keyops=(1, 2, 6))
    camera(ctx)
    w32(ctx, 0x1017b940, r.choice([1, 1, 1, 0, 5]))
    w32(ctx, 0x1017d0ac, 0)       # round 33: the C tests 1.2's 3DFX flag (g_w33Voodoo, off) instead of 1.0's
    n = r.choice([0, 1, 2, 3, 3, 4, 5, 8, 30, 200, 255])
    verts = []
    for i in range(n):
        v = ctx.buf(0x40)
        z = r.choice([r.uniform(0.1, 3000.0), r.uniform(-50.0, 50.0), 0.0, -0.0, 1e-30, 1e30, 1.0])
        wb(ctx, v + 8, struct.pack('<f', z))
        wb(ctx, v + 0x18, struct.pack('<ii', r.choice([r.randrange(-500, 1500), -2 ** 31, 2 ** 31 - 1, 16777217, 33554435]), r.randrange(-500, 1500)))
        verts.append(v)
    arr = ctx.buf(max(n, 1) * 4, struct.pack('<%dI' % n, *verts) if n else b'\0' * 4)
    desc = ctx.buf(20)
    handle = ctx.buf(4, struct.pack('<I', desc))
    texture = handle if r.random() < 0.9 else 0
    return [texture, arr, n]


def g_clipped(ctx):
    r = ctx.rng
    begin(ctx, keyops=())
    clip_env(ctx)
    camera(ctx)
    fog_globals(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
    w32(ctx, 0x1017d1e4, r.choice([0, 0, r.choice(ST['pool'])]))
    w32(ctx, 0x1014cf0c, ctx.u32())
    wf(ctx, 0x10185bf8, r.uniform(0, 5000))
    n = r.choice([3, 3, 3, 4, 4, 5, 6, 8, 12])
    verts = scene_verts(ctx, n)
    wb(ctx, verts + 0x10, struct.pack('<f', r.choice([0.0, 1.0, 5.0, 100.0, -3.0, r.uniform(-10, 600)])))
    for i in range(n):
        wf(ctx, verts + i * 0x58 + 0x20, r.choice([24.0, 24.0, 0.0, 1.0, 100.0, float('nan'), -24.0]))
    return [verts, n if r.random() < 0.97 else r.choice([0, 1, 2])]


def g_model(ctx):
    r = ctx.rng
    begin(ctx, keyops=())
    clip_env(ctx)
    camera(ctx)
    fog_globals(ctx)
    w32(ctx, 0x1017b940, r.choice([0, 1, 1]))
    nrec = 12
    table = ctx.buf(0x18 + nrec * 0x58)
    spread = r.choice([50.0, 300.0])
    for i in range(nrec):
        o = table + 0x18 + i * 0x58
        wb(ctx, o + 0xc, struct.pack('<3f', r.uniform(-spread, spread), r.uniform(-spread, spread), r.choice([r.uniform(-20, 60), r.uniform(-5, 1500), r.uniform(0, 120)])))
        wb(ctx, o + 0x40, struct.pack('<3f', r.uniform(0, 1), r.uniform(0, 1), r.uniform(0, 1)))
    holder = ctx.buf(0x40)
    w32(ctx, holder + 0x30, table)
    n = r.choice([3, 3, 4, 5, 6, 8, 12, 2, 0, 1])
    part = ctx.buf(0x80)
    w32(ctx, part + 0x1c, r.choice([0, 0, 0, 1, 0x80000000, 0xffffffff, r.getrandbits(31)]))
    wb(ctx, part + 2, struct.pack('<H', n))
    off = 0x40
    w32(ctx, part + 4, off)
    wb(ctx, part + off, bytes(r.randrange(0, nrec) for _ in range(n)) + b'\0\0\0\0')
    w32(ctx, part + 0x24, holder)
    obj = ctx.buf(0x40)
    wf(ctx, obj + 0x18, r.choice([0.0, 0.0, 1.0, 5.0, 100.0, -3.0, r.uniform(-10, 600), float('nan')]))
    return [obj, part if r.random() < 0.95 else 0]


def g_none(ctx):
    begin(ctx)
    return []


SPECS = [
    dict(name='Render_Ftol', orig=TRAMP, c='Test_Ftol', gen=g_ftol, trials=1500),
    dict(name='Render_FogLevel', orig=0x10058c3e, c='Render_FogLevel', gen=g_fog, trials=1500),
    dict(name='Render_ProjectVertex', orig=0x100a62c4, c='Render_ProjectVertex', gen=g_project, trials=1500),
    dict(name='Render_ClipPolygon', orig=0x100a0512, c='Render_ClipPolygon', gen=g_clip, trials=1500, fp=True),
    dict(name='Render_GetTextureImage', orig=0x100072ff, c='Render_GetTextureImage', gen=g_getimage, trials=1500),
    dict(name='Render_SubmitVerts_10006609', orig=0x10006609, c='Render_SubmitVerts_10006609', gen=g_sub1, trials=1000),
    dict(name='Render_SubmitVerts_10006744', orig=0x10006744, c='Render_SubmitVerts_10006744', gen=g_sub2, trials=1000),
    dict(name='Render_SubmitVerts_100068d9', orig=0x100068d9, c='Render_SubmitVerts_100068d9', gen=g_sub3, trials=1000),
    dict(name='Render_SubmitVertexPtrs_100a0339', orig=0x100a0339, c='Render_SubmitVertexPtrs_100a0339', gen=g_ptrs, trials=800),
    # round 33: Render_SubmitClipped_100ac738 and Sim_100ad1f3 are 1.2's now (rhw 1 / z, ...): spec_w2n_shadow_v12.py
]

# round 31: the texture name table is ours now (w31_map.py: the mapping step around the C run)
exec(open('tools/verify/w31_map.py').read())
SOURCES = SOURCES + [x for x in W31_SOURCES + ['../tools/verify/plat_stub.c'] if x not in SOURCES]
SPECS = w31_wrap(SPECS, None)
